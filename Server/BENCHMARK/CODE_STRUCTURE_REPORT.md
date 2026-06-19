# コード構成レポート

## 対象範囲

このレポートは、PBVR server側リポジトリについて、ビルド構成、MPI/OpenMP並列化、可視化処理、通信、I/O、タイマー解析に関係する主要ファイルを整理する。

ローカルPCでは大規模性能測定は行わず、静的解析、PBSジョブ生成、タイマーCSV集計、レポート生成のみを行う。

## スパコン実行環境メモ

現在のベンチマークスクリプトは以下のモジュールを前提にしている。

```sh
module load cuda/11.4
module load gnu/cur
module load intel/2023.2.1
module load mpt/2.23-ga
```

MPI + OpenMP 実行は以下の形式で行う。

```sh
mpirun -n <MPIプロセス数> omplace <実行バイナリ> <入力引数>
```

この環境では `omplace -nt ${OMP_NUM_THREADS}` は使わない。OpenMPスレッド数はPBSの `ompthreads` と `OMP_NUM_THREADS` で管理する。

## リポジトリ構成

| 項目 | 場所 | 内容 |
| --- | --- | --- |
| トップレベルビルド | `Makefile`, `pbvr.conf`, `arch/` | PBVR全体のビルド入口。 |
| serial向けmachine file | `arch/Makefile_machine_s86` | MPI/OpenMPなし構成の候補。 |
| OpenMP向けmachine file | `arch/Makefile_machine_s86_omp` | OpenMP有効構成。 |
| MPI+OpenMP向けmachine file | `arch/Makefile_machine_s86_mpi_omp` | `mpicxx`, `mpicc`, `mpif90`, `-qopenmp`, `-xCORE-AVX2` を使用。 |
| メインサーバ | `App/main.cpp`, `App/Server.cpp`, `App/ServerWorker.cpp` | MPI初期化、rank分岐、task broadcast。 |
| 共通InSitu処理 | `InSituLib/shared/` | 粒子gather、ParticleProperty、非同期write補助。 |
| 構造格子 | `InSituLib/struct/kvs_wrapper.cpp` | 構造格子用PBVR wrapper。 |
| 非構造格子 | `InSituLib/unstruct/kvs_wrapper.cpp` | アンサンブル粒子生成、タイマー、MPI shift などの主要対象。 |
| AMR | `InSituLib/AMR/kvs_wrapper.cpp` | AMR用粒子生成と旧タイマー出力。 |
| Function parser | `FunctionParser/` | TF式、RPN、数式評価関連。 |
| 可視化モジュール | `VisModule/` | Cell、Renderer、Compositor、FileFormat、Timerなど。 |
| C Example | `Example/C/s86_mpi_omp/` | Hydrogen / ensemble のテストコード。 |
| Fortran Example | `Example/Fortran/` | Fortran連携例。 |

## ビルド方法

基本ビルド:

```sh
make PBVR_MACHINE=Makefile_machine_s86_mpi_omp -j ${MAKE_JOBS}
```

`BENCHMARK/build.sh` は `Server/BENCHMARK` から実行されるが、内部では1階層上の `Server` を `REPO_ROOT` としてビルドする。

## デフォルト実行バイナリ

```text
Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/run
```

`config.sh` の `EXECUTABLE`、環境変数 `EXECUTABLE`、または `generate_benchmark_cases.py --executable` で変更できる。

## 入力データ

| 種類 | 場所 | 内容 |
| --- | --- | --- |
| Example JSON | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/default.json` | 可視化設定。 |
| ensemble statistics JSON | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/default_ensemble_statistics.json` | 統計量TF設定。存在しない場合は対象Example側を確認する。 |
| Hydrogen生成データ | `Example/C/s86_mpi_omp/*/Hydrogen.cpp` | 多くのExampleはコード内でデータ生成する。 |
| 外部入力 | 実行時指定 | `INPUT_ARGS` または `cases.csv` の `input_args` で指定する。 |

## 出力データ

| 出力 | 場所 | 内容 |
| --- | --- | --- |
| 粒子ファイル | Example実行時の出力先 | wrapper設定に依存する。 |
| タイマーCSV | `ensemble_timer_summary.csv` | `InSituLib/unstruct/kvs_wrapper.cpp` が出力する。 |
| PBSログ | `benchmark_results/<case>/<case>.log` | PBSジョブの標準出力/標準エラー。 |
| 集計CSV | `benchmark_analysis/*.csv` | `parse_timing.py` / `summarize_results.py` の出力。 |

## 動作確認用コード

| テスト | 場所 | 内容 |
| --- | --- | --- |
| Function parser test | `FunctionParser/test.cpp` | 数式parser確認。 |
| Hydrogen unstruct | `Example/C/s86_mpi_omp/Hydrogen_unstruct/` | 非構造格子Hydrogen例。 |
| Ensemble Hydrogen | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/` | 今回のデフォルトベンチマーク対象。 |
| Ensemble Hydrogen mpi4 | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/` | サンプルtimer CSVが残っている場合の参照先。 |
| AMR Hydrogen | `Example/C/s86_mpi_omp/Hydrogen_AMR/` | AMRパス確認。 |

## 主要ファイル

| 領域 | ファイル |
| --- | --- |
| 非構造粒子生成 | `InSituLib/unstruct/kvs_wrapper.cpp` |
| 構造格子粒子生成 | `InSituLib/struct/kvs_wrapper.cpp` |
| AMR粒子生成 | `InSituLib/AMR/kvs_wrapper.cpp` |
| Cell補間/勾配 | `VisModule/Visualization/Mapper/*Cell.*` |
| 粒子render/compositing | `VisModule/Visualization/Renderer/ParticleBufferCompositor.cpp`, `ParticleBufferAccumulator.cpp`, `ParticleVolumeRenderer.*` |
| Transfer Function | `FunctionParser/`, `VisModule/Visualization/Mapper/TransferFunction*` |
| MPI通信 | `App/*`, `InSituLib/shared/kvs_wrapper_common.cpp`, `InSituLib/unstruct/kvs_wrapper.cpp` |
| I/O | `InSituLib/shared/particle_write_thread.h`, `VisModule/DaemonAndSampler/Daemon/ParticleFile.cpp` |

## タイマー出力形式

既存のensemble timerは以下のCSV形式で出力される。

```text
step,scope,parent_section,section,level,mpi_avg_sec,mpi_max_sec,mpi_min_sec,thread_avg_sec,thread_max_sec,thread_min_sec,uniform_particle_count,ave_particle_count,var_particle_count,cov_particle_count
```

並列性能評価では、平均だけでなく `mpi_max_sec` を重視する。
