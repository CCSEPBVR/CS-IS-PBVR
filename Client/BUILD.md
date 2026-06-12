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

## 注意点

- KVS ランタイムリソースのコピー方法を変更する場合は、各 `.pro` ファイルを個別に編集せず、`Client/KvsRuntimeResources.pri` を更新してください。
- WSLg の Wayland/Xwayland 環境では `QScreen::grabWindow(0)` による全画面スクリーンショットが null または黒画像になる場合があります。スクリーンショットが必要なテストは Xvfb 上で実行してください。
