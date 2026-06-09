#include "EnsembleTransferFunctionEditor.h"
#include "ui_EnsembleTransferFunctionEditor.h"

#include "ColorMap.h"
#include "OpacityMap.h"
#include "../ExtendedQT/Histogram.h"

#include <QButtonGroup>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>

#include <cmath>

#include "../../Shared/JsonKeys.h"

namespace
{
constexpr int UserRangeMode = 1;
constexpr int ServerRangeMode = 2;

struct EnsembleTFSetting
{
    QString displayName;
    QString statisticName;
    QString variableExpression;
    QVector<QColor> colorMap;
    QVector<float> opacityMap;
    bool useUserMinMax = true;
    double userMin = 0.0;
    double userMax = 1.0;
    double serverMin = 0.0;
    double serverMax = 1.0;
    std::vector<int> histogram;
};

QJsonArray ColorMapToPatchJson( const QVector<QColor>& colors )
{
    QJsonArray array;
    for( const auto& color : colors )
    {
        QJsonArray rgb;
        rgb.append( color.red() );
        rgb.append( color.green() );
        rgb.append( color.blue() );
        array.append( rgb );
    }
    return array;
}

QJsonObject ColorMapToDefaultJson( const QVector<QColor>& colors )
{
    QJsonArray values;
    for( const auto& color : colors )
    {
        values.append( color.red() );
        values.append( color.green() );
        values.append( color.blue() );
    }

    QJsonObject map;
    map["role"] = QStringLiteral( "color map" );
    map["encoding"] = QStringLiteral( "flat RGB uint8 triplets" );
    map["length"] = values.size();
    map["values"] = values;
    return map;
}

QJsonArray OpacityMapToPatchJson( const QVector<float>& opacities )
{
    QJsonArray array;
    for( const float opacity : opacities ) array.append( opacity );
    return array;
}

QJsonObject OpacityMapToDefaultJson( const QVector<float>& opacities )
{
    QJsonArray values;
    for( const float opacity : opacities ) values.append( opacity );

    QJsonObject map;
    map["role"] = QStringLiteral( "opacity map" );
    map["encoding"] = QStringLiteral( "float values from 0 to 1" );
    map["length"] = values.size();
    map["values"] = values;
    return map;
}

QJsonObject RangeToDefaultJson( double min, double max )
{
    QJsonObject user;
    user["min"] = min;
    user["max"] = max;

    QJsonObject server;
    server["min"] = min;
    server["max"] = max;

    QJsonObject range;
    range["active_range"] = QStringLiteral( "user" );
    range["user"] = user;
    range["server"] = server;
    return range;
}

QJsonObject TransferFunctionToDefaultJson( const EnsembleTFSetting& setting )
{
    QJsonObject color;
    color["variable"] = setting.variableExpression;
    color["range"] = RangeToDefaultJson( setting.userMin, setting.userMax );
    color["map"] = ColorMapToDefaultJson( setting.colorMap );

    QJsonObject opacity;
    opacity["variable"] = setting.variableExpression;
    opacity["range"] = RangeToDefaultJson( setting.userMin, setting.userMax );
    opacity["map"] = OpacityMapToDefaultJson( setting.opacityMap );

    QJsonObject tf;
    tf["id"] = 1;
    tf["label"] = QStringLiteral( "TF1" );
    tf["color"] = color;
    tf["opacity"] = opacity;
    return tf;
}

bool ReadDoubleMember( const QJsonObject& object, const QString& key, double* value )
{
    if( !object.contains( key ) || !object.value( key ).isDouble() ) return false;
    *value = object.value( key ).toDouble();
    return true;
}

bool ReadRange( const QJsonObject& parent,
                bool* useUserMinMax,
                double* userMin,
                double* userMax,
                double* serverMin,
                double* serverMax )
{
    if( !parent.value( "range" ).isObject() ) return false;

    const QJsonObject range = parent.value( "range" ).toObject();
    const QString activeRange = range.value( "active_range" ).toString();
    if( !activeRange.isEmpty() ) *useUserMinMax = activeRange != QStringLiteral( "server" );

    bool ok = true;
    if( range.value( "user" ).isObject() )
    {
        const QJsonObject user = range.value( "user" ).toObject();
        ok = ReadDoubleMember( user, "min", userMin ) && ok;
        ok = ReadDoubleMember( user, "max", userMax ) && ok;
    }
    else ok = false;

    if( range.value( "server" ).isObject() )
    {
        const QJsonObject server = range.value( "server" ).toObject();
        ok = ReadDoubleMember( server, "min", serverMin ) && ok;
        ok = ReadDoubleMember( server, "max", serverMax ) && ok;
    }
    else ok = false;

    return ok;
}

QVector<QColor> ReadColorMap( const QJsonValue& value, bool* ok )
{
    QVector<QColor> colors;
    *ok = false;

    QJsonArray values;
    if( value.isObject() ) values = value.toObject().value( "values" ).toArray();
    else if( value.isArray() ) values = value.toArray();
    if( values.isEmpty() ) return colors;

    if( values.first().isArray() )
    {
        for( const auto& entry : values )
        {
            const QJsonArray rgb = entry.toArray();
            if( rgb.size() < 3 ) return QVector<QColor>();
            colors.append( QColor( rgb.at( 0 ).toInt(), rgb.at( 1 ).toInt(), rgb.at( 2 ).toInt() ) );
        }
    }
    else
    {
        for( int i = 0; i + 2 < values.size(); i += 3 )
        {
            colors.append( QColor( values.at( i ).toInt(), values.at( i + 1 ).toInt(), values.at( i + 2 ).toInt() ) );
        }
    }

    *ok = !colors.isEmpty();
    return colors;
}

QVector<float> ReadOpacityMap( const QJsonValue& value, bool* ok )
{
    QVector<float> opacities;
    *ok = false;

    QJsonArray values;
    if( value.isObject() ) values = value.toObject().value( "values" ).toArray();
    else if( value.isArray() ) values = value.toArray();
    if( values.isEmpty() ) return opacities;

    for( const auto& entry : values ) opacities.append( static_cast<float>( entry.toDouble() ) );
    *ok = !opacities.isEmpty();
    return opacities;
}

QString NormalizeStatisticName( const QString& statistic )
{
    const QString normalized = statistic.trimmed().toLower();
    if( normalized == QStringLiteral( "average" ) || normalized == QStringLiteral( "avg" ) ||
        normalized == QStringLiteral( "mean" ) )
    {
        return QStringLiteral( "average" );
    }
    if( normalized == QStringLiteral( "variance" ) ) return QStringLiteral( "variance" );
    if( normalized == QStringLiteral( "cv" ) || normalized == QStringLiteral( "cov" ) ||
        normalized == QStringLiteral( "coefficient_variation" ) ||
        normalized == QStringLiteral( "coefficient of variation" ) ||
        normalized == QStringLiteral( "coefficient-of-variation" ) )
    {
        return QStringLiteral( "cv" );
    }
    return QString();
}

std::vector<int> ReadHistogram( const QJsonValue& value, bool* ok )
{
    std::vector<int> histogram;
    *ok = false;
    if( !value.isArray() ) return histogram;

    const QJsonArray array = value.toArray();
    histogram.reserve( static_cast<size_t>( array.size() ) );
    for( const auto& entry : array ) histogram.push_back( entry.toInt() );

    *ok = true;
    return histogram;
}
}

