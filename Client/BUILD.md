# Client ビルド手順

このプロジェクトは Qt の `.pro` ファイルから qmake で Makefile を生成してビルドします。

## 前提条件

- Qt qmake が利用できること。
  - この環境では `/home/nsakamoto/qt-everywhere-src-6.11.0/install/bin/qmake`
- GNU make と g++ が利用できること。
- KVS が `KVS/Install` にインストールされていること。
  - `Client/SETTINGS.pri` は `../KVS/Install/kvs.conf` を参照します。
- macOS で後述の動画録画対象テストを有効にする場合、Qt 6.8 以降と Qt Multimedia が利用できること。現在の使用環境は Qt 6.11.0 です。導入方法と対象テストは「macOS のテスト動画録画」を参照してください。

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

## macOS のテスト動画録画

### 対象テストと録画方式

次の3テストは、macOS で `screencapture` によるデスクトップ収録から、アプリ内の描画を Qt Multimedia で動画化する方式へ変更しました。

| テスト | `Tests/TestsConfig.pri` の有効化設定 | 動画ファイル |
| --- | --- | --- |
| PlayBackControlToolBarTest | `TEST_ENABLE_PLAYBACKCONTROLTOOLBAR = 1` | `PlayBackControlToolBarTest.mp4` |
| TimeStepControlToolBarTest | `TEST_ENABLE_TIMESTEPCONTROLTOOLBAR = 1` | `TimeStepControlToolBarTest.mp4` |
| AnimationControlTest | `TEST_ENABLE_ANIMATIONCONTROL = 1` | `AnimationControlTest.mp4` |

共通処理は `Tests/TestWindowVideoRecorder.h` と `Tests/TestWindowVideoRecorder.cpp` にあります。メインウィンドウの UI と OpenGL フレームバッファーを取得し、所属する表示中の Qt ダイアログを重ねて、10fps・音声なしの MP4/H.264 として保存します。ダイアログが録画範囲からはみ出す場合は、録画画像内で位置・サイズを補正します。OS のデスクトップや他アプリ、ネイティブダイアログは録画対象に含みません。テストの共通起動処理ではネイティブダイアログを無効にしています。

この方式は OS の画面収録 API を使用しないため、対象3テストの録画に macOS の画面収録許可は不要です。Windows・Linux の録画処理や、他のテストの録画方式はこの変更の対象外です。

### Qt Multimedia の追加とビルド設定

`QVideoFrameInput` を使用するため、Qt 6.8 以降が必要です。対象3テストのいずれかを macOS で有効にすると、`Tests/Tests.pro` が `QT += multimedia` と共通録画クラスを追加します。

1. Qt Maintenance Tool を起動し、「コンポーネントの追加・削除」で、使用する Qt バージョン・macOS キットの Qt Multimedia を追加します。Qt Creator で選択しているキットと、追加先の Qt が一致していることを確認してください。
2. Qt Multimedia の FFmpeg バックエンドと、MP4/H.264 のエンコードに必要な実行時ライブラリ・プラグインを利用できる状態にします。公式配布の Qt を使用する場合も、別の Mac にテストアプリを配布するときはこれらの配置を確認してください。
3. `Tests/TestsConfig.pri` で実行するテストを有効にします。
4. Qt Creator で qmake を再実行してから再ビルドします。コマンドラインの場合も、使用するキットの qmake で Makefile を更新してからビルドしてください。
5. 生成された `pbvr_client_qtest.app` の実際の出力先を確認し、テストを実行します。

ここで必要な FFmpeg バックエンドは Qt Multimedia のプラグインです。外部の `ffmpeg` 実行ファイルのインストールや `PBVR_FFMPEG_EXECUTABLE` の指定は、macOS の対象3テストでは不要です。

`QT_MEDIA_BACKEND` が未設定または空の場合、テストが `ffmpeg` を指定します。実行環境に別の値を指定している場合は、設定を解除するか `QT_MEDIA_BACKEND=ffmpeg` に変更してください。別のバックエンドが明示されている場合、テストは録画失敗として理由を記録します。

### 動画・レポートの確認

