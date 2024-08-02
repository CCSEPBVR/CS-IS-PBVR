# 時系列データの読み込み

本章ではサンプルコードとしてSeriesVtu2Kvsmlを使用します.

以下Example/VtuExample.cppより, SeriesVtu2Kvsmlを引用するため, 別途Example/VtusExample.cppを開いてください.

## 連番ファイルの読み込み

SeriesVtu2Kvsmlは連番ファイルexample_0.vtu, example_1.vtu, ..., example_N.vtuファイルを時系列データとして読み込むコード例です.

まず, `example_*.vtu` のようなファイルパターン文字列 `src` に対し, 各ファイルのFileFormatを取得するためのクラス `cvt::NumeralSequenceFiles<>` のインスタンスを生成します.

`cvt::NumeralSequenceFiles<>` クラスより各ステップのFileFormatを取り出すには `eachTimeStep` メンバを使用します.
`eachTimeStep` メンバの返り値は`begin` と `end` メンバを持ちそれぞれイテレータを返すため, for-each文を使用することができます.

```c++
    cvt::NumeralSequenceFiles<cvt::VtkXmlUnstructuredGrid> time_series( src );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    int sub_volume_id = 1;
    int sub_volume_count = 1;

    for ( auto whole_vtu : time_series.eachTimeStep() )
    {
        std::cout << "Reading " << whole_vtu.filename() << " ..." << std::endl;
        // ...
    }
```

## PFIファイルの出力

各タイムステップデータの情報を, PFIファイルに登録する必要があります.

SeriesVtu2Kvsmlのサンプルコードでは `time_step` をタイムステップとして, 各タイムステップデータを登録しています.

```c++
            if ( time_step == 0 )
            {
                pfi_map.emplace(
                    static_cast<int>( object->cellType() ),
                    cvt::UnstructuredPfi( object->veclen(), last_time_step, sub_volume_count ) );
            }
            pfi_map.at( static_cast<int>( object->cellType() ) )
                .registerObject( &exporter, time_step, sub_volume_id );
```
