# PBVR MPI + OpenMP ベンチマーク手順書

この `Server/BENCHMARK` ディレクトリには、PBVR / 並列可視化コードについて、逐次版と MPI + OpenMP ハイブリッド並列版を比較するための補助スクリプトと調査レポートを置いています。

Codex を実行しているローカルPCでは、大規模な性能測定は行いません。ローカルでは、静的解析、小規模なスクリプト確認、PBSジョブ生成、既存タイマーCSVの集計、レポート生成だけを行います。実際の性能測定はスパコン上で実行してください。

## 前提環境

添付の SGI 利用マニュアルを優先し、PBSジョブは以下の環境を前提に生成します。

- OS: Linux
- Scheduler: PBS Professional
- Compiler: Intel oneAPI
- MPI: HPE MPT
- Module system: Environment Modules
- CUDA module: `cuda/11.4`
- GNU module: `gnu/cur`
- Intel module: `intel/2023.2.1`
- MPI module: `mpt/2.23-ga`
- MPI実行: `mpirun -n {mpiprocs} omplace`
- MPI + OpenMP 実行: `mpirun -n {mpiprocs} omplace`
- OpenMPスレッド数: `OMP_NUM_THREADS`
- Intel OpenMP affinity: `KMP_AFFINITY=disabled`

## 追加ファイル一覧

| ファイル | 役割 |
| --- | --- |
| `config.sh` | ベンチマーク全体の設定ファイルです。queue、walltime、実行バイナリ、module名などを管理します。 |
| `build.sh` | スパコン上で module を読み込み、PBVR と対象Exampleをビルドします。 |
| `run_local_tests.sh` | ローカルPCで可能な範囲の確認を行います。PBS投入や大規模実行はしません。 |
| `generate_benchmark_cases.py` | `cases.csv` を生成します。正しさ確認、強スケーリング、弱スケーリング、MPI x OpenMP構成探索のケースを作ります。 |
| `generate_pbs_jobs.py` | `cases.csv` から PBSジョブスクリプトを生成します。 |
| `submit_all.sh` | 生成済みPBSジョブをすべて投入します。 |
| `submit_correctness.sh` | 正しさ確認用ジョブだけを投入します。 |
| `submit_strong_scaling.sh` | 強スケーリング用ジョブだけを投入します。 |
| `submit_weak_scaling.sh` | 弱スケーリング用ジョブだけを投入します。 |
| `submit_mpi_omp_sweep.sh` | MPI x OpenMP構成探索ジョブだけを投入します。 |
| `parse_timing.py` | ログや `ensemble_timer_summary.csv` からタイマー情報を抽出します。 |
| `summarize_results.py` | speedup、efficiency、区間寄与率、max/avg、ボトルネック候補を集計します。 |
| `compare_outputs.py` | 逐次版と並列版の出力を比較します。数値テキストは許容誤差つきで比較します。 |
| `make_report.py` | 集計CSVからMarkdown形式のベンチマークレポートを生成します。 |

## 生成されるファイル

| ファイル | 内容 |
| --- | --- |
| `cases.csv` | ベンチマークケース一覧です。 |
| `pbs_jobs/<phase>/*.pbs` | PBSジョブスクリプトです。 |
| `benchmark_results/<case>/<case>.log` | 各ケースの実行ログです。 |
| `benchmark_analysis/timing_raw.csv` | 抽出したタイマー生データです。 |
| `benchmark_analysis/timing_summary.csv` | ケース・区間ごとの min / avg / max / std / max_over_avg です。 |
| `benchmark_analysis/scaling_summary.csv` | speedup と parallel efficiency の集計です。 |
| `benchmark_analysis/section_summary.csv` | 区間ごとの serial 対 opt 比較です。 |
| `benchmark_analysis/bottleneck_candidates.csv` | 自動判定されたボトルネック候補です。 |
| `benchmark_analysis/BENCHMARK_REPORT.md` | 人が読むためのベンチマーク結果レポートです。 |

## 設定方法

ベンチマーク用ファイルは `Server/BENCHMARK` にまとまっています。まず `Server/BENCHMARK/config.sh` を確認してください。

主な設定項目は以下です。

