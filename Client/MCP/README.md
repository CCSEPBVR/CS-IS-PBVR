# PBVR MCP（get_stateのみ）

Claude Desktop → MCP stdio → `server.py` → `command.txt` → Client →
`structured.log` → Python → Claude Desktop の構成です。PythonはPBVRを起動しません。
PBVRは利用者が手動起動します。通信フォルダーごとにPBVR・Python各1インスタンスを前提とします。

公開MCPツールは`get_state`のみです。状態取得では設定変更、データ要求、粒子再生成を行いません。
要求・応答はUTF-8のJSON Linesで、`request_id`により照合します。自動再送は行いません。

現在のPBVR側ファイル通信はWin32 API専用で、通常画面版のWindows Clientが対象です。
以下のパス・シェル記法はLinuxの一般的な形式で統一しています。
Linux上でPBVRのMCP通信を利用するには、PBVR側のファイル通信実装の移植が必要です。

## キャンセルと全データ削除

- Claudeがキャンセルすると、Pythonの要求管理処理はthreading.EventでPythonのファイル処理
  スレッドへ通知します。AnyIOのキャンセルから終了待ちを保護し、直接のasyncioキャンセルも
  繰り返し受け止め、ファイル処理スレッドの終了を確認してから要求ロックを解放します。
  ファイル処理スレッドはポーリング待ちから通知で起き、読み取りの区切りでも通知を確認します。
  通常は残りのタイムアウトいっぱいまで待ちません。実行中のOSファイルI/Oの即時中断は保証しません。
- ファイル処理スレッドが送信直前の確認でキャンセルを検知した場合、要求を追記しません。
  その確認とwrite/flushの間の通知は送信と競合し、要求が追記される可能性があります。
  送信済み要求は削除・取り消し・再送しません。ClaudeのキャンセルはPBVRの操作取り消しでは
  ありません。将来、色変更等を追加した場合も、送信済み操作はキャンセル後に実行され得ます。
  今回、取り消しプロトコルや追加の公開ツールは実装していません。遅延応答はrequest_idで区別します。
- Object Editorの最後の行が削除されると、PBVRのGUI処理が読み込み世代を無効化し、
  TimeStepControlToolBarの保持ステップを値なしに戻し、Current Time Step表示を空にします。
  再生タイマーも停止するため、別データを登録しただけでは自動取得しません。
- PBVRのデータ読み込みスレッドからの結果には、開始時に捕捉した要求通番をGUI処理で照合します。
  全削除前の結果は適用せず、新規に生成されたオブジェクトを解放します。接続リセット前の
  コールバックも通番で区別します。全体完了は有効な世代の読み込みに限って記録します。
- ハイブリッドでは従来どおりローカル・サーバー両方の完了を待ちます。サーバーの既存バイナリ
  応答には要求IDがないため、無効化後も古い要求の完了フラグを保持し、その応答を回収するまで
  次の読み込み要求を開始しません。同じステップ番号を再指定しても古い完了と混同しません。
  サーバー応答が来なければ待機は継続します。接続を切り直すと既存のreset経路で解放されます。
  get_stateの取得自体はデータ読み込み完了を待ちません。

## ファイル構成

| ファイル | 役割 |
|---|---|
| server.py | get_stateだけを公開するFastMCP stdioエントリーポイント |
| transport.py | 一意ID付き要求の追記、応答追尾、排他、期限管理 |
| MCPFileChannel.h/.cpp | ワーカースレッド専用のファイル読み書き・履歴整合性確認 |
| MCPProtocol.h/.cpp | 要求解析、対応コマンド判定、JSON応答生成 |
| MCPWorker.h/.cpp | ファイル監視タイマー、順次受付、GUIへの状態取得依頼 |
| MCPState.h/.cpp | GUIスレッドで適用済み状態をQt値型へコピー |
| ../Widgets/MCPWidget.h/.cpp/.ui | 最新10件の表示専用ドック |
| PROTOCOL.md | ファイル通信とget_stateのキー・型・取得元・単位 |

既存ファイルの変更箇所：

