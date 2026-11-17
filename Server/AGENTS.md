# Server

このディレクトリは PBVR Server を扱う。
以降の説明は特に説明がない限り `Server` ディレクトリを基準とした説明となる。

## ビルド方法

以下のコマンドを実行する。

```sh
make -j $(nproc)
```

`make clean` は、ユーザーから明示的に指示された場合、または生成物の不整合が確認された場合のみ、ビルド前に実行する。
```sh
make clean
make -j $(nproc)
```

## 構成上の注意

- `pbvr.conf` の `PBVR_MACHINE` の値は `arch/Makefile_machine*`のどのファイルを参照するのかを指定している
- `CS-IS-PBVR` ディレクトリを基準として `KVS/Install/kvs.conf` の `KVS_SUPPORT_EXTENDED_FILE_FORMAT` により、Extended File Format 関連コードのコンパイル有無が変わる
- MPI 用コードは `arch/Makefile_machine*` で `MPICXX` が定義されている場合に有効になる
- MPI 用コードが有効でない場合は `CPU_VER` が有効になっている

## 実行方法

`Server/pbvr_server`に実行ファイルが生成される

通常実行時は `Server` ディレクトリを基準として以下のコマンドを実行する。
CSモードでMPI並列実行できない場合と、ISモードの場合は逐次実行で行う。
CSモードでの実行は特に指定しない場合は逐次実行で行う。

```sh
./pbvr_server
```

MPI並列実行時は `Server` ディレクトリを基準として以下のコマンドを実行する。
CSモードでMPI並列実行できる場合は2並列で行う。

```sh
mpirun -n [並列数] ./pbvr_server
```

サーバのポート番号を指定する場合は `Server` ディレクトリを基準として以下のコマンドを実行する。何も指定しなければ60000になる

```sh
./pbvr_server [ポート番号]
```
