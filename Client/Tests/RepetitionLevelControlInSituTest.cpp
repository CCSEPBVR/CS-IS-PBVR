#include "RepetitionLevelControlInSituTest.h"
#include <algorithm>

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QItemSelectionModel>
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
#include <QSettings>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTest>
#include <QTextStream>
#include <QTreeView>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/RepetitionLevelControl.h"
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
constexpr int k_default_json_retry_ms = 5000;
// Apply 後に最初に通知された最新ステップ M' に対し、記録対象とするステップのしきい値は M' + k_generation_margin_steps。
// InSituLib は各ステップの冒頭で default.json を読み、粒子出力後に state.txt の LATEST_STEP を更新するため、
// Apply 時点で生成中だったステップは高々 M'+1 であり、M'+2 以降は Apply 後の Repetition Level で生成されたことが保証される。
constexpr int k_generation_margin_steps = 2;
constexpr int k_threshold_wait_timeout_ms = 300000; // しきい値ステップの受信を待つ上限(受信回数は計算速度により変わる)
// 粒子数比の期待比(Level の比)に対する許容率。倍化はちょうど期待比付近になるが、サンプリング誤差で数%下振れするため。
constexpr double k_ratio_tolerance = 0.15;
// 記録対象の決定方法: Apply 後、粒子数が前ケース(C_prev)から k_particle_change_ratio 超変化し、
// かつ次のステップでも k_particle_stable_ratio 以内で安定したステップを「反映済み」とみなして記録する。
// Level 変化は粒子数が 2 倍以上動く(100% 以上)のに対し、同一 Level のステップ間変動は小さい(〜1%未満)ため区別できる。
// これにより、M'+2 の固定マージンでは拾ってしまう「Apply 後も古い Level で生成された在庫ステップ」を除外する。
constexpr double k_particle_change_ratio = 0.25; // C_prev からの相対変化の下限
constexpr double k_particle_stable_ratio = 0.10; // 連続 2 ステップの相対差の上限(安定とみなす)
const char* const k_item_id = "C-RLC-03";
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

// JSON オブジェクトから最初に見つかった整数キー(深さ優先)を返す
bool findIntegerKey( const QJsonObject& object, const QString& key, int* value )
{
    if ( object.contains( key ) && object.value( key ).isDouble() )
    {
        if ( value ) { *value = object.value( key ).toInt(); }
        return true;
    }
    for ( auto it = object.begin(); it != object.end(); ++it )
    {
        if ( it.value().isObject() && findIntegerKey( it.value().toObject(), key, value ) ) { return true; }
    }
    return false;
}
}

namespace ClientTests
{

RepetitionLevelControlInSituTest::RepetitionLevelControlInSituTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArrayLiteral( "1800000" ) );
}

QString RepetitionLevelControlInSituTest::repoRootPath() const
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

QString RepetitionLevelControlInSituTest::configuredValue( const char* name ) const
{
    const QString env = QProcessEnvironment::systemEnvironment().value( QString::fromLatin1( name ) );
    if ( !env.trimmed().isEmpty() ) { return env.trimmed(); }

    QSettings settings( testPathConfigFilePath( repoRootPath() ), QSettings::IniFormat );
    settings.beginGroup( QStringLiteral( "paths" ) );
    const QString value = settings.value( QString::fromLatin1( name ) ).toString();
    settings.endGroup();
    return value.trimmed();
}

QString RepetitionLevelControlInSituTest::configuredPathOrEmpty( const char* name ) const
{
    if ( configuredValue( name ).isEmpty() ) { return QString(); }
    return configuredPath( name, repoRootPath() );
}

