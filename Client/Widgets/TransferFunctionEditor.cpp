#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QtGlobal>

TransferFunctionEditor::TransferFunctionEditor( WebSocketPair* websockets, QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::TransferFunctionEditor )
    , m_web_sockets( websockets )
    , m_is_operator( false )
    , m_transfer_function( new TransferFunction() )
{
    ui->setupUi(this);

    // FIXME:旧サーバ(WebSocket対応前)のサーバで伝達関数が17を以上になるとクラッシュする不具合があった記憶があります
    ui->numberOfTransferFunctionSpinBox->setMaximum( 16 );

    ui->colorMap  ->setColors( toQVectorColors( std::vector<kvs::RGBColor>( 256, kvs::RGBColor( 0, 0, 0 ) ) ) );
    ui->opacityMap->setOpacities( toQVectorOpacities( std::vector<float>( 256, 0 ) ) );

    connect( ui->numberOfTransferFunctionSpinBox        , &QSpinBox::valueChanged        , this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );

    connect( ui->colorSynthesizerLineEdit               , &QLineEdit::textChanged        , this, &TransferFunctionEditor::onColorSynthesizerChanged );
    connect( ui->colorFunctionComboBox                  , &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onColorComboBoxChanged );
    connect( ui->colorFunctionVariableLineEdit          , &QLineEdit::textChanged        , this, &TransferFunctionEditor::onColorFunctionVariableChanged );
    connect( ui->colorFunctionVariableEditorPushButton  , &QPushButton::clicked          , this, &TransferFunctionEditor::onColorFunctionVariableEditorClicked );
    connect( ui->colorUserDefinedMinMaxRadioButton      , &QRadioButton::clicked         , this, &TransferFunctionEditor::onColorRangeModeRadioButtonClicked );
    connect( ui->colorServerSideMinMaxRadioButton       , &QRadioButton::clicked         , this, &TransferFunctionEditor::onColorRangeModeRadioButtonClicked );
    connect( ui->colorUserDefinedMinDoubleSpinBox       , &QDoubleSpinBox::valueChanged  , this, &TransferFunctionEditor::onColorUserDefinedMinChanged );
    connect( ui->colorUserDefinedMaxDoubleSpinBox       , &QDoubleSpinBox::valueChanged  , this, &TransferFunctionEditor::onColorUserDefinedMaxChanged );
    connect( ui->colorMapEditPushButton                 , &QPushButton::clicked          , this, &TransferFunctionEditor::onColorMapEditorClicked );

    connect( ui->opacitySynthesizerLineEdit             , &QLineEdit::textChanged        , this, &TransferFunctionEditor::onOpacitySynthesizerChanged );
    connect( ui->opacityFunctionComboBox                , &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onOpacityComboBoxChanged );
    connect( ui->opacityFunctionVariableLineEdit        , &QLineEdit::textChanged        , this, &TransferFunctionEditor::onOpacityFunctionVariableChanged );
    connect( ui->opacityFunctionVariableEditorPushButton, &QPushButton::clicked          , this, &TransferFunctionEditor::onOpacityFunctionVariableEditorClicked );
    connect( ui->opacityUserDefinedMinMaxRadioButton    , &QRadioButton::clicked         , this, &TransferFunctionEditor::onOpacityRangeModeRadioButtonClicked );
    connect( ui->opacityServerSideMinMaxRadioButton     , &QRadioButton::clicked         , this, &TransferFunctionEditor::onOpacityRangeModeRadioButtonClicked );
    connect( ui->opacityUserDefinedMinDoubleSpinBox     , &QDoubleSpinBox::valueChanged  , this, &TransferFunctionEditor::onOpacityUserDefinedMinChanged );
    connect( ui->opacityUserDefinedMaxDoubleSpinBox     , &QDoubleSpinBox::valueChanged  , this, &TransferFunctionEditor::onOpacityUserDefinedMaxChanged );
    connect( ui->opacityMapEditPushButton               , &QPushButton::clicked          , this, &TransferFunctionEditor::onOpacityMapEditorClicked );

    connect( ui->exportPushButton                       , &QPushButton::clicked          , this, &TransferFunctionEditor::onExport );
    connect( ui->importPushButton                       , &QPushButton::clicked          , this, &TransferFunctionEditor::onImport );
    connect( ui->applyPushButton                        , &QPushButton::clicked          , this, &TransferFunctionEditor::onApply );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::reset()
{
}

void TransferFunctionEditor::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    ui->numberOfTransferFunctionSpinBox->setEnabled( m_is_operator );

    ui->colorSynthesizerLineEdit             ->setEnabled( m_is_operator );
    ui->colorFunctionVariableLineEdit        ->setEnabled( m_is_operator );
    ui->colorFunctionVariableEditorPushButton->setEnabled( m_is_operator );
    ui->colorUserDefinedMinMaxRadioButton    ->setEnabled( m_is_operator );
    ui->colorServerSideMinMaxRadioButton     ->setEnabled( m_is_operator );
    ui->colorUserDefinedMinDoubleSpinBox     ->setEnabled( m_is_operator );
    ui->colorUserDefinedMaxDoubleSpinBox     ->setEnabled( m_is_operator );
    ui->colorMapEditPushButton               ->setEnabled( m_is_operator );

    ui->opacitySynthesizerLineEdit             ->setEnabled( m_is_operator );
    ui->opacityFunctionVariableLineEdit        ->setEnabled( m_is_operator );
    ui->opacityFunctionVariableEditorPushButton->setEnabled( m_is_operator );
    ui->opacityUserDefinedMinMaxRadioButton    ->setEnabled( m_is_operator );
    ui->opacityServerSideMinMaxRadioButton     ->setEnabled( m_is_operator );
    ui->opacityUserDefinedMinDoubleSpinBox     ->setEnabled( m_is_operator );
    ui->opacityUserDefinedMaxDoubleSpinBox     ->setEnabled( m_is_operator );
    ui->opacityMapEditPushButton               ->setEnabled( m_is_operator );

    ui->exportPushButton                       ->setEnabled( m_is_operator );
    ui->importPushButton                       ->setEnabled( m_is_operator );
    ui->applyPushButton                        ->setEnabled( m_is_operator );
}

