# Intel oneAPI 向け最適化候補

## コンパイラ・実行環境

現在の想定モジュールは以下である。

```sh
module load cuda/11.4
module load gnu/cur
module load intel/2023.2.1
module load mpt/2.23-ga
```

MPI + OpenMP 用 machine file では、Intel向けに以下のようなオプションが使われる。

```text
-qopenmp -xCORE-AVX2 -O3
```

ベクトル化状況を確認する場合は、通常測定用ビルドとは別に、Intelの最適化レポートを有効にしたビルドを作る。

```sh
CXXFLAGS_EXTRA="-qopt-report=5 -qopt-report-phase=vec"
```

実際に使えるオプション名はスパコン上の oneAPI バージョンに合わせて確認する。

## 静的最適化候補

| 候補 | 対象 | 期待効果 | リスク | メモ |
| --- | --- | --- | --- | --- |
| SIMD化状況の確認 | `calculate_scalar_and_chain_rule_grad()` 周辺 | 中〜高 | 低 | Function parser呼び出しや仮想関数がSIMD化を阻害する可能性がある。 |
| 非連続メモリアクセス削減 | セル補間、粒子buffer | 中 | 中 | ランダムなセルアクセスやAoS配置がキャッシュ効率を下げる可能性がある。 |
| 幾何計算の重複削減 | `uniform_volume_calculation`, gradient系 | 高 | 中 | メッシュが固定なら体積や幾何情報をキャッシュできる可能性がある。 |
| OpenMP schedule調整 | uniform loop, histogram loop | 中 | 低〜中 | `dynamic` は負荷分散に効くが、スケジューリングコストが増える。 |
| critical merge削減 | particle buffer merge | 中 | 中 | prefix-sum方式などでcriticalを避けられるが、メモリ設計が必要。 |
| chain-rule微分処理改善 | `*_chain_rule_*` | 中〜高 | 中 | 解析微分/自動微分は速くなる可能性があるが、数値検証が必須。 |
| active variable限定 | q/grad setup | 中〜高 | 中 | 未使用変数を避けられるが、active判定の正しさが重要。 |
| 数学関数の見直し | Function parser / TF eval | 低〜中 | 中 | `pow(x,2)` などは置換可能だが、TF式の意味を変えないこと。 |
| NUMA/配置確認 | PBS `ompthreads`, `omplace` | 中 | 低 | この環境では `omplace -nt` は使わず、`mpirun -n <N> omplace` を使う。 |
| I/O分離 | output系タイマー | 中 | 低 | I/O込み時間とI/O除外時間を分けて評価する。 |
| gather/compositing分散化 | `MPI_Gatherv` 周辺 | 高 | 高 | アルゴリズム変更になるため、初期の低リスク最適化ではない。 |

## SIMD効果の確認方法

1. 通常のタイミング用バイナリをビルドする。
2. Intel最適化レポート有効版を別ビルドする。
3. 同じ小規模ケースを実行する。
4. `uniform_calc_scalar_grad`, `uniform_volume_calculation`, `uniform_particle_sampling_loop` を比較する。
5. レポート中の vectorized / not vectorized の理由を見る。
6. 最初からSIMD幅を無理に変えず、まずコンパイラレポートと区間計測で確認する。