| ファイル | 変更 |
|---|---|
| ../App/MainWindow.h/.cpp | ドック・メニュー登録、起動時の監視開始、GUIコピーの接続、終了時停止 |
| ../Widgets/Widgets.pro | MCPWidgetとMCPのC++ソース・ヘッダー・UIの登録 |
| ../Widgets/ObjectEditor.h/.cpp | オブジェクト状態取得、登録済みステップ保持、読込結果のGUI反映 |
| ../Widgets/TimeStepControlToolBar.h/.cpp | 全体の最終要求完了ステップ保持 |
| ../Utils/Worker.h | モデルの直接更新を値の受け渡しへ変更 |

ほかにrequirements.txt、OS別の設定例、MCP専用.gitignore、ctrl/.gitkeepがあります。
`.gitkeep`はGitに空の通信ディレクトリを収録するためのファイルで、通信には使用しません。

## 状態取得とスレッドの役割

- Pythonのファイル処理スレッドとPBVRのMCPワーカーがファイル通信を担当します。
- PBVRのGUI処理が適用済み状態をコピーし、Qtの値型でMCPワーカーへ渡します。
  通信スレッドからGUI・描画オブジェクトを直接参照しません。
- ObjectEditorはシーンへの登録・置き換え時に各オブジェクトのステップを記録します。
  全体の現在ステップはTimeStepControlToolBarが最後に完了した要求の値を保持します。
  Next入力や処理中の要求とは区別します。
- PBVRのデータ読み込みスレッドは入力のコピーから読み込み結果を生成し、
  GUI処理が要求の有効性を判定してからモデルへ反映します。

## 1. 専用Python環境の準備（利用者が実施）

Python 3.11以上を使用します。以下はLinuxでのPython環境の準備例です。
`/home/user/src/CS-IS-PBVR`は利用環境のリポジトリの絶対パスへ置き換えてください。
Python環境の準備だけでは、Linuxで未対応のPBVR側ファイル通信は利用可能になりません。

```sh
cd /home/user/src/CS-IS-PBVR/Client/MCP
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
```

`python3`がPython 3.11以上を指す環境で使用してください。
環境の有効化は不要です。依存は移植元と同じMCP SDK 1.27.2（mcp）のみを直接指定し、
Pillowなどの画像処理用依存は追加しません。

## 2. 通信先とPBVRの手動起動

双方で環境変数`PBVR_CTRL_DIR`を必須とし、同じ既存の絶対ディレクトリを指定します。
既定値・作業ディレクトリからの推測・ディレクトリ自動作成はありません。
初期配置は次です（`ctrl/.gitkeep`でディレクトリを収録）。

```text
/home/user/src/CS-IS-PBVR/Client/MCP/ctrl
```

PBVRを手動起動する環境にも、Pythonと同じ通信先を設定します。Linuxのシェル記法では次の形式です。
現在のWindows Clientで使用する場合は、パスと環境変数の設定記法をWindows環境に合わせてください。

```sh
export PBVR_CTRL_DIR=/home/user/src/CS-IS-PBVR/Client/MCP/ctrl
```

**Claude Desktop設定のenvはPythonにだけ適用され、別途手動起動したPBVRには伝わりません。**
フォルダーの変更時には両方を停止し、PBVRの起動環境とClaudeのenvを同時に変更してください。

PBVR起動時に存在しない通信ファイルは作成します。structured.logの既存内容は消去しません。
通信先未指定・相対パス・不存在・権限エラーでは、MCPドックへエラーを保持して受付を停止します。
PBVRの通常利用は継続できます。Tools → MCPで確認し、修正後にPBVRを再起動してください。

## 3. Python環境の準備完了後にClaude Desktop設定を切り替える

設定例はOSごとに用意しています。

| OS | 設定例 | 仮想環境内のPython |
|---|---|---|
| Windows | [claude_desktop_config.example.json](claude_desktop_config.example.json) | `.venv/Scripts/python.exe` |
| macOS | [claude_desktop_config.macos.example.json](claude_desktop_config.macos.example.json) | `.venv/bin/python` |
| Linux | [claude_desktop_config.linux.example.json](claude_desktop_config.linux.example.json) | `.venv/bin/python` |

Linux形式のパスで記載すると、設定内容は次のようになります。