void TransferFunctionEditor::onReceiveTransferFunctionParameter( const QJsonObject& payload )
{
    // NOTE:初回導通時に送られてくる初期伝達関数
    if( !m_transfer_function ) { return; }

    const auto kTFNumber              = QString::fromUtf8( "TFNumber" );
    const auto kColorSynthesizer      = QString::fromUtf8( Protocol::Key::ColorSynthesizer );
    const auto kOpacitySynthesizer    = QString::fromUtf8( Protocol::Key::OpacitySynthesizer );
    const auto kData                  = QString::fromUtf8( Protocol::Key::Data );
    const auto kIndex                 = QString::fromUtf8( "Index" );

    const auto kColorVariable         = QString::fromUtf8( Protocol::Key::ColorVariable );
    const auto kColorRangeMode        = QString::fromUtf8( Protocol::Key::ColorRangeMode );
    const auto kColorUserRangeMin     = QString::fromUtf8( Protocol::Key::ColorUserRangeMin );
    const auto kColorUserRangeMax     = QString::fromUtf8( Protocol::Key::ColorUserRangeMax );
    const auto kColorServerRangeMin   = QString::fromUtf8( Protocol::Key::ColorServerRangeMin );
    const auto kColorServerRangeMax   = QString::fromUtf8( Protocol::Key::ColorServerRangeMax );
    const auto kColorMap              = QString::fromUtf8( Protocol::Key::ColorMap );
    const auto kColorHistogram        = QString::fromUtf8( Protocol::Key::ColorHistogram );

    const auto kOpacityVariable       = QString::fromUtf8( Protocol::Key::OpacityVariable );
    const auto kOpacityRangeMode      = QString::fromUtf8( Protocol::Key::OpacityRangeMode );
    const auto kOpacityUserRangeMin   = QString::fromUtf8( Protocol::Key::OpacityUserRangeMin );
    const auto kOpacityUserRangeMax   = QString::fromUtf8( Protocol::Key::OpacityUserRangeMax );
    const auto kOpacityServerRangeMin = QString::fromUtf8( Protocol::Key::OpacityServerRangeMin );
    const auto kOpacityServerRangeMax = QString::fromUtf8( Protocol::Key::OpacityServerRangeMax );
    const auto kOpacityMap            = QString::fromUtf8( Protocol::Key::OpacityMap );
    const auto kOpacityHistogram      = QString::fromUtf8( Protocol::Key::OpacityHistogram );

    if( payload.contains( kTFNumber ) )
    {
        ui->numberOfTransferFunctionSpinBox->setValue( payload.value( kTFNumber ).toInt() );
    }

    // Synthesizer
    if( payload.contains( kColorSynthesizer ) )
    {
        m_transfer_function->setColorSynthesizer( payload.value( kColorSynthesizer ).toString().toUtf8().constData() );
        ui->colorSynthesizerLineEdit->setText( QString::fromUtf8( m_transfer_function->colorSynthesizer() ) );
    }

    if( payload.contains( kOpacitySynthesizer ) )
    {
        m_transfer_function->setOpacitySynthesizer( payload.value( kOpacitySynthesizer ).toString().toUtf8().constData() );
        ui->opacitySynthesizerLineEdit->setText( QString::fromUtf8( m_transfer_function->opacitySynthesizer() ) );
    }

    if( !payload.contains( kData ) || !payload.value( kData ).isArray() ) return;
    const QJsonArray tfArray = payload.value( kData ).toArray();

    const int curCount = static_cast<int>( m_transfer_function->count() );

    auto applyPatchToIndex = [&]( int idx, const QJsonObject& tf )
    {
        if( idx < 0 || idx >= curCount ) { return; }

        // Color
        if( tf.contains( kColorVariable ) )
        {
            m_transfer_function->at( idx ).color.variable = tf.value( kColorVariable ).toString().toUtf8().constData();
        }
        if( tf.contains( kColorRangeMode ) )
        {
            m_transfer_function->at( idx ).color.rangeMode =
                static_cast<TransferFunction::RangeMode>( tf.value( kColorRangeMode ).toInt() );
        }
        if( tf.contains( kColorUserRangeMin ) )
        {
            m_transfer_function->at( idx ).color.userDefinedMinMax.first = tf.value( kColorUserRangeMin ).toDouble();
        }
        if( tf.contains( kColorUserRangeMax ) )
        {
            m_transfer_function->at( idx ).color.userDefinedMinMax.second = tf.value( kColorUserRangeMax ).toDouble();
        }
        if( tf.contains( kColorServerRangeMin ) )
        {
            m_transfer_function->at( idx ).color.serverSideMinMax.first = tf.value( kColorServerRangeMin ).toDouble();
        }
        if( tf.contains( kColorServerRangeMax ) )
        {
            m_transfer_function->at( idx ).color.serverSideMinMax.second = tf.value( kColorServerRangeMax ).toDouble();
        }
        if( tf.contains( kColorMap ) && tf.value( kColorMap ).isArray() )
        {
            const QJsonArray colorArr = tf.value( kColorMap ).toArray();

            std::vector<kvs::RGBColor> colorMapTemp;
            colorMapTemp.reserve( colorArr.size() );

            for( const QJsonValue& rgbVal : colorArr )
            {
                if( !rgbVal.isArray() ) continue;
                const QJsonArray rgb = rgbVal.toArray();
                if( rgb.size() == 3 )
                {
                    kvs::RGBColor color(
                        static_cast<unsigned char>( rgb[0].toInt() ),
                        static_cast<unsigned char>( rgb[1].toInt() ),
                        static_cast<unsigned char>( rgb[2].toInt() ) );
                    colorMapTemp.push_back( color );
                }
            }

            m_transfer_function->at( idx ).color.map = std::move( colorMapTemp );
        }
        if( tf.contains( kColorHistogram ) && tf.value( kColorHistogram ).isArray() )
        {
            const QJsonArray colorHistArr = tf.value( kColorHistogram ).toArray();

            std::vector<int> colorHistogramTemp;
            colorHistogramTemp.reserve( colorHistArr.size() );

            for( const QJsonValue& hv : colorHistArr )
            {
                colorHistogramTemp.push_back( hv.toInt() );
            }

            m_transfer_function->at( idx ).color.histogram = std::move( colorHistogramTemp );
        }

        // Opacity
        if( tf.contains( kOpacityVariable ) )
        {
            m_transfer_function->at( idx ).opacity.variable = tf.value( kOpacityVariable ).toString().toUtf8().constData();
        }
        if( tf.contains( kOpacityRangeMode ) )
        {
            m_transfer_function->at( idx ).opacity.rangeMode =
                static_cast<TransferFunction::RangeMode>( tf.value( kOpacityRangeMode ).toInt() );
        }
        if( tf.contains( kOpacityUserRangeMin ) )
        {
            m_transfer_function->at( idx ).opacity.userDefinedMinMax.first = tf.value( kOpacityUserRangeMin ).toDouble();
        }
        if( tf.contains( kOpacityUserRangeMax ) )
        {
            m_transfer_function->at( idx ).opacity.userDefinedMinMax.second = tf.value( kOpacityUserRangeMax ).toDouble();
        }
        if( tf.contains( kOpacityServerRangeMin ) )
        {
            m_transfer_function->at( idx ).opacity.serverSideMinMax.first = tf.value( kOpacityServerRangeMin ).toDouble();
        }
        if( tf.contains( kOpacityServerRangeMax ) )
        {
            m_transfer_function->at( idx ).opacity.serverSideMinMax.second = tf.value( kOpacityServerRangeMax ).toDouble();
        }
        if( tf.contains( kOpacityMap ) && tf.value( kOpacityMap ).isArray() )
        {
            const QJsonArray opacityArr = tf.value( kOpacityMap ).toArray();

            std::vector<float> opacityMapTemp;
            opacityMapTemp.reserve( opacityArr.size() );

            for( const QJsonValue& ov : opacityArr )
            {
                opacityMapTemp.push_back( static_cast<float>( ov.toDouble() ) );
            }

            m_transfer_function->at( idx ).opacity.map = std::move( opacityMapTemp );
        }
        if( tf.contains( kOpacityHistogram ) && tf.value( kOpacityHistogram ).isArray() )
        {
            const QJsonArray opacityHistArr = tf.value( kOpacityHistogram ).toArray();

            std::vector<int> opacityHistogramTemp;
            opacityHistogramTemp.reserve( opacityHistArr.size() );

            for( const QJsonValue& hv : opacityHistArr )
            {
                opacityHistogramTemp.push_back( hv.toInt() );
            }

            m_transfer_function->at( idx ).opacity.histogram = std::move( opacityHistogramTemp );
        }
    };

    for( int i = 0; i < tfArray.size(); ++i )
    {
        const QJsonValue v = tfArray.at( i );
        if( !v.isObject() ) continue;

        const QJsonObject tf = v.toObject();

        int idx = i;
        if( tf.contains( kIndex ) )
        {
            idx = tf.value( kIndex ).toInt();
        }

        applyPatchToIndex( idx, tf );
    }

    m_last_sent_tf  = *m_transfer_function;
    m_has_last_sent = true;

    // NOTE:初回導通なので両UIを更新
    updateUIFromUserInput( UpdateTarget::Both );
    updateUIFromServer( UpdateTarget::Both );
}

