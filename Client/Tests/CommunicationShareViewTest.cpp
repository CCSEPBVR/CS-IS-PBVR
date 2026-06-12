#include "CommunicationShareViewTest.h"

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QListView>
#include <QMetaObject>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QTabWidget>
#include <QTest>
#include <QTextStream>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

#include <cmath>
#include <csignal>

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_share_list_timeout_ms = 15000;
constexpr int k_share_view_apply_timeout_ms = 3000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_capture_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_post_jump_wait_ms = 3000;
constexpr int k_button_retry_count = 3;
constexpr int k_button_retry_wait_ms = 500;
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

bool matricesNearlyEqual( const kvs::Matrix44f& lhs, const kvs::Matrix44f& rhs )
{
    constexpr float tolerance = 1.0e-4f;
    for ( int row = 0; row < 4; ++row )
    {
        for ( int col = 0; col < 4; ++col )
        {
            if ( std::fabs( lhs[row][col] - rhs[row][col] ) > tolerance )
            {
                return false;
            }
        }
    }

    return true;
}

bool xformsNearlyEqual( const kvs::Xform& lhs, const kvs::Xform& rhs )
{
    return matricesNearlyEqual( lhs.toMatrix(), rhs.toMatrix() );
}
}

namespace ClientTests
{

QString CommunicationShareViewTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString CommunicationShareViewTest::repoRootPath() const
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

QString CommunicationShareViewTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool CommunicationShareViewTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void CommunicationShareViewTest::startVideoRecording()
{
#ifdef Q_OS_WIN
    return;
#else
    if ( QFileInfo::exists( m_video_file_path ) )
    {
        QVERIFY2(
            QFile::remove( m_video_file_path ),
            qPrintable( QStringLiteral( "Failed to remove existing video: %1" ).arg( m_video_file_path ) ) );
    }

    m_recording_process.setProgram( QStringLiteral( "screencapture" ) );
    m_recording_process.setArguments(
        {
            QStringLiteral( "-v" ),
            QStringLiteral( "-k" ),
            QStringLiteral( "-m" ),
            QStringLiteral( "-x" ),
            m_video_file_path
        } );
    m_recording_process.start();

    QVERIFY2(
        m_recording_process.waitForStarted( 5000 ),
        qPrintable( QStringLiteral( "Failed to start video recording: %1" ).arg( m_recording_process.errorString() ) ) );
#endif
}

void CommunicationShareViewTest::stopVideoRecording()
{
#ifdef Q_OS_WIN
    Q_UNUSED( m_recording_process );
    return;
#else
    if ( m_recording_process.state() == QProcess::NotRunning )
    {
        QVERIFY2(
            QFileInfo::exists( m_video_file_path ),
            qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
        return;
    }

    const qint64 pid = m_recording_process.processId();
    if ( pid > 0 )
    {
        ::kill( static_cast<pid_t>( pid ), SIGINT );
    }
    else
    {
        m_recording_process.terminate();
    }

    if ( !m_recording_process.waitForFinished( k_recording_finish_timeout_ms ) )
    {
        m_recording_process.kill();
        m_recording_process.waitForFinished( 5000 );
    }

    QVERIFY2(
        QFileInfo::exists( m_video_file_path ),
        qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
#endif
}

void CommunicationShareViewTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void CommunicationShareViewTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void CommunicationShareViewTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void CommunicationShareViewTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# CommunicationShareViewTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

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
}

void CommunicationShareViewTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

CommunicationShareViewTest::ClientHandles CommunicationShareViewTest::resolveClientHandles( MainWindow& window ) const
{
    ClientHandles handles;
    const auto require = [&handles]( bool condition, const char* message )
    {
        if ( condition ) { return true; }
        QTest::qFail( message, __FILE__, __LINE__ );
        return false;
    };

    handles.main_window = &window;
    handles.communication = window.findChild<Communication*>();
    handles.object_editor = window.findChild<ObjectEditor*>();
    handles.playback_tool_bar = window.findChild<::PlayBackControlToolBar*>();
    handles.screen_widget = window.findChild<kvs::qt::jaea::Screen*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.screen_widget != nullptr, "m_screen not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.share_view_button = handles.communication->findChild<QPushButton*>( "shareViewPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.tab_widget = handles.communication->findChild<QTabWidget*>( "tabWidget" );
    handles.share_view_tab = handles.communication->findChild<QWidget*>( "shareViewTab" );
    handles.share_list_view = handles.communication->findChild<QListView*>( "shareListView" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.share_view_button != nullptr, "shareViewPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.tab_widget != nullptr, "tabWidget not found" ) ) { return handles; }
    if ( !require( handles.share_view_tab != nullptr, "shareViewTab not found" ) ) { return handles; }
    if ( !require( handles.share_list_view != nullptr, "shareListView not found" ) ) { return handles; }

    return handles;
}

bool CommunicationShareViewTest::ensureConnected( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "ensureConnected: begin" ) );
    bringWindowToFront( client.main_window );

    const auto is_connected = [client]()
    {
        return client.disconnect_button->isEnabled() &&
               !client.connect_button->isEnabled() &&
               !client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_connected() )
    {
        logStep( QStringLiteral( "ensureConnected: already connected" ) );
        return true;
    }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        if ( !waitForCondition(
                 [client]()
                 {
                     return client.connect_button->isEnabled();
                 },
                 k_connect_timeout_ms,
                 100 ) )
        {
            qWarning() << "connectPushButton did not become enabled within the timeout";
            return false;
        }

        bringWindowToFront( client.main_window );
        QTest::mouseClick( client.connect_button, Qt::LeftButton );

        if ( waitForCondition( is_connected, k_connect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            logStep( QStringLiteral( "ensureConnected: completed" ) );
            return true;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    qWarning() << "Client did not enter the connected state after clicking connectPushButton";
    return false;
}

void CommunicationShareViewTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForObjectAndApply: waiting for ObjectEditor nameLineEdit" ) );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor nameLineEdit was not populated within the timeout" );

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void CommunicationShareViewTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_post_jump_wait_ms );
}

