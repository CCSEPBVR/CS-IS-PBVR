# 混合要素を含む非構造格子の読み込み

混合要素を含む非構造格子ファイルを読み込む場合,

1.  要素ごとのKVSMLファイル,
2.  要素ごとのPFIファイル出力と
3.  PFLファイルの作成

が必要となります.

本章ではコード例としてVtu2Exampleを使用します.

以下Example/VtuExample.cppより, Vtu2Kvsmlを引用するため, 別途Example/VtuExample.cppを開いてください.

## 要素ごとのKVSMLファイル出力

FileFormat, Importer, KVSオブジェクトとExporterについては[基本的な使用方法](basics.md)を確認してください.

混合要素を含む非構造格子ファイルから要素ごとの非構造格子FileFormatを生成する方法として, `eachCellType` メンバを使用します.
`eachCellType` メンバの返り値は`begin` と `end` メンバを持ちそれぞれイテレータを返すため, for-each文を使用することができます.

Vtu2Kvsmlでは, `input_vtu` が読み込んでいるVTK非構造格子データに含まれる, 各要素データを格納したFileFormatが `vtu` に格納されます.

```c++
    for ( auto vtu : input_vtu.eachCellType() )
    {
        //...
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );
        std::cout << "  cell type: " << importer.cellType() << std::endl;
        //...
    }
```

## 要素ごとのPFIファイル出力

Vtu2Kvsmlではfor-each文の中で各要素に対しPFIファイルを出力しています.

```c++
        cvt::UnstructuredPfi pfi( object->veclen(), last_time_step, sub_volume_count );
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        pfi.write( directory, local_base );
```

## PFLファイルの出力

PBVRは混合要素のデータに対し, PFLファイルを作成する必要があります.

Vtu2Kvsmlでは以下PFLファイルを出力しています.

```c++
    cvt::Pfl pfl;

    for ( auto vtu : input_vtu.eachCellType() )
    {
       // ...
        pfl.registerPfi( directory, local_base );
    }

    pfl.write( directory, base );
```

## 最終的な出力

Vtu2Kvsmlでは最終的に

-   各要素のKVSMLファイル, datファイル一式
-   各要素のPFIファイル
-   PFLファイル

が出力されます.

PFLファイルをPBVRから読み込むことで, 可視化することが可能です.
