# 基本的な使用方法

本章では最も単純な読み書きの例として, Vti2Kvsmlを使用します.

以下Example/VtiExample.cppより, Vti2Kvsmlを引用するため, 別途Example/VtiExample.cppを開いてください.

Vti2KvsmlはVTI形式の読み込みと, KVSML形式の書き込みを行うサンプルコードです.

## VTI形式ファイルの読み込み

まず, VTI形式のファイル読み込みを実行します.

対象のファイルフォーマットに対応したFileFormatクラスのインスタンスを生成します.
ここではVTIファイルのため, `cvt::VtkXmlImageData` クラスのインスタンスを生成します.

```c++
    cvt::VtkXmlImageData input_vtu( src );
```

次に対応するImporterクラス `cvt::VtkImporter<cvt::VtkXmlImageData>` のインスタンスを生成します.

```c++
    cvt::VtkImporter<cvt::VtkXmlImageData> importer( &input_vti );
```

`cvt::VtkImporter` 及びKVSのImporterクラスはボリュームオブジェクトを継承しているため, キャストして使用するか, そのままメンバにアクセスしてください.

```c++
    kvs::StructuredVolumeObject* object = &importer;
    object->updateMinMaxCoords();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );
    object->print( std::cout, kvs::Indent( 2 ) );
```

## KVSML形式ファイルの書き込み

次に, KVSML形式のファイル書き込みを行います.

まず適切なExporterクラスのインスタンスを生成します.

KVSのExporterも使用することができますが, ここではcvt以下のExporterを使用します.
ExporterはFileFormatを継承しており, また `cvt::StructuredVolumeObjectExporter` はKVSML形式のためのFileFormatを継承しているため, そのままメンバ関数を呼び出し, 書き込むことができます.
このとき, cvt以下のExporterはPBVR向けファイル名を設定するための `write` メンバ関数があるため, 今回はこれを使用します.

```c++
    cvt::StructuredVolumeObjectExporter exporter( &importer );
    exporter.setWritingDataTypeToExternalBinary();
    exporter.write( directory, base, time_step, sub_volume_id, sub_volume_count );
```

## PFIファイルの書き込み

最後に, PFIファイルを生成します.

```c++
    cvt::UnstructuredPfi pfi( exporter.veclen(), last_time_step, sub_volume_count );
    pfi.registerObject( &exporter, time_step, sub_volume_id );
    pfi.write( directory, base );
```

## 最終的な出力

Vti2Kvsmlは最終的に

-   KVSML形式ファイル, datファイル一式と,
-   PFIファイル

が出力されます.

PFIファイルをPBVRから読み込むことで, 可視化することが可能です.
