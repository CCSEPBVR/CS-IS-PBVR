# タイマーレポート

## タイマー実装

| タイマー | ファイル | 開始行 | 終了行 | 対象処理 | rank別 | thread別 | 性能評価で使えるか |
| --- | --- | ---: | ---: | --- | --- | --- | --- |
| `vismodule::Timer` | `VisModule/Utility/Timer.*` | N/A | N/A | 汎用wall clock計測 | 呼び出し側依存 | 呼び出し側依存 | 粗い区間計測に有効。並列比較ではMPI maxを見る。 |
| `EnsembleTimerCollector` | `InSituLib/unstruct/kvs_wrapper.cpp` | 225 | 405 | ensemble粒子生成の区間計測 | MPI avg/max/minあり | thread avg/max/minあり | 主要な性能評価用タイマー。 |
| `EnsembleTimerScope` | `InSituLib/unstruct/kvs_wrapper.cpp` | 412 | 430 | RAII形式の区間計測 | rank内集計 | 単体ではなし | 有効。 |
| `ChainRuleTimingBreakdown` | `InSituLib/unstruct/kvs_wrapper.cpp` | around 1347 | around 1485 | scalar + chain-rule gradient の細分化 | collectorに集約 | collectorに集約 | 局所kernel診断に有効。 |
| `AMR time struct` | `InSituLib/AMR/kvs_wrapper.cpp` | around 502 | 508 | AMR initialize/sampling/write/reduce | MPI max reduce | なし | AMRパス評価に有効。 |
| `particle_write_thread` timer | `InSituLib/shared/particle_write_thread.h` | 54 | 65 | 非同期粒子書き込み | MPI集約なし | worker thread | I/O診断に有効。 |
| `thread_timer` helpers | `InSituLib/shared/thread_timer.h` | 113 | 140 | chronoベースの汎用区間 | 呼び出し側依存 | 呼び出し側依存 | 補助的に利用可能。 |

## Ensemble Timer CSV形式

`InSituLib/unstruct/kvs_wrapper.cpp` は `ensemble_timer_summary.csv` を以下の形式で出力する。

```text
step,scope,parent_section,section,level,mpi_avg_sec,mpi_max_sec,mpi_min_sec,thread_avg_sec,thread_max_sec,thread_min_sec,uniform_particle_count,ave_particle_count,var_particle_count,cov_particle_count
```

並列実行では最遅rankが全体時間を決めるため、比較には `mpi_max_sec` を使う。

## 主要タイマー区間

| section | 分類 | 内容 | 確認観点 |
| --- | --- | --- | --- |
| `ensemble_generate_particles_total` | total | 粒子生成全体 | 全体時間。 |
| `set_parameter_path` | initialization | パラメータパス設定 | 不自然に大きい場合はファイル/パス処理を確認。 |
| `read_parameter_file` | initialization / I/O | パラメータ読み込み | rank間偏りやI/O待ち。 |
| `init_transfer_functions` | initialization | TF初期化 | 初期化コスト。 |
| `create_cells` | data_conversion | Cell作成 | CellBase生成・bind前処理。 |
| `sampling_prepare` | particle_generation | sampling前処理 | 本来小さい想定。大きい場合は範囲を再確認。 |
| `omp_uniform_sampling` | OpenMP_parallel_region | uniform粒子生成のOpenMP領域 | thread max/avgを見る。 |
| `uniform_bind_cell_array` | interpolation | Cell array bind | bind重複やcellアクセスの確認。 |
| `uniform_volume_calculation` | local_compute | セル体積計算 | HexahedralCell体積計算のボトルネック候補。 |
| `uniform_particle_count_calculation` | sampling | セルごとの粒子数計算 | 負荷分散に影響。 |
| `uniform_particle_sampling_loop` | sampling | 粒子生成ループ本体 | 乱数、補間、TF、storeが含まれる。 |
| `uniform_calculate_scalars` | interpolation | scalar補間 | chain-ruleと重複がないか確認。 |
| `uniform_chain_rule_grad` | local_compute | chain-rule法線計算 | dF/dq、grad_q、normalize。 |
| `uniform_calc_scalar_grad` | local_compute | scalar + gradient helper全体 | 細分化timerで内訳を見る。 |
| `uniform_q_values_grad_q_setup` | interpolation | q値とgrad_qの準備 | 変数数依存。 |
| `uniform_tf_scalar_eval` | visualization_kernel | TF評価 | Function parser評価コスト。 |
| `uniform_chain_rule_dfdq` | visualization_kernel | dF/dq計算 | 数値微分なら評価回数に注意。 |
| `uniform_normal_normalize` | local_compute | 法線正規化 | NaN/zero処理含む。 |
| `thread_particle_merge` | synchronization | thread-local粒子bufferのmerge | critical直列化候補。 |
| `mpi_shift_exchange` | halo_exchange / MPI | ensemble shift通信 | payload量とWait時間。 |
| `mpi_shift_size_exchange` | MPI | shiftサイズ交換 | max/avgが高い場合は待ち。 |
| `mpi_shift_payload_*` | MPI / pack / unpack | payload別通信 | cellids/scalars/coords/normals等の内訳。 |
| `omp_shift_interpolation` | OpenMP_parallel_region | shift後補間 | thread不均衡。 |
| `shift_calculate_scalars` | interpolation | shift粒子scalar計算 | uniform側との比較。 |
| `shift_chain_rule_grad` | local_compute | shift粒子chain-rule法線 | uniform側との比較。 |
| `stat_average_variance` | local_compute | 統計量計算 | ensemble統計の局所計算。 |
| `stat_histogram` | local_compute / MPI | histogram作成/集約 | bins数・変数数・MPI_Reduce。 |
| `omp_rejection` | OpenMP_parallel_region | rejection処理 | thread max/avg。 |
| `rejection_thread_merge` | synchronization | rejection後merge | critical候補。 |
| `output_particles_*` | I/O | 粒子出力 | I/O込み/除外で比較。 |
| `output_history` | I/O | history出力 | rank 0 集中の可能性。 |
| `async_io_wait` | I/O / synchronization | 非同期I/O待ち | I/Oボトルネック候補。 |
| `final_barrier_state` | synchronization | 最終同期 | 直前の不均衡が現れる。 |

## 評価方針

- serial相当またはbaselineとの比較では `mpi_max_sec` を使う。
- `max_over_avg` が大きい区間は、負荷不均衡または通信待ちの候補とする。
- OpenMPの偏りは `thread_max_sec / thread_avg_sec` を見る。
- I/O込み総時間とI/O除外総時間は分けて評価する。
- compositing / gather は `MPI_Gatherv` 周辺と output 系タイマーを重点確認する。

## raw CSV

検索結果の一覧は `timer_occurrences.csv` を参照する。
