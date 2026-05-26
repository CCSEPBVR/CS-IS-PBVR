#include "CommunicationUserInfoTest.h"

#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTextBrowser>
#include <QTest>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

#include <csignal>

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
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
}

namespace ClientTests
{

QString CommunicationUserInfoTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString CommunicationUserInfoTest::repoRootPath() const
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

QString CommunicationUserInfoTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool CommunicationUserInfoTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void CommunicationUserInfoTest::startVideoRecording()
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

void CommunicationUserInfoTest::stopVideoRecording()
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
        m_recording_process.terminate();
    }
    if ( m_recording_process.state() != QProcess::NotRunning &&
         !m_recording_process.waitForFinished( 5000 ) )
    {
        m_recording_process.kill();
        m_recording_process.waitForFinished( 5000 );
    }

    QVERIFY2(
        QFileInfo::exists( m_video_file_path ),
        qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
#endif
}

void CommunicationUserInfoTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void CommunicationUserInfoTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

CommunicationUserInfoTest::ClientHandles CommunicationUserInfoTest::resolveClientHandles( MainWindow& window ) const
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

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.transfer_operator_apply_button = handles.communication->findChild<QPushButton*>( "transferOperatorApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.transfer_operator_id_line_edit = handles.communication->findChild<QLineEdit*>( "transferOperatorIDlineEdit" );
    handles.chat_line_edit = handles.communication->findChild<QLineEdit*>( "chatLineEdit" );
    handles.text_browser = handles.communication->findChild<QTextBrowser*>( "textBrowser" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.transfer_operator_apply_button != nullptr, "transferOperatorApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_operator_id_line_edit != nullptr, "transferOperatorIDlineEdit not found" ) ) { return handles; }
    if ( !require( handles.chat_line_edit != nullptr, "chatLineEdit not found" ) ) { return handles; }
    if ( !require( handles.text_browser != nullptr, "textBrowser not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }

    return handles;
}

void CommunicationUserInfoTest::ensureConnected( const ClientHandles& client ) const
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
        return;
    }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        QVERIFY2(
            waitForCondition(
                [client]()
                {
                    return client.connect_button->isEnabled();
                },
                k_connect_timeout_ms,
                100 ),
            "connectPushButton did not become enabled within the timeout" );

        bringWindowToFront( client.main_window );
        QTest::mouseClick( client.connect_button, Qt::LeftButton );

        if ( waitForCondition( is_connected, k_connect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            logStep( QStringLiteral( "ensureConnected: completed" ) );
            return;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    QFAIL( "Client did not enter the connected state after clicking connectPushButton" );
}

void CommunicationUserInfoTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void CommunicationUserInfoTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

void CommunicationUserInfoTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    const QString default_client_executable =
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() );

    m_operator_client_executable = envOrDefault( "PBVR_OPERATOR_CLIENT_EXECUTABLE", default_client_executable );
    m_guest_client_executable = envOrDefault( "PBVR_GUEST_CLIENT_EXECUTABLE", default_client_executable );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_EXECUTABLE", repoRootPath() ) );
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        ClientTests::configuredPath( "SPX_VOLUME_DATA", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "CommunicationUserInfoTest.mov" ) );

    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );

}

void CommunicationUserInfoTest::cleanupTestCase()
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
}

void CommunicationUserInfoTest::performs_communication_user_info_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

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

    operator_client.object_editor->show();
    guest_client.object_editor->show();

    startVideoRecording();
    logStep( QStringLiteral( "scenario: recording started" ) );

    bringWindowToFront( operator_client.main_window );
    QTest::mouseClick( operator_client.connect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    ensureConnected( operator_client );

    bringWindowToFront( guest_client.main_window );
    QTest::mouseClick( guest_client.connect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    ensureConnected( guest_client );

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

    waitForObjectAndApply( operator_client );
    clickJumpAndWaitForCompletion( operator_client );

    QVERIFY2(
        waitForCondition(
            [operator_client, guest_client]()
            {
                return !operator_client.id_line_edit->text().trimmed().isEmpty() &&
                       !guest_client.id_line_edit->text().trimmed().isEmpty() &&
                       operator_client.id_line_edit->text().trimmed() != guest_client.id_line_edit->text().trimmed();
            },
            k_connect_timeout_ms,
            100 ),
        "Operator/guest IDs were not populated within the timeout" );

    setLineEditText( operator_client.transfer_operator_id_line_edit, guest_client.id_line_edit->text().trimmed() );
    QTest::mouseClick( operator_client.transfer_operator_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( operator_client.chat_line_edit, QStringLiteral( "test" ) );
    QTest::keyClick( operator_client.chat_line_edit, Qt::Key_Return );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition(
            [guest_client]()
            {
                return guest_client.text_browser->toPlainText().contains( QStringLiteral( "test" ) );
            },
            k_connect_timeout_ms,
            100 ),
        "Guest chat view did not receive the 'test' message within the timeout" );

    bringWindowToFront( guest_client.main_window );
    stopVideoRecording();
    operator_client.main_window->close();
    guest_client.main_window->close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    logStep( QStringLiteral( "scenario: completed" ) );
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::CommunicationUserInfoTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
