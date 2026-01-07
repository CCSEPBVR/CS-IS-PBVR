#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"

TransferFunctionEditor::TransferFunctionEditor( WebSocketPair* websockets, QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::TransferFunctionEditor )
    , m_web_sockets( websockets )
    , m_is_operator( false )
    , m_transfer_function( new TransferFunction() )
{
    ui->setupUi( this );

    ui->numberOfTransferFunctionSpinBox->setMaximum( 16 ); // FIXME:旧サーバ(WebSocket対応前のサーバでは伝達関数の最大値が17を超えるとクラッシュするという不具合があったはずです)

    ui->colorMap->setColors( toQVectorColors( std::vector<kvs::RGBColor>( 256, kvs::RGBColor( 0, 0, 0 ) ) ) );
    ui->opacityMap->setOpacities( toQVectorOpacities( std::vector<float>( 256, 0 ) ) );

    connect( ui->numberOfTransferFunctionSpinBox        , &QSpinBox::valueChanged           , this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );

    connect( ui->colorSynthesizerLineEdit               , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onColorSynthesizerChanged );
    connect( ui->colorFunctionComboBox                  , &QComboBox::currentIndexChanged   , this, &TransferFunctionEditor::onColorComboBoxChanged );
    connect( ui->colorFunctionVariableLineEdit          , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onColorFunctionVariableChanged );
    connect( ui->colorFunctionVariableEditorPushButton  , &QPushButton::clicked             , this, &TransferFunctionEditor::onColorFunctionVariableEditorClicked );
    connect( ui->colorUserDefinedMinMaxRadioButton      , &QRadioButton::clicked            , this, &TransferFunctionEditor::onColorRangeModeRadioButtonClicked );
    connect( ui->colorServerSideMinMaxRadioButton       , &QRadioButton::clicked            , this, &TransferFunctionEditor::onColorRangeModeRadioButtonClicked );
    connect( ui->colorUserDefinedMinDoubleSpinBox       , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onColorUserDefinedMinChanged );
    connect( ui->colorUserDefinedMaxDoubleSpinBox       , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onColorUserDefinedMaxChanged );
    connect( ui->colorMapEditPushButton                 , &QPushButton::clicked             , this, &TransferFunctionEditor::onColorMapEditorClicked );

    connect( ui->opacitySynthesizerLineEdit             , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onOpacitySynthesizerChanged );
    connect( ui->opacityFunctionComboBox                , &QComboBox::currentIndexChanged   , this, &TransferFunctionEditor::onOpacityComboBoxChanged );
    connect( ui->opacityFunctionVariableLineEdit        , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onOpacityFunctionVariableChanged );
    connect( ui->opacityFunctionVariableEditorPushButton, &QPushButton::clicked             , this, &TransferFunctionEditor::onOpacityFunctionVariableEditorClicked );
    connect( ui->opacityUserDefinedMinMaxRadioButton    , &QRadioButton::clicked            , this, &TransferFunctionEditor::onOpacityRangeModeRadioButtonClicked );
    connect( ui->opacityServerSideMinMaxRadioButton     , &QRadioButton::clicked            , this, &TransferFunctionEditor::onOpacityRangeModeRadioButtonClicked );
    connect( ui->opacityUserDefinedMinDoubleSpinBox     , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onOpacityUserDefinedMinChanged );
    connect( ui->opacityUserDefinedMaxDoubleSpinBox     , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onOpacityUserDefinedMaxChanged );
    connect( ui->opacityMapEditPushButton               , &QPushButton::clicked             , this, &TransferFunctionEditor::onOpacityMapEditorClicked );

    connect( ui->exportPushButton                       , &QPushButton::clicked             , this, &TransferFunctionEditor::onExport );
    connect( ui->importPushButton                       , &QPushButton::clicked             , this, &TransferFunctionEditor::onImport );
    connect( ui->applyPushButton                        , &QPushButton::clicked             , this, &TransferFunctionEditor::onApply );

    updateUI();
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::onOperatorStateUpdate( bool operatorState )
{
    m_is_operator = operatorState;
    updateUI();
}

void TransferFunctionEditor::onReset()
{
    ui->numberOfTransferFunctionSpinBox->setValue( 0 );
}

void TransferFunctionEditor::onReceiveInitializeTransferFunctionParameter( const QJsonObject& dataArray )
{
    ui->numberOfTransferFunctionSpinBox->setValue( dataArray[ QString::fromUtf8(Protocol::Key::ColorVariable) ].toArray().size() );
    ui->colorSynthesizerLineEdit->setText( dataArray[ QString::fromUtf8(Protocol::Key::ColorSynthesizer) ].toString().toUtf8().constData() );
    ui->opacitySynthesizerLineEdit->setText( dataArray[ QString::fromUtf8(Protocol::Key::OpacitySynthesizer) ].toString().toUtf8().constData() );

    for( int i = 0; i < dataArray[ QString::fromUtf8(Protocol::Key::ColorVariable) ].toArray().size(); ++i )
    {
        m_transfer_function->at( i ).color.variable                 = dataArray.value( QString::fromUtf8( Protocol::Key::ColorVariable ) ).toArray().at( i ).toString().toUtf8().constData();
        m_transfer_function->at( i ).color.rangeMode                = static_cast<TransferFunction::RangeMode>( dataArray.value( QString::fromUtf8( Protocol::Key::ColorRangeMode ) ).toArray().at( i ).toInt() );
        m_transfer_function->at( i ).color.userDefinedMinMax.first  = dataArray.value( QString::fromUtf8( Protocol::Key::ColorUserRangeMin ) ).toArray().at( i ).toDouble();
        m_transfer_function->at( i ).color.userDefinedMinMax.second = dataArray.value( QString::fromUtf8( Protocol::Key::ColorUserRangeMax ) ).toArray().at( i ).toDouble();
        m_transfer_function->at( i ).color.serverSideMinMax.first   = dataArray.value( QString::fromUtf8( Protocol::Key::ColorServerRangeMin ) ).toArray().at( i ).toDouble();
        m_transfer_function->at( i ).color.serverSideMinMax.second  = dataArray.value( QString::fromUtf8( Protocol::Key::ColorServerRangeMax ) ).toArray().at( i ).toDouble();

        std::vector<kvs::RGBColor> colorMapTemp;
        for( const QJsonValue& rgbVal : dataArray[ QString::fromUtf8( Protocol::Key::ColorMap ) ].toArray() )
        {
            QJsonArray rgb = rgbVal.toArray();
            if( rgb.size() == 3 )
            {
                kvs::RGBColor c(
                    static_cast<unsigned char>( rgb[0].toInt() ),
                    static_cast<unsigned char>( rgb[1].toInt() ),
                    static_cast<unsigned char>( rgb[2].toInt() )
                    );
                colorMapTemp.push_back( c );
            }
        }
        m_transfer_function->at( i ).color.map = colorMapTemp;


        std::vector<int> colorHistogramTemp;
        for(const QJsonValue& v : dataArray.value( QString::fromUtf8( Protocol::Key::ColorHistogram ) ).toArray() )
        {
            colorHistogramTemp.push_back( v.toInt() );
        }
        m_transfer_function->at( i ).color.histogram = colorHistogramTemp;

        m_transfer_function->at( i ).opacity.variable                 = dataArray.value( QString::fromUtf8( Protocol::Key::OpacityVariable ) ).toArray().at( i ).toString().toUtf8().constData();
        m_transfer_function->at( i ).opacity.rangeMode                = static_cast<TransferFunction::RangeMode>( dataArray.value( QString::fromUtf8( Protocol::Key::OpacityRangeMode ) ).toArray().at( i ).toInt() );
        m_transfer_function->at( i ).opacity.userDefinedMinMax.first  = dataArray.value( QString::fromUtf8( Protocol::Key::OpacityUserRangeMin ) ).toArray().at( i ).toDouble();
        m_transfer_function->at( i ).opacity.userDefinedMinMax.second = dataArray.value( QString::fromUtf8( Protocol::Key::OpacityUserRangeMax ) ).toArray().at( i ).toDouble();
        m_transfer_function->at( i ).opacity.serverSideMinMax.first   = dataArray.value( QString::fromUtf8( Protocol::Key::OpacityServerRangeMin ) ).toArray().at( i ).toDouble();
        m_transfer_function->at( i ).opacity.serverSideMinMax.second  = dataArray.value( QString::fromUtf8( Protocol::Key::OpacityServerRangeMax ) ).toArray().at( i ).toDouble();

        std::vector<float> opacityMapTemp;
        for( const QJsonValue& v : dataArray[ QString::fromUtf8( Protocol::Key::OpacityMap ) ].toArray() )
        {
            opacityMapTemp.push_back( static_cast<float>( v.toDouble() ) );
        }
        m_transfer_function->at( i ).opacity.map = opacityMapTemp;


        std::vector<int> opacityHistogramTemp;
        for(const QJsonValue& v : dataArray.value( QString::fromUtf8( Protocol::Key::OpacityHistogram ) ).toArray() )
        {
            opacityHistogramTemp.push_back( v.toInt() );
        }
        m_transfer_function->at( i ).opacity.histogram = opacityHistogramTemp;
    }

    updateUI();
}

void TransferFunctionEditor::onReceiveTransferFunctionParameter( const QString& colorSynth, const QString& opacitySynth, const QJsonArray& dataArray )
{
    ui->numberOfTransferFunctionSpinBox->setValue( dataArray.size() );

    // Synthesizer 情報を UI に反映
    ui->colorSynthesizerLineEdit->setText( colorSynth );
    ui->opacitySynthesizerLineEdit->setText( opacitySynth );

    for( int i = 0; i < dataArray.size(); ++i )
    {
        QJsonObject tf = dataArray[i].toObject();

        // Color
        m_transfer_function->at( i ).color.variable                 = tf.value( QString::fromUtf8( Protocol::Key::ColorVariable ) ).toString().toUtf8().constData();
        m_transfer_function->at( i ).color.rangeMode                = static_cast<TransferFunction::RangeMode>( tf.value( QString::fromUtf8( Protocol::Key::ColorRangeMode ) ).toInt() );
        m_transfer_function->at( i ).color.userDefinedMinMax.first  = tf.value( QString::fromUtf8( Protocol::Key::ColorUserRangeMin ) ).toDouble();
        m_transfer_function->at( i ).color.userDefinedMinMax.second = tf.value( QString::fromUtf8( Protocol::Key::ColorUserRangeMax ) ).toDouble();
        m_transfer_function->at( i ).color.serverSideMinMax.first   = tf.value( QString::fromUtf8( Protocol::Key::ColorServerRangeMin ) ).toDouble();
        m_transfer_function->at( i ).color.serverSideMinMax.second  = tf.value( QString::fromUtf8( Protocol::Key::ColorServerRangeMax ) ).toDouble();

        QJsonArray colorArr = tf.value( QString::fromUtf8( Protocol::Key::ColorMap ) ).toArray();
        std::vector<kvs::RGBColor> colorMapTemp;
        for( const QJsonValue& rgbVal : colorArr )
        {
            QJsonArray rgb = rgbVal.toArray();
            if( rgb.size() == 3 )
            {
                kvs::RGBColor color(
                    static_cast<unsigned char>( rgb[0].toInt() ),
                    static_cast<unsigned char>( rgb[1].toInt() ),
                    static_cast<unsigned char>( rgb[2].toInt() )
                    );
                colorMapTemp.push_back( color );
            }
        }
        m_transfer_function->at( i ).color.map = colorMapTemp;

        // Opacity
        m_transfer_function->at( i ).opacity.variable                 = tf.value( QString::fromUtf8( Protocol::Key::OpacityVariable ) ).toString().toUtf8().constData();
        m_transfer_function->at( i ).opacity.rangeMode                = static_cast<TransferFunction::RangeMode>( tf.value( QString::fromUtf8( Protocol::Key::OpacityRangeMode ) ).toInt() );
        m_transfer_function->at( i ).opacity.userDefinedMinMax.first  = tf.value( QString::fromUtf8( Protocol::Key::OpacityUserRangeMin ) ).toDouble();
        m_transfer_function->at( i ).opacity.userDefinedMinMax.second = tf.value( QString::fromUtf8( Protocol::Key::OpacityUserRangeMax ) ).toDouble();
        m_transfer_function->at( i ).opacity.serverSideMinMax.first   = tf.value( QString::fromUtf8( Protocol::Key::OpacityServerRangeMin ) ).toDouble();
        m_transfer_function->at( i ).opacity.serverSideMinMax.second  = tf.value( QString::fromUtf8( Protocol::Key::OpacityServerRangeMax ) ).toDouble();

        QJsonArray opacityArr = tf.value( QString::fromUtf8( Protocol::Key::OpacityMap ) ).toArray();
        std::vector<float> opacityMapTemp;
        for( const QJsonValue& v : opacityArr )
        {
            opacityMapTemp.push_back( static_cast<float>( v.toDouble() ) );
        }
        m_transfer_function->at( i ).opacity.map = opacityMapTemp;
    }

    updateUI();
}

void TransferFunctionEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TransferFunctionEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TransferFunctionEditor::updateUI()
{
    if( !m_transfer_function ) return;

    if( m_transfer_function->count() == 0 )
    {
        clear();
        disable();
        if( m_is_operator )
        {
            ui->numberOfTransferFunctionSpinBox->setEnabled( true );
            ui->importPushButton->setEnabled( true );
        }
        else
        {
            ui->numberOfTransferFunctionSpinBox->setEnabled( false );
            ui->importPushButton->setEnabled( false );
        }
    }
    else
    {
        if( m_is_operator )
        {
            enable();
            ui->numberOfTransferFunctionSpinBox->setEnabled( true );
            ui->importPushButton->setEnabled( true );
        }
        else
        {
            disable();
            ui->numberOfTransferFunctionSpinBox->setEnabled( false );
            ui->importPushButton->setEnabled( false );
        }
    }

    int colorIndex      = ui->colorFunctionComboBox->currentIndex();
    int opacityIndex    = ui->opacityFunctionComboBox->currentIndex();

    // Color
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        const auto& colorItem = m_transfer_function->at( colorIndex ).color;
        ui->colorFunctionVariableLineEdit       ->setText( QString::fromUtf8( colorItem.variable ) );
        ui->colorUserDefinedMinMaxRadioButton   ->setChecked( colorItem.rangeMode == TransferFunction::UserRange );
        ui->colorServerSideMinMaxRadioButton    ->setChecked( colorItem.rangeMode == TransferFunction::ServerSide );
        ui->colorUserDefinedMinDoubleSpinBox    ->setValue( colorItem.userDefinedMinMax.first );
        ui->colorUserDefinedMaxDoubleSpinBox    ->setValue( colorItem.userDefinedMinMax.second );
        ui->colorServerSideMinDoubleSpinBox     ->setValue( colorItem.serverSideMinMax.first );
        ui->colorServerSideMaxDoubleSpinBox     ->setValue( colorItem.serverSideMinMax.second );
        ui->colorMap                            ->setColors( toQVectorColors( colorItem.map ) );
        ui->colorHistogram                      ->setDatas( colorItem.histogram );
    }

    // Opacity
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        const auto& opacityItem = m_transfer_function->at( opacityIndex ).opacity;
        ui->opacityFunctionVariableLineEdit     ->setText( QString::fromUtf8( opacityItem.variable ) );
        ui->opacityUserDefinedMinMaxRadioButton ->setChecked( opacityItem.rangeMode == TransferFunction::UserRange );
        ui->opacityServerSideMinMaxRadioButton  ->setChecked( opacityItem.rangeMode == TransferFunction::ServerSide );
        ui->opacityUserDefinedMinDoubleSpinBox  ->setValue( opacityItem.userDefinedMinMax.first );
        ui->opacityUserDefinedMaxDoubleSpinBox  ->setValue( opacityItem.userDefinedMinMax.second );
        ui->opacityServerSideMinDoubleSpinBox   ->setValue( opacityItem.serverSideMinMax.first );
        ui->opacityServerSideMaxDoubleSpinBox   ->setValue( opacityItem.serverSideMinMax.second );
        ui->opacityMap                          ->setOpacities( toQVectorOpacities( opacityItem.map ) );
        ui->opacityHistogram                    ->setDatas( opacityItem.histogram );
    }
}

