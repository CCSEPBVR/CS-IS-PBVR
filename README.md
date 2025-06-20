# PBVR
- [PBVR Wiki](https://github.com/CCSEPBVR/CS-IS-PBVR/wiki)
- PBVR Clientで使用している[KVSライブラリ](https://github.com/TO0603/KVS)

## Homebrew(CS-PBVR)
macOS・Linuxで使用することができるパッケージ管理ツールHomebrewを使ってCS-PBVRをインストールすることができる。<br>
HomebrewのTapという機能を使用しパッケージを作成しており、公式のパッケージではない。<br>
HomebrewにはRequirementsが[macOS](https://docs.brew.sh/Installation)・[Linux](https://docs.brew.sh/Homebrew-on-Linux)で設定されている。<br>また、[Support Tiers](https://docs.brew.sh/Support-Tiers)が設定されておりTier1をサポートしている。<br>
以下のコマンドを実行してインストールする。通常版・データ形式拡張版どちらかを選択してインストールする。<br>
データ形式拡張版ではフィルタプログラム・KVSMLコンバータを使用せずにデータを可視化することができる。<br>
大規模データはフィルタプログラム・KVSMLコンバータを使用し、KVSML形式に変換して可視化することを推奨する。
```
brew tap ccsepbvr/pbvr
brew install pbvr # 通常版
brew install pbvr-extended-fileformat #データ形式拡張版
```
以下のコマンドが実行できるようになる。<br>
サーバ・クライアント・フィルタプログラムの使い方は[こちら](https://github.com/CCSEPBVR/CS-IS-PBVR/wiki/Tutorial_JP)。<br>
KVSMLコンバータの使い方は[こちら](https://github.com/CCSEPBVR/CS-IS-PBVR/wiki/KVSMLConverter_JP)。<br>
サンプルデータは[こちら](https://github.com/CCSEPBVR/CS-IS-PBVR/releases/tag/SampleData)。
```
pbvr_server # サーバプログラム
pbvr_client # クライアントプログラム
pbvr_filter # フィルタプログラム
kvsml-converter # KVSMLコンバータ
```