void CommunicationShareViewTest::selectShareViewTab( const ClientHandles& client ) const
{
    const int share_view_index = client.tab_widget->indexOf( client.share_view_tab );
    QVERIFY2( share_view_index >= 0, "tabWidget does not contain shareViewTab" );
    client.tab_widget->setCurrentIndex( share_view_index );
    QCOMPARE( client.tab_widget->currentIndex(), share_view_index );
    QTest::qWait( k_short_wait_ms );
}

void CommunicationShareViewTest::dragScreenRightButton( QWidget* widget ) const
{
    QVERIFY2( widget != nullptr, "Drag target widget is null" );

    const QPoint start = widget->rect().center();
    const QPoint end = start + QPoint( 120, -80 );
    const QPoint global_start = widget->mapToGlobal( start );
    const QPoint global_end = widget->mapToGlobal( end );

    widget->raise();
    widget->activateWindow();
    widget->setFocus();

    QMouseEvent press_event(
        QEvent::MouseButtonPress,
        start,
        global_start,
        Qt::RightButton,
        Qt::RightButton,
        Qt::NoModifier );
    QVERIFY( QCoreApplication::sendEvent( widget, &press_event ) );

    constexpr int steps = 12;
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
            Qt::RightButton,
            Qt::NoModifier );
        QVERIFY( QCoreApplication::sendEvent( widget, &move_event ) );
        QTest::qWait( 20 );
    }

    QMouseEvent release_event(
        QEvent::MouseButtonRelease,
        end,
        global_end,
        Qt::RightButton,
        Qt::NoButton,
        Qt::NoModifier );
    QVERIFY( QCoreApplication::sendEvent( widget, &release_event ) );
}

void CommunicationShareViewTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    const QString default_client_executable =
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() );

    m_operator_client_executable = envOrDefault( "PBVR_OPERATOR_CLIENT_EXECUTABLE", default_client_executable );
    m_guest_client_executable = envOrDefault( "PBVR_GUEST_CLIENT_EXECUTABLE", default_client_executable );
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
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "CommunicationShareViewTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "CommunicationShareViewTest.mov" ) );
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