bool RepetitionLevelControlInSituTest::waitForCondition(
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

void RepetitionLevelControlInSituTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void RepetitionLevelControlInSituTest::bringRepetitionLevelControlToFront( RepetitionLevelControl* control ) const
{
    QVERIFY2( control != nullptr, "RepetitionLevelControl is null" );
    control->show();
    control->raise();
    control->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void RepetitionLevelControlInSituTest::markStep( const QString& description, const QString& status )
{
    const QString line = QStringLiteral( "%1 %2" ).arg( QString::fromLatin1( k_item_id ), description );
    m_steps.push_back( { line, status } );
    qInfo().noquote() << QStringLiteral( "[%1] %2" ).arg( status, line );
}

bool RepetitionLevelControlInSituTest::check( bool condition, const QString& message )
{
    if ( condition ) { return true; }
    m_failures << message;
    markStep( message, QStringLiteral( "FAIL" ) );
    return false;
}

// 既存の RepetitionLevelControlTest::saveScreenshot と同じ方法(プライマリスクリーン全体)で撮影し、
// テスト用ファイルに加えてマニュアル用のコピー(img/manual/ 以下)も出力する。
void RepetitionLevelControlInSituTest::saveScreenshot(
    int seq,
    const QString& content,
    const QString& caption,
    const QString& manual_rel_path,
    int time_step )
{
    ScreenshotEntry entry;
    entry.seq = seq;
    entry.file_name = QStringLiteral( "%1_%2_%3.png" ).arg( QString::fromLatin1( k_item_id ) ).arg( seq ).arg( content );
    entry.caption = caption;
    entry.manual_rel_path = manual_rel_path;
    entry.time_step = time_step;

    if ( !ClientTests::screenshotsEnabled() )
    {
        m_screenshots.push_back( entry );
        markStep( QStringLiteral( "スクリーンショット(連番 %1)は撮影無効のため SKIP" ).arg( seq ), QStringLiteral( "SKIP" ) );
        return;
    }

    QTest::qWait( k_capture_settle_ms );
    QScreen* screen = QGuiApplication::primaryScreen();
    if ( !check( screen != nullptr, QStringLiteral( "プライマリスクリーンが見つからない(連番 %1)" ).arg( seq ) ) )
    {
        m_screenshots.push_back( entry );
        return;
    }

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( entry.file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );
    if ( !check( !screenshot.isNull(), QStringLiteral( "スクリーンショットの取得に失敗(連番 %1)" ).arg( seq ) ) )
    {
        m_screenshots.push_back( entry );
        return;
    }
    entry.captured = screenshot.save( file_path );
    check( entry.captured, QStringLiteral( "スクリーンショットの保存に失敗: %1" ).arg( file_path ) );

    if ( entry.captured )
    {
        const QString destination = QDir( m_manual_dir_path ).absoluteFilePath( manual_rel_path );
        QDir().mkpath( QFileInfo( destination ).absolutePath() );
        if ( QFileInfo::exists( destination ) ) { QFile::remove( destination ); }
        entry.copied = QFile::copy( file_path, destination );
        check( entry.copied, QStringLiteral( "マニュアル用コピーに失敗: %1" ).arg( manual_rel_path ) );
    }
    m_screenshots.push_back( entry );
    markStep( QStringLiteral( "スクリーンショットを撮影: %1 → manual: img/manual/%2" ).arg( caption, manual_rel_path ) );
}

void RepetitionLevelControlInSituTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    if ( !report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        qWarning().noquote() << QStringLiteral( "Failed to write test report: %1" ).arg( m_report_path );
        return;
    }

    QTextStream stream( &report_file );
    stream << "# RepetitionLevelControlInSituTest\n\n";
    stream << "- 対応項目: PBVRリグレッションテストチェックシート 追加項目仕様_アンサンブル C-RLC-03(ISモードで Next Repetition Level を操作する — サーバ側粒子生成数の変更)\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- 前提: サーバ(pbvr_server)とアンサンブルシミュレーション(ens_Hydrogen_unstruct)は起動済み。テストからは起動・終了しない\n";
    stream << "- 接続モード: In-Situ(RemoteInSitu)、Keep Last ON\n";
    stream << "- ENSEMBLE_VIS_PARAM_DIR: `" << ( m_vis_param_dir.isEmpty() ? QStringLiteral( "(未設定: default.json の突合は SKIP)" ) : m_vis_param_dir ) << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`(マニュアル用コピー: `img/manual/`)\n\n";

    stream << "## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << step.status << ": " << step.description << "\n";
    }

    stream << "\n## Repetition Level と Total Particles の対応表\n\n";
    stream << "| 区分 | 設定した Repetition Level | サーバ再配信 RepeatLevel(参考) | Total Particles | 記録タイムステップ | 変化基準 C_prev | Apply 後の最新ステップ通知 M' | 下限(M'+" << k_generation_margin_steps << ") | 読み取り回数 | 変化・安定の経緯 | 記録ステップの default_XXXXX.json repeat_level |\n";
    stream << "|---|---|---|---|---|---|---|---|---|---|---|\n";
    const auto num = []( int value ) { return value >= 0 ? QString::number( value ) : QStringLiteral( "-" ); };
    for ( const LevelRecord& record : m_records )
    {
        const bool initial = record.label == QStringLiteral( "初期値" );
        stream << "| " << record.label
               << " | " << num( record.level )
               << " | " << num( record.echoed_level )
               << " | " << num( record.total_particles )
               << " | " << num( record.time_step )
               << " | " << ( initial ? QStringLiteral( "-" ) : num( record.prev_count ) )
               << " | " << num( record.max_step_after_apply )
               << " | " << num( record.threshold_step )
               << " | " << ( initial ? QStringLiteral( "-" ) : QString::number( record.updates_waited ) )
               << " | " << ( record.change_note.isEmpty() ? QStringLiteral( "-" ) : record.change_note )
               << " | " << record.default_json_check << " |\n";
    }
    stream << "\n- 初期値: 接続時(Initialize)に受信した RepeatLevel = "
           << ( m_initial_level >= 0 ? QString::number( m_initial_level ) : QStringLiteral( "(未受信)" ) ) << "\n";
    stream << "- particle_limit(default.json から取得): "
           << ( m_particle_limit > 0 ? QString::number( m_particle_limit ) : QStringLiteral( "(不明)" ) ) << "\n";

    stream << "\n## 判定\n\n";
    if ( m_judgements.isEmpty() )
    {
        stream << "- 判定は実施されなかった(手順の途中で失敗)\n";
    }
    for ( const QString& judgement : m_judgements )
    {
        stream << "- " << judgement << "\n";
    }
    if ( !m_failures.isEmpty() )
    {
        stream << "\n### 失敗内容\n\n";
        for ( const QString& failure : m_failures )
        {
            stream << "- " << failure << "\n";
        }
    }

    stream << "\n## 備考\n\n";
    stream << "- Keep Last ON のクライアントは、サーバの state.txt 監視スレッドが配信する LatestTimeStep(UpdateMaxTimeStep)の値をそのまま次の要求ステップにするため、各受信は「その時点の最新ステップ」へのジャンプになる(+1 ではない)。\n";
    stream << "- InSituLib は各ステップの冒頭で default.json を読み、粒子出力後に state.txt の LATEST_STEP を更新する。サーバの最新ステップ通知(M')はポーリング(1 秒)と通信遅延を含み、Apply による default.json 書き込みも非同期のため、シミュレーションが速いと M'+" << k_generation_margin_steps << " のステップでもまだ Apply 前の Repetition Level で生成されていることがある(固定マージンでは不十分)。\n";
    stream << "- そこで記録対象は「粒子数が前ケース(C_prev)から " << static_cast<int>( k_particle_change_ratio * 100.0 ) << "%% 超変化し、かつ次ステップでも " << static_cast<int>( k_particle_stable_ratio * 100.0 ) << "%% 以内で安定したステップ」とする(下限は M'+" << k_generation_margin_steps << ")。Level 変化は粒子数が 2 倍以上動くのに対し同一 Level のステップ間変動は小さいため、粒子数で反映を検出できる。C_prev は 初期値 → Level 1 → Level 8 → Level 16 の順に直前ケースの粒子数を使う。\n";
    stream << "- 比例判定は固定の「2 以上」ではなく、期待比(Level の比。N8/N1=8、N16/N8=2)に対する許容率 "
           << static_cast<int>( k_ratio_tolerance * 100.0 ) << "%% の下限で判定する。倍化はちょうど期待比付近になり、サンプリング誤差で期待比を数%% 下回る(例: 8→16 で約 1.995)ことを許容するため。\n";
    stream << "- CS モード(既存 RepetitionLevelControlTest)ではクライアント描画側の繰り返し数だけが変わり、サーバへは送信されない。本テストは RemoteInSitu かつ Operator のときだけ送信される経路(RepetitionLevelControl::onApply)を確認する。\n";
    stream << "- サーバ側の反映は「しきい値以上のタイムステップの Total Particles」と(ENSEMBLE_VIS_PARAM_DIR 設定時)記録ステップの default_XXXXX.json の settings.sampling.repeat_level で判定する。default.json はシミュレーションが読み込み後に default_old.json へリネームするため、ステップ別 JSON で突合する。サーバの再配信(RepetitionLevelParameter)は uWebSockets の ws->publish が送信元を除外するため単一クライアントでは受信できず、参考情報として記録するのみ(2 クライアント実行時に他クライアントで確認する)。\n";

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.seq << ". " << entry.caption << "\n\n";
        if ( entry.time_step >= 0 ) { stream << "- time step: " << entry.time_step << "\n"; }
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
        stream << "| " << k_item_id << " | " << entry.seq << " | `img/" << entry.file_name << "` | `img/manual/" << entry.manual_rel_path
               << "` | " << ( entry.time_step >= 0 ? QString::number( entry.time_step ) : QStringLiteral( "-" ) )
               << " | " << ( entry.copied ? "出力済み" : "未出力(SKIP)" ) << " |\n";
    }
}

