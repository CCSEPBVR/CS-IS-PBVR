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

    this->addWidget( containerWidget );
    this->setMovable( false );

    connect( m_color_function_combo_box, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &ColorMapSelectorToolBar::onColorFunctionComboBoxIndexChanged );
}

ColorMapSelectorToolBar::~ColorMapSelectorToolBar() {}

void ColorMapSelectorToolBar::onTransferFunctionUpdate( TransferFunction* tf )
{
    if( !tf ) { return; }

    const int prev_index = m_color_function_combo_box->currentIndex();

    m_transfer_function_storage = *tf;
    m_transfer_function = &m_transfer_function_storage;

    QSignalBlocker blocker( m_color_function_combo_box );

    m_color_function_combo_box->clear();

    const int count = static_cast<int>( m_transfer_function->count() );
    for( int i = 0; i < count; ++i )
    {
        const QString label = QString( "C%1" ).arg( i + 1 );
        m_color_function_combo_box->addItem( label );
    }

    int new_index = prev_index;
    if( new_index < 0 ) { new_index = 0; }
    if( new_index >= count ) { new_index = count - 1; }

    if( count > 0 ) { m_color_function_combo_box->setCurrentIndex( new_index ); }

    updateCurrentIndex();
}



void ColorMapSelectorToolBar::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ColorMapSelectorToolBar::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ColorMapSelectorToolBar::updateColorMapByIndex( int index )
{    
    if( !m_transfer_function || !m_color_map_bar ) { return; }

    const int count = m_transfer_function->count();
    if( count <= 0 )                  { return; }
    if( index < 0 || index >= count ) { return; }

    const auto& colors = m_transfer_function->at( index ).color.map; // std::vector<kvs::RGBColor>
    const int n = static_cast<int>( colors.size() );
    if( n <= 0 ) { return; }

    kvs::ColorMap color_map( 256, 0.0, 1.0 );

    if( n == 1 )
    {
        color_map.addPoint( 0.0, colors[0] );
        color_map.addPoint( 1.0, colors[0] );
    }
    else
    {
        for( int i = 0; i < n; ++i )
        {
            const double position = static_cast<double>( i ) / static_cast<double>( n - 1 );
            color_map.addPoint( position, colors[static_cast<size_t>( i )] );
        }
    }

    color_map.create();
    m_color_map_bar->setColorMap( color_map );
    if( m_transfer_function->at( index ).color.rangeMode == TransferFunction::UserRange )
    {
        m_color_map_bar->setRange( m_transfer_function->at( index ).color.userDefinedMinMax.first, m_transfer_function->at( index ).color.userDefinedMinMax.second );
    }
    else
    {
        m_color_map_bar->setRange( m_transfer_function->at( index ).color.serverSideMinMax.first, m_transfer_function->at( index ).color.serverSideMinMax.second );
    }

    m_screen->update();
}

void ColorMapSelectorToolBar::updateCurrentIndex()
{
    const int index = m_color_function_combo_box->currentIndex();
    updateColorMapByIndex( index );
}

void ColorMapSelectorToolBar::onColorFunctionComboBoxIndexChanged( int index )
{
    updateColorMapByIndex( index );
}