void TransferFunctionEditor::onReceiveRequestDataAtTransferFunctionParameter( const QJsonObject& payload )
{    
    // NOTE:時系列更新時に遅れてくる伝達関数
    // UserRangeMode,UserRangeMin,UserRangeMaxはUIに反映しないColorMapSelectorで使用する
    if( !m_transfer_function ) return;

    const auto kData                  = QString::fromUtf8( Protocol::Key::Data );

    const auto kColorRangeMode        = QString::fromUtf8( Protocol::Key::ColorRangeMode );
    const auto kColorUserRangeMin     = QString::fromUtf8( Protocol::Key::ColorUserRangeMin );
    const auto kColorUserRangeMax     = QString::fromUtf8( Protocol::Key::ColorUserRangeMax );
    const auto kColorServerRangeMin   = QString::fromUtf8( Protocol::Key::ColorServerRangeMin );
    const auto kColorServerRangeMax   = QString::fromUtf8( Protocol::Key::ColorServerRangeMax );
    const auto kColorMap              = QString::fromUtf8( Protocol::Key::ColorMap );
    const auto kColorHistogram        = QString::fromUtf8( Protocol::Key::ColorHistogram );

    const auto kOpacityRangeMode      = QString::fromUtf8( Protocol::Key::OpacityRangeMode );
    const auto kOpacityUserRangeMin   = QString::fromUtf8( Protocol::Key::OpacityUserRangeMin );
    const auto kOpacityUserRangeMax   = QString::fromUtf8( Protocol::Key::OpacityUserRangeMax );
    const auto kOpacityServerRangeMin = QString::fromUtf8( Protocol::Key::OpacityServerRangeMin );
    const auto kOpacityServerRangeMax = QString::fromUtf8( Protocol::Key::OpacityServerRangeMax );
    const auto kOpacityHistogram      = QString::fromUtf8( Protocol::Key::OpacityHistogram );

    if( !payload.contains( kData ) || !payload.value( kData ).isArray() ) return;
    const QJsonArray tfArray = payload.value( kData ).toArray();
    const int tfNumber       = tfArray.size();

    const int colorIndex = ui->colorFunctionComboBox->currentIndex();
    if( colorIndex < 0 || colorIndex >= tfNumber ) return;

    TransferFunction legendBar;
    // LegendBar用
    for( int i = 0; i < tfNumber; ++i )
    {
        const QJsonValue v = tfArray.at( i );
        if( !v.isObject() ) continue;

        const QJsonObject tf = v.toObject();

        TransferFunction::Item item;

        // Color
        if( tf.contains( kColorRangeMode ) )
        {
            item.color.rangeMode = static_cast<TransferFunction::RangeMode>( tf.value( kColorRangeMode ).toInt() );
        }

        if( tf.contains( kColorUserRangeMin ) )
        {
            item.color.userDefinedMinMax.first = tf.value( kColorUserRangeMin ).toDouble();
        }

        if( tf.contains( kColorUserRangeMax ) )
        {
            item.color.userDefinedMinMax.second = tf.value( kColorUserRangeMax ).toDouble();
        }

        if( tf.contains( kColorServerRangeMin ) )
        {
            item.color.serverSideMinMax.first = tf.value( kColorServerRangeMin ).toDouble();
        }

        if( tf.contains( kColorServerRangeMax ) )
        {
            item.color.serverSideMinMax.second = tf.value( kColorServerRangeMax ).toDouble();
        }

        if( tf.contains( kColorMap ) && tf.value( kColorMap ).isArray() )
        {
            const QJsonArray colorArr = tf.value( kColorMap ).toArray();

            std::vector<kvs::RGBColor> colorMapTemp;
            colorMapTemp.reserve( colorArr.size() );

            for( const QJsonValue& rgbVal : colorArr )
            {
                if( !rgbVal.isArray() ) continue;
                const QJsonArray rgb = rgbVal.toArray();
                if( rgb.size() == 3 )
                {
                    kvs::RGBColor color(
                        static_cast<unsigned char>( rgb[0].toInt() ),
                        static_cast<unsigned char>( rgb[1].toInt() ),
                        static_cast<unsigned char>( rgb[2].toInt() ) );
                    colorMapTemp.push_back( color );
                }
            }

            item.color.map = std::move( colorMapTemp );
        }

        legendBar.addTransferFunction( item );
    }

    // NOTE:コンボボックスのアイテム分だけループする、それ以外は無視
    for( int i = 0; i < ui->colorFunctionComboBox->count(); ++i )
    {
        const QJsonValue v = tfArray.at( i );
        if( !v.isObject() ) continue;

        const QJsonObject tf = v.toObject();

        // Color
        // if( tf.contains( kColorRangeMode ) )
        // {
        //     m_transfer_function->at( i ).color.rangeMode = static_cast<TransferFunction::RangeMode>( tf.value( kColorRangeMode ).toInt() );
        // }

        // if( tf.contains( kColorUserRangeMin ) )
        // {
        //     m_transfer_function->at( i ).color.userDefinedMinMax.first = tf.value( kColorUserRangeMin ).toDouble();
        // }
        // if( tf.contains( kColorUserRangeMax ) )
        // {
        //     m_transfer_function->at( i ).color.userDefinedMinMax.second = tf.value( kColorUserRangeMax ).toDouble();
        // }
        if( tf.contains( kColorServerRangeMin ) )
        {
            m_transfer_function->at( i ).color.serverSideMinMax.first = tf.value( kColorServerRangeMin ).toDouble();
        }
        if( tf.contains( kColorServerRangeMax ) )
        {
            m_transfer_function->at( i ).color.serverSideMinMax.second = tf.value( kColorServerRangeMax ).toDouble();
        }
        if( tf.contains( kColorHistogram ) && tf.value( kColorHistogram ).isArray() )
        {
            const QJsonArray colorHistArr = tf.value( kColorHistogram ).toArray();

            std::vector<int> colorHistogramTemp;
            colorHistogramTemp.reserve( colorHistArr.size() );

            for( const QJsonValue& hv : colorHistArr )
            {
                colorHistogramTemp.push_back( hv.toInt() );
            }

            m_transfer_function->at( i ).color.histogram = std::move( colorHistogramTemp );
        }
        // Opacity
        // if( tf.contains( kOpacityRangeMode ) )
        // {
        //     m_transfer_function->at( i ).opacity.rangeMode = static_cast<TransferFunction::RangeMode>( tf.value( kOpacityRangeMode ).toInt() );
        // }

        // if( tf.contains( kOpacityUserRangeMin ) )
        // {
        //     m_transfer_function->at( i ).opacity.userDefinedMinMax.first = tf.value( kOpacityUserRangeMin ).toDouble();
        // }
        // if( tf.contains( kOpacityUserRangeMax ) )
        // {
        //     m_transfer_function->at( i ).opacity.userDefinedMinMax.second = tf.value( kOpacityUserRangeMax ).toDouble();
        // }
        if( tf.contains( kOpacityServerRangeMin ) )
        {
            m_transfer_function->at( i ).opacity.serverSideMinMax.first = tf.value( kOpacityServerRangeMin ).toDouble();
        }
        if( tf.contains( kOpacityServerRangeMax ) )
        {
            m_transfer_function->at( i ).opacity.serverSideMinMax.second = tf.value( kOpacityServerRangeMax ).toDouble();
        }
        if( tf.contains( kOpacityHistogram ) && tf.value( kOpacityHistogram ).isArray() )
        {
            const QJsonArray OpacityHistArr = tf.value( kOpacityHistogram ).toArray();

            std::vector<int> OpacityHistogramTemp;
            OpacityHistogramTemp.reserve( OpacityHistArr.size() );

            for( const QJsonValue& hv : OpacityHistArr )
            {
                OpacityHistogramTemp.push_back( hv.toInt() );
            }

            m_transfer_function->at( i ).opacity.histogram = std::move( OpacityHistogramTemp );
        }
    }
    updateUIFromServer( UpdateTarget::Both );
    updateLastSentTransferFunction( &legendBar );
}

