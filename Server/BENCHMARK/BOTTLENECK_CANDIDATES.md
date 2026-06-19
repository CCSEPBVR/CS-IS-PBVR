# ボトルネック候補

このファイルは実測前の静的な候補一覧である。実測後の自動抽出結果は `benchmark_analysis/bottleneck_candidates.csv` を参照する。

| 優先度 | 区間 / 領域 | 症状 | 根拠 | 原因仮説 | 次に確認すること |
| ---: | --- | --- | --- | --- | --- |
| 1 | `uniform_volume_calculation` | 粒子生成で支配的になりやすい | `InSituLib/unstruct/kvs_wrapper.cpp` のタイマー区間 | HexahedralCell の体積計算、ヤコビアン、幾何計算の重複 | 体積キャッシュ案、四面体分割版、SIMDレポートと比較する |
| 1 | `uniform_particle_sampling_loop` | ローカル計算が重い | OpenMP dynamic loop around `InSituLib/unstruct/kvs_wrapper.cpp:1997` | 乱数、局所座標生成、補間、TF評価、粒子格納が集中 | fine timer と thread max/avg を確認する |
| 1 | `uniform_calc_scalar_grad` / `shift_calc_scalar_grad` | チェーンルール法線計算が重い | `InSituLib/unstruct/kvs_wrapper.cpp:1367` 以降の細分化タイマー | q/grad setup、TF評価、dF/dq、正規化 | `q setup`, `TF eval`, `dF/dq`, `normalize` を分けて見る |
| 1 | `MPI_Gatherv` 粒子 gather | rank 0 集中 | `InSituLib/unstruct/kvs_wrapper.cpp:563-569` | rank 0 のメモリ帯域、ネットワークfan-in | 粒子数不均衡と gather / output 区間を比較する |
| 1 | `mpi_shift_exchange` / `MPI_Waitall` | 通信待ち | `InSituLib/unstruct/kvs_wrapper.cpp:2439-2555` | payload不均衡、非同期通信後すぐ待っている | payload別タイマーと rank max/avg を確認する |
| 2 | `thread_particle_merge` | OpenMPの直列化 | `InSituLib/unstruct/kvs_wrapper.cpp:2298` の critical | thread-local vector のmergeが直列化 | 粒子数に対するmerge寄与率を見る |
| 2 | `async_io_wait` / `output_particles_*` | I/O待ち | output系タイマー | ストレージ待ち、rank 0 書き込み集中 | I/O込み / I/O除外の総時間を分ける |
| 2 | `EnsembleCellHistogram` の `MPI_Allreduce` | 集団通信payloadが大きい | `InSituLib/unstruct/EnsembleCellHistogram.cpp:235-236` | `ncells * nvariables` のfloat配列集約 | collective時間とメモリ量を見る |
| 2 | `stat_histogram` | histogram作成/集約が重い | `EnsembleCellHistogram.cpp:635-695` | thread-local histogram merge と `MPI_Reduce` | bins数、変数数、OpenMP merge時間を見る |
| 3 | `MPI_Barrier` final/state | 最遅rankに支配される | `InSituLib/unstruct/kvs_wrapper.cpp:1303`, `3009`, `3494` | 直前区間の不均衡がbarrierで見える | 直前区間の max/avg と相関を見る |
