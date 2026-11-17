#!/usr/bin/env bash
# =============================================================================
# regtest_S-IS-34_missing_default_json.sh
#   S-IS-34: default_old.json / default.json 欠損時の動作確認(サーバ側シェルスクリプト)
#   対応チェックシート: PBVRリグレッションテストチェックシート_v3.X.X.xlsx Server シート ISPBVR 34
#
# 使い方:
#   ./regtest_S-IS-34_missing_default_json.sh [シミュレーション実行ディレクトリ] [MPI ランク数(=アンサンブル数)]
#     $1  既定: このスクリプトのあるディレクトリ
#     $2  既定: 2
#   環境変数 VIS_PARAM_DIR / PARTICLE_DIR / TF_NAME が設定されている場合はそれに従う
#   (未設定時は VIS_PARAM_DIR=./, PARTICLE_DIR=./, TF_NAME=default)。
#   MPIRUN 環境変数で mpirun コマンドを差し替えられる(既定: mpirun)。
#
# 手順:
#   1. $1 に cd し ./RESET.sh を実行
#   2. default.json / default_old.json を _backup_missing_json/ へ退避
#   3. mpirun -n $2 ./run を実行(標準出力・標準エラーを sim_stdout.log に保存)
#   4. 判定(終了コード / 警告行 / history_XXXXX.json / state.txt / 粒子ファイル無し)
#   5. 退避したファイルを戻し ./RESET.sh を再実行
#   6. $1/output-tests/yyyymmdd/ServerEnsembleTest/TestResult_S-IS-34.md に結果を出力
# =============================================================================

# bash 3.2(macOS 標準)でも動作するよう set -u は使わない

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SIM_DIR="${1:-$SCRIPT_DIR}"
NUM_RANKS="${2:-2}"
MPIRUN="${MPIRUN:-mpirun}"
TF_NAME="${TF_NAME:-default}"
ITEM_ID="S-IS-34"

if ! cd "$SIM_DIR"; then
    echo "[$ITEM_ID] FAIL: シミュレーション実行ディレクトリに移動できない: $SIM_DIR" >&2
    exit 2
fi
SIM_DIR="$( pwd )"

# 環境変数(未設定時は ./ と default)
VIS_PARAM_DIR_RAW="${VIS_PARAM_DIR:-./}"
PARTICLE_DIR_RAW="${PARTICLE_DIR:-./}"
VIS_PARAM_DIR_ABS="$( cd "$VIS_PARAM_DIR_RAW" 2>/dev/null && pwd || echo "$VIS_PARAM_DIR_RAW" )"
PARTICLE_DIR_ABS="$( cd "$PARTICLE_DIR_RAW" 2>/dev/null && pwd || echo "$PARTICLE_DIR_RAW" )"

DATE_STAMP="$( date +%Y%m%d )"
REPORT_DIR="$SIM_DIR/output-tests/$DATE_STAMP/ServerEnsembleTest"
REPORT_PATH="$REPORT_DIR/TestResult_${ITEM_ID}.md"
BACKUP_DIR="$SIM_DIR/_backup_missing_json"
SIM_LOG="$SIM_DIR/sim_stdout.log"
RUN_MARKER="$REPORT_DIR/.run_marker"   # 実行前に touch し、実行後に生成されたファイルの一覧に使う

JSON_FILE="${TF_NAME}.json"
JSON_OLD_FILE="${TF_NAME}_old.json"

EXPECTED_LINE_1="[WARN] Failed to load transfer function json."
EXPECTED_LINE_2="[WARN] Files: ${JSON_FILE} and ${JSON_OLD_FILE}"
EXPECTED_LINE_3="[INFO] Set default particle parameters and skip object generation."

mkdir -p "$REPORT_DIR"

# ---- 結果の蓄積 ----
STEPS=()          # "STATUS|説明"
FAILURES=()
OVERALL="PASS"
EXIT_CODE="-"
LINE_FOUND=("NO" "NO" "NO")
HISTORY_SUMMARY=()
OUTPUT_FILES=()
HISTORY_FILES=()

log_step() { # status, message
    STEPS+=("$1|$ITEM_ID $2")
    echo "[$1] $ITEM_ID $2"
}
fail() {
    OVERALL="FAIL"
    FAILURES+=("$1")
    log_step "FAIL" "$1"
}
pass() { log_step "PASS" "$1"; }
skip() { log_step "SKIP" "$1"; }