void TransferFunctionEditor::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TransferFunctionEditor::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TransferFunctionEditor::updateUIFromUserInput( UpdateTarget target )
{
    // NOTE:ユーザの操作によってのみ更新されるUI
    if( !m_transfer_function ) return;

    const int tfCount = static_cast<int>( m_transfer_function->count() );

    if( target == UpdateTarget::Color || target == UpdateTarget::Both )
    { // Color
        if( ui->colorFunctionComboBox->count() < tfCount ) return;
        const int colorIndex = ui->colorFunctionComboBox->currentIndex();
        if( colorIndex < 0 || colorIndex >= tfCount )
        {
            // コンボ操作でシグナルが飛んで再入しないようにする
            const QSignalBlocker blocker( ui->colorFunctionComboBox );

            ui->colorFunctionVariableLineEdit->clear();

            ui->colorUserDefinedMinDoubleSpinBox->setValue( 0.0 );
            ui->colorUserDefinedMaxDoubleSpinBox->setValue( 0.0 );
            ui->colorServerSideMinDoubleSpinBox->setValue( 0.0 );
            ui->colorServerSideMaxDoubleSpinBox->setValue( 0.0 );

            ui->colorMap->setColors(
                toQVectorColors(
                    std::vector<kvs::RGBColor>( 256, kvs::RGBColor( 0, 0, 0 ) )
                    )
                );

            ui->colorHistogram->setDatas( {} );
            return;
        }

        const auto& c = m_transfer_function->at( static_cast<size_t>( colorIndex ) ).color;

        ui->colorFunctionVariableLineEdit    ->setText( QString::fromStdString( c.variable ) );
        ui->colorUserDefinedMinMaxRadioButton->setChecked( c.rangeMode == TransferFunction::UserRange );
        ui->colorServerSideMinMaxRadioButton ->setChecked( c.rangeMode == TransferFunction::ServerSide );
        ui->colorUserDefinedMinDoubleSpinBox ->setValue( c.userDefinedMinMax.first );
        ui->colorUserDefinedMaxDoubleSpinBox ->setValue( c.userDefinedMinMax.second );
        ui->colorMap                         ->setColors( toQVectorColors( c.map ) );
    }
    if( target == UpdateTarget::Opacity || target == UpdateTarget::Both )
    { // Opacity
        if( ui->opacityFunctionComboBox->count() < tfCount ) return;
        const int OpacityIndex = ui->opacityFunctionComboBox->currentIndex();
        if( OpacityIndex < 0 || OpacityIndex >= tfCount )
        {
            // コンボ操作でシグナルが飛んで再入しないようにする
            const QSignalBlocker blocker( ui->opacityFunctionComboBox );

            ui->opacityFunctionVariableLineEdit->clear();

            ui->opacityUserDefinedMinDoubleSpinBox->setValue( 0.0 );
            ui->opacityUserDefinedMaxDoubleSpinBox->setValue( 0.0 );
            ui->opacityServerSideMinDoubleSpinBox->setValue( 0.0 );
            ui->opacityServerSideMaxDoubleSpinBox->setValue( 0.0 );

            ui->opacityMap->setOpacities( toQVectorOpacities( std::vector<float>( 256, 0 ) ) );

            ui->opacityHistogram->setDatas( {} );
            return;
        }

        const auto& o = m_transfer_function->at( static_cast<size_t>( OpacityIndex ) ).opacity;

        ui->opacityFunctionVariableLineEdit    ->setText( QString::fromStdString( o.variable ) );
        ui->opacityUserDefinedMinMaxRadioButton->setChecked( o.rangeMode == TransferFunction::UserRange );
        ui->opacityServerSideMinMaxRadioButton ->setChecked( o.rangeMode == TransferFunction::ServerSide );
        ui->opacityUserDefinedMinDoubleSpinBox ->setValue( o.userDefinedMinMax.first );
        ui->opacityUserDefinedMaxDoubleSpinBox ->setValue( o.userDefinedMinMax.second );
        ui->opacityMap                         ->setOpacities( toQVectorOpacities( o.map ) );
    }
}

