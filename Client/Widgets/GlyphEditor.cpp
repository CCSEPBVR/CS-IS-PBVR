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

    ui->sizeAreaScrollArea     ->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    ui->colorDataAreaScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

    // Direction
    connect( ui->direction1ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );
    connect( ui->direction2ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );
    connect( ui->direction3ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );

    // Size
    connect( ui->sizeConstantRadioButton        , &QRadioButton::clicked    , this, &GlyphEditor::onSizeUiState );
    connect( ui->sizeVariableArrayRadioButton   , &QRadioButton::clicked    , this, &GlyphEditor::onSizeUiState );
    connect( ui->sizeNumberOfVariablesSpinBox   , &QSpinBox::valueChanged   , this, &GlyphEditor::onSizeNumberOfVariableValueChanged );

    // ColorMap
    connect( ui->editColorMapPushButton, &QPushButton::clicked, this, &GlyphEditor::onColorMapEditorClicked );

    // ColorData
    connect( ui->colorDataConstantRadioButton       , &QRadioButton::clicked    , this, &GlyphEditor::onColorDataUiState );
    connect( ui->colorDataVariableArrayRadioButton  , &QRadioButton::clicked    , this, &GlyphEditor::onColorDataUiState );
    connect( ui->colorDataNumberOfVariablesSpinBox  , &QSpinBox::valueChanged   , this, &GlyphEditor::onColorDataNumberOfVariableValueChanged );

    connect( ui->applyPushButton, &QPushButton::clicked, this, &GlyphEditor::onApply );
}

GlyphEditor::~GlyphEditor()
{
    delete ui;
}

void GlyphEditor::reset()
{
    m_has_last_snap_shot = false;
}

void GlyphEditor::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    if( m_color_map_editor.isVisible() ) m_color_map_editor.close();

    ui->typeComboBox                     ->setEnabled( m_is_operator );
    ui->scaleFactorDoubleSpinBox         ->setEnabled( m_is_operator );

    ui->direction1ComboBox               ->setEnabled( m_is_operator );
    ui->direction2ComboBox               ->setEnabled( m_is_operator );
    ui->direction3ComboBox               ->setEnabled( m_is_operator );

    ui->sizeConstantRadioButton          ->setEnabled( m_is_operator );
    ui->sizeVariableArrayRadioButton     ->setEnabled( m_is_operator );
    ui->sizeNumberOfVariablesSpinBox     ->setEnabled( m_is_operator );
    for( int i = 0; i < ui->sizeVariableGridLayout->count(); ++i )
    {
        if( QLayoutItem* item = ui->sizeVariableGridLayout->itemAt( i ) )
        {
            if( QWidget* w = item->widget() )
            {
                if( auto comboBox = qobject_cast<QComboBox*>( w ) )
                    comboBox->setEnabled( m_is_operator );
            }
        }
    }

    ui->uniformRadioButton               ->setEnabled( m_is_operator );
    ui->allPointsRadioButton             ->setEnabled( m_is_operator );
    ui->everyNthPointRadioButton         ->setEnabled( m_is_operator );

    ui->numberOfSamplePointsSpinBox      ->setEnabled( m_is_operator );
    ui->seedSpinBox                      ->setEnabled( m_is_operator );
    ui->strideSpinBox                    ->setEnabled( m_is_operator );

    ui->editColorMapPushButton           ->setEnabled( m_is_operator );

    ui->colorDataConstantRadioButton     ->setEnabled( m_is_operator );
    ui->colorDataVariableArrayRadioButton->setEnabled( m_is_operator );
    ui->colorDataNumberOfVariablesSpinBox->setEnabled( m_is_operator );
    for( int i = 0; i < ui->colorDataVariableGridLayout->count(); ++i )
    {
        if( QLayoutItem* item = ui->colorDataVariableGridLayout->itemAt( i ) )
        {
            if( QWidget* w = item->widget() )
            {
                if( auto comboBox = qobject_cast<QComboBox*>( w ) )
                    comboBox->setEnabled( m_is_operator );
            }
        }
    }

    ui->applyPushButton                  ->setEnabled( m_is_operator );
}

