"""Serialized JSON Lines file transport; independent of MCP tool definitions."""
from __future__ import annotations

import asyncio
import hashlib
import json
import logging
import math
import os
import threading
import time
import uuid
from pathlib import Path

import anyio

LOG = logging.getLogger(__name__)
VERSION = 1
MAX_LINE = 1024 * 1024


class _RequestCancelled(Exception):
    """Internal notification from the file worker; never a PBVR undo."""


def _check_cancelled(cancelled: threading.Event) -> None:
    if cancelled.is_set():
        raise _RequestCancelled()


class FileTransport:
    def __init__(self, directory: Path, timeout: float = 120.0):
        if not directory.is_absolute() or not directory.is_dir():
            raise ValueError("PBVR_CTRL_DIR must name an existing absolute directory")
        if not math.isfinite(timeout) or timeout <= 0:
            raise ValueError("PBVR_MCP_TIMEOUT_SECONDS must be finite and positive")
        self.directory = directory.resolve(strict=True)
        self.timeout = timeout
        self._lock = asyncio.Lock()

    @classmethod
    def from_environment(cls) -> FileTransport:
        raw = os.environ.get("PBVR_CTRL_DIR", "")
        if not raw:
            raise ValueError("PBVR_CTRL_DIR is required (no fallback directory)")
        return cls(Path(raw), float(os.environ.get("PBVR_MCP_TIMEOUT_SECONDS", "120")))

    async def request(self, command: str) -> dict:
        # The lock includes the complete wait, and remains held after cancellation
        # until the file worker exits. Thus cancellation cannot overlap requests.
        async with self._lock:
            cancelled = threading.Event()
            task = asyncio.create_task(asyncio.to_thread(self._request, command, cancelled))
            try:
                return await asyncio.shield(task)
            except asyncio.CancelledError:
                cancelled.set()
                # AnyIO level cancellation must not interrupt cleanup on each
                # checkpoint. Also tolerate repeated direct asyncio cancellation.
                # Never cancel the to_thread task: its completion is our proof
                # that the real file worker has exited and closed its files.
                with anyio.CancelScope(shield=True):
                    while not task.done():
                        try:
                            await asyncio.shield(task)
                        except asyncio.CancelledError:
                            continue
                        except Exception:
                            break
                    try:
                        task.result()
                    except (Exception, asyncio.CancelledError):
                        pass  # Retrieve worker errors; caller remains cancelled.
                raise

    def _request(self, command: str, cancelled: threading.Event) -> dict:
        _check_cancelled(cancelled)
        request_id = str(uuid.uuid4())
        request = {"version": VERSION, "request_id": request_id, "command": command}
        log_path = self.directory / "structured.log"
        # PBVR normally creates the log first. Creating an empty log is safe when
        # PBVR has not started; that case will simply time out.
        with log_path.open("ab"):
            pass
        with log_path.open("rb") as log:
            identity = os.fstat(log.fileno())
            prefix = hashlib.sha256()
            while chunk := log.read(65536):
                _check_cancelled(cancelled)
                prefix.update(chunk)
            offset = log.tell()
            discard_initial_fragment = False
            if offset:
                log.seek(-1, os.SEEK_END)
                discard_initial_fragment = log.read(1) != b"\n"
            command_path = self.directory / "command.txt"
            _check_cancelled(cancelled)
            with command_path.open("a+b") as output:
                output.seek(0, os.SEEK_END)
                if output.tell():
                    output.seek(-1, os.SEEK_END)
                    if output.read(1) != b"\n":
                        raise RuntimeError("command.txt has an incomplete tail; do not append a new request")
                payload = (json.dumps(request, ensure_ascii=False, allow_nan=False) + "\n").encode("utf-8")
                # Last check before sending. Notification can still race with
                # write/flush: a sent request is never removed, undone or retried.
                _check_cancelled(cancelled)
                if output.write(payload) != len(payload):
                    raise OSError("incomplete command write; request is not retried")
                output.flush()

            deadline = time.monotonic() + self.timeout
            pending = b""
            while time.monotonic() < deadline:
                _check_cancelled(cancelled)
                current = log_path.stat()  # deletion/permissions propagate as errors
                if (current.st_dev, current.st_ino) != (identity.st_dev, identity.st_ino):
                    raise RuntimeError("structured.log was replaced; request is not retried")
                if current.st_size < offset:
                    raise RuntimeError("structured.log was truncated; request is not retried")
                # Verify the consumed prefix, including a partially read line.
                # This also detects truncate-and-refill between polls.
                log.seek(0)
                check = hashlib.sha256()
                remaining = offset
                while remaining:
                    _check_cancelled(cancelled)
                    block = log.read(min(65536, remaining))
                    if not block:
                        raise RuntimeError("structured.log changed while reading")
                    check.update(block)
                    remaining -= len(block)
                if check.digest() != prefix.digest():
                    raise RuntimeError("structured.log prefix was modified; request is not retried")
                chunk = log.read(65536)
                prefix.update(chunk)
                offset += len(chunk)
                pending += chunk
                while b"\n" in pending:
                    _check_cancelled(cancelled)
                    line, pending = pending.split(b"\n", 1)
                    if len(line) > MAX_LINE:
                        raise RuntimeError("response line exceeds 1 MiB")
                    if discard_initial_fragment:
                        # This record began before our request. Wait for its LF,
                        # then discard its continuation, never parse half a JSON.
                        discard_initial_fragment = False
                        continue
                    try:
                        record = json.loads(line.decode("utf-8"), parse_constant=_invalid_constant)
                    except (ValueError, UnicodeError) as error:
                        raise RuntimeError("invalid UTF-8/JSON response line") from error
                    if not isinstance(record, dict):
                        raise RuntimeError("response must be a JSON object")
                    if record.get("request_id") != request_id:
                        LOG.debug("Ignoring response for another request")
                        continue
                    if (type(record.get("version")) is not int or record["version"] != VERSION
                            or not isinstance(record.get("session_id"), str) or not record["session_id"]
                            or type(record.get("ok")) is not bool):
                        raise RuntimeError("invalid response envelope")
                    if record["ok"]:
                        if not isinstance(record.get("result"), dict):
                            raise RuntimeError("response result must be an object")
                    elif not isinstance(record.get("error"), dict):
                        raise RuntimeError("response error must be an object")
                    return record
                if len(pending) > MAX_LINE:
                    raise RuntimeError("response line exceeds 1 MiB")
                if not chunk:
                    cancelled.wait(0.1)  # Wake promptly on cancellation.
            _check_cancelled(cancelled)
            raise TimeoutError(f"PBVR response timed out after {self.timeout:g}s (request_id={request_id}); cause unknown; no retry")


def _invalid_constant(value: str):
    raise ValueError(f"non-JSON constant: {value}")