void TransferFunctionEditor::updateUIFromServer( UpdateTarget target )
{
    // NOTE:時系列更新時によってのみ更新されるUI
    if( !m_transfer_function ) return;

    const int tfCount = static_cast<int>( m_transfer_function->count() );

     // Color
    if( target == UpdateTarget::Color || target == UpdateTarget::Both )
    {
        if( ui->colorFunctionComboBox->count() < tfCount ) return;
        const int colorIndex = ui->colorFunctionComboBox->currentIndex();
        if( colorIndex < 0 || colorIndex >= tfCount ) return;

        const auto& c = m_transfer_function->at( static_cast<size_t>( colorIndex ) ).color;

        ui->colorServerSideMinDoubleSpinBox->setValue( c.serverSideMinMax.first );
        ui->colorServerSideMaxDoubleSpinBox->setValue( c.serverSideMinMax.second );
        ui->colorHistogram                 ->setDatas( c.histogram );
    }
     // Opacity
    if( target == UpdateTarget::Opacity || target == UpdateTarget::Both )
    {
        if( ui->opacityFunctionComboBox->count() < tfCount ) return;
        const int opacityIndex = ui->opacityFunctionComboBox->currentIndex();
        if( opacityIndex < 0 || opacityIndex >= tfCount ) return;

        const auto& c = m_transfer_function->at( static_cast<size_t>( opacityIndex ) ).opacity;

        ui->opacityServerSideMinDoubleSpinBox->setValue( c.serverSideMinMax.first );
        ui->opacityServerSideMaxDoubleSpinBox->setValue( c.serverSideMinMax.second );
        ui->opacityHistogram                 ->setDatas( c.histogram );
    }
}

