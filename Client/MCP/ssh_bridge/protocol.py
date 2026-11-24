"""PBVRのJSON LinesとSFTP中継の管理情報を検証する。"""

import json
import uuid

PROTOCOL = "pbvr-jsonl-v1"
VERSION = 1
HEARTBEAT_MAX_AGE = 30
MAX_COMMAND = 64 * 1024
MAX_RESPONSE = 1024 * 1024


def state_request() -> tuple[str, bytes]:
    """一意の要求IDと、PBVR用の状態取得要求1行を返す。"""
    request_id = str(uuid.uuid4())
    request = {"version": VERSION, "request_id": request_id, "command": "get_state"}
    return request_id, (json.dumps(request) + "\n").encode("utf-8")


def session_name(value: object) -> str:
    """パスに使える中継セッションIDを検証して返す。"""
    if (not isinstance(value, str) or len(value) != 32
            or any(char not in "0123456789abcdef" for char in value)):
        raise RuntimeError("Invalid bridge session ID")
    return value


def file_offset(value: object) -> int:
    """管理情報の非負整数オフセットを検証して返す。"""
    if type(value) is not int or value < 0:
        raise RuntimeError("Invalid bridge file offset")
    return value


def json_object(data: bytes) -> dict:
    """UTF-8のJSONオブジェクトを読み、非標準の数値表記を拒否する。"""
    def invalid_constant(value: str) -> None:
        """JSONの文法外の数値をエラーにする。"""
        raise ValueError(f"Invalid JSON constant: {value}")

    try:
        value = json.loads(data.decode("utf-8"), parse_constant=invalid_constant)
    except ValueError as error:
        raise RuntimeError("Invalid UTF-8 JSON in bridge communication") from error
    if not isinstance(value, dict):
        raise RuntimeError("Expected a JSON object in bridge communication")
    return value


def json_line(data: bytes, limit: int) -> dict:
    """サイズ制限内の完全なJSON Linesレコード1件を返す。"""
    if len(data) > limit + 1 or not data.endswith(b"\n") or b"\n" in data[:-1]:
        raise RuntimeError("Expected one complete JSON line within the size limit")
    return json_object(data)


def response_record(data: bytes, request_id: str) -> dict:
    """要求に対応するPBVRの成功または失敗応答を検証して返す。"""
    response = json_line(data, MAX_RESPONSE)
    if (type(response.get("version")) is not int or response["version"] != VERSION
            or response.get("request_id") != request_id
            or not isinstance(response.get("session_id"), str) or not response["session_id"]
            or type(response.get("ok")) is not bool):
        raise RuntimeError("Invalid or mismatched PBVR response envelope")
    key = "result" if response["ok"] else "error"
    if not isinstance(response.get(key), dict):
        raise RuntimeError(f"PBVR response {key} must be an object")
    return response
