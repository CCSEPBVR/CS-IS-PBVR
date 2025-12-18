#include "GlyphEditor.h"
#include "ui_GlyphEditor.h"

GlyphEditor::GlyphEditor( WebSocketPair* websockets, QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::GlyphEditor )
    , m_web_sockets( websockets )
    , m_is_operator( false )
{
    ui->setupUi( this );

    QStringList types;
    types << "Arrow" << "Diamond" << "Sphere";
    ui->typeComboBox->addItems( types );

    ui->sizeAreaScrollArea      ->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    ui->colorDataAreaScrollArea ->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    onReset();

    // Direction
    connect( ui->direction1ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );
    connect( ui->direction2ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );
    connect( ui->direction3ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );

    // Size
    connect( ui->sizeConstantRadioButton        , &QRadioButton::clicked    , this, &GlyphEditor::onSizeUiState );
    connect( ui->sizeVariableArrayRadioButton   , &QRadioButton::clicked    , this, &GlyphEditor::onSizeUiState );
    connect( ui->sizeNumberOfVariablesSpinBox   , &QSpinBox::valueChanged   , this, &GlyphEditor::onSizeNumberOfVariableValueChanged );

    // Color Map
    connect( ui->editColorMapPushButton, &QPushButton::clicked, this, &GlyphEditor::onColorMapEditorClicked );

    // Color Data
    connect( ui->colorDataConstantRadioButton       , &QRadioButton::clicked    , this, &GlyphEditor::onColorDataUiState );
    connect( ui->colorDataVariableArrayRadioButton  , &QRadioButton::clicked    , this, &GlyphEditor::onColorDataUiState );
    connect( ui->colorDataNumberOfVariablesSpinBox  , &QSpinBox::valueChanged   , this, &GlyphEditor::onColorDataNumberOfVariableValueChanged );

    connect( ui->applyPushButton, &QPushButton::clicked, this, &GlyphEditor::onApply );

    onSizeUiState();
    onColorDataUiState();
}

GlyphEditor::~GlyphEditor()
{
    delete ui;
}

void GlyphEditor::onUpdateServerState( bool serverState )
{

}

void GlyphEditor::onOperatorStateUpdate( bool operatorState )
{
    m_is_operator = operatorState;

    if( m_color_map_editor.isVisible() ) m_color_map_editor.close();

    ui->typeComboBox                        ->setEnabled( m_is_operator );
    ui->scaleFactorDoubleSpinBox            ->setEnabled( m_is_operator );
    ui->direction1ComboBox                  ->setEnabled( m_is_operator );
    ui->direction2ComboBox                  ->setEnabled( m_is_operator );
    ui->direction3ComboBox                  ->setEnabled( m_is_operator );
    ui->sizeConstantRadioButton             ->setEnabled( m_is_operator );
    ui->sizeVariableArrayRadioButton        ->setEnabled( m_is_operator );
    ui->sizeNumberOfVariablesSpinBox        ->setEnabled( m_is_operator );
    for( int i = 0; i < ui->sizeVariableGridLayout->count(); ++i )
    {
        if( QLayoutItem* item = ui->sizeVariableGridLayout->itemAt( i ) )
        {
            if( QWidget* w = item->widget() )
            {
                if( auto cb = qobject_cast<QComboBox*>( w ) )
                    cb->setEnabled( m_is_operator );
            }
        }
    }

    ui->uniformRadioButton                  ->setEnabled( m_is_operator );
    ui->allPointsRadioButton                ->setEnabled( m_is_operator );
    ui->everyNthPointRadioButton            ->setEnabled( m_is_operator );

    ui->numberOfSamplePointsSpinBox         ->setEnabled( m_is_operator );
    ui->seedSpinBox                         ->setEnabled( m_is_operator );
    ui->strideSpinBox                       ->setEnabled( m_is_operator );

    ui->editColorMapPushButton              ->setEnabled( m_is_operator );

    ui->colorDataConstantRadioButton        ->setEnabled( m_is_operator );
    ui->colorDataVariableArrayRadioButton   ->setEnabled( m_is_operator );
    ui->colorDataNumberOfVariablesSpinBox   ->setEnabled( m_is_operator );
    for( int i = 0; i < ui->colorDataVariableGridLayout->count(); ++i )
    {
        if( QLayoutItem* item = ui->colorDataVariableGridLayout->itemAt( i ) )
        {
            if( QWidget* w = item->widget() )
            {
                if( auto cb = qobject_cast<QComboBox*>( w ) )
                    cb->setEnabled( m_is_operator );
            }
        }
    }

    ui->applyPushButton                     ->setEnabled( m_is_operator );
}