# ---- 後片付け(途中終了時も必ず実行) ----
CLEANED=0
cleanup() {
    if [ "$CLEANED" -eq 1 ]; then return; fi
    CLEANED=1
    if [ -f "$BACKUP_DIR/$JSON_FILE" ]; then
        mv -f "$BACKUP_DIR/$JSON_FILE" "$VIS_PARAM_DIR_ABS/$JSON_FILE" && pass "退避した $JSON_FILE を元に戻した" || fail "$JSON_FILE を元に戻せない"
    fi
    if [ -f "$BACKUP_DIR/$JSON_OLD_FILE" ]; then
        mv -f "$BACKUP_DIR/$JSON_OLD_FILE" "$VIS_PARAM_DIR_ABS/$JSON_OLD_FILE" && pass "退避した $JSON_OLD_FILE を元に戻した" || fail "$JSON_OLD_FILE を元に戻せない"
    fi
    rmdir "$BACKUP_DIR" 2>/dev/null || true
    if [ -x ./RESET.sh ]; then
        ./RESET.sh >/dev/null 2>&1 && pass "./RESET.sh を再実行した(後片付け)" || fail "後片付けの ./RESET.sh が失敗した"
    fi
}
trap cleanup EXIT

# ---- 1. RESET.sh ----
if [ ! -x ./RESET.sh ]; then
    fail "./RESET.sh が無い(または実行権限が無い): $SIM_DIR"
else
    if ./RESET.sh >/dev/null 2>&1; then
        pass "./RESET.sh を実行した(default_old.json → default.json を復元、history_0* / default_0* / particle_out/* / state.txt を削除)"
    else
        fail "./RESET.sh が失敗した"
    fi
fi

# ---- 2. default.json / default_old.json を退避 ----
mkdir -p "$BACKUP_DIR"
MOVED=0
for f in "$JSON_FILE" "$JSON_OLD_FILE"; do
    if [ -f "$VIS_PARAM_DIR_ABS/$f" ]; then
        if mv -f "$VIS_PARAM_DIR_ABS/$f" "$BACKUP_DIR/$f"; then
            MOVED=$((MOVED + 1))
        else
            fail "$f を退避できない"
        fi
    else
        skip "$VIS_PARAM_DIR_ABS/$f は存在しない(退避不要)"
    fi
done
if [ "$MOVED" -gt 0 ]; then
    pass "$JSON_FILE / $JSON_OLD_FILE を $BACKUP_DIR へ退避した($MOVED ファイル)"
fi
if [ -f "$VIS_PARAM_DIR_ABS/$JSON_FILE" ] || [ -f "$VIS_PARAM_DIR_ABS/$JSON_OLD_FILE" ]; then
    fail "退避後も $JSON_FILE / $JSON_OLD_FILE が残っている"
fi

# ---- 3. mpirun -n $2 ./run ----
if [ ! -x ./run ]; then
    fail "./run が無い(または実行権限が無い)。ビルド済みか確認する"
else
    echo "[$ITEM_ID] 実行: $MPIRUN -n $NUM_RANKS ./run (log: $SIM_LOG)"
    touch "$RUN_MARKER"
    sleep 1
    "$MPIRUN" -n "$NUM_RANKS" ./run >"$SIM_LOG" 2>&1
    EXIT_CODE=$?
    pass "$MPIRUN -n $NUM_RANKS ./run を実行し、標準出力・標準エラーを sim_stdout.log に保存した(終了コード $EXIT_CODE)"
fi

# ---- 4. 判定 ----
# 4-1 終了コード
if [ "$EXIT_CODE" = "0" ]; then
    pass "終了コードが 0(シミュレーションが停止しない)"
else
    fail "終了コードが 0 ではない: $EXIT_CODE"
fi

# 4-2 警告行
if [ -f "$SIM_LOG" ]; then
    i=0
    for expected in "$EXPECTED_LINE_1" "$EXPECTED_LINE_2" "$EXPECTED_LINE_3"; do
        if grep -qF -- "$expected" "$SIM_LOG"; then
            LINE_FOUND[$i]="YES"
            pass "sim_stdout.log に「$expected」が含まれる"
        else
            fail "sim_stdout.log に「$expected」が含まれない"
        fi
        i=$((i + 1))
    done
else
    fail "sim_stdout.log が無い"
fi

# 4-3 history_XXXXX.json
HISTORY_LIST=()
while IFS= read -r f; do
    [ -n "$f" ] && HISTORY_LIST+=("$f")
done < <( ls -1 "$VIS_PARAM_DIR_ABS"/history_[0-9]*.json 2>/dev/null | sort )

if [ "${#HISTORY_LIST[@]}" -eq 0 ]; then
    fail "VIS_PARAM_DIR($VIS_PARAM_DIR_ABS)に history_XXXXX.json が無い"