EnsembleTransferFunctionEditor::EnsembleTransferFunctionEditor( WebSocketPair* websockets, QWidget* parent )
    : QDialog( parent )
    , ui( new Ui::EnsembleTransferFunctionEditor )
    , m_web_sockets( websockets )
    , m_color_map_editor( this )
    , m_opacity_map_editor( this )
{
    ui->setupUi( this );
    initializeButtonGroups();
    initializeTransferFunctionWidgets();

    connect( ui->m_apply_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onApply );
    connect( ui->m_export_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onExport );
    connect( ui->m_import_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onImport );
    connect( ui->m_average_radio_button, &QRadioButton::toggled, this, [this]( bool checked )
    {
        if( checked ) emit statisticChanged( selectedStatistic() );
    } );
    connect( ui->m_variance_radio_button, &QRadioButton::toggled, this, [this]( bool checked )
    {
        if( checked ) emit statisticChanged( selectedStatistic() );
    } );
    connect( ui->m_coefficient_variation_radio_button, &QRadioButton::toggled, this, [this]( bool checked )
    {
        if( checked ) emit statisticChanged( selectedStatistic() );
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

void EnsembleTransferFunctionEditor::initializeTransferFunctionWidgets()
{
    ui->m_statistics_synthesizer_line_edit->setText( QStringLiteral( "q1" ) );
    ui->m_export_push_button->setAutoDefault( false );
    ui->m_export_push_button->setDefault( false );
    ui->m_import_push_button->setAutoDefault( false );
    ui->m_import_push_button->setDefault( false );
    ui->m_apply_push_button->setAutoDefault( false );
    ui->m_apply_push_button->setDefault( false );

    ui->m_average_color_map->installEventFilter( this );
    ui->m_variance_color_map->installEventFilter( this );
    ui->m_coefficient_variation_color_map->installEventFilter( this );
    ui->m_average_opacity_map->installEventFilter( this );
    ui->m_variance_opacity_map->installEventFilter( this );
    ui->m_coefficient_variation_opacity_map->installEventFilter( this );
}

void EnsembleTransferFunctionEditor::setRepeatLevel( size_t repeatLevel )
{
    m_repeat_level = repeatLevel;
}

void EnsembleTransferFunctionEditor::onReceiveEnsembleStatisticsParameter( const QJsonObject& payload )
{
    const QString dataKey = QString::fromUtf8( Protocol::Key::Data );
    if( !payload.value( dataKey ).isArray() )
    {
        qDebug() << "[Client][EnsembleTFE] EnsembleStatisticsParameter has no Data array";
        return;
    }

    auto updateBlock = [&]( const QString& displayName,
                            QDoubleSpinBox* serverMinSpinBox,
                            QDoubleSpinBox* serverMaxSpinBox,
                            Histogram* histogramWidget,
                            const QJsonObject& patch )
    {
        const QString minKey = QString::fromUtf8( Protocol::Key::OpacityServerRangeMin );
        const QString maxKey = QString::fromUtf8( Protocol::Key::OpacityServerRangeMax );
        const QString histogramKey = QString::fromUtf8( Protocol::Key::OpacityHistogram );

        if( patch.value( minKey ).isDouble() && patch.value( maxKey ).isDouble() )
        {
            const double opacityMin = patch.value( minKey ).toDouble();
            const double opacityMax = patch.value( maxKey ).toDouble();
            if( !std::isfinite( opacityMin ) || !std::isfinite( opacityMax ) )
            {
                qDebug() << "[Client][EnsembleTFE] ignore invalid server range for" << displayName
                         << "min =" << opacityMin << "max =" << opacityMax
                         << "reason = non-finite";
            }
            else if( opacityMin > opacityMax )
            {
                qDebug() << "[Client][EnsembleTFE] ignore invalid server range for" << displayName
                         << "min =" << opacityMin << "max =" << opacityMax
                         << "reason = min > max";
            }
            else
            {
                serverMinSpinBox->setValue( opacityMin );
                serverMaxSpinBox->setValue( opacityMax );
                qDebug() << "[Client][EnsembleTFE] update server range for" << displayName
                         << "min =" << opacityMin << "max =" << opacityMax;
            }
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] keep server range for" << displayName
                     << "reason = missing opacity range keys";
        }

        if( patch.contains( histogramKey ) )
        {
            bool histogramOk = false;
            const std::vector<int> histogram = ReadHistogram( patch.value( histogramKey ), &histogramOk );
            if( histogramOk )
            {
                // Empty histograms are treated as an explicit request to clear the display.
                histogramWidget->setDatas( histogram );
                histogramWidget->update();
                qDebug() << "[Client][EnsembleTFE] update histogram for" << displayName
                         << "bins =" << histogram.size();
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep histogram for" << displayName
                         << "reason = OpacityHistogram is not an array";
            }
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] keep histogram for" << displayName
                     << "reason = missing OpacityHistogram";
        }
    };

    const QJsonArray data = payload.value( dataKey ).toArray();
    for( const auto& value : data )
    {
        if( !value.isObject() )
        {
            qDebug() << "[Client][EnsembleTFE] skip non-object statistics patch";
            continue;
        }

        const QJsonObject patch = value.toObject();
        const int index = patch.value( QString::fromUtf8( Protocol::Key::Index ) ).toInt( 0 );
        if( index != 0 )
        {
            qDebug() << "[Client][EnsembleTFE] skip statistics patch with unsupported Index =" << index;
            continue;
        }

        const QString statistic = NormalizeStatisticName(
            patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString() );
        qDebug() << "[Client][EnsembleTFE] receive statistic patch"
                 << patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString()
                 << "normalized =" << statistic;

        if( statistic == QStringLiteral( "average" ) )
        {
            updateBlock( QStringLiteral( "Average" ),
                         ui->m_average_server_min_double_spin_box,
                         ui->m_average_server_max_double_spin_box,
                         ui->m_average_histogram,
                         patch );
        }
        else if( statistic == QStringLiteral( "variance" ) )
        {
            updateBlock( QStringLiteral( "Variance" ),
                         ui->m_variance_server_min_double_spin_box,
                         ui->m_variance_server_max_double_spin_box,
                         ui->m_variance_histogram,
                         patch );
        }
        else if( statistic == QStringLiteral( "cv" ) )
        {
            updateBlock( QStringLiteral( "Coefficient of Variation" ),
                         ui->m_coefficient_variation_server_min_double_spin_box,
                         ui->m_coefficient_variation_server_max_double_spin_box,
                         ui->m_coefficient_variation_histogram,
                         patch );
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] skip unknown statistic patch";
        }
    }
}

