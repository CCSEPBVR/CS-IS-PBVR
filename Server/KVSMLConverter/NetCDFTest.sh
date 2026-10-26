#!/usr/bin/env bash

# VTK NetCDFサンプルデータを単一ファイルとファイル系列として変換する。

SCRIPT_PATH="$0"
case "${SCRIPT_PATH}" in
    */*) SCRIPT_DIRECTORY="${SCRIPT_PATH%/*}" ;;
    *) SCRIPT_DIRECTORY="." ;;
esac
SCRIPT_DIRECTORY="$(cd -- "${SCRIPT_DIRECTORY}" && pwd -P)" || exit 2
SCRIPT_NAME="${SCRIPT_PATH##*/}"
DEFAULT_CONVERTER="${SCRIPT_DIRECTORY}/Example/Release/kvsml-converter"
DEFAULT_DATA_DIRECTORY="${SCRIPT_DIRECTORY}/Example/Input/VTKSampleData/data"
DEFAULT_OUTPUT_PARENT="${SCRIPT_DIRECTORY}/Example/Output"
DEFAULT_VTK_LIBRARY_DIRECTORY="/Users/sakamoto/Work/VTK-9.3.1/install/lib"

CONVERTER="${DEFAULT_CONVERTER}"
DATA_DIRECTORY="${DEFAULT_DATA_DIRECTORY}"
OUTPUT_ROOT=""
REQUESTED_HELP=false

# 形式名|入力モード|主入力|期待KVSML数|補助入力|PFI/PFL必須
CASES=(
    'CF|single|vtkNetCDFCFReader_0.nc|1||true'
    'CF|series|vtkNetCDFCFReader_*.nc|3||true'
    'CAM|single|vtkNetCDFCAMReader_point_0.nc|1|vtkNetCDFCAMReader_connectivity.nc|true'
    'CAM|series|vtkNetCDFCAMReader_point_*.nc|3|vtkNetCDFCAMReader_connectivity.nc|true'
    'MPAS|single|vtkMPASReader_0.nc|1||true'
    'MPAS|series|vtkMPASReader_*.nc|3||true'
    'UGRID|single|vtkNetCDFUGRIDReader_0.nc|1||false'
    'UGRID|series|vtkNetCDFUGRIDReader_*.nc|3||false'
    'SLAC|single|vtkSLACReader_volume.ncdf|1|vtkSLACReader_mode_0.ncdf|true'
    'SLAC|series|vtkSLACReader_volume.ncdf|3|vtkSLACReader_mode_*.ncdf|true'
)

SUCCESSFUL_COUNT=0
TEMPORARY_DIRECTORY=""
RESULTS_DIRECTORY=""

# 関数: print_line
# 役割: Bashとzshの両方で1行を標準出力へ表示する。
print_line()
{
    printf '%s\n' "$1"
}

# 関数: absolute_path
# 役割: 存在しない末尾部分を含むパスを、既存の親ディレクトリを基準に絶対化する。
# 引数: 絶対化するパス
# 出力: 絶対パス
absolute_path()
{
    local input_path="$1"
    local path="${input_path}"
    local suffix=""
    local parent
    local current_directory

    current_directory="$(pwd -P)" || return 1
    case "${path}" in
        /*) ;;
        *) path="${current_directory}/${path}" ;;
    esac

    while [[ ! -d "${path}" ]]; do
        parent="${path%/*}"
        if [[ -z "${parent}" ]]; then
            parent="/"
        fi
        if [[ "${parent}" == "${path}" ]]; then
            return 1
        fi
        suffix="/${path##*/}${suffix}"
        path="${parent}"
    done

    parent="$(cd -- "${path}" && pwd -P)" || return 1
    if [[ "${parent}" == "/" ]]; then
        print_line "/${suffix#/}"
    else
        print_line "${parent}${suffix}"
    fi
}

# 関数: quote_for_display
# 役割: コマンド表示用に、パスをBashとzshで共通のシングルクォート形式へ変換する。
# 引数: クォートする文字列
quote_for_display()
{
    local value="$1"

    printf '%s' "'"
    printf '%s' "${value}" | sed "s/'/'\\\\''/g"
    printf '%s' "'"
}

