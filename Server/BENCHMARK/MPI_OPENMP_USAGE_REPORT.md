# MPI / OpenMP 利用箇所レポート

## MPI呼び出しの主な箇所

| ファイル | 行 | MPI呼び出し | 用途 | 通信対象 | ボトルネック候補 |
| --- | ---: | --- | --- | --- | --- |
| `App/main.cpp` | 19 | `MPI_Init` | MPI初期化 | 全rank | 初期化のみ。通常は低リスク。 |
| `App/main.cpp` | 25 | `MPI_Comm_rank` | rank判定 | `MPI_COMM_WORLD` | 低。 |
| `App/main.cpp` | 26 | `MPI_Comm_size` | プロセス数取得 | `MPI_COMM_WORLD` | 低。 |
| `App/main.cpp` | 43 | `MPI_Finalize` | MPI終了 | 全rank | 終了待ちが出る場合のみ確認。 |
| `App/ServerWorker.cpp` | 49 | `MPI_Bcast` | task payload size 配布 | rank 0 から worker | 小さいbroadcastが多い場合は候補。 |
| `App/ServerWorker.cpp` | 61 | `MPI_Bcast` | task payload 配布 | rank 0 から worker | rank 0 fan-out。 |
| `App/TaskSignalTransferProtocol.cpp` | 19 | `MPI_Bcast` | control message size 配布 | rank 0 から worker | 同期待ち候補。 |
| `App/TaskSignalTransferProtocol.cpp` | 34 | `MPI_Bcast` | control message body 配布 | rank 0 から worker | 同期待ち候補。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 519 | `MPI_Allgather` | processor名収集 | split communicator setup | setupのみ。低。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 600 | `MPI_Allgather` | 粒子数収集 | split communicator | 粒子数不均衡の指標。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 620 | `MPI_Gatherv` | 粒子座標収集 | split comm root | rank 0 集中。高。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 624 | `MPI_Gatherv` | 粒子色収集 | split comm root | rank 0 集中。高。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 628 | `MPI_Gatherv` | 粒子法線収集 | split comm root | rank 0 集中。高。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 676 | `MPI_Reduce` | histogram集約 | rank 0 | 中。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 699 | `MPI_Reduce` | global max集約 | rank 0 | 中。 |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 700 | `MPI_Reduce` | global min集約 | rank 0 | 中。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 301 | `MPI_Reduce` | timer粒子数/統計集約 | rank 0 | 診断用。低〜中。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 347 | `MPI_Reduce` | timer avg/max/min集約 | rank 0 | 診断用。低〜中。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 540 | `MPI_Allgather` | 粒子数収集 | split communicator | 不均衡指標。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 563 | `MPI_Gatherv` | 粒子座標収集 | split comm root | compositing/gather集中。高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 566 | `MPI_Gatherv` | 粒子色収集 | split comm root | 高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 569 | `MPI_Gatherv` | 粒子法線収集 | split comm root | 高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 826 | `MPI_Reduce` | ensemble統計min range | root | 中。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 828 | `MPI_Reduce` | ensemble統計histogram | root | bins数が多い場合は中〜高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 1303 | `MPI_Barrier` | ファイル書き込み待ち | 全rank | I/O不均衡があると高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2430 | `MPI_Sendrecv` | ensemble ring shift size交換 | 隣接rank | payload不均衡時に候補。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2439 | `MPI_Isend` | shift size送信 | 隣接rank | 直後Waitなら重なりは限定的。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2440 | `MPI_Irecv` | shift size受信 | 隣接rank | 直後Waitなら重なりは限定的。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2441 | `MPI_Waitall` | size交換完了待ち | 隣接rank | 待ち時間候補。高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2542 | `MPI_Irecv` | shift payload受信 | 隣接rank | 粒子数が多い場合高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2548 | `MPI_Isend` | shift payload送信 | 隣接rank | 粒子数が多い場合高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2554 | `MPI_Waitall` | payload受信完了待ち | 隣接rank | 待ち時間候補。高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2555 | `MPI_Waitall` | payload送信完了待ち | 隣接rank | 待ち時間候補。高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 3009 | `MPI_Barrier` | state/final同期 | 全rank | 直前区間の不均衡を反映。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 3536 | `MPI_Allreduce` | 座標min/max集約 | 全rank | 中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 235 | `MPI_Allreduce` | cell-center `sum(x)` | ensemble comm | payload大。高候補。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 236 | `MPI_Allreduce` | cell-center `sum(x^2)` | ensemble comm | payload大。高候補。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 592 | `MPI_Allreduce` | statistic min | ensemble comm | 中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 593 | `MPI_Allreduce` | statistic max | ensemble comm | 中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 695 | `MPI_Reduce` | statistic histogram bins | root | bins/variables増加時に中〜高。 |

