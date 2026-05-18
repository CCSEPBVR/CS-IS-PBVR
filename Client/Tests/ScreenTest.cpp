#include "ScreenTest.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPixmap>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QTextStream>
#include <QTest>
#include <QTreeView>
#include <QWidget>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_button_retry_count = 3;
constexpr int k_button_retry_wait_ms = 1000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_post_connect_settle_ms = 1000;
constexpr int k_screen_settle_ms = 3000;
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
}

QString ScreenTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString ScreenTest::defaultClientExecutablePath() const
{
#if defined( Q_OS_WIN )
    return sourceTreePath( QStringLiteral( "Client/build/Release/pbvr_client.exe" ) );
#elif defined( Q_OS_MACOS )
    return sourceTreePath(
        QStringLiteral( "Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" ) );
#else
    return sourceTreePath( QStringLiteral( "Client/build/pbvr_client" ) );
#endif
}

QString ScreenTest::defaultServerExecutablePath() const
{
#if defined( Q_OS_WIN )
    return sourceTreePath( QStringLiteral( "Server/x64/Release/pbvr_server.exe" ) );
#elif defined( Q_OS_MACOS )
    return sourceTreePath( QStringLiteral( "Server/pbvr_server" ) );
#else
    return sourceTreePath( QStringLiteral( "Server/pbvr_server" ) );
#endif
}

QString ScreenTest::repoRootPath() const
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

QString ScreenTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool ScreenTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void ScreenTest::dragMouse(
    QWidget* widget,
    Qt::MouseButton button,
    const QPoint& start,
    const QPoint& end,
    Qt::KeyboardModifiers modifiers,
    int steps ) const
{
    QVERIFY2( widget != nullptr, "Drag target widget is null" );

    widget->setFocus();
    const QPoint global_start = widget->mapToGlobal( start );
    const QPoint global_end = widget->mapToGlobal( end );

    QMouseEvent press_event(
        QEvent::MouseButtonPress,
        start,
        global_start,
        button,
        button,
        modifiers );
    QVERIFY( QCoreApplication::sendEvent( widget, &press_event ) );

    for ( int i = 1; i <= steps; ++i )
    {
        const qreal t = static_cast<qreal>( i ) / static_cast<qreal>( steps );
        const QPoint pos(
            start.x() + qRound( ( end.x() - start.x() ) * t ),
            start.y() + qRound( ( end.y() - start.y() ) * t ) );
        const QPoint global_pos = widget->mapToGlobal( pos );
        QMouseEvent move_event(
            QEvent::MouseMove,
            pos,
            global_pos,
            Qt::NoButton,
            button,
            modifiers );
        QVERIFY( QCoreApplication::sendEvent( widget, &move_event ) );
        QTest::qWait( 20 );
    }

    QMouseEvent release_event(
        QEvent::MouseButtonRelease,
        end,
        global_end,
        button,
        Qt::NoButton,
        modifiers );
    QVERIFY( QCoreApplication::sendEvent( widget, &release_event ) );
}

void ScreenTest::saveScreenshot( QMainWindow& window, const QString& file_name, const QString& caption ) const
{
    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( file_name );
    const QPixmap screenshot = window.grab();

    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the main window" );
    QVERIFY2(
        screenshot.save( file_path ),
        qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( file_path ) ) );

    const_cast<ScreenTest*>( this )->m_screenshots.append( { file_path, caption } );
}

void ScreenTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# ScreenTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- サーバ起動ラッパー: `"
           << ( QFileInfo::exists( m_server_target_wrapper_executable ) ?
                    m_server_target_wrapper_executable :
                    QStringLiteral( "未使用" ) )
           << "`\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 実施手順\n\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Communication.ui の connectPushButton を押し、サーバへ接続した。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": remoteVizClientServerRadioButton を選択し、volumeDataFilePathLineEdit にボリュームデータを入力して settingApplyPushButton を押した。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": ObjectEditor.ui の nameLineEdit にテキストが入るまで待機し、applyPushButton を押した。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": PlayBackControlToolBar.cpp の m_jump_push_button を押し、再度有効になるまで待機した。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": MainWindow.cpp の m_screen に対して右ドラッグ、左ドラッグ、Shift+左ドラッグ、Homeキー操作を行った。\n\n";

    stream << "## 自動判定項目\n\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 必要なUI部品を取得できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": サーバプロセスを起動し、クライアントが接続状態になること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": ObjectEditor にオブジェクト情報が反映されること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": m_jump_push_button が操作後に再度有効になること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": スクリーンショットを保存できること。\n\n";

    stream << "## 目視確認対象\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "- 要確認: " << entry.caption << "\n";
    }
    if ( m_screenshots.isEmpty() )
    {
        stream << "- NOT RUN: スクリーンショットは保存されていない。\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        const QString file_name = QFileInfo( entry.path ).fileName();
        stream << "### " << entry.caption << "\n\n";
        stream << "![" << entry.caption << "](img/" << file_name << ")\n\n";
    }

    stream << "## 未自動化・保留事項\n\n";
    stream << "- スクリーンショットの描画内容が期待どおりかどうかは目視確認対象とする。\n";
}

void ScreenTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        defaultClientExecutablePath() );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        defaultServerExecutablePath() );
    m_server_target_wrapper_executable = envOrDefault(
        "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE",
        sourceTreePath( QStringLiteral( "server_target_wrapper.sh" ) ) );
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        QStringLiteral( "/User/user/Work/SampleData/ucd/old/out/spx.pfl" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "ScreenTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );

    QVERIFY2(
        QFileInfo::exists( m_client_executable ),
        qPrintable( QStringLiteral( "Client executable not found: %1" ).arg( m_client_executable ) ) );
    QVERIFY2(
        QFileInfo::exists( m_server_executable ),
        qPrintable( QStringLiteral( "Server executable not found: %1" ).arg( m_server_executable ) ) );
    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    if ( QFileInfo::exists( m_server_target_wrapper_executable ) )
    {
        m_server_process.setProgram( m_server_target_wrapper_executable );
        m_server_process.setArguments( { m_server_executable } );
    }
    else
    {
        m_server_process.setProgram( m_server_executable );
    }
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.setProcessChannelMode( QProcess::MergedChannels );
    m_server_process.start();

    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
}

void ScreenTest::cleanupTestCase()
{
    writeMarkdownReport();

    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }
}

void ScreenTest::performs_screen_interaction_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    main_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    auto* communication = main_window.findChild<Communication*>();
    auto* object_editor = main_window.findChild<ObjectEditor*>();
    auto* playback_tool_bar = main_window.findChild<PlayBackControlToolBar*>();
    auto* screen = main_window.findChild<kvs::qt::jaea::Screen*>();

    QVERIFY2( communication != nullptr, "Communication dock not found" );
    QVERIFY2( object_editor != nullptr, "ObjectEditor dock not found" );
    QVERIFY2( playback_tool_bar != nullptr, "Playback toolbar not found" );
    QVERIFY2( screen != nullptr, "m_screen not found" );

    communication->show();
    object_editor->show();
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( 250 );

    auto* connect_button = communication->findChild<QPushButton*>( "connectPushButton" );
    auto* volume_path_edit = communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    auto* remote_viz_radio = communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    auto* setting_apply_button = communication->findChild<QPushButton*>( "settingApplyPushButton" );
    auto* id_line_edit = communication->findChild<QLineEdit*>( "IDLineEdit" );
    auto* disconnect_button = communication->findChild<QPushButton*>( "disconnectPushButton" );

    QVERIFY2( connect_button != nullptr, "connectPushButton not found" );
    QVERIFY2( volume_path_edit != nullptr, "volumeDataFilePathLineEdit not found" );
    QVERIFY2( remote_viz_radio != nullptr, "remoteVizClientServerRadioButton not found" );
    QVERIFY2( setting_apply_button != nullptr, "settingApplyPushButton not found" );
    QVERIFY2( id_line_edit != nullptr, "IDLineEdit not found" );
    QVERIFY2( disconnect_button != nullptr, "disconnectPushButton not found" );

    const auto is_connected = [connect_button, disconnect_button, id_line_edit]()
    {
        return disconnect_button->isEnabled() &&
               !connect_button->isEnabled() &&
               !id_line_edit->text().trimmed().isEmpty();
    };

    bool connected = is_connected();
    for ( int attempt = 0; attempt < k_button_retry_count && !connected; ++attempt )
    {
        QVERIFY2(
            waitForCondition(
                [connect_button]()
                {
                    return connect_button->isEnabled();
                },
                k_connect_timeout_ms,
                100 ),
            "connectPushButton did not become enabled within the timeout" );

        connect_button->click();
        connected = waitForCondition(
            is_connected,
            k_connect_timeout_ms,
            100 );

        if ( !connected ) { QTest::qWait( k_button_retry_wait_ms ); }
    }

    QVERIFY2( connected, "Client did not enter the connected state" );
    QTest::qWait( k_post_connect_settle_ms );

    const bool user_id_received = waitForCondition(
        [id_line_edit]()
        {
            return !id_line_edit->text().trimmed().isEmpty();
        },
        k_connect_timeout_ms,
        100 );
    QVERIFY2( user_id_received, "IDLineEdit was not populated within the timeout" );

    if ( !remote_viz_radio->isChecked() )
    {
        QTest::mouseClick( remote_viz_radio, Qt::LeftButton );
    }
    QVERIFY( remote_viz_radio->isChecked() );

    volume_path_edit->setFocus();
    volume_path_edit->clear();
    QTest::keyClicks( volume_path_edit, QDir::toNativeSeparators( m_volume_data_path ) );
    QCOMPARE( volume_path_edit->text(), QDir::toNativeSeparators( m_volume_data_path ) );

    QTest::mouseClick( setting_apply_button, Qt::LeftButton );

    auto* tree_view = object_editor->findChild<QTreeView*>( "treeView" );
    auto* name_line_edit = object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    auto* apply_button = object_editor->findChild<QPushButton*>( "applyPushButton" );

    QVERIFY2( tree_view != nullptr, "ObjectEditor treeView not found" );
    QVERIFY2( name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" );
    QVERIFY2( apply_button != nullptr, "ObjectEditor applyPushButton not found" );
    QVERIFY2( tree_view->model() != nullptr, "ObjectEditor model not found" );

    const bool has_objects = waitForCondition(
        [tree_view]()
        {
            return tree_view->model() != nullptr && tree_view->model()->rowCount() > 0;
        },
        k_object_load_timeout_ms,
        200 );
    QVERIFY2( has_objects, "ObjectEditor did not receive any rows within the timeout" );

    const QModelIndex first_row = tree_view->model()->index( 0, 0 );
    QVERIFY( first_row.isValid() );

    tree_view->scrollTo( first_row );
    tree_view->setCurrentIndex( first_row );
    QTest::mouseClick(
        tree_view->viewport(),
        Qt::LeftButton,
        Qt::NoModifier,
        tree_view->visualRect( first_row ).center() );

    const bool name_loaded = waitForCondition(
        [name_line_edit]()
        {
            return !name_line_edit->text().trimmed().isEmpty();
        },
        k_object_load_timeout_ms,
        100 );
    QVERIFY2( name_loaded, "ObjectEditor nameLineEdit was not populated within the timeout" );

    QTest::mouseClick( apply_button, Qt::LeftButton );

    auto* jump_button = playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    QVERIFY2( jump_button != nullptr, "m_jump_push_button not found" );
    QTest::mouseClick( jump_button, Qt::LeftButton );

    const bool jump_enabled = waitForCondition(
        [jump_button]()
        {
            return jump_button->isEnabled();
        },
        k_jump_button_enable_timeout_ms,
        200 );
    QVERIFY2( jump_enabled, "m_jump_push_button did not become enabled within the timeout" );

    screen->raise();
    screen->activateWindow();
    screen->setFocus();

    const QPoint center = screen->rect().center();
    dragMouse( screen, Qt::RightButton, center, center + QPoint( 120, -80 ) );
    QTest::qWait( k_screen_settle_ms );
    saveScreenshot(
        main_window,
        QStringLiteral( "01_screen_after_right_drag.png" ),
        QStringLiteral( "m_screenを右クリックでドラッグしたことを表す状態" ) );

    dragMouse( screen, Qt::LeftButton, center, center + QPoint( -100, 90 ) );
    QTest::qWait( k_screen_settle_ms );
    saveScreenshot(
        main_window,
        QStringLiteral( "02_screen_after_left_drag.png" ),
        QStringLiteral( "m_screenを左クリックでドラッグしたことを表す状態" ) );

    QTest::keyPress( screen, Qt::Key_Shift );
    dragMouse( screen, Qt::LeftButton, center, center + QPoint( 180, 160 ), Qt::ShiftModifier );
    QTest::keyRelease( screen, Qt::Key_Shift );
    QTest::qWait( k_screen_settle_ms );
    saveScreenshot(
        main_window,
        QStringLiteral( "03_screen_after_shift_left_drag.png" ),
        QStringLiteral( "m_screenをShiftキーを押しながら左クリックでドラッグしたことを表す状態" ) );

    QTest::keyClick( screen, Qt::Key_Home );
    QTest::qWait( 500 );
    saveScreenshot(
        main_window,
        QStringLiteral( "04_screen_after_home.png" ),
        QStringLiteral( "Homeボタンを押したことを表す状態" ) );

    m_test_succeeded = true;
}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ScreenTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
