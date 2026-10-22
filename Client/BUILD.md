# Client ビルド手順

このプロジェクトは Qt の `.pro` ファイルから qmake で Makefile を生成してビルドします。

## 前提条件

- Qt qmake が利用できること。
  - この環境では `/home/nsakamoto/qt-everywhere-src-6.11.0/install/bin/qmake`
- GNU make と g++ が利用できること。
- KVS が `KVS/Install` にインストールされていること。
  - `Client/SETTINGS.pri` は `../KVS/Install/kvs.conf` を参照します。

## クライアントとテストをまとめてビルドする

既存の `Client/build` ディレクトリで次のコマンドを実行します。

```sh
cd /home/nsakamoto/CS-IS-PBVR/Client/build
/home/nsakamoto/qt-everywhere-src-6.11.0/install/bin/qmake -o Makefile ../pbvr_client.pro CONFIG+=release CONFIG+=c++17
make -j4
```

この手順では `Client/pbvr_client.pro` に定義されている次のサブプロジェクトをビルドします。

- `Utils`
- `FunctionParser`
- `ExtendedKVS`
- `ExtendedQT`
- `Widgets`
- `App`
- `Tests`

## テストターゲットだけをビルドする

クライアント共通ライブラリと `App` ターゲットがすでにビルド済みの場合は、テストプロジェクトだけを直接ビルドできます。

```sh
cd /home/nsakamoto/CS-IS-PBVR/Client/build/Tests
/home/nsakamoto/qt-everywhere-src-6.11.0/install/bin/qmake -o Makefile ../../Tests/Tests.pro CONFIG+=release CONFIG+=c++17
make -j4
```

## テストを実行する

`pbvr_client_qtest` は `Client/build/Tests` ディレクトリから実行します。

```sh
cd /home/nsakamoto/CS-IS-PBVR/Client/build/Tests
./pbvr_client_qtest
```

WSL/WSLg 上で目視確認しながら実行する場合は、Qt の platform plugin を X11/xcb に指定します。
WSLg では全画面スクリーンショットが黒くなる場合があるため、目視確認用の実行ではスクリーンショットを無効化します。

```sh
cd /home/nsakamoto/CS-IS-PBVR/Client/build/Tests
PBVR_DISABLE_SCREENSHOTS=1 QT_QPA_PLATFORM=xcb ./pbvr_client_qtest
```

スクリーンショットを出力するテスト実行では、WSLg の画面ではなく Xvfb の仮想 X11 ディスプレイ上で実行します。

```sh
cd /home/nsakamoto/CS-IS-PBVR/Client/build/Tests
xvfb-run -a \
  -s "-screen 0 1920x1080x24 +extension GLX +render -noreset" \
  env QT_QPA_PLATFORM=xcb ./pbvr_client_qtest
```

`PBVR_DISABLE_SCREENSHOTS` は `1`, `true`, `yes`, `on` のいずれかを指定すると有効になります。

## ビルド成果物

ビルドに成功すると、主な成果物は次の場所に生成されます。

- `Client/build/App/pbvr_client`
- `Client/build/Tests/pbvr_client_qtest`

KVS のランタイム用シェーダーとフォントリソースは、ビルド時に次の場所へ自動コピーされます。

- `Client/build/App/Font`
- `Client/build/App/Shader`
- `Client/build/Tests/Font`
- `Client/build/Tests/Shader`

このコピー処理は `Client/KvsRuntimeResources.pri` にまとめて定義されており、`Client/App/App.pro` と `Client/Tests/Tests.pro` の両方から読み込まれます。

## macOS のテストアプリ署名

macOS では `Tests.pro` のリンク後処理が、KVS リソースのコピー後に `pbvr_client_qtest.app` を自動署名します。署名前に Font と Shader を `Contents/Resources` に移し、従来の `Contents/MacOS` 配下には参照用シンボリックリンクを置きます。有効な Apple Development 証明書を使用すると、再ビルド後も画面収録の許可を維持できます。利用できる証明書は、通常のターミナルで `security find-identity -v -p codesigning` を実行して確認できます。

macOS の Preference 設定はユーザーの AppConfigLocation に保存します。旧 `Contents/MacOS/config.ini` があれば署名前に `Contents/Resources/config.ini` へ移し、ユーザー設定が未作成の場合は初回起動時に内容を引き継ぎます。

有効な Apple Development 署名 ID が1つなら自動選択します。複数ある場合は、Qt Creator のビルド環境に `PBVR_TEST_CODESIGN_IDENTITY` を設定し、使用する署名 ID の40桁のハッシュを指定できます。署名 ID が0件または複数件で未指定の場合、指定した ID が無効な場合、Apple Development 署名・検証が失敗した場合は、`WARNING` を表示してアドホック署名を試します。アドホック署名・検証も失敗した場合は `WARNING` を表示してビルドを続行しますが、アプリを起動できない可能性があります。アドホック署名では、再ビルド後に画面収録の許可を再要求される場合があります。KVS リソースのコピー・移動など、署名以外のエラーはビルドエラーとして扱います。`.pro` を変更した後は qmake を再実行して Makefile を更新してください。このリンク後処理は qmake の Makefile ビルド向けです。

## 注意点

- KVS ランタイムリソースのコピー方法を変更する場合は、各 `.pro` ファイルを個別に編集せず、`Client/KvsRuntimeResources.pri` を更新してください。
- WSLg の Wayland/Xwayland 環境では `QScreen::grabWindow(0)` による全画面スクリーンショットが null または黒画像になる場合があります。スクリーンショットが必要なテストは Xvfb 上で実行してください。
