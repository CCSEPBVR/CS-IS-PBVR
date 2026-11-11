#include "EnsembleTransferFunctionEditor.h"
#include "ui_EnsembleTransferFunctionEditor.h"

#include "ColorMap.h"
#include "OpacityMap.h"
#include "../ExtendedQT/Histogram.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QVariant>

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

    QJsonObject range;
    range["active_range"] = QStringLiteral( "user" );
    range["user"] = user;
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
                double* userMax )
{
    if( !parent.value( "range" ).isObject() ) return false;

    const QJsonObject range = parent.value( "range" ).toObject();
    const QString activeRange = range.value( "active_range" ).toString();
    if( !activeRange.isEmpty() ) *useUserMinMax = activeRange != QStringLiteral( "server" );

    // Server MinMax is runtime state supplied by the server and is intentionally not imported.
    bool ok = true;
    if( range.value( "user" ).isObject() )
    {
        const QJsonObject user = range.value( "user" ).toObject();
        ok = ReadDoubleMember( user, "min", userMin ) && ok;
        ok = ReadDoubleMember( user, "max", userMax ) && ok;
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

// "member:N" 形式の statistic なら N を返し、そうでなければ -1 を返す。
// 表示対象のアンサンブルメンバー指定（サーバー側 Server.cpp / Worker.h が受け付ける形式）。
int ParseMemberStatistic( const QString& statistic )
{
    const QString normalized = statistic.trimmed().toLower();
    const QString prefix = QStringLiteral( "member:" );
    if( !normalized.startsWith( prefix ) ) return -1;
    bool ok = false;
    const int member = normalized.mid( prefix.size() ).toInt( &ok );
    return ( ok && member >= 0 ) ? member : -1;
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

// 常用対数モードの注記。対数化の対象は平均・分散のみで、変動係数は対数化前の
// 平均・分散から算出されるため対象外。MinMax 欄に表示される値のスケールを
// 取り違えないよう、グループタイトルに [log10] を付けて示す。
void UpdateMinMaxGroupTitle( Ui::EnsembleTransferFunctionEditor* ui, const bool logAppliesToCurrentStatistic )
{
    const bool logScale = ui->m_log_scale_statistics_check_box->isChecked();
    ui->m_min_max_group_box->setTitle( ( logScale && logAppliesToCurrentStatistic )
                                           ? QStringLiteral( "Ensemble MinMax [log10]" )
                                           : QStringLiteral( "Ensemble MinMax" ) );
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
    connect( ui->m_log_scale_statistics_check_box, &QCheckBox::toggled, this,
             [this]( const bool checked )
             {
                 qDebug() << "[Client][EnsembleTFE] log_scale_statistics checkbox:" << checked
                          << "(sent to server on Apply)";
                 UpdateMinMaxGroupTitle( ui, m_current_statistic != CoefficientVariationStatistic );
             } );
}

EnsembleTransferFunctionEditor::~EnsembleTransferFunctionEditor()
{
    delete ui;
}

void EnsembleTransferFunctionEditor::emitLegendTransferFunctionUpdate()
{
    saveCurrentStatisticState();
    m_legend_transfer_function = createLegendTransferFunction();
    emit updateLegendTransferFunction( &m_legend_transfer_function, static_cast<int>( selectedStatisticIndex() ) );
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

    // コンボボックスの統計量3項目（.ui 定義）に itemData = -1 を入れる。
    // メンバー項目は itemData = メンバーID（0以上）で追加し、種別は index でなく itemData で判定する。
    for( int i = 0; i < StatisticCount; ++i )
    {
        ui->m_statistics_combo_box->setItemData( i, QVariant( -1 ) );
    }

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
    ui->m_log_scale_statistics_check_box->setChecked( false );
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
    // 切断・再接続時はメンバー項目をいったん消す（接続後に NumEnsemble を受信して作り直す）
    m_num_ensemble = 0;
    m_current_member = -1;
    rebuildMemberItems();
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
    UpdateMinMaxGroupTitle( ui, statistic != CoefficientVariationStatistic );
}

EnsembleTransferFunctionEditor::StatisticIndex EnsembleTransferFunctionEditor::selectedStatisticIndex() const
{
    // メンバー選択中は直前に選ばれていた統計量を返す（パネル・凡例はその状態を保持し続ける）
    if( selectedMemberId() >= 0 ) return m_current_statistic;
    const int index = ui->m_statistics_combo_box->currentIndex();
    if( index == VarianceStatistic ) return VarianceStatistic;
    if( index == CoefficientVariationStatistic ) return CoefficientVariationStatistic;
    return AverageStatistic;
}

// コンボボックスで選択中のメンバーID。統計量項目が選ばれていれば -1。
int EnsembleTransferFunctionEditor::selectedMemberId() const
{
    const QVariant data = ui->m_statistics_combo_box->currentData();
    if( !data.isValid() ) return -1;  // itemData の無い項目（セパレータ等）
    const int member = data.toInt();
    return ( member >= 0 ) ? member : -1;
}

// コンボボックスのメンバー項目（統計量3項目より後ろ）を m_num_ensemble に合わせて作り直す。
// 選択中のメンバーが作り直し後も存在すれば選択を維持し、無くなれば直前の統計量へ戻す。
void EnsembleTransferFunctionEditor::rebuildMemberItems()
{
    QComboBox* combo = ui->m_statistics_combo_box;
    const int previousMember = selectedMemberId();
    const bool wasBlocked = combo->blockSignals( true );

    while( combo->count() > StatisticCount )
    {
        combo->removeItem( combo->count() - 1 );
    }
    if( m_num_ensemble > 0 )
    {
        combo->insertSeparator( combo->count() );
        for( int member = 0; member < m_num_ensemble; ++member )
        {
            combo->addItem( QStringLiteral( "Member %1" ).arg( member ), QVariant( member ) );
        }
    }

    bool fallbackToStatistic = false;
    if( previousMember >= 0 )
    {
        const int index = combo->findData( QVariant( previousMember ) );
        if( index >= 0 )
        {
            combo->setCurrentIndex( index );
        }
        else
        {
            m_current_member = -1;
            combo->setCurrentIndex( static_cast<int>( m_current_statistic ) );
            fallbackToStatistic = true;
        }
    }

    combo->blockSignals( wasBlocked );
    if( fallbackToStatistic ) loadStatisticState( m_current_statistic );
}

TransferFunction EnsembleTransferFunctionEditor::createLegendTransferFunction() const
{
    return createLegendTransferFunction( m_statistics );
}

TransferFunction EnsembleTransferFunctionEditor::createLegendTransferFunction( const std::array<StatisticUiState, StatisticCount>& statistics ) const
{
    TransferFunction transferFunction;
    const QString variableExpression = ui->m_statistics_synthesizer_line_edit->text().trimmed();
    transferFunction.setColorSynthesizer( variableExpression.toUtf8().constData() );
    transferFunction.setOpacitySynthesizer( variableExpression.toUtf8().constData() );

    for( const auto& statistic : statistics )
    {
        TransferFunction::Item item;
        item.color.variable = variableExpression.toUtf8().constData();
        item.color.rangeMode = statistic.useUserMinMax ? TransferFunction::UserRange : TransferFunction::ServerSide;
        item.color.userDefinedMinMax = { statistic.userMin, statistic.userMax };
        item.color.serverSideMinMax = { statistic.serverMin, statistic.serverMax };
        item.color.histogram = statistic.histogram;
        item.color.map.reserve( static_cast<size_t>( statistic.colorMap.size() ) );
        for( const QColor& color : statistic.colorMap )
        {
            item.color.map.emplace_back(
                static_cast<unsigned char>( color.red() ),
                static_cast<unsigned char>( color.green() ),
                static_cast<unsigned char>( color.blue() ) );
        }

        item.opacity.variable = variableExpression.toUtf8().constData();
        item.opacity.rangeMode = item.color.rangeMode;
        item.opacity.userDefinedMinMax = item.color.userDefinedMinMax;
        item.opacity.serverSideMinMax = item.color.serverSideMinMax;
        item.opacity.map.reserve( static_cast<size_t>( statistic.opacityMap.size() ) );
        for( const float opacity : statistic.opacityMap )
        {
            item.opacity.map.push_back( opacity );
        }

        transferFunction.addTransferFunction( item );
    }

    return transferFunction;
}

void EnsembleTransferFunctionEditor::onStatisticSelectionChanged( int )
{
    saveCurrentStatisticState();
    const int member = selectedMemberId();
    if( member >= 0 )
    {
        // メンバー選択中はパネルを直前の統計量の状態のまま残す
        // （メンバー粒子は生成済みデータの表示であり、伝達関数の編集対象ではないため）
        m_current_member = member;
        return;
    }
    m_current_member = -1;
    m_current_statistic = selectedStatisticIndex();
    loadStatisticState( m_current_statistic );
}

void EnsembleTransferFunctionEditor::onReceiveEnsembleStatisticsParameter( const QJsonObject& payload )
{
    // アンサンブルメンバー数（root レベル）。Member 選択肢を作る。
    // 直後の root Statistic（member:N）の復元でコンボボックス項目を参照するため、先に処理する。
    const QJsonValue receivedNumEnsemble =
        payload.value( QString::fromUtf8( Protocol::Key::NumEnsemble ) );
    if( receivedNumEnsemble.isDouble() )
    {
        const int numEnsemble = qMax( 0, receivedNumEnsemble.toInt() );
        if( numEnsemble != m_num_ensemble )
        {
            qDebug() << "[Client][EnsembleTFE] receive num_ensemble=" << numEnsemble;
            m_num_ensemble = numEnsemble;
            rebuildMemberItems();
        }
    }
    else
    {
        qDebug() << "[Client][EnsembleTFE] EnsembleStatisticsParameter has no NumEnsemble key"
                 << "(server does not include the feature?)";
    }

    const QString rawRootStatistic =
        payload.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString();
    const int rootMember = ParseMemberStatistic( rawRootStatistic );
    if( rootMember >= 0 )
    {
        // member:N の復元。接続直後の publish と、他クライアントの Apply の再 publish が届く。
        const int index = ui->m_statistics_combo_box->findData( QVariant( rootMember ) );
        if( index >= 0 )
        {
            saveCurrentStatisticState();
            m_current_member = rootMember;
            const bool wasBlocked = ui->m_statistics_combo_box->blockSignals( true );
            ui->m_statistics_combo_box->setCurrentIndex( index );
            ui->m_statistics_combo_box->blockSignals( wasBlocked );
            // パネルは直前の統計量の状態のまま（onStatisticSelectionChanged と同じ扱い）
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] skip member restore:" << rawRootStatistic
                     << "(no combo box item; num_ensemble =" << m_num_ensemble << ")";
        }
    }

    // member:N でなければ従来どおり統計量名として解釈する
    const QString rootStatistic = ( rootMember >= 0 )
        ? QString()
        : NormalizeStatisticName( rawRootStatistic );
    if( !rootStatistic.isEmpty() )
    {
        StatisticIndex receivedStatistic = AverageStatistic;
        if( rootStatistic == QStringLiteral( "variance" ) )
        {
            receivedStatistic = VarianceStatistic;
        }
        else if( rootStatistic == QStringLiteral( "cv" ) )
        {
            receivedStatistic = CoefficientVariationStatistic;
        }

        saveCurrentStatisticState();
        m_current_member = -1;
        m_current_statistic = receivedStatistic;
        const bool wasBlocked = ui->m_statistics_combo_box->blockSignals( true );
        ui->m_statistics_combo_box->setCurrentIndex( receivedStatistic );
        ui->m_statistics_combo_box->blockSignals( wasBlocked );
        loadStatisticState( m_current_statistic );
    }

    // 常用対数モードの現在値（root レベル）。接続時の初期 publish と、
    // 他クライアントの Apply をサーバが再 publish したものの両方がここに届く。
    const QJsonValue receivedLogScale =
        payload.value( QString::fromUtf8( Protocol::Key::LogScaleStatistics ) );
    if( receivedLogScale.isBool() )
    {
        qDebug() << "[Client][EnsembleTFE] receive log_scale_statistics=" << receivedLogScale.toBool();
        ui->m_log_scale_statistics_check_box->setChecked( receivedLogScale.toBool() );
    }
    else
    {
        qDebug() << "[Client][EnsembleTFE] EnsembleStatisticsParameter has no LogScaleStatistics key"
                 << "(server does not include the feature?)";
    }

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

    emitLegendTransferFunctionUpdate();
}

void EnsembleTransferFunctionEditor::onReceiveRequestDataAtEnsembleStatisticsParameter( const QJsonObject& payload )
{
    saveCurrentStatisticState();

    const QString dataKey = QString::fromUtf8( Protocol::Key::Data );
    if( !payload.value( dataKey ).isArray() )
    {
        qDebug() << "[Client][EnsembleTFE] RequestDataAt EnsembleStatisticsParameter has no Data array";
        return;
    }

    std::array<StatisticUiState, StatisticCount> legendStatistics = m_statistics;

    auto updateBlock = [&]( StatisticIndex index, const QJsonObject& patch )
    {
        StatisticUiState& state = m_statistics[index];
        StatisticUiState& legendState = legendStatistics[index];
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
            legendState.useUserMinMax = rangeMode != ServerRangeMode;
        }

        if( patch.value( userMinKey ).isDouble() && patch.value( userMaxKey ).isDouble() )
        {
            legendState.userMin = patch.value( userMinKey ).toDouble();
            legendState.userMax = patch.value( userMaxKey ).toDouble();
        }

        if( patch.value( minKey ).isDouble() && patch.value( maxKey ).isDouble() )
        {
            const double serverMin = patch.value( minKey ).toDouble();
            const double serverMax = patch.value( maxKey ).toDouble();
            if( !std::isfinite( serverMin ) || !std::isfinite( serverMax ) )
            {
                qDebug() << "[Client][EnsembleTFE] ignore invalid RequestDataAt server range for" << displayName
                         << "min =" << serverMin << "max =" << serverMax
                         << "reason = non-finite";
            }
            else if( serverMin > serverMax )
            {
                qDebug() << "[Client][EnsembleTFE] ignore invalid RequestDataAt server range for" << displayName
                         << "min =" << serverMin << "max =" << serverMax
                         << "reason = min > max";
            }
            else
            {
                state.serverMin = serverMin;
                state.serverMax = serverMax;
                legendState.serverMin = serverMin;
                legendState.serverMax = serverMax;
                qDebug() << "[Client][EnsembleTFE] update RequestDataAt server range for" << displayName
                         << "min =" << serverMin << "max =" << serverMax;
            }
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] keep RequestDataAt server range for" << displayName
                     << "reason = missing ensemble range keys";
        }

        if( patch.contains( colorMapKey ) )
        {
            bool colorMapOk = false;
            const QVector<QColor> colors = ReadColorMap( patch.value( colorMapKey ), &colorMapOk );
            if( colorMapOk )
            {
                legendState.colorMap = colors;
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep RequestDataAt color map for" << displayName
                         << "reason = EnsembleColorMap is invalid";
            }
        }

        if( patch.contains( opacityMapKey ) )
        {
            bool opacityMapOk = false;
            const QVector<float> opacities = ReadOpacityMap( patch.value( opacityMapKey ), &opacityMapOk );
            if( opacityMapOk )
            {
                legendState.opacityMap = opacities;
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep RequestDataAt opacity map for" << displayName
                         << "reason = EnsembleOpacityMap is invalid";
            }
        }

        if( patch.contains( histogramKey ) )
        {
            bool histogramOk = false;
            const std::vector<int> histogram = ReadHistogram( patch.value( histogramKey ), &histogramOk );
            if( histogramOk )
            {
                state.histogram = histogram;
                legendState.histogram = histogram;
                qDebug() << "[Client][EnsembleTFE] update RequestDataAt histogram for" << displayName
                         << "bins =" << histogram.size();
            }
            else
            {
                qDebug() << "[Client][EnsembleTFE] keep RequestDataAt histogram for" << displayName
                         << "reason = EnsembleHistogram is not an array";
            }
        }
        else
        {
            qDebug() << "[Client][EnsembleTFE] keep RequestDataAt histogram for" << displayName
                     << "reason = missing EnsembleHistogram";
        }

        if( index == m_current_statistic ) loadStatisticState( index );
    };

    const QJsonArray data = payload.value( dataKey ).toArray();
    for( const auto& value : data )
    {
        if( !value.isObject() )
        {
            qDebug() << "[Client][EnsembleTFE] skip non-object RequestDataAt statistics patch";
            continue;
        }

        const QJsonObject patch = value.toObject();
        const QString statistic = NormalizeStatisticName(
            patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString() );

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
            qDebug() << "[Client][EnsembleTFE] skip unknown RequestDataAt statistic patch";
        }
    }

    m_legend_transfer_function = createLegendTransferFunction( legendStatistics );
    emit updateLegendTransferFunction( &m_legend_transfer_function, static_cast<int>( selectedStatisticIndex() ) );
}