```sh
QUEUE=sc16
PROJECT=job
WALLTIME=00:30:00
NCPUS_PER_NODE=40
PBVR_WEAK_BASE_ENS=4
MODULE_CUDA=cuda/11.4
MODULE_GNU=gnu/cur
MODULE_INTEL=intel/2023.2.1
MODULE_MPI=mpt/2.23-ga
MPI_RUNNER=mpirun
PLACEMENT_CMD=omplace
EXECUTABLE=Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/run
WEAK_EXECUTABLE=Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_4eweak_scale/run
STRONG_EXECUTABLES="Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi2/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi8/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi16/run"
INPUT_ARGS=''
OUTPUT_ROOT=benchmark_results
JOB_ROOT=pbs_jobs
```

実行バイナリや入力引数が異なる場合は、`EXECUTABLE` と `INPUT_ARGS` を変更してください。

例:

```sh
export EXECUTABLE=Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/run
export WEAK_EXECUTABLE=Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_4eweak_scale/run
export STRONG_EXECUTABLES="Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi2/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi8/run Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi16/run"
export INPUT_ARGS="default.json"
```


## 生成される主なケース

今回の対象コードはアンサンブルシミュレーション前提で、1 MPIプロセスでは強制中断されます。そのため、`generate_benchmark_cases.py` は `mpiprocs=1` のケースを生成しません。

正しさ確認の基本ケースは以下です。

- `correctness_mpi_2x1`
- `correctness_mpi_2x2`
- `correctness_hybrid_4x4`

強スケーリングの基本ケースは以下です。

- `strong_4x1`
- `strong_8x1`
- `strong_16x1`
- `strong_32x1`
- `strong_64x1`

強スケーリングではアンサンブル数を4に固定するため、MPI数に応じて以下のExampleを使い分けます。

- `strong_4x1`: `ens_Hydrogen_unstruct`
- `strong_8x1`: `ens_Hydrogen_unstruct_mpi2`
- `strong_16x1`: `ens_Hydrogen_unstruct_mpi4`
- `strong_32x1`: `ens_Hydrogen_unstruct_mpi8`
- `strong_64x1`: `ens_Hydrogen_unstruct_mpi16`

弱スケーリングケースは総MPIプロセス数が `10,20,40,80` になるように生成します。1ノード40コア想定では、`weak_80` は `select=2:ncpus=40:mpiprocs=40:ompthreads=1` になり、`mpirun -n 80` で実行します。`PBVR_WEAK_BASE_ENS=4` の場合、10 MPIだけは4の倍数ではないため分布パターンの繰り返しが完全周期になりません。完全周期で比較したい場合は `PBVR_WEAK_BASE_ENS=5` など、10を割り切れる値にしてください。MPI x OpenMP構成探索では、`NCPUS_PER_NODE` の範囲内で `mpiprocs >= 2` の組み合わせだけを生成します。1ノード40コア想定では以下です。

- `sweep_40x1`
- `sweep_20x2`
- `sweep_10x4`
- `sweep_8x5`
- `sweep_5x8`
- `sweep_4x10`
- `sweep_2x20`


## 弱スケーリング用Hydrogenサンプル

強スケーリング、正しさ確認、MPI x OpenMP構成探索では従来の `ens_Hydrogen_unstruct` を使います。弱スケーリング評価だけ、MPI数を増やしても物理値分布が大きく変化しないように、以下の新規Exampleを使います。

```text
Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_4eweak_scale/run
```

このパスは `WEAK_EXECUTABLE` で変更できます。通常の `EXECUTABLE` は正しさ確認とMPI x OpenMP構成探索に使い、強スケーリングは `generate_benchmark_cases.py` 内のMPI数別マッピングを優先します。

このサンプルでは `effective_ens_id = mpi_rank % PBVR_WEAK_BASE_ENS` を使い、`kd = 1.0 + effective_ens_id` とします。デフォルトの `PBVR_WEAK_BASE_ENS` は4です。MPI数を4の倍数にすると、同じensemble patternが繰り返されます。

分布確認ログは通常rank 0のみ出力します。全rankで確認したい場合は以下を指定します。

```sh
export PBVR_WEAK_DEBUG=1
```

## PBSジョブの形式

生成されるPBSジョブは、おおむね以下の形式です。

