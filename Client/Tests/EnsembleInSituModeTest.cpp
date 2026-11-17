#include "EnsembleInSituModeTest.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QComboBox>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSet>
#include <QSettings>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTest>
#include <QTextStream>
#include <QTreeView>

#include <algorithm>
#include <cmath>

#include <kvs/qt/Application>
#include <kvs/PointObject>

#include "../App/MainWindow.h"
#include "../ExtendedQT/ColorMap.h"
#include "../ExtendedQT/OpacityMap.h"
#include "../Widgets/Communication.h"
#include "../Widgets/EnsembleTransferFunctionEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "../Widgets/TotalParticlesToolBar.h"
#include "../../Shared/JsonKeys.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_data_request_timeout_ms = 180000;
constexpr int k_echo_observe_ms = 3000;   // サーバ再配信の観察時間(参考情報。単一クライアントでは届かない)
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_capture_settle_ms = 700;
constexpr int k_quiet_request_ms = 3000;
constexpr int k_histogram_resolution = 256;
const char* const k_item_a = "S-IS-33";
const char* const k_item_c = "S-IS-35";
kvs::qt::Application* g_test_app = nullptr;

QString findRepoRootFrom( const QString& start_path )
{
    QDir dir( start_path );
    while ( dir.exists() )
    {
        if ( dir.exists( QStringLiteral( ".git" ) ) &&
             dir.exists( QStringLiteral( "Client" ) ) &&
             dir.exists( QStringLiteral( "Server" ) ) )
        {
            return dir.absolutePath();
        }
        if ( !dir.cdUp() ) { break; }
    }
    return QString();
}

QString normalizeStatisticName( QString name )
{
    name = name.trimmed().toLower();
    if ( name == QStringLiteral( "mean" ) ) { return QStringLiteral( "average" ); }
    if ( name == QStringLiteral( "coefficient of variation" ) || name == QStringLiteral( "coefficient_of_variation" ) )
    {
        return QStringLiteral( "cv" );
    }
    return name;
}

bool opacitiesNearlyEqual( const QVector<float>& a, const QVector<float>& b )
{
    if ( a.size() != b.size() ) { return false; }
    for ( int i = 0; i < a.size(); ++i )
    {
        if ( std::abs( a.at( i ) - b.at( i ) ) > 1.0e-6f ) { return false; }
    }
    return true;
}
}

namespace ClientTests
{

EnsembleInSituModeTest::EnsembleInSituModeTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArrayLiteral( "1800000" ) );
}

QString EnsembleInSituModeTest::repoRootPath() const
{
    const QString app_root = findRepoRootFrom( QCoreApplication::applicationDirPath() );
    if ( !app_root.isEmpty() ) { return app_root; }

    const QString cwd_root = findRepoRootFrom( QDir::currentPath() );
    if ( !cwd_root.isEmpty() ) { return cwd_root; }

    const QString source_root =
        findRepoRootFrom( QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() );
    if ( !source_root.isEmpty() ) { return source_root; }

    return QDir::currentPath();
}

QString EnsembleInSituModeTest::configuredValue( const char* name ) const
{
    const QString env = QProcessEnvironment::systemEnvironment().value( QString::fromLatin1( name ) );
    if ( !env.trimmed().isEmpty() ) { return env.trimmed(); }

    QSettings settings( testPathConfigFilePath( repoRootPath() ), QSettings::IniFormat );
    settings.beginGroup( QStringLiteral( "paths" ) );
    const QString value = settings.value( QString::fromLatin1( name ) ).toString();
    settings.endGroup();
    return value.trimmed();
}

QString EnsembleInSituModeTest::configuredPathOrEmpty( const char* name ) const
{
    if ( configuredValue( name ).isEmpty() ) { return QString(); }
    return configuredPath( name, repoRootPath() );
}

bool EnsembleInSituModeTest::waitForCondition(
    const std::function<bool()>& condition,
    int timeout_ms,
    int interval_ms ) const
{
    QElapsedTimer timer;
    timer.start();
    while ( timer.elapsed() < timeout_ms )
    {
        if ( condition() ) { return true; }
        QTest::qWait( interval_ms );
    }
    return condition();
}

void EnsembleInSituModeTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void EnsembleInSituModeTest::bringEditorToFront( EnsembleTransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "EnsembleTransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void EnsembleInSituModeTest::markStep( const QString& item_id, const QString& description, const QString& status )
{
    const QString line = QStringLiteral( "%1 %2" ).arg( item_id, description );
    m_steps.push_back( { line, status } );
    qInfo().noquote() << QStringLiteral( "[%1] %2" ).arg( status, line );
}

bool EnsembleInSituModeTest::check( ItemResult& result, bool condition, const QString& message )
{
    if ( condition ) { return true; }
    result.messages.push_back( message );
    result.status = QStringLiteral( "FAIL" );
    markStep( result.id, message, QStringLiteral( "FAIL" ) );
    return false;
}

void EnsembleInSituModeTest::skipItem( ItemResult& result, const QString& reason )
{
    result.status = QStringLiteral( "SKIP" );
    result.messages.push_back( reason );
    markStep( result.id, reason, QStringLiteral( "SKIP" ) );
}

void EnsembleInSituModeTest::finishItem( ItemResult& result )
{
    if ( result.status == QStringLiteral( "NOT RUN" ) ) { result.status = QStringLiteral( "PASS" ); }
    m_item_results.push_back( result );
    markStep( result.id, QStringLiteral( "%1: %2" ).arg( result.title, result.status ), result.status );
}

// 既存テスト(InSituModeTest)と同じ方法(プライマリスクリーン全体)で撮影し、
// テスト用ファイルに加えてマニュアル用のコピー(img/manual/ 以下)も出力する。
void EnsembleInSituModeTest::saveScreenshot(
    ItemResult& result,
    int seq,
    const QString& content,
    const QString& caption,
    const QString& manual_rel_path,
    int time_step )
{
    ScreenshotEntry entry;
    entry.item_id = result.id;
    entry.seq = seq;
    entry.file_name = QStringLiteral( "%1_%2_%3.png" ).arg( result.id ).arg( seq ).arg( content );
    entry.caption = caption;
    entry.manual_rel_path = manual_rel_path;
    entry.time_step = time_step;

    if ( !ClientTests::screenshotsEnabled() )
    {
        m_screenshots.push_back( entry );
        markStep( result.id, QStringLiteral( "スクリーンショット(連番 %1)は撮影無効のため SKIP" ).arg( seq ), QStringLiteral( "SKIP" ) );
        return;
    }

    QTest::qWait( k_capture_settle_ms );
    QScreen* screen = QGuiApplication::primaryScreen();
    if ( !check( result, screen != nullptr, QStringLiteral( "プライマリスクリーンが見つからない(連番 %1)" ).arg( seq ) ) )
    {
        m_screenshots.push_back( entry );
        return;
    }

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( entry.file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );
    if ( !check( result, !screenshot.isNull(), QStringLiteral( "スクリーンショットの取得に失敗(連番 %1)" ).arg( seq ) ) )
    {
        m_screenshots.push_back( entry );
        return;
    }
    entry.captured = screenshot.save( file_path );
    check( result, entry.captured, QStringLiteral( "スクリーンショットの保存に失敗: %1" ).arg( file_path ) );
    if ( entry.captured )
    {
        const QString destination = QDir( m_manual_dir_path ).absoluteFilePath( manual_rel_path );
        QDir().mkpath( QFileInfo( destination ).absolutePath() );
        if ( QFileInfo::exists( destination ) ) { QFile::remove( destination ); }
        entry.copied = QFile::copy( file_path, destination );
        check( result, entry.copied, QStringLiteral( "マニュアル用コピーに失敗: %1" ).arg( manual_rel_path ) );
    }
    m_screenshots.push_back( entry );
    markStep( result.id, QStringLiteral( "スクリーンショットを撮影: %1 → manual: img/manual/%2" ).arg( caption, manual_rel_path ) );
}

void EnsembleInSituModeTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    if ( !report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        qWarning().noquote() << QStringLiteral( "Failed to write test report: %1" ).arg( m_report_path );
        return;
    }

    QTextStream stream( &report_file );
    stream << "# EnsembleInSituModeTest\n\n";
    stream << "- 対応項目: PBVRリグレッションテストチェックシート Server シート ISPBVR 33 / 35(追加項目仕様_アンサンブル S-IS-33 / S-IS-35)\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- 前提: サーバ(pbvr_server)とアンサンブルシミュレーション(ens_Hydrogen_unstruct、RESET.sh 済み、default.json / default_old.json あり)は起動済み。テストからは起動・終了しない\n";
    stream << "- 接続モード: In-Situ(RemoteInSitu)\n";
    stream << "- ENSEMBLE_VIS_PARAM_DIR: `" << ( m_vis_param_dir.isEmpty() ? QStringLiteral( "(未設定)" ) : m_vis_param_dir ) << "`\n";
    stream << "- ENSEMBLE_PARTICLE_DIR: `" << ( m_particle_dir.isEmpty() ? QStringLiteral( "(未設定)" ) : m_particle_dir ) << "`\n";
    stream << "- PBVR_SERVER_STDERR_LOG: `" << ( m_server_stderr_log.isEmpty() ? QStringLiteral( "(未設定)" ) : m_server_stderr_log ) << "`\n";
    stream << "- TF_NAME: `" << ( m_tf_name.isEmpty() ? QStringLiteral( "default" ) : m_tf_name ) << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`(マニュアル用コピー: `img/manual/`)\n\n";

    stream << "## 項目結果\n\n";
    stream << "| 項目ID | 内容 | 結果 | 備考 |\n|---|---|---|---|\n";
    for ( const ItemResult& item : m_item_results )
    {
        QString notes = item.messages.join( QStringLiteral( "<br>" ) );
        notes.replace( QLatin1Char( '|' ), QStringLiteral( "\\|" ) );
        stream << "| " << item.id << " | " << item.title << " | " << item.status << " | " << notes << " |\n";
    }

    stream << "\n## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << step.status << ": " << step.description << "\n";
    }

    stream << "\n## 初期 EnsembleStatisticsParameter\n\n";
    stream << "- Data 要素の Statistic: " << ( m_initial_statistics.isEmpty() ? QStringLiteral( "(未受信)" ) : m_initial_statistics.join( QStringLiteral( ", " ) ) ) << "\n";

    stream << "\n## サーバ側ファイル(S-IS-33)\n\n";
    if ( m_checked_files.isEmpty() )
    {
        stream << "- 確認なし(ENSEMBLE_VIS_PARAM_DIR / ENSEMBLE_PARTICLE_DIR 未設定、または確認前に失敗)\n";
    }
    for ( const QString& file : m_checked_files )
    {
        stream << "- " << file << "\n";
    }

    stream << "\n## S-IS-35 の対象ステップ\n\n";
    if ( m_part_c_time_step >= 0 )
    {
        stream << "- time step: " << m_part_c_time_step << "\n";
        stream << "- 編集したファイル: `" << m_part_c_target_path << "`\n";
        stream << "- バックアップ: `" << m_part_c_backup_path << "`(テスト終了時に元へ戻す)\n";
    }
    else
    {
        stream << "- 実施なし(SKIP または失敗)\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.item_id << "-" << entry.seq << " " << entry.caption << "\n\n";
        stream << "- マニュアル用コピー先: `img/manual/" << entry.manual_rel_path << "`("
               << ( entry.copied ? "出力済み" : ( entry.captured ? "コピー失敗" : "SKIP" ) ) << ")\n\n";
        if ( entry.captured )
        {
            stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
        }
    }

    stream << "## マニュアル用画像\n\n";
    stream << "| 項目ID | 連番 | テスト用ファイル | マニュアル用コピー先 | タイムステップ | 状態 |\n|---|---|---|---|---|---|\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "| " << entry.item_id << " | " << entry.seq << " | `img/" << entry.file_name << "` | `img/manual/" << entry.manual_rel_path
               << "` | " << ( entry.time_step >= 0 ? QString::number( entry.time_step ) : QStringLiteral( "-" ) )
               << " | " << ( entry.copied ? "出力済み" : "未出力(SKIP)" ) << " |\n";
    }

    stream << "\n## 備考\n\n";
    stream << "- S-IS-34(default_old.json / default.json 欠損時)はサーバ側スクリプト `Server/Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/test_missing_default_json.sh` で確認する。\n";
    stream << "- 「次のタイムステップ更新を待つ」は、Apply 後に TimeStepControlToolBar::requestDataAt を受信し、同じタイムステップの dataRequestCompleted を受信するまで待機することを指す。\n";
    stream << "- 粒子の変化は画像比較ではなく、表示中の PointObject(座標・色・法線・サイズ)のフィンガープリント(SHA-256)で判定する。\n";
    stream << "- IsEnsemble はサーバが Initialize 時に送る SelectedFile イベントで受信する(ObjectInfoParameter には含まれない)。\n";
    stream << "- サーバの再配信(EnsembleStatisticsParameter / RepetitionLevelParameter)は uWebSockets の ws->publish が送信元を除外するため単一クライアントでは受信できず、参考情報として記録するのみ。\n";
}

