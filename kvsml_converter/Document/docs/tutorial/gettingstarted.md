# はじめに

## サンプルコードの簡易実行

ビルド成功後, Example/Release, またはインストール先/binにkvsml-converterというバイナリが生成されます.

このバイナリはExample/Input以下の特定のファイルを入力とし, Example以下のサンプルコード例を実行するものです.

ここではもっとも単純なvtu2kvsmlを以下実行します.

```bash
<パス>/kvsml-converter vtu2kvsml <Example/Inputへのパス> <出力先>
```

もし, kvsml-converterのルートディレクトリがカレントディレクトリであるならば, 以下コマンドとなります.

```bash
# 出力先を./fire_ug
Example/Release/kvsml-converter vtu2kvsml ./Example/Input ./fire_ug
```

実行すると, 以下ファイルが作成されます.

-   KVSMLファイル一式
    -   fire_ug_2_00000_0000001_0000001.kvsml
    -   fire_ug_2_00000_0000001_0000001_coord.dat
    -   fire_ug_2_00000_0000001_0000001_connect.dat
    -   fire_ug_2_00000_0000001_0000001_value.dat
-   fire_ug.pfi
-   fire_ug.pfl

## PBVRでの可視化

本章ではWindows版PBVRを使用します.

まず, サーバを起動します.

```bash
.\pbvr_server_win.exe
```

次に別ターミナルで, クライアントを起動します.

```bash
.\pbvr_client_cs_cpu_win.exe -vin <fire_ug.pfiへのパス>
# または
.\pbvr_client_cs_cpu_win.exe -vin <fire_ug.pflへのパス>
```