else
    pass "VIS_PARAM_DIR に history_XXXXX.json が ${#HISTORY_LIST[@]} ファイル存在する"
    if ! command -v python3 >/dev/null 2>&1; then
        fail "python3 が見つからないため history_XXXXX.json を検証できない"
    else
        for h in "${HISTORY_LIST[@]}"; do
            HISTORY_FILES+=("$h")
            # python3 で読み込み、判定と要約(各統計の O1 min / max)を出力する
            PY_OUT="$( python3 - "$h" <<'PYEOF'
import json, sys
path = sys.argv[1]
errors = []
summary = []
try:
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
except Exception as e:
    print("ERROR|json 読み込み失敗: %s" % e)
    sys.exit(0)
if data.get("is_ensemble") is not True:
    errors.append("is_ensemble が true ではない")
stats = data.get("statistics")
if not isinstance(stats, dict):
    errors.append("statistics が無い")
    stats = {}
for key in ("average", "variance", "coefficient_of_variation"):
    s = stats.get(key)
    if not isinstance(s, dict):
        errors.append("statistics.%s が無い" % key)
        continue
    for tf in ("O1", "C1"):
        e = s.get(tf)
        if not isinstance(e, dict):
            errors.append("statistics.%s.%s が無い" % (key, tf))
            continue
        mn = e.get("min")
        mx = e.get("max")
        hist = e.get("histogram")
        if not isinstance(mn, (int, float)) or not isinstance(mx, (int, float)):
            errors.append("statistics.%s.%s の min / max が無い" % (key, tf))
        elif mn > mx:
            errors.append("statistics.%s.%s の min(%s) > max(%s)" % (key, tf, mn, mx))
        if not isinstance(hist, list) or len(hist) != 256:
            errors.append("statistics.%s.%s の histogram の長さが 256 ではない(%s)" % (key, tf, len(hist) if isinstance(hist, list) else "無し"))
        if tf == "O1" and isinstance(mn, (int, float)) and isinstance(mx, (int, float)):
            summary.append("%s O1 min=%s max=%s" % (key, mn, mx))
if errors:
    print("FAIL|" + "; ".join(errors) + "|" + ", ".join(summary))
else:
    print("PASS|ok|" + ", ".join(summary))
PYEOF
)"
            PY_STATUS="${PY_OUT%%|*}"
            PY_REST="${PY_OUT#*|}"
            PY_MSG="${PY_REST%%|*}"
            PY_SUMMARY="${PY_REST#*|}"
            [ "$PY_SUMMARY" = "$PY_REST" ] && PY_SUMMARY=""
            HISTORY_SUMMARY+=("$( basename "$h" ): ${PY_SUMMARY:-（要約なし）}")
            if [ "$PY_STATUS" = "PASS" ]; then
                pass "$( basename "$h" ): is_ensemble == true、statistics.average / variance / coefficient_of_variation の O1 / C1 に min / max / histogram(256)があり min <= max"
            else
                fail "$( basename "$h" ): $PY_MSG"
            fi
        done
    fi
fi

# 4-4 state.txt
if [ -f "$VIS_PARAM_DIR_ABS/state.txt" ]; then
    pass "VIS_PARAM_DIR に state.txt が存在する"
else
    fail "VIS_PARAM_DIR($VIS_PARAM_DIR_ABS)に state.txt が無い"
fi

# 4-5 粒子ファイル(ave_ / var_ / cov_)が無いこと
PARTICLE_FOUND=()
for d in "$PARTICLE_DIR_ABS" "$SIM_DIR/particle_out"; do
    [ -d "$d" ] || continue
    while IFS= read -r f; do
        [ -n "$f" ] && PARTICLE_FOUND+=("$f")
    done < <( ls -1 "$d"/ave_* "$d"/var_* "$d"/cov_* 2>/dev/null )
done
if [ "${#PARTICLE_FOUND[@]}" -eq 0 ]; then
    pass "PARTICLE_DIR($PARTICLE_DIR_ABS)に ave_ / var_ / cov_ で始まる粒子ファイルが存在しない(伝達関数が無いため粒子は生成されない)"
else
    fail "PARTICLE_DIR に ave_ / var_ / cov_ で始まる粒子ファイルが存在する(${#PARTICLE_FOUND[@]} ファイル): ${PARTICLE_FOUND[0]} ..."
fi