void GlyphEditor::onReset()
{
    onUpdateNumberOfVector( 0 );
    ui->typeComboBox->setCurrentIndex( GlyphParameter::Type::Arrow );
    ui->scaleFactorDoubleSpinBox->setValue( 1.0 );

    ui->sizeConstantRadioButton->setChecked( true );
    ui->sizeNumberOfVariablesSpinBox->setValue( 0 );

    ui->uniformRadioButton->setChecked( true );
    ui->numberOfSamplePointsSpinBox->setValue( 1000 );
    ui->seedSpinBox->setValue( 1 );
    ui->strideSpinBox->setValue( 3 );

    ui->colorMap->setColors( defaultColor() );

    ui->colorDataConstantRadioButton->setChecked( true );
    ui->colorDataNumberOfVariablesSpinBox->setValue( 0 );

    ui->colorDataConstantRadioButton->setChecked( true );
    ui->colorDataNumberOfVariablesSpinBox->setValue( 0 );
}

void GlyphEditor::onUpdateNumberOfVector( const int numberOfVector )
{
    if( numberOfVector < 3 ) return;

    m_vector_list.clear();

    for( int i = 0; i < numberOfVector; ++i )
    {
        const QString name = QString( "q%1" ).arg( i + 1 );
        m_vector_list << name;
    }

    // UI へ反映
    ui->direction1ComboBox->clear();
    ui->direction2ComboBox->clear();
    ui->direction3ComboBox->clear();

    ui->direction1ComboBox->addItems( m_vector_list );
    ui->direction2ComboBox->addItems( m_vector_list );
    ui->direction3ComboBox->addItems( m_vector_list );

    ui->sizeNumberOfVariablesSpinBox      ->setMaximum( numberOfVector );
    ui->colorDataNumberOfVariablesSpinBox ->setMaximum( numberOfVector );
}

