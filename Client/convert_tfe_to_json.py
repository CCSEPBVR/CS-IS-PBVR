"""旧形式の .tfe を現行 PBVR の伝達関数 JSON に変換する。

使用例:
    python Client/convert_tfe_to_json.py input.tfe
    python Client/convert_tfe_to_json.py input.tfe -o output.json

出力先の省略時は入力の拡張子を .json に変更する。既存ファイルは上書きしない。
入力は UTF-8（BOM 付きも可）とし、式の評価やテーブルの補間は行わない。
"""

import argparse
import json
import math
from pathlib import Path
import re
import sys


# Shared/TransferFunctionFile.h の保存形式と検証条件に合わせる。
FORMAT = "PBVR transfer-function parameters"
SCHEMA_VERSION = 1
RESOLUTION = 256
MAXIMUM_COUNT = 16
FLOAT_MAX = float.fromhex("0x1.fffffep+127")
INTEGER_PATTERN = re.compile(r"[+-]?[0-9]+")
NUMBER_PATTERN = re.compile(r"[+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?")


class TfeError(ValueError):
    """入力の項目名と行番号を伴う変換エラーを表す。"""


def fail(path, line, key, reason):
    """入力位置・項目名・理由を含む例外を送出する。戻り値はない。"""
    raise TfeError(f"{path}:{line}: {key}: {reason}")


def read_entries(path):
    """入力ファイルを読み、キーから（値、行番号）への辞書と末尾行番号を返す。"""
    entries = {}
    lines = path.read_text(encoding="utf-8-sig").splitlines()
    for line_number, line in enumerate(lines, 1):
        if not line.strip():
            continue
        key, separator, value = line.partition("=")
        key = key.strip()
        if not separator or not key:
            fail(path, line_number, "<line>", "expected KEY=VALUE")
        if key in entries:
            fail(path, line_number, key, f"duplicate key; first defined on line {entries[key][1]}")
        entries[key] = (value.strip(), line_number)
    return entries, len(lines) + 1


def get_entry(entries, key, path, end_line):
    """必須項目の（値、行番号）を返す。欠落時は入力末尾を示す例外を送出する。"""
    if key not in entries:
        fail(path, end_line, key, "missing required key (reached end of file)")
    return entries[key]


def parse_integer(value, path, line, key):
    """整数表記を検証し、整数値を返す。"""
    if not INTEGER_PATTERN.fullmatch(value):
        fail(path, line, key, "expected an integer")
    try:
        return int(value)
    except ValueError:
        fail(path, line, key, "integer is too large")


def parse_number(value, path, line, key):
    """サーバーで表現可能な有限の数値を検証し、浮動小数点数を返す。"""
    if not NUMBER_PATTERN.fullmatch(value):
        fail(path, line, key, "expected a number")
    number = float(value)
    if not math.isfinite(number) or abs(number) > FLOAT_MAX:
        fail(path, line, key, "expected a finite number representable by the server")
    return number


def read_expression(entries, key, path, end_line):
    """空文字とヌル文字を検証し、評価せずに式を返す。"""
    value, line = get_entry(entries, key, path, end_line)
    if not value or "\0" in value:
        fail(path, line, key, "expression is empty or contains a null character")
    return value


def read_channel(entries, index, color, path, end_line):
    """指定番号の色または不透明度を検証し、JSON 用のチャネル辞書を返す。"""
    suffix = "C" if color else "O"
    prefix = f"TF_NAME{index}_"
    name_key = prefix + suffix
    name, line = get_entry(entries, name_key, path, end_line)
    if name != f"{suffix}{index}":
        fail(path, line, name_key, f"unsupported channel name; expected {suffix}{index}")

    variable = read_expression(entries, prefix + "VAR_" + suffix, path, end_line)
    min_key = prefix + "MIN_" + suffix
    max_key = prefix + "MAX_" + suffix
    min_value, min_line = get_entry(entries, min_key, path, end_line)
    max_value, max_line = get_entry(entries, max_key, path, end_line)
    minimum = parse_number(min_value, path, min_line, min_key)
    maximum = parse_number(max_value, path, max_line, max_key)
    if minimum > maximum:
        fail(path, entries[max_key][1], max_key, f"must be >= {min_key}")

    table_key = prefix + "TABLE_" + suffix
    table, line = get_entry(entries, table_key, path, end_line)
    tokens = table.split(",")
    # 旧形式で使われる末尾のカンマだけを許容し、途中の空要素は拒否する。
    if tokens[-1].strip() == "":
        tokens.pop()
    length = RESOLUTION * 3 if color else RESOLUTION
    if len(tokens) != length:
        fail(path, line, table_key, f"expected {length} elements, got {len(tokens)}")
    values = []
    for position, token in enumerate(tokens):
        value_key = f"{table_key}[{position}]"
        token = token.strip()
        number = (parse_integer(token, path, line, value_key) if color
                  else parse_number(token, path, line, value_key))
        upper = 255 if color else 1
        if not 0 <= number <= upper:
            fail(path, line, value_key, f"value must be between 0 and {upper}")
        values.append(number)
    return {
        "variable": variable,
        "range": {"user": {"min": minimum, "max": maximum}},
        "map": {
            "role": "color map" if color else "opacity map",
            "encoding": "flat RGB uint8 triplets" if color else "float values from 0 to 1",
            "length": length,
            "values": values,
        },
    }


