#include "EnsembleTransferFunctionEditor.h"
#include "ui_EnsembleTransferFunctionEditor.h"

#include <QButtonGroup>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QRadioButton>

#include "../../Shared/JsonKeys.h"

namespace
{
QJsonArray ColorMapToJson( const QVector<QColor>& colors )
{
    QJsonArray array;
    for ( const auto& color : colors )
    {
        QJsonArray rgb;
        rgb.append( color.red() );
        rgb.append( color.green() );
        rgb.append( color.blue() );
        array.append( rgb );
    }
    return array;
}

QJsonArray OpacityMapToJson( const QVector<float>& opacities )
{
    QJsonArray array;
    for ( const float opacity : opacities ) array.append( opacity );
    return array;
}
}

EnsembleTransferFunctionEditor::EnsembleTransferFunctionEditor( WebSocketPair* websockets, QWidget* parent )
    : QDialog( parent )
    , ui( new Ui::EnsembleTransferFunctionEditor )
    , m_web_sockets( websockets )
{
    ui->setupUi( this );
    initializeButtonGroups();

    connect( ui->m_apply_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onApply );
    connect( ui->m_average_radio_button, &QRadioButton::toggled, this, [this]( bool checked )
    {
        if ( checked ) emit statisticChanged( selectedStatistic() );
    } );
    connect( ui->m_variance_radio_button, &QRadioButton::toggled, this, [this]( bool checked )
    {
        if ( checked ) emit statisticChanged( selectedStatistic() );
    } );
    connect( ui->m_coefficient_variation_radio_button, &QRadioButton::toggled, this, [this]( bool checked )
    {
        if ( checked ) emit statisticChanged( selectedStatistic() );
    } );
}

EnsembleTransferFunctionEditor::~EnsembleTransferFunctionEditor()
{
    delete ui;
}

void EnsembleTransferFunctionEditor::initializeButtonGroups()
{
    m_statistics_group = new QButtonGroup( this );
    m_statistics_group->setObjectName( QStringLiteral( "StatisticsGroup" ) );
    m_statistics_group->addButton( ui->m_average_radio_button );
    m_statistics_group->addButton( ui->m_variance_radio_button );
    m_statistics_group->addButton( ui->m_coefficient_variation_radio_button );
    m_statistics_group->setExclusive( true );

    m_average_min_max_group = new QButtonGroup( this );
    m_average_min_max_group->setObjectName( QStringLiteral( "AverageMinMaxGroup" ) );
    m_average_min_max_group->addButton( ui->m_average_server_min_max_radio_button );
    m_average_min_max_group->addButton( ui->m_average_user_min_max_radio_button );
    m_average_min_max_group->setExclusive( true );

    m_variance_min_max_group = new QButtonGroup( this );
    m_variance_min_max_group->setObjectName( QStringLiteral( "VarianceMinMaxGroup" ) );
    m_variance_min_max_group->addButton( ui->m_variance_server_min_max_radio_button );
    m_variance_min_max_group->addButton( ui->m_variance_user_min_max_radio_button );
    m_variance_min_max_group->setExclusive( true );

    m_coefficient_variation_min_max_group = new QButtonGroup( this );
    m_coefficient_variation_min_max_group->setObjectName( QStringLiteral( "CoefficientVariationMinMaxGroup" ) );
    m_coefficient_variation_min_max_group->addButton( ui->m_coefficient_variation_server_min_max_radio_button );
    m_coefficient_variation_min_max_group->addButton( ui->m_coefficient_variation_user_min_max_radio_button );
    m_coefficient_variation_min_max_group->setExclusive( true );
}

void EnsembleTransferFunctionEditor::setRepeatLevel( size_t repeatLevel )
{
    m_repeat_level = repeatLevel;
}

QString EnsembleTransferFunctionEditor::selectedStatistic() const
{
    if ( ui->m_variance_radio_button->isChecked() ) return QStringLiteral( "variance" );
    if ( ui->m_coefficient_variation_radio_button->isChecked() ) return QStringLiteral( "cv" );
    return QStringLiteral( "mean" );
}