QString EnsembleTransferFunctionEditor::selectedStatistic() const
{
    // メンバー選択中は "member:N" を返す。サーバーは統計量名と同じ経路でこれを受け付け、
    // PARTICLE_DIR/ensNNNN/t_* の粒子を返す（Server/Utils/Worker.h resolveParticlePath）。
    const int member = selectedMemberId();
    if( member >= 0 ) return QStringLiteral( "member:%1" ).arg( member );
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
                              statistic.colorMap,
                              statistic.opacityMap );
    }

    QJsonObject root;
    root[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::EnsembleStatisticsParameter );
    root[QString::fromUtf8( Protocol::Key::Statistic )] = selectedStatistic();
    // 常用対数モードは平均・分散をまとめて1つの設定なので、統計量ごとの Data ではなく root レベルで送る
    root[QString::fromUtf8( Protocol::Key::LogScaleStatistics )] =
        ui->m_log_scale_statistics_check_box->isChecked();
    root[QString::fromUtf8( Protocol::Key::Data )] = data;

    // 送信ペイロードの検証ログ。チェックボックスではなく root に実際に入った値を読み戻して出す。
    qDebug() << "[Client][EnsembleTFE] send EnsembleStatisticsParameter:"
             << "statistic=" << root.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString()
             << "log_scale_statistics="
             << root.value( QString::fromUtf8( Protocol::Key::LogScaleStatistics ) ).toBool()
             << "patches=" << data.size();

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

    QJsonObject ensembleSettings;
    ensembleSettings["log_scale_statistics"] = ui->m_log_scale_statistics_check_box->isChecked();
    settings["ensemble"] = ensembleSettings;

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
        if( !ReadRange( color, &useUser, &userMin, &userMax ) ) hadMissingKeys = true;

        state.userMin = userMin;
        state.userMax = userMax;
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

    // settings.ensemble.log_scale_statistics は optional。無ければ現在の GUI 値を保つ
    // （サーバ側 reader も欠損時は既定値扱い。既存ファイルに ensemble キーは無いため警告にしない）。
    const QJsonValue importedLogScale = root.value( QStringLiteral( "settings" ) ).toObject()
                                            .value( QStringLiteral( "ensemble" ) ).toObject()
                                            .value( QStringLiteral( "log_scale_statistics" ) );
    if( importedLogScale.isBool() )
    {
        ui->m_log_scale_statistics_check_box->setChecked( importedLogScale.toBool() );
    }

    if( !importedExpression.isEmpty() ) ui->m_statistics_synthesizer_line_edit->setText( importedExpression );

    loadStatisticState( m_current_statistic );

    if( hadMissingKeys )
    {
        QMessageBox::warning( this,
                              tr( "Ensemble Transfer Function Editor" ),
                              tr( "Some default.json keys were missing. Existing GUI values were kept for those fields." ) );
    }
}