EnsembleInSituModeTest::ClientHandles
EnsembleInSituModeTest::resolveClientHandles( MainWindow& window ) const
{
    ClientHandles handles;
    const auto require = []( bool condition, const char* message )
    {
        if ( condition ) { return true; }
        QTest::qFail( message, __FILE__, __LINE__ );
        return false;
    };

    handles.main_window = &window;
    handles.communication = window.findChild<Communication*>();
    handles.object_editor = window.findChild<ObjectEditor*>();
    handles.playback_tool_bar = window.findChild<::PlayBackControlToolBar*>();
    handles.time_step_tool_bar = window.findChild<TimeStepControlToolBar*>();
    handles.total_particles_tool_bar = window.findChild<TotalParticlesToolBar*>();
    handles.ensemble_editor = window.findChild<EnsembleTransferFunctionEditor*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.total_particles_tool_bar != nullptr, "TotalParticlesToolBar not found" ) ) { return handles; }
    if ( !require( handles.ensemble_editor != nullptr, "EnsembleTransferFunctionEditor not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_insitu_radio = handles.communication->findChild<QRadioButton*>( "remoteVizInsituRadioButton" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.keep_last_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_keep_last_push_button" );
    handles.next_time_step_spin_box = handles.time_step_tool_bar->findChild<QSpinBox*>( "m_next_time_step_spin_box" );
    handles.ensemble_apply_button = handles.ensemble_editor->findChild<QPushButton*>( "m_apply_push_button" );
    handles.variable_line_edit = handles.ensemble_editor->findChild<QLineEdit*>( "m_statistics_synthesizer_line_edit" );
    handles.statistic_combo_box = handles.ensemble_editor->findChild<QComboBox*>( "m_statistics_combo_box" );
    handles.user_min_max_radio = handles.ensemble_editor->findChild<QRadioButton*>( "m_user_min_max_radio_button" );
    handles.server_min_max_radio = handles.ensemble_editor->findChild<QRadioButton*>( "m_server_min_max_radio_button" );
    handles.user_min_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_user_min_double_spin_box" );
    handles.user_max_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_user_max_double_spin_box" );
    handles.server_min_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_server_min_double_spin_box" );
    handles.server_max_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_server_max_double_spin_box" );
    handles.color_map = handles.ensemble_editor->findChild<ColorMap*>( "m_color_map" );
    handles.opacity_map = handles.ensemble_editor->findChild<OpacityMap*>( "m_opacity_map" );

    const auto labels = handles.total_particles_tool_bar->findChildren<QLabel*>();
    for ( QLabel* label : labels )
    {
        if ( label != nullptr && label->text() != QStringLiteral( "Total Particles : " ) )
        {
            handles.total_particles_display = label;
            break;
        }
    }

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_insitu_radio != nullptr, "remoteVizInsituRadioButton not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.keep_last_button != nullptr, "m_keep_last_push_button not found" ) ) { return handles; }
    if ( !require( handles.next_time_step_spin_box != nullptr, "m_next_time_step_spin_box not found" ) ) { return handles; }
    if ( !require( handles.total_particles_display != nullptr, "Total Particles display label not found" ) ) { return handles; }
    if ( !require( handles.ensemble_apply_button != nullptr, "m_apply_push_button not found" ) ) { return handles; }
    if ( !require( handles.variable_line_edit != nullptr, "m_statistics_synthesizer_line_edit not found" ) ) { return handles; }
    if ( !require( handles.statistic_combo_box != nullptr, "m_statistics_combo_box not found" ) ) { return handles; }
    if ( !require( handles.user_min_max_radio != nullptr, "m_user_min_max_radio_button not found" ) ) { return handles; }
    if ( !require( handles.server_min_max_radio != nullptr, "m_server_min_max_radio_button not found" ) ) { return handles; }
    if ( !require( handles.user_min_spin_box != nullptr, "m_user_min_double_spin_box not found" ) ) { return handles; }
    if ( !require( handles.user_max_spin_box != nullptr, "m_user_max_double_spin_box not found" ) ) { return handles; }
    if ( !require( handles.server_min_spin_box != nullptr, "m_server_min_double_spin_box not found" ) ) { return handles; }
    if ( !require( handles.server_max_spin_box != nullptr, "m_server_max_double_spin_box not found" ) ) { return handles; }
    if ( !require( handles.color_map != nullptr, "m_color_map not found" ) ) { return handles; }
    if ( !require( handles.opacity_map != nullptr, "m_opacity_map not found" ) ) { return handles; }
    return handles;
}

void EnsembleInSituModeTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "connectPushButton did not become enabled within the timeout" );
    QTest::mouseClick( client.connect_button, Qt::LeftButton );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.disconnect_button->isEnabled() &&
                       !client.connect_button->isEnabled() &&
                       !client.id_line_edit->text().trimmed().isEmpty();
            },
            k_connect_timeout_ms,
            100 ),
        "Client did not enter the connected state" );
}

void EnsembleInSituModeTest::configureInSituVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2( client.remote_viz_insitu_radio->isEnabled(), "remoteVizInsituRadioButton is disabled" );
    if ( !client.remote_viz_insitu_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_insitu_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_insitu_radio->isChecked(), "remoteVizInsituRadioButton was not checked" );
    QVERIFY2( client.setting_apply_button->isEnabled(), "settingApplyPushButton is disabled" );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
}

// Initialize の EnsembleStatisticsParameter(Data が 3 要素: average / variance / cv)を待つ
bool EnsembleInSituModeTest::waitForInitialStatistics( QSignalSpy& parameter_spy, QStringList* statistics ) const
{
    const QString data_key = QString::fromUtf8( Protocol::Key::Data );
    const QString statistic_key = QString::fromUtf8( Protocol::Key::Statistic );
    int inspected = 0;
    return waitForCondition(
        [&parameter_spy, &inspected, data_key, statistic_key, statistics]()
        {
            while ( inspected < parameter_spy.count() )
            {
                const QList<QVariant> arguments = parameter_spy.at( inspected++ );
                if ( arguments.isEmpty() ) { continue; }
                const QJsonObject payload = qvariant_cast<QJsonObject>( arguments.at( 0 ) );
                const QJsonArray data = payload.value( data_key ).toArray();
                if ( data.size() != 3 ) { continue; }
                QStringList names;
                QSet<QString> normalized;
                for ( const QJsonValue& value : data )
                {
                    const QString name = value.toObject().value( statistic_key ).toString();
                    names << name;
                    normalized.insert( normalizeStatisticName( name ) );
                }
                if ( normalized.contains( QStringLiteral( "average" ) ) &&
                     normalized.contains( QStringLiteral( "variance" ) ) &&
                     normalized.contains( QStringLiteral( "cv" ) ) )
                {
                    if ( statistics ) { *statistics = names; }
                    return true;
                }
            }
            return false;
        },
        k_object_load_timeout_ms,
        100 );
}

// IsEnsemble はサーバが Initialize 時に送る SelectedFile イベント(ObjectEditor::onReceiveSelectedFile)に含まれる。
// ObjectInfoParameter(Apply 時のパッチ配信)には含まれないので、Communication::receiveSelectedFile を監視する。
bool EnsembleInSituModeTest::waitForIsEnsemble( QSignalSpy& selected_file_spy ) const
{
    const QString key = QString::fromUtf8( Protocol::Key::IsEnsemble );
    return waitForCondition(
        [&selected_file_spy, key]()
        {
            for ( int index = 0; index < selected_file_spy.count(); ++index )
            {
                const QList<QVariant> arguments = selected_file_spy.at( index );
                if ( arguments.isEmpty() ) { continue; }
                const QJsonObject payload = qvariant_cast<QJsonObject>( arguments.at( 0 ) );
                if ( payload.value( key ).toBool( false ) ) { return true; }
            }
            return false;
        },
        k_object_load_timeout_ms,
        100 );
}

void EnsembleInSituModeTest::waitForObjectRows( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_tree_view->model() != nullptr &&
                       client.object_tree_view->model()->rowCount() > 0;
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor treeView did not receive an item within the timeout" );
}

void EnsembleInSituModeTest::selectFirstObjectAndApply( const ClientHandles& client ) const
{
    QAbstractItemModel* model = client.object_tree_view->model();
    QVERIFY2( model != nullptr, "ObjectEditor treeView model not found" );
    const QModelIndex first = model->index( 0, 0 );
    QVERIFY2( first.isValid(), "ObjectEditor first row is invalid" );
    client.object_tree_view->scrollTo( first );
    client.object_tree_view->setCurrentIndex( first );
    QVERIFY2( client.object_tree_view->selectionModel() != nullptr, "ObjectEditor selection model not found" );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        first,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    QVERIFY2(
        waitForCondition( [client]() { return client.object_apply_button->isEnabled(); }, k_object_load_timeout_ms, 100 ),
        "ObjectEditor applyPushButton did not become enabled within the timeout" );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

QAction* EnsembleInSituModeTest::findEnsembleEditorAction( MainWindow* window ) const
{
    if ( !window ) { return nullptr; }
    for ( QAction* action : window->findChildren<QAction*>() )
    {
        const QString label = action->text().remove( QLatin1Char( '&' ) );
        if ( label.contains( QStringLiteral( "Ensemble Transfer Function Editor" ) ) ) { return action; }
    }
    return nullptr;
}

void EnsembleInSituModeTest::clickJumpAndWaitForCompletion( const ClientHandles& client, QSignalSpy& completion_spy ) const
{
    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_data_request_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled within the timeout" );
    const int completion_start = completion_spy.count();
    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2(
        waitForCondition(
            [client, &completion_spy, completion_start]()
            {
                return client.jump_button->isEnabled() && completion_spy.count() > completion_start;
            },
            k_data_request_timeout_ms,
            200 ),
        "m_jump_push_button did not become enabled again (dataRequestCompleted not received)" );
}

void EnsembleInSituModeTest::setKeepLast( const ClientHandles& client, bool enabled ) const
{
    QVERIFY2( client.keep_last_button->isEnabled(), "m_keep_last_push_button is disabled" );
    if ( client.keep_last_button->isChecked() != enabled )
    {
        QTest::mouseClick( client.keep_last_button, Qt::LeftButton );
        QTest::qWait( k_window_settle_ms );
    }
    QVERIFY2( client.keep_last_button->isChecked() == enabled, "Keep Last state did not change" );
}

void EnsembleInSituModeTest::waitForPositiveParticleCount( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition( [this, client]() { return totalParticles( client ) > 0; }, k_data_request_timeout_ms, 100 ),
        qPrintable(
            QStringLiteral( "Total Particles did not become positive. current='%1'" )
                .arg( client.total_particles_display->text() ) ) );
}

int EnsembleInSituModeTest::totalParticles( const ClientHandles& client ) const
{
    bool ok = false;
    const int value = client.total_particles_display->text().trimmed().toInt( &ok );
    return ok ? value : -1;
}

int EnsembleInSituModeTest::latestCompletedTimeStep( const Spies& spies ) const
{
    int latest = -1;
    for ( int index = 0; index < spies.completion_spy->count(); ++index )
    {
        const QList<QVariant> arguments = spies.completion_spy->at( index );
        if ( arguments.isEmpty() ) { continue; }
        latest = std::max( latest, arguments.at( 0 ).toInt() );
    }
    return latest;
}

// 次のタイムステップ更新を待つ:
// この呼び出し以降に emit された requestDataAt(受信済みより新しいステップ)を待ち、
// 同じタイムステップの dataRequestCompleted を受信するまで待機する。
bool EnsembleInSituModeTest::waitForNextTimeStep( const Spies& spies, int* time_step ) const
{
    const int request_start = spies.request_spy->count();
    const int completion_start = spies.completion_spy->count();
    const int previous_latest = latestCompletedTimeStep( spies );

    QElapsedTimer timer;
    timer.start();
    int inspected_request = request_start;
    while ( timer.elapsed() < k_data_request_timeout_ms )
    {
        const int remaining = k_data_request_timeout_ms - static_cast<int>( timer.elapsed() );
        if ( !waitForCondition(
                 [spies, inspected_request]() { return spies.request_spy->count() > inspected_request; },
                 remaining,
                 50 ) )
        {
            return false;
        }
        const int requested_step = spies.request_spy->at( inspected_request++ ).at( 0 ).toInt();
        if ( requested_step <= previous_latest ) { continue; }

        const int request_remaining = k_data_request_timeout_ms - static_cast<int>( timer.elapsed() );
        const bool completed = waitForCondition(
            [spies, completion_start, requested_step]()
            {
                for ( int index = completion_start; index < spies.completion_spy->count(); ++index )
                {
                    if ( spies.completion_spy->at( index ).at( 0 ).toInt() == requested_step ) { return true; }
                }
                return false;
            },
            request_remaining,
            50 );
        if ( !completed ) { return false; }
        if ( time_step ) { *time_step = requested_step; }
        return true;
    }
    return false;
}

// requestDataAt が quiet_ms の間 emit されなくなるまで待つ(Keep Last OFF 後の残要求を流す)
bool EnsembleInSituModeTest::waitForQuietRequests( const Spies& spies, int quiet_ms ) const
{
    QElapsedTimer total;
    total.start();
    while ( total.elapsed() < k_data_request_timeout_ms )
    {
        const int request_count = spies.request_spy->count();
        const int completion_count = spies.completion_spy->count();
        QTest::qWait( quiet_ms );
        if ( spies.request_spy->count() == request_count && spies.completion_spy->count() == completion_count )
        {
            return true;
        }
    }
    return false;
}

