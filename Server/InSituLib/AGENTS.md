# InSituLib

このディレクトリは構造格子・非構造格子向けの In-Situ PBVR ライブラリを扱う。

## ディレクトリ構造

- `struct/` は構造格子向け処理を扱う。
- `unstruct/` は非構造格子向け処理を扱う。
- `shared/` は両方から利用する共通処理を扱う。

## ビルド上の注意

- `CPU_VER` の有無で MPI コードのコンパイル範囲が変わる。
- `PBVR_SHIFT_ALLOC_MEM` は MPI が利用できる構成でのみ有効にする。
- `MPI_Alloc_mem` や `MPI_Free_mem` を使用するコードは、MPI ヘッダが有効になる条件と同じプリプロセッサ条件で囲む。
- `struct`、`unstruct`、`shared` の呼び出し関係を維持する。

ビルドは原則として `Server` の親 Makefile から行う。