QString EnsembleTransferFunctionEditor::selectedStatistic() const
{
    if( ui->m_variance_radio_button->isChecked() ) return QStringLiteral( "variance" );
    if( ui->m_coefficient_variation_radio_button->isChecked() ) return QStringLiteral( "cv" );
    return QStringLiteral( "average" );
}

bool EnsembleTransferFunctionEditor::eventFilter( QObject* watched, QEvent* event )
{
    if( event->type() != QEvent::MouseButtonRelease )
    {
        return QDialog::eventFilter( watched, event );
    }

    auto* mouseEvent = static_cast<QMouseEvent*>( event );
    if( mouseEvent->button() != Qt::LeftButton ) return QDialog::eventFilter( watched, event );

    if( watched == ui->m_average_color_map || watched == ui->m_variance_color_map ||
        watched == ui->m_coefficient_variation_color_map )
    {
        auto* colorMap = qobject_cast<ColorMap*>( watched );
        if( !colorMap ) return QDialog::eventFilter( watched, event );

        m_color_map_editor.adjustSize();
        m_color_map_editor.setDefaultColorMap( colorMap->getColors() );
        if( m_color_map_editor.exec() == QDialog::Accepted )
        {
            colorMap->setColors( m_color_map_editor.getColorMap() );
        }
        return true;
    }

    if( watched == ui->m_average_opacity_map || watched == ui->m_variance_opacity_map ||
        watched == ui->m_coefficient_variation_opacity_map )
    {
        auto* opacityMap = qobject_cast<OpacityMap*>( watched );
        if( !opacityMap ) return QDialog::eventFilter( watched, event );

        m_opacity_map_editor.adjustSize();
        m_opacity_map_editor.setDefaultOpacityMap( opacityMap->getOpacities() );
        if( m_opacity_map_editor.exec() == QDialog::Accepted )
        {
            opacityMap->setOpacities( m_opacity_map_editor.getOpacityMap() );
        }
        return true;
    }

    return QDialog::eventFilter( watched, event );
}