標準の保存先は、リポジトリ直下から見て次の構成です。`yyyyMMdd` はテスト実行日、`テスト名` は上表の名前です。

```text
Client/output-tests/yyyyMMdd/テスト名/
    テスト名.mp4
    TestResult.md
```

`PBVR_TEST_OUTPUT_DIR` を指定した場合は、そのディレクトリに直接保存します。複数テストで同じ保存先を指定すると `TestResult.md` が上書きされるため、レポートを残す場合はテストごとに保存先を分けて実行してください。AnimationControlTest のキーフレームファイル `Test.anim` も同じ出力ディレクトリに保存します。

`TestResult.md` にはテスト結果、動画の埋め込みと相対リンク、録画の診断情報を記録します。埋め込み再生に対応しない Markdown ビューアーでは、動画へのリンクから開いてください。録画失敗時は動画を埋め込まず、確認できた失敗理由を記載します。

確認時は、今回の QtTest ログの PASS／FAIL と、動画・レポートの更新時刻を照合してください。ダイアログの表示を確認する場合は、動画と、診断情報の「合成対象ダイアログ」「ダイアログを含むフレーム取得数」を確認します。AnimationControlTest の `.anim` 保存・読み込みでは、表示直後とファイル名入力後に各1秒の確認時間を設けています。

問題が起きた場合は、次の情報を確認します。

- qmake が `Unknown module(s) in QT: multimedia` で失敗する場合：使用中のキットに Qt Multimedia を追加できているか。
- 録画を開始できない場合：レポートと QtTest ログのエラー、`QT_MEDIA_BACKEND`、FFmpeg バックエンドと MP4/H.264 の対応状況。
- 動画の内容や保存に問題がある場合：診断情報の解像度・送信フレーム数・取得見送り回数・動画時間・保存完了状態。

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

前述の新方式を使用する3テストの録画には、画面収録許可を維持するための Apple Development 署名は不要です。以下の画面収録許可に関する説明は、OS の画面収録 API を使用する他のテストや機能に適用されます。テストアプリの署名処理自体は引き続き実施します。

macOS では `Tests.pro` のリンク後処理が、KVS リソースのコピー後に `pbvr_client_qtest.app` を自動署名します。署名前に Font と Shader を `Contents/Resources` に移し、従来の `Contents/MacOS` 配下には参照用シンボリックリンクを置きます。有効な Apple Development 証明書を使用すると、再ビルド後も画面収録の許可を維持できます。利用できる証明書は、通常のターミナルで `security find-identity -v -p codesigning` を実行して確認できます。

macOS の Preference 設定はユーザーの AppConfigLocation に保存します。旧 `Contents/MacOS/config.ini` があれば署名前に `Contents/Resources/config.ini` へ移し、ユーザー設定が未作成の場合は初回起動時に内容を引き継ぎます。

有効な Apple Development 署名 ID が1つなら自動選択します。複数ある場合は、Qt Creator のビルド環境に `PBVR_TEST_CODESIGN_IDENTITY` を設定し、使用する署名 ID の40桁のハッシュを指定できます。署名 ID が0件または複数件で未指定の場合、指定した ID が無効な場合、Apple Development 署名・検証が失敗した場合は、`WARNING` を表示してアドホック署名を試します。アドホック署名・検証も失敗した場合は `WARNING` を表示してビルドを続行しますが、アプリを起動できない可能性があります。アドホック署名では、再ビルド後に画面収録の許可を再要求される場合があります。KVS リソースのコピー・移動など、署名以外のエラーはビルドエラーとして扱います。`.pro` を変更した後は qmake を再実行して Makefile を更新してください。このリンク後処理は qmake の Makefile ビルド向けです。

## 注意点

- KVS ランタイムリソースのコピー方法を変更する場合は、各 `.pro` ファイルを個別に編集せず、`Client/KvsRuntimeResources.pri` を更新してください。
- WSLg の Wayland/Xwayland 環境では `QScreen::grabWindow(0)` による全画面スクリーンショットが null または黒画像になる場合があります。スクリーンショットが必要なテストは Xvfb 上で実行してください。