```sh
#PBS -q ${QUEUE}
#PBS -l select=${NODES}:ncpus=${NCPUS}:mpiprocs=${MPIPROCS}:ompthreads=${OMPTHREADS}
#PBS -l walltime=${WALLTIME}
#PBS -N ${JOB_NAME}
#PBS -j oe

cd $PBS_O_WORKDIR

. /etc/profile.d/modules.sh
module load ${MODULE_INTEL}
module load ${MODULE_MPI}

export OMP_NUM_THREADS=${OMPTHREADS}
export KMP_AFFINITY=disabled

${MPI_RUNNER} -n ${TOTAL_MPIPROCS} omplace ${EXECUTABLE} ${INPUT_ARGS} > ${LOG_FILE} 2>&1
```

この環境では `omplace -nt ${OMP_NUM_THREADS}` を付けるとエラーになるため、`omplace` には `-nt` を付けません。MPIプロセス数は `mpirun -n ${TOTAL_MPIPROCS}` で指定します。`TOTAL_MPIPROCS` は `nodes * mpiprocs` です。OpenMPスレッド数は従来通り `OMP_NUM_THREADS` とPBSの `ompthreads` で管理します。

## スパコン上での基本実行手順

### 1. リポジトリをスパコンへコピー

任意の方法で、このリポジトリをスパコン上の作業ディレクトリへコピーしてください。その後、以下のように `Server/BENCHMARK` へ移動して作業します。

```sh
cd Server/BENCHMARK
```

### 2. 設定確認

```sh
vi config.sh
```

必要に応じて以下を変更します。

- `QUEUE`
- `PROJECT`
- `WALLTIME`
- `EXECUTABLE`
- `INPUT_ARGS`
- `NCPUS_PER_NODE`

### 3. ビルド

```sh
./build.sh
```

`build.sh` は `Server/BENCHMARK` から実行しますが、内部では1階層上の `Server` を `REPO_ROOT` としてビルドします。通常用の `EXECUTABLE`、強スケーリング用の `STRONG_EXECUTABLES`、弱スケーリング用の `WEAK_EXECUTABLE` を重複排除してビルドします。このスクリプトは、マニュアルに基づいて以下を行います。

```sh
. /etc/profile.d/modules.sh
module load cuda/11.4
module load gnu/cur
module load intel/2023.2.1
module load mpt/2.23-ga
```

その後、`PBVR_MACHINE=Makefile_machine_s86_mpi_omp` を使ってビルドします。

### 4. ベンチマークケース生成

```sh
python3 generate_benchmark_cases.py
```

これにより `cases.csv` が作成されます。

### 5. PBSジョブ生成

```sh
python3 generate_pbs_jobs.py
```

これにより `pbs_jobs/` 以下にPBSジョブスクリプトが作成されます。

### 6. まず正しさ確認ジョブを投入

```sh
./submit_correctness.sh
```

正しさ確認が通らない場合は、性能評価には進まないでください。

### 7. 性能評価ジョブを投入

正しさ確認後、必要なフェーズを投入します。

```sh
./submit_strong_scaling.sh
./submit_weak_scaling.sh
./submit_mpi_omp_sweep.sh
```

すべてまとめて投入する場合は以下です。

```sh
./submit_all.sh
```

## 正しさ確認

逐次版と並列版の出力比較には `compare_outputs.py` を使います。

例:

```sh
python3 compare_outputs.py \
  --reference benchmark_results/correctness_mpi_2x1 \
  --candidate benchmark_results/correctness_hybrid_4x4 \
  --rtol 1e-5 \
  --atol 1e-7
```

比較方針は以下です。

- 完全一致できるファイルは hash で比較します。
- テキスト中の数値は、浮動小数点誤差を考慮して `rtol` / `atol` で比較します。
- バイナリや画像については、現状では厳密一致が基本です。
- 画像専用の比較指標が必要な場合は、後でPSNRや差分画像などを追加してください。

## タイマー集計

ジョブ終了後、以下を実行します。

```sh
python3 parse_timing.py --input benchmark_results
```

既存の `ensemble_timer_summary.csv` がある場合もパースできます。

