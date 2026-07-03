#include "EnsembleTransferFunctionEditor.h"
#include "ui_EnsembleTransferFunctionEditor.h"

#include "ColorMap.h"
#include "OpacityMap.h"
#include "../ExtendedQT/Histogram.h"

#include <QButtonGroup>
#include <QComboBox>
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
    initialize();

    connect( ui->m_statistics_combo_box,
             QOverload<int>::of( &QComboBox::currentIndexChanged ),
             this,
             &EnsembleTransferFunctionEditor::onStatisticSelectionChanged );
    connect( ui->m_apply_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onApply );
    connect( ui->m_export_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onExport );
    connect( ui->m_import_push_button, &QPushButton::clicked, this, &EnsembleTransferFunctionEditor::onImport );
}

EnsembleTransferFunctionEditor::~EnsembleTransferFunctionEditor()
{
    delete ui;
}

void EnsembleTransferFunctionEditor::initialize()
{
    m_min_max_group = new QButtonGroup( this );
    m_min_max_group->setObjectName( QStringLiteral( "MinMaxGroup" ) );
    m_min_max_group->addButton( ui->m_server_min_max_radio_button );
    m_min_max_group->addButton( ui->m_user_min_max_radio_button );
    m_min_max_group->setExclusive( true );

    ui->m_statistics_synthesizer_line_edit->setText( QStringLiteral( "q1" ) );
    ui->m_export_push_button->setAutoDefault( false );
    ui->m_export_push_button->setDefault( false );
    ui->m_import_push_button->setAutoDefault( false );
    ui->m_import_push_button->setDefault( false );
    ui->m_apply_push_button->setAutoDefault( false );
    ui->m_apply_push_button->setDefault( false );

    ui->m_color_map->installEventFilter( this );
    ui->m_opacity_map->installEventFilter( this );

    m_statistics[AverageStatistic].displayName = QStringLiteral( "Average" );
    m_statistics[AverageStatistic].statisticName = QStringLiteral( "average" );
    m_statistics[VarianceStatistic].displayName = QStringLiteral( "Variance" );
    m_statistics[VarianceStatistic].statisticName = QStringLiteral( "variance" );
    m_statistics[CoefficientVariationStatistic].displayName = QStringLiteral( "Coefficient of Variation" );
    m_statistics[CoefficientVariationStatistic].statisticName = QStringLiteral( "cv" );

    m_default_color_map = ui->m_color_map->getColors();
    m_default_opacity_map = ui->m_opacity_map->getOpacities();
    const std::vector<int> histogram = ui->m_histogram->getDatas();
    for( auto& statistic : m_statistics )
    {
        statistic.colorMap = m_default_color_map;
        statistic.opacityMap = m_default_opacity_map;
        statistic.useUserMinMax = ui->m_user_min_max_radio_button->isChecked();
        statistic.userMin = ui->m_user_min_double_spin_box->value();
        statistic.userMax = ui->m_user_max_double_spin_box->value();
        statistic.serverMin = ui->m_server_min_double_spin_box->value();
        statistic.serverMax = ui->m_server_max_double_spin_box->value();
        statistic.histogram = histogram;
    }

    m_current_statistic = AverageStatistic;
    loadStatisticState( m_current_statistic );
}

void EnsembleTransferFunctionEditor::reset()
{
    ui->m_statistics_synthesizer_line_edit->setText( QStringLiteral( "q1" ) );
    for( auto& statistic : m_statistics )
    {
        statistic.colorMap = m_default_color_map;
        statistic.opacityMap = m_default_opacity_map;
        statistic.useUserMinMax = true;
        statistic.userMin = 0.0;
        statistic.userMax = 1.0;
        statistic.serverMin = 0.0;
        statistic.serverMax = 1.0;
        statistic.histogram.clear();
    }

    m_current_statistic = AverageStatistic;
    const bool wasBlocked = ui->m_statistics_combo_box->blockSignals( true );
    ui->m_statistics_combo_box->setCurrentIndex( AverageStatistic );
    ui->m_statistics_combo_box->blockSignals( wasBlocked );
    loadStatisticState( m_current_statistic );
}