# ---- 出力ファイル一覧とコピー(RESET.sh の前に保存する) ----
# 実行後に生成・更新されたファイル(VIS_PARAM_DIR / PARTICLE_DIR / particle_out 直下)
if [ -f "$RUN_MARKER" ]; then
    while IFS= read -r f; do
        [ -n "$f" ] && OUTPUT_FILES+=("$f")
    done < <( for d in "$VIS_PARAM_DIR_ABS" "$PARTICLE_DIR_ABS" "$SIM_DIR/particle_out"; do
                  [ -d "$d" ] && find "$d" -maxdepth 1 -type f -newer "$RUN_MARKER" 2>/dev/null
              done | grep -v "/sim_stdout.log$" | sort -u )
    rm -f "$RUN_MARKER"
fi

[ -f "$SIM_LOG" ] && cp -f "$SIM_LOG" "$REPORT_DIR/sim_stdout.log"
for h in "${HISTORY_FILES[@]}"; do
    cp -f "$h" "$REPORT_DIR/" 2>/dev/null || true
done

# ---- 5. 後片付け ----
cleanup

# ---- 6. Markdown レポート ----
{
    echo "# ServerEnsembleTest Part B: ${ITEM_ID} default_old.json / default.json 欠損時の動作確認"
    echo
    echo "- 対応項目: PBVRリグレッションテストチェックシート Server シート ISPBVR 34(追加項目仕様_アンサンブル ${ITEM_ID})"
    echo "- 結果: ${OVERALL}"
    echo "- 実行日: $( date '+%Y-%m-%d %H:%M:%S' )"
    echo "- 実行ディレクトリ: \`${SIM_DIR}\`"
    echo "- MPI ランク数(アンサンブル数): ${NUM_RANKS}(コマンド: \`${MPIRUN} -n ${NUM_RANKS} ./run\`)"
    echo "- 環境変数: VIS_PARAM_DIR=\`${VIS_PARAM_DIR_RAW}\`(実体: \`${VIS_PARAM_DIR_ABS}\`), PARTICLE_DIR=\`${PARTICLE_DIR_RAW}\`(実体: \`${PARTICLE_DIR_ABS}\`), TF_NAME=\`${TF_NAME}\`"
    echo "- 退避したファイル: \`${JSON_FILE}\` / \`${JSON_OLD_FILE}\`(→ \`_backup_missing_json/\`、終了時に復元)"
    echo "- 終了コード: ${EXIT_CODE}"
    echo "- 警告行の有無:"
    echo "    - \`${EXPECTED_LINE_1}\`: ${LINE_FOUND[0]}"
    echo "    - \`${EXPECTED_LINE_2}\`: ${LINE_FOUND[1]}"
    echo "    - \`${EXPECTED_LINE_3}\`: ${LINE_FOUND[2]}"
    echo "- sim_stdout.log と history_XXXXX.json のコピー: \`${REPORT_DIR}/\`"
    echo
    echo "## 実施手順"
    echo
    for s in "${STEPS[@]}"; do
        echo "- ${s%%|*}: ${s#*|}"
    done
    echo
    echo "## history_XXXXX.json の要約(各統計の O1 min / max)"
    echo
    if [ "${#HISTORY_SUMMARY[@]}" -eq 0 ]; then
        echo "- (history_XXXXX.json なし)"
    else
        for s in "${HISTORY_SUMMARY[@]}"; do
            echo "- ${s}"
        done
    fi
    echo
    echo "## 出力ファイル一覧(RESET.sh 実行前)"
    echo
    if [ "${#OUTPUT_FILES[@]}" -eq 0 ]; then
        echo "- (なし)"
    else
        for f in "${OUTPUT_FILES[@]}"; do
            echo "- \`${f}\`"
        done
    fi
    echo
    echo "## 判定"
    echo
    echo "- ${OVERALL}"
    if [ "${#FAILURES[@]}" -gt 0 ]; then
        echo
        echo "### 失敗内容"
        echo
        for f in "${FAILURES[@]}"; do
            echo "- ${f}"
        done
    fi
    echo
    echo "## 備考"
    echo
    echo "- 同梱の main.cpp は時系列ループが 1 回のため、通常は history_00000.json のみ出力される。ステップ数を増やしてビルドしている場合は全ステップ分を確認する。"
    echo "- 既存 Server シート ISPBVR 項目4(default.tf が無くても…)のアンサンブル版。警告文は InSituLib/unstruct/kvs_wrapper.cpp の SetParticleParameter に依存する。"
} > "$REPORT_PATH"

echo "[$ITEM_ID] 結果: $OVERALL"
echo "[$ITEM_ID] レポート: $REPORT_PATH"

if [ "$OVERALL" = "PASS" ]; then
    exit 0
fi
exit 1