QByteArray EnsembleInSituModeTest::particleFingerprint( const ClientHandles& client ) const
{
    QCryptographicHash hash( QCryptographicHash::Sha256 );
    QAbstractItemModel* model = client.object_tree_view->model();
    if ( !model ) { return QByteArray(); }

    int point_object_count = 0;
    for ( int row = 0; row < model->rowCount(); ++row )
    {
        const QVariant value = model->index( row, 0 ).data( Qt::UserRole );
        if ( !value.canConvert<ObjectInfoExtractor::ObjectInfo>() ) { continue; }

        const ObjectInfoExtractor::ObjectInfo info = value.value<ObjectInfoExtractor::ObjectInfo>();
        if ( !info.isDisplay || !info.isEnsemble || !info.object ) { continue; }
        if ( info.format != ObjectInfoExtractor::ClientServerPointObject &&
             info.format != ObjectInfoExtractor::InsituServerPointObject )
        {
            continue;
        }

        const auto* point_object = dynamic_cast<const kvs::PointObject*>( info.object );
        if ( !point_object ) { continue; }

        ++point_object_count;
        hash.addData( info.uuid.data(), static_cast<qsizetype>( info.uuid.size() ) );
        const std::size_t vertex_count = point_object->numberOfVertices();
        hash.addData( reinterpret_cast<const char*>( &vertex_count ), sizeof( vertex_count ) );

        const auto add_values = [&hash]( const auto& values )
        {
            if ( values.empty() ) { return; }
            hash.addData(
                reinterpret_cast<const char*>( values.data() ),
                static_cast<qsizetype>( values.byteSize() ) );
        };
        add_values( point_object->coords() );
        add_values( point_object->colors() );
        add_values( point_object->normals() );
        add_values( point_object->sizes() );
    }

    return point_object_count > 0 ? hash.result() : QByteArray();
}

void EnsembleInSituModeTest::selectStatistic( const ClientHandles& client, int index ) const
{
    QVERIFY2( index >= 0 && index < client.statistic_combo_box->count(), "Statistic index is out of range" );
    client.statistic_combo_box->setCurrentIndex( index );
    QCOMPARE( client.statistic_combo_box->currentIndex(), index );
    QCoreApplication::processEvents();
}

EnsembleInSituModeTest::StatisticState
EnsembleInSituModeTest::captureStatisticState( const ClientHandles& client, int index ) const
{
    selectStatistic( client, index );
    StatisticState state;
    state.statistic = client.ensemble_editor->selectedStatistic();
    state.use_user_range = client.user_min_max_radio->isChecked();
    state.user_min = client.user_min_spin_box->value();
    state.user_max = client.user_max_spin_box->value();
    state.server_min = client.server_min_spin_box->value();
    state.server_max = client.server_max_spin_box->value();
    state.colors = client.color_map->getColors();
    state.opacities = client.opacity_map->getOpacities();
    return state;
}

std::array<EnsembleInSituModeTest::StatisticState, 3>
EnsembleInSituModeTest::captureAllStatisticStates( const ClientHandles& client ) const
{
    std::array<StatisticState, 3> states;
    for ( int index = 0; index < 3; ++index )
    {
        states[static_cast<size_t>( index )] = captureStatisticState( client, index );
    }
    return states;
}

bool EnsembleInSituModeTest::statesEqual( const StatisticState& a, const StatisticState& b, QString* why ) const
{
    if ( a.use_user_range != b.use_user_range ) { if ( why ) { *why = QStringLiteral( "レンジモード" ); } return false; }
    if ( std::abs( a.user_min - b.user_min ) > 1.0e-7 ) { if ( why ) { *why = QStringLiteral( "User Min" ); } return false; }
    if ( std::abs( a.user_max - b.user_max ) > 1.0e-7 ) { if ( why ) { *why = QStringLiteral( "User Max" ); } return false; }
    if ( std::abs( a.server_min - b.server_min ) > 1.0e-7 ) { if ( why ) { *why = QStringLiteral( "Server Min" ); } return false; }
    if ( std::abs( a.server_max - b.server_max ) > 1.0e-7 ) { if ( why ) { *why = QStringLiteral( "Server Max" ); } return false; }
    if ( a.colors != b.colors ) { if ( why ) { *why = QStringLiteral( "Color Map" ); } return false; }
    if ( !opacitiesNearlyEqual( a.opacities, b.opacities ) ) { if ( why ) { *why = QStringLiteral( "Opacity Map" ); } return false; }
    return true;
}

bool EnsembleInSituModeTest::waitForStatisticEcho( QSignalSpy& parameter_spy, int start_index, const QString& statistic_name ) const
{
    const QString key = QString::fromUtf8( Protocol::Key::Statistic );
    int inspected = start_index;
    return waitForCondition(
        [&parameter_spy, &inspected, key, statistic_name]()
        {
            while ( inspected < parameter_spy.count() )
            {
                const QList<QVariant> arguments = parameter_spy.at( inspected++ );
                if ( arguments.isEmpty() ) { continue; }
                const QJsonObject payload = qvariant_cast<QJsonObject>( arguments.at( 0 ) );
                if ( normalizeStatisticName( payload.value( key ).toString() ) == statistic_name ) { return true; }
            }
            return false;
        },
        k_echo_observe_ms,
        50 );
}

// m_statistics_combo_box で統計を選択して Apply し、次のタイムステップ更新で粒子が変化するまで待つ
bool EnsembleInSituModeTest::applyStatisticAndWaitForChange(
    ItemResult& result,
    const ClientHandles& client,
    const Spies& spies,
    int statistic_index,
    const QString& statistic_name,
    const QByteArray& previous_fingerprint,
    int* time_step,
    QByteArray* new_fingerprint )
{
    bringEditorToFront( client.ensemble_editor );
    selectStatistic( client, statistic_index );
    const int echo_start = spies.parameter_spy->count();
    QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton );
    markStep( result.id, QStringLiteral( "EnsembleTransferFunctionEditor.ui: m_statistics_combo_box で %1 を選択し m_apply_push_button を押した" ).arg( client.statistic_combo_box->currentText() ) );

    // サーバの再配信(ws->publish)は uWebSockets の仕様で送信元クライアントには届かないため、
    // 受信の有無は判定に使わず参考情報として記録する。サーバ側への反映は次ステップの粒子変化で判定する。
    const bool echoed = waitForStatisticEcho( *spies.parameter_spy, echo_start, statistic_name );
    markStep( result.id, QStringLiteral( "サーバ再配信 EnsembleStatisticsParameter(Statistic=%1)(参考): %2" )
                             .arg( statistic_name, echoed ? QStringLiteral( "受信" ) : QStringLiteral( "未受信(単一クライアントでは送信元に配信されない仕様)" ) ),
              QStringLiteral( "INFO" ) );

    // 統計量の切替がシミュレーション側の粒子生成に反映されるまで数ステップかかることがあるため、
    // 粒子フィンガープリントが変化するまで新しいステップを待ち続ける。
    QElapsedTimer timer;
    timer.start();
    int step = -1;
    QByteArray fingerprint;
    bool changed = false;
    while ( timer.elapsed() < k_data_request_timeout_ms )
    {
        if ( !waitForNextTimeStep( spies, &step ) ) { break; }
        fingerprint = particleFingerprint( client );
        if ( !fingerprint.isEmpty() && fingerprint != previous_fingerprint && totalParticles( client ) > 0 )
        {
            changed = true;
            break;
        }
    }
    if ( time_step ) { *time_step = step; }
    if ( new_fingerprint ) { *new_fingerprint = fingerprint; }
    if ( !check( result, step >= 0, QStringLiteral( "%1 の Apply 後、次のタイムステップ更新が発生しない(シミュレーションが新しいステップを出力しているか確認する)" ).arg( statistic_name ) ) )
    {
        return false;
    }
    markStep( result.id, QStringLiteral( "次のタイムステップ更新(time step %1)を待った" ).arg( step ) );
    check( result, totalParticles( client ) > 0, QStringLiteral( "%1 の Total Particles が正ではない" ).arg( statistic_name ) );
    return check( result, changed, QStringLiteral( "%1 の粒子フィンガープリントが直前の統計量のものから変化しない" ).arg( statistic_name ) );
}

QJsonObject EnsembleInSituModeTest::readJsonObject( const QString& path, bool* ok ) const
{
    if ( ok ) { *ok = false; }
    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) { return QJsonObject(); }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson( file.readAll(), &error );
    if ( error.error != QJsonParseError::NoError || !document.isObject() ) { return QJsonObject(); }
    if ( ok ) { *ok = true; }
    return document.object();
}