# 関数: print_usage
# 役割: コマンドラインの使用方法を標準出力へ表示する。
# 引数: なし
print_usage()
{
    print_line "Usage: ${SCRIPT_NAME} [--converter PATH] [--data-directory PATH] [--output-root PATH]"
}

# 関数: parse_arguments
# 役割: コマンドライン引数を解析し、指定値を設定する。
# 結果: ヘルプ要求は0、引数エラーは2を返す。
parse_arguments()
{
    while (( $# > 0 )); do
        case "$1" in
            --converter)
                if (( $# < 2 )); then
                    print_line "--converter requires a path" >&2
                    return 2
                fi
                CONVERTER="$2"
                shift 2
                ;;
            --data-directory)
                if (( $# < 2 )); then
                    print_line "--data-directory requires a path" >&2
                    return 2
                fi
                DATA_DIRECTORY="$2"
                shift 2
                ;;
            --output-root)
                if (( $# < 2 )); then
                    print_line "--output-root requires a path" >&2
                    return 2
                fi
                OUTPUT_ROOT="$2"
                shift 2
                ;;
            -h|--help)
                REQUESTED_HELP=true
                return 0
                ;;
            *)
                print_line "Unknown argument: $1" >&2
                print_usage >&2
                return 2
                ;;
        esac
    done

    return 0
}

# 関数: resolve_paths
# 役割: 入出力パスを絶対パスへ解決し、出力先の重複を確認する。
# 結果: 出力先が既存の場合を含む準備エラーは2を返す。
resolve_paths()
{
    CONVERTER="$(absolute_path "${CONVERTER}")" || return 2
    DATA_DIRECTORY="$(absolute_path "${DATA_DIRECTORY}")" || return 2
    if [[ -z "${OUTPUT_ROOT}" ]]; then
        TIMESTAMP="$(date '+%Y%m%d%H%M%S')"
        OUTPUT_ROOT="${DEFAULT_OUTPUT_PARENT}/VTKSampleData${TIMESTAMP}"
    fi
    OUTPUT_ROOT="$(absolute_path "${OUTPUT_ROOT}")" || return 2

    if [[ -e "${OUTPUT_ROOT}" ]]; then
        print_line "The output root already exists: ${OUTPUT_ROOT}" >&2
        return 2
    fi

    return 0
}

# 関数: prepare_environment
# 役割: VTK実行時ライブラリと一時的なケース結果保存先を準備する。
# 結果: 一時ディレクトリを作成できない場合は2を返す。
prepare_environment()
{
    # シェルの設定ファイルに依存せず、コンバーターへ直接継承させる。
    VTK_LIBRARY_DIRECTORY="${VTK_LIB_PATH:-${DEFAULT_VTK_LIBRARY_DIRECTORY}}"
    export DYLD_LIBRARY_PATH="${VTK_LIBRARY_DIRECTORY}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}"

    TEMPORARY_DIRECTORY="$(mktemp -d "${TMPDIR:-/tmp}/kvsml-converter-test.XXXXXX")" || return 2
    trap 'rm -rf -- "${TEMPORARY_DIRECTORY}"' EXIT INT TERM

    RESULTS_DIRECTORY="${TEMPORARY_DIRECTORY}/cases"
    mkdir -p "${RESULTS_DIRECTORY}" || return 2
    return 0
}

# 関数: prepare_test_directories
# 役割: 単一ファイル用と系列用の出力ディレクトリを作成する。
# 結果: ディレクトリを作成できない場合は2を返す。
prepare_test_directories()
{
    mkdir -p "${OUTPUT_ROOT}/single" "${OUTPUT_ROOT}/series" || return 2
    return 0
}

# 関数: save_output_file_list
# 役割: 指定した出力ディレクトリ直下のファイル一覧をソートして保存する。
# 引数: 出力ディレクトリ、一覧の保存先
save_output_file_list()
{
    local output_directory="$1"
    local list_file="$2"

    find "${output_directory}" -maxdepth 1 -type f -print | LC_ALL=C sort > "${list_file}"
}

# 関数: run_converter
# 役割: 補助入力を必要に応じて標準入力へ渡し、コンバーターの出力と終了コードを保存する。
# 引数: 主入力、出力先、補助入力、標準出力先、標準エラー先、終了コード保存先
run_converter()
{
    local primary_path="$1"
    local output_directory="$2"
    local auxiliary_path="$3"
    local stdout_file="$4"
    local stderr_file="$5"
    local return_code_file="$6"
    local return_code

    if [[ -n "${auxiliary_path}" ]]; then
        print_line "${auxiliary_path}" |
            "${CONVERTER}" "${primary_path}" "${output_directory}" \
                > "${stdout_file}" 2> "${stderr_file}"
        return_code=$?
    else
        "${CONVERTER}" "${primary_path}" "${output_directory}" \
            > "${stdout_file}" 2> "${stderr_file}"
        return_code=$?
    fi

    print_line "${return_code}" > "${return_code_file}"
    return 0
}

# 関数: collect_generated_files
# 役割: 変換前後のファイル一覧を比較し、新しく生成されたファイルを特定する。
# 引数: 変換前一覧、変換後一覧、生成ファイル一覧の保存先
collect_generated_files()
{
    local before_file="$1"
    local after_file="$2"
    local generated_file="$3"

    comm -13 "${before_file}" "${after_file}" > "${generated_file}"
}

# 関数: collect_diagnostics
# 役割: 標準エラーと標準出力からエラー相当の診断を抽出し、重複を除いて保存する。
# 引数: 標準出力、標準エラー、診断の保存先
collect_diagnostics()
{
    local stdout_file="$1"
    local stderr_file="$2"
    local diagnostics_file="$3"

    {
        sed -n -E '/(error|failed|failure|cannot|unsupported|empty|differs|invalid)/Ip' "${stderr_file}"
        sed -n -E '/(error|failed|failure|cannot|unsupported|empty|differs|invalid)/Ip' "${stdout_file}"
    } | awk '!seen[$0]++' > "${diagnostics_file}"
}

# 関数: validate_generated_files
# 役割: KVSML数、PFI/PFLの生成状況、コンバーター終了コードを共通条件として検証する。
# 引数: 生成ファイル一覧、期待KVSML数、PFI/PFL必須値、終了コード、診断先、結果保存先
validate_generated_files()
{
    local generated_file="$1"
    local expected_count="$2"
    local expects_metadata="$3"
    local return_code="$4"
    local diagnostics_file="$5"
    local result_directory="$6"
    local generated_path
    local kvsml_count=0
    local has_pfi=false
    local has_pfl=false

    while IFS= read -r generated_path; do
        case "${generated_path}" in
            *.kvsml) (( kvsml_count += 1 )) ;;
            *.pfi) has_pfi=true ;;
            *.pfl) has_pfl=true ;;
        esac
    done < "${generated_file}"

    print_line "${kvsml_count}" > "${result_directory}/kvsml-count"
    print_line "${has_pfi}" > "${result_directory}/has-pfi"
    print_line "${has_pfl}" > "${result_directory}/has-pfl"

    if (( kvsml_count != expected_count )); then
        print_line "Expected ${expected_count} new KVSML file(s), but found ${kvsml_count}." >> "${diagnostics_file}"
    fi
    if [[ "${expects_metadata}" == true && ( "${has_pfi}" != true || "${has_pfl}" != true ) ]]; then
        print_line "Expected new PFI and PFL metadata files were not both found." >> "${diagnostics_file}"
    fi
    if [[ "${expects_metadata}" == false && ( "${has_pfi}" == true || "${has_pfl}" == true ) ]]; then
        print_line "Polygon conversion unexpectedly generated volume PFI/PFL metadata." >> "${diagnostics_file}"
    fi
    if (( return_code != 0 )); then
        print_line "The converter exited with status ${return_code}." >> "${diagnostics_file}"
    fi
}

