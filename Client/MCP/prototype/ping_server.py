"""疎通確認用の ping を Streamable HTTP で公開する。"""

from mcp.server.fastmcp import FastMCP


mcp = FastMCP("PBVR MCP Prototype", host="127.0.0.1", port=18000, json_response=True)


@mcp.tool()
def ping(message: str = "hello") -> str:
    """受け取った文字列に PC 側であることを示す接頭辞を付けて返す。"""
    return f"pong from PC: {message}"


if __name__ == "__main__":
    mcp.run(transport="streamable-http")