RepetitionLevelControlInSituTest::ClientHandles
RepetitionLevelControlInSituTest::resolveClientHandles( MainWindow& window ) const
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
    handles.repetition_level_control = window.findChild<RepetitionLevelControl*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.total_particles_tool_bar != nullptr, "TotalParticlesToolBar not found" ) ) { return handles; }
    if ( !require( handles.repetition_level_control != nullptr, "RepetitionLevelControl dock not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_insitu_radio = handles.communication->findChild<QRadioButton*>( "remoteVizInsituRadioButton" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.keep_last_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_keep_last_push_button" );
    handles.current_repetition_level_label =
        handles.repetition_level_control->findChild<QLabel*>( "dispCurrentRepetitionLevel" );
    handles.new_repetition_level_spin_box =
        handles.repetition_level_control->findChild<QSpinBox*>( "spinBoxNewRepetitionLevel" );
    handles.repetition_apply_button = handles.repetition_level_control->findChild<QPushButton*>( "applyPushButton" );

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
    if ( !require( handles.total_particles_display != nullptr, "Total Particles display label not found" ) ) { return handles; }
    if ( !require( handles.current_repetition_level_label != nullptr, "dispCurrentRepetitionLevel not found" ) ) { return handles; }
    if ( !require( handles.new_repetition_level_spin_box != nullptr, "spinBoxNewRepetitionLevel not found" ) ) { return handles; }
    if ( !require( handles.repetition_apply_button != nullptr, "RepetitionLevelControl applyPushButton not found" ) ) { return handles; }
    return handles;
}

void RepetitionLevelControlInSituTest::connectClient( const ClientHandles& client ) const
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

void RepetitionLevelControlInSituTest::configureInSituVisualization( const ClientHandles& client ) const
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

// spy[start_index..] の中から RepeatLevel を含む payload を探す。
// expected_level < 0 のときは最初に受信した値を返す。
bool RepetitionLevelControlInSituTest::waitForRepeatLevel(
    QSignalSpy& spy,
    int start_index,
    int expected_level,
    int timeout_ms,
    int* received_level ) const
{
    const QString key = QString::fromUtf8( Protocol::Key::RepeatLevel );
    int inspected = start_index;
    return waitForCondition(
        [&spy, &inspected, key, expected_level, received_level]()
        {
            while ( inspected < spy.count() )
            {
                const QList<QVariant> arguments = spy.at( inspected++ );
                if ( arguments.isEmpty() ) { continue; }
                const QJsonObject payload = qvariant_cast<QJsonObject>( arguments.at( 0 ) );
                if ( !payload.value( key ).isDouble() ) { continue; }
                const int level = payload.value( key ).toInt();
                if ( expected_level >= 0 && level != expected_level ) { continue; }
                if ( received_level ) { *received_level = level; }
                return true;
            }
            return false;
        },
        timeout_ms,
        50 );
}

void RepetitionLevelControlInSituTest::waitForObjectAndApply( const ClientHandles& client ) const
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

    QAbstractItemModel* model = client.object_tree_view->model();
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

void RepetitionLevelControlInSituTest::clickJumpAndWaitForCompletion( const ClientHandles& client, QSignalSpy& completion_spy ) const
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

void RepetitionLevelControlInSituTest::enableKeepLast( const ClientHandles& client ) const
{
    QVERIFY2( client.keep_last_button->isEnabled(), "m_keep_last_push_button is disabled" );
    if ( !client.keep_last_button->isChecked() )
    {
        QTest::mouseClick( client.keep_last_button, Qt::LeftButton );
    }
    QVERIFY2( client.keep_last_button->isChecked(), "Keep Last was not enabled" );
}

void RepetitionLevelControlInSituTest::waitForPositiveParticleCount( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [this, client]() { return totalParticles( client ) > 0; },
            k_data_request_timeout_ms,
            100 ),
        qPrintable(
            QStringLiteral( "Total Particles did not become positive. current='%1'" )
                .arg( client.total_particles_display->text() ) ) );
}

int RepetitionLevelControlInSituTest::totalParticles( const ClientHandles& client ) const
{
    bool ok = false;
    const int value = client.total_particles_display->text().trimmed().toInt( &ok );
    return ok ? value : -1;
}

int RepetitionLevelControlInSituTest::latestCompletedTimeStep( const Spies& spies ) const
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
// この呼び出し以降に emit された TimeStepControlToolBar::requestDataAt を待ち、
// 同じタイムステップの dataRequestCompleted を受信するまで待機する。
bool RepetitionLevelControlInSituTest::waitForNextTimeStep( const Spies& spies, int* time_step ) const
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
        // Keep Last により受信済みステップを再要求することがあるため、新しいステップの要求のみ対象にする
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

// LatestTimeStep イベント(Communication::updateObjectLatestTimeStep)の UpdateMaxTimeStep を取り出す。
// サーバの state.txt 監視スレッドが LATEST_STEP の増加を検出するたびに配信する値で、
// クライアントの Keep Last はこの値をそのまま次の要求ステップにする。
int RepetitionLevelControlInSituTest::notifiedMaxTimeStepAt( const Spies& spies, int index ) const
{
    if ( !spies.latest_step_spy || index < 0 || index >= spies.latest_step_spy->count() ) { return -1; }
    const QList<QVariant> arguments = spies.latest_step_spy->at( index );
    if ( arguments.isEmpty() ) { return -1; }
    const QJsonObject payload = arguments.at( 0 ).toJsonObject();
    const QString key = QString::fromUtf8( Protocol::Key::UpdateMaxTimeStep.data(), static_cast<int>( Protocol::Key::UpdateMaxTimeStep.size() ) );
    if ( !payload.contains( key ) || !payload.value( key ).isDouble() ) { return -1; }
    return payload.value( key ).toInt();
}

int RepetitionLevelControlInSituTest::latestNotifiedMaxTimeStep( const Spies& spies ) const
{
    int latest = -1;
    if ( !spies.latest_step_spy ) { return latest; }
    for ( int index = 0; index < spies.latest_step_spy->count(); ++index )
    {
        latest = std::max( latest, notifiedMaxTimeStepAt( spies, index ) );
    }
    return latest;
}

// start_index 以降に受信した最初の LatestTimeStep イベントの UpdateMaxTimeStep を待つ。
bool RepetitionLevelControlInSituTest::waitForNotifiedMaxTimeStep( const Spies& spies, int start_index, int timeout_ms, int* value ) const
{
    if ( !spies.latest_step_spy ) { return false; }
    int found = -1;
    const bool received = waitForCondition(
        [this, spies, start_index, &found]()
        {
            for ( int index = start_index; index < spies.latest_step_spy->count(); ++index )
            {
                const int step = notifiedMaxTimeStepAt( spies, index );
                if ( step >= 0 ) { found = step; return true; }
            }
            return false;
        },
        timeout_ms,
        50 );
    if ( received && value ) { *value = found; }
    return received;
}