void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( const int numberOfTransferFunction )
{
    if( !m_transfer_function ) return;

    constexpr int bins = 256;

    const int current = static_cast<int>( m_transfer_function->count() );
    const int target  = numberOfTransferFunction;

    auto makeDefaultItem = []() -> TransferFunction::Item
    {
        TransferFunction::Item item;
        item.color.variable            = "q1";
        item.color.rangeMode           = TransferFunction::UserRange;
        item.color.userDefinedMinMax   = { 0.0, 1.0 };
        item.color.serverSideMinMax    = { 0.0, 1.0 };
        item.color.map                 = toStdVectorColors( defaultColor() );
        item.color.histogram           = std::vector<int>( bins, 0 );

        item.opacity.variable          = "q1";
        item.opacity.rangeMode         = TransferFunction::UserRange;
        item.opacity.userDefinedMinMax = { 0.0, 1.0 };
        item.opacity.serverSideMinMax  = { 0.0, 1.0 };
        item.opacity.histogram         = std::vector<int>( bins, 0 );

        item.opacity.map.resize( bins );
        for( int i = 0; i < bins; ++i )
        {
            item.opacity.map[i] = static_cast<float>( i ) / static_cast<float>( bins - 1 );
        }

        return item;
    };

    if( target > current )
    {
        const int add = target - current;
        for( int i = 0; i < add; ++i )
        {
            m_transfer_function->addTransferFunction( makeDefaultItem() );
        }
    }
    else if( target < current )
    {
        const int remove = current - target;
        for( int i = 0; i < remove; ++i )
        {
            m_transfer_function->removeTransferFunction( m_transfer_function->count() - 1 );
        }
    }

    // NOTE:コンボボックスのアップデート
    const int tfCount = static_cast<int>( m_transfer_function->count() );

    auto syncCombo = [tfCount]( QComboBox* combo, const QString& prefix )
    {
        int index          = combo->currentIndex();
        const int maxIndex = tfCount - 1;

        if( tfCount <= 0 )
        {
            combo->clear();
            combo->setCurrentIndex( -1 );
            return;
        }

        if( index > maxIndex ) index = maxIndex;
        if( index < 0 )        index = 0;

        while( combo->count() > tfCount )
        {
            combo->removeItem( combo->count() - 1 );
        }

        for( int i = combo->count(); i < tfCount; ++i )
        {
            combo->addItem( QString( "%1%2" ).arg( prefix ).arg( i + 1 ) ); // NOTE:C1,C2... / O1,O2...
        }

        combo->setCurrentIndex( index );
    };

    syncCombo( ui->colorFunctionComboBox,   "C" );
    syncCombo( ui->opacityFunctionComboBox, "O" );
}

// Color
void TransferFunctionEditor::onColorSynthesizerChanged( const QString &colorSynthesizer )
{
    if( !m_transfer_function ) return;
    m_transfer_function->setColorSynthesizer( colorSynthesizer.toUtf8().constData() );
}

void TransferFunctionEditor::onColorComboBoxChanged()
{
    updateUIFromUserInput( UpdateTarget::Color );
    updateUIFromServer( UpdateTarget::Color );
}

void TransferFunctionEditor::onColorFunctionVariableChanged( const QString& colorFunctionVariable )
{
    updateSelectedColorTransferFunction( ui->colorFunctionComboBox, [&]( TransferFunction::ColorTransferFunction& c )
                                        {
                                            c.variable = colorFunctionVariable.toUtf8().constData();
                                        } );
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
                QString var = model->item( i, 1 )->text();
                if( i < static_cast<int>( m_transfer_function->count() ) )
                {
                    auto& colorItem = m_transfer_function->at( i ).color;
                    colorItem.variable = var.toUtf8().constData();
                }
            }
            updateUIFromUserInput( UpdateTarget::Color );
        }
    }
}

void TransferFunctionEditor::onColorRangeModeRadioButtonClicked()
{
    updateSelectedColorTransferFunction( ui->colorFunctionComboBox, [&]( TransferFunction::ColorTransferFunction& c )
                                        {
                                            c.rangeMode = ui->colorUserDefinedMinMaxRadioButton->isChecked()
                                            ? TransferFunction::UserRange
                                            : TransferFunction::ServerSide;
                                        } );
}

void TransferFunctionEditor::onColorUserDefinedMinChanged( const double colorUserDefinedMin )
{
    updateSelectedColorTransferFunction( ui->colorFunctionComboBox, [&]( auto& c ) { c.userDefinedMinMax.first = colorUserDefinedMin; } );
}