void TransferFunctionEditor::clear()
{
    // Color
    ui->colorSynthesizerLineEdit            ->clear();
    ui->colorFunctionVariableLineEdit       ->clear();

    QButtonGroup* colorRadioButtonGroup = new QButtonGroup( this );
    colorRadioButtonGroup                   ->setExclusive( false );
    colorRadioButtonGroup                   ->addButton( ui->colorUserDefinedMinMaxRadioButton );
    colorRadioButtonGroup                   ->addButton( ui->colorServerSideMinMaxRadioButton );
    colorRadioButtonGroup                   ->setExclusive( true );

    ui->colorUserDefinedMinMaxRadioButton   ->setChecked( false );
    ui->colorServerSideMinMaxRadioButton    ->setChecked( false );
    ui->colorUserDefinedMinDoubleSpinBox    ->setValue( 0.0 );
    ui->colorUserDefinedMaxDoubleSpinBox    ->setValue( 0.0 );
    ui->colorServerSideMinDoubleSpinBox     ->setValue( 0.0 );
    ui->colorServerSideMaxDoubleSpinBox     ->setValue( 0.0 );
    ui->colorMap                            ->setColors( toQVectorColors( std::vector<kvs::RGBColor>( 256, kvs::RGBColor( 0, 0, 0 ) ) ) );
    ui->colorHistogram                      ->setDatas( std::vector<int>( 256,  0 ) );

    // Opacity
    ui->opacitySynthesizerLineEdit          ->clear();
    ui->opacityFunctionVariableLineEdit     ->clear();

    QButtonGroup* opacityRadioButtonGroup = new QButtonGroup( this );
    opacityRadioButtonGroup                 ->setExclusive( false );
    opacityRadioButtonGroup                 ->addButton( ui->opacityUserDefinedMinMaxRadioButton );
    opacityRadioButtonGroup                 ->addButton( ui->opacityServerSideMinMaxRadioButton );
    opacityRadioButtonGroup                 ->setExclusive( true );

    ui->opacityUserDefinedMinMaxRadioButton ->setChecked( false );
    ui->opacityServerSideMinMaxRadioButton  ->setChecked( false );
    ui->opacityUserDefinedMinDoubleSpinBox  ->setValue( 0.0 );
    ui->opacityUserDefinedMaxDoubleSpinBox  ->setValue( 0.0 );
    ui->opacityServerSideMinDoubleSpinBox   ->setValue( 0.0 );
    ui->opacityServerSideMaxDoubleSpinBox   ->setValue( 0.0 );
    ui->opacityMap                          ->setOpacities( toQVectorOpacities( std::vector<float>( 256, 0 ) ) );
    ui->opacityHistogram                    ->setDatas( std::vector<int>( 256,  0 ) );
}