void GlyphEditor::onReceiveGlyphParameter( const QJsonObject& glyphParameter )
{
    // const QJsonObject glyphParameter = dataArray.first().toObject();

    // Type
    ui->typeComboBox->setCurrentIndex( glyphParameter.value( QString::fromUtf8( Protocol::Key::Type ) ).toInt() );

    // ScaleFactor
    ui->scaleFactorDoubleSpinBox->setValue( glyphParameter.value( QString::fromUtf8( Protocol::Key::ScaleFactor ) ).toDouble() );

    // Direction
    ui->direction1ComboBox->setCurrentIndex( glyphParameter.value( QString::fromUtf8( Protocol::Key::Direction1 ) ).toInt() );
    ui->direction2ComboBox->setCurrentIndex( glyphParameter.value( QString::fromUtf8( Protocol::Key::Direction2 ) ).toInt() );
    ui->direction3ComboBox->setCurrentIndex( glyphParameter.value( QString::fromUtf8( Protocol::Key::Direction3 ) ).toInt() );

    // Size Mode
    GlyphParameter::DataMode sizeDataMode = static_cast<GlyphParameter::DataMode>( glyphParameter.value( QString::fromUtf8( Protocol::Key::SizeMode ) ).toInt() );

    if( sizeDataMode == GlyphParameter::DataMode::Constant )
    {
        ui->sizeConstantRadioButton->setChecked( true );
    }
    else if( sizeDataMode == GlyphParameter::DataMode::VariableArray )
    {
        ui->sizeVariableArrayRadioButton->setChecked( true );
    }

    const auto sizeVariables = glyphParameter.value( QString::fromUtf8( Protocol::Key::SizeVariables ) ).toArray();
    ui->sizeNumberOfVariablesSpinBox->setValue( sizeVariables.size() );
    int sizeComboIndex = 0;
    for( int i = 0; i < ui->sizeVariableGridLayout->count(); ++i )
    {
        auto* item = ui->sizeVariableGridLayout->itemAt( i );
        if( !item ) continue;

        if( auto* cb = qobject_cast<QComboBox*>( item->widget() ) )
        {
            if( sizeComboIndex < sizeVariables.size() )
            {
                const int val = sizeVariables.at( sizeComboIndex ).toInt();
                cb->setCurrentIndex( val );
            }
            ++sizeComboIndex;
        }
    }

    GlyphParameter::DistributionMode distributionMode = static_cast<GlyphParameter::DistributionMode>( glyphParameter.value( QString::fromUtf8( Protocol::Key::DistributionMode ) ).toInt() );
    if( distributionMode == GlyphParameter::DistributionMode::UniformDistribution  )
        ui->uniformRadioButton->setChecked( true );
    else if( distributionMode == GlyphParameter::DistributionMode::AllPoints )
        ui->allPointsRadioButton->setChecked( true );
    else if( distributionMode == GlyphParameter::EveryNthPoints )
        ui->everyNthPointRadioButton->setChecked( true );

    ui->numberOfSamplePointsSpinBox ->setValue( glyphParameter.value( QString::fromUtf8( Protocol::Key::NumberOfSamplePoints ) ).toInt() );
    ui->seedSpinBox                 ->setValue( glyphParameter.value( QString::fromUtf8( Protocol::Key::Seed ) ).toInt() );
    ui->strideSpinBox               ->setValue( glyphParameter.value( QString::fromUtf8( Protocol::Key::Stride ) ).toInt() );

    // ColorMap array
    const QJsonArray colorMapJson = glyphParameter.value( "ColorMap" ).toArray();

    QVector<QColor> colors;
    colors.reserve( colorMapJson.size() );
    for( const auto& c : colorMapJson )
    {
        const QJsonObject obj = c.toObject();

        const int r = obj.value( QString::fromUtf8( Protocol::Key::R ) ).toInt();
        const int g = obj.value( QString::fromUtf8( Protocol::Key::G ) ).toInt();
        const int b = obj.value( QString::fromUtf8( Protocol::Key::B ) ).toInt();

        colors.append( QColor( r, g, b ) );
    }
    ui->colorMap->setColors( colors );

    // Color Data Mode
    GlyphParameter::DataMode colorDataMode = static_cast<GlyphParameter::DataMode>( glyphParameter.value( QString::fromUtf8( Protocol::Key::ColorDataMode ) ).toInt() );

    if( colorDataMode == GlyphParameter::DataMode::Constant )
    {
        ui->colorDataConstantRadioButton->setChecked( true );
    }
    else if( colorDataMode == GlyphParameter::DataMode::VariableArray )
    {
        ui->colorDataVariableArrayRadioButton->setChecked( true );
    }

    const auto colorDataVariables = glyphParameter.value( "ColorDataVariables" ).toArray();
    ui->colorDataNumberOfVariablesSpinBox->setValue( colorDataVariables.size() );
    int colorDataComboIndex = 0;
    for( int i = 0; i < ui->colorDataVariableGridLayout->count(); ++i )
    {
        auto* item = ui->colorDataVariableGridLayout->itemAt( i );
        if( !item ) continue;

        if( auto* cb = qobject_cast<QComboBox*>( item->widget() ) )
        {
            if( colorDataComboIndex < colorDataVariables.size() )
            {
                const int val = colorDataVariables.at( colorDataComboIndex ).toInt();
                cb->setCurrentIndex( val );
            }
            ++colorDataComboIndex;
        }
    }
}

void GlyphEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void GlyphEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void GlyphEditor::clearLayout( QLayout* layout )
{
    while( QLayoutItem* item = layout->takeAt( 0 ) )
    {
        if( QWidget* widget = item->widget() ) widget->deleteLater();
        delete item;
    }
}

void GlyphEditor::updateUiEnableState( const bool isVariableArray, QSpinBox* spinBox, QGridLayout* gridLayout )
{
    if( m_is_operator ) spinBox->setEnabled( isVariableArray );

    // レイアウト内のすべてのウィジェットを有効/無効に切り替える
    for( int i = 0; i < gridLayout->count(); ++i )
    {
        QLayoutItem* item = gridLayout->itemAt( i );
        if( !item ) continue;

        QWidget* widget = item->widget();
        if( widget )
        {
            if( m_is_operator ) widget->setEnabled( isVariableArray );
        }
    }
}