# 関数: validate_case_specific_result
# 役割: CAMの標準入力プロンプト回数と系列のタイムステップを形式固有条件として検証する。
# 引数: 形式名、入力モード、補助入力名、標準出力、生成ファイル一覧、診断先
validate_case_specific_result()
{
    local case_name="$1"
    local case_mode="$2"
    local auxiliary_input="$3"
    local stdout_file="$4"
    local generated_file="$5"
    local diagnostics_file="$6"
    local prompt_count
    local step
    local step_token

    if [[ "${case_name}" == "CAM" && -n "${auxiliary_input}" ]]; then
        prompt_count="$(grep -F -c -- 'Enter the CAM connectivity file path:' "${stdout_file}")"
        if (( prompt_count != 1 )); then
            print_line "Expected the CAM connectivity prompt exactly once, but found it ${prompt_count} time(s)." >> "${diagnostics_file}"
        fi
    fi

    if [[ "${case_name}" == "CAM" && "${case_mode}" == "series" ]]; then
        for (( step = 0; step <= 2; step += 1 )); do
            step_token="$(printf '_%05d' "${step}")"
            if ! grep -E '\.kvsml$' "${generated_file}" | grep -F -q -- "${step_token}"; then
                print_line "CAM series output is missing PBVR time step ${step}." >> "${diagnostics_file}"
            fi
        done
    fi
}

