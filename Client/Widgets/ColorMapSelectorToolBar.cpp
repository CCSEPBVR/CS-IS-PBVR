#include "ColorMapSelectorToolBar.h"

ColorMapSelectorToolBar::ColorMapSelectorToolBar( kvs::qt::jaea::Screen* screen,
                                                  kvs::ColorMapBar* colorMapBar,
                                                  QWidget* parent )
    : QToolBar( parent )
    , m_screen( screen )
    , m_color_map_bar( colorMapBar )
{
    initialize();
}

ColorMapSelectorToolBar::~ColorMapSelectorToolBar() {}

void ColorMapSelectorToolBar::initialize()
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_color_function_label = new QLabel( "Color Function : ", this );
    m_color_function_combo_box = new QComboBox( this );

    // Layoutにウィジェットを追加
    layout->addWidget( m_color_function_label );
    layout->addWidget( m_color_function_combo_box );

    // ToolBarにウィジェットを追加
    this->addWidget( containerWidget );

    connect( m_color_function_combo_box                  , &QComboBox::currentIndexChanged   , this, &ColorMapSelectorToolBar::updateUIFromCurrentItem );
}

void ColorMapSelectorToolBar::updateColorMapBar( QStandardItemModel* model )
{
    if ( !model ) return;
    m_model = model;
    qDebug() << "[updateColorMapBar] rowCount:" << m_model->rowCount();

    int previousIndex = -1;
    QString previousFunction;

    // 現在の選択状態を保持
    if ( m_color_function_combo_box )
    {
        previousIndex = m_color_function_combo_box->currentIndex();
        previousFunction = m_color_function_combo_box->currentText();
        m_color_function_combo_box->clear(); // 既存の項目をクリア
    }

    for ( int row = 0; row < m_model->rowCount(); ++row )
    {
        QStandardItem* item = m_model->item( row );
        if ( !item )
        {
            qDebug() << "[updateColorMapBar] Null item at row:" << row;
            continue;
        }

        const QString colorFunction = item->data( TransferFunctionItem::ColorFunction ).toString();
        qDebug() << "--- TransferFunction Item [" << row << "] ---";
        qDebug() << "ColorFunction:" << colorFunction;

        if ( m_color_function_combo_box )
        {
            m_color_function_combo_box->addItem( colorFunction );
        }
    }

    // 以前の選択を復元
    if ( m_color_function_combo_box )
    {
        int restoredIndex = m_color_function_combo_box->findText( previousFunction );
        if ( restoredIndex >= 0 )
        {
            m_color_function_combo_box->setCurrentIndex( restoredIndex );
        }
        else
        {
            m_color_function_combo_box->setCurrentIndex( 0 );
        }
    }

    updateUIFromCurrentItem();
}


void ColorMapSelectorToolBar::updateUIFromCurrentItem()
{
    if ( !m_model || !m_color_function_combo_box || !m_color_map_bar ) return;

    int index = m_color_function_combo_box->currentIndex();
    if ( index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if ( !item ) return;

    // --- カラーマップ変換 ---
    QVariant colorMapVar = item->data( TransferFunctionItem::ColorMap );
    if ( colorMapVar.canConvert<QVariantList>() )
    {
        QVariantList colorList = colorMapVar.toList();
        const int n = colorList.size();

        kvs::ColorMap color_map( 256, 0.0, 1.0 );

        for ( int i = 0; i < n; ++i )
        {
            const double position = static_cast<double>(i) / (n - 1); // 正規化位置
            QColor qcolor = colorList[i].value<QColor>();
            kvs::RGBColor rgb( qcolor.red(), qcolor.green(), qcolor.blue() );
            color_map.addPoint( position, rgb );
        }

        color_map.create();

        m_color_map_bar->setColorMap( color_map );
    }

    int resultColorRangeMode = item->data( TransferFunctionItem::ResultColorRangeMode ).toInt();
    float colorMin = 0.0f;
    float colorMax = 0.0f;

    if( resultColorRangeMode == TransferFunctionItem::UserRange )
    {
        colorMin = item->data( TransferFunctionItem::ColorUserRangeMin ).toDouble();
        colorMax = item->data( TransferFunctionItem::ColorUserRangeMax ).toDouble();
    }
    else if( resultColorRangeMode == TransferFunctionItem::ServerRange )
    {
        colorMin = item->data( TransferFunctionItem::ColorServerRangeMin ).toDouble();
        colorMax = item->data( TransferFunctionItem::ColorServerRangeMax ).toDouble();
    }
    m_color_map_bar->setRange( colorMin, colorMax );

    m_screen->update();
}