void TransferFunctionEditor::disable()
{
    ui->colorSynthesizerLineEdit                ->setEnabled( false );
    ui->colorFunctionVariableLineEdit           ->setEnabled( false );
    ui->colorFunctionVariableEditorPushButton   ->setEnabled( false );
    ui->colorUserDefinedMinMaxRadioButton       ->setEnabled( false );
    ui->colorServerSideMinMaxRadioButton        ->setEnabled( false );
    ui->colorUserDefinedMinDoubleSpinBox        ->setEnabled( false );
    ui->colorUserDefinedMaxDoubleSpinBox        ->setEnabled( false );
    ui->colorServerSideMinDoubleSpinBox         ->setEnabled( false );
    ui->colorServerSideMaxDoubleSpinBox         ->setEnabled( false );
    ui->colorMapEditPushButton                  ->setEnabled( false );

    ui->opacitySynthesizerLineEdit              ->setEnabled( false );
    ui->opacityFunctionVariableLineEdit         ->setEnabled( false );
    ui->opacityFunctionVariableEditorPushButton ->setEnabled( false );
    ui->opacityUserDefinedMinMaxRadioButton     ->setEnabled( false );
    ui->opacityServerSideMinMaxRadioButton      ->setEnabled( false );
    ui->opacityUserDefinedMinDoubleSpinBox      ->setEnabled( false );
    ui->opacityUserDefinedMaxDoubleSpinBox      ->setEnabled( false );
    ui->opacityServerSideMinDoubleSpinBox       ->setEnabled( false );
    ui->opacityServerSideMaxDoubleSpinBox       ->setEnabled( false );
    ui->opacityMapEditPushButton                ->setEnabled( false );

    ui->exportPushButton                        ->setEnabled( false );
    ui->applyPushButton                         ->setEnabled( false );
}