bool EnsembleTransferFunctionEditor::validateForApply() const
{
    if( ui->m_statistics_synthesizer_line_edit->text().trimmed().isEmpty() )
    {
        QMessageBox::warning( const_cast<EnsembleTransferFunctionEditor*>( this ),
                              tr( "Ensemble Transfer Function Editor" ),
                              tr( "Synthesizer expression is empty." ) );
        return false;
    }

    auto validateStatistic = [this]( const QString& name,
                                     bool useUser,
                                     double userMin,
                                     double userMax,
                                     double serverMin,
                                     double serverMax,
                                     int colorCount,
                                     int opacityCount )
    {
        if( colorCount == 0 || opacityCount == 0 )
        {
            QMessageBox::warning( const_cast<EnsembleTransferFunctionEditor*>( this ),
                                  tr( "Ensemble Transfer Function Editor" ),
                                  tr( "%1 has an unset transfer function map." ).arg( name ) );
            return false;
        }

        const double minValue = useUser ? userMin : serverMin;
        const double maxValue = useUser ? userMax : serverMax;
        if( minValue > maxValue )
        {
            QMessageBox::warning( const_cast<EnsembleTransferFunctionEditor*>( this ),
                                  tr( "Ensemble Transfer Function Editor" ),
                                  tr( "%1 MinMax is invalid." ).arg( name ) );
            return false;
        }
        return true;
    };

    return validateStatistic( QStringLiteral( "Average" ),
                              ui->m_average_user_min_max_radio_button->isChecked(),
                              ui->m_average_user_min_double_spin_box->value(),
                              ui->m_average_user_max_double_spin_box->value(),
                              ui->m_average_server_min_double_spin_box->value(),
                              ui->m_average_server_max_double_spin_box->value(),
                              ui->m_average_color_map->getColors().size(),
                              ui->m_average_opacity_map->getOpacities().size() ) &&
           validateStatistic( QStringLiteral( "Variance" ),
                              ui->m_variance_user_min_max_radio_button->isChecked(),
                              ui->m_variance_user_min_double_spin_box->value(),
                              ui->m_variance_user_max_double_spin_box->value(),
                              ui->m_variance_server_min_double_spin_box->value(),
                              ui->m_variance_server_max_double_spin_box->value(),
                              ui->m_variance_color_map->getColors().size(),
                              ui->m_variance_opacity_map->getOpacities().size() ) &&
           validateStatistic( QStringLiteral( "Coefficient of Variation" ),
                              ui->m_coefficient_variation_user_min_max_radio_button->isChecked(),
                              ui->m_coefficient_variation_user_min_double_spin_box->value(),
                              ui->m_coefficient_variation_user_max_double_spin_box->value(),
                              ui->m_coefficient_variation_server_min_double_spin_box->value(),
                              ui->m_coefficient_variation_server_max_double_spin_box->value(),
                              ui->m_coefficient_variation_color_map->getColors().size(),
                              ui->m_coefficient_variation_opacity_map->getOpacities().size() );
}

