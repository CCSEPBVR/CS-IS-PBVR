"""PC から張った接続で s86 のローカルポートを PC に転送する。"""

import argparse
import asyncio
import contextlib
import getpass
import hashlib
import hmac
import logging
import os
import secrets


NODE_PORT = 60000
NODE_HTTP_PORT = 60001
PC_TUNNEL_PORT = 18001
PC_HTTP_PORT = 18000
HANDSHAKE_TIMEOUT = 10
LOG = logging.getLogger(__name__)


def signature(token, challenge, role):
    """接続ごとのチャレンジと役割に対する認証値を返す。"""
    return hmac.new(token, challenge + b" " + role, hashlib.sha256).hexdigest().encode("ascii")


async def read_line(reader):
    """期限内に改行で終わる制御メッセージを1行読み取る。"""
    line = await asyncio.wait_for(reader.readline(), HANDSHAKE_TIMEOUT)
    if not line.endswith(b"\n") or len(line) > 256:
        raise ValueError("Missing or invalid tunnel handshake")
    return line[:-1]


async def close_writer(writer):
    """書き込み側を閉じ、切断時の例外を処理する。"""
    writer.close()
    with contextlib.suppress(OSError):
        await writer.wait_closed()


async def copy_stream(reader, writer):
    """一方向のバイト列を転送し、EOF を相手側に伝える。"""
    while True:
        data = await reader.read(65536)
        if not data:
            writer.write_eof()
            await writer.drain()
            return
        writer.write(data)
        await writer.drain()


async def relay(left_reader, left_writer, right_reader, right_writer):
    """2本の TCP 接続間で、両方向のストリームを転送する。"""
    tasks = [
        asyncio.create_task(copy_stream(left_reader, right_writer)),
        asyncio.create_task(copy_stream(right_reader, left_writer)),
    ]
    try:
        await asyncio.gather(*tasks)
    finally:
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)


class NodeTunnel:
    def __init__(self, token):
        """接続認証に使うトークンと保留中の転送を初期化する。"""
        self.token = token
        self.control = None
        self.pending = {}

    async def accept_pc(self, reader, writer):
        """PC の制御接続を登録するか、要求済みのデータ接続を引き渡す。"""
        handed_off = False
        try:
            challenge = secrets.token_hex(32).encode("ascii")
            writer.write(challenge + b"\n")
            await writer.drain()
            proof, _, role = (await read_line(reader)).partition(b" ")
            if not hmac.compare_digest(proof, signature(self.token, challenge, role)):
                raise ValueError("Tunnel authentication failed")

            if role == b"control":
                if self.control is not None:
                    raise ValueError("A PC control connection is already active")
                self.control = writer
                try:
                    writer.write(b"OK\n")
                    await writer.drain()
                    LOG.info("PC tunnel connected")
                    await reader.read(1)
                finally:
                    self.control = None
                    for future in self.pending.values():
                        if not future.done():
                            future.set_exception(ConnectionError("PC control connection closed"))
                    LOG.info("PC tunnel disconnected")
            elif role.startswith(b"data "):
                future = self.pending.get(role[5:])
                if future is None or future.done():
                    raise ValueError("No matching HTTP connection")
                writer.write(b"OK\n")
                await writer.drain()
                if not future.done():
                    future.set_result((reader, writer))
                    handed_off = True
            else:
                raise ValueError("Unknown tunnel connection role")
        except (OSError, asyncio.TimeoutError, ValueError) as error:
            LOG.warning("PC connection rejected or closed: %s", error)
        finally:
            if not handed_off:
                await close_writer(writer)

    async def accept_http(self, reader, writer):
        """s86 側の HTTP 接続に対応する PC 側データ接続を要求する。"""
        connection_id = secrets.token_hex(16).encode("ascii")
        future = asyncio.get_running_loop().create_future()
        self.pending[connection_id] = future
        try:
            if self.control is None:
                raise ConnectionError("PC tunnel is not connected")
            self.control.write(connection_id + b"\n")
            await asyncio.wait_for(self.control.drain(), HANDSHAKE_TIMEOUT)
            pc_reader, pc_writer = await asyncio.wait_for(future, HANDSHAKE_TIMEOUT)
            await relay(reader, writer, pc_reader, pc_writer)
        except (OSError, asyncio.TimeoutError) as error:
            LOG.warning("HTTP connection closed: %s", error or "timed out waiting for the PC")
        finally:
            del self.pending[connection_id]
            if future.done() and not future.cancelled() and future.exception() is None:
                await close_writer(future.result()[1])
            else:
                future.cancel()
            await close_writer(writer)


