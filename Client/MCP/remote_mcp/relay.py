import asyncio


MAX_MESSAGE_SIZE = 64 * 1024 * 1024


# stdio の行末を取り除き、サイズを検査して WebSocket 用の UTF-8 文字列にする。
def decode_line(line, limit):
    body = line.removesuffix(b"\n").removesuffix(b"\r")
    if not line.endswith(b"\n") or not body or b"\r" in body:
        raise ValueError("Invalid stdio message framing")
    if len(body) > limit:
        raise ValueError("MCP message exceeds the size limit")
    return body.decode("utf-8")


# WebSocket のテキストメッセージを stdio の UTF-8 一行に変換する。
def encode_line(message, limit):
    if not isinstance(message, str) or not message or "\n" in message or "\r" in message:
        raise ValueError("Expected a single MCP text message")
    body = message.encode("utf-8")
    if len(body) > limit:
        raise ValueError("MCP message exceeds the size limit")
    return body + b"\n"


# stdio から読み取ったメッセージを、読み取り順に WebSocket へ送信する。
async def send_messages(reader, connection, limit):
    while line := await reader.readline():
        await connection.send(decode_line(line, limit))


# WebSocket から受信したメッセージを、受信順に stdio へ書き込む。
async def receive_messages(connection, writer, limit):
    async for message in connection:
        writer.write(encode_line(message, limit))
        await writer.drain()


# 双方向の転送を開始し、片方向の終了に合わせてもう一方を停止する。
async def relay(reader, writer, connection, limit):
    tasks = (
        asyncio.create_task(send_messages(reader, connection, limit)),
        asyncio.create_task(receive_messages(connection, writer, limit)),
        asyncio.create_task(connection.wait_closed()),
    )
    try:
        done, _ = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
        for task in done:
            task.result()
        if tasks[2] in done and connection.close_code not in (1000, 1001):
            raise OSError(f"WebSocket closed with code {connection.close_code}")
    finally:
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)
