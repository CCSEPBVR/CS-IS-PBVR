#ifndef TRANSFERFUNCTIONITEM_H
#define TRANSFERFUNCTIONITEM_H

#include <QStandardItem>
#include "ColorMap.h"
#include "OpacityMap.h"
#include "Histogram.h"

class TransferFunctionItem
{
public:
    enum RangeSelectionMode
    {
        None        = 0,
        UserRange = 1,
        ServerRange = 2
    };

    enum TransferFunctionItemRole
    {
        ColorFunction               = Qt::UserRole + 1,  // C[N]
        ColorVariable               = Qt::UserRole + 2,  // q[N]
        TemporaryColorRangeMode     = Qt::UserRole + 3,  // カラーレンジモード（未確定）
        CurrentColorRangeMode       = Qt::UserRole + 4,  // カラーレンジモード（確定）
        ResultColorRangeMode        = Qt::UserRole + 5,  // カラー:受信したヒストグラムのレンジモード
        ColorUserRangeMin           = Qt::UserRole + 6,  // カラー: ユーザ定義レンジ最小
        ColorUserRangeMax           = Qt::UserRole + 7,  // カラー: ユーザ定義レンジ最大
        ColorServerRangeMin         = Qt::UserRole + 8,  // カラー: サーバレンジ(最小)
        ColorServerRangeMax         = Qt::UserRole + 9,  // カラー: サーバレンジ(最大)
        ColorMap                    = Qt::UserRole + 10, // カラーマップ
        ColorHistogram              = Qt::UserRole + 11, // カラーヒストグラム
        OpacityFunction             = Qt::UserRole + 12, // O[N]
        OpacityVariable             = Qt::UserRole + 13, // q[N]
        TemporaryOpacityRangeMode   = Qt::UserRole + 14, // オパシティレンジモード（未確定）
        CurrentOpacityRangeMode     = Qt::UserRole + 15, // オパシティレンジモード（確定）
        ResultOpacityRangeMode      = Qt::UserRole + 16, // オパシティ:受信したヒストグラムのレンジモード
        OpacityUserRangeMin         = Qt::UserRole + 17, // オパシティ: ユーザ定義レンジ最小
        OpacityUserRangeMax         = Qt::UserRole + 18, // オパシティ: ユーザ定義レンジ最大
        OpacityServerRangeMin       = Qt::UserRole + 19, // オパシティ: サーバレンジ(最小)
        OpacityServerRangeMax       = Qt::UserRole + 20, // オパシティ: サーバレンジ(最大)
        OpacityMap                  = Qt::UserRole + 21, // オパシティマップ
        OpacityHistogram            = Qt::UserRole + 22, // オパシティヒストグラム
    };

    // コンストラクタ
    TransferFunctionItem(){}

    QList<QStandardItem*> createItem( int numberOfTransferFunction )
    {
        QStandardItem* transferFunctionItem = new QStandardItem( "TransferFunction Item" );

        // --- Color ---
        transferFunctionItem->setData( "C" + QString::number( numberOfTransferFunction ), ColorFunction );
        transferFunctionItem->setData( "q1", ColorVariable );
        transferFunctionItem->setData( QVariant::fromValue( RangeSelectionMode::UserRange ), TemporaryColorRangeMode );
        transferFunctionItem->setData( QVariant::fromValue( RangeSelectionMode::None ), CurrentColorRangeMode );
        transferFunctionItem->setData( QVariant::fromValue( RangeSelectionMode::None ), ResultColorRangeMode );
        transferFunctionItem->setData( 0.0, ColorUserRangeMin );
        transferFunctionItem->setData( 1.0, ColorUserRangeMax );
        transferFunctionItem->setData( 0.0, ColorServerRangeMin );
        transferFunctionItem->setData( 1.0, ColorServerRangeMax );
        class ColorMap colorMap;
        transferFunctionItem->setData( QVariant::fromValue( colorMap.getColors() ), TransferFunctionItem::ColorMap );
        class Histogram colorHistogram;
        transferFunctionItem->setData( QVariant::fromValue( colorHistogram.getDatas() ), TransferFunctionItem::ColorHistogram );

        // --- Opacity ---
        transferFunctionItem->setData( "O" + QString::number( numberOfTransferFunction ), OpacityFunction );
        transferFunctionItem->setData( "q1", OpacityVariable );
        transferFunctionItem->setData( QVariant::fromValue( RangeSelectionMode::UserRange ), TemporaryOpacityRangeMode );
        transferFunctionItem->setData( QVariant::fromValue( RangeSelectionMode::None ), CurrentOpacityRangeMode );
        transferFunctionItem->setData( QVariant::fromValue( RangeSelectionMode::None ), ResultOpacityRangeMode );
        transferFunctionItem->setData( 0.0, OpacityUserRangeMin );
        transferFunctionItem->setData( 1.0, OpacityUserRangeMax );
        transferFunctionItem->setData( 0.0, OpacityServerRangeMin );
        transferFunctionItem->setData( 1.0, OpacityServerRangeMax );
        class OpacityMap opacityMap;
        transferFunctionItem->setData( QVariant::fromValue( opacityMap.getOpacities() ), TransferFunctionItem::OpacityMap );
        class Histogram opacityHistogram;
        transferFunctionItem->setData( QVariant::fromValue( opacityHistogram.getDatas() ), TransferFunctionItem::OpacityHistogram );

        return QList<QStandardItem*>() << transferFunctionItem;
    }
};

#endif // TRANSFERFUNCTIONITEM_H
