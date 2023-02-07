# 分散データの読み込み

混合要素を含む非構造格子ファイルを読み込む場合,

1.  要素ごとのKVSMLファイル,
2.  要素ごとのPFIファイル出力と
3.  PFLファイルの作成

が必要となります.

本章ではサンプルコードとしてPvts2ExampleとSeriesPvtu2KvsmlWholeを使用します.

以下Example/PvtuExample.cppより, Pvtu2KvsmlとSeriesPvtu2KvsmlWholeを引用するため, 別途Example/PvtusExample.cppを開いてください.

## 各部分領域ル出力

FileFormat, Importer, KVSオブジェクトとExporterについては[基本的な使用方法](basics.md)を確認してください.

分散データクラスはnumberOfPiecesメンバを持ち, 部分領域数を取得することができます.

```c++
    std::cout << "reading " << src << " ..." << std::endl;
    cvt::VtkXmlPUnstructuredGrid input_pvtu( src );
    std::cout << "#piece: " << input_pvtu.numberOfPieces() << std::endl;
```

分散ファイルから部分領域FileFormatを生成する方法として, `eachPiece` メンバを使用します.
`eachPiece` メンバの返り値は`begin` と `end` メンバを持ちそれぞれイテレータを返すため, for-each文を使用することができます.

Pvtu2Kvsmlでは, `input_pvtu` が読み込んでいる分散非構造格子データに含まれる, 部分領域を格納したFileFormatが `vtu` に格納されます.

```c++
    cvt::VtkXmlPUnstructuredGrid input_pvtu( src );
    // ...
            for ( auto vtu : input_pvtu.eachPiece() )
            {
               // ...
            }
```

## 全領域の読み込み

分散データクラスの `get` メンバを使用すると, 全領域を一つにしたFileFormatを取得することができます.

```c++
    // input_pvtuは分散データのFileFormat
    for ( auto input_pvtu : time_series.eachTimeStep() )
    {
        cvt::VtkXmlUnstructuredGrid vtu = input_pvtu.get();
        // ...
    }
```

## PFIファイルの出力

部分領域の各情報を, PFIファイルに登録する必要があります.

Example/PvtuExample.cppのサンプルコードでは `sub_volume_id` を部分領域番号として, 部分領域を登録しています.

```c++
            pfi_map.at( static_cast<int>( object->cellType() ) )
                .registerObject( &exporter, time_step, sub_volume_id );
```
