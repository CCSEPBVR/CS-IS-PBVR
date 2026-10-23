"""Claude Desktop stdio entry point. Never launches PBVR."""
import logging

from mcp.server.fastmcp import FastMCP

from transport import FileTransport

logging.basicConfig(level=logging.INFO)  # logging defaults to stderr
mcp = FastMCP("PBVR")
transport = FileTransport.from_environment()


@mcp.tool()
async def get_state() -> dict:
    """Read PBVR's applied state without requesting data or waiting for rendering.

    PBVR must be started manually with the same PBVR_CTRL_DIR. A busy GUI can
    delay this snapshot. A timeout does not identify its cause.
    """
    return await transport.request("get_state")


if __name__ == "__main__":
    mcp.run(transport="stdio")