bool EnsembleInSituModeTest::writeJsonObject( const QString& path, const QJsonObject& root ) const
{
    QFile file( path );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) ) { return false; }
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Indented ) );
    return true;
}

QString EnsembleInSituModeTest::stepJsonPath( int time_step ) const
{
    if ( m_vis_param_dir.isEmpty() ) { return QString(); }
    const QString prefix = m_tf_name.isEmpty() ? QStringLiteral( "default" ) : m_tf_name;
    return QDir( m_vis_param_dir ).absoluteFilePath(
        QStringLiteral( "%1_%2.json" ).arg( prefix ).arg( time_step, 5, 10, QLatin1Char( '0' ) ) );
}

// S-IS-33: サーバ側ファイルの確認(ENSEMBLE_VIS_PARAM_DIR / ENSEMBLE_PARTICLE_DIR が設定されている場合)
void EnsembleInSituModeTest::checkServerSideFiles( ItemResult& result )
{
    if ( m_vis_param_dir.isEmpty() )
    {
        markStep( result.id, QStringLiteral( "history_XXXXX.json / state.txt の確認は ENSEMBLE_VIS_PARAM_DIR 未設定のため SKIP" ), QStringLiteral( "SKIP" ) );
        result.messages.push_back( QStringLiteral( "サーバ側ファイル(VIS_PARAM_DIR)の確認: SKIP" ) );
    }
    else
    {
        QDir dir( m_vis_param_dir );
        const QStringList histories = dir.entryList( QStringList() << QStringLiteral( "history_*.json" ), QDir::Files, QDir::Name );
        if ( check( result, !histories.isEmpty(), QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR に history_XXXXX.json が存在しない: %1" ).arg( m_vis_param_dir ) ) )
        {
            const QString history_path = dir.absoluteFilePath( histories.last() );
            m_checked_files << QStringLiteral( "VIS_PARAM_DIR: %1(history ファイル数 %2、直近を確認)" ).arg( histories.last() ).arg( histories.size() );
            bool ok = false;
            const QJsonObject history = readJsonObject( history_path, &ok );
            if ( check( result, ok, QStringLiteral( "history json を読めない: %1" ).arg( history_path ) ) )
            {
                check( result, history.value( QStringLiteral( "is_ensemble" ) ).toBool( false ), QStringLiteral( "%1 の is_ensemble が true ではない" ).arg( histories.last() ) );
                const QJsonObject statistics = history.value( QStringLiteral( "statistics" ) ).toObject();
                const std::array<QString, 3> keys = {
                    QStringLiteral( "average" ), QStringLiteral( "variance" ), QStringLiteral( "coefficient_of_variation" ) };
                for ( const QString& key : keys )
                {
                    if ( !check( result, statistics.value( key ).isObject(), QStringLiteral( "%1 に statistics.%2 が無い" ).arg( histories.last(), key ) ) ) { continue; }
                    const QJsonObject statistic = statistics.value( key ).toObject();
                    for ( const QString& tf_key : { QStringLiteral( "O1" ), QStringLiteral( "C1" ) } )
                    {
                        if ( !check( result, statistic.value( tf_key ).isObject(), QStringLiteral( "statistics.%1.%2 が無い" ).arg( key, tf_key ) ) ) { continue; }
                        const QJsonObject entry = statistic.value( tf_key ).toObject();
                        check( result, entry.value( QStringLiteral( "min" ) ).isDouble(), QStringLiteral( "statistics.%1.%2.min が無い" ).arg( key, tf_key ) );
                        check( result, entry.value( QStringLiteral( "max" ) ).isDouble(), QStringLiteral( "statistics.%1.%2.max が無い" ).arg( key, tf_key ) );
                        check( result, entry.value( QStringLiteral( "resolution" ) ).toInt( -1 ) == k_histogram_resolution,
                               QStringLiteral( "statistics.%1.%2.resolution が 256 ではない: %3" ).arg( key, tf_key ).arg( entry.value( QStringLiteral( "resolution" ) ).toInt( -1 ) ) );
                        check( result, entry.value( QStringLiteral( "histogram" ) ).toArray().size() == k_histogram_resolution,
                               QStringLiteral( "statistics.%1.%2.histogram の長さが 256 ではない: %3" ).arg( key, tf_key ).arg( entry.value( QStringLiteral( "histogram" ) ).toArray().size() ) );
                    }
                }
                markStep( result.id, QStringLiteral( "%1 の is_ensemble / statistics(average, variance, coefficient_of_variation の O1・C1 min/max/resolution/histogram)を確認した" ).arg( histories.last() ) );
            }
        }
        const bool state_exists = dir.exists( QStringLiteral( "state.txt" ) );
        check( result, state_exists, QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR に state.txt が存在しない" ) );
        if ( state_exists )
        {
            m_checked_files << QStringLiteral( "VIS_PARAM_DIR: state.txt" );
            markStep( result.id, QStringLiteral( "state.txt の存在を確認した" ) );
        }
    }

    if ( m_particle_dir.isEmpty() )
    {
        markStep( result.id, QStringLiteral( "ave_ / var_ / cov_ 粒子ファイルの確認は ENSEMBLE_PARTICLE_DIR 未設定のため SKIP" ), QStringLiteral( "SKIP" ) );
        result.messages.push_back( QStringLiteral( "サーバ側ファイル(PARTICLE_DIR)の確認: SKIP" ) );
        return;
    }

    QDir particle_dir( m_particle_dir );
    for ( const QString& prefix : { QStringLiteral( "ave_" ), QStringLiteral( "var_" ), QStringLiteral( "cov_" ) } )
    {
        const QStringList files = particle_dir.entryList( QStringList() << prefix + QStringLiteral( "*" ), QDir::Files, QDir::Name );
        if ( check( result, !files.isEmpty(), QStringLiteral( "ENSEMBLE_PARTICLE_DIR に %1* で始まる粒子ファイルが無い: %2" ).arg( prefix, m_particle_dir ) ) )
        {
            const int shown = std::min( 5, static_cast<int>( files.size() ) );
            QStringList sample;
            for ( int i = 0; i < shown; ++i ) { sample << files.at( i ); }
            m_checked_files << QStringLiteral( "PARTICLE_DIR: %1*(%2 ファイル)例: %3%4" )
                                   .arg( prefix ).arg( files.size() ).arg( sample.join( QStringLiteral( ", " ) ) )
                                   .arg( files.size() > shown ? QStringLiteral( ", ..." ) : QString() );
        }
    }
    const bool minmax_exists = particle_dir.exists( QStringLiteral( "t_pfi_coords_minmax.txt" ) );
    check( result, minmax_exists, QStringLiteral( "ENSEMBLE_PARTICLE_DIR に t_pfi_coords_minmax.txt が存在しない" ) );
    if ( minmax_exists ) { m_checked_files << QStringLiteral( "PARTICLE_DIR: t_pfi_coords_minmax.txt" ); }
    markStep( result.id, QStringLiteral( "ENSEMBLE_PARTICLE_DIR の ave_ / var_ / cov_ 粒子ファイルと t_pfi_coords_minmax.txt を確認した" ) );
}

// ===========================================================================
// Part A: S-IS-33 アンサンブルシミュレーションの動作確認
// ===========================================================================
void EnsembleInSituModeTest::runPartA( ItemResult& result, const ClientHandles& client, const Spies& spies, MainWindow& window )
{
    // ---- Communication.ui ----
    connectClient( client );
    markStep( result.id, QStringLiteral( "Communication.ui: connectPushButton を押し、接続状態になった" ) );
    configureInSituVisualization( client );
    markStep( result.id, QStringLiteral( "Communication.ui: remoteVizInsituRadioButton を選択し settingApplyPushButton を押した" ) );

    const bool received = waitForInitialStatistics( *spies.parameter_spy, &m_initial_statistics );
    QVERIFY2( received, "Initialize EnsembleStatisticsParameter with 3 Data elements (average / variance / cv) was not received" );
    markStep( result.id, QStringLiteral( "Communication::receiveEnsembleStatisticsParameter(Initialize)を受信し、Data が 3 要素(%1)であることを確認した" ).arg( m_initial_statistics.join( QStringLiteral( " / " ) ) ) );

    // ---- ObjectEditor.ui ----
    waitForObjectRows( client );
    markStep( result.id, QStringLiteral( "ObjectEditor.ui: treeView に行が追加された" ) );
    if ( check( result, waitForIsEnsemble( *spies.selected_file_spy ), QStringLiteral( "Communication::receiveSelectedFile(Initialize の ObjectInfo)の IsEnsemble が true ではない" ) ) )
    {
        markStep( result.id, QStringLiteral( "Communication::receiveSelectedFile(Initialize の ObjectInfo)の IsEnsemble が true であることを確認した" ) );
    }
    selectFirstObjectAndApply( client );
    markStep( result.id, QStringLiteral( "ObjectEditor.ui: 先頭行を選択し applyPushButton を押した" ) );

    // ---- MainWindow: QAction「Ensemble Transfer Function Editor」 ----
    QAction* ensemble_action = findEnsembleEditorAction( &window );
    if ( check( result, ensemble_action != nullptr, QStringLiteral( "QAction「Ensemble Transfer Function Editor」が見つからない" ) ) )
    {
        const bool enabled = waitForCondition( [ensemble_action]() { return ensemble_action->isEnabled(); }, k_object_load_timeout_ms, 100 );
        if ( check( result, enabled, QStringLiteral( "QAction「Ensemble Transfer Function Editor」が有効にならない" ) ) )
        {
            markStep( result.id, QStringLiteral( "MainWindow: QAction「Ensemble Transfer Function Editor」が有効になった" ) );
        }
    }
    bringWindowToFront( client.main_window );
    saveScreenshot(
        result, 1, QStringLiteral( "menu_enabled" ),
        QStringLiteral( "ens_Hydrogen_unstruct, IsEnsemble=true, Ensemble Transfer Function Editor 有効" ),
        QStringLiteral( "Server/ISPBVR/33_1.png" ), -1 );

    // ---- PlayBackControlToolBar ----
    clickJumpAndWaitForCompletion( client, *spies.completion_spy );
    markStep( result.id, QStringLiteral( "PlayBackControlToolBar: m_jump_push_button を押し、再度有効になるまで待機した" ) );
    setKeepLast( client, true );
    markStep( result.id, QStringLiteral( "PlayBackControlToolBar: m_keep_last_push_button を ON にした" ) );
    waitForPositiveParticleCount( client );
    const int average_step = latestCompletedTimeStep( spies );
    markStep( result.id, QStringLiteral( "TotalParticlesToolBar の粒子数が正になった(%1、time step %2)" ).arg( totalParticles( client ) ).arg( average_step ) );

    bringWindowToFront( client.main_window );
    saveScreenshot(
        result, 2, QStringLiteral( "average_particles" ),
        QStringLiteral( "ens_Hydrogen_unstruct, Average, time step:%1" ).arg( average_step ),
        QStringLiteral( "Server/ISPBVR/33_2.png" ), average_step );
    const QByteArray average_fingerprint = particleFingerprint( client );
    check( result, !average_fingerprint.isEmpty(), QStringLiteral( "Average の粒子(アンサンブル PointObject)が ObjectEditor に見つからない" ) );

    // ---- EnsembleTransferFunctionEditor.ui: Variance ----
    int variance_step = -1;
    QByteArray variance_fingerprint;
    const bool variance_ok = applyStatisticAndWaitForChange(
        result, client, spies, 1, QStringLiteral( "variance" ), average_fingerprint, &variance_step, &variance_fingerprint );
    if ( variance_ok )
    {
        markStep( result.id, QStringLiteral( "Variance: Total Particles が正で、粒子の指紋が Average のときと異なることを確認した(time step %1)" ).arg( variance_step ) );
    }
    bringWindowToFront( client.main_window );
    saveScreenshot(
        result, 3, QStringLiteral( "variance_particles" ),
        QStringLiteral( "ens_Hydrogen_unstruct, Variance, time step:%1" ).arg( variance_step ),
        QStringLiteral( "Server/ISPBVR/33_3.png" ), variance_step );

    // ---- EnsembleTransferFunctionEditor.ui: Coefficient of Variation ----
    int cv_step = -1;
    QByteArray cv_fingerprint;
    const bool cv_ok = applyStatisticAndWaitForChange(
        result, client, spies, 2, QStringLiteral( "cv" ),
        variance_fingerprint.isEmpty() ? average_fingerprint : variance_fingerprint, &cv_step, &cv_fingerprint );
    if ( cv_ok )
    {
        markStep( result.id, QStringLiteral( "Coefficient of Variation: Total Particles が正で、粒子の指紋が Variance のときと異なることを確認した(time step %1)" ).arg( cv_step ) );
    }
    bringWindowToFront( client.main_window );
    saveScreenshot(
        result, 4, QStringLiteral( "cv_particles" ),
        QStringLiteral( "ens_Hydrogen_unstruct, Coefficient of Variation, time step:%1" ).arg( cv_step ),
        QStringLiteral( "Server/ISPBVR/33_4.png" ), cv_step );

    // ---- 元に戻す: Average ----
    bringEditorToFront( client.ensemble_editor );
    selectStatistic( client, 0 );
    {
        const int echo_start = spies.parameter_spy->count();
        QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton );
        const bool echoed = waitForStatisticEcho( *spies.parameter_spy, echo_start, QStringLiteral( "average" ) );
        markStep( result.id, QStringLiteral( "サーバ再配信 EnsembleStatisticsParameter(Statistic=average)(参考): %1" )
                                 .arg( echoed ? QStringLiteral( "受信" ) : QStringLiteral( "未受信(単一クライアントでは送信元に配信されない仕様)" ) ),
                  QStringLiteral( "INFO" ) );
        markStep( result.id, QStringLiteral( "EnsembleTransferFunctionEditor.ui: m_statistics_combo_box で Average を選択し m_apply_push_button を押した(元に戻す)" ) );
    }

    // ---- サーバ側ファイルの確認 ----
    checkServerSideFiles( result );
}

// ===========================================================================
// Part C: S-IS-35 default_XXXXX.json にアンサンブルセクションが無いステップの動作確認
// ===========================================================================
void EnsembleInSituModeTest::runPartC( ItemResult& result, const ClientHandles& client, const Spies& spies )
{
    // S-IS-35 は「default_XXXXX.json にアンサンブル伝達関数セクションが無い」という、正常運用では発生しない状態を
    // テスト側で人工的に作って検証する防御分岐の確認(参考項目)。サーバの JSON 書き出しは常に 3 セクションを出力し、
    // アンサンブルサンプルは全ステップでそれらを保持するため、通常のアンサンブル実行では到達しない。
    // コスト(サーバ側 default_XXXXX.json への書き込みアクセス)に見合わないため、既定では自動実行しない。
    // 実行する場合は ENSEMBLE_RUN_PART_C=1(環境変数または TestPathConfig.ini)を設定する。
    const QString run_part_c = configuredValue( "ENSEMBLE_RUN_PART_C" ).trimmed().toLower();
    const bool part_c_enabled = ( run_part_c == QStringLiteral( "1" ) || run_part_c == QStringLiteral( "true" ) || run_part_c == QStringLiteral( "yes" ) );
    if ( !part_c_enabled )
    {
        skipItem( result, QStringLiteral( "S-IS-35 は正常運用では発生しない防御分岐の参考項目のため、既定では自動実行しない(実行するには ENSEMBLE_RUN_PART_C=1 を設定)" ) );
        return;
    }

    if ( m_vis_param_dir.isEmpty() )
    {
        skipItem( result, QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR が未設定(または存在しない)のため Part C 全体を SKIP" ) );
        return;
    }
    if ( !QFileInfo( m_vis_param_dir ).isWritable() )
    {
        skipItem( result, QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR に書き込めないため Part C 全体を SKIP: %1" ).arg( m_vis_param_dir ) );
        return;
    }

    // ---- PlayBackControlToolBar: Keep Last OFF → 残要求を流す ----
    // (自動更新でステップが進まないようにし、Server MinMax 等が記録後に更新されないようにするため先に OFF にする)
    bringWindowToFront( client.main_window );
    setKeepLast( client, false );
    markStep( result.id, QStringLiteral( "PlayBackControlToolBar: m_keep_last_push_button を OFF にした" ) );
    waitForQuietRequests( spies, k_quiet_request_ms );

    // ---- EnsembleTransferFunctionEditor.ui: GUI 値と Variable を記録 ----
    bringEditorToFront( client.ensemble_editor );
    const std::array<StatisticState, 3> baseline_states = captureAllStatisticStates( client );
    const QString baseline_variable = client.variable_line_edit->text().trimmed();
    selectStatistic( client, 0 );
    markStep( result.id, QStringLiteral( "EnsembleTransferFunctionEditor.ui: 3統計の GUI 値(Color Map / Opacity Map / User MinMax / レンジモード / Server MinMax)と Variable(%1)を記録した" ).arg( baseline_variable ) );

    // ---- 対象ステップ N の選択: Part A で受信済みで最新より前、かつ default_0000N.json が存在するもの ----
    const int latest = latestCompletedTimeStep( spies );
    QSet<int> received_steps;
    for ( int index = 0; index < spies.completion_spy->count(); ++index )
    {
        const QList<QVariant> arguments = spies.completion_spy->at( index );
        if ( !arguments.isEmpty() ) { received_steps.insert( arguments.at( 0 ).toInt() ); }
    }
    int target_step = -1;
    for ( int step : received_steps )
    {
        if ( step >= latest ) { continue; }
        if ( !QFileInfo::exists( stepJsonPath( step ) ) ) { continue; }
        target_step = std::max( target_step, step );
    }
    if ( target_step < 0 )
    {
        skipItem( result, QStringLiteral( "受信済みで最新(%1)より前のステップの %2_0000N.json が ENSEMBLE_VIS_PARAM_DIR に見つからないため SKIP(シミュレーションのステップ数を確認する)" )
                              .arg( latest ).arg( m_tf_name.isEmpty() ? QStringLiteral( "default" ) : m_tf_name ) );
        return;
    }
    m_part_c_time_step = target_step;
    m_part_c_target_path = stepJsonPath( target_step );
    m_part_c_backup_path = QDir( m_output_dir_path ).absoluteFilePath( QFileInfo( m_part_c_target_path ).fileName() + QStringLiteral( ".bak" ) );
    markStep( result.id, QStringLiteral( "対象ステップ N = %1(%2)を選択した" ).arg( target_step ).arg( QFileInfo( m_part_c_target_path ).fileName() ) );

    // ---- サーバ側ファイルの編集 ----
    if ( QFileInfo::exists( m_part_c_backup_path ) ) { QFile::remove( m_part_c_backup_path ); }
    if ( !check( result, QFile::copy( m_part_c_target_path, m_part_c_backup_path ), QStringLiteral( "バックアップの作成に失敗: %1" ).arg( m_part_c_backup_path ) ) )
    {
        skipItem( result, QStringLiteral( "バックアップを作成できないため Part C を中止" ) );
        return;
    }
    bool ok = false;
    QJsonObject step_json = readJsonObject( m_part_c_target_path, &ok );
    if ( !check( result, ok, QStringLiteral( "%1 を JSON として読めない" ).arg( m_part_c_target_path ) ) )
    {
        return;
    }
    const std::array<QString, 3> ensemble_keys = {
        QStringLiteral( "mean_transfer_functions" ),
        QStringLiteral( "variance_transfer_functions" ),
        QStringLiteral( "coefficient_of_variation_transfer_functions" ) };
    int removed = 0;
    for ( const QString& key : ensemble_keys )
    {
        if ( step_json.contains( key ) ) { step_json.remove( key ); ++removed; }
    }
    check( result, removed > 0, QStringLiteral( "%1 にアンサンブルセクション(mean_/variance_/coefficient_of_variation_transfer_functions)が元々存在しない" ).arg( QFileInfo( m_part_c_target_path ).fileName() ) );
    check( result, step_json.contains( QStringLiteral( "settings" ) ) && step_json.contains( QStringLiteral( "transfer_functions" ) ),
           QStringLiteral( "%1 に settings / transfer_functions が無い" ).arg( QFileInfo( m_part_c_target_path ).fileName() ) );
    if ( !check( result, writeJsonObject( m_part_c_target_path, step_json ), QStringLiteral( "%1 の上書き保存に失敗" ).arg( m_part_c_target_path ) ) )
    {
        return;
    }
    markStep( result.id, QStringLiteral( "%1 をバックアップし、mean_ / variance_ / coefficient_of_variation_transfer_functions の 3 キーを削除して上書き保存した(settings と transfer_functions は残す)" ).arg( QFileInfo( m_part_c_target_path ).fileName() ) );

    // ---- TimeStepControlToolBar: m_next_time_step_spin_box = N ----
    bringWindowToFront( client.main_window );
    if ( !check( result,
                 target_step >= client.next_time_step_spin_box->minimum() && target_step <= client.next_time_step_spin_box->maximum(),
                 QStringLiteral( "N = %1 が m_next_time_step_spin_box の範囲(%2〜%3)外" ).arg( target_step ).arg( client.next_time_step_spin_box->minimum() ).arg( client.next_time_step_spin_box->maximum() ) ) )
    {
        return;
    }
    client.next_time_step_spin_box->setFocus();
    client.next_time_step_spin_box->setValue( target_step );
    QCoreApplication::processEvents();
    check( result, client.next_time_step_spin_box->value() == target_step, QStringLiteral( "m_next_time_step_spin_box に %1 を設定できない" ).arg( target_step ) );
    markStep( result.id, QStringLiteral( "TimeStepControlToolBar: m_next_time_step_spin_box を %1 に設定した" ).arg( target_step ) );

    // ---- Jump → dataRequestCompleted(N) ----
    const bool jump_ready = waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_data_request_timeout_ms, 200 );
    if ( !check( result, jump_ready, QStringLiteral( "m_jump_push_button が有効にならない" ) ) ) { return; }
    const int ensemble_param_start = spies.request_parameter_spy->count();
    const int tf_param_start = spies.request_tf_spy->count();
    const int completion_start = spies.completion_spy->count();
    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    markStep( result.id, QStringLiteral( "PlayBackControlToolBar: m_jump_push_button を押した" ) );
    const bool completed = waitForCondition(
        [spies, completion_start, target_step]()
        {
            for ( int index = completion_start; index < spies.completion_spy->count(); ++index )
            {
                if ( spies.completion_spy->at( index ).at( 0 ).toInt() == target_step ) { return true; }
            }
            return false;
        },
        k_data_request_timeout_ms,
        100 );
    if ( check( result, completed, QStringLiteral( "TimeStepControlToolBar::dataRequestCompleted(%1)を受信しない" ).arg( target_step ) ) )
    {
        markStep( result.id, QStringLiteral( "TimeStepControlToolBar::dataRequestCompleted(%1)を受信した" ).arg( target_step ) );
    }
    QTest::qWait( k_short_wait_ms );

    // ---- 判定 ----
    const int ensemble_param_count = spies.request_parameter_spy->count() - ensemble_param_start;
    const int tf_param_count = spies.request_tf_spy->count() - tf_param_start;
    if ( check( result, ensemble_param_count == 0, QStringLiteral( "待機中に Communication::receiveRequestDataAtEnsembleStatisticsParameter が emit された(%1 回。RequestDataAt 応答に EnsembleTransferFunctionParameter が含まれている)" ).arg( ensemble_param_count ) ) )
    {
        markStep( result.id, QStringLiteral( "Communication::receiveRequestDataAtEnsembleStatisticsParameter が emit されないことを確認した" ) );
    }
    if ( check( result, tf_param_count > 0, QStringLiteral( "Communication::receiveRequestDataAtTransferFunctionParameter が emit されない(TransferFunctionParameter が含まれない)" ) ) )
    {
        markStep( result.id, QStringLiteral( "Communication::receiveRequestDataAtTransferFunctionParameter が emit されることを確認した(%1 回)" ).arg( tf_param_count ) );
    }
    const int particles = totalParticles( client );
    if ( check( result, particles > 0, QStringLiteral( "TotalParticlesToolBar の粒子数が正ではない: %1" ).arg( client.total_particles_display->text() ) ) )
    {
        markStep( result.id, QStringLiteral( "TotalParticlesToolBar の粒子数が正であることを確認した(%1)" ).arg( particles ) );
    }

    bringEditorToFront( client.ensemble_editor );
    bool editor_unchanged = client.variable_line_edit->text().trimmed() == baseline_variable;
    check( result, editor_unchanged, QStringLiteral( "Variable が編集前(%1)と異なる: %2" ).arg( baseline_variable, client.variable_line_edit->text() ) );
    for ( int index = 0; index < 3; ++index )
    {
        const StatisticState current = captureStatisticState( client, index );
        QString why;
        if ( !statesEqual( current, baseline_states[static_cast<size_t>( index )], &why ) )
        {
            editor_unchanged = false;
            check( result, false, QStringLiteral( "%1 の GUI 値が編集前と異なる(%2)" ).arg( current.statistic, why ) );
        }
    }
    selectStatistic( client, 0 );
    if ( editor_unchanged )
    {
        markStep( result.id, QStringLiteral( "EnsembleTransferFunctionEditor の 3統計の GUI 値と Variable が編集前の記録と同一であることを確認した" ) );
    }

    // ---- pbvr_server 標準エラー出力の確認(PBVR_SERVER_STDERR_LOG 設定時) ----
    if ( m_server_stderr_log.isEmpty() || !QFileInfo::exists( m_server_stderr_log ) )
    {
        markStep( result.id, QStringLiteral( "pbvr_server 標準エラー出力の確認は PBVR_SERVER_STDERR_LOG 未設定(または存在しない)のため SKIP" ), QStringLiteral( "SKIP" ) );
        result.messages.push_back( QStringLiteral( "サーバ警告行の確認: SKIP" ) );
    }
    else
    {
        QFile log( m_server_stderr_log );
        bool found = false;
        if ( log.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
            QTextStream in( &log );
            const QString expected_prefix = QStringLiteral( "[Server][RequestDataAt][EnsembleTF][WARN]" );
            const QString expected_step = QStringLiteral( "timestep=%1" ).arg( target_step );
            const QString expected_reason = QStringLiteral( "sections are missing or empty" );
            while ( !in.atEnd() )
            {
                const QString line = in.readLine();
                if ( line.contains( expected_prefix ) && line.contains( expected_step ) && line.contains( expected_reason ) )
                {
                    found = true;
                    break;
                }
            }
        }
        if ( check( result, found, QStringLiteral( "%1 に「[Server][RequestDataAt][EnsembleTF][WARN] timestep=%2 ... sections are missing or empty」の行が無い" ).arg( m_server_stderr_log ).arg( target_step ) ) )
        {
            markStep( result.id, QStringLiteral( "pbvr_server 標準エラー出力に timestep=%1 の [EnsembleTF][WARN] 行があることを確認した" ).arg( target_step ) );
        }
    }

    // ---- スクリーンショット ----
    bringWindowToFront( client.main_window );
    saveScreenshot(
        result, 1, QStringLiteral( "missing_sections_particles" ),
        QStringLiteral( "S-IS-35 %1 アンサンブルセクション無し, time step:%2" ).arg( QFileInfo( m_part_c_target_path ).fileName() ).arg( target_step ),
        QStringLiteral( "Server/ISPBVR/35_1.png" ), target_step );
    bringEditorToFront( client.ensemble_editor );
    saveScreenshot(
        result, 2, QStringLiteral( "missing_sections_editor" ),
        QStringLiteral( "S-IS-35 %1 アンサンブルセクション無し, time step:%2(エディタ、変化なし)" ).arg( QFileInfo( m_part_c_target_path ).fileName() ).arg( target_step ),
        QStringLiteral( "Server/ISPBVR/35_2.png" ), target_step );
}

void EnsembleInSituModeTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_output_dir_path = datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "EnsembleInSituModeTest" ) );
    m_screenshot_dir_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) );
    m_manual_dir_path = QDir( m_screenshot_dir_path ).absoluteFilePath( QStringLiteral( "manual" ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_vis_param_dir = configuredPathOrEmpty( "ENSEMBLE_VIS_PARAM_DIR" );
    m_particle_dir = configuredPathOrEmpty( "ENSEMBLE_PARTICLE_DIR" );
    m_server_stderr_log = configuredPathOrEmpty( "PBVR_SERVER_STDERR_LOG" );
    m_tf_name = configuredValue( "TF_NAME" );
    if ( !m_vis_param_dir.isEmpty() && !QDir( m_vis_param_dir ).exists() )
    {
        qWarning().noquote() << QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR does not exist; server-side checks are skipped: %1" ).arg( m_vis_param_dir );
        m_vis_param_dir.clear();
    }
    if ( !m_particle_dir.isEmpty() && !QDir( m_particle_dir ).exists() )
    {
        qWarning().noquote() << QStringLiteral( "ENSEMBLE_PARTICLE_DIR does not exist; particle file checks are skipped: %1" ).arg( m_particle_dir );
        m_particle_dir.clear();
    }
    m_test_succeeded = false;

    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_manual_dir_path ), qPrintable( QStringLiteral( "Failed to create manual directory: %1" ).arg( m_manual_dir_path ) ) );

    QFile report( m_report_path );
    QVERIFY2( report.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ), "Failed to initialize TestResult.md" );
    QTextStream stream( &report );
    stream << "# EnsembleInSituModeTest\n\n- 結果: RUNNING\n";
}

