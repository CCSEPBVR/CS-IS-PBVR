# サンプル概要

## サンプル一覧

以下 Example 以下に格納されています.

| ファイル名         | 関数名                 | 概要                                                                   |
| ------------------ | ---------------------- | ---------------------------------------------------------------------- |
| AVSExample.cpp     | AvsUcd2Kvsml           | AVS UCD ファイルの読み込み                                             |
| CgnsExample.cpp    | Cgns2Kvsml             | CGNS ファイルの読み込み (kvsml-converter より実行不可)                 |
| EnSightExample.cpp | Case2Kvsml             | EnSight Gold バイナリファイルの読み込み (kvsml-converter より実行不可) |
| Plot3dExample.cpp  | Plot3d2Kvsml           | PLOT3D ファイルの読み込み                                              |
| PvtsExample.cpp    | Pvts2Kvsml             | PVTS 形式ファイル読み込み                                              |
| PvtuExample.cpp    | Pvtu2Kvsml             | 単一 PVTU 形式ファイル読み込み                                         |
| PvtuExample.cpp    | SeriesPvtu2Kvsml       | 時系列 PVTU 形式ファイル分散読み込み                                   |
| PvtuExample.cpp    | SeriesPvtu2KvsmlWhole  | 時系列 PVTU 形式ファイル一括読み込み                                   |
| StlExample.cpp     | Stl2Stl                | STL 読み込み                                                           |
| StlExample.cpp     | Stl2Kvsml              | STL 読み込み                                                           |
| VtiExample.cpp     | StructuredPoints2Kvsml | StructuredPoints 形式(レガシー VTK 形式)読み込み                       |
| VtiExample.cpp     | Vti2Kvsml              | 単一 VTI 形式ファイル読み込み                                          |
| VtiExample.cpp     | SeriesVti2Kvsml        | 時系列 VTI 形式ファイル読み込み                                        |
| VtmExample.cpp     | AccessToVtm            | VTK MultiBlock データアクセス例                                        |
| VtmExample.cpp     | SeriesVtm2Kvsml        | 非構造格子のみの VTK MultiBlock 読み込み                               |
| VtpExample.cpp     | Vtp2Kvsml              | VTP 形式ファイル読み込み                                               |
| VtrExample.cpp     | Vtr2Kvsml              | VTR 形式ファイル読み込み                                               |
| VtsExample.cpp     | Vts2Kvsml              | VTS 形式ファイル読み込み                                               |
| VtuExample.cpp     | Vtu2Kvsml              | VTU 形式ファイル読み込み                                               |
| VtuExample.cpp     | SeriesVtu2Kvsml        | 時系列 VTU 形式ファイル読み込み                                        |
| VtuExample.cpp     | PointVtu2Kvsml         | 節点のみの VTU 形式ファイル読み込み                                    |
| VtuExample.cpp     | LineVtu2Kvsml          | 線分のみの VTU 形式ファイル読み込み                                    |
| VtuExample.cpp     | TriangleVtu2Kvsml      | 三角要素のみの VTU 形式ファイル読み込み                                |

## 入力例一覧

以下 Example/Input 以下に格納されています.

| ディレクトリ名         | 概要                                                                                       |
| ---------------------- | ------------------------------------------------------------------------------------------ |
| Hex                    | 時系列六面体非構造格子                                                                     |
| MultiBlock             | 複数のデータ型を格納したマルチブロック                                                     |
| pbvr_sample_data       | [CCSEPBVR/pbvr_sample_data](https://github.com/CCSEPBVR/pbvr_sample_data) (外部リポジトリ) |
| Polygon                | ポリゴンデータ                                                                             |
| Pvts                   | 分散構造格子                                                                               |
| Pvtu                   | 時系列分散混合要素非構造格子                                                               |
| Tetra                  | 時系列四面体非構造格子                                                                     |
| TetraAndHex            | 時系列四面体と六面体混合非構造格子                                                         |
| Triangle               | 三角要素非構造格子                                                                         |
| UnstructuredMultiBlock | 時系列混合要素非構造格子マルチブロック                                                     |
| Vertex                 | 節点要素非構造格子                                                                         |
| Vti                    | 時系列構造格子                                                                             |
| VTKExamples            | [Kitware/vtk-examples](https://github.com/Kitware/vtk-examples) (外部リポジトリ)           |