bool EnsembleTransferFunctionEditor::validateForExport() const
{
    if( ui->m_statistics_synthesizer_line_edit->text().trimmed().isEmpty() )
    {
        QMessageBox::warning( const_cast<EnsembleTransferFunctionEditor*>( this ),
                              tr( "Ensemble Transfer Function Editor" ),
                              tr( "Synthesizer expression is empty." ) );
        return false;
    }

    auto validateUserRange = [this]( const QString& name, double minValue, double maxValue, int colorCount, int opacityCount )
    {
        if( colorCount == 0 || opacityCount == 0 )
        {
            QMessageBox::warning( const_cast<EnsembleTransferFunctionEditor*>( this ),
                                  tr( "Ensemble Transfer Function Editor" ),
                                  tr( "%1 has an unset transfer function map." ).arg( name ) );
            return false;
        }
        if( minValue > maxValue )
        {
            QMessageBox::warning( const_cast<EnsembleTransferFunctionEditor*>( this ),
                                  tr( "Ensemble Transfer Function Editor" ),
                                  tr( "%1 User MinMax is invalid." ).arg( name ) );
            return false;
        }
        return true;
    };

    return validateUserRange( QStringLiteral( "Average" ),
                              ui->m_average_user_min_double_spin_box->value(),
                              ui->m_average_user_max_double_spin_box->value(),
                              ui->m_average_color_map->getColors().size(),
                              ui->m_average_opacity_map->getOpacities().size() ) &&
           validateUserRange( QStringLiteral( "Variance" ),
                              ui->m_variance_user_min_double_spin_box->value(),
                              ui->m_variance_user_max_double_spin_box->value(),
                              ui->m_variance_color_map->getColors().size(),
                              ui->m_variance_opacity_map->getOpacities().size() ) &&
           validateUserRange( QStringLiteral( "Coefficient of Variation" ),
                              ui->m_coefficient_variation_user_min_double_spin_box->value(),
                              ui->m_coefficient_variation_user_max_double_spin_box->value(),
                              ui->m_coefficient_variation_color_map->getColors().size(),
                              ui->m_coefficient_variation_opacity_map->getOpacities().size() );
}