void EnsembleTransferFunctionEditor::onApply()
{
    if ( !m_web_sockets || !m_web_sockets->isConnected() ) return;

    auto appendStatisticPatch = []( QJsonArray& data,
                                    const QString& statistic,
                                    bool useServerRange,
                                    double userMin,
                                    double userMax,
                                    double serverMin,
                                    double serverMax,
                                    const QVector<QColor>& colors,
                                    const QVector<float>& opacities )
    {
        const int rangeMode = useServerRange ? 2 : 1;

        QJsonObject patch;
        patch[QString::fromUtf8( Protocol::Key::Statistic )] = statistic;
        patch[QString::fromUtf8( Protocol::Key::Index )] = 0;
        patch[QString::fromUtf8( Protocol::Key::ColorVariable )] = QStringLiteral( "q1" );
        patch[QString::fromUtf8( Protocol::Key::ColorRangeMode )] = rangeMode;
        patch[QString::fromUtf8( Protocol::Key::ColorUserRangeMin )] = userMin;
        patch[QString::fromUtf8( Protocol::Key::ColorUserRangeMax )] = userMax;
        patch[QString::fromUtf8( Protocol::Key::ColorServerRangeMin )] = serverMin;
        patch[QString::fromUtf8( Protocol::Key::ColorServerRangeMax )] = serverMax;
        patch[QString::fromUtf8( Protocol::Key::ColorMap )] = ColorMapToJson( colors );
        patch[QString::fromUtf8( Protocol::Key::OpacityVariable )] = QStringLiteral( "q1" );
        patch[QString::fromUtf8( Protocol::Key::OpacityRangeMode )] = rangeMode;
        patch[QString::fromUtf8( Protocol::Key::OpacityUserRangeMin )] = userMin;
        patch[QString::fromUtf8( Protocol::Key::OpacityUserRangeMax )] = userMax;
        patch[QString::fromUtf8( Protocol::Key::OpacityServerRangeMin )] = serverMin;
        patch[QString::fromUtf8( Protocol::Key::OpacityServerRangeMax )] = serverMax;
        patch[QString::fromUtf8( Protocol::Key::OpacityMap )] = OpacityMapToJson( opacities );
        data.append( patch );
    };

    QJsonArray data;
    appendStatisticPatch(
        data,
        QStringLiteral( "mean" ),
        ui->m_average_server_min_max_radio_button->isChecked(),
        ui->m_average_user_min_double_spin_box->value(),
        ui->m_average_user_max_double_spin_box->value(),
        ui->m_average_server_min_double_spin_box->value(),
        ui->m_average_server_max_double_spin_box->value(),
        ui->m_average_color_map->getColors(),
        ui->m_average_opacity_map->getOpacities() );

    appendStatisticPatch(
        data,
        QStringLiteral( "variance" ),
        ui->m_variance_server_min_max_radio_button->isChecked(),
        ui->m_variance_user_min_double_spin_box->value(),
        ui->m_variance_user_max_double_spin_box->value(),
        ui->m_variance_server_min_double_spin_box->value(),
        ui->m_variance_server_max_double_spin_box->value(),
        ui->m_variance_color_map->getColors(),
        ui->m_variance_opacity_map->getOpacities() );

    appendStatisticPatch(
        data,
        QStringLiteral( "cv" ),
        ui->m_coefficient_variation_server_min_max_radio_button->isChecked(),
        ui->m_coefficient_variation_user_min_double_spin_box->value(),
        ui->m_coefficient_variation_user_max_double_spin_box->value(),
        ui->m_coefficient_variation_server_min_double_spin_box->value(),
        ui->m_coefficient_variation_server_max_double_spin_box->value(),
        ui->m_coefficient_variation_color_map->getColors(),
        ui->m_coefficient_variation_opacity_map->getOpacities() );

    QJsonObject root;
    root[QString::fromUtf8( Protocol::Key::Event )] =
        QString::fromUtf8( Protocol::Events::EnsembleStatisticsParameter );
    root[QString::fromUtf8( Protocol::Key::RepeatLevel )] = static_cast<int>( m_repeat_level );
    root["TFNumber"] = 1;
    root[QString::fromUtf8( Protocol::Key::ColorSynthesizer )] = QStringLiteral( "C1" );
    root[QString::fromUtf8( Protocol::Key::OpacitySynthesizer )] = QStringLiteral( "O1" );
    root[QString::fromUtf8( Protocol::Key::Data )] = data;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}