void EnsembleTransferFunctionEditor::saveCurrentStatisticState()
{
    StatisticUiState& statistic = m_statistics[m_current_statistic];
    statistic.colorMap = ui->m_color_map->getColors();
    statistic.opacityMap = ui->m_opacity_map->getOpacities();
    statistic.useUserMinMax = ui->m_user_min_max_radio_button->isChecked();
    statistic.userMin = ui->m_user_min_double_spin_box->value();
    statistic.userMax = ui->m_user_max_double_spin_box->value();
    statistic.serverMin = ui->m_server_min_double_spin_box->value();
    statistic.serverMax = ui->m_server_max_double_spin_box->value();
    statistic.histogram = ui->m_histogram->getDatas();
}

void EnsembleTransferFunctionEditor::loadStatisticState( StatisticIndex statistic )
{
    const StatisticUiState& state = m_statistics[statistic];
    ui->m_user_min_max_radio_button->setChecked( state.useUserMinMax );
    ui->m_server_min_max_radio_button->setChecked( !state.useUserMinMax );
    ui->m_user_min_double_spin_box->setValue( state.userMin );
    ui->m_user_max_double_spin_box->setValue( state.userMax );
    ui->m_server_min_double_spin_box->setValue( state.serverMin );
    ui->m_server_max_double_spin_box->setValue( state.serverMax );
    ui->m_color_map->setColors( state.colorMap );
    ui->m_opacity_map->setOpacities( state.opacityMap );
    ui->m_histogram->setDatas( state.histogram );
    ui->m_color_map->update();
    ui->m_opacity_map->update();
    ui->m_histogram->update();
}

EnsembleTransferFunctionEditor::StatisticIndex EnsembleTransferFunctionEditor::selectedStatisticIndex() const
{
    const int index = ui->m_statistics_combo_box->currentIndex();
    if( index == VarianceStatistic ) return VarianceStatistic;
    if( index == CoefficientVariationStatistic ) return CoefficientVariationStatistic;
    return AverageStatistic;
}

void EnsembleTransferFunctionEditor::onStatisticSelectionChanged( int )
{
    saveCurrentStatisticState();
    m_current_statistic = selectedStatisticIndex();
    loadStatisticState( m_current_statistic );
}

