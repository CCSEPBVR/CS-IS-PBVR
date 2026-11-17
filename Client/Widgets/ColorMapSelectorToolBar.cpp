#include "ColorMapSelectorToolBar.h"

ColorMapSelectorToolBar::ColorMapSelectorToolBar( kvs::qt::jaea::Screen* screen, QWidget* parent )
    : QToolBar( parent )
    , m_screen( screen )
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_color_function_label = new QLabel( "Color Function : ", this );
    m_color_function_combo_box = new QComboBox( this );
    m_color_function_combo_box->setSizeAdjustPolicy( QComboBox::AdjustToContents );

    layout->addWidget( m_color_function_label );
    layout->addWidget( m_color_function_combo_box );

    this->addWidget( containerWidget );
    this->setMovable( false );

    connect( m_color_function_combo_box, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &ColorMapSelectorToolBar::onColorFunctionComboBoxIndexChanged );
}

ColorMapSelectorToolBar::~ColorMapSelectorToolBar() {}

void ColorMapSelectorToolBar::setMode( Mode mode )
{
    if( m_mode == mode ) { return; }

    m_mode = mode;
    rebuildComboBox( 0 );
    updateCurrentIndex();
}

void ColorMapSelectorToolBar::onTransferFunctionUpdate( TransferFunction* tf )
{
    if( !tf ) { return; }

    const int prev_index = m_color_function_combo_box->currentIndex();

    m_transfer_function_storage = *tf;
    if( m_mode != Mode::TransferFunction ) { return; }

    rebuildComboBox( prev_index );
    updateCurrentIndex();
}

void ColorMapSelectorToolBar::onEnsembleTransferFunctionUpdate( TransferFunction* tf, int selectedIndex )
{
    if( !tf ) { return; }

    m_ensemble_transfer_function_storage = *tf;
    if( m_mode != Mode::EnsembleTransferFunction ) { return; }

    rebuildComboBox( selectedIndex );
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

void ColorMapSelectorToolBar::rebuildComboBox( int preferredIndex )
{
    TransferFunction* tf = currentTransferFunction();
    if( !tf ) { return; }

    const int count = static_cast<int>( tf->count() );

    QSignalBlocker blocker( m_color_function_combo_box );
    m_color_function_combo_box->clear();

    int minimum_width = 0;
    for( int i = 0; i < count; ++i )
    {
        QString label;
        if( m_mode == Mode::EnsembleTransferFunction )
        {
            if( i == 0 )      { label = QStringLiteral( "Average" ); }
            else if( i == 1 ) { label = QStringLiteral( "Variance" ); }
            else if( i == 2 ) { label = QStringLiteral( "Coefficient of Variation" ); }
            else              { label = QString( "Statistic %1" ).arg( i + 1 ); }
        }
        else
        {
            label = QString( "C%1" ).arg( i + 1 );
        }
        m_color_function_combo_box->addItem( label );

        const int label_width = m_color_function_combo_box->fontMetrics().horizontalAdvance( label );
        if( label_width > minimum_width ) { minimum_width = label_width; }
    }

    if( count > 0 )
    {
        m_color_function_combo_box->setMinimumWidth( minimum_width + 48 );
    }

    int new_index = preferredIndex;
    if( new_index < 0 ) { new_index = 0; }
    if( new_index >= count ) { new_index = count - 1; }

    if( count > 0 ) { m_color_function_combo_box->setCurrentIndex( new_index ); }
}

TransferFunction* ColorMapSelectorToolBar::currentTransferFunction()
{
    return m_mode == Mode::EnsembleTransferFunction ? m_ensemble_transfer_function : m_transfer_function;
}

void ColorMapSelectorToolBar::updateColorMapByIndex( int index )
{    
    TransferFunction* tf = currentTransferFunction();
    if( !tf || !m_color_map_bar ) { return; }

    const int count = tf->count();
    if( count <= 0 )                  { return; }
    if( index < 0 || index >= count ) { return; }

    const auto& colors = tf->at( index ).color.map; // std::vector<kvs::RGBColor>
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
    if( tf->at( index ).color.rangeMode == TransferFunction::UserRange )
    {
        m_color_map_bar->setRange( tf->at( index ).color.userDefinedMinMax.first, tf->at( index ).color.userDefinedMinMax.second );
    }
    else
    {
        m_color_map_bar->setRange( tf->at( index ).color.serverSideMinMax.first, tf->at( index ).color.serverSideMinMax.second );
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