void TransferFunctionEditor::enable()
{
    ui->colorSynthesizerLineEdit                ->setEnabled( true );
    ui->colorFunctionVariableLineEdit           ->setEnabled( true );
    ui->colorFunctionVariableEditorPushButton   ->setEnabled( true );
    ui->colorUserDefinedMinMaxRadioButton       ->setEnabled( true );
    ui->colorServerSideMinMaxRadioButton        ->setEnabled( true );
    ui->colorUserDefinedMinDoubleSpinBox        ->setEnabled( true );
    ui->colorUserDefinedMaxDoubleSpinBox        ->setEnabled( true );
    ui->colorServerSideMinDoubleSpinBox         ->setEnabled( true );
    ui->colorServerSideMaxDoubleSpinBox         ->setEnabled( true );
    ui->colorMapEditPushButton                  ->setEnabled( true );

    ui->opacitySynthesizerLineEdit              ->setEnabled( true );
    ui->opacityFunctionVariableLineEdit         ->setEnabled( true );
    ui->opacityFunctionVariableEditorPushButton ->setEnabled( true );
    ui->opacityUserDefinedMinMaxRadioButton     ->setEnabled( true );
    ui->opacityServerSideMinMaxRadioButton      ->setEnabled( true );
    ui->opacityUserDefinedMinDoubleSpinBox      ->setEnabled( true );
    ui->opacityUserDefinedMaxDoubleSpinBox      ->setEnabled( true );
    ui->opacityServerSideMinDoubleSpinBox       ->setEnabled( true );
    ui->opacityServerSideMaxDoubleSpinBox       ->setEnabled( true );
    ui->opacityMapEditPushButton                ->setEnabled( true );

    ui->exportPushButton                        ->setEnabled( true );
    ui->applyPushButton                         ->setEnabled( true );
}