async def connect_to_node(token, role):
    """ローカルの SSH 転送に接続し、指定した役割で認証する。"""
    reader, writer = await asyncio.wait_for(
        asyncio.open_connection("127.0.0.1", PC_TUNNEL_PORT), HANDSHAKE_TIMEOUT
    )
    connected = False
    try:
        challenge = await read_line(reader)
        writer.write(signature(token, challenge, role) + b" " + role + b"\n")
        await writer.drain()
        if await read_line(reader) != b"OK":
            raise ValueError("The s86 tunnel did not acknowledge the handshake")
        connected = True
        return reader, writer
    finally:
        if not connected:
            await close_writer(writer)


async def connect_http(token, connection_id):
    """PC の MCP サーバーと、要求元に対応するデータ接続をつなぐ。"""
    try:
        pc_reader, pc_writer = await asyncio.wait_for(
            asyncio.open_connection("127.0.0.1", PC_HTTP_PORT), HANDSHAKE_TIMEOUT
        )
        try:
            node_reader, node_writer = await connect_to_node(token, b"data " + connection_id)
            try:
                await relay(node_reader, node_writer, pc_reader, pc_writer)
            finally:
                await close_writer(node_writer)
        finally:
            await close_writer(pc_writer)
    except (OSError, asyncio.TimeoutError, ValueError) as error:
        LOG.warning("Could not complete the PC HTTP connection: %s", error or "timed out")


async def run_pc(token):
    """PC 側から制御接続を張り、HTTP 接続ごとの転送を管理する。"""
    reader, writer = await connect_to_node(token, b"control")
    LOG.info("Connected to the s86 tunnel; forwarding to 127.0.0.1:%d", PC_HTTP_PORT)
    tasks = set()
    try:
        while True:
            line = await reader.readline()
            if not line:
                raise ConnectionError("s86 control connection closed")
            connection_id = line.rstrip(b"\n")
            if len(connection_id) != 32 or not line.endswith(b"\n"):
                raise ValueError("Invalid connection ID")
            task = asyncio.create_task(connect_http(token, connection_id))
            tasks.add(task)
            task.add_done_callback(tasks.discard)
    finally:
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)
        await close_writer(writer)


async def run_node(token, bind):
    """計算ノードの転送用ポートとループバックの HTTP ポートを開く。"""
    tunnel = NodeTunnel(token)
    pc_server = await asyncio.start_server(tunnel.accept_pc, bind, NODE_PORT, limit=1024)
    async with pc_server:
        http_server = await asyncio.start_server(tunnel.accept_http, "127.0.0.1", NODE_HTTP_PORT)
        async with http_server:
            LOG.info("Waiting for PC tunnel on %s:%d", bind, NODE_PORT)
            LOG.info("MCP endpoint on s86: http://127.0.0.1:%d/mcp", NODE_HTTP_PORT)
            await pc_server.serve_forever()


def main():
    """コマンドラインと検証用トークンを読み取り、指定側の中継を起動する。"""
    parser = argparse.ArgumentParser(description="PBVR MCP reverse tunnel prototype")
    modes = parser.add_subparsers(dest="mode", required=True)
    node = modes.add_parser("node")
    node.add_argument("--bind", required=True, help="Compute node address reachable from the SSH login node")
    modes.add_parser("pc")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
    token = os.environ.get("PBVR_MCP_PROTOTYPE_TOKEN") or getpass.getpass("Prototype token: ")
    if len(token) < 32:
        parser.error("The prototype token must contain at least 32 characters")
    try:
        if args.mode == "node":
            asyncio.run(run_node(token.encode("utf-8"), args.bind))
        else:
            asyncio.run(run_pc(token.encode("utf-8")))
    except (OSError, asyncio.TimeoutError, ValueError) as error:
        parser.exit(1, f"Tunnel stopped: {error or 'timed out'}\n")
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