void GlyphEditor::updateVariableGridLayout( QSpinBox* spinBox, QGridLayout* grid )
{
    const int newCount = spinBox->value();

    // 既存のコンボボックスを収集
    QVector<QComboBox*> combos;
    for( int i = 0; i < grid->count(); ++i )
    {
        if( QWidget* w = grid->itemAt( i )->widget() )
        {
            if( auto cb = qobject_cast<QComboBox*>( w ) )
            {
                combos.append( cb );
            }
        }
    }

    const int currentCount = combos.size();

    //===========================
    // 行を増やす
    //===========================
    for( int i = currentCount; i < newCount; ++i )
    {
        QLabel* label = new QLabel( QString( "Variable%1" ).arg( i + 1 ), this );

        QComboBox* combo = new QComboBox( this );
        combo->addItems( m_vector_list );
        combo->setEnabled( m_is_operator );

        grid->addWidget( label, i, 0 );
        grid->addWidget( combo, i, 1 );
    }

    //===========================
    // 行を減らす
    //===========================
    for( int i = currentCount - 1; i >= newCount; --i )
    {
        if( auto* labelItem = grid->itemAtPosition( i, 0 ) )
            delete labelItem->widget();

        if( auto* comboItem = grid->itemAtPosition( i, 1 ) )
            delete comboItem->widget();
    }
}

void GlyphEditor::onDirectionComboBoxIndexChanged()
{
    QStringList selectedList;
    selectedList << ui->direction1ComboBox->currentText()
                 << ui->direction2ComboBox->currentText()
                 << ui->direction3ComboBox->currentText();

    QList<QComboBox*> comboBoxes =
        {
            ui->direction1ComboBox,
            ui->direction2ComboBox,
            ui->direction3ComboBox
        };

    QSet<QString> used;
    for( QComboBox* combo : comboBoxes )
    {
        QString value = combo->currentText();
        if( !value.isEmpty() && used.contains( value ) )
        {
            // 重複していたら別の未使用の値を探して設定
            for( const QString& candidate : m_vector_list )
            {
                if( !used.contains( candidate ) )
                {
                    combo->blockSignals( true );
                    int index = combo->findText( candidate );
                    if( index != -1 )
                        combo->setCurrentIndex( index );
                    combo->blockSignals( false );
                    used.insert( candidate );
                    break;
                }
            }
        }
        else
        {
            if( !value.isEmpty() ) used.insert( value );
        }
    }
}

void GlyphEditor::onSizeUiState()
{
    const bool isVariableArray = ui->sizeVariableArrayRadioButton->isChecked();
    updateUiEnableState( isVariableArray, ui->sizeNumberOfVariablesSpinBox, ui->sizeVariableGridLayout );
}

void GlyphEditor::onSizeNumberOfVariableValueChanged()
{
    updateVariableGridLayout( ui->sizeNumberOfVariablesSpinBox, ui->sizeVariableGridLayout );
}

void GlyphEditor::onColorMapEditorClicked()
{
    ColorMapEditor colorMapEditor;
    colorMapEditor.adjustSize();
    colorMapEditor.setDefaultColorMap( ui->colorMap->getColors() );

    if( colorMapEditor.exec() == QDialog::Accepted )
    {
        ui->colorMap->setColors( colorMapEditor.getColorMap() );
    }
}

void GlyphEditor::onColorDataUiState()
{
    const bool isVariableArray = ui->colorDataVariableArrayRadioButton->isChecked();
    updateUiEnableState( isVariableArray, ui->colorDataNumberOfVariablesSpinBox, ui->colorDataVariableGridLayout );
}

void GlyphEditor::onColorDataNumberOfVariableValueChanged()
{
    updateVariableGridLayout( ui->colorDataNumberOfVariablesSpinBox, ui->colorDataVariableGridLayout );
}