// completion_start 以降の dataRequestCompleted のうち、タイムステップが threshold 以上のものを受信するまで
// Keep Last による更新を繰り返し待つ。計算速度が速ければ 1 回の受信で到達し、遅ければ複数回待つ。
bool RepetitionLevelControlInSituTest::waitForTimeStepAtOrAfter(
    const Spies& spies,
    int completion_start,
    int threshold,
    int timeout_ms,
    int* time_step,
    int* updates )
{
    QElapsedTimer timer;
    timer.start();
    int reported = completion_start;
    while ( true )
    {
        for ( int index = completion_start; index < spies.completion_spy->count(); ++index )
        {
            const QList<QVariant> arguments = spies.completion_spy->at( index );
            if ( arguments.isEmpty() ) { continue; }
            const int step = arguments.at( 0 ).toInt();
            if ( index >= reported )
            {
                reported = index + 1;
                if ( step < threshold )
                {
                    markStep( QStringLiteral( "受信(time step %1)はしきい値 %2 未満のため記録しない(Apply 前に生成中だった可能性があるステップ)" ).arg( step ).arg( threshold ),
                              QStringLiteral( "INFO" ) );
                }
            }
            if ( step >= threshold )
            {
                if ( time_step ) { *time_step = step; }
                if ( updates ) { *updates = index - completion_start + 1; }
                return true;
            }
        }
        if ( timer.elapsed() >= timeout_ms ) { return false; }

        int next_step = -1;
        if ( !waitForNextTimeStep( spies, &next_step ) ) { return false; }
    }
}

// Apply 後、粒子数が前ケース(prev_count = C_prev)から十分変化し、かつ次ステップでも安定したステップを
// 「新しい Repetition Level が反映されたステップ」とみなして記録する。threshold_floor(= M'+2)を下限として、
// それ未満のステップ(確実に Apply 前の在庫ステップ)は判定に使わない。
// サーバが別マシンで default_XXXXX.json を突合できない場合でも、粒子数の大きな変化(Level 変化は 2 倍以上)で
// 反映を検出できる。固定ステップマージンではシミュレーションが速いと在庫ステップを拾うため、本方式に変更した。
bool RepetitionLevelControlInSituTest::waitForStableChangedParticleCount(
    const ClientHandles& client,
    const Spies& spies,
    int threshold_floor,
    int prev_count,
    int timeout_ms,
    int* time_step,
    int* total_particles,
    int* updates,
    QString* note )
{
    QElapsedTimer timer;
    timer.start();
    int received = 0;        // threshold_floor 以上で読み取ったステップ数
    int candidate_total = -1; // 変化を確認した直近の粒子数(安定確認待ち)
    int candidate_step = -1;
    QStringList log;

    const auto rel_diff = []( int a, int b ) -> double
    {
        if ( b <= 0 ) { return 1.0; }
        return std::abs( static_cast<double>( a ) - static_cast<double>( b ) ) / static_cast<double>( b );
    };

    while ( timer.elapsed() < timeout_ms )
    {
        int step = -1;
        if ( !waitForNextTimeStep( spies, &step ) )
        {
            if ( note ) { *note = log.join( QStringLiteral( " / " ) ) + QStringLiteral( " / 新しいステップ待ちでタイムアウト" ); }
            return false;
        }
        if ( step < threshold_floor )
        {
            markStep( QStringLiteral( "受信(time step %1)は下限 %2 未満のため使わない(Apply 前の在庫ステップ)" ).arg( step ).arg( threshold_floor ),
                      QStringLiteral( "INFO" ) );
            continue;
        }

        // ラベルがそのステップの粒子数に更新されるのを待って読む
        waitForCondition( [this, client]() { return totalParticles( client ) > 0; }, k_short_wait_ms * 5, 100 );
        const int total = totalParticles( client );
        ++received;

        const double change = rel_diff( total, prev_count );
        if ( change <= k_particle_change_ratio )
        {
            // まだ前ケースと同程度 = Apply 前の Level で生成されたステップの可能性が高い。記録せず次を待つ。
            markStep( QStringLiteral( "time step %1: 粒子数 %2 は前ケース %3 からの変化が小さい(%4%、下限 %5%)。反映前とみなし次ステップを待つ" )
                          .arg( step ).arg( total ).arg( prev_count )
                          .arg( change * 100.0, 0, 'f', 1 ).arg( k_particle_change_ratio * 100.0, 0, 'f', 0 ),
                      QStringLiteral( "INFO" ) );
            candidate_total = -1; // 変化が途切れたら候補をリセット
            continue;
        }

        if ( candidate_total > 0 )
        {
            const double stable = rel_diff( total, candidate_total );
            if ( stable <= k_particle_stable_ratio )
            {
                // 前ケースから変化し、連続 2 ステップで安定 → 反映済みとして記録
                log << QStringLiteral( "前ケース %1 → %2(%3%% 変化)で安定(連続差 %4%%)" )
                           .arg( prev_count ).arg( total )
                           .arg( change * 100.0, 0, 'f', 1 ).arg( stable * 100.0, 0, 'f', 1 );
                if ( time_step ) { *time_step = std::max( step, latestCompletedTimeStep( spies ) ); }
                if ( total_particles ) { *total_particles = total; }
                if ( updates ) { *updates = received; }
                if ( note ) { *note = log.join( QStringLiteral( " / " ) ); }
                return true;
            }
        }
        // 変化を確認したが安定未確認(または候補が変わった)。今回値を候補にして次ステップで確認する。
        markStep( QStringLiteral( "time step %1: 粒子数 %2 は前ケース %3 から %4%% 変化。安定確認のため次ステップを待つ" )
                      .arg( step ).arg( total ).arg( prev_count ).arg( change * 100.0, 0, 'f', 1 ),
                  QStringLiteral( "INFO" ) );
        candidate_total = total;
        candidate_step = step;
    }

    if ( note )
    {
        *note = QStringLiteral( "前ケース %1 からの変化・安定を %2 秒以内に確認できず(最後の候補: step %3 / %4)" )
                    .arg( prev_count ).arg( timeout_ms / 1000 )
                    .arg( candidate_step ).arg( candidate_total );
    }
    return false;
}

