# スパコンからのSFTP中継

スパコン側でClaude CodeとPython MCPサーバーを動かし、WindowsまたはmacOSのクライアントPCで中継と通常画面版のPBVRを動かします。Claude CodeはPythonサーバーにstdioで接続し、中継はSSHログインノードにSFTPで接続します。公開ツールは`get_state`です。

## 準備

- スパコンに専用の共有ディレクトリを作成します。SFTPログイン先とClaude Codeの実行環境から同じ実体を読み書きできる必要があります。両方でのパス名は異なっていても構いません。
- この共有ディレクトリにつきPython MCPサーバーと中継を各1プロセス起動します。SFTPサーバーは`posix-rename@openssh.com`に対応している必要があります。ハートビートの判定にはファイルの更新時刻を使うため、PC・スパコン・ファイルサーバーで時刻を合わせてください。
- PCにはPython 3.10以上が必要です。SSHログイン先のホスト鍵を確認し、`known_hosts`に登録します。中継は登録済みのホスト鍵を照合します。
- `ssh_bridge/config.example.json`を`ssh_bridge/config.json`にコピーし、ログイン先、ユーザー名、鍵、`known_hosts`と`remote_ctrl_dir`を設定します。`remote_ctrl_dir`はSFTP側から見た共有領域の絶対パスです。`local_ctrl_dir`の相対パスは設定ファイルの位置を基準とします。
- SSH鍵エージェントを使う場合は、`prompt_passphrase`を`false`にし、`key_filename`を省略できます。鍵のパスフレーズは対話入力し、中継のメモリに保持します。

## PC側の起動

PC側で`Client/MCP`を作業ディレクトリにして、OSに合ったコマンドを使います。

Windows PowerShell

```powershell
py -3 -m venv ssh_bridge\.venv
.\ssh_bridge\.venv\Scripts\python.exe -m pip install -r ssh_bridge/requirements.txt
.\ssh_bridge\.venv\Scripts\python.exe -m ssh_bridge.bridge --config ssh_bridge/config.json
```

macOS

```sh
python3 -m venv ssh_bridge/.venv
./ssh_bridge/.venv/bin/python -m pip install -r ssh_bridge/requirements.txt
./ssh_bridge/.venv/bin/python -m ssh_bridge.bridge --config ssh_bridge/config.json
```

1. 中継に表示されたセッション用の絶対パスを、PBVRを起動する側の端末で`PBVR_CTRL_DIR`に設定します。Windows PowerShellでは`$env:PBVR_CTRL_DIR = '表示された絶対パス'`、macOSでは`export PBVR_CTRL_DIR='表示された絶対パス'`を使います。
2. その端末から通常画面版のPBVRを手動起動します。GUIが起動したら中継端末に戻り、Enterを押します。
3. 中継は確認用の`get_state`を1件送り、成功応答を受けてからSFTPへ接続します。設定に応じてSSH鍵のパスフレーズを入力します。`Bridge ready`が表示されたら、Claude Codeから要求できます。`startup_timeout`はEnterを押した後の確認用応答の待ち時間です。

## スパコン側の設定

[Python MCPサーバー用の仮想環境](../README.md#1-専用python環境の準備利用者が実施)をスパコン側の実行環境に用意し、Claude CodeのMCP設定に登録します。次の例では、Claude Codeから見える絶対パスを指定します。

```json
{
  "mcpServers": {
    "pbvr": {
      "command": "/workspace/Client/MCP/.venv/bin/python",
      "args": ["/workspace/Client/MCP/server.py"],
      "env": {
        "PBVR_TRANSPORT": "sftp-files",
        "PBVR_CTRL_DIR": "/shared/your-account/pbvr-mcp",
        "PBVR_MCP_TIMEOUT_SECONDS": "120"
      }
    }
  }
}
```

スパコン側の`PBVR_CTRL_DIR`は共有ディレクトリの親を指し、PC側PBVRの値は中継が表示したローカルのセッションディレクトリを指します。スパコン側の設定は中継を再起動しても使えます。`PBVR_MCP_TIMEOUT_SECONDS`は共有領域へ要求を公開した後の応答待ち時間です。`PBVR_TRANSPORT`を省略すると既存のローカル接続になります。

## 転送と復旧

- 中継はPC内での準備確認に成功した後、共有領域のUUIDディレクトリを作り、`bridge.active.json`で公開します。確認用の要求と応答はPC内に残ります。本番の要求は共有領域とPC側で別々のオフセットを使います。
- Python MCPサーバーは共有領域の`command.txt`に要求を追記し、`bridge.request.json`でIDとバイト範囲を公開します。中継は同じJSON行をPC側の`command.txt`へ追記し、PBVRが`structured.log`へ書いた対応する応答を共有領域へ返します。応答転送とオフセット更新が完了したら`bridge.done.json`を公開します。
- 一時切断時は同じ中継プロセスが再接続し、応答の転送済み部分を照合して残りを返します。Claude Codeで待機をキャンセルした場合も、公開済みの要求は中継で完了し得ます。先行要求の処理中に次を呼び出した場合は、先行処理が終わってから改めて呼び出します。
- タイムアウト、通信ファイルの不整合、再接続の失敗などでセッションが停止したら、中継とPBVRを終了します。中継を起動し直すと新しい通信ディレクトリが表示されるので、そのパスでPBVRを手動起動します。過去の要求の自動再送は行いません。通信ファイルを手動で整理する場合は、当該セッションの停止後に行います。

実機で確認する項目は、ローカル接続での`get_state`、SFTP中継での準備確認と`get_state`、一時切断後の応答転送、中継とPBVRの再起動後の新しいセッションへの切り替えです。