void TransferFunctionEditor::applyTransferFunction()
{
    // FIXME:差分更新(送信受信側)を実装してください
    if( !m_web_sockets->isConnected() )
    {
        qDebug() << "Not connected";
        return;
    }

    QJsonArray transferFunctionsArray;
    for( int i = 0; i < m_transfer_function->count(); ++i )
    {
        QJsonObject tfObj;

        QString colorName = QString( "C%1" ).arg( i + 1 );
        tfObj[QString::fromUtf8( Protocol::Key::ColorFunction )] = colorName;
        tfObj[QString::fromUtf8( Protocol::Key::ColorVariable )] = QString::fromUtf8( m_transfer_function->at( i ).color.variable );
        // tfObj["TemporaryColorRangeMode"] = static_cast<int>( tf.color.rangeModeTemp );
        // tfObj["CurrentColorRangeMode"] = static_cast<int>( tf.color.rangeModeCurrent );
        // tfObj["ResultColorRangeMode"] = static_cast<int>( tf.color.rangeModeResult );
        tfObj[QString::fromUtf8( Protocol::Key::ColorRangeMode )]       = m_transfer_function->at( i ).color.rangeMode;
        tfObj[QString::fromUtf8( Protocol::Key::ColorUserRangeMin )]    = m_transfer_function->at( i ).color.userDefinedMinMax.first;
        tfObj[QString::fromUtf8( Protocol::Key::ColorUserRangeMax )]    = m_transfer_function->at( i ).color.userDefinedMinMax.second;
        // tfObj["ColorServerRangeMin"]    = m_transfer_function->at( i ).color.serverSideMinMax.first;
        // tfObj["ColorServerRangeMax"]    = m_transfer_function->at( i ).color.serverSideMinMax.second;

        QJsonArray colorMapArray;

        const auto& map = m_transfer_function->at(i).color.map;
        for( const auto& rgb : map )
        {
            QJsonObject rgbObj;
            rgbObj[QString::fromUtf8( Protocol::Key::R )] = static_cast<int>( rgb.r() );
            rgbObj[QString::fromUtf8( Protocol::Key::G )] = static_cast<int>( rgb.g() );
            rgbObj[QString::fromUtf8( Protocol::Key::B )] = static_cast<int>( rgb.b() );
            colorMapArray.append( rgbObj );
        }

        tfObj[QString::fromUtf8( Protocol::Key::ColorMap )] = colorMapArray;

        QJsonArray colorArray;
        for( const auto& c : m_transfer_function->at( i ).color.map )
        {
            QJsonArray rgb;
            rgb.append( c.red() );
            rgb.append( c.green() );
            rgb.append( c.blue() );
            colorArray.append( rgb );
        }
        tfObj["ColorMap"] = colorArray;

        QString opacityName = QString( "O%1" ).arg( i + 1 );
        tfObj[QString::fromUtf8( Protocol::Key::OpacityFunction )] = opacityName.toUtf8().constData();
        tfObj[QString::fromUtf8( Protocol::Key::OpacityVariable )] = QString::fromUtf8( m_transfer_function->at( i ).opacity.variable );
        // tfObj["TemporaryOopacityRangeMode"] = static_cast<int>( tf.opacity.rangeModeTemp );
        // tfObj["CurrentOpacityRangeMode"] = static_cast<int>( tf.opacity.rangeModeCurrent );
        // tfObj["ResultOpacityRangeMode"] = static_cast<int>( tf.opacity.rangeModeResult );
        tfObj[QString::fromUtf8( Protocol::Key::OpacityRangeMode )]     = m_transfer_function->at( i ).opacity.rangeMode;
        tfObj[QString::fromUtf8( Protocol::Key::OpacityUserRangeMin )]  = m_transfer_function->at( i ).opacity.userDefinedMinMax.first;
        tfObj[QString::fromUtf8( Protocol::Key::OpacityUserRangeMax )]  = m_transfer_function->at( i ).opacity.userDefinedMinMax.second;
        // tfObj["OpacityServerRangeMin"]    = m_transfer_function->at( i ).opacity.serverSideMinMax.first;
        // tfObj["OpacityServerRangeMax"]    = m_transfer_function->at( i ).opacity.serverSideMinMax.second;

        QJsonArray opacityArray;
        for( float v : m_transfer_function->at( i ).opacity.map )
        {
            opacityArray.append( v );
        }
        tfObj[QString::fromUtf8( Protocol::Key::OpacityMap )] = opacityArray;

        transferFunctionsArray.append( tfObj );
    }

    QJsonObject root;
    root[QString::fromUtf8( Protocol::Key::Event )]               = QString::fromUtf8( Protocol::Events::TransferFunctionParameter );
    root[QString::fromUtf8( Protocol::Key::ColorSynthesizer )]   = QString::fromUtf8( m_transfer_function->colorSynthesizer() );
    root[QString::fromUtf8( Protocol::Key::OpacitySynthesizer )] = QString::fromUtf8( m_transfer_function->opacitySynthesizer() );
    root[QString::fromUtf8( Protocol::Key::Data )]                = transferFunctionsArray;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}