bool RepetitionLevelControlInSituTest::applyRepetitionLevel(
    const ClientHandles& client,
    const Spies& spies,
    int level,
    int* echoed_level,
    int* latest_step_start_index,
    int* completion_start_index )
{
    bringRepetitionLevelControlToFront( client.repetition_level_control );
    if ( !check( client.repetition_level_control->isVisible(), QStringLiteral( "RepetitionLevelControl が表示されない" ) ) ) { return false; }
    if ( !check( client.new_repetition_level_spin_box->isEnabled() && client.repetition_apply_button->isEnabled(),
                 QStringLiteral( "spinBoxNewRepetitionLevel / applyPushButton が無効(Operator ではない)" ) ) )
    {
        return false;
    }
    if ( !check( level >= client.new_repetition_level_spin_box->minimum() && level <= client.new_repetition_level_spin_box->maximum(),
                 QStringLiteral( "Repetition Level %1 がスピンボックスの範囲外" ).arg( level ) ) )
    {
        return false;
    }

    client.new_repetition_level_spin_box->setFocus();
    client.new_repetition_level_spin_box->setValue( level );
    QCoreApplication::processEvents();
    if ( !check( client.new_repetition_level_spin_box->value() == level,
                 QStringLiteral( "spinBoxNewRepetitionLevel に %1 を設定できない" ).arg( level ) ) )
    {
        return false;
    }

    const int echo_start = spies.repetition_spy->count();
    // Apply 直前の受信位置を記録する(Apply 後に届いた LatestTimeStep / dataRequestCompleted だけを判定に使うため)
    if ( latest_step_start_index ) { *latest_step_start_index = spies.latest_step_spy ? spies.latest_step_spy->count() : 0; }
    if ( completion_start_index ) { *completion_start_index = spies.completion_spy->count(); }
    QTest::mouseClick( client.repetition_apply_button, Qt::LeftButton );
    markStep( QStringLiteral( "RepetitionLevelControl.ui: spinBoxNewRepetitionLevel を %1 に設定し applyPushButton を押した" ).arg( level ) );

    // RepetitionLevelControl::onApply はローカルで Current Repetition Level を更新し、
    // RemoteInSitu かつ Operator のときサーバへ RepetitionLevelParameter を送信する。
    const bool displayed = waitForCondition(
        [client, level]() { return client.current_repetition_level_label->text().trimmed().toInt() == level; },
        k_short_wait_ms * 3,
        50 );
    check( displayed, QStringLiteral( "dispCurrentRepetitionLevel が %1 にならない(実際: %2)" ).arg( level ).arg( client.current_repetition_level_label->text() ) );
    if ( displayed )
    {
        markStep( QStringLiteral( "dispCurrentRepetitionLevel が %1 であることを確認した(Apply 時のローカル更新)" ).arg( level ) );
    }

    // サーバの再配信(ws->publish)は uWebSockets の仕様で送信元クライアントには届かないため、
    // 受信の有無は判定に使わず参考情報として記録する(2 クライアント実行時は他クライアントで受信できる)。
    int received = -1;
    const bool echoed = waitForRepeatLevel( *spies.repetition_spy, echo_start, level, k_echo_observe_ms, &received );
    if ( echoed_level ) { *echoed_level = received; }
    markStep( QStringLiteral( "サーバ再配信 RepetitionLevelParameter(参考): %1" )
                  .arg( echoed ? QStringLiteral( "受信(RepeatLevel=%1)" ).arg( received )
                               : QStringLiteral( "未受信(単一クライアントでは送信元に配信されない仕様)" ) ),
              QStringLiteral( "INFO" ) );
    return true;
}

QString RepetitionLevelControlInSituTest::defaultJsonPath() const
{
    if ( m_vis_param_dir.isEmpty() ) { return QString(); }
    const QString name = ( m_tf_name.isEmpty() ? QStringLiteral( "default" ) : m_tf_name ) + QStringLiteral( ".json" );
    return QDir( m_vis_param_dir ).absoluteFilePath( name );
}

// タイムステップ別 JSON(default_XXXXX.json)。シミュレーションが各ステップの粒子生成に使った設定の控えで、
// サーバは RequestDataAt の応答に同じファイルを読む。
QString RepetitionLevelControlInSituTest::stepJsonPath( int time_step ) const
{
    if ( m_vis_param_dir.isEmpty() || time_step < 0 ) { return QString(); }
    const QString prefix = m_tf_name.isEmpty() ? QStringLiteral( "default" ) : m_tf_name;
    return QDir( m_vis_param_dir ).absoluteFilePath(
        QStringLiteral( "%1_%2.json" ).arg( prefix ).arg( time_step, 5, 10, QLatin1Char( '0' ) ) );
}

// 記録したステップの default_XXXXX.json の settings.sampling.repeat_level が設定値と一致することを確認する
// (ENSEMBLE_VIS_PARAM_DIR 未設定時は SKIP)。default.json はシミュレーションが読み込み後に default_old.json へ
// リネームするため、ステップ別 JSON で「そのステップが設定値で生成された」ことを確認する。
QString RepetitionLevelControlInSituTest::checkStepJsonRepeatLevel( int time_step, int expected_level )
{
    const QString path = stepJsonPath( time_step );
    if ( path.isEmpty() )
    {
        markStep( QStringLiteral( "default_XXXXX.json の repeat_level 突合は ENSEMBLE_VIS_PARAM_DIR 未設定(または記録ステップ不明)のため SKIP" ), QStringLiteral( "SKIP" ) );
        return QStringLiteral( "SKIP" );
    }

    int actual = -1;
    bool readable = false;
    const bool matched = waitForCondition(
        [&]()
        {
            QFile file( path );
            if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) { return false; }
            QJsonParseError error;
            const QJsonDocument document = QJsonDocument::fromJson( file.readAll(), &error );
            if ( error.error != QJsonParseError::NoError || !document.isObject() ) { return false; }
            readable = true;
            const QJsonObject settings = document.object().value( QStringLiteral( "settings" ) ).toObject();
            const QJsonObject sampling = settings.value( QStringLiteral( "sampling" ) ).toObject();
            if ( !sampling.value( QStringLiteral( "repeat_level" ) ).isDouble() ) { return false; }
            actual = sampling.value( QStringLiteral( "repeat_level" ) ).toInt();
            return actual == expected_level;
        },
        k_default_json_retry_ms,
        200 );

    if ( !readable )
    {
        check( false, QStringLiteral( "記録ステップの JSON を読めない: %1" ).arg( path ) );
        return QStringLiteral( "FAIL(読めない)" );
    }
    if ( !check( matched, QStringLiteral( "time step %1 の settings.sampling.repeat_level が %2 ではない(実際: %3): %4" ).arg( time_step ).arg( expected_level ).arg( actual ).arg( path ) ) )
    {
        return QStringLiteral( "FAIL(%1)" ).arg( actual );
    }
    markStep( QStringLiteral( "time step %1 の default_%2.json の settings.sampling.repeat_level が %3 であることを確認した" )
                  .arg( time_step ).arg( time_step, 5, 10, QLatin1Char( '0' ) ).arg( expected_level ) );
    return QStringLiteral( "PASS(%1)" ).arg( actual );
}