void EnsembleTransferFunctionEditor::onReceiveEnsembleStatisticsParameter( const QJsonObject& payload )
{
    const QString dataKey = QString::fromUtf8( Protocol::Key::Data );
    if( !payload.value( dataKey ).isArray() )
    {
        qDebug() << "[Client][EnsembleTFE] EnsembleStatisticsParameter has no Data array";
        return;
    }

    auto updateBlock = [&]( StatisticIndex index, const QJsonObject& patch )
    {
        StatisticUiState& state = m_statistics[index];
        const QString& displayName = state.displayName;
        const QString rangeModeKey = QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode );
        const QString userMinKey = QString::fromUtf8( Protocol::Key::EnsembleUserRangeMin );
        const QString userMaxKey = QString::fromUtf8( Protocol::Key::EnsembleUserRangeMax );
        const QString minKey = QString::fromUtf8( Protocol::Key::EnsembleServerRangeMin );
        const QString maxKey = QString::fromUtf8( Protocol::Key::EnsembleServerRangeMax );
        const QString colorMapKey = QString::fromUtf8( Protocol::Key::EnsembleColorMap );
        const QString opacityMapKey = QString::fromUtf8( Protocol::Key::EnsembleOpacityMap );
        const QString histogramKey = QString::fromUtf8( Protocol::Key::EnsembleHistogram );

        if( patch.contains( rangeModeKey ) )
        {
            const int rangeMode = patch.value( rangeModeKey ).toInt();
            state.useUserMinMax = rangeMode != ServerRangeMode;
        }

        if( patch.value( userMinKey ).isDouble() && patch.value( userMaxKey ).isDouble() )
        {
            state.userMin = patch.value( userMinKey ).toDouble();
            state.userMax = patch.value( userMaxKey ).toDouble();
        }

        if( patch.value( minKey ).isDouble() && patch.value( maxKey ).isDouble() )
        {
            const double serverMin = patch.value( minKey ).toDouble();
            const double serverMax = patch.value( maxKey ).toDouble();
            if( !std::isfinite( serverMin ) || !std::isfinite( serverMax ) )
            {
                qDebug() << "[Client][EnsembleTFE] ignore invalid server range for" << displayName
                         << "min =" << serverMin << "max =" << serverMax
                         << "reason = non-finite";
            }
            else if( serverMin > serverMax )
            {
                qDebug() << "[Client][EnsembleTFE] ignore invalid server range for" << displayName
                         << "min =" << serverMin << "max =" << serverMax
                         << "reason = min > max";
            }
            else
            {
                state.serverMin = serverMin;
                state.serverMax = serverMax;
                qDebug() << "[Client][EnsembleTFE] update server range for" << displayName
                         << "min =" << serverMin << "max =" << serverMax;
            }
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] keep server range for" << displayName
                     << "reason = missing ensemble range keys";
        }

        if( patch.contains( colorMapKey ) )
        {
            bool colorMapOk = false;
            const QVector<QColor> colors = ReadColorMap( patch.value( colorMapKey ), &colorMapOk );
            if( colorMapOk )
            {
                state.colorMap = colors;
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep color map for" << displayName
                         << "reason = EnsembleColorMap is invalid";
            }
        }

        if( patch.contains( opacityMapKey ) )
        {
            bool opacityMapOk = false;
            const QVector<float> opacities = ReadOpacityMap( patch.value( opacityMapKey ), &opacityMapOk );
            if( opacityMapOk )
            {
                state.opacityMap = opacities;
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep opacity map for" << displayName
                         << "reason = EnsembleOpacityMap is invalid";
            }
        }

        if( patch.contains( histogramKey ) )
        {
            bool histogramOk = false;
            const std::vector<int> histogram = ReadHistogram( patch.value( histogramKey ), &histogramOk );
            if( histogramOk )
            {
                // Empty histograms are treated as an explicit request to clear the display.
                state.histogram = histogram;
                qDebug() << "[Client][EnsembleTFE] update histogram for" << displayName
                         << "bins =" << histogram.size();
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep histogram for" << displayName
                         << "reason = EnsembleHistogram is not an array";
            }
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] keep histogram for" << displayName
                     << "reason = missing EnsembleHistogram";
        }

        if( index == m_current_statistic ) loadStatisticState( index );
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
        const QString ensembleVariable =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleVariable ) ).toString();
        if( !ensembleVariable.isEmpty() )
        {
            ui->m_statistics_synthesizer_line_edit->setText( ensembleVariable );
        }

        const QString statistic = NormalizeStatisticName(
            patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString() );
        qDebug() << "[Client][EnsembleTFE] receive statistic patch"
                 << patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString()
                 << "normalized =" << statistic;

        if( statistic == QStringLiteral( "average" ) )
        {
            updateBlock( AverageStatistic, patch );
        }
        else if( statistic == QStringLiteral( "variance" ) )
        {
            updateBlock( VarianceStatistic, patch );
        }
        else if( statistic == QStringLiteral( "cv" ) )
        {
            updateBlock( CoefficientVariationStatistic, patch );
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] skip unknown statistic patch";
        }
    }
}

QString EnsembleTransferFunctionEditor::selectedStatistic() const
{
    return m_statistics[selectedStatisticIndex()].statisticName;
}

bool EnsembleTransferFunctionEditor::eventFilter( QObject* watched, QEvent* event )
{
    if( event->type() != QEvent::MouseButtonRelease )
    {
        return QDialog::eventFilter( watched, event );
    }

    auto* mouseEvent = static_cast<QMouseEvent*>( event );
    if( mouseEvent->button() != Qt::LeftButton ) return QDialog::eventFilter( watched, event );

    if( watched == ui->m_color_map )
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

    if( watched == ui->m_opacity_map )
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

    for( const auto& statistic : m_statistics )
    {
        if( !validateStatistic( statistic.displayName,
                                statistic.useUserMinMax,
                                statistic.userMin,
                                statistic.userMax,
                                statistic.serverMin,
                                statistic.serverMax,
                                statistic.colorMap.size(),
                                statistic.opacityMap.size() ) )
        {
            return false;
        }
    }
    return true;
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

    for( const auto& statistic : m_statistics )
    {
        if( !validateUserRange( statistic.displayName,
                                statistic.userMin,
                                statistic.userMax,
                                statistic.colorMap.size(),
                                statistic.opacityMap.size() ) )
        {
            return false;
        }
    }
    return true;
}