void EnsembleInSituModeTest::cleanupTestCase()
{
    // Part C で編集したファイルが残っていれば元に戻す(テスト途中で失敗した場合の保険)
    if ( !m_part_c_backup_path.isEmpty() && QFileInfo::exists( m_part_c_backup_path ) && !m_part_c_target_path.isEmpty() )
    {
        if ( QFileInfo::exists( m_part_c_target_path ) ) { QFile::remove( m_part_c_target_path ); }
        if ( !QFile::copy( m_part_c_backup_path, m_part_c_target_path ) )
        {
            qWarning().noquote() << QStringLiteral( "Failed to restore %1 from %2" ).arg( m_part_c_target_path, m_part_c_backup_path );
        }
    }
    writeMarkdownReport();
}

void EnsembleInSituModeTest::performs_ensemble_in_situ_mode_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    const ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();

    QSignalSpy request_spy( client.time_step_tool_bar, &TimeStepControlToolBar::requestDataAt );
    QSignalSpy completion_spy( client.time_step_tool_bar, &TimeStepControlToolBar::dataRequestCompleted );
    QSignalSpy parameter_spy( client.communication, &Communication::receiveEnsembleStatisticsParameter );
    QSignalSpy request_parameter_spy( client.communication, &Communication::receiveRequestDataAtEnsembleStatisticsParameter );
    QSignalSpy request_tf_spy( client.communication, &Communication::receiveRequestDataAtTransferFunctionParameter );
    QSignalSpy selected_file_spy( client.communication, &Communication::receiveSelectedFile );
    QVERIFY2( request_spy.isValid(), "requestDataAt QSignalSpy is invalid" );
    QVERIFY2( completion_spy.isValid(), "dataRequestCompleted QSignalSpy is invalid" );
    QVERIFY2( parameter_spy.isValid(), "EnsembleStatisticsParameter QSignalSpy is invalid" );
    QVERIFY2( request_parameter_spy.isValid(), "RequestDataAt EnsembleStatisticsParameter QSignalSpy is invalid" );
    QVERIFY2( request_tf_spy.isValid(), "RequestDataAt TransferFunctionParameter QSignalSpy is invalid" );
    QVERIFY2( selected_file_spy.isValid(), "SelectedFile QSignalSpy is invalid" );
    Spies spies;
    spies.request_spy = &request_spy;
    spies.completion_spy = &completion_spy;
    spies.parameter_spy = &parameter_spy;
    spies.request_parameter_spy = &request_parameter_spy;
    spies.request_tf_spy = &request_tf_spy;
    spies.selected_file_spy = &selected_file_spy;

    // ---- Part A ----
    ItemResult part_a;
    part_a.id = QString::fromLatin1( k_item_a );
    part_a.title = QStringLiteral( "アンサンブルシミュレーションの動作確認" );
    runPartA( part_a, client, spies, main_window );
    finishItem( part_a );

    // ---- Part C(同じセッションで続けて実行) ----
    ItemResult part_c;
    part_c.id = QString::fromLatin1( k_item_c );
    part_c.title = QStringLiteral( "default_XXXXX.json にアンサンブルセクションが無いステップの動作確認(参考・正常運用では発生しない防御分岐。既定で自動実行しない)" );
    runPartC( part_c, client, spies );

    // ---- 後片付け ----
    if ( !m_part_c_backup_path.isEmpty() && QFileInfo::exists( m_part_c_backup_path ) )
    {
        if ( QFileInfo::exists( m_part_c_target_path ) ) { QFile::remove( m_part_c_target_path ); }
        const bool restored = QFile::copy( m_part_c_backup_path, m_part_c_target_path );
        if ( check( part_c, restored, QStringLiteral( "%1 を %2 から元に戻せない" ).arg( m_part_c_target_path, m_part_c_backup_path ) ) )
        {
            markStep( part_c.id, QStringLiteral( "%1 を %2 に戻した" ).arg( QFileInfo( m_part_c_backup_path ).fileName(), QFileInfo( m_part_c_target_path ).fileName() ) );
        }
    }
    bringWindowToFront( client.main_window );
    if ( client.keep_last_button->isEnabled() && !client.keep_last_button->isChecked() )
    {
        QTest::mouseClick( client.keep_last_button, Qt::LeftButton );
        QTest::qWait( k_window_settle_ms );
        markStep( part_c.id, QStringLiteral( "PlayBackControlToolBar: m_keep_last_push_button を ON に戻した" ) );
    }
    finishItem( part_c );

    // ---- Communication.ui: 切断 ----
    bringWindowToFront( client.main_window );
    QTest::mouseClick( client.disconnect_button, Qt::LeftButton );
    const bool disconnected = waitForCondition(
        [client]() { return client.connect_button->isEnabled() && !client.disconnect_button->isEnabled(); },
        k_connect_timeout_ms,
        100 );
    markStep( QStringLiteral( "S-IS-33/35" ), QStringLiteral( "Communication.ui: disconnectPushButton を押した" ), disconnected ? QStringLiteral( "PASS" ) : QStringLiteral( "FAIL" ) );

    bool any_failed = !disconnected;
    for ( const ItemResult& item : m_item_results )
    {
        if ( item.status == QStringLiteral( "FAIL" ) ) { any_failed = true; }
    }
    m_test_succeeded = !any_failed;

    main_window.close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

    QVERIFY2( !any_failed, "One or more check-sheet items (S-IS-33 / S-IS-35) failed. See TestResult.md" );
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::EnsembleInSituModeTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
