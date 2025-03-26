# KVSML変換ライブラリ

## 概要

このライブラリはVTK, CGNS, EnSight GoldやPLOT3D形式ファイルなどから, PVBRで可視化可能なKVSML, PFIとPFL形式ファイルへ変換する関数とクラスを提供するものです.

## 必要環境

### 共通

-   [CCSEPBVR/KVS](https://github.com/CCSEPBVR/KVS)
-   [VTK 9.2以降](https://gitlab.kitware.com/vtk/vtk)

### Linux, MacOS

-   g++ または clang
-   GNU Make

### Windows

-   Visual Studio  

### 推奨環境

-   mkdocs (マニュアルのビルド, 閲覧)

## マニュアル

詳細はDocument以下を参照してください.
Markdown形式でテキストエディタなどから閲覧が可能です.

mkdocsをインストールしている場合, 以下コマンドで閲覧が可能です.

    cd Documentへのパス
    mkdocs serve

## ビルドとインストール

まず, GNU Makeならば _Makefile.def_ , Visual Studio nmakeならば _Makefile.vs.def_ を編集しKVSとVTKへのパスを設定してください.

次に, GNU Makeの場合, `make`, Visual Studio nmakeの場合, `nmake /A` を実行してください.

ビルド成功後, GNU Makeの場合, `make install`, Visual Studio nmakeの場合, `nmake install` を実行してください.

詳細はDocument/docs/install.mdを参照してください.

## サンプルの実行

ビルド成功後, 各種サンプルを実行するkvsml-converterというバイナリが生成されます.
以下にVTUファイルからKVSMLファイルへの変換実行例を示します.

    ./kvsml-converter seriesvtu2kvsml Example/Inputディレクトリへの絶対パス ファイル出力先ディレクトリパス

サンプルなどの詳細はDocument以下を参照してください.
