# s86 から PC 上の MCP ping を呼ぶプロトタイプ

PC の Streamable HTTP サーバーが返す `pong from PC: ...` を、PBS 対話ジョブの Apptainer 内で動く Claude Code から取得する。既存の PBVR GUI、`server.py`、SFTP 共有領域から独立した疎通確認。

| 場所 | 待受 | 用途 |
| --- | --- | --- |
| PC | `127.0.0.1:18000` | Streamable HTTP サーバー |
| PC | `127.0.0.1:18001` | SSH のローカル転送 |
| s86 計算ノード | 計算ノードのアドレスの `60000` | PC からの転送接続 |
| s86 計算ノード | `127.0.0.1:60001` | Claude Code の HTTP 接続 |

`60002` は予備。SSH の接続先はログインノード。PC 側の中継が `ssh -L` 経由で計算ノードへ制御接続を張り、s86 側で新しい HTTP 接続を受けるごとに PC からデータ接続を追加する。HTTP は TCP のバイト列として転送するので、MCP のセッションやイベントストリームも SDK が処理する。

この手順は PC が Windows で Python 3.10 以上を使え、s86 に既存の Claude Code と Ollama の環境があることを前提とする。PBS 対話ジョブ側の中継には Python 3.6 以上の標準ライブラリを使う。PC 側の Python 環境には [既存の MCP 要件](../requirements.txt) をインストールする。

## 1. PC から検証用ファイルを配置する

Developer Command Prompt for VS 2022 を開き、PBVR リポジトリのルートで実行する。`USER` を s86 のユーザー名に置き換える。2つの作業ディレクトリが隣にある場合の例。

```bat
set "S86_LOGIN=USER@s86fs2.tokai-sc.jaea.go.jp"
ssh %S86_LOGIN% "mkdir -p ~/pbvr-mcp-prototype"
scp Client\MCP\prototype\reverse_tunnel.py Client\MCP\prototype\.mcp.json %S86_LOGIN%:pbvr-mcp-prototype/
scp ..\claude_sandbox_s86-main\run.sh %S86_LOGIN%:pbvr-mcp-prototype/run-pbvr-prototype.sh
```

PC の MCP 用仮想環境が未作成の場合だけ、次を実行する。

```bat
py -3 -m venv Client\MCP\.venv
Client\MCP\.venv\Scripts\python.exe -m pip install -r Client\MCP\requirements.txt
```

検証ごとに新しいトークンを作る。表示された値を後で s86 と PC の入力欄に同じように入力する。

```bat
Client\MCP\.venv\Scripts\python.exe -c "import secrets; print(secrets.token_hex(32))"
```

## 2. s86 の PBS 対話ジョブで中継を起動する

s86 のログインノードで、普段使う設定で GPU の対話ジョブを取得する。以下は既存の起動スクリプトと同じキュー・リソースを指定する例。

```bash
qsub -I -q sg4di -P agent -l select=1:ncpus=12:mpiprocs=1:ompthreads=12:ngpus=1 -l walltime=01:00:00
```

以降は割り当てられた計算ノードのシェルで実行する。既定シェルが Bash 以外の場合は先に `bash` を起動する。表示された計算ノード名は PC 側でも使う。短いホスト名での SSH 転送を確認済みの場合は、その名前を PC 側の設定に使える。

```bash
cd ~/pbvr-mcp-prototype
hostname -f
read -r -s -p 'Prototype token: ' PBVR_MCP_PROTOTYPE_TOKEN
printf '\n'
PBVR_MCP_PROTOTYPE_TOKEN="$PBVR_MCP_PROTOTYPE_TOKEN" python3 reverse_tunnel.py node --bind "$(hostname -f)" &
RELAY_PID=$!
unset PBVR_MCP_PROTOTYPE_TOKEN
```

`Waiting for PC tunnel` と `MCP endpoint on s86` の両方が出れば、2つの待受が開いている。`--bind` にはログインノードが到達できる計算ノードのアドレスを指定する。名前が複数アドレスに解決される環境では、疎通確認済みのインターフェースの IP アドレスを明示する。

## 3. PC のサーバーと転送を起動する

PC で Developer Command Prompt for VS 2022 を3つ開く。いずれも PBVR リポジトリのルートを使用する。

1つ目で MCP サーバーを起動する。

```bat
Client\MCP\.venv\Scripts\python.exe Client\MCP\prototype\ping_server.py
```

2つ目で SSH 転送を起動する。`S86_NODE` は割り当てられた計算ノード名に置き換える。

```bat
set "S86_LOGIN=USER@s86fs2.tokai-sc.jaea.go.jp"
set "S86_NODE=COMPUTE_NODE"
ssh -N -o ExitOnForwardFailure=yes -L 127.0.0.1:18001:%S86_NODE%:60000 %S86_LOGIN%
```

3つ目で PC 側の中継を起動し、s86 側と同じトークンを入力する。

```bat
Client\MCP\.venv\Scripts\python.exe Client\MCP\prototype\reverse_tunnel.py pc
```

PC に `Connected to the s86 tunnel`、s86 に `PC tunnel connected` と表示されることを確認する。トークンは PC 側中継を認証する。SSH の暗号化は PC とログインノードの間に適用される。ログインノードから計算ノードまでの転送には、疎通確認用の文字列だけを流す。

## 4. Apptainer の外側で HTTP を確認する

手順2の PBS ジョブのシェルで次を実行する。

```bash
curl --noproxy '*' --fail --show-error --include --max-time 20 \
  -H 'Content-Type: application/json' \
  -H 'Accept: application/json, text/event-stream' \
  --data '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"s86-probe","version":"0.1"}}}' \
  http://127.0.0.1:60001/mcp
```

HTTP 200 が返り、本文の `serverInfo.name` が `PBVR MCP Prototype` なら、s86 の入口から PC の MCP サーバーまで到達している。

## 5. Apptainer 内の Claude Code で ping を確認する

同じ PBS ジョブのシェルで、検証用にコピーした起動スクリプトを実行する。`--on-node` は取得済みのジョブを使う指定。`--pbvr-mcp-prototype` を指定した起動で計算ノードのネットワークを共有する。Ollama は空いているループバックポートを使用する。

```bash
bash ./run-pbvr-prototype.sh --on-node --pbvr-mcp-prototype "$PWD"
```

1. プロジェクトの MCP サーバーを有効にする確認が出たら、`pbvr-prototype` の URL を確認して許可する。
2. Claude Code の `/mcp` で `pbvr-prototype` が接続済みになり、`ping` が見えることを確認する。
3. Claude Code に「`pbvr-prototype` の `ping` ツールを `message: "s86-apptainer"` で呼び出してください」と指示する。ツールの実行結果が `pong from PC: s86-apptainer` なら成功。

手順4が成功して手順5で接続できない場合は、起動ログの `using the compute node network`、プロジェクトの MCP 許可、`/mcp` のエラー詳細を確認する。ツールが表示されて実行されない場合は、Claude Code のツール許可とモデルのツール呼び出しを確認する。

終了時は Claude Code を閉じ、PBS シェルで `kill "$RELAY_PID"` を実行してからジョブを終了する。PC 側の3プロセスはそれぞれ Ctrl+C で停止する。