void EnsembleTransferFunctionEditor::onApply()
{
    saveCurrentStatisticState();
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
        patch[QString::fromUtf8( Protocol::Key::EnsembleVariable )] = variableExpression;
        patch[QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode )] = rangeMode;
        patch[QString::fromUtf8( Protocol::Key::EnsembleUserRangeMin )] = userMin;
        patch[QString::fromUtf8( Protocol::Key::EnsembleUserRangeMax )] = userMax;
        patch[QString::fromUtf8( Protocol::Key::EnsembleServerRangeMin )] = serverMin;
        patch[QString::fromUtf8( Protocol::Key::EnsembleServerRangeMax )] = serverMax;
        patch[QString::fromUtf8( Protocol::Key::EnsembleColorMap )] = ColorMapToPatchJson( colors );
        patch[QString::fromUtf8( Protocol::Key::EnsembleOpacityMap )] = OpacityMapToPatchJson( opacities );
        data.append( patch );
    };

    QJsonArray data;
    for( const auto& statistic : m_statistics )
    {
        appendStatisticPatch( data,
                              statistic.statisticName,
                              statistic.useUserMinMax,
                              statistic.userMin,
                              statistic.userMax,
                              statistic.serverMin,
                              statistic.serverMax,
                              statistic.colorMap,
                              statistic.opacityMap );
    }

    QJsonObject root;
    root[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::EnsembleStatisticsParameter );
    root[QString::fromUtf8( Protocol::Key::Data )] = data;

    m_web_sockets->text()->sendTextMessage( QJsonDocument( root ).toJson( QJsonDocument::Compact ) );
}

void EnsembleTransferFunctionEditor::onExport()
{
    saveCurrentStatisticState();
    if( !validateForExport() ) return;

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr( "Export Ensemble Transfer Function File" ),
        QStringLiteral( "default.json" ),
        tr( "JSON Files (*.json)" ) );
    if( fileName.isEmpty() ) return;

    const QString variableExpression = ui->m_statistics_synthesizer_line_edit->text().trimmed();
    auto exportSetting = [&]( StatisticIndex index )
    {
        const StatisticUiState& state = m_statistics[index];
        return EnsembleTFSetting {
            state.displayName,
            state.statisticName,
            variableExpression,
            state.colorMap,
            state.opacityMap,
            true,
            state.userMin,
            state.userMax,
            state.userMin,
            state.userMax,
            state.histogram
        };
    };

    const EnsembleTFSetting average = exportSetting( AverageStatistic );
    const EnsembleTFSetting variance = exportSetting( VarianceStatistic );
    const EnsembleTFSetting coefficientVariation = exportSetting( CoefficientVariationStatistic );

    QJsonObject transferFunctionSettings;
    transferFunctionSettings["transfer_function_count"] = 1;
    transferFunctionSettings["transfer_function_resolution"] = 256;
    transferFunctionSettings["color_synthesis"] = QStringLiteral( "C1" );
    transferFunctionSettings["opacity_synthesis"] = QStringLiteral( "O1" );

    QJsonObject settings;
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
    saveCurrentStatisticState();

    bool hadMissingKeys = false;
    QString importedExpression;

    auto importStatistic = [&]( const QString& sectionName, StatisticIndex index )
    {
        StatisticUiState& state = m_statistics[index];
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
        double userMin = state.userMin;
        double userMax = state.userMax;
        double serverMin = state.serverMin;
        double serverMax = state.serverMax;
        if( !ReadRange( color, &useUser, &userMin, &userMax, &serverMin, &serverMax ) ) hadMissingKeys = true;

        state.userMin = userMin;
        state.userMax = userMax;
        state.serverMin = serverMin;
        state.serverMax = serverMax;
        state.useUserMinMax = useUser;

        bool colorMapOk = false;
        const QVector<QColor> colors = ReadColorMap( color.value( "map" ), &colorMapOk );
        if( colorMapOk ) state.colorMap = colors;
        else hadMissingKeys = true;

        bool opacityMapOk = false;
        const QVector<float> opacities = ReadOpacityMap( opacity.value( "map" ), &opacityMapOk );
        if( opacityMapOk ) state.opacityMap = opacities;
        else hadMissingKeys = true;
    };

    // Missing optional/default.json fields keep the current GUI values so a partial file can still be useful.
    importStatistic( QStringLiteral( "mean_transfer_functions" ), AverageStatistic );
    importStatistic( QStringLiteral( "variance_transfer_functions" ), VarianceStatistic );
    importStatistic( QStringLiteral( "coefficient_of_variation_transfer_functions" ), CoefficientVariationStatistic );

    if( !importedExpression.isEmpty() ) ui->m_statistics_synthesizer_line_edit->setText( importedExpression );

    loadStatisticState( m_current_statistic );

    if( hadMissingKeys )
    {
        QMessageBox::warning( this,
                              tr( "Ensemble Transfer Function Editor" ),
                              tr( "Some default.json keys were missing. Existing GUI values were kept for those fields." ) );
    }
}
