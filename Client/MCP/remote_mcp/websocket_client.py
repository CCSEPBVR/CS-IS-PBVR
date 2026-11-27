import argparse
import asyncio
from dataclasses import dataclass
import json
import logging
import os
from pathlib import Path
import sys
from urllib.parse import urlsplit

from websockets.asyncio.client import connect
from websockets.exceptions import InvalidMessage, WebSocketException

from relay import MAX_MESSAGE_SIZE, relay


@dataclass(frozen=True)
class ClientConfig:
    url: str
    connect_timeout: float
    retry_interval: float
    max_message_size: int
    command: str
    args: list[str]
    cwd: Path
    env: dict[str, str]


# JSON 設定を読み込み、接続先、制限値、子プロセスの起動条件を検証する。
def load_config(path):
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    if not isinstance(data, dict):
        raise ValueError("Configuration must be a JSON object")
    websocket = data.get("websocket")
    mcp = data.get("mcp_server")
    if not isinstance(websocket, dict) or not isinstance(mcp, dict):
        raise ValueError("websocket and mcp_server must be JSON objects")

    url = websocket.get("url")
    if not isinstance(url, str):
        raise ValueError("websocket.url is required")
    address = urlsplit(url)
    if (address.scheme != "ws" or address.hostname not in ("127.0.0.1", "::1")
            or not address.port or address.username is not None or address.fragment):
        raise ValueError("websocket.url must specify a loopback IP and an explicit port")
    connect_timeout = websocket.get("connect_timeout", 60)
    retry_interval = websocket.get("retry_interval", 1)
    for value in (connect_timeout, retry_interval):
        if type(value) not in (int, float) or not 0 < value <= 86400:
            raise ValueError("Timeout and retry interval must be positive and at most 86400 seconds")
    limit = websocket.get("max_message_size", MAX_MESSAGE_SIZE)
    if type(limit) is not int or limit < 1:
        raise ValueError("websocket.max_message_size must be a positive integer")

    command = mcp.get("command")
    args = mcp.get("args", [])
    cwd = mcp.get("cwd", ".")
    env = mcp.get("env", {})
    if not isinstance(command, str) or not command.strip() or "\0" in command:
        raise ValueError("mcp_server.command must be a non-empty string")
    if not isinstance(args, list) or any(not isinstance(arg, str) or "\0" in arg for arg in args):
        raise ValueError("mcp_server.args must be an array of strings")
    if not isinstance(cwd, str) or not cwd or "\0" in cwd:
        raise ValueError("mcp_server.cwd must be a non-empty path")
    if not isinstance(env, dict) or any(
        not name or "=" in name or "\0" in name or not isinstance(value, str) or "\0" in value
        for name, value in env.items()
    ):
        raise ValueError("mcp_server.env must map environment variable names to string values")

    return ClientConfig(url, connect_timeout, retry_interval, limit, command, args,
                        (path.parent / cwd).resolve(), env)


# SSH トンネル経由で最初の接続を確立するまで、一時的な接続失敗だけを再試行する。
async def connect_initial(config):
    try:
        async with asyncio.timeout(config.connect_timeout):
            while True:
                try:
                    return await connect(
                        config.url, proxy=None, max_size=config.max_message_size, max_queue=4,
                        compression=None, open_timeout=5, close_timeout=3,
                    )
                except (OSError, InvalidMessage):
                    await asyncio.sleep(config.retry_interval)
    except TimeoutError as exc:
        raise TimeoutError(f"Timed out connecting to {config.url}") from exc


# 子プロセスの残りの出力を読み捨て、パイプの詰まりによる終了待ちを防ぐ。
async def discard_output(reader):
    while await reader.read(65536):
        pass


# stdin を閉じて終了を待ち、猶予時間を過ぎた場合は子プロセスを停止する。
async def stop_process(process):
    process.stdin.close()
    drain = asyncio.create_task(discard_output(process.stdout))
    try:
        try:
            await asyncio.wait_for(process.wait(), timeout=3)
        except TimeoutError:
            try:
                process.kill()
            except ProcessLookupError:
                pass
            await asyncio.wait_for(process.wait(), timeout=3)
    finally:
        drain.cancel()
        await asyncio.gather(drain, return_exceptions=True)


# WebSocket 接続後に MCP サーバーを起動し、中継終了時に子プロセスを回収する。
async def run(config):
    connection = await connect_initial(config)
    try:
        process = await asyncio.create_subprocess_exec(
            config.command, *config.args, cwd=config.cwd, env={**os.environ, **config.env},
            stdin=asyncio.subprocess.PIPE, stdout=asyncio.subprocess.PIPE,
            limit=config.max_message_size + 2,
        )
        logging.info("Connected to %s and started the MCP server", config.url)
        terminated_early = False
        try:
            await relay(process.stdout, process.stdin, connection, config.max_message_size)
            terminated_early = process.stdout.at_eof() or process.returncode is not None
        finally:
            await stop_process(process)
        if terminated_early and process.returncode:
            raise OSError(f"MCP server exited with status {process.returncode}")
    finally:
        await connection.close()


# 設定を検証してクライアント側の中継を起動し、終了コードを返す。
def main():
    parser = argparse.ArgumentParser(description="Relay a local stdio MCP server through SSH")
    parser.add_argument("--config", type=Path, required=True)
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s", stream=sys.stderr)
    try:
        asyncio.run(run(load_config(args.config.resolve())))
    except (OSError, ValueError, WebSocketException) as exc:
        logging.error("MCP relay failed: %s", exc)
        return 1
    except KeyboardInterrupt:
        return 130
    return 0


if __name__ == "__main__":
    sys.exit(main())