// default.json の settings.sampling.repeat_level を突合する(ENSEMBLE_VIS_PARAM_DIR 未設定時は SKIP)。
// strict == false のときは値を記録するだけで判定しない(接続直後の初期値は default.json と一致しない場合がある)。
QString RepetitionLevelControlInSituTest::checkDefaultJsonRepeatLevel( int expected_level, int* particle_limit, bool strict )
{
    const QString path = defaultJsonPath();
    if ( path.isEmpty() )
    {
        markStep( QStringLiteral( "default.json の repeat_level 突合は ENSEMBLE_VIS_PARAM_DIR 未設定のため SKIP" ), QStringLiteral( "SKIP" ) );
        return QStringLiteral( "SKIP" );
    }

    int actual = -1;
    bool readable = false;
    const bool matched = waitForCondition(
        [&]()
        {
            QFile file( path );
            if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) { return false; }
            QJsonParseError error;
            const QJsonDocument document = QJsonDocument::fromJson( file.readAll(), &error );
            if ( error.error != QJsonParseError::NoError || !document.isObject() ) { return false; }
            readable = true;
            const QJsonObject settings = document.object().value( QStringLiteral( "settings" ) ).toObject();
            const QJsonObject sampling = settings.value( QStringLiteral( "sampling" ) ).toObject();
            if ( particle_limit )
            {
                int limit = -1;
                if ( findIntegerKey( settings, QStringLiteral( "particle_limit" ), &limit ) ) { *particle_limit = limit; }
            }
            if ( !sampling.value( QStringLiteral( "repeat_level" ) ).isDouble() ) { return false; }
            actual = sampling.value( QStringLiteral( "repeat_level" ) ).toInt();
            return actual == expected_level;
        },
        k_default_json_retry_ms,
        200 );

    if ( !readable )
    {
        if ( !strict )
        {
            markStep( QStringLiteral( "default.json を読めないため初期値の突合は記録のみ: %1" ).arg( path ), QStringLiteral( "SKIP" ) );
            return QStringLiteral( "SKIP(読めない)" );
        }
        check( false, QStringLiteral( "default.json を読めない: %1" ).arg( path ) );
        return QStringLiteral( "FAIL(読めない)" );
    }
    if ( !strict )
    {
        markStep( QStringLiteral( "default.json の settings.sampling.repeat_level を記録した(%1、Initialize の RepeatLevel は %2)" ).arg( actual ).arg( expected_level ) );
        return QStringLiteral( "記録(%1)" ).arg( actual );
    }
    if ( !check( matched, QStringLiteral( "default.json の settings.sampling.repeat_level が %1 ではない(実際: %2): %3" ).arg( expected_level ).arg( actual ).arg( path ) ) )
    {
        return QStringLiteral( "FAIL(%1)" ).arg( actual );
    }
    markStep( QStringLiteral( "default.json の settings.sampling.repeat_level が %1 であることを確認した: %2" ).arg( expected_level ).arg( path ) );
    return QStringLiteral( "PASS(%1)" ).arg( actual );
}

void RepetitionLevelControlInSituTest::runLevelCase(
    const ClientHandles& client,
    const Spies& spies,
    int level,
    int seq,
    const QString& manual_rel_path )
{
    LevelRecord record;
    record.label = QString::number( level );
    record.level = level;
    record.max_step_before_apply = latestNotifiedMaxTimeStep( spies );
    // 変化判定の基準 = 直前に記録したケースの粒子数(初期値 → Level 1 → Level 8 → Level 16 の順に連鎖)
    record.prev_count = ( !m_records.empty() && m_records.back().total_particles > 0 )
                            ? m_records.back().total_particles : -1;

    const auto skip_rest = [&]( const QString& reason )
    {
        record.default_json_check = QStringLiteral( "-" );
        m_records.push_back( record );
        ScreenshotEntry skipped;
        skipped.seq = seq;
        skipped.file_name = QStringLiteral( "%1_%2_level%3.png" ).arg( QString::fromLatin1( k_item_id ) ).arg( seq ).arg( level );
        skipped.caption = QStringLiteral( "Repetition Level = %1(%2のため撮影 SKIP)" ).arg( level ).arg( reason );
        skipped.manual_rel_path = manual_rel_path;
        m_screenshots.push_back( skipped );
        markStep( QStringLiteral( "Repetition Level %1 の後続手順(ステップ待ち・撮影)は%2のため SKIP" ).arg( level ).arg( reason ), QStringLiteral( "SKIP" ) );
    };

    int latest_step_start = 0;
    int completion_start = 0;
    const bool applied = applyRepetitionLevel( client, spies, level, &record.echoed_level, &latest_step_start, &completion_start );
    if ( !applied )
    {
        skip_rest( QStringLiteral( "Apply 失敗" ) );
        return;
    }

    // Apply 後に最初にサーバから通知された最新ステップ M'(LatestTimeStep イベントの UpdateMaxTimeStep)。
    // Apply 時点でシミュレーションが生成中だったステップは高々 M'+1(そのステップは変更前の Repetition Level で生成される)ため、
    // M' + k_generation_margin_steps 以上のステップを受信してから Total Particles を記録する。
    int notified_after = -1;
    const bool notified = waitForNotifiedMaxTimeStep( spies, latest_step_start, k_data_request_timeout_ms, &notified_after );
    if ( !check( notified, QStringLiteral( "Repetition Level %1 の Apply 後、%2 秒以内に LatestTimeStep(UpdateMaxTimeStep)を受信しない。シミュレーションが新しいステップを出力しているか確認する" ).arg( level ).arg( k_data_request_timeout_ms / 1000 ) ) )
    {
        skip_rest( QStringLiteral( "LatestTimeStep 未受信" ) );
        return;
    }
    record.max_step_after_apply = notified_after;
    record.threshold_step = notified_after + k_generation_margin_steps;
    markStep( QStringLiteral( "Apply 後に最初に通知された最新ステップ M' = %1(Apply 前の通知: %2)。下限 time step >= %3(M' + %4)、変化基準 C_prev = %5" )
                  .arg( notified_after )
                  .arg( record.max_step_before_apply >= 0 ? QString::number( record.max_step_before_apply ) : QStringLiteral( "なし" ) )
                  .arg( record.threshold_step )
                  .arg( k_generation_margin_steps )
                  .arg( record.prev_count > 0 ? QString::number( record.prev_count ) : QStringLiteral( "不明" ) ) );

    // 下限(M'+2)を満たしつつ、粒子数が前ケース C_prev から変化して安定したステップを記録する。
    int time_step = -1;
    int total = -1;
    int updates = 0;
    const bool reached = waitForStableChangedParticleCount(
        client, spies, record.threshold_step, record.prev_count, k_threshold_wait_timeout_ms,
        &time_step, &total, &updates, &record.change_note );
    if ( check( reached, QStringLiteral( "Repetition Level %1 の Apply 後、粒子数が前ケース(%2)から変化して安定するステップを %3 秒以内に受信しない(%4)。Keep Last が ON でシミュレーションが新しいステップを出力しているか確認する" )
                             .arg( level )
                             .arg( record.prev_count > 0 ? QString::number( record.prev_count ) : QStringLiteral( "不明" ) )
                             .arg( k_threshold_wait_timeout_ms / 1000 )
                             .arg( record.change_note ) ) )
    {
        record.updates_waited = updates;
        record.total_particles = total;
        record.time_step = time_step;
        markStep( QStringLiteral( "time step %1(下限 %2 以上、Apply 後 %3 回目の読み取り)で粒子数が安定したので記録した(N%4 = %5、%6)" )
                      .arg( record.time_step ).arg( record.threshold_step ).arg( updates ).arg( level ).arg( total ).arg( record.change_note ) );
    }

    record.default_json_check = checkStepJsonRepeatLevel( record.time_step, level );
    m_records.push_back( record );

    bringWindowToFront( client.main_window );
    bringRepetitionLevelControlToFront( client.repetition_level_control );
    saveScreenshot(
        seq,
        QStringLiteral( "level%1" ).arg( level ),
        QStringLiteral( "Repetition Level = %1 を Apply し、Apply 後に生成されたタイムステップの粒子" ).arg( level ),
        manual_rel_path,
        record.time_step );
}