void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( const int numberOfTransferFunction )
{
    if( !m_transfer_function ) return;

    int currentCount = static_cast<int>( m_transfer_function->count() );

    if( numberOfTransferFunction > currentCount )
    {
        // 追加
        for( int i = 0; i < numberOfTransferFunction - currentCount; ++i )
        {
            TransferFunction::Item item;

            item.color.variable             = "q1";
            item.color.rangeMode            = TransferFunction::UserRange;
            item.color.userDefinedMinMax    = std::pair<double,double>( 0, 1 );
            item.color.serverSideMinMax     = std::pair<double,double>( 0, 1 );
            item.color.map                  = toStdVectorColors( defaultColor() );
            item.color.histogram            = std::vector<int>( 256,  0 );

            item.opacity.variable           = "q1";
            item.opacity.rangeMode          = TransferFunction::UserRange;
            item.opacity.userDefinedMinMax  = std::pair<double,double>( 0, 1 );
            item.opacity.serverSideMinMax   = std::pair<double,double>( 0, 1 );
            item.opacity.map.resize( 256 );
            for( int i = 0; i < 256; ++i )
            {
                item.opacity.map[i] = static_cast<float>( i ) / 255.0f;
            }
            item.opacity.histogram          = std::vector<int>( 256,  0 );

            m_transfer_function->addTransferFunction( item );
        }
    }
    else if( numberOfTransferFunction < currentCount )
    {
        // 削除
        for( int i = 0; i < currentCount - numberOfTransferFunction; ++i )
        {
            m_transfer_function->removeTransferFunction( m_transfer_function->count() - 1 );
        }
    }

    // 以下でコンボボックスの更新
    ui->colorFunctionComboBox->blockSignals( true );
    ui->opacityFunctionComboBox->blockSignals( true );

    size_t tfCount = m_transfer_function->count();

    // Color
    int currentColorIndex      = ui->colorFunctionComboBox->currentIndex();
    int newColorMaxIndex       = static_cast<int>( tfCount ) - 1;

    // 範囲から外れていれば丸める
    if( currentColorIndex > newColorMaxIndex ) currentColorIndex = newColorMaxIndex;
    if( currentColorIndex < 0 )                currentColorIndex = 0;

    // アイテム削除
    while( ui->colorFunctionComboBox->count() > static_cast<int>( tfCount ) )
    {
        ui->colorFunctionComboBox->removeItem( ui->colorFunctionComboBox->count() - 1 );
    }

    // アイテム追加
    for( size_t i = ui->colorFunctionComboBox->count(); i < tfCount; ++i )
    {
        ui->colorFunctionComboBox->addItem( QString( "C%1" ).arg( i + 1 ) );
    }

    // 選択復元
    ui->colorFunctionComboBox->setCurrentIndex( currentColorIndex );

    // Opacity
    int currentOpacityIndex    = ui->opacityFunctionComboBox->currentIndex();
    int newOpacityMaxIndex     = static_cast<int>( tfCount ) - 1;

    if( currentOpacityIndex > newOpacityMaxIndex ) currentOpacityIndex = newOpacityMaxIndex;
    if( currentOpacityIndex < 0 )                  currentOpacityIndex = 0;

    while( ui->opacityFunctionComboBox->count() > static_cast<int>( tfCount ) )
    {
        ui->opacityFunctionComboBox->removeItem( ui->opacityFunctionComboBox->count() - 1 );
    }

    for( size_t i = ui->opacityFunctionComboBox->count(); i < tfCount; ++i )
    {
        ui->opacityFunctionComboBox->addItem( QString( "O%1" ).arg( i + 1 ) );
    }

    ui->opacityFunctionComboBox->setCurrentIndex( currentOpacityIndex );

    ui->colorFunctionComboBox->blockSignals( false );
    ui->opacityFunctionComboBox->blockSignals( false );

    // UI更新
    updateUI();
    emit transferFunctionUpdate();
}

void TransferFunctionEditor::onColorSynthesizerChanged( const QString &colorSynthesizer )
{
    if( !m_transfer_function ) return;
    m_transfer_function->setColorSynthesizer( colorSynthesizer.toUtf8().constData() );
}

void TransferFunctionEditor::onColorComboBoxChanged()
{
    updateUI();
}

void TransferFunctionEditor::onColorFunctionVariableChanged( const QString &colorFunctionVariable )
{
    if( !m_transfer_function ) return;
    int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& colorItem = m_transfer_function->at( colorIndex ).color;
        colorItem.variable = colorFunctionVariable.toUtf8().constData();
    }
}

void TransferFunctionEditor::onColorFunctionVariableEditorClicked()
{
    if( !m_transfer_function ) return;
    int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        m_variable_editor.setup( VariableEditor::COLOR, m_transfer_function );
        m_variable_editor.initialize();
        m_variable_editor.adjustSize();

        if( m_variable_editor.exec() == QDialog::Accepted )
        {
            QStandardItemModel* model = m_variable_editor.model();
            for( int i = 0; i < model->rowCount(); ++i )
            {
                QString var = model->item( i, 1 )->text(); // 1列目がVariable
                if( i < static_cast<int>( m_transfer_function->count() ) )
                {
                    auto& colorItem = m_transfer_function->at( i ).color;
                    colorItem.variable = var.toUtf8().constData();
                }
            }
            updateUI();
        }
    }
}