void EnsembleTransferFunctionEditor::onApply()
{
    if( !validateForApply() ) return;
    if( !m_web_sockets || !m_web_sockets->isConnected() )
    {
        QMessageBox::warning( this, tr( "Ensemble Transfer Function Editor" ), tr( "WebSocket is not connected." ) );
        return;
    }

    const QString variableExpression = ui->m_statistics_synthesizer_line_edit->text().trimmed();
    auto appendStatisticPatch = [&]( QJsonArray& data,
                                     const QString& statistic,
                                     bool useUserRange,
                                     double userMin,
                                     double userMax,
                                     double serverMin,
                                     double serverMax,
                                     const QVector<QColor>& colors,
                                     const QVector<float>& opacities )
    {
        const int rangeMode = useUserRange ? UserRangeMode : ServerRangeMode;

        QJsonObject patch;
        patch[QString::fromUtf8( Protocol::Key::Statistic )] = statistic;
        patch[QString::fromUtf8( Protocol::Key::Index )] = 0;
        patch[QString::fromUtf8( Protocol::Key::ColorVariable )] = variableExpression;
        patch[QString::fromUtf8( Protocol::Key::ColorRangeMode )] = rangeMode;
        patch[QString::fromUtf8( Protocol::Key::ColorUserRangeMin )] = userMin;
        patch[QString::fromUtf8( Protocol::Key::ColorUserRangeMax )] = userMax;
        patch[QString::fromUtf8( Protocol::Key::ColorServerRangeMin )] = serverMin;
        patch[QString::fromUtf8( Protocol::Key::ColorServerRangeMax )] = serverMax;
        patch[QString::fromUtf8( Protocol::Key::ColorMap )] = ColorMapToPatchJson( colors );
        patch[QString::fromUtf8( Protocol::Key::OpacityVariable )] = variableExpression;
        patch[QString::fromUtf8( Protocol::Key::OpacityRangeMode )] = rangeMode;
        patch[QString::fromUtf8( Protocol::Key::OpacityUserRangeMin )] = userMin;
        patch[QString::fromUtf8( Protocol::Key::OpacityUserRangeMax )] = userMax;
        patch[QString::fromUtf8( Protocol::Key::OpacityServerRangeMin )] = serverMin;
        patch[QString::fromUtf8( Protocol::Key::OpacityServerRangeMax )] = serverMax;
        patch[QString::fromUtf8( Protocol::Key::OpacityMap )] = OpacityMapToPatchJson( opacities );
        data.append( patch );
    };

    QJsonArray data;
    appendStatisticPatch( data,
                          QStringLiteral( "average" ),
                          ui->m_average_user_min_max_radio_button->isChecked(),
                          ui->m_average_user_min_double_spin_box->value(),
                          ui->m_average_user_max_double_spin_box->value(),
                          ui->m_average_server_min_double_spin_box->value(),
                          ui->m_average_server_max_double_spin_box->value(),
                          ui->m_average_color_map->getColors(),
                          ui->m_average_opacity_map->getOpacities() );
    appendStatisticPatch( data,
                          QStringLiteral( "variance" ),
                          ui->m_variance_user_min_max_radio_button->isChecked(),
                          ui->m_variance_user_min_double_spin_box->value(),
                          ui->m_variance_user_max_double_spin_box->value(),
                          ui->m_variance_server_min_double_spin_box->value(),
                          ui->m_variance_server_max_double_spin_box->value(),
                          ui->m_variance_color_map->getColors(),
                          ui->m_variance_opacity_map->getOpacities() );
    appendStatisticPatch( data,
                          QStringLiteral( "cv" ),
                          ui->m_coefficient_variation_user_min_max_radio_button->isChecked(),
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

void EnsembleTransferFunctionEditor::onExport()
{
    if( !validateForExport() ) return;

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr( "Export Ensemble Transfer Function File" ),
        QStringLiteral( "default.json" ),
        tr( "JSON Files (*.json)" ) );
    if( fileName.isEmpty() ) return;

    const QString variableExpression = ui->m_statistics_synthesizer_line_edit->text().trimmed();
    const EnsembleTFSetting average = {
        QStringLiteral( "Average" ),
        QStringLiteral( "average" ),
        variableExpression,
        ui->m_average_color_map->getColors(),
        ui->m_average_opacity_map->getOpacities(),
        true,
        ui->m_average_user_min_double_spin_box->value(),
        ui->m_average_user_max_double_spin_box->value(),
        ui->m_average_user_min_double_spin_box->value(),
        ui->m_average_user_max_double_spin_box->value(),
        ui->m_average_histogram->getDatas()
    };
    const EnsembleTFSetting variance = {
        QStringLiteral( "Variance" ),
        QStringLiteral( "variance" ),
        variableExpression,
        ui->m_variance_color_map->getColors(),
        ui->m_variance_opacity_map->getOpacities(),
        true,
        ui->m_variance_user_min_double_spin_box->value(),
        ui->m_variance_user_max_double_spin_box->value(),
        ui->m_variance_user_min_double_spin_box->value(),
        ui->m_variance_user_max_double_spin_box->value(),
        ui->m_variance_histogram->getDatas()
    };
    const EnsembleTFSetting coefficientVariation = {
        QStringLiteral( "Coefficient of Variation" ),
        QStringLiteral( "cv" ),
        variableExpression,
        ui->m_coefficient_variation_color_map->getColors(),
        ui->m_coefficient_variation_opacity_map->getOpacities(),
        true,
        ui->m_coefficient_variation_user_min_double_spin_box->value(),
        ui->m_coefficient_variation_user_max_double_spin_box->value(),
        ui->m_coefficient_variation_user_min_double_spin_box->value(),
        ui->m_coefficient_variation_user_max_double_spin_box->value(),
        ui->m_coefficient_variation_histogram->getDatas()
    };

    QJsonObject transferFunctionSettings;
    transferFunctionSettings["transfer_function_count"] = 1;
    transferFunctionSettings["transfer_function_resolution"] = 256;
    transferFunctionSettings["color_synthesis"] = QStringLiteral( "C1" );
    transferFunctionSettings["opacity_synthesis"] = QStringLiteral( "O1" );

    QJsonObject samplingSettings;
    samplingSettings["m_repeat_level"] = static_cast<double>( m_repeat_level );

    QJsonObject settings;
    settings["sampling"] = samplingSettings;
    settings["transfer_function"] = transferFunctionSettings;

    QJsonArray normalTransferFunctions;
    normalTransferFunctions.append( TransferFunctionToDefaultJson( average ) );

    QJsonArray meanTransferFunctions;
    meanTransferFunctions.append( TransferFunctionToDefaultJson( average ) );

    QJsonArray varianceTransferFunctions;
    varianceTransferFunctions.append( TransferFunctionToDefaultJson( variance ) );

    QJsonArray coefficientVariationTransferFunctions;
    coefficientVariationTransferFunctions.append( TransferFunctionToDefaultJson( coefficientVariation ) );

    QJsonObject root;
    root["format"] = QStringLiteral( "PBVR readable transfer function settings" );
    root["format_version"] = 1;
    root["settings"] = settings;
    root["transfer_functions"] = normalTransferFunctions;
    root["mean_transfer_functions"] = meanTransferFunctions;
    root["variance_transfer_functions"] = varianceTransferFunctions;
    root["coefficient_of_variation_transfer_functions"] = coefficientVariationTransferFunctions;

    QFile file( fileName );
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QMessageBox::warning( this, tr( "Ensemble Transfer Function Editor" ), tr( "Could not open export file." ) );
        return;
    }
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Indented ) );
}