void RepetitionLevelControlInSituTest::judgeParticleCounts()
{
    int n1 = -1;
    int n8 = -1;
    int n16 = -1;
    for ( const LevelRecord& record : m_records )
    {
        if ( record.level == 1 ) { n1 = record.total_particles; }
        else if ( record.level == 8 ) { n8 = record.total_particles; }
        else if ( record.level == 16 ) { n16 = record.total_particles; }
    }

    if ( n1 <= 0 || n8 <= 0 || n16 <= 0 )
    {
        m_judgements << QStringLiteral( "FAIL: N1 / N8 / N16 のいずれかが取得できていない(N1=%1, N8=%2, N16=%3)" ).arg( n1 ).arg( n8 ).arg( n16 );
        check( false, QStringLiteral( "粒子数の比較判定ができない" ) );
        return;
    }

    // 初期値(Initialize の RepeatLevel、通常 4)との比較: Level 1 の粒子数は初期値より少ないこと
    int n0 = -1;
    for ( const LevelRecord& record : m_records )
    {
        if ( record.label == QStringLiteral( "初期値" ) ) { n0 = record.total_particles; }
    }
    if ( n0 > 0 && m_initial_level > 1 )
    {
        const bool initial_ok = n1 < n0;
        m_judgements << QStringLiteral( "%1: 初期値(Level %2)N0=%3 > N1=%4(N0/N1 = %5、参考: 初期 Level %2 に対する比)" )
                            .arg( initial_ok ? QStringLiteral( "PASS" ) : QStringLiteral( "FAIL" ) )
                            .arg( m_initial_level ).arg( n0 ).arg( n1 ).arg( static_cast<double>( n0 ) / static_cast<double>( n1 ), 0, 'f', 2 );
        check( initial_ok, QStringLiteral( "Level 1 の粒子数(N1=%1)が初期値 Level %2 の粒子数(N0=%3)より少なくない" ).arg( n1 ).arg( m_initial_level ).arg( n0 ) );
    }

    const bool increasing = n1 < n8 && n8 < n16;
    m_judgements << QStringLiteral( "%1: N1 < N8 < N16(N1=%2, N8=%3, N16=%4)" )
                        .arg( increasing ? QStringLiteral( "PASS" ) : QStringLiteral( "FAIL" ) ).arg( n1 ).arg( n8 ).arg( n16 );
    check( increasing, QStringLiteral( "N1 < N8 < N16 を満たさない(N1=%1, N8=%2, N16=%3)" ).arg( n1 ).arg( n8 ).arg( n16 ) );

    // 粒子数は Repetition Level にほぼ比例するため、実測比を「期待比(Level の比)」と突き合わせる。
    //   N8/N1 の期待比 = 8/1 = 8、N16/N8 の期待比 = 16/8 = 2
    // 倍化はちょうど期待比付近になり、サンプリング誤差で期待比を数%下回ることがある(例: 8→16 で 1.995)。
    // 固定の「2 以上」では境界で誤判定するため、期待比に対する許容率(k_ratio_tolerance)で判定する。
    const double ratio_8_1 = static_cast<double>( n8 ) / static_cast<double>( n1 );
    const double ratio_16_8 = static_cast<double>( n16 ) / static_cast<double>( n8 );
    const double expected_8_1 = 8.0;   // Level 8 / Level 1
    const double expected_16_8 = 2.0;  // Level 16 / Level 8
    const double lower_8_1 = expected_8_1 * ( 1.0 - k_ratio_tolerance );
    const double lower_16_8 = expected_16_8 * ( 1.0 - k_ratio_tolerance );
    const bool proportional = ratio_8_1 >= lower_8_1 && ratio_16_8 >= lower_16_8;
    const bool limit_reached = m_particle_limit > 0 && ( n8 >= m_particle_limit || n16 >= m_particle_limit );
    if ( proportional )
    {
        m_judgements << QStringLiteral( "PASS: N8/N1 = %1(期待 %2、下限 %3)、N16/N8 = %4(期待 %5、下限 %6)" )
                            .arg( ratio_8_1, 0, 'f', 3 ).arg( expected_8_1, 0, 'f', 1 ).arg( lower_8_1, 0, 'f', 2 )
                            .arg( ratio_16_8, 0, 'f', 3 ).arg( expected_16_8, 0, 'f', 1 ).arg( lower_16_8, 0, 'f', 2 );
    }
    else if ( limit_reached )
    {
        m_judgements << QStringLiteral( "SKIP: particle_limit(%1)に到達したため比例判定をスキップ(N8/N1 = %2, N16/N8 = %3)" )
                            .arg( m_particle_limit ).arg( ratio_8_1, 0, 'f', 3 ).arg( ratio_16_8, 0, 'f', 3 );
        markStep( QStringLiteral( "particle_limit 到達のため比例判定をスキップした" ), QStringLiteral( "SKIP" ) );
    }
    else
    {
        m_judgements << QStringLiteral( "FAIL: N8/N1 = %1(下限 %2)、N16/N8 = %3(下限 %4)を満たさない(許容率 %5%%。particle_limit: %6)" )
                            .arg( ratio_8_1, 0, 'f', 3 ).arg( lower_8_1, 0, 'f', 2 )
                            .arg( ratio_16_8, 0, 'f', 3 ).arg( lower_16_8, 0, 'f', 2 )
                            .arg( static_cast<int>( k_ratio_tolerance * 100.0 ) )
                            .arg( m_particle_limit > 0 ? QString::number( m_particle_limit ) : QStringLiteral( "不明" ) );
        check( false, QStringLiteral( "粒子数が Repetition Level に比例していない(N8/N1 = %1 < %2 または N16/N8 = %3 < %4)" )
                          .arg( ratio_8_1, 0, 'f', 3 ).arg( lower_8_1, 0, 'f', 2 ).arg( ratio_16_8, 0, 'f', 3 ).arg( lower_16_8, 0, 'f', 2 ) );
    }
}

void RepetitionLevelControlInSituTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_output_dir_path = datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "RepetitionLevelControlInSituTest" ) );
    m_screenshot_dir_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) );
    m_manual_dir_path = QDir( m_screenshot_dir_path ).absoluteFilePath( QStringLiteral( "manual" ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_vis_param_dir = configuredPathOrEmpty( "ENSEMBLE_VIS_PARAM_DIR" );
    m_tf_name = configuredValue( "TF_NAME" );
    if ( !m_vis_param_dir.isEmpty() && !QDir( m_vis_param_dir ).exists() )
    {
        qWarning().noquote() << QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR does not exist; default.json checks are skipped: %1" ).arg( m_vis_param_dir );
        m_vis_param_dir.clear();
    }
    m_test_succeeded = false;
    m_scenario_completed = false;

    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_manual_dir_path ), qPrintable( QStringLiteral( "Failed to create manual directory: %1" ).arg( m_manual_dir_path ) ) );

    QFile report( m_report_path );
    QVERIFY2( report.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ), "Failed to initialize TestResult.md" );
    QTextStream stream( &report );
    stream << "# RepetitionLevelControlInSituTest\n\n- 結果: RUNNING\n";
}

void RepetitionLevelControlInSituTest::cleanupTestCase()
{
    writeMarkdownReport();
}

