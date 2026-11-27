"""PBVRのMCPツールをstdioで公開する。"""
import logging

from mcp.server.fastmcp import FastMCP

from transport import FileTransport


logging.basicConfig(level=logging.INFO)  # MCPのstdioを保つためログは標準エラーに出す。
mcp = FastMCP("PBVR")
transport = FileTransport.from_environment()


@mcp.tool()
async def get_state() -> dict:
    """適用済みの状態を取得し、PBVRの応答を返す。

    データ要求や描画待機は行わない。GUIの処理状況によって応答は遅れる。
    """
    return await transport.request("get_state")


if __name__ == "__main__":
    mcp.run(transport="stdio")
