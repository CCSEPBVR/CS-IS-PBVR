#include "ColorMapSelectorToolBar.h"

ColorMapSelectorToolBar::ColorMapSelectorToolBar( kvs::qt::jaea::Screen* screen, QWidget* parent )
    : QToolBar( parent )
    , m_screen( screen )
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_color_function_label = new QLabel( "Color Function : ", this );
    m_color_function_combo_box = new QComboBox( this );

    layout->addWidget( m_color_function_label );
    layout->addWidget( m_color_function_combo_box );

    // ToolBarにウィジェットを追加
    this->addWidget( containerWidget );
    this->setMovable( false );

    connect( m_color_function_combo_box, &QComboBox::currentIndexChanged, this, &ColorMapSelectorToolBar::updateUIFromCurrentItem );
}

ColorMapSelectorToolBar::~ColorMapSelectorToolBar() {}

void ColorMapSelectorToolBar::updateColorMapBar( QStandardItemModel* model )
{

}

void ColorMapSelectorToolBar::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ColorMapSelectorToolBar::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ColorMapSelectorToolBar::updateUIFromCurrentItem()
{

}