void GlyphEditor::onUpdateNumberOfVector( const int numberOfVector )
{
    if( numberOfVector < 3 ) { return; }

    m_variable_list.clear();
    for( int i = 0; i < numberOfVector; ++i )
    {
        const QString variableName = QString( "q%1" ).arg( i + 1 );
        m_variable_list << variableName;
    }

    ui->direction1ComboBox->clear();
    ui->direction2ComboBox->clear();
    ui->direction3ComboBox->clear();

    ui->direction1ComboBox->addItems( m_variable_list );
    ui->direction2ComboBox->addItems( m_variable_list );
    ui->direction3ComboBox->addItems( m_variable_list );

    ui->sizeNumberOfVariablesSpinBox      ->setMaximum( numberOfVector );
    ui->colorDataNumberOfVariablesSpinBox ->setMaximum( numberOfVector );
}

void GlyphEditor::onReceiveGlyphParameter( const QJsonObject& payload )
{
    const auto kType                 = QString::fromUtf8( Protocol::Key::Type );
    const auto kScaleFactor          = QString::fromUtf8( Protocol::Key::ScaleFactor );
    const auto kDirection1           = QString::fromUtf8( Protocol::Key::Direction1 );
    const auto kDirection2           = QString::fromUtf8( Protocol::Key::Direction2 );
    const auto kDirection3           = QString::fromUtf8( Protocol::Key::Direction3 );
    const auto kSizeMode             = QString::fromUtf8( Protocol::Key::SizeMode );
    const auto kSizeVariables        = QString::fromUtf8( Protocol::Key::SizeVariables );
    const auto kDistributionMode     = QString::fromUtf8( Protocol::Key::DistributionMode );
    const auto kNumberOfSamplePoints = QString::fromUtf8( Protocol::Key::NumberOfSamplePoints );
    const auto kSeed                 = QString::fromUtf8( Protocol::Key::Seed );
    const auto kStride               = QString::fromUtf8( Protocol::Key::Stride );
    const auto kColorMap             = QString::fromUtf8( Protocol::Key::ColorMap );
    const auto kColorDataMode        = QString::fromUtf8( Protocol::Key::ColorDataMode );
    const auto kColorDataVariables   = QString::fromUtf8( Protocol::Key::ColorDataVariables );

    // Type
    if( payload.contains( kType ) )
    {
        ui->typeComboBox->setCurrentIndex( payload.value( kType ).toInt() );
    }

    // ScaleFactor
    if( payload.contains( kScaleFactor ) )
    {
        ui->scaleFactorDoubleSpinBox->setValue( payload.value( kScaleFactor ).toDouble() );
    }

    // Direction
    if( payload.contains( kDirection1 ) && payload.contains( kDirection2 ) && payload.contains( kDirection3 ) )
    {
        ui->direction1ComboBox->setCurrentIndex( payload.value( kDirection1 ).toInt() );
        ui->direction2ComboBox->setCurrentIndex( payload.value( kDirection2 ).toInt() );
        ui->direction3ComboBox->setCurrentIndex( payload.value( kDirection3 ).toInt() );
    }

    // SizeMode
    if( payload.contains( kSizeMode ) )
    {
        const int rawSizeMode = payload.value( kSizeMode ).toInt();
        const auto sizeDataMode = static_cast<GlyphParameter::DataMode>( rawSizeMode );

        ui->sizeConstantRadioButton     ->setChecked( sizeDataMode == GlyphParameter::DataMode::Constant );
        ui->sizeVariableArrayRadioButton->setChecked( sizeDataMode == GlyphParameter::DataMode::VariableArray );
    }

    // SizeVariables
    if( payload.contains( kSizeVariables ) && payload.value( kSizeVariables ).isArray() )
    {
        const QJsonArray sizeVariables = payload.value( kSizeVariables ).toArray();
        ui->sizeNumberOfVariablesSpinBox->setValue( sizeVariables.size() );

        int sizeComboIndex = 0;
        for( int i = 0; i < ui->sizeVariableGridLayout->count() && sizeComboIndex < sizeVariables.size(); ++i )
        {
            auto* item = ui->sizeVariableGridLayout->itemAt( i );
            if( !item ) continue;

            if( auto* comboBox = qobject_cast<QComboBox*>( item->widget() ) )
            {
                const QSignalBlocker b( comboBox );
                comboBox->setCurrentIndex( sizeVariables.at( sizeComboIndex ).toInt() );
                ++sizeComboIndex;
            }
        }
    }

    // Distribution
    if( payload.contains( kDistributionMode ) )
    {
        const auto distributionMode =
            static_cast<GlyphParameter::DistributionMode>( payload.value( kDistributionMode ).toInt() );

        ui->uniformRadioButton     ->setChecked( distributionMode == GlyphParameter::DistributionMode::UniformDistribution );
        ui->allPointsRadioButton   ->setChecked( distributionMode == GlyphParameter::DistributionMode::AllPoints );
        ui->everyNthPointRadioButton->setChecked( distributionMode == GlyphParameter::DistributionMode::EveryNthPoints );
    }

    if( payload.contains( kNumberOfSamplePoints ) )
    {
        ui->numberOfSamplePointsSpinBox->setValue( payload.value( kNumberOfSamplePoints ).toInt() );
    }
    if( payload.contains( kSeed ) )
    {
        ui->seedSpinBox->setValue( payload.value( kSeed ).toInt() );
    }
    if( payload.contains( kStride ) )
    {
        ui->strideSpinBox->setValue( payload.value( kStride ).toInt() );
    }

    // ColorMap
    if( payload.contains( kColorMap ) && payload.value( kColorMap ).isArray() )
    {
        const QJsonArray colorMapJson = payload.value( kColorMap ).toArray();
        QVector<QColor> colors;

        if( !colorMapJson.isEmpty() && colorMapJson.at(0).isObject() )
        {
            // 形式A: [{R,G,B}, ...]
            colors.reserve( colorMapJson.size() );
            for( const auto& c : colorMapJson )
            {
                const QJsonObject obj = c.toObject();
                const int r = obj.value( QString::fromUtf8( Protocol::Key::R ) ).toInt();
                const int g = obj.value( QString::fromUtf8( Protocol::Key::G ) ).toInt();
                const int b = obj.value( QString::fromUtf8( Protocol::Key::B ) ).toInt();
                colors.append( QColor( r, g, b ) );
            }
        }
        else
        {
            // 形式B: [r,g,b,r,g,b,...]
            const int n = colorMapJson.size() / 3;
            colors.reserve( n );
            for( int i = 0; i + 2 < colorMapJson.size(); i += 3 )
            {
                const int r = colorMapJson.at( i ).toInt();
                const int g = colorMapJson.at( i + 1 ).toInt();
                const int b = colorMapJson.at( i + 2 ).toInt();
                colors.append( QColor( r, g, b ) );
            }
        }

        ui->colorMap->setColors( colors );
    }

    // ColorDataMode
    if( payload.contains( kColorDataMode ) )
    {
        const int rawColorDataMode = payload.value( kColorDataMode ).toInt();
        const auto colorDataMode = static_cast<GlyphParameter::DataMode>( rawColorDataMode );

        ui->colorDataConstantRadioButton     ->setChecked( colorDataMode == GlyphParameter::DataMode::Constant );
        ui->colorDataVariableArrayRadioButton->setChecked( colorDataMode == GlyphParameter::DataMode::VariableArray );
    }

    // ColorDataVariables
    if( payload.contains( kColorDataVariables ) && payload.value( kColorDataVariables ).isArray() )
    {
        const QJsonArray colorDataVariables = payload.value( kColorDataVariables ).toArray();
        ui->colorDataNumberOfVariablesSpinBox->setValue( colorDataVariables.size() );

        int colorDataComboIndex = 0;
        for( int i = 0; i < ui->colorDataVariableGridLayout->count() && colorDataComboIndex < colorDataVariables.size(); ++i )
        {
            auto* item = ui->colorDataVariableGridLayout->itemAt( i );
            if( !item ) continue;

            if( auto* comboBox = qobject_cast<QComboBox*>( item->widget() ) )
            {
                const QSignalBlocker b( comboBox );
                comboBox->setCurrentIndex( colorDataVariables.at( colorDataComboIndex ).toInt() );
                ++colorDataComboIndex;
            }
        }
    }

    m_last_snap_shot     = captureUiSnapshot();
    m_has_last_snap_shot = true;
}

void GlyphEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void GlyphEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

bool GlyphEditor::fuzzyEqual( double a, double b )
{
    // 0付近も考慮した簡易版（必要なら閾値で調整してOK）
    const double diff = qAbs( a - b );
    const double scale = qMax( 1.0, qMax( qAbs( a ), qAbs( b ) ) );
    return diff <= 1e-12 * scale;
}

QVector<int> GlyphEditor::captureVariableIndicesFromGrid( const QGridLayout* grid, int count )
{
    QVector<int> out;
    out.reserve( count );

    for( int i = 0; i < count; ++i )
    {
        auto* item = grid->itemAtPosition( i, 1 );
        if( !item ) { out.push_back( 0 ); continue; }

        if( auto* combo = qobject_cast<QComboBox*>( item->widget() ) )
        {
            out.push_back( combo->currentIndex() );
        }
        else
        {
            out.push_back( 0 );
        }
    }

    return out;
}

GlyphEditor::GlyphUiSnapshot GlyphEditor::captureUiSnapshot() const
{
    GlyphUiSnapshot s;

    s.type = ui->typeComboBox->currentIndex();
    s.scaleFactor = ui->scaleFactorDoubleSpinBox->value();

    s.dir1 = ui->direction1ComboBox->currentIndex();
    s.dir2 = ui->direction2ComboBox->currentIndex();
    s.dir3 = ui->direction3ComboBox->currentIndex();

    s.sizeMode =
        ui->sizeConstantRadioButton->isChecked()
            ? GlyphParameter::DataMode::Constant
            : GlyphParameter::DataMode::VariableArray;

    {
        const int n = ui->sizeNumberOfVariablesSpinBox->value();
        s.sizeVariables = captureVariableIndicesFromGrid( ui->sizeVariableGridLayout, n );
    }

    if( ui->uniformRadioButton->isChecked() )
        s.distributionMode = GlyphParameter::DistributionMode::UniformDistribution;
    else if( ui->allPointsRadioButton->isChecked() )
        s.distributionMode = GlyphParameter::DistributionMode::AllPoints;
    else
        s.distributionMode = GlyphParameter::DistributionMode::EveryNthPoints;

    s.numberOfSamplePoints = ui->numberOfSamplePointsSpinBox->value();
    s.seed = ui->seedSpinBox->value();
    s.stride = ui->strideSpinBox->value();

    s.colorMap = ui->colorMap->getColors();

    s.colorDataMode =
        ui->colorDataConstantRadioButton->isChecked()
            ? GlyphParameter::DataMode::Constant
            : GlyphParameter::DataMode::VariableArray;

    {
        const int n = ui->colorDataNumberOfVariablesSpinBox->value();
        s.colorDataVariables = captureVariableIndicesFromGrid( ui->colorDataVariableGridLayout, n );
    }

    return s;
}

