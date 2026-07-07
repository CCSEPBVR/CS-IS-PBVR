# AGENTS.md

## リポジトリ構造

このリポジトリは Client/Server 構成の可視化プログラムです。

- `Client/` にはクライアント側の処理があります。
- `Server/` にはサーバ側の処理があります。
- `Shared/JsonKeys.h` では共有 JSON event/key 名を定義しています。プロトコル文字列を局所的に直書きせず、`Protocol::Key` と `Protocol::Events` を使ってください。
