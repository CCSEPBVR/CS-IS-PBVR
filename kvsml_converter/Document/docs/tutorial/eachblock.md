# マルチブロックの読み込み

VTK MultiBlock形式, またはそれに変換可能なファイル形式は「ブロック」と呼ばれる単位で複数のデータファイルをまとめて格納, 読み込むことができます.

本章ではVTK MultiBlock形式へアクセスするサンプルコードとしてAccess2Vtmを使用します.

以下Example/VtmExample.cppより, Access2Vtmを引用するため, 別途Example/VtmExample.cppを開いてください.

## ブロックへのアクセス

マルチブロックからブロックごとの子FileFormatを生成する方法として, `eachBlock` メンバを使用します.
`eachBlock` メンバの返り値は`begin` と `end` メンバを持ちそれぞれイテレータを返すため, for-each文を使用することができます.

Access2Vtmでは, `input_vtm` が読み込んでいるVTK MultiBlockデータに含まれる, ブロックを格納したFileFormatが `format` に格納されます.

```c++
    cvt::VtkXmlMultiBlock input_vtm( src );

    for ( auto format : input_vtm.eachBlock() )
    {
      // ..
    }
```

VTK MultiBlock形式はポリゴン, 構造格子や非構造格子など複数のデータ形式を格納することができます.
そのため, もしVTMファイルに格納されているブロックが未知か, 複数のデータ形式が格納されていた場合, ImporterによりKVSオブジェクトへ変換するためには, `dynamic_cast` などにより変換可能か判定する必要があります.

Access2Vtmの以下のコードでは, 生成された `format` がポリゴンオブジェクトに変換可能か判定し, 変換可能な場合は `kvs::PolygonObject` へ変換しています.

```c++
        if ( auto polydata_format = dynamic_cast<cvt::VtkXmlPolyData*>( format.get() ) )
        {
            std::cout << "A polygon object was converted" << std::endl;
            cvt::VtkImporter<cvt::VtkXmlPolyData> importer( polydata_format );
            kvs::PolygonObject* polygon_object = kvs::PolygonObject::DownCast( &importer );
            polygon_object->print( std::cout, kvs::Indent( 2 ) );
        }
```