void GlyphEditor::updateUiEnableState( const bool isVariableArray, QGridLayout* gridLayout )
{
    const bool enabled = m_is_operator && isVariableArray;

    for( int i = 0; i < gridLayout->count(); ++i )
    {
        if( QLayoutItem* item = gridLayout->itemAt( i ) )
        {
            if( QWidget* w = item->widget() ) w->setEnabled( enabled );
        }
    }
}

void GlyphEditor::updateVariableGridLayout( QSpinBox* spinBox, QGridLayout* grid )
{
    const int newCount = spinBox->value();

    int currentCount = 0;
    for( ;; ++currentCount )
    {
        auto* item = grid->itemAtPosition( currentCount, 1 );
        if( !item || !qobject_cast<QComboBox*>( item->widget() ) ) break;
    }

    for( int i = currentCount; i < newCount; ++i )
    {
        auto* label = new QLabel( QStringLiteral( "Variable%1" ).arg( i + 1 ), this );

        auto* combo = new QComboBox( this );
        combo->addItems( m_variable_list );
        combo->setEnabled( m_is_operator );

        grid->addWidget( label, i, 0 );
        grid->addWidget( combo, i, 1 );
    }

    for( int i = currentCount - 1; i >= newCount; --i )
    {
        if( auto* item = grid->itemAtPosition( i, 0 ) )
        {
            if( QWidget* w = item->widget() ) delete w;
        }
        if( auto* item = grid->itemAtPosition( i, 1 ) )
        {
            if( QWidget* w = item->widget() ) delete w;
        }
    }
}