```json
{
  "mcpServers": {
    "pbvr": {
      "command": "/home/user/src/CS-IS-PBVR/Client/MCP/.venv/bin/python",
      "args": ["/home/user/src/CS-IS-PBVR/Client/MCP/server.py"],
      "env": {
        "PBVR_CTRL_DIR": "/home/user/src/CS-IS-PBVR/Client/MCP/ctrl",
        "PBVR_MCP_TIMEOUT_SECONDS": "120"
      }
    }
  }
}
```

`command`、`args`、`PBVR_CTRL_DIR`をすべて利用環境に実在する絶対パスへ置き換えてください。
`~`や`$HOME`、`%USERPROFILE%`の自動展開には依存しません。

**macOS・Linuxの例はパス形式を示すもので、PBVRのMCP通信の対応を意味しません。**
現在の`MCPFileChannel`はWin32 API専用で、macOS・Linuxでは通信を開始できません。
これらのOSでの利用にはPBVR側のファイル通信実装の移植が別途必要です。
Linuxの例は対応するstdio MCPホストでの設定用であり、Claude DesktopのLinux対応を示しません。

1. Python環境の準備を完了してからClaude Desktopを完全終了します。
2. 実設定ファイルを日時付きの別名へコピーしてバックアップします。
3. 実設定をエディターで開き、`mcpServers.pbvr`だけを
   上表から選んだ設定例のpbvr項目へ置き換えます。
   他のMCPサーバー・トップレベル項目・pbvrに必要な独自設定は保持してください。
   設定例のファイル全体で実設定を上書きしないでください。
4. `command`をOSに応じた仮想環境内Pythonの絶対パス、`args`を移植先`server.py`の絶対パス、
   `env.PBVR_CTRL_DIR`をPBVRと同一の絶対パスにします。
   `PBVR_VISUALIZER`などプロトタイプの自動起動用指定は不要です。
5. PBVRを前項の環境で手動起動してからClaude Desktopを起動します。

タイムアウトはPython側の`PBVR_MCP_TIMEOUT_SECONDS`（正の有限数、秒、既定120）です。
変更後はClaude DesktopのPythonサーバーを再起動します。MCP診断はstderrへ出力し、
stdoutにはstdioプロトコル以外を出力しません。

## 4. 運用

GUIが重い処理中であれば状態取得もGUIの順番を待ちます。タイムアウトの原因は特定しません。
自動再送はなく、遅れて届いた応答はrequest_idが異なる次の要求に採用しません。

MCPドックは起動時非表示で、未表示・閉じた状態でも監視・ログ更新を続けます。
コマンド・書き込み成功応答・エラーを合計最新10件保持し、JSON応答全体が1件です。
閉じる操作は非表示にするだけです。PBVR終了時は監視タイマーとファイルを閉じ、
スレッドの終了を待ってから描画オブジェクトを破棄します。
接続状態表示・ハートビート・接続監視は設けていません。

ファイル異常の検出後はPBVRのMCP受付を停止し、過去の要求を読み直しません。
Pythonの待機中のログ異常もその要求を失敗させ、自動再送しません。
復旧には両方を停止し、ファイルを整理してPBVR→Claude Desktopの順に起動します。

自動ローテーションはありません。保存量と表示10件は別です。ログを整理する場合は：

1. Claude Desktopを完全終了し、当該Pythonサーバーが停止したことを確認する。
2. PBVRを終了する。
3. command.txtとstructured.logを必要に応じバックアップし、削除または空ファイルにする。
   書きかけの最終行もこの停止中に整理する。
4. PBVRを手動起動してからClaude Desktopを起動する。

履歴の書き換えを検出するため、消費済みファイルのSHA-256を再確認します。
ディスク読み取り量は履歴量に比例するため、大きくなる前に停止して整理してください。
ネットワーク共有、敵対的な同時書き換え、電源断時の永続化保証は対象外です。
Windowsが開いているファイルの削除・置換を拒否する場合、失敗した操作はファイル変更になりません。

## 未対応事項

操作コマンド、画像取得、転送関数詳細、OpenXR、Windows以外のPBVR側ファイル通信、
複数インスタンス、自動ローテーションには対応していません。
データ読み込み中の色・不透明度Applyによる設定と描画データの混在は、別途対応が必要です。
将来の操作コマンドはMCPProtocolの受付とGUI処理、Python側ツールを追加する形で拡張できます。