void EnsembleTransferFunctionEditor::onImport()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr( "Import Ensemble Transfer Function File" ),
        QStringLiteral( "." ),
        tr( "JSON Files (*.json)" ) );
    if( fileName.isEmpty() ) return;

    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QMessageBox::warning( this, tr( "Ensemble Transfer Function Editor" ), tr( "Could not open import file." ) );
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson( file.readAll(), &error );
    if( error.error != QJsonParseError::NoError || !document.isObject() )
    {
        QMessageBox::warning( this, tr( "Ensemble Transfer Function Editor" ), tr( "Invalid JSON file." ) );
        return;
    }

    const QJsonObject root = document.object();
    if( root.value( "settings" ).isObject() )
    {
        const QJsonObject settings = root.value( "settings" ).toObject();
        if( settings.value( "sampling" ).isObject() )
        {
            const QJsonObject sampling = settings.value( "sampling" ).toObject();
            if( sampling.value( "m_repeat_level" ).isDouble() )
            {
                m_repeat_level = static_cast<size_t>( sampling.value( "m_repeat_level" ).toDouble() );
            }
        }
    }

    bool hadMissingKeys = false;
    QString importedExpression;

    auto importStatistic = [&]( const QString& sectionName,
                                ColorMap* colorMapWidget,
                                OpacityMap* opacityMapWidget,
                                QRadioButton* userRadioButton,
                                QRadioButton* serverRadioButton,
                                QDoubleSpinBox* userMinSpinBox,
                                QDoubleSpinBox* userMaxSpinBox,
                                QDoubleSpinBox* serverMinSpinBox,
                                QDoubleSpinBox* serverMaxSpinBox )
    {
        QJsonArray section = root.value( sectionName ).toArray();
        if( section.isEmpty() && sectionName == QStringLiteral( "mean_transfer_functions" ) )
        {
            section = root.value( "transfer_functions" ).toArray();
        }
        if( section.isEmpty() || !section.first().isObject() )
        {
            hadMissingKeys = true;
            return;
        }

        const QJsonObject tf = section.first().toObject();
        const QJsonObject color = tf.value( "color" ).toObject();
        const QJsonObject opacity = tf.value( "opacity" ).toObject();
        if( color.isEmpty() || opacity.isEmpty() )
        {
            hadMissingKeys = true;
            return;
        }

        const QString colorVariable = color.value( "variable" ).toString();
        if( !colorVariable.isEmpty() && importedExpression.isEmpty() ) importedExpression = colorVariable;

        bool useUser = true;
        double userMin = userMinSpinBox->value();
        double userMax = userMaxSpinBox->value();
        double serverMin = serverMinSpinBox->value();
        double serverMax = serverMaxSpinBox->value();
        if( !ReadRange( color, &useUser, &userMin, &userMax, &serverMin, &serverMax ) ) hadMissingKeys = true;

        userMinSpinBox->setValue( userMin );
        userMaxSpinBox->setValue( userMax );
        serverMinSpinBox->setValue( serverMin );
        serverMaxSpinBox->setValue( serverMax );
        userRadioButton->setChecked( useUser );
        serverRadioButton->setChecked( !useUser );

        bool colorMapOk = false;
        const QVector<QColor> colors = ReadColorMap( color.value( "map" ), &colorMapOk );
        if( colorMapOk ) colorMapWidget->setColors( colors );
        else hadMissingKeys = true;

        bool opacityMapOk = false;
        const QVector<float> opacities = ReadOpacityMap( opacity.value( "map" ), &opacityMapOk );
        if( opacityMapOk ) opacityMapWidget->setOpacities( opacities );
        else hadMissingKeys = true;
    };

    // Missing optional/default.json fields keep the current GUI values so a partial file can still be useful.
    importStatistic( QStringLiteral( "mean_transfer_functions" ),
                     ui->m_average_color_map,
                     ui->m_average_opacity_map,
                     ui->m_average_user_min_max_radio_button,
                     ui->m_average_server_min_max_radio_button,
                     ui->m_average_user_min_double_spin_box,
                     ui->m_average_user_max_double_spin_box,
                     ui->m_average_server_min_double_spin_box,
                     ui->m_average_server_max_double_spin_box );
    importStatistic( QStringLiteral( "variance_transfer_functions" ),
                     ui->m_variance_color_map,
                     ui->m_variance_opacity_map,
                     ui->m_variance_user_min_max_radio_button,
                     ui->m_variance_server_min_max_radio_button,
                     ui->m_variance_user_min_double_spin_box,
                     ui->m_variance_user_max_double_spin_box,
                     ui->m_variance_server_min_double_spin_box,
                     ui->m_variance_server_max_double_spin_box );
    importStatistic( QStringLiteral( "coefficient_of_variation_transfer_functions" ),
                     ui->m_coefficient_variation_color_map,
                     ui->m_coefficient_variation_opacity_map,
                     ui->m_coefficient_variation_user_min_max_radio_button,
                     ui->m_coefficient_variation_server_min_max_radio_button,
                     ui->m_coefficient_variation_user_min_double_spin_box,
                     ui->m_coefficient_variation_user_max_double_spin_box,
                     ui->m_coefficient_variation_server_min_double_spin_box,
                     ui->m_coefficient_variation_server_max_double_spin_box );

    if( !importedExpression.isEmpty() ) ui->m_statistics_synthesizer_line_edit->setText( importedExpression );

    ui->m_average_color_map->update();
    ui->m_average_opacity_map->update();
    ui->m_variance_color_map->update();
    ui->m_variance_opacity_map->update();
    ui->m_coefficient_variation_color_map->update();
    ui->m_coefficient_variation_opacity_map->update();

    if( hadMissingKeys )
    {
        QMessageBox::warning( this,
                              tr( "Ensemble Transfer Function Editor" ),
                              tr( "Some default.json keys were missing. Existing GUI values were kept for those fields." ) );
    }
}