void GlyphEditor::onDirectionComboBoxIndexChanged()
{
    QList<QComboBox*> comboBoxes =
        {
            ui->direction1ComboBox,
            ui->direction2ComboBox,
            ui->direction3ComboBox
        };

    QSet<QString> used;

    for( QComboBox* combo : comboBoxes )
    {
        const QString value = combo->currentText();

        if( value.isEmpty() ) { continue; }

        if( used.contains( value ) )
        {
            for( const QString& candidate : m_variable_list )
            {
                if( !used.contains( candidate ) )
                {
                    combo->blockSignals( true );
                    combo->setCurrentIndex( combo->findText( candidate ) );
                    combo->blockSignals( false );
                    used.insert( candidate );
                    break;
                }
            }
        }
        else
        {
            used.insert( value );
        }
    }
}

void GlyphEditor::onSizeUiState()
{
    updateUiEnableState( ui->sizeVariableArrayRadioButton->isChecked(), ui->sizeVariableGridLayout );
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
    updateUiEnableState( ui->colorDataVariableArrayRadioButton->isChecked(), ui->colorDataVariableGridLayout );
}

void GlyphEditor::onColorDataNumberOfVariableValueChanged()
{
    updateVariableGridLayout( ui->colorDataNumberOfVariablesSpinBox, ui->colorDataVariableGridLayout );
}

