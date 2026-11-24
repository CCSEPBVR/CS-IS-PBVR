"""手動起動したPBVRとスパコンの共有領域をSFTPで中継する。"""

import argparse
from dataclasses import dataclass
import errno
import getpass
import json
import logging
import math
import os
from pathlib import Path
import posixpath
import time
import uuid

import paramiko

from .protocol import (
    HEARTBEAT_MAX_AGE, MAX_COMMAND, MAX_RESPONSE, PROTOCOL, VERSION, file_offset,
    json_line, json_object, response_record, session_name, state_request,
)

LOG = logging.getLogger(__name__)


@dataclass
class PendingRequest:
    """PCに送信済みの要求と、再接続時にも使う応答のバイト列。"""

    request_id: str
    command_end: int
    deadline: float
    response: bytes | None = None


class Bridge:
    """単一セッションの要求と応答を順に転送する。"""

    def __init__(self, config: dict, config_dir: Path):
        """設定を解決し、新しいセッションと転送位置を初期化する。"""
        self.config = config
        self.session = uuid.uuid4().hex
        self.remote_root = config["remote_ctrl_dir"]
        if not isinstance(self.remote_root, str) or not self.remote_root.startswith("/"):
            raise ValueError("remote_ctrl_dir must be an absolute SFTP path")
        self.remote = posixpath.join(self.remote_root, self.session)
        self.active_path = posixpath.join(self.remote_root, "bridge.active.json")
        base = Path(config.get("local_ctrl_dir", "sessions")).expanduser()
        self.local = (config_dir / base / self.session).resolve()
        self.command_path = self.local / "command.txt"
        self.log_path = self.local / "structured.log"
        self.poll_interval = float(config.get("poll_interval", 0.5))
        self.startup_timeout = float(config.get("startup_timeout", 30))
        self.reconnect_timeout = float(config.get("reconnect_timeout", 10))
        if not (0.1 <= self.poll_interval <= 3 and 0 < self.reconnect_timeout <= 10
                and math.isfinite(self.startup_timeout) and self.startup_timeout > 0):
            raise ValueError("Expected poll_interval in 0.1..3, reconnect_timeout in (0, 10], "
                             "and a finite positive startup_timeout")
        self.passphrase = None
        self.client = None
        self.sftp = None
        self.owns_remote = False
        self.command_end = 0
        self.log_end = 0
        self.local_command_base = 0
        self.local_log_base = 0
        self.pbvr_session = ""
        self.pending = None
        self.last_done = None

    def connect(self) -> None:
        """既知のホスト鍵と公開鍵認証を使ってSFTP接続を開く。"""
        self.disconnect()
        self.client = paramiko.SSHClient()
        self.client.load_system_host_keys()
        known_hosts = self.config.get("known_hosts")
        if known_hosts:
            self.client.load_host_keys(os.path.expanduser(known_hosts))
        self.client.set_missing_host_key_policy(paramiko.RejectPolicy())
        key = self.config.get("key_filename")
        self.client.connect(
            hostname=self.config["host"], port=int(self.config.get("port", 22)),
            username=self.config["username"], key_filename=os.path.expanduser(key) if key else None,
            passphrase=self.passphrase, allow_agent=True, look_for_keys=False,
            timeout=5, banner_timeout=5, auth_timeout=5, channel_timeout=5,
        )
        self.client.get_transport().set_keepalive(5)
        self.sftp = self.client.open_sftp()
        self.sftp.get_channel().settimeout(5)

    def disconnect(self) -> None:
        """現在のSSH接続とSFTPチャネルを閉じる。"""
        if self.client is not None:
            self.client.close()
        self.client = None
        self.sftp = None

    def read_metadata(self, path: str) -> tuple[dict, float]:
        """SFTP上の管理情報とその更新時刻を返し、未作成なら空の辞書を返す。"""
        try:
            with self.sftp.open(path, "rb") as stream:
                return json_object(stream.read()), stream.stat().st_mtime
        except OSError as error:
            if error.errno != errno.ENOENT:
                raise
            return {}, 0

    def write_metadata(self, path: str, value: dict) -> None:
        """SFTP上の管理情報を一時ファイルから原子的に置換する。"""
        temporary = f"{path}.{self.session}.tmp"
        with self.sftp.open(temporary, "wb") as stream:
            stream.write(json.dumps(value).encode("utf-8"))
            stream.flush()
        self.sftp.posix_rename(temporary, path)

    def status(self, state: str, command_end: int, log_end: int) -> None:
        """セッションの状態と転送位置をハートビートとして公開する。"""
        self.write_metadata(posixpath.join(self.remote, "bridge.status.json"), {
            "protocol": PROTOCOL, "session": self.session, "state": state,
            "command_end": command_end, "log_end": log_end,
        })

    def append_command(self, data: bytes, offset: int) -> None:
        """既存のPC側コマンドファイルの末尾へ要求を1回だけ追記する。"""
        try:
            with self.command_path.open("r+b") as stream:
                if stream.seek(0, os.SEEK_END) != offset:
                    raise RuntimeError("Local command file differs from the bridge offset")
                if stream.write(data) != len(data):
                    raise RuntimeError("Incomplete local command write")
                stream.flush()
                os.fsync(stream.fileno())
        except OSError as error:
            raise RuntimeError(f"Local command write failed: {error}") from error

    def read_response(self, offset: int) -> bytes | None:
        """PC側の指定位置から完全な応答行を読み、未完ならNoneを返す。"""
        try:
            with self.log_path.open("rb") as stream:
                if os.fstat(stream.fileno()).st_size < offset:
                    raise RuntimeError("Local PBVR log was truncated")
                stream.seek(offset)
                data = stream.read(MAX_RESPONSE + 2)
        except OSError as error:
            raise RuntimeError(f"Local PBVR log read failed: {error}") from error
        if len(data) > MAX_RESPONSE + 1:
            raise RuntimeError("Local PBVR response exceeds the size limit")
        return data if b"\n" in data else None

    def prepare_pbvr(self) -> None:
        """手動起動を待ち、PC内の状態取得でPBVRの受付を確認する。"""
        self.local.mkdir(parents=True, exist_ok=False)
        self.command_path.touch(exist_ok=False)
        self.log_path.touch(exist_ok=False)
        LOG.info("Set PBVR_CTRL_DIR to this absolute path and start PBVR manually: %s", self.local)
        input("After PBVR has started, press Enter to check readiness: ")
        request_id, data = state_request()
        self.append_command(data, 0)
        deadline = time.monotonic() + self.startup_timeout
        while time.monotonic() < deadline:
            response = self.read_response(0)
            if response is not None:
                record = response_record(response, request_id)
                if not record["ok"]:
                    raise RuntimeError("PBVR rejected the readiness request")
                self.pbvr_session = record["session_id"]
                self.local_command_base = len(data)
                self.local_log_base = len(response)
                return
            time.sleep(self.poll_interval)
        raise TimeoutError("PBVR readiness check timed out; restart the bridge and PBVR")

    def publish_session(self) -> None:
        """準備済みの新しいセッションをスパコン側の共有領域へ公開する。"""
        active, _ = self.read_metadata(self.active_path)
        if active:
            previous = session_name(active.get("session"))
            path = posixpath.join(self.remote_root, previous, "bridge.status.json")
            status, updated = self.read_metadata(path)
            if (status and status.get("state") not in ("stopped", "error")
                    and time.time() - updated <= HEARTBEAT_MAX_AGE):
                raise RuntimeError("Another bridge is active; stop it before starting a new bridge")
        self.sftp.mkdir(self.remote)
        self.owns_remote = True
        for name in ("command.txt", "structured.log"):
            with self.sftp.open(posixpath.join(self.remote, name), "wb"):
                pass
        self.status("ready", 0, 0)
        self.write_metadata(self.active_path, {"protocol": PROTOCOL, "session": self.session})

    def dispatch(self, request: dict) -> None:
        """共有領域の要求を照合し、PBVRへ1回だけ渡す。"""
        start = file_offset(request.get("command_start"))
        end = file_offset(request.get("command_end"))
        timeout = request.get("timeout")
        request_id = request.get("request_id")
        if (request.get("session") != self.session or start != self.command_end
                or file_offset(request.get("log_start")) != self.log_end
                or not 0 < end - start <= MAX_COMMAND + 1
                or not isinstance(request_id, str) or not request_id
                or type(timeout) not in (int, float) or not math.isfinite(timeout) or timeout <= 0):
            raise RuntimeError("Invalid bridge request metadata or offsets")
        with self.sftp.open(posixpath.join(self.remote, "command.txt"), "rb") as stream:
            if stream.stat().st_size != end:
                raise RuntimeError("Shared command file differs from the requested range")
            stream.seek(start)
            data = stream.read(end - start)
        if len(data) != end - start:
            raise RuntimeError("Incomplete shared command transfer")
        command = json_line(data, MAX_COMMAND)
        if (type(command.get("version")) is not int or command["version"] != VERSION
                or command.get("request_id") != request_id or command.get("command") != "get_state"):
            raise RuntimeError("Shared command does not match the pending get_state request")
        try:
            log_size = self.log_path.stat().st_size
        except OSError as error:
            raise RuntimeError(f"Local PBVR log stat failed: {error}") from error
        if log_size != self.local_log_base + self.log_end:
            raise RuntimeError("Local PBVR log changed outside a bridge request")
        self.append_command(data, self.local_command_base + start)
        self.pending = PendingRequest(request_id, end, time.monotonic() + timeout)

    def collect(self, pending: PendingRequest) -> None:
        """応答を確定し、未転送のバイト列と完了通知を共有領域へ書く。"""
        if pending.response is None:
            data = self.read_response(self.local_log_base + self.log_end)
            if data is None:
                if time.monotonic() >= pending.deadline:
                    raise RuntimeError("PBVR response timed out; the request will not be replayed")
                self.status("busy", self.command_end, self.log_end)
                return
            record = response_record(data, pending.request_id)
            if record["session_id"] != self.pbvr_session:
                raise RuntimeError("PBVR restarted; restart the bridge and PBVR together")
            pending.response = data

        data = pending.response
        end = self.log_end + len(data)
        with self.sftp.open(posixpath.join(self.remote, "structured.log"), "r+b") as stream:
            size = stream.stat().st_size
            if not self.log_end <= size <= end:
                raise RuntimeError("Shared log size differs from the pending transfer")
            stream.seek(self.log_end)
            sent = size - self.log_end
            if stream.read(sent) != data[:sent]:
                raise RuntimeError("Shared log content differs from the pending response")
            if sent < len(data):
                stream.seek(size)
                stream.write(data[sent:])
                stream.flush()

        # 次の要求は完了通知と更新済みオフセットの両方を確認してから送られる。
        self.status("ready", pending.command_end, end)
        self.write_metadata(posixpath.join(self.remote, "bridge.done.json"), {
            "session": self.session, "request_id": pending.request_id, "log_end": end,
        })
        self.command_end = pending.command_end
        self.log_end = end
        self.last_done = pending.request_id
        self.pending = None

    def step(self) -> None:
        """セッションの有効性を確認し、要求または応答を1段階進める。"""
        active, _ = self.read_metadata(self.active_path)
        if active.get("session") != self.session or active.get("protocol") != PROTOCOL:
            raise RuntimeError("Active bridge session changed")
        halt, _ = self.read_metadata(posixpath.join(self.remote, "bridge.halt.json"))
        if halt:
            raise RuntimeError("MCP server halted the session; restart the bridge and PBVR")
        if self.pending is not None:
            self.collect(self.pending)
            return
        request, _ = self.read_metadata(posixpath.join(self.remote, "bridge.request.json"))
        if request and (self.last_done is None or request.get("request_id") != self.last_done):
            self.dispatch(request)
        self.status("busy" if self.pending else "ready", self.command_end, self.log_end)

    def run(self) -> None:
        """準備完了後に中継を開始し、一時切断時に同じセッションへ再接続する。"""
        self.prepare_pbvr()
        if self.config.get("prompt_passphrase", True):
            self.passphrase = getpass.getpass("SSH key passphrase: ")
        self.connect()
        self.publish_session()
        LOG.info("Bridge ready. Session: %s; shared directory: %s", self.session, self.remote)
        disconnected_at = None
        while True:
            try:
                if self.sftp is None:
                    self.connect()
                self.step()
                disconnected_at = None
            except (EOFError, OSError, paramiko.SSHException) as error:
                if isinstance(error, (paramiko.AuthenticationException, paramiko.BadHostKeyException)):
                    raise
                if isinstance(error, OSError) and error.errno not in (
                        None, errno.ECONNRESET, errno.EPIPE, errno.ETIMEDOUT, errno.ECONNABORTED,
                        errno.ECONNREFUSED, errno.ENETUNREACH, errno.EHOSTUNREACH):
                    raise
                if disconnected_at is None:
                    disconnected_at = time.monotonic()
                    LOG.warning("SFTP connection lost; attempting recovery: %s", error)
                self.disconnect()
                if time.monotonic() - disconnected_at + self.poll_interval >= self.reconnect_timeout:
                    raise RuntimeError("SFTP recovery deadline expired") from error
            time.sleep(self.poll_interval)

    def close(self) -> None:
        """可能なら終了状態を通知し、SSH接続を閉じる。"""
        try:
            if self.owns_remote and self.sftp is not None:
                self.status("stopped", self.command_end, self.log_end)
        except (EOFError, OSError, paramiko.SSHException):
            pass
        finally:
            self.disconnect()
            self.passphrase = None


def main() -> int:
    """設定ファイルを読み、中継を起動して終了コードを返す。"""
    parser = argparse.ArgumentParser(description="Relay PBVR requests and responses over SFTP")
    parser.add_argument("--config", type=Path, required=True)
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
    bridge = None
    try:
        path = args.config.resolve()
        bridge = Bridge(json_object(path.read_bytes()), path.parent)
        bridge.run()
    except KeyboardInterrupt:
        LOG.info("Bridge stopped by user")
        return 0
    except Exception as error:
        LOG.error("Bridge stopped: %s", error)
        return 1
    finally:
        if bridge is not None:
            bridge.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