void TransferFunctionEditor::onColorUserDefinedMaxChanged( const double colorUserDefinedMax )
{
    updateSelectedColorTransferFunction( ui->colorFunctionComboBox, [&]( auto& c ) { c.userDefinedMinMax.second = colorUserDefinedMax; } );
}

void TransferFunctionEditor::onColorMapEditorClicked()
{
    if( !m_transfer_function ) return;

    m_color_map_editor.adjustSize();
    m_color_map_editor.setDefaultColorMap( ui->colorMap->getColors() );

    if( m_color_map_editor.exec() == QDialog::Accepted )
    {
        updateSelectedColorTransferFunction( ui->colorFunctionComboBox, [&]( auto& c )
                                            {
                                                c.map = toStdVectorColors( m_color_map_editor.getColorMap() );
                                            } );
    }

    updateUIFromUserInput( UpdateTarget::Color );
}

// Opacity
void TransferFunctionEditor::onOpacitySynthesizerChanged( const QString &opacitySynthesizer )
{
    if( !m_transfer_function ) return;
    m_transfer_function->setOpacitySynthesizer( opacitySynthesizer.toUtf8().constData() );
}

void TransferFunctionEditor::onOpacityComboBoxChanged()
{
    updateUIFromUserInput( UpdateTarget::Opacity );
    updateUIFromServer( UpdateTarget::Opacity );
}

void TransferFunctionEditor::onOpacityFunctionVariableChanged( const QString& opacityFunctionVariable )
{
    updateSelectedOpacityTransferFunction( ui->opacityFunctionComboBox, [&]( TransferFunction::OpacityTransferFunction& c )
                                          {
                                              c.variable = opacityFunctionVariable.toUtf8().constData();
                                          } );
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
                QString var = model->item( i, 1 )->text();
                if( i < static_cast<int>( m_transfer_function->count() ) )
                {
                    auto& opacityItem = m_transfer_function->at( i ).opacity;
                    opacityItem.variable = var.toUtf8().constData();
                }
            }
            updateUIFromUserInput( UpdateTarget::Opacity );
        }
    }
}

void TransferFunctionEditor::onOpacityRangeModeRadioButtonClicked()
{
    updateSelectedOpacityTransferFunction( ui->opacityFunctionComboBox, [&]( TransferFunction::OpacityTransferFunction& o )
                                          {
                                              o.rangeMode = ui->opacityUserDefinedMinMaxRadioButton->isChecked()
                                              ? TransferFunction::UserRange
                                              : TransferFunction::ServerSide;
                                          } );
}

void TransferFunctionEditor::onOpacityUserDefinedMinChanged( const double opacityUserDefinedMin )
{
    updateSelectedOpacityTransferFunction( ui->opacityFunctionComboBox, [&]( auto& o ) { o.userDefinedMinMax.first = opacityUserDefinedMin; } );
}

void TransferFunctionEditor::onOpacityUserDefinedMaxChanged( const double opacityUserDefinedMax )
{
    updateSelectedOpacityTransferFunction( ui->opacityFunctionComboBox, [&]( auto& o ) { o.userDefinedMinMax.second = opacityUserDefinedMax; } );
}

void TransferFunctionEditor::onOpacityMapEditorClicked()
{
    if( !m_transfer_function ) return;

    m_opacity_map_editor.adjustSize();
    m_opacity_map_editor.setDefaultOpacityMap( ui->opacityMap->getOpacities() );

    if( m_opacity_map_editor.exec() == QDialog::Accepted )
    {
        updateSelectedOpacityTransferFunction( ui->opacityFunctionComboBox, [&]( auto& o )
                                              {
                                                  o.map = toStdVectorOpacities( m_opacity_map_editor.getOpacityMap() );
                                              } );
    }

    updateUIFromUserInput( UpdateTarget::Opacity );
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
    updateUIFromUserInput( UpdateTarget::Both );
}