# 関数: determine_case_result
# 役割: 診断の有無からケースの成否を確定し、コンソール表示と成功件数を更新する。
# 引数: 形式名、入力モード、診断先、成否の保存先
determine_case_result()
{
    local case_name="$1"
    local case_mode="$2"
    local diagnostics_file="$3"
    local result_file="$4"
    local case_result

    if [[ ! -s "${diagnostics_file}" ]]; then
        case_result="Success"
        print_line "[SUCCESS] ${case_name} / ${case_mode}"
        (( SUCCESSFUL_COUNT += 1 ))
    else
        case_result="Failure"
        print_line "[FAILURE] ${case_name} / ${case_mode}"
    fi

    print_line "${case_result}" > "${result_file}"
}

# 関数: run_test_case
# 役割: 1ケースについて、実行、結果収集、共通検証、形式固有検証、成否確定を順に行う。
# 引数: ケース番号、ケース定義
run_test_case()
{
    local case_index="$1"
    local case_spec="$2"
    local case_name
    local case_mode
    local primary_input
    local expected_count
    local auxiliary_input
    local expects_metadata
    local result_directory
    local output_directory
    local primary_path
    local auxiliary_path=""
    local command_text
    local before_file
    local after_file
    local generated_file
    local diagnostics_file
    local stdout_file
    local stderr_file
    local return_code_file
    local return_code

    IFS='|' read -r case_name case_mode primary_input expected_count auxiliary_input expects_metadata <<< "${case_spec}"

    # 1. ケース結果の保存先を準備し、レポート生成に必要なケース情報を保存する。
    result_directory="${RESULTS_DIRECTORY}/${case_index}"
    mkdir -p "${result_directory}" || return 2
    output_directory="${OUTPUT_ROOT}/${case_mode}"
    primary_path="${DATA_DIRECTORY}/${primary_input}"
    if [[ -n "${auxiliary_input}" ]]; then
        auxiliary_path="${DATA_DIRECTORY}/${auxiliary_input}"
    fi
    command_text="$(quote_for_display "${CONVERTER}") $(quote_for_display "${primary_path}") $(quote_for_display "${output_directory}")"

    print_line "${case_spec}" > "${result_directory}/case-spec"
    print_line "${case_name}" > "${result_directory}/case-name"
    print_line "${case_mode}" > "${result_directory}/case-mode"
    print_line "${primary_input}" > "${result_directory}/primary-input"
    print_line "${primary_path}" > "${result_directory}/primary-path"
    print_line "${expected_count}" > "${result_directory}/expected-kvsml-count"
    print_line "${auxiliary_input}" > "${result_directory}/auxiliary-input"
    print_line "${auxiliary_path}" > "${result_directory}/auxiliary-path"
    print_line "${expects_metadata}" > "${result_directory}/expects-pfi-pfl"
    print_line "${command_text}" > "${result_directory}/command"
    print_line "${output_directory}" > "${result_directory}/output-directory"

    stdout_file="${result_directory}/stdout"
    stderr_file="${result_directory}/stderr"
    return_code_file="${result_directory}/return-code"
    before_file="${result_directory}/before"
    after_file="${result_directory}/after"
    generated_file="${result_directory}/generated"
    diagnostics_file="${result_directory}/diagnostics"

    # 2. 変換前の出力ファイル一覧を保存する。
    save_output_file_list "${output_directory}" "${before_file}"

    # 3-4. コンバーターを実行し、標準出力、標準エラー、終了コードを保存する。
    run_converter "${primary_path}" "${output_directory}" "${auxiliary_path}" \
        "${stdout_file}" "${stderr_file}" "${return_code_file}"
    return_code="$(<"${return_code_file}")"

    # 5. 診断抽出とレポート記載に先立ち、端末カラー制御文字を除去する。
    /usr/bin/perl -i -pe 's/\e\[[0-?]*[ -\/]*[@-~]//g' "${stdout_file}" "${stderr_file}"

    # 6-7. 変換後の一覧を保存し、新しく生成されたファイルを特定する。
    save_output_file_list "${output_directory}" "${after_file}"
    collect_generated_files "${before_file}" "${after_file}" "${generated_file}"

    # 8. コンバーターの両ストリームから診断を抽出し、重複を除く。
    collect_diagnostics "${stdout_file}" "${stderr_file}" "${diagnostics_file}"

    # 9. 全ケース共通の条件を検証する。
    validate_generated_files "${generated_file}" "${expected_count}" "${expects_metadata}" \
        "${return_code}" "${diagnostics_file}" "${result_directory}"

    # 10. 形式と入力モードに固有の条件を検証する。
    validate_case_specific_result "${case_name}" "${case_mode}" "${auxiliary_input}" \
        "${stdout_file}" "${generated_file}" "${diagnostics_file}"

    # 11-12. ケースの成否を確定し、コンソールへ結果を表示する。
    determine_case_result "${case_name}" "${case_mode}" "${diagnostics_file}" \
        "${result_directory}/result"
    return 0
}