void RepetitionLevelControlInSituTest::performs_repetition_level_control_in_situ_scenario()
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

    QSignalSpy repetition_spy( client.communication, &Communication::receiveRepetitionLevelParameter );
    QSignalSpy request_spy( client.time_step_tool_bar, &TimeStepControlToolBar::requestDataAt );
    QSignalSpy completion_spy( client.time_step_tool_bar, &TimeStepControlToolBar::dataRequestCompleted );
    QSignalSpy latest_step_spy( client.communication, &Communication::updateObjectLatestTimeStep );
    QVERIFY2( repetition_spy.isValid(), "receiveRepetitionLevelParameter QSignalSpy is invalid" );
    QVERIFY2( request_spy.isValid(), "requestDataAt QSignalSpy is invalid" );
    QVERIFY2( completion_spy.isValid(), "dataRequestCompleted QSignalSpy is invalid" );
    QVERIFY2( latest_step_spy.isValid(), "updateObjectLatestTimeStep QSignalSpy is invalid" );
    Spies spies;
    spies.repetition_spy = &repetition_spy;
    spies.request_spy = &request_spy;
    spies.completion_spy = &completion_spy;
    spies.latest_step_spy = &latest_step_spy;

    // ---- Communication.ui ----
    connectClient( client );
    markStep( QStringLiteral( "Communication.ui: connectPushButton を押し、接続状態になった" ) );
    configureInSituVisualization( client );
    markStep( QStringLiteral( "Communication.ui: remoteVizInsituRadioButton を選択し settingApplyPushButton を押した" ) );

    QVERIFY2(
        waitForRepeatLevel( repetition_spy, 0, -1, k_object_load_timeout_ms, &m_initial_level ),
        "Initialize RepeatLevel (Communication::receiveRepetitionLevelParameter) was not received" );
    markStep( QStringLiteral( "Communication::receiveRepetitionLevelParameter(Initialize)を受信し、RepeatLevel = %1 を記録した" ).arg( m_initial_level ) );

    // ---- ObjectEditor.ui ----
    waitForObjectAndApply( client );
    markStep( QStringLiteral( "ObjectEditor.ui: treeView の先頭行を選択し applyPushButton を押した" ) );

    // ---- PlayBackControlToolBar ----
    clickJumpAndWaitForCompletion( client, completion_spy );
    markStep( QStringLiteral( "PlayBackControlToolBar: m_jump_push_button を押し、再度有効になるまで待機した" ) );
    enableKeepLast( client );
    markStep( QStringLiteral( "PlayBackControlToolBar: m_keep_last_push_button を ON にした" ) );
    waitForPositiveParticleCount( client );
    markStep( QStringLiteral( "TotalParticlesToolBar の粒子数が正になった(%1)" ).arg( totalParticles( client ) ) );

    // ---- RepetitionLevelControl.ui(接続直後) ----
    bringRepetitionLevelControlToFront( client.repetition_level_control );
    QVERIFY2( client.repetition_level_control->isVisible(), "RepetitionLevelControl did not become visible" );
    const bool initial_displayed = waitForCondition(
        [client, this]() { return client.current_repetition_level_label->text().trimmed().toInt() == m_initial_level; },
        k_short_wait_ms * 3,
        50 );
    if ( check( initial_displayed, QStringLiteral( "dispCurrentRepetitionLevel(%1)が Initialize の RepeatLevel(%2)と一致しない" ).arg( client.current_repetition_level_label->text() ).arg( m_initial_level ) ) )
    {
        markStep( QStringLiteral( "RepetitionLevelControl.ui: dispCurrentRepetitionLevel が Initialize の RepeatLevel(%1)と一致することを確認した" ).arg( m_initial_level ) );
    }
    if ( check( client.new_repetition_level_spin_box->isEnabled() && client.repetition_apply_button->isEnabled(),
                QStringLiteral( "spinBoxNewRepetitionLevel / applyPushButton が有効ではない(Operator ではない可能性)" ) ) )
    {
        markStep( QStringLiteral( "RepetitionLevelControl.ui: spinBoxNewRepetitionLevel と applyPushButton が有効(Operator)であることを確認した" ) );
    }
    {
        LevelRecord initial;
        initial.label = QStringLiteral( "初期値" );
        initial.level = m_initial_level;
        initial.echoed_level = m_initial_level;
        initial.total_particles = totalParticles( client );
        initial.time_step = latestCompletedTimeStep( spies );
        // 初期値は default.json(存在すれば)の値と particle_limit を記録するだけで判定しない
        initial.default_json_check = QStringLiteral( "default.json: " ) + checkDefaultJsonRepeatLevel( m_initial_level, &m_particle_limit, false );
        m_records.push_back( initial );
        markStep( QStringLiteral( "Total Particles を記録した(初期値 = %1)" ).arg( initial.total_particles ) );
    }
    bringWindowToFront( client.main_window );
    bringRepetitionLevelControlToFront( client.repetition_level_control );
    saveScreenshot(
        1,
        QStringLiteral( "connected" ),
        QStringLiteral( "接続直後(Current Repetition Level にサーバの値が反映)" ),
        QStringLiteral( "Client/RepetitionLevelControl/3_1.png" ),
        latestCompletedTimeStep( spies ) );

    // ---- Repetition Level 1 / 8 / 16 ----
    runLevelCase( client, spies, 1, 2, QStringLiteral( "Client/RepetitionLevelControl/3_2.png" ) );
    runLevelCase( client, spies, 8, 3, QStringLiteral( "Client/RepetitionLevelControl/3_3.png" ) );
    runLevelCase( client, spies, 16, 4, QStringLiteral( "Client/RepetitionLevelControl/3_4.png" ) );

    // ---- 判定 ----
    judgeParticleCounts();

    // ---- 後片付け: Initialize で受信した値に戻す ----
    if ( m_initial_level > 0 )
    {
        int echoed = -1;
        if ( applyRepetitionLevel( client, spies, m_initial_level, &echoed ) )
        {
            markStep( QStringLiteral( "RepetitionLevelControl.ui: spinBoxNewRepetitionLevel を初期値 %1 に戻し applyPushButton を押した(サーバ側 default.json を元に戻す)" ).arg( m_initial_level ) );
        }
    }

    // ---- Communication.ui: 切断 ----
    bringWindowToFront( client.main_window );
    QTest::mouseClick( client.disconnect_button, Qt::LeftButton );
    const bool disconnected = waitForCondition(
        [client]() { return client.connect_button->isEnabled() && !client.disconnect_button->isEnabled(); },
        k_connect_timeout_ms,
        100 );
    if ( check( disconnected, QStringLiteral( "disconnectPushButton を押しても未接続状態にならない" ) ) )
    {
        markStep( QStringLiteral( "Communication.ui: disconnectPushButton を押し、未接続状態になった" ) );
    }

    m_scenario_completed = true;
    m_test_succeeded = m_failures.isEmpty();

    main_window.close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

    QVERIFY2( m_failures.isEmpty(), qPrintable( QStringLiteral( "C-RLC-03 failed: %1" ).arg( m_failures.join( QStringLiteral( " / " ) ) ) ) );
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::RepetitionLevelControlInSituTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
