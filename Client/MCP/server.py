"""PBVRのMCPツールをstdioで公開する。"""
import logging
import os

from mcp.server.fastmcp import FastMCP

from transport import FileTransport

def create_transport() -> FileTransport:
    """環境変数が指定する通信方式を初期化して返す。"""
    mode = os.environ.get("PBVR_TRANSPORT", "local")
    if mode == "local":
        return FileTransport.from_environment()
    if mode == "sftp-files":
        from sftp_transport import SftpFileTransport

        return SftpFileTransport.from_environment()
    raise ValueError("PBVR_TRANSPORT must be local or sftp-files")


logging.basicConfig(level=logging.INFO)  # MCPのstdioを保つためログは標準エラーに出す。
mcp = FastMCP("PBVR")
transport = create_transport()


@mcp.tool()
async def get_state() -> dict:
    """適用済みの状態を取得し、PBVRの応答を返す。

    データ要求や描画待機は行わない。GUIの処理状況によって応答は遅れる。
    """
    return await transport.request("get_state")


if __name__ == "__main__":
    mcp.run(transport="stdio")