# 関数: run_all_test_cases
# 役割: 定義済みの全ケースを配列順に実行し、成功件数を集計する。
# 結果: 全ケース完了時は0、ケース結果保存先の準備エラーは2を返す。
run_all_test_cases()
{
    local case_index=0
    local case_spec

    SUCCESSFUL_COUNT=0
    for case_spec in "${CASES[@]}"; do
        (( case_index += 1 ))
        run_test_case "${case_index}" "${case_spec}" || return 2
    done

    return 0
}

# 関数: write_report_header
# 役割: レポートのタイトルと実行条件、全体の成功件数を記述する。
# 出力: 標準出力（write_reportからReport.mdへリダイレクトされる）。
write_report_header()
{
    print_line "# KVSMLConverter VTK Sample Data Conversion Report"
    print_line ""
    print_line "- Converter: \`${CONVERTER}\`"
    print_line "- Input directory: \`${DATA_DIRECTORY}\`"
    print_line "- Single-file input: timestep 0"
    print_line '- Series input: literal `*` wildcard passed to the converter'
    print_line "- Assumption: one timestep and one subvolume per input file"
    print_line "- Result: ${SUCCESSFUL_COUNT}/${#CASES[@]} cases succeeded"
    print_line ""
}

# 関数: write_report_summary
# 役割: 一時保存したケース結果を実行順に読み込み、サマリーテーブルを記述する。
# 出力: 標準出力（write_reportからReport.mdへリダイレクトされる）。
write_report_summary()
{
    local case_index
    local result_directory
    local case_name
    local case_mode
    local case_result
    local return_code
    local kvsml_count

    print_line "## Summary"
    print_line ""
    print_line "| Format | Input mode | Result | Exit status | New KVSML files |"
    print_line "| --- | --- | --- | ---: | ---: |"

    for (( case_index = 1; case_index <= ${#CASES[@]}; case_index += 1 )); do
        result_directory="${RESULTS_DIRECTORY}/${case_index}"
        case_name="$(<"${result_directory}/case-name")"
        case_mode="$(<"${result_directory}/case-mode")"
        case_result="$(<"${result_directory}/result")"
        return_code="$(<"${result_directory}/return-code")"
        kvsml_count="$(<"${result_directory}/kvsml-count")"
        print_line "| ${case_name} | ${case_mode} | ${case_result} | ${return_code} | ${kvsml_count} |"
    done
}

# 関数: write_report_case_details
# 役割: 各ケースの詳細、生成ファイル、失敗時の診断と標準ストリームを記述する。
# 出力: 標準出力（write_reportからReport.mdへリダイレクトされる）。
write_report_case_details()
{
    local case_index
    local result_directory
    local case_name
    local case_mode
    local case_result
    local expected_count
    local command_text
    local auxiliary_path
    local generated_file
    local generated_path
    local diagnostics_file
    local diagnostic
    local stdout_file
    local stderr_file

    for (( case_index = 1; case_index <= ${#CASES[@]}; case_index += 1 )); do
        result_directory="${RESULTS_DIRECTORY}/${case_index}"
        case_name="$(<"${result_directory}/case-name")"
        case_mode="$(<"${result_directory}/case-mode")"
        case_result="$(<"${result_directory}/result")"
        expected_count="$(<"${result_directory}/expected-kvsml-count")"
        command_text="$(<"${result_directory}/command")"
        auxiliary_path="$(<"${result_directory}/auxiliary-path")"
        generated_file="${result_directory}/generated"
        diagnostics_file="${result_directory}/diagnostics"
        stdout_file="${result_directory}/stdout"
        stderr_file="${result_directory}/stderr"

        print_line ""
        print_line "## ${case_name} / ${case_mode}"
        print_line ""
        if [[ "${case_result}" == "Success" ]]; then
            print_line "Success: generated ${expected_count} expected KVSML file(s)."
        else
            print_line "Failure diagnostics:"
            print_line ""
            while IFS= read -r diagnostic; do
                print_line "- ${diagnostic}"
            done < "${diagnostics_file}"
        fi

        print_line ""
        print_line "Command:"
        print_line ""
        print_line '```console'
        print_line "${command_text}"
        print_line '```'

        if [[ -n "${auxiliary_path}" ]]; then
            print_line ""
            print_line "Auxiliary input sent on standard input: \`${auxiliary_path}\`"
        fi

        print_line ""
        print_line "Generated or updated files:"
        print_line ""
        if [[ -s "${generated_file}" ]]; then
            while IFS= read -r generated_path; do
                print_line "- \`${generated_path##*/}\`"
            done < "${generated_file}"
        else
            print_line "- None"
        fi

        if [[ "${case_result}" == "Failure" ]]; then
            print_line ""
            print_line "Standard output:"
            print_line ""
            print_line '```text'
            if [[ -s "${stdout_file}" ]]; then
                command cat "${stdout_file}"
            else
                print_line "(no output)"
            fi
            print_line '```'
            print_line ""
            print_line "Standard error:"
            print_line ""
            print_line '```text'
            if [[ -s "${stderr_file}" ]]; then
                command cat "${stderr_file}"
            else
                print_line "(no output)"
            fi
            print_line '```'
        fi
    done
}

# 関数: write_report
# 役割: 全ケースの一時保存結果から、出力先のReport.mdを一度だけ生成する。
# 前提: 全ケースの実行と結果保存が完了していること。
write_report()
{
    REPORT_PATH="${OUTPUT_ROOT}/Report.md"
    {
        write_report_header
        write_report_summary
        write_report_case_details
    } > "${REPORT_PATH}"
}

# 関数: determine_exit_status
# 役割: 全ケース成功時は0、失敗ケースがある場合は1を返す。
# 引数: なし（SUCCESSFUL_COUNTとCASESを参照する）。
determine_exit_status()
{
    if (( SUCCESSFUL_COUNT == ${#CASES[@]} )); then
        return 0
    fi
    return 1
}

# 関数: main
# 役割: 引数解析から全ケース実行、レポート生成、終了ステータス決定までを順に統括する。
# 引数: スクリプトへ渡されたコマンドライン引数
main()
{
    local parse_result

    # 1. コマンドライン引数を解析する。
    parse_arguments "$@"
    parse_result=$?
    if (( parse_result != 0 )); then
        return "${parse_result}"
    fi
    if [[ "${REQUESTED_HELP}" == true ]]; then
        print_usage
        return 0
    fi

    # 2. 入出力パスを確定する。
    resolve_paths || return 2

    # 3. 実行環境と一時的な結果保存先を準備する。
    prepare_environment || return 2
    prepare_test_directories || return 2

    # 4. 全ケースを順次実行し、結果を一時ディレクトリへ保存する。
    run_all_test_cases || return 2

    # 5. 全ケース完了後にだけMarkdownレポートを生成する。
    write_report

    # 6. 成功件数に基づいてプロセスの終了ステータスを決定する。
    print_line "Report: ${REPORT_PATH}"
    determine_exit_status
}

main "$@"
