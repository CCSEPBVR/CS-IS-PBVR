#include "RepetitionLevelControlTest.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QTextStream>
#include <QTest>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/RepetitionLevelControl.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_after_jump_wait_ms = 3000;
constexpr int k_after_repetition_apply_wait_ms = 3000;
constexpr int k_capture_settle_ms = 300;
kvs::qt::Application* g_test_app = nullptr;

void logStep( const QString& message )
{
    qInfo().noquote() << message;
}

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
}

namespace ClientTests
{

QString RepetitionLevelControlTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString RepetitionLevelControlTest::repoRootPath() const
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

QString RepetitionLevelControlTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool RepetitionLevelControlTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void RepetitionLevelControlTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void RepetitionLevelControlTest::bringRepetitionLevelControlToFront( RepetitionLevelControl* control ) const
{
    QVERIFY2( control != nullptr, "RepetitionLevelControl is null" );
    control->show();
    control->raise();
    control->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void RepetitionLevelControlTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void RepetitionLevelControlTest::saveScreenshot( const QString& file_name, const QString& caption )
{
    if ( !ClientTests::screenshotsEnabled() ) { return; }

    QTest::qWait( k_capture_settle_ms );

    QScreen* screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen != nullptr, "Primary screen not found" );

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );

    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the primary screen" );
    QVERIFY2(
        screenshot.save( file_path ),
        qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( file_path ) ) );

    m_screenshots.push_back( { file_name, caption } );
}

void RepetitionLevelControlTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# RepetitionLevelControlTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- サーバターゲットラッパー: `" << m_server_target_wrapper_executable << "`\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    stream << "\n## 自動判定項目\n\n";
    if ( m_test_succeeded )
    {
        stream << "- PASS: Communication.ui の connectPushButton、remoteVizClientServerRadioButton、volumeDataFilePathLineEdit、settingApplyPushButton を取得できた。\n";
        stream << "- PASS: ObjectEditor.ui の nameLineEdit と applyPushButton を取得し、nameLineEdit にテキストが入るまで待機できた。\n";
        stream << "- PASS: PlayBackControlToolBar.cpp の m_jump_push_button を取得し、クリック後に再度有効になるまで待機できた。\n";
        stream << "- PASS: RepetitionLevelControl.ui の spinBoxNewRepetitionLevel と applyPushButton を取得し、1、8、16 を設定できた。\n";
        stream << "- PASS: Markdown レポートとスクリーンショットを出力できた。\n";
    }
    else
    {
        stream << "- FAIL: テストが最後まで完了しなかった。QtTest の失敗箇所を確認する。\n";
    }

    stream << "\n## 目視確認対象\n\n";
    stream << "- 要確認: RepetitionLevelControl.ui を開いた時に、現在設定されている Repetition Level が表示されていること。\n";
    stream << "- 要確認: Repetition Level を 1 に設定した時のオブジェクト表示。\n";
    stream << "- 要確認: Repetition Level を 8 に設定した時のオブジェクト表示。\n";
    stream << "- 要確認: Repetition Level を 16 に設定した時のオブジェクト表示。\n";
    stream << "- 注意: スクリーンショットの視覚的な正しさは自動判定していない。\n";

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.caption << "\n\n";
        stream << "!["
               << entry.caption
               << "](./img/"
               << entry.file_name
               << ")\n\n";
    }

    stream << "## 未自動化・保留事項\n\n";
    stream << "- 3D 表示の見た目、粒子密度、描画差分の妥当性は目視確認する。\n";
}

void RepetitionLevelControlTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

RepetitionLevelControlTest::ClientHandles RepetitionLevelControlTest::resolveClientHandles( MainWindow& window ) const
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
    handles.repetition_level_control = window.findChild<RepetitionLevelControl*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.repetition_level_control != nullptr, "RepetitionLevelControl dock not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.repetition_apply_button = handles.repetition_level_control->findChild<QPushButton*>( "applyPushButton" );
    handles.new_repetition_level_spin_box =
        handles.repetition_level_control->findChild<QSpinBox*>( "spinBoxNewRepetitionLevel" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.repetition_apply_button != nullptr, "RepetitionLevelControl applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.new_repetition_level_spin_box != nullptr, "spinBoxNewRepetitionLevel not found" ) ) { return handles; }

    return handles;
}

void RepetitionLevelControlTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.connect_button->isEnabled();
            },
            k_connect_timeout_ms,
            100 ),
        "connectPushButton did not become enabled within the timeout" );

    QTest::mouseClick( client.connect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

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

void RepetitionLevelControlTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_client_server_radio->isChecked(), "remoteVizClientServerRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void RepetitionLevelControlTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_apply_button->isEnabled() &&
                       !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor nameLineEdit was not populated within the timeout" );

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void RepetitionLevelControlTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.jump_button->isEnabled();
            },
            k_jump_button_enable_timeout_ms,
            200 ),
        "m_jump_push_button did not become enabled within the timeout" );

    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.jump_button->isEnabled();
            },
            k_jump_button_enable_timeout_ms,
            200 ),
        "m_jump_push_button did not become enabled again within the timeout" );

    QTest::qWait( k_after_jump_wait_ms );
}

void RepetitionLevelControlTest::openRepetitionLevelControl( const ClientHandles& client ) const
{
    bringRepetitionLevelControlToFront( client.repetition_level_control );
    QVERIFY2( client.repetition_level_control->isVisible(), "RepetitionLevelControl did not become visible" );
    QVERIFY2( client.repetition_apply_button->isEnabled(), "RepetitionLevelControl applyPushButton is disabled" );
}

void RepetitionLevelControlTest::applyRepetitionLevel( const ClientHandles& client, int repetition_level ) const
{
    openRepetitionLevelControl( client );
    QVERIFY2(
        repetition_level >= client.new_repetition_level_spin_box->minimum() &&
            repetition_level <= client.new_repetition_level_spin_box->maximum(),
        qPrintable( QStringLiteral( "Repetition Level is out of spin box range: %1" ).arg( repetition_level ) ) );

    client.new_repetition_level_spin_box->setValue( repetition_level );
    QCOMPARE( client.new_repetition_level_spin_box->value(), repetition_level );
    QTest::mouseClick( client.repetition_apply_button, Qt::LeftButton );
    QTest::qWait( k_after_repetition_apply_wait_ms );
}

void RepetitionLevelControlTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() ) );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_EXECUTABLE", repoRootPath() ) );
    m_server_target_wrapper_executable = envOrDefault(
        "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE", repoRootPath() ) );
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        ClientTests::configuredPath( "SPX_VOLUME_DATA", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "RepetitionLevelControlTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

}

void RepetitionLevelControlTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void RepetitionLevelControlTest::performs_repetition_level_control_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();

    connectClient( client );
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButton を押した。" ) );

    configureRemoteVisualization( client );
    markStepCompleted( QStringLiteral( "Communication.ui: remoteVizClientServerRadioButton を押し、volumeDataFilePathLineEdit に TestPathConfig.ini の SPX_VOLUME_DATA を設定し、settingApplyPushButton を押した。" ) );

    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: nameLineEdit にテキストが入るまで待機し、applyPushButton を押した。" ) );

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_button を押し、再度有効になるまで待機した。" ) );

    openRepetitionLevelControl( client );
    markStepCompleted( QStringLiteral( "RepetitionLevelControl.ui を開いた。" ) );

    saveScreenshot(
        QStringLiteral( "01_current_repetition_level_display.png" ),
        QStringLiteral( "現在設定されている Repetition Level が表示されていること" ) );
    markStepCompleted( QStringLiteral( "現在設定されている Repetition Level が表示されていることをスクリーンショット撮影した。" ) );

    applyRepetitionLevel( client, 1 );
    markStepCompleted( QStringLiteral( "RepetitionLevelControl.ui: applyPushButton を押した。" ) );

    saveScreenshot(
        QStringLiteral( "02_repetition_level_1.png" ),
        QStringLiteral( "Repetition Level を 1 に設定した時のオブジェクト" ) );
    markStepCompleted( QStringLiteral( "Repetition Level を 1 に設定した時のオブジェクトをスクリーンショット撮影した。" ) );

    applyRepetitionLevel( client, 8 );
    markStepCompleted( QStringLiteral( "RepetitionLevelControl.ui: spinBoxNewRepetitionLevel を 8 に設定し、applyPushButton を押した。" ) );

    saveScreenshot(
        QStringLiteral( "03_repetition_level_8.png" ),
        QStringLiteral( "Repetition Level を 8 に設定した時のオブジェクト" ) );
    markStepCompleted( QStringLiteral( "Repetition Level を 8 に設定した時のオブジェクトをスクリーンショット撮影した。" ) );

    applyRepetitionLevel( client, 16 );
    markStepCompleted( QStringLiteral( "RepetitionLevelControl.ui: spinBoxNewRepetitionLevel を 16 に設定し、applyPushButton を押した。" ) );

    saveScreenshot(
        QStringLiteral( "04_repetition_level_16.png" ),
        QStringLiteral( "Repetition Level を 16 に設定した時のオブジェクト" ) );
    markStepCompleted( QStringLiteral( "Repetition Level を 16 に設定した時のオブジェクトをスクリーンショット撮影した。" ) );

    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::RepetitionLevelControlTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