def convert(path):
    """単一の .tfe を読み込み、全項目の検証を終えた JSON 用辞書を返す。"""
    entries, end_line = read_entries(path)
    resolution_value, line = get_entry(entries, "TF_RESOLUTION", path, end_line)
    if parse_integer(resolution_value, path, line, "TF_RESOLUTION") != RESOLUTION:
        fail(path, line, "TF_RESOLUTION", "only resolution 256 is supported")
    count_value, line = get_entry(entries, "TF_NUMBER", path, end_line)
    count = parse_integer(count_value, path, line, "TF_NUMBER")
    if not 1 <= count <= MAXIMUM_COUNT:
        fail(path, line, "TF_NUMBER", "expected 1 to 16 transfer functions")

    allowed = {"TF_RESOLUTION", "TF_NUMBER", "TF_SYNTH_C", "TF_SYNTH_O"}
    for index in range(1, count + 1):
        for suffix in ("C", "O"):
            for field in ("", "VAR_", "MIN_", "MAX_", "TABLE_"):
                allowed.add(f"TF_NAME{index}_{field}{suffix}")
    for key, (_, line) in entries.items():
        if key not in allowed:
            fail(path, line, key, "unsupported key or transfer function index")

    settings = {
        "transfer_function_count": count,
        "transfer_function_resolution": RESOLUTION,
        "color_synthesis": read_expression(entries, "TF_SYNTH_C", path, end_line),
        "opacity_synthesis": read_expression(entries, "TF_SYNTH_O", path, end_line),
    }
    items = []
    for index in range(1, count + 1):
        items.append({
            "id": index,
            "label": f"TF{index}",
            "color": read_channel(entries, index, True, path, end_line),
            "opacity": read_channel(entries, index, False, path, end_line),
        })
    return {
        "format": FORMAT,
        "schema_version": SCHEMA_VERSION,
        "settings": {"transfer_function": settings},
        "transfer_functions": items,
    }


def serialize(document):
    """辞書を整形した JSON 文字列にし、数値の values 配列だけを1行にして返す。"""
    contents = json.dumps(document, ensure_ascii=False, allow_nan=False, indent=2)
    # 数値配列のみを対象にし、式の文字列に含まれる文字は変更しない。
    pattern = r'(?m)^( *"values": \[)([\s0-9eE+.,-]*)(\])'
    for match in reversed(list(re.finditer(pattern, contents))):
        compact = "".join(match.group(2).split())
        contents = contents[:match.start(2)] + compact + contents[match.end(2):]
    return contents + "\n"


def main():
    """コマンドライン引数を読み、変換・新規出力を行い、終了コードを返す。"""
    parser = argparse.ArgumentParser(description="Convert one legacy PBVR .tfe file to JSON.")
    parser.add_argument("input", type=Path, help="input .tfe file (UTF-8)")
    parser.add_argument("-o", "--output", type=Path,
                        help="output .json file (default: input path with .json suffix)")
    args = parser.parse_args()
    input_path = args.input
    output_path = args.output if args.output is not None else input_path.with_suffix(".json")
    if input_path.suffix.lower() != ".tfe":
        parser.error("input file must have a .tfe extension")
    if output_path.suffix.lower() != ".json":
        parser.error("output file must have a .json extension")
    try:
        if output_path.exists() or output_path.is_symlink():
            raise TfeError(f"{output_path}: output already exists; refusing to overwrite")
        contents = serialize(convert(input_path))
        # 排他作成により、検証中に出力先が作成された場合も上書きを防ぐ。
        with output_path.open("x", encoding="utf-8", newline="\n") as stream:
            stream.write(contents)
    except (TfeError, OSError, UnicodeError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    print(f"Converted {input_path} -> {output_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
