# KVSMLConverter

このディレクトリは PBVR Server を扱う。
以降の説明は特に説明がない限り `Server/KVSMLConverer` ディレクトリを基準とした説明となる。

## ビルド方法

以下のコマンドを実行する。

```sh
make -j 
```

`make clean` は、ユーザーから明示的に指示された場合、または生成物の不整合が確認された場合のみ、ビルド前に実行する。

```sh
make clean
make -j
```

## 実行方法

`Server/KVSMLConverter/Example/Release/kvsml-converter` に実行ファイルが生成される

