# ベンチマーク計画

## 目的

PBVR のアンサンブル可視化コードについて、MPI + OpenMP 並列版の性能を評価し、性能劣化要因を切り分けられる状態にする。主に以下を確認する。

- ローカル計算の重さ
- MPI通信時間
- 同期待ち
- I/O時間
- compositing / gather 処理
- OpenMP の負荷不均衡

今回の対象コードは 1 MPI プロセスでは動作しないため、`mpiprocs=1` のケースは生成しない。

## Phase 1: 正しさ確認

基本ケース:

- `correctness_mpi_2x1`
- `correctness_mpi_2x2`
- `correctness_hybrid_4x4`

確認項目:

- 出力ファイルまたは画像が妥当であること
- 浮動小数点値は許容誤差つきで比較すること
- 粒子数、ポリゴン数、サンプル数
- min/max 値
- 代表点または代表セルの値
- ヒストグラム総数

正しさ確認が通らない場合は、性能評価に進まない。

## Phase 2: 同一問題サイズ比較

- 全体問題サイズを固定する。
- 基準ケースと最適化ケースを比較する。
- 並列実行では平均値だけでなく `mpi_max_sec` を使う。
- rank別 raw ログがある場合は min / avg / max / std / max_over_avg を確認する。

## Phase 3: 強スケーリング

基本ケース:

- `strong_2x1`
- `strong_4x1`
- `strong_8x1`
- `strong_16x1`
- `strong_32x1`

全体問題サイズを固定し、MPIプロセス数を増やして speedup と parallel efficiency を計算する。

## Phase 4: 弱スケーリング

1 rank あたりの問題サイズをおおむね固定し、ノード数またはMPIプロセス数に応じて全体問題サイズを増やす。

特に以下を確認する。

- global communication
- compositing / gather
- I/O

## Phase 5: MPI x OpenMP 構成探索

1ノード40コア想定の基本ケース:

- `sweep_40x1`
- `sweep_20x2`
- `sweep_10x4`
- `sweep_8x5`
- `sweep_5x8`
- `sweep_4x10`
- `sweep_2x20`

`NCPUS_PER_NODE` は `config.sh` で変更できる。ケース生成時には `mpiprocs >= 2` かつ `mpiprocs * ompthreads <= NCPUS_PER_NODE` の組み合わせだけを採用する。

## 自動判定基準

### max/avg

| 値 | 判断 |
| --- | --- |
| 1.00-1.05 | 良好 |
| 1.05-1.20 | 軽度の偏り |
| 1.20以上 | 要確認 |
| 2.00以上 | 強い負荷不均衡または待ち時間の可能性 |

### 区間寄与率

```text
contribution = opt_section_max / opt_total_max
```

全体時間に対する各区間の寄与率。寄与率が高い区間を優先して調査する。

### 区間 speedup

```text
section_speedup = baseline_section_time / opt_section_max
```

1.0 未満の場合、その区間は基準ケースより遅くなっている。

### 並列効率

```text
parallel_efficiency = speedup / total_cores
```

## 標準実行フロー

```sh
./build.sh
python3 generate_benchmark_cases.py
python3 generate_pbs_jobs.py
./submit_correctness.sh
# 正しさ確認後
./submit_strong_scaling.sh
./submit_weak_scaling.sh
./submit_mpi_omp_sweep.sh
python3 parse_timing.py --input benchmark_results
python3 summarize_results.py
python3 make_report.py
```
