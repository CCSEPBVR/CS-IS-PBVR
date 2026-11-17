# Client

このディレクトリは PBVR Client の Qt GUI アプリケーションを扱う。

## ビルド方法

ユーザーから別の指示がない場合は、`Client` ディレクトリを基準として以下を実行する。

```sh
mkdir -p build
cd build
qmake ../pbvr_client.pro CONFIG+=Release CONFIG+=c++17
make -j 4
```

並列ビルド後に単列 `make` を実行し、並列ログで見落としたエラーがないことを確認する。

## クリーン再生成

以下の場合に限り、`Client/build` の中身を削除し、`qmake` から再実行する。

- 異なる Qt バージョンの生成物が混在している
- 生成された Makefile に古い Qt パスが残っている
- ソースやプロジェクト設定の変更が反映されない
- ユーザーが明示的に指示した

削除対象は `Client/build` の中身に限定し、ソースディレクトリを削除しない。

## 成果物確認

ビルド後に以下の実行ファイルが存在することを確認する。

```text
Client/build/App/pbvr_client.app/Contents/MacOS/pbvr_client
Client/build/Tests/pbvr_client_qtest.app/Contents/MacOS/pbvr_client_qtest
```

## QtTest 実行方法

テスト対象を `TestsConfig.pri` で有効化し、ビルド実行後に `Client` ディレクトリを基準として以下を実行する。

```sh
cd build/Tests
./pbvr_client_qtest.app/Contents/MacOS/pbvr_client_qtest
```

## 注意点

- ビルド後に実行ファイルが存在しない場合は失敗として報告する
- 失敗として報告する場合は、作業ディレクトリ、コマンド、終了コード、最初の致命的エラーを簡潔に報告する