void TransferFunctionEditor::onApply()
{
    if( !m_web_sockets->isConnected() ) { return; }
    if( !m_transfer_function ) return;

    if( !m_has_last_sent )
    {
        m_last_sent_tf   = *m_transfer_function;
        m_has_last_sent  = true;
        // NOTE:初回接続時にもし差分がなかった場合の処理
        // 未実装です、必要になる場面はないかもしれません。
    }

    auto nearlyEqual = []( double a, double b )
    {
        const double eps = 1e-12;
        return std::abs(a - b) <= eps;
    };

    auto sameColorMap = []( const std::vector<kvs::RGBColor>& a, const std::vector<kvs::RGBColor>& b ) -> bool
    {
        if( a.size() != b.size() ) return false;
        for( size_t i = 0; i < a.size(); ++i )
        {
            if( a[i].red()   != b[i].red() )   return false;
            if( a[i].green() != b[i].green() ) return false;
            if( a[i].blue()  != b[i].blue() )  return false;
        }
        return true;
    };

    auto sameOpacityMap = []( const std::vector<float>& a,
                             const std::vector<float>& b ) -> bool
    {
        if( a.size() != b.size() ) return false;
        for( size_t i = 0; i < a.size(); ++i )
        {
            if( std::abs( (double)a[i] - (double)b[i] ) > 1e-7 ) return false;
        }
        return true;
    };

    const int curCount  = static_cast<int>( m_transfer_function->count() );
    const int prevCount = static_cast<int>( m_last_sent_tf.count() );

    QJsonObject transferFunctionParameter;
    transferFunctionParameter[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::TransferFunctionParameter );

    if( curCount != prevCount )
    {
        transferFunctionParameter["TFNumber"] = curCount;
    }

    // Synthesizer 差分
    const QString curColorSynth    = QString::fromUtf8( m_transfer_function->colorSynthesizer() );
    const QString curOpacitySynth  = QString::fromUtf8( m_transfer_function->opacitySynthesizer() );
    const QString prevColorSynth   = QString::fromUtf8( m_last_sent_tf.colorSynthesizer() );
    const QString prevOpacitySynth = QString::fromUtf8( m_last_sent_tf.opacitySynthesizer() );

    if( curColorSynth != prevColorSynth )
        transferFunctionParameter[QString::fromUtf8( Protocol::Key::ColorSynthesizer )] = curColorSynth;

    if( curOpacitySynth != prevOpacitySynth )
        transferFunctionParameter[QString::fromUtf8( Protocol::Key::OpacitySynthesizer )] = curOpacitySynth;

    QJsonArray patches;

    for( int i = 0; i < curCount; ++i )
    {
        QJsonObject patch;
        patch["Index"] = i;

        const bool full = ( i >= prevCount );

        const auto& curItem = m_transfer_function->at( i );
        const auto* prevItem = full ? nullptr : &m_last_sent_tf.at( i );

        const QString kColorVariable      = QString::fromUtf8( Protocol::Key::ColorVariable );
        const QString kColorRangeMode     = QString::fromUtf8( Protocol::Key::ColorRangeMode );
        const QString kColorUserRangeMin  = QString::fromUtf8( Protocol::Key::ColorUserRangeMin );
        const QString kColorUserRangeMax  = QString::fromUtf8( Protocol::Key::ColorUserRangeMax );
        const QString kColorMap           = QString::fromUtf8( Protocol::Key::ColorMap );

        const QString kOpacityVariable    = QString::fromUtf8( Protocol::Key::OpacityVariable );
        const QString kOpacityRangeMode   = QString::fromUtf8( Protocol::Key::OpacityRangeMode );
        const QString kOpacityUserRangeMin= QString::fromUtf8( Protocol::Key::OpacityUserRangeMin );
        const QString kOpacityUserRangeMax= QString::fromUtf8( Protocol::Key::OpacityUserRangeMax );
        const QString kOpacityMap         = QString::fromUtf8( Protocol::Key::OpacityMap );

        // Color
        if( full || curItem.color.variable != prevItem->color.variable )
            patch[kColorVariable] = QString::fromUtf8( curItem.color.variable );

        if( full || curItem.color.rangeMode != prevItem->color.rangeMode )
            patch[kColorRangeMode] = (int)curItem.color.rangeMode;

        if( full || !nearlyEqual( curItem.color.userDefinedMinMax.first, prevItem->color.userDefinedMinMax.first ) )
            patch[kColorUserRangeMin] = curItem.color.userDefinedMinMax.first;

        if( full || !nearlyEqual( curItem.color.userDefinedMinMax.second, prevItem->color.userDefinedMinMax.second ) )
            patch[kColorUserRangeMax] = curItem.color.userDefinedMinMax.second;

        // ColorMap(Range変更では送らない)
        if( full || !sameColorMap(curItem.color.map, prevItem->color.map) )
        {
            QJsonArray colorArray;
            const auto& cmap = curItem.color.map;
            for( const auto& c : cmap )
            {
                QJsonArray rgb;
                rgb.append( c.red() );
                rgb.append( c.green() );
                rgb.append( c.blue() );
                colorArray.append( rgb );
            }
            patch[kColorMap] = colorArray;
        }

        // Opacity
        if( full || curItem.opacity.variable != prevItem->opacity.variable )
            patch[kOpacityVariable] = QString::fromUtf8( curItem.opacity.variable );

        if( full || curItem.opacity.rangeMode != prevItem->opacity.rangeMode )
            patch[kOpacityRangeMode] = (int)curItem.opacity.rangeMode;

        if( full || !nearlyEqual( curItem.opacity.userDefinedMinMax.first, prevItem->opacity.userDefinedMinMax.first ) )
            patch[kOpacityUserRangeMin] = curItem.opacity.userDefinedMinMax.first;

        if( full || !nearlyEqual( curItem.opacity.userDefinedMinMax.second, prevItem->opacity.userDefinedMinMax.second ) )
            patch[kOpacityUserRangeMax] = curItem.opacity.userDefinedMinMax.second;

        // OpacityMap(Range変更では送らない)
        if( full || !sameOpacityMap(curItem.opacity.map, prevItem->opacity.map) )
        {
            QJsonArray opacityArray;
            const auto& omap = curItem.opacity.map;
            for( float v : omap ) opacityArray.append( v );
            patch[kOpacityMap] = opacityArray;
        }

        if( patch.keys().size() > 1 )
            patches.append( patch );
    }

    const bool hasAnyPatch = !patches.isEmpty();
    const bool hasMeta =
        transferFunctionParameter.contains("TFNumber") ||
        transferFunctionParameter.contains(QString::fromUtf8(Protocol::Key::ColorSynthesizer)) ||
        transferFunctionParameter.contains(QString::fromUtf8(Protocol::Key::OpacitySynthesizer));

    if( !hasAnyPatch && !hasMeta )
    {
        // NOTE:変更がない
        return;
    }

    transferFunctionParameter[QString::fromUtf8( Protocol::Key::Data )] = patches;

    m_web_sockets->text()->sendTextMessage(
        QJsonDocument( transferFunctionParameter ).toJson( QJsonDocument::Compact ) );

    // NOTE:最終送信内容の更新
    m_last_sent_tf  = *m_transfer_function;
    m_has_last_sent = true;

    emit transferFunctionUpdate();
    // qDebug().noquote() << "[TransferFunctionEditor::onApply] diff(pretty):\n" << QJsonDocument( transferFunctionParameter ).toJson( QJsonDocument::Indented );
}
