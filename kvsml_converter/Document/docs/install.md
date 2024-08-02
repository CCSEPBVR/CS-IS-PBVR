# インストール

## 依存ライブラリのインストール

本稿ではKVS及びVTKは事前にインストールされているものとします.
インストール方法は各ライブラリリポジトリを参考にしてください.

-   [CCSEPBVR/KVS](https://github.com/CCSEPBVR/KVS)
-   [VTK 9.2以降](https://gitlab.kitware.com/vtk/vtk)

特に, 動的ライブラリを生成した場合, パスを通すか, 今後のビルド時に動的ライブラリをバイナリと同一のディレクトリにコピーしてください.

## 設定ファイルの変更

GNU Makeならば _Makefile.def_ , Visual Studio nmakeならば _Makefile.vs.def_ を編集してください.
特に, 以下変数を編集してください.

-   `KVS_INCLUDE_PATH`, または `KVS_INCLUDE_OPTION`. KVSのインストール先includeディレクトリ.
-   `KVS_LIBRARY_PATH`, または `KVS_LIBRARY_OPTION`. KVSのインストール先libディレクトリ.
-   `VTK_INCLUDE_PATH`, または `VTK_INCLUDE_OPTION`. VTKのインストール先includeディレクトリ. インクルードが例えば`vtk-9.2/vtkSmartPointer.h` ではなく, `vtkSmartPointer.h` となるよう指定してください.
-   `VTK_LIBRARY_PATH`, または  `VTK_LIBRARY_PATH`. VTKのインストール先libディレクトリ.

## ビルド

GNU Makeの場合, `make`, Visual Studio nmakeの場合, `nmake /A` を実行してください.

## インストール

ビルド成功後, GNU Makeの場合, `make install`, Visual Studio nmakeの場合, `nmake install` を実行してください.

## 関数マニュアルのビルド

もしDoxygenをインストールしている場合, `doxygen` コマンドなどによりHTML形式とLatex形式のマニュアルを生成することができます.