void GlyphEditor::onApply()
{
    if( !m_web_sockets->isConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    QJsonObject glyphParameter;

    glyphParameter[QString::fromUtf8( Protocol::Key::Event )]         = QString::fromUtf8( Protocol::Events::GlyphParameter );
    // Type
    glyphParameter[QString::fromUtf8( Protocol::Key::Type )]          = ui->typeComboBox->currentIndex();

    // ScaleFactor
    glyphParameter[QString::fromUtf8( Protocol::Key::ScaleFactor )]   = ui->scaleFactorDoubleSpinBox->value();

    // Direction
    glyphParameter[QString::fromUtf8( Protocol::Key::Direction1 )]    = ui->direction1ComboBox->currentIndex();
    glyphParameter[QString::fromUtf8( Protocol::Key::Direction2 )]    = ui->direction2ComboBox->currentIndex();
    glyphParameter[QString::fromUtf8( Protocol::Key::Direction3 )]    = ui->direction3ComboBox->currentIndex();

    // Size Mode
    GlyphParameter::DataMode sizeDataMode =
        ui->sizeConstantRadioButton->isChecked()
            ? GlyphParameter::DataMode::Constant
            : GlyphParameter::DataMode::VariableArray;
    glyphParameter[QString::fromUtf8( Protocol::Key::SizeMode )]      = static_cast<int>(sizeDataMode);

    // Size Variables
    QJsonArray sizeVariables;
    for( int i = 0; i < ui->sizeVariableGridLayout->count(); ++i )
    {
        if( auto* item = ui->sizeVariableGridLayout->itemAt( i ) )
        {
            if( auto* cb = qobject_cast<QComboBox*>( item->widget() ) )
                sizeVariables.append( cb->currentIndex() );
        }
    }
    glyphParameter[QString::fromUtf8( Protocol::Key::SizeVariables )] = sizeVariables;

    // Distribution
    GlyphParameter::DistributionMode distributionMode;
    if( ui->uniformRadioButton->isChecked() )
        distributionMode = GlyphParameter::DistributionMode::UniformDistribution;
    else if( ui->allPointsRadioButton->isChecked() )
        distributionMode = GlyphParameter::DistributionMode::AllPoints;
    else
        distributionMode = GlyphParameter::DistributionMode::EveryNthPoints;

    glyphParameter[QString::fromUtf8( Protocol::Key::DistributionMode )]     = static_cast<int>( distributionMode );
    glyphParameter[QString::fromUtf8( Protocol::Key::NumberOfSamplePoints )] = ui->numberOfSamplePointsSpinBox->value();
    glyphParameter[QString::fromUtf8( Protocol::Key::Seed )]                 = ui->seedSpinBox->value();
    glyphParameter[QString::fromUtf8( Protocol::Key::Stride )]               = ui->strideSpinBox->value();

    // Color Map
    QJsonArray colorMapArray;
    const auto& map = toStdVectorColors( ui->colorMap->getColors() );
    for( const auto& rgb : map )
    {
        QJsonObject rgbObj;
        rgbObj[QString::fromUtf8( Protocol::Key::R )] = static_cast<int>( rgb.r() );
        rgbObj[QString::fromUtf8( Protocol::Key::G )] = static_cast<int>( rgb.g() );
        rgbObj[QString::fromUtf8( Protocol::Key::B )] = static_cast<int>( rgb.b() );
        colorMapArray.append( rgbObj );
    }
    glyphParameter[QString::fromUtf8( Protocol::Key::ColorMap )] = colorMapArray;

    // Color Data Mode
    GlyphParameter::DataMode colorDataMode =
        ui->colorDataConstantRadioButton->isChecked()
            ? GlyphParameter::DataMode::Constant
            : GlyphParameter::DataMode::VariableArray;
    glyphParameter[QString::fromUtf8( Protocol::Key::ColorDataMode )]     = static_cast<int>(colorDataMode);

    // Color Data Variables
    QJsonArray colorDataVariables;
    for( int i = 0; i < ui->colorDataVariableGridLayout->count(); ++i )
    {
        if( auto* item = ui->colorDataVariableGridLayout->itemAt( i ) )
        {
            if( auto* cb = qobject_cast<QComboBox*>( item->widget() ) )
                colorDataVariables.append( cb->currentIndex() );
        }
    }
    glyphParameter[QString::fromUtf8( Protocol::Key::ColorDataVariables )]    = colorDataVariables;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( glyphParameter ).toJson( QJsonDocument::Compact ) );
}