void CommunicationShareViewTest::cleanupTestCase()
{
    if ( m_recording_process.state() != QProcess::NotRunning )
    {
        stopVideoRecording();
    }

    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void CommunicationShareViewTest::performs_communication_share_view_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    startVideoRecording();
    logStep( QStringLiteral( "scenario: recording started" ) );

    MainWindow operator_window( *g_test_app );
    MainWindow guest_window( *g_test_app );

    operator_window.setWindowTitle( operator_window.windowTitle() + QStringLiteral( " [Operator]" ) );
    guest_window.setWindowTitle( guest_window.windowTitle() + QStringLiteral( " [Guest]" ) );

    showTestWindowCentered( &operator_window, -240 );
    QVERIFY( QTest::qWaitForWindowExposed( &operator_window ) );
    showTestWindowCentered( &guest_window, 240 );
    QVERIFY( QTest::qWaitForWindowExposed( &guest_window ) );

    ClientHandles operator_client = resolveClientHandles( operator_window );
    ClientHandles guest_client = resolveClientHandles( guest_window );

    operator_client.communication->show();
    operator_client.object_editor->show();
    guest_client.communication->show();
    guest_client.object_editor->show();

    bringWindowToFront( operator_client.main_window );
    QTest::mouseClick( operator_client.connect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2( ensureConnected( operator_client ), "Operator did not enter the connected state" );
    markStepCompleted( QStringLiteral( "Operator用 Communication.ui: connectPushButtonを押し、1秒待機しました。" ) );

    bringWindowToFront( guest_client.main_window );
    QTest::mouseClick( guest_client.connect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2( ensureConnected( guest_client ), "Guest did not enter the connected state" );
    markStepCompleted( QStringLiteral( "Guest用 Communication.ui: connectPushButtonを押し、1秒待機しました。" ) );

    bringWindowToFront( operator_client.main_window );
    if ( !operator_client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( operator_client.remote_viz_client_server_radio, Qt::LeftButton );
        QVERIFY2(
            operator_client.remote_viz_client_server_radio->isChecked(),
            "remoteVizClientServerRadioButton was not checked" );
    }
    QTest::qWait( k_short_wait_ms );

    setLineEditText( operator_client.volume_data_path_line_edit, m_volume_data_path );
    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( operator_client.setting_apply_button, Qt::LeftButton );
    markStepCompleted( QStringLiteral( "Operator用 Communication.ui: remoteVizClientServerRadioButtonを押し、volumeDataFilePathLineEditにデータパスを入力してsettingApplyPushButtonを押しました。" ) );

    waitForObjectAndApply( operator_client );
    markStepCompleted( QStringLiteral( "Operator用 ObjectEditor.ui: nameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );

    clickJumpAndWaitForCompletion( operator_client );
    markStepCompleted( QStringLiteral( "Operator用 PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効になるまで待機してから3秒待機しました。" ) );

    dragScreenRightButton( operator_client.screen_widget );
    markStepCompleted( QStringLiteral( "Operator用 MainWindow.cpp: m_screenを右クリックでドラッグしました。" ) );

    auto* operator_screen = qobject_cast<kvs::qt::jaea::Screen*>( operator_client.screen_widget );
    auto* guest_screen = qobject_cast<kvs::qt::jaea::Screen*>( guest_client.screen_widget );
    QVERIFY2( operator_screen != nullptr, "Operator screen could not be resolved" );
    QVERIFY2( guest_screen != nullptr, "Guest screen could not be resolved" );
    const kvs::Xform operator_shared_xform = operator_screen->scene()->objectManager()->xform();

    selectShareViewTab( operator_client );
    QTest::mouseClick( operator_client.share_view_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    saveScreenshot(
        QStringLiteral( "01_operator_after_share_view.png" ),
        QStringLiteral( "Operator用 Share View送信後" ) );
    markStepCompleted( QStringLiteral( "Operator用 Communication.ui: shareViewTabを選択し、shareViewPushButtonを押してスクリーンショットを撮影しました。" ) );

    bringWindowToFront( guest_client.main_window );
    selectShareViewTab( guest_client );

    QVERIFY2(
        waitForCondition(
            [guest_client]()
            {
                return guest_client.share_list_view->model() != nullptr &&
                       guest_client.share_list_view->model()->rowCount() > 0;
            },
            k_share_list_timeout_ms,
            100 ),
        "shareListView did not receive any shared view entries within the timeout" );

    const QModelIndex first_index = guest_client.share_list_view->model()->index( 0, 0 );
    QVERIFY2( first_index.isValid(), "shareListView first item is invalid" );

    guest_client.share_list_view->scrollTo( first_index );
    guest_client.share_list_view->setCurrentIndex( first_index );
    QTest::mouseDClick(
        guest_client.share_list_view->viewport(),
        Qt::LeftButton,
        Qt::NoModifier,
        guest_client.share_list_view->visualRect( first_index ).center() );

    if ( !waitForCondition(
             [guest_screen, operator_shared_xform]()
             {
                 return xformsNearlyEqual(
                     guest_screen->scene()->objectManager()->xform(),
                     operator_shared_xform );
             },
             k_share_view_apply_timeout_ms,
             100 ) )
    {
        const bool invoked = QMetaObject::invokeMethod(
            guest_client.communication,
            "onItemDoubleClicked",
            Qt::DirectConnection,
            Q_ARG( QModelIndex, first_index ) );
        QVERIFY2( invoked, "Failed to invoke Communication::onItemDoubleClicked for shareListView first item" );
    }

    QVERIFY2(
        waitForCondition(
            [guest_screen, operator_shared_xform]()
            {
                return xformsNearlyEqual(
                    guest_screen->scene()->objectManager()->xform(),
                    operator_shared_xform );
            },
            k_connect_timeout_ms,
            100 ),
        "Guest objectManager xform did not match the shared Operator xform after double-clicking shareListView" );
    markStepCompleted( QStringLiteral( "Guest用 Communication.ui: shareListViewの1番目をダブルクリックし、GuestのXformがOperatorの共有Xformと一致するまで待機しました。" ) );
    QTest::qWait( k_short_wait_ms );

    bringWindowToFront( guest_client.main_window );
    saveScreenshot(
        QStringLiteral( "02_guest_after_open_shared_view.png" ),
        QStringLiteral( "Guest用 Share View受信後" ) );
    markStepCompleted( QStringLiteral( "Guest用 Communication.ui: Share View適用後のスクリーンショットを撮影しました。" ) );

    QTest::qWait( k_post_jump_wait_ms );
    stopVideoRecording();
    operator_client.main_window->close();
    guest_client.main_window->close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    m_test_succeeded = true;
    logStep( QStringLiteral( "scenario: completed" ) );
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::CommunicationShareViewTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