void TransferFunctionEditor::onColorRangeModeRadioButtonClicked()
{
    if( !m_transfer_function ) return;
    int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& colorItem = m_transfer_function->at( colorIndex ).color;
        if( ui->colorUserDefinedMinMaxRadioButton->isChecked() )
        {
            colorItem.rangeMode = TransferFunction::UserRange;
        }
        else
        {
            colorItem.rangeMode = TransferFunction::ServerSide;
        }
    }
}

void TransferFunctionEditor::onColorUserDefinedMinChanged( const double &colorUserDefinedMin )
{
    if( !m_transfer_function ) return;
    int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& colorItem = m_transfer_function->at( colorIndex ).color;
        colorItem.userDefinedMinMax.first = colorUserDefinedMin;
    }
}

void TransferFunctionEditor::onColorUserDefinedMaxChanged( const double &colorUserDefinedMax )
{
    if( !m_transfer_function ) return;
    int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& colorItem = m_transfer_function->at( colorIndex ).color;
        colorItem.userDefinedMinMax.second = colorUserDefinedMax;
    }
}

void TransferFunctionEditor::onColorMapEditorClicked()
{
    if( !m_transfer_function ) return;
    int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex >= 0 && colorIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        m_color_map_editor.adjustSize();
        m_color_map_editor.setDefaultColorMap( ui->colorMap->getColors() );

        if( m_color_map_editor.exec() == QDialog::Accepted )
        {
            auto& colorItem = m_transfer_function->at( colorIndex ).color;
            colorItem.map = toStdVectorColors( m_color_map_editor.getColorMap() );
        }
        updateUI();
    }
}

// Opacity
void TransferFunctionEditor::onOpacitySynthesizerChanged( const QString &opacitySynthesizer )
{
    if( !m_transfer_function ) return;
    m_transfer_function->setOpacitySynthesizer( opacitySynthesizer.toUtf8().constData() );
}

void TransferFunctionEditor::onOpacityComboBoxChanged()
{
    updateUI();
}

void TransferFunctionEditor::onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable )
{
    if( !m_transfer_function ) return;
    int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& opacityItem = m_transfer_function->at( opacityIndex ).opacity;
        opacityItem.variable = opacityFunctionVariable.toUtf8().constData();
    }
}

void TransferFunctionEditor::onOpacityFunctionVariableEditorClicked()
{
    if( !m_transfer_function ) return;
    int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        m_variable_editor.setup( VariableEditor::OPACITY, m_transfer_function );
        m_variable_editor.initialize();
        m_variable_editor.adjustSize();

        if( m_variable_editor.exec() == QDialog::Accepted )
        {
            QStandardItemModel* model = m_variable_editor.model();
            for( int i = 0; i < model->rowCount(); ++i )
            {
                QString var = model->item( i, 1 )->text(); // 1列目がVariable
                if( i < static_cast<int>( m_transfer_function->count() ) )
                {
                    auto& opacityItem = m_transfer_function->at( i ).opacity;
                    opacityItem.variable = var.toUtf8().constData();
                }
            }
            updateUI();
        }
    }
}

void TransferFunctionEditor::onOpacityRangeModeRadioButtonClicked()
{
    if( !m_transfer_function ) return;
    int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& opacityItem = m_transfer_function->at( opacityIndex ).opacity;
        if( ui->opacityUserDefinedMinMaxRadioButton->isChecked() )
        {
            opacityItem.rangeMode = TransferFunction::UserRange;
        }
        else
        {
            opacityItem.rangeMode = TransferFunction::ServerSide;
        }
    }
}

void TransferFunctionEditor::onOpacityUserDefinedMinChanged( const double &opacityUserDefinedMin )
{
    if( !m_transfer_function ) return;
    int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& opacityItem = m_transfer_function->at( opacityIndex ).opacity;
        opacityItem.userDefinedMinMax.first = opacityUserDefinedMin;
    }
}

void TransferFunctionEditor::onOpacityUserDefinedMaxChanged( const double &opacityUserDefinedMax )
{
    if( !m_transfer_function ) return;
    int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        auto& opacityItem = m_transfer_function->at( opacityIndex ).opacity;
        opacityItem.userDefinedMinMax.second = opacityUserDefinedMax;
    }
}

void TransferFunctionEditor::onOpacityMapEditorClicked()
{
    if( !m_transfer_function ) return;
    int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
    if( opacityIndex >= 0 && opacityIndex < static_cast<int>( m_transfer_function->count() ) )
    {
        m_opacity_map_editor.adjustSize();
        m_opacity_map_editor.setDefaultOpacityMap( ui->opacityMap->getOpacities() );

        if( m_opacity_map_editor.exec() == QDialog::Accepted )
        {
            auto& opacityItem = m_transfer_function->at( opacityIndex ).opacity;
            opacityItem.map = toStdVectorOpacities( m_opacity_map_editor.getOpacityMap() );
        }
        updateUI();
    }
}