```sh
python3 parse_timing.py --input Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/ensemble_timer_summary.csv
```

出力先はデフォルトで `benchmark_analysis/` です。

## 結果集計

```sh
python3 summarize_results.py
```

以下を生成します。

- `scaling_summary.csv`
- `section_summary.csv`
- `bottleneck_candidates.csv`

判断では、並列版の平均時間だけではなく、必ず `max` を使います。並列実行では最も遅いrankが全体時間を決めるためです。

## レポート生成

```sh
python3 make_report.py
```

以下が生成されます。

```text
benchmark_analysis/BENCHMARK_REPORT.md
```

## 判断基準

### max/avg

| max/avg | 判断 |
| --- | --- |
| 1.00〜1.05 | 良好 |
| 1.05〜1.20 | 軽度の偏り |
| 1.20以上 | 要確認 |
| 2.00以上 | 強い負荷不均衡、または待ち時間の可能性 |

### 区間寄与率

```text
contribution = opt_section_max / opt_total_max
```

全体時間に対する各区間の寄与率です。

### 区間speedup

```text
section_speedup = serial_section_time / opt_section_max
```

1.0未満の場合、その区間は並列版で逐次版より遅くなっています。

### parallel efficiency

```text
parallel_efficiency = speedup / total_cores
```

強スケーリング時の並列効率です。

## 評価フェーズ

### Phase 1: 正しさ確認

小規模データで以下を比較します。

- serial
- 2 MPI x 1 thread
- 2 MPI x N threads
- M MPI x 1 thread
- M MPI x T threads

確認対象は以下です。

- 出力画像
- 出力ファイル
- 中間物理量
- min/max値
- 総量
- 代表格子点の値
- 粒子数
- ポリゴン数
- サンプル数

### Phase 2: 同一問題サイズ比較

全系サイズを固定し、serial と opt を比較します。

並列版の比較値には `opt max` を使います。

### Phase 3: 強スケーリング

全系サイズを固定し、MPI数やOpenMPスレッド数を増やします。

### Phase 4: 弱スケーリング

1 rank あたりの問題サイズを固定し、rank数に比例して全体サイズを増やします。

特に以下を見ます。

- global communication
- compositing
- I/O

### Phase 5: MPI x OpenMP 構成探索

1ノード40コアを想定し、以下のような組み合わせを比較します。

- 40 MPI x 1 thread
- 20 MPI x 2 threads
- 10 MPI x 4 threads
- 5 MPI x 8 threads
- 4 MPI x 10 threads
- 2 MPI x 20 threads
- 2 MPI x 20 threads

40コア固定ではなくしたい場合は、`config.sh` の `NCPUS_PER_NODE` を変更してください。

## ローカルPCでできる確認

ローカルPCでは以下を実行できます。

```sh
./run_local_tests.sh
```

これは以下だけを確認します。

- Pythonスクリプトの構文チェック
- `cases.csv` の生成
- PBSジョブスクリプトの生成
- 既存サンプル `ensemble_timer_summary.csv` のパース
- 集計CSVとMarkdownレポートの生成

PBSジョブ投入や大規模実行は行いません。

## 配置に関する注意

- ベンチマーク補助ファイルは `Server/BENCHMARK` に集約しています。
- `build.sh` は `REPO_ROOT=Server` を参照してビルドします。
- 生成されるPBSジョブは、`PBS_O_WORKDIR` を `BENCHMARK_DIR` とみなし、`REPO_ROOT=$BENCHMARK_DIR/..` から実行バイナリを参照します。
- ログと集計結果は `Server/BENCHMARK/benchmark_results` と `Server/BENCHMARK/benchmark_analysis` に出力されます。

## 注意点

- 大規模性能測定は必ずスパコン上で行ってください。
- 正しさ確認が通る前に性能評価結果を解釈しないでください。
- 並列版の評価では平均時間だけでなく、必ず最大rank時間を確認してください。
- I/O込み時間とI/O除外時間は分けて確認してください。
- compositing、`MPI_Gatherv`、`MPI_Waitall`、`MPI_Barrier`、OpenMP `critical` は重点確認対象です。
- 今回の対象コードは1 MPIプロセスでは動作しないため、`mpiprocs=1` のケースは生成しません。