## MPIリスクまとめ

- `MPI_Gatherv` による粒子座標・色・法線の収集は rank 0 集中になりやすい。
- ensemble shift通信は隣接rank通信だが、`MPI_Waitall` が支配的になる可能性がある。
- `MPI_Barrier` は直前区間の不均衡やI/O待ちを見える化する。
- `EnsembleCellHistogram` の `MPI_Allreduce` は `ncells * nvariables` の配列を扱うため、大規模ケースでは重くなりやすい。
- 並列性能判断では平均だけでなく `max` と `max/avg` を必ず見る。

## OpenMP利用箇所

| ファイル | 行 | OpenMP構文 | 対象処理 | schedule | reduction | ボトルネック候補 |
| --- | ---: | --- | --- | --- | --- | --- |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 1938 | `#pragma omp parallel` | uniform粒子生成領域 | N/A | thread-local collector | 大きいmain kernel。高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 1997 | `#pragma omp for schedule(dynamic) nowait` | uniform cell sampling loop | dynamic | 直接reductionなし | dynamic overheadと負荷分散の両方を確認。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2298 | `#pragma omp critical` | thread-local粒子buffer merge | N/A | manual merge | 直列化候補。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2566 | `#pragma omp parallel` | shift補間/統計領域 | N/A | thread-local collector | 高。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2598 | `#pragma omp for` | shift粒子補間loop | default static | なし | payload不均衡時に候補。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2782 | `#pragma omp parallel` | rejection/統計出力準備 | N/A | thread-local | 中。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2808 | `#pragma omp for schedule(dynamic)` | rejection loop | dynamic | なし | 中。 |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2862 | `#pragma omp critical` | rejection thread merge | N/A | manual merge | 粒子数が多い場合に中〜高。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 181 | `#pragma omp parallel` | cell-center値評価 | N/A | thread-local | セル数大で高。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 190 | `#pragma omp for schedule(dynamic)` | cell-center評価loop | dynamic | なし | 中〜高。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 270 | `#pragma omp parallel` | statistic min/max | N/A | thread-local | 中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 277 | `#pragma omp for schedule(dynamic)` | statistic min/max loop | dynamic | manual | 中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 308 | `#pragma omp critical` | thread-local min/max merge | N/A | manual | 変数数依存。低〜中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 353 | `#pragma omp parallel` | local histogram | N/A | thread-local histogram | メモリ使用量に注意。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 359 | `#pragma omp for schedule(dynamic)` | histogram loop | dynamic | manual | 中。 |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 377 | `#pragma omp critical` | histogram merge | N/A | manual | bins/variables増加時に中。 |
| `InSituLib/AMR/kvs_wrapper.cpp` | 793 | `#pragma omp parallel` | AMR sampling | N/A | manual | AMR pathでは高。 |
| `InSituLib/AMR/kvs_wrapper.cpp` | 855 | `#pragma omp for` | AMR cell loop | default | なし | 中。 |
| `InSituLib/AMR/kvs_wrapper.cpp` | 991 | `#pragma omp for schedule(dynamic,1)` | AMR particle loop | dynamic,1 | なし | schedule overhead候補。 |
| `InSituLib/AMR/kvs_wrapper.cpp` | 1149 | `#pragma omp critical` | AMR merge | N/A | manual | 高候補。 |
| `VisModule/DaemonAndSampler/Daemon/ParticleFile.cpp` | 108 | `#pragma omp parallel` | particle file処理 | N/A | なし | I/O並列処理候補。 |

## OpenMPリスクまとめ

- uniform粒子生成loopはdynamic scheduleのため、負荷分散には効くがschedule overheadが出る可能性がある。
- thread-local bufferによりhot loop中のatomicは避けているが、最後のcritical mergeが目立つ可能性がある。
- histogramはthread-local方式で妥当だが、mergeコストは `nthreads * nvariables * nbins` に比例する。
- scattered cell accessやAoS/SoA配置により、SIMD化やcache効率が制限される可能性がある。
- NUMA配置はPBSの `ompthreads`、`mpirun -n <N> omplace`、first-touchの影響を受ける。

## raw CSV

完全な検索結果は以下を参照する。

- `mpi_calls.csv`
- `openmp_usage.csv`
