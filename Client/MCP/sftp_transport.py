"""共有領域を介してSFTP中継へMCP要求を渡す。"""

import json
import logging
import os
from pathlib import Path
import threading
import time
import uuid

from ssh_bridge.protocol import (
    HEARTBEAT_MAX_AGE, MAX_RESPONSE, PROTOCOL, file_offset, json_object,
    response_record, session_name, state_request,
)
from transport import FileTransport, _RequestCancelled, _check_cancelled

LOG = logging.getLogger(__name__)


def read_metadata(path: Path) -> dict:
    """共有領域の管理情報を読み、未作成の場合は空の辞書を返す。"""
    try:
        return json_object(path.read_bytes())
    except FileNotFoundError:
        return {}


def write_metadata(path: Path, value: dict) -> None:
    """共有領域へ管理情報を原子的に公開する。"""
    temporary = path.with_name(f"{path.name}.{uuid.uuid4().hex}.tmp")
    try:
        with temporary.open("xb") as stream:
            stream.write(json.dumps(value).encode("utf-8"))
            stream.flush()
            os.fsync(stream.fileno())
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)


class SftpFileTransport(FileTransport):
    """既存の要求直列化を使い、共有領域の中継セッションと通信する。"""

    def _active_session(self) -> str:
        """現在公開されている互換中継のセッションIDを返す。"""
        active = read_metadata(self.directory / "bridge.active.json")
        if active.get("protocol") != PROTOCOL:
            raise RuntimeError("No compatible PBVR bridge session is ready")
        return session_name(active.get("session"))

    def _status(self, directory: Path) -> dict:
        """中継のセッションとハートビートを検証し、状態を返す。"""
        path = directory / "bridge.status.json"
        with path.open("rb") as stream:
            status = json_object(stream.read())
            age = time.time() - os.fstat(stream.fileno()).st_mtime
        if (status.get("session") != directory.name or status.get("protocol") != PROTOCOL
                or status.get("state") not in ("ready", "busy") or age > HEARTBEAT_MAX_AGE):
            raise RuntimeError("PBVR bridge has stopped or its heartbeat expired")
        return status

    def _request(self, command: str, cancelled: threading.Event) -> dict:
        """要求を現在の中継へ1件公開し、対応するPBVR応答を返す。"""
        _check_cancelled(cancelled)
        if command != "get_state":
            raise ValueError("The PBVR bridge supports only get_state")
        session = self._active_session()
        directory = self.directory / session
        halt_path = directory / "bridge.halt.json"
        request_path = directory / "bridge.request.json"
        done_path = directory / "bridge.done.json"
        if halt_path.exists():
            raise RuntimeError("PBVR bridge session is halted; restart the bridge and PBVR")
        previous = read_metadata(request_path)
        done = read_metadata(done_path)
        status = self._status(directory)
        if (status["state"] != "ready" or previous and (
                done.get("session") != session or done.get("request_id") != previous.get("request_id"))):
            raise RuntimeError("PBVR bridge is still handling the preceding request")

        command_path = directory / "command.txt"
        log_path = directory / "structured.log"
        command_start = file_offset(status.get("command_end"))
        log_start = file_offset(status.get("log_end"))
        request_id, payload = state_request()
        request = {"session": session, "request_id": request_id, "command_start": command_start,
                   "command_end": command_start + len(payload), "log_start": log_start,
                   "timeout": self.timeout}
        if command_path.stat().st_size != command_start or log_path.stat().st_size != log_start:
            raise RuntimeError("Shared communication files differ from bridge offsets")
        if self._active_session() != session:
            raise RuntimeError("PBVR bridge session changed before sending the request")
        _check_cancelled(cancelled)
        published = False
        try:
            with command_path.open("ab") as stream:
                if stream.write(payload) != len(payload):
                    raise OSError("Incomplete shared command write")
                stream.flush()
                os.fsync(stream.fileno())
            write_metadata(request_path, request)
            published = True
            deadline = time.monotonic() + self.timeout
            while True:
                _check_cancelled(cancelled)
                if self._active_session() != session:
                    raise RuntimeError("PBVR bridge session changed while waiting for a response")
                done = read_metadata(done_path)
                if done.get("session") == session and done.get("request_id") == request_id:
                    length = file_offset(done.get("log_end")) - log_start
                    if not 0 < length <= MAX_RESPONSE + 1:
                        raise RuntimeError("Invalid bridge response range")
                    with log_path.open("rb") as stream:
                        stream.seek(log_start)
                        data = stream.read(length)
                    if len(data) != length:
                        raise RuntimeError("Incomplete bridge response transfer")
                    return response_record(data, request_id)
                self._status(directory)
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError(f"PBVR bridge response timed out (request_id={request_id})")
                cancelled.wait(min(0.1, remaining))
        except Exception as error:
            # 公開済み要求は呼び出し元がキャンセルしても、中継で完了できる。
            if not (published and isinstance(error, _RequestCancelled)):
                try:
                    write_metadata(halt_path, request)
                except OSError:
                    LOG.exception("Could not publish the bridge halt marker")
            raise
