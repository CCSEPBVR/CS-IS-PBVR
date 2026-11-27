# PBVR MCP（get_stateのみ）

ChatGPT Work（デスクトップアプリ内）またはClaude Desktop → MCP stdio → `server.py` → `command.txt` → Client →
`structured.log` → Python → 接続元アプリの構成です。PythonはPBVRを起動しません。
PBVRは利用者が手動起動します。通信フォルダーごとにPBVR・Python各1インスタンスを前提とします。

公開MCPツールは`get_state`のみです。状態取得では設定変更、データ要求、粒子再生成を行いません。
要求・応答はUTF-8のJSON Linesで、`request_id`により照合します。自動再送は行いません。

通常画面版のWindows・macOS Clientを対象とします。ローカル接続では接続元アプリとPBVRを同一端末で起動します。
s86のClaude Codeから接続する場合は、[s86からのリモート接続](#s86からのリモート接続)を参照してください。
WindowsはWin32 API、macOSはPOSIX APIでローカルファイルを読み書きします。
LinuxのPBVR側ファイル通信は未対応です。OpenXRも対象外です。
2026-11-10に、利用者がWindowsのChatGPTデスクトップアプリ内のWorkでSTDIO接続し、
`get_state`を取得できたことを確認しています。接続手順は3.1節を参照してください。
macOSの設定画面・接続、および第5節のその他の検証項目は未確認です。
今回のREADME更新ではビルド・プログラム実行・テストを行っていません。

## キャンセルと全データ削除

- 接続元アプリがキャンセルを通知すると、Pythonの要求管理処理はthreading.EventでPythonのファイル処理
  スレッドへ通知します。AnyIOのキャンセルから終了待ちを保護し、直接のasyncioキャンセルも
  繰り返し受け止め、ファイル処理スレッドの終了を確認してから要求ロックを解放します。
  ファイル処理スレッドはポーリング待ちから通知で起き、読み取りの区切りでも通知を確認します。
  通常は残りのタイムアウトいっぱいまで待ちません。実行中のOSファイルI/Oの即時中断は保証しません。
- ファイル処理スレッドが送信直前の確認でキャンセルを検知した場合、要求を追記しません。
  その確認とwrite/flushの間の通知は送信と競合し、要求が追記される可能性があります。
  送信済み要求は削除・取り消し・再送しません。接続元アプリのキャンセルはPBVRの操作取り消しでは
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
| server.py | stdioでget_stateだけを公開するFastMCPエントリーポイント |
| transport.py | 一意ID付き要求の追記、応答追尾、排他、期限管理 |
| remote_mcp/ | Windows側のWebSocket中継と依存定義 |
| remote_mcp.windows.example.json | s86のClaude Codeから接続するためのWindows側中継設定例 |
| MCPFileChannel.h/.cpp | ワーカースレッド専用のファイル読み書き・履歴整合性確認 |
| MCPProtocol.h/.cpp | 要求解析、対応コマンド判定、JSON応答生成 |
| MCPWorker.h/.cpp | ファイル監視タイマー、順次受付、GUIへの状態取得依頼 |
| MCPState.h/.cpp | GUIスレッドで適用済み状態をQt値型へコピー |
| ../Widgets/MCPWidget.h/.cpp/.ui | 最新10件の表示専用ドック |
| PROTOCOL.md | ファイル通信とget_stateのキー・型・取得元・単位 |
| chatgpt_mcp.windows.example.toml | Windows用のChatGPT MCP設定例 |
| chatgpt_mcp.macos.example.toml | macOS用のChatGPT MCP設定例 |

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

Python 3.11以上を使用します。パスは利用環境に実在するリポジトリの絶対パスへ置き換えます。
以下のコマンドは利用者が実施するための例です。

Windows（Developer Command Prompt for VS 2022）：

```cmd
cd /d "C:\path\to\CS-IS-PBVR\Client\MCP"
py -3.11 -m venv .venv
.venv\Scripts\python.exe -m pip install -r requirements.txt
```

macOS（ターミナル）：

```sh
cd '/Users/your-user/path/to/CS-IS-PBVR/Client/MCP'
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
```

Windowsの`py -3.11`はインストール済みの3.11以上を指定する起動方法に置き換えられます。
macOSは`python3`がPython 3.11以上を指す環境で使用してください。
環境の有効化は不要です。依存は移植元と同じMCP SDK 1.27.2（mcp）のみを直接指定し、
Pillowなどの画像処理用依存は追加しません。

## 2. 通信先とPBVRの手動起動

双方で環境変数`PBVR_CTRL_DIR`を必須とし、同じ既存の絶対ディレクトリを指定します。
既定値・作業ディレクトリからの推測・ディレクトリ自動作成はありません。
初期配置はリポジトリ内の`Client/MCP/ctrl`です（`ctrl/.gitkeep`でディレクトリを収録）。
PBVRを手動起動する環境にも、Pythonと同じ通信先を設定します。
WindowsではDeveloper Command Prompt for VS 2022から、次のようにPBVRを起動してください。

```cmd
set "PBVR_CTRL_DIR=C:\path\to\CS-IS-PBVR\Client\MCP\ctrl"
"C:\path\to\PBVR\pbvr_client.exe"
```

macOSでは次の設定を行ったターミナルから、実際のClient実行ファイルを直接起動してください。
`.app`の場合は内部の`Contents/MacOS`にある実行ファイルのパスを指定します。
Finderや既に起動済みのアプリには、このターミナルの環境変数は伝わりません。

```sh
export PBVR_CTRL_DIR='/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/ctrl'
'/path/to/PBVR.app/Contents/MacOS/pbvr_client'
```

**ChatGPT・ClaudeのMCP設定の環境変数はPythonにだけ適用され、別途手動起動したPBVRには伝わりません。**
フォルダーの変更時には両方を停止し、PBVRの起動環境と接続元アプリの設定を同時に変更してください。

PBVR起動時に存在しない通信ファイルは作成します。structured.logの既存内容は消去しません。
通信先未指定・相対パス・不存在・権限エラーでは、MCPドックへエラーを保持して受付を停止します。
PBVRの通常利用は継続できます。Tools → MCPで確認し、修正後にPBVRを再起動してください。

## 3. 接続元アプリの設定

ChatGPTとClaudeのどちらか一方だけでPBVR用Pythonサーバーを起動します。
実際のアプリ設定は利用者が変更してください。設定例で実設定ファイル全体を上書きしないでください。

### 3.1 ChatGPT Work（Windowsのデスクトップアプリ内で接続確認済み）

WindowsのChatGPTデスクトップアプリ内のWorkで、以下の手順によるSTDIO接続と
`get_state`の取得を利用者が確認済みです（2026-11-10）。
Web・クラウド環境への接続手順ではありません。

1. 第1節のPython環境を準備します。
2. Claude Code・Claude Desktopなど、ほかの接続元のPBVR用Pythonサーバーを停止します。
3. 第2節のとおり、`PBVR_CTRL_DIR`を設定した同じPowerShellからPBVRを手動起動します。
   起動済みのPBVRには環境変数が伝わらないため、必要に応じて起動し直します。
   Tools → MCPを開き、エラーが表示されていないことを確認します。データの読み込みは必須ではありません。
4. ChatGPTの「設定 → MCP servers → Add server」を開き、下表の項目を登録します。
   画面によっては「設定 → プラグイン → MCP → 追加 → カスタムMCPに接続する」です。

macOS用の設定値も併記しますが、画面名・配置および接続は未確認です。

| 項目 | Windows | macOS |
|---|---|---|
| 名前 | `pbvr` | `pbvr` |
| タイプ | `STDIO` | `STDIO` |
| 起動用コマンド | `C:/path/to/CS-IS-PBVR/Client/MCP/.venv/Scripts/python.exe` | `/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/.venv/bin/python` |
| 引数（1項目） | `C:/path/to/CS-IS-PBVR/Client/MCP/server.py` | `/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/server.py` |
| 環境変数 `PBVR_CTRL_DIR` | `C:/path/to/CS-IS-PBVR/Client/MCP/ctrl` | `/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/ctrl` |
| 環境変数 `PBVR_MCP_TIMEOUT_SECONDS` | `120` | `120` |
| 環境変数パススルー | 追加不要 | 追加不要 |
| 作業ディレクトリ | `C:/path/to/CS-IS-PBVR/Client/MCP` | `/Users/your-user/path/to/CS-IS-PBVR/Client/MCP` |

パスはすべて実在する絶対パスに置き換えます。UIの各欄には囲み引用符を付けず入力します。
仮想環境の有効化やシェルを経由する起動用コマンドは不要です。

細かな設定は、ホストの`config.toml`の`mcp_servers.pbvr`で指定します。
公式資料の既定の保存先はWindowsの`%USERPROFILE%/.codex/config.toml`、macOSの`~/.codex/config.toml`です。
`CODEX_HOME`やプロジェクト設定を使用している場合は、実際にアプリが参照する設定を確認してください。
これはClaudeのJSON設定とは別の形式です。

- Windows: [chatgpt_mcp.windows.example.toml](chatgpt_mcp.windows.example.toml)
- macOS: [chatgpt_mcp.macos.example.toml](chatgpt_mcp.macos.example.toml)

アプリを完全終了して設定をバックアップし、既存の`pbvr`表があれば編集、なければ表を追加します。
同名の表を重複追加しないでください。他のサーバーや設定は保持します。

5. UIで登録・保存した後にアプリを完全終了し、`config.toml`の既存の`[mcp_servers.pbvr]`内へ
   次の2行を追加します。同じキーがあれば値を更新します。

   ```toml
   tool_timeout_sec = 180
   enabled_tools = ["get_state"]
   ```

   `[mcp_servers.pbvr.env]`の項目として書かず、`[mcp_servers.pbvr]`内に配置してください。

UIとTOMLは同じ登録内容を編集する方法であり、別名のPBVRサーバーを二重登録しません。

Pythonの応答待ちは120秒、ホストのツール実行期限は180秒とする例です。
公式資料のホスト側既定値60秒ではPythonより先に期限切れになるため、明示的に設定します。
初期のログ読み取り・I/O・要求ロック待ちはPythonの応答待ち期限に含まれないため、180秒以内の完了保証はありません。
`get_state`は一度に1件ずつ呼び出してください。Python側の値を増やす際はホスト側も調整します。

6. PBVRを起動した状態でChatGPTを起動し、`pbvr`を有効にします。
   アプリが再起動を案内した場合はそれに従い、設定変更後にPythonサーバーを再起動します。
   PythonサーバーはChatGPTが起動するため、`server.py`を別途手動実行する必要はありません。
7. Workのローカルプロジェクトのチャットで`/mcp`を開き、`pbvr`の接続状況を確認します。
   「pbvrのget_stateを1回呼び出して、現在の状態を取得してください」と依頼します。
   ツール実行の承認画面が表示された場合は承認します。
8. 応答に`ok:true`と`result`が含まれることを確認します。PBVRのMCPドックにもコマンドと応答が表示されます。
   データ未登録時の`result.objects:[]`・`result.current_time_step:null`は正常です。

接続できない場合は、Python実行ファイル・`server.py`のパスと依存パッケージ、
双方の`PBVR_CTRL_DIR`、PBVRのMCPドックのエラーを確認してください。
`pbvr`が表示されない場合は、ローカルのチャットであること、サーバーの有効化と再起動を確認します。
Pythonの診断はstderrへ出力されます。

公式の接続方法・TOML・タイムアウトの説明：
[Model Context Protocol](https://learn.chatgpt.com/docs/extend/mcp)（2026-10-27参照）。
ローカルホストの設定が適用されるチャットが対象であり、Web・クラウド環境への接続設定は本手順の範囲外です。

### 3.2 Claude Desktop

設定例はOSごとに用意しています。

| OS | 設定例 | 仮想環境内のPython |
|---|---|---|
| Windows | [claude_desktop_config.example.json](claude_desktop_config.example.json) | `.venv/Scripts/python.exe` |
| macOS | [claude_desktop_config.macos.example.json](claude_desktop_config.macos.example.json) | `.venv/bin/python` |
| Linux | [claude_desktop_config.linux.example.json](claude_desktop_config.linux.example.json) | `.venv/bin/python` |

macOS形式のパスで記載すると、設定内容は次のようになります。

```json
{
  "mcpServers": {
    "pbvr": {
      "command": "/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/.venv/bin/python",
      "args": ["/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/server.py"],
      "env": {
        "PBVR_CTRL_DIR": "/Users/your-user/path/to/CS-IS-PBVR/Client/MCP/ctrl",
        "PBVR_MCP_TIMEOUT_SECONDS": "120"
      }
    }
  }
}
```

`command`、`args`、`PBVR_CTRL_DIR`をすべて利用環境に実在する絶対パスへ置き換えてください。
`~`や`$HOME`、`%USERPROFILE%`の自動展開には依存しません。

Linuxの例はパス形式の参考として保持しています。PBVRのMCP通信およびClaude DesktopのLinux対応を示しません。

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

### 3.3 ChatGPT・Claudeの切り替え

1. 実行中の要求の終了を待ちます。応答待ちがあれば完了またはキャンセル後の終了を確認します。
2. 使用中の接続元アプリを完全終了し、そのPBVR用Pythonサーバーが停止したことを確認します。
3. ChatGPT側は`pbvr`を無効、Claude側はPBVR項目を外すなど、使わない側で自動起動しない設定にします。
   設定変更前にはバックアップし、他のMCPサーバー設定は保持してください。
4. 切り替え先だけを有効にし、同じ`PBVR_CTRL_DIR`で接続します。

PBVRは、通信先の変更やファイル異常がなければ起動したままで構いません。
通常の切り替えでは通信ファイルを削除・編集しません。
別名のサーバー登録や別チャット・別クライアントによる二重起動にも注意してください。
複数Pythonプロセスの競合を防ぐプロセス間ロックは実装していません。

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
復旧には両方を停止し、ファイルを整理してPBVR→使用する接続元アプリの順に起動します。

自動ローテーションはありません。保存量と表示10件は別です。ログを整理する場合は：

1. 使用中の接続元アプリを完全終了し、当該Pythonサーバーが停止したことを確認する。
2. PBVRを終了する。
3. command.txtとstructured.logを必要に応じバックアップし、削除または空ファイルにする。
   書きかけの最終行もこの停止中に整理する。
4. PBVRを手動起動してから使用する接続元アプリを起動する。

履歴の書き換えを検出するため、消費済みファイルのSHA-256を再確認します。
ディスク読み取り量は履歴量に比例するため、大きくなる前に停止して整理してください。
ネットワーク共有、敵対的な同時書き換え、電源断時の永続化保証は対象外です。
Windowsが開いているファイルの削除・置換を拒否する場合、失敗した操作はファイル変更になりません。

## 5. 利用者による実機確認（Windows・macOSそれぞれ）

WindowsのChatGPT WorkでのSTDIO接続と`get_state`取得は利用者が確認済みです（3.1節）。
以下は追加の検証項目であり、すべてを確認済みとするものではありません。
利用者が対象OSのClientをビルドした後、未確認の項目を順に確認してください。

1. 両プロセスに同じ既存の絶対通信パスを設定し、PBVR→接続元アプリの順で起動する。
   macOSではMCP受付がWindows専用エラーにならないこと、日本語・空白を含むパスでも接続できることを確認する。
2. 公開ツールが`get_state`だけであることを確認し、明示的に呼び出す。
   `ok:true`、空データでは`objects:[]`・`current_time_step:null`を確認する。
   データ登録後もGUIの適用済み状態と応答を照合し、取得が設定変更やデータ要求を起こさないことを確認する。
3. ClaudeとChatGPTを前節どおり切り替え、どちらからも状態取得できることを確認する。
4. PBVRを停止して状態取得を要求し、Pythonの応答待ち期限で失敗し、自動再送しないことを確認する。
   PBVRを再起動してから新しい要求を1回送り、古い応答と混同しないことを確認する。
5. 応答待ちをキャンセルし、次の要求が開始できること、要求が自動再送されないことを確認する。
6. 専用の検証用通信フォルダーで、監視中の削除・置換・短縮・既読部分変更を各ファイルへ個別に行う。
   command.txtは次の監視時、structured.logは次の応答書き込み時に異常を検出し、受付を停止することを確認する。
   structured.logへの別ライターの追記、起動時の未完末尾、書き込み権限・同期エラーも確認する。
   OSが操作を拒否した場合は、変更が成功したケースとは区別する。各ケース後は両プロセスを停止して復旧する。

macOSもWindowsと同様にファイルID・消費済み部分のハッシュで履歴を検証します。
macOSではデバイス番号とinodeで同一性を判定し、書き込み後に`fsync`します。
短い書き込みは未書き込み部分を続け、open/read/write/fsyncの中断（EINTR）は再試行します。
要求や応答全体の再送はしません。seekやファイル情報取得などの失敗はエラーとして扱います。
途中の失敗は受付停止として扱い、部分行を成功応答として表示しません。

## 未対応事項

操作コマンド、画像取得、転送関数詳細、OpenXR、Windows・macOS以外のPBVR側ファイル通信、
複数インスタンス、自動ローテーションには対応していません。
データ読み込み中の色・不透明度Applyによる設定と描画データの混在は、別途対応が必要です。
将来の操作コマンドはMCPProtocolの受付とGUI処理、Python側ツールを追加する形で拡張できます。

## s86からのリモート接続

WebSocket中継を使います。Claude Codeはs86側の中継にstdioで接続し、Windows側の中継がSSHローカルポート転送を経由して接続します。Windows側の中継は、このリポジトリの`server.py`をstdioの子プロセスとして起動します。通信ファイルはWindows上のPBVRと`server.py`の間で使います。

### Windows側の準備

第1節のMCP用Python環境を準備します。Windows側の中継コードと依存定義は、このリポジトリの`Client/MCP/remote_mcp`にあります。Developer Command Prompt for VS 2022で`Client\MCP`に移動し、依存ライブラリを追加します。`remote_mcp.windows.json`を既に作成している場合は、その設定を使用できます。

```cmd
cd /d "C:\path\to\CS-IS-PBVR\Client\MCP"
.venv\Scripts\python.exe -m pip install -r remote_mcp\requirements.txt
if not exist remote_mcp.windows.json copy /-Y remote_mcp.windows.example.json remote_mcp.windows.json
```

`remote_mcp.windows.json`の`C:/path/to/CS-IS-PBVR`を実際のリポジトリの絶対パスに置き換えます。`mcp_server.env.PBVR_CTRL_DIR`を第2節でPBVR Clientに指定する値と揃え、PBVR Clientを起動します。接続確認に使うデータを開き、同じ通信ディレクトリを使用するローカルのMCPサーバーは終了しておきます。

### 接続と確認

1. s86のログインノードで次を実行します。Phase 4と同じ中継ソースと依存ライブラリを配備済みの環境を使います。`~/claude-sandbox/remote-mcp`はPhase 4の配備先です。別の場所に配備している場合は`run.sh`のパスを置き換えます。

   ```sh
   bash ~/claude-sandbox/remote-mcp/run.sh --remote-mcp ~/work
   ```

2. `Remote MCP endpoint`が表示されたら、Windowsの別のDeveloper Command Prompt for VS 2022で`ComputeHost`と`ComputePort`を表示値に置き換えてSSH転送を開始し、その端末を開いたままにします。

   ```cmd
   ssh -N -o ExitOnForwardFailure=yes -L 127.0.0.1:28080:ComputeHost:ComputePort s86
   ```

3. s86側の待機プロンプトでEnterを押し、Windows側の準備で使った端末ですぐに中継を起動します。

   ```cmd
   .venv\Scripts\python.exe remote_mcp\websocket_client.py --config remote_mcp.windows.json
   ```

4. Claude Codeの`/mcp`で`pbvr-remote`の接続を確認します。`get_state`を呼び出し、`ok: true`と`result.objects`の内容がWindows側で開いたデータと一致することを確認します。
5. Claude CodeとWindows側のSSH転送を終了し、中継とその子プロセスのMCPサーバーが終了することを確認します。