void GlyphEditor::onApply()
{
    if( !m_web_sockets->isConnected() ) return;

    if( !m_has_last_snap_shot )
    {
        m_last_snap_shot     = captureUiSnapshot();
        m_has_last_snap_shot = true;
        return;
    }

    const auto kEvent                = QString::fromUtf8( Protocol::Key::Event );
    const auto kType                 = QString::fromUtf8( Protocol::Key::Type );
    const auto kScaleFactor          = QString::fromUtf8( Protocol::Key::ScaleFactor );
    const auto kDirection1           = QString::fromUtf8( Protocol::Key::Direction1 );
    const auto kDirection2           = QString::fromUtf8( Protocol::Key::Direction2 );
    const auto kDirection3           = QString::fromUtf8( Protocol::Key::Direction3 );
    const auto kSizeMode             = QString::fromUtf8( Protocol::Key::SizeMode );
    const auto kSizeVariables        = QString::fromUtf8( Protocol::Key::SizeVariables );
    const auto kDistributionMode     = QString::fromUtf8( Protocol::Key::DistributionMode );
    const auto kNumberOfSamplePoints = QString::fromUtf8( Protocol::Key::NumberOfSamplePoints );
    const auto kSeed                 = QString::fromUtf8( Protocol::Key::Seed );
    const auto kStride               = QString::fromUtf8( Protocol::Key::Stride );
    const auto kColorMap             = QString::fromUtf8( Protocol::Key::ColorMap );
    const auto kColorDataMode        = QString::fromUtf8( Protocol::Key::ColorDataMode );
    const auto kColorDataVariables   = QString::fromUtf8( Protocol::Key::ColorDataVariables );

    const GlyphUiSnapshot now = captureUiSnapshot();
    QJsonObject diff;

    auto putInt = [&]( const QString& key, int v )
    {
        diff.insert( key, v );
    };
    auto putDouble = [&]( const QString& key, double v )
    {
        diff.insert( key, v );
    };
    auto putIntArray = [&]( const QString& key, const QVector<int>& vec )
    {
        QJsonArray a;
        for( int v : vec ) a.append( v );
        diff.insert( key, a );
    };

    // Type
    if( now.type != m_last_snap_shot.type ) putInt( kType, now.type );

    // ScaleFactor
    if( !fuzzyEqual( now.scaleFactor, m_last_snap_shot.scaleFactor ) ) putDouble( kScaleFactor, now.scaleFactor );

    // Direction
    if( now.dir1 != m_last_snap_shot.dir1 ) putInt( kDirection1, now.dir1 );
    if( now.dir2 != m_last_snap_shot.dir2 ) putInt( kDirection2, now.dir2 );
    if( now.dir3 != m_last_snap_shot.dir3 ) putInt( kDirection3, now.dir3 );

    // Size
    if( now.sizeMode != m_last_snap_shot.sizeMode ) putInt( kSizeMode, static_cast<int>( now.sizeMode ) );

    if( now.sizeVariables != m_last_snap_shot.sizeVariables ) putIntArray( kSizeVariables, now.sizeVariables );

    // Distribution
    if( now.distributionMode != m_last_snap_shot.distributionMode ) putInt( kDistributionMode, static_cast<int>( now.distributionMode ) );

    if( now.numberOfSamplePoints != m_last_snap_shot.numberOfSamplePoints ) putInt( kNumberOfSamplePoints, now.numberOfSamplePoints );

    if( now.seed != m_last_snap_shot.seed ) putInt( kSeed, now.seed );

    if( now.stride != m_last_snap_shot.stride ) putInt( kStride, now.stride );

    // ColorMap
    if( now.colorMap != m_last_snap_shot.colorMap )
    {
        QJsonArray a;
        for( const auto& c : now.colorMap )
        {
            a.append( c.red() );
            a.append( c.green() );
            a.append( c.blue() );
        }
        diff.insert( kColorMap, a );
    }

    // ColorData
    if( now.colorDataMode != m_last_snap_shot.colorDataMode ) putInt( kColorDataMode, static_cast<int>( now.colorDataMode ) );

    if( now.colorDataVariables != m_last_snap_shot.colorDataVariables ) putIntArray( kColorDataVariables, now.colorDataVariables );

    if( diff.isEmpty() ) return;

    QJsonObject glyphParameter;
    glyphParameter.insert( kEvent, QString::fromUtf8( Protocol::Events::GlyphParameter ) );

    for( auto it = diff.begin(); it != diff.end(); ++it )
        glyphParameter.insert( it.key(), it.value() );

    m_web_sockets->text()->sendTextMessage( QJsonDocument( glyphParameter ).toJson( QJsonDocument::Compact ) );
    emit glyphParameterUpdate();

    m_last_snap_shot     = now;
    m_has_last_snap_shot = true;
    qDebug().noquote() << "[GlyphEditor::onApply] diff(pretty):\n" << QJsonDocument( glyphParameter ).toJson( QJsonDocument::Indented );
}