void TransferFunctionEditor::onExport()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr( "Export Transfer Function File" ), ".", tr( "Transfer Function Files (*.tfe *.TFE *.tf *.TF)" ) );

    if( fileName.isEmpty() ) return;

    QFile file( fileName );
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;

    QTextStream out( &file );

    // 基本情報
    out << "TF_RESOLUTION=" << 256 << "\n";
    out << "TF_NUMBER=" << m_transfer_function->count() << "\n";
    out << "TF_SYNTH_C=" << ui->colorSynthesizerLineEdit->text() << "\n";
    out << "TF_SYNTH_O=" << ui->opacitySynthesizerLineEdit->text() << "\n";

    for( int i = 0; i < m_transfer_function->count(); ++i )
    {
        const auto& tf = m_transfer_function->at( i );
        int idx = i + 1;

        // 色情報
        out << "TF_NAME" << idx << "_C=" << idx << "\n";
        out << "TF_NAME" << idx << "_VAR_C=" << QString::fromUtf8( tf.color.variable ) << "\n";
        out << "TF_NAME" << idx << "_MIN_C=" << tf.color.userDefinedMinMax.first << "\n";
        out << "TF_NAME" << idx << "_MAX_C=" << tf.color.userDefinedMinMax.second << "\n";

        if( !tf.color.map.empty() )
        {
            QStringList colorTable;
            for( const auto& c : tf.color.map )
            {
                colorTable << QString::number( c.red() )
                           << QString::number( c.green() )
                           << QString::number( c.blue() );
            }
            out << "TF_NAME" << idx << "_TABLE_C=" << colorTable.join( "," ) << "\n";
        }

        // 不透明度情報
        out << "TF_NAME" << idx << "_O=" << idx << "\n";
        out << "TF_NAME" << idx << "_VAR_O=" << QString::fromUtf8( tf.opacity.variable ) << "\n";
        out << "TF_NAME" << idx << "_MIN_O=" << tf.opacity.userDefinedMinMax.first << "\n";
        out << "TF_NAME" << idx << "_MAX_O=" << tf.opacity.userDefinedMinMax.second << "\n";

        if( !tf.opacity.map.empty() )
        {
            QStringList opacityTable;
            for( float v : tf.opacity.map )
            {
                opacityTable << QString::number( v, 'g', 8 );
            }
            out << "TF_NAME" << idx << "_TABLE_O=" << opacityTable.join( "," ) << "\n";
        }
    }

    file.close();
}

void TransferFunctionEditor::onImport()
{
    QFileDialog fileDialog( this, tr( "Import Transfer Function File" ), ".", tr( "Transfer Function Files (*.tfe *.TFE *.tf *.TF)" ) );
    fileDialog.setFileMode( QFileDialog::ExistingFile );

    if( !fileDialog.exec() ) return;

    QString selectedFile = fileDialog.selectedFiles().first();
    QFile file( selectedFile );
    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

    QTextStream in( &file );

    QMap<QString, std::function<void( const QString& ) >> keyHandlers = {
        { "TF_NUMBER", [this]( const QString& value ) {
             int number = value.toInt();
             ui->numberOfTransferFunctionSpinBox->setValue( number );
         } },
        { "TF_SYNTH_C", [this]( const QString& value ) {
             ui->colorSynthesizerLineEdit->setText( value );
         } },
        { "TF_SYNTH_O", [this]( const QString& value ) {
             ui->opacitySynthesizerLineEdit->setText( value );
         } },
    };

    QRegularExpression tfRegex( R"(TF_NAME(\d+)_([A-Z]+)_([CO]))" );

    while( !in.atEnd() )
    {
        QString line = in.readLine().trimmed();
        if( !line.contains( '=' ) ) continue;

        QString key = line.section( '=', 0, 0 );
        QString value = line.section( '=', 1 );

        if( keyHandlers.contains( key ) )
        {
            keyHandlers[key]( value );
            continue;
        }

        QRegularExpressionMatch match = tfRegex.match( key );
        if( !match.hasMatch() ) continue;

        int index = match.captured( 1 ).toInt() - 1;
        QString field = match.captured( 2 );
        QString type = match.captured( 3 ); // C or O

        if( type == "C" )
        {
            if( field == "VAR" )        m_transfer_function->at( index ).color.variable = value.toUtf8().constData();
            else if( field == "MIN" )   m_transfer_function->at( index ).color.userDefinedMinMax.first = value.toDouble();
            else if( field == "MAX" )   m_transfer_function->at( index ).color.userDefinedMinMax.second = value.toDouble();
            else if( field == "TABLE" )
            {
                QStringList strValues = value.split( ',', Qt::SkipEmptyParts );
                QVector<QColor> colors;
                for( int j = 0; j + 2 < strValues.size(); j += 3 )
                {
                    colors.append( QColor( strValues[j].toInt(), strValues[j+1].toInt(), strValues[j+2].toInt() ) );
                }
                m_transfer_function->at( index ).color.map = toStdVectorColors( colors );
            }
        }
        else if( type == "O" )
        {
            if( field == "VAR" )        m_transfer_function->at( index ).opacity.variable = value.toUtf8().constData();
            else if( field == "MIN" )   m_transfer_function->at( index ).opacity.userDefinedMinMax.first = value.toDouble();
            else if( field == "MAX" )   m_transfer_function->at( index ).opacity.userDefinedMinMax.second = value.toDouble();
            else if( field == "TABLE" )
            {
                QStringList strValues = value.split( ',', Qt::SkipEmptyParts );
                QVector<float> table;
                for( const QString& s : strValues )
                {
                    table.append( s.toDouble() );
                }
                m_transfer_function->at( index ).opacity.map = toStdVectorOpacities( table );
            }
        }
    }
    updateUI();
}

void TransferFunctionEditor::onApply()
{
    applyTransferFunction();
}
