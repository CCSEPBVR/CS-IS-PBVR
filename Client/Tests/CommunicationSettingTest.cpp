#include "CommunicationSettingTest.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTextStream>
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
constexpr int k_disconnect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_server_message_timeout_ms = 120000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_post_apply_settle_ms = 1000;
constexpr int k_jump_finish_wait_ms = 5000;
constexpr int k_button_retry_count = 3;
constexpr int k_button_retry_wait_ms = 500;
constexpr int k_radio_retry_count = 3;
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

QString CommunicationSettingTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString CommunicationSettingTest::repoRootPath() const
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

QString CommunicationSettingTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool CommunicationSettingTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void CommunicationSettingTest::startVideoRecording()
{
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
}

void CommunicationSettingTest::stopVideoRecording()
{
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
}

void CommunicationSettingTest::appendServerOutput()
{
    if ( m_server_process.bytesAvailable() > 0 || m_server_process.waitForReadyRead( 10 ) )
    {
        m_server_output_buffer.append( QString::fromLocal8Bit( m_server_process.readAll() ) );
    }
}

void CommunicationSettingTest::clearServerOutput()
{
    appendServerOutput();
    m_server_output_buffer.clear();
}

void CommunicationSettingTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void CommunicationSettingTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

CommunicationSettingTest::ClientHandles CommunicationSettingTest::resolveClientHandles( MainWindow& window ) const
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
    handles.local_viz_radio = handles.communication->findChild<QRadioButton*>( "localVizRadioButton" );
    handles.uniform_radio = handles.communication->findChild<QRadioButton*>( "uniformRadioButton" );
    handles.metropolis_radio = handles.communication->findChild<QRadioButton*>( "metropolisRadioButton" );
    handles.rejection_radio = handles.communication->findChild<QRadioButton*>( "rejectionRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.transfer_function_path_line_edit = handles.communication->findChild<QLineEdit*>( "transferFunctionFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.local_viz_radio != nullptr, "localVizRadioButton not found" ) ) { return handles; }
    if ( !require( handles.uniform_radio != nullptr, "uniformRadioButton not found" ) ) { return handles; }
    if ( !require( handles.metropolis_radio != nullptr, "metropolisRadioButton not found" ) ) { return handles; }
    if ( !require( handles.rejection_radio != nullptr, "rejectionRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_path_line_edit != nullptr, "transferFunctionFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }

    return handles;
}

void CommunicationSettingTest::ensureConnected( const ClientHandles& client ) const
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

void CommunicationSettingTest::ensureDisconnected( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "ensureDisconnected: begin" ) );
    bringWindowToFront( client.main_window );

    const auto is_disconnected = [client]()
    {
        return client.connect_button->isEnabled() &&
               !client.disconnect_button->isEnabled() &&
               client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_disconnected() )
    {
        logStep( QStringLiteral( "ensureDisconnected: already disconnected" ) );
        return;
    }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        QVERIFY2(
            waitForCondition(
                [client]()
                {
                    return client.disconnect_button->isEnabled();
                },
                k_disconnect_timeout_ms,
                100 ),
            "disconnectPushButton did not become enabled within the timeout" );

        bringWindowToFront( client.main_window );
        QTest::mouseClick( client.disconnect_button, Qt::LeftButton );

        if ( waitForCondition( is_disconnected, k_disconnect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            logStep( QStringLiteral( "ensureDisconnected: completed" ) );
            return;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    QFAIL( "Client did not enter the disconnected state after clicking disconnectPushButton" );
}

void CommunicationSettingTest::selectRadioButton( QRadioButton* radio_button, const char* object_name ) const
{
    QVERIFY2( radio_button != nullptr, "Sampling radio button was not found" );

    if ( radio_button->isChecked() ) { return; }

    QVERIFY2(
        waitForCondition(
            [radio_button]()
            {
                return radio_button->isEnabled() && radio_button->isVisible();
            },
            k_connect_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "%1 did not become clickable within the timeout" ).arg( object_name ) ) );

    for ( int attempt = 0; attempt < k_radio_retry_count; ++attempt )
    {
        QTest::mouseClick( radio_button, Qt::LeftButton );
        if ( radio_button->isChecked() ) { return; }

        QTest::qWait( k_button_retry_wait_ms );
    }

    radio_button->click();
    QVERIFY2(
        radio_button->isChecked(),
        qPrintable( QStringLiteral( "%1 was not checked" ).arg( object_name ) ) );
}

void CommunicationSettingTest::configureLocalSampling( const ClientHandles& client, QRadioButton* sampling_radio ) const
{
    QVERIFY2( sampling_radio != nullptr, "Sampling radio button was not found" );

    ensureConnected( client );

    bringWindowToFront( client.main_window );
    selectRadioButton( client.local_viz_radio, "localVizRadioButton" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::qWait( k_short_wait_ms );
    setLineEditText( client.transfer_function_path_line_edit, m_transfer_function_path );
    QTest::qWait( k_short_wait_ms );

    selectRadioButton( sampling_radio, sampling_radio->objectName().toUtf8().constData() );
    QTest::qWait( k_short_wait_ms );

    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_post_apply_settle_ms );
}

void CommunicationSettingTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void CommunicationSettingTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_jump_finish_wait_ms );
}

void CommunicationSettingTest::waitForServerSamplingMessage( const QString& text )
{
    QVERIFY2(
        waitForCondition(
            [this, text]()
            {
                appendServerOutput();
                return m_server_output_buffer.contains( text, Qt::CaseSensitive );
            },
            k_server_message_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "Server output did not contain '%1'. Current output: %2" )
                        .arg( text, m_server_output_buffer ) ) );

    markSamplingCheckVerified( text );
}

void CommunicationSettingTest::markSamplingCheckVerified( const QString& text )
{
    for ( SamplingCheckEntry& entry : m_sampling_checks )
    {
        if ( entry.expected_message == text )
        {
            entry.verified = true;
            return;
        }
    }
}

void CommunicationSettingTest::writeSummaryReport() const
{
    QFile report_file( m_summary_file_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open summary report: %1" ).arg( m_summary_file_path ) ) );

    QTextStream stream( &report_file );
    stream << "# CommunicationSettingTest\n\n";
    stream << "- Result: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- Client executable: `" << m_client_executable << "`\n";
    stream << "- Server executable: `" << m_server_executable << "`\n";
    stream << "- Volume data: `" << m_volume_data_path << "`\n";
    stream << "- Transfer function: `" << m_transfer_function_path << "`\n";
    stream << "- Video: `" << m_video_file_path << "`\n";
    stream << "- Output directory: `" << m_output_dir_path << "`\n\n";
    stream << "## Server Sampling Checks\n\n";

    for ( const SamplingCheckEntry& entry : m_sampling_checks )
    {
        stream << "- " << entry.label << ": " << ( entry.verified ? "PASS" : "NOT VERIFIED" ) << "\n";
        stream << "  Expected message: `" << entry.expected_message << "`\n";
    }

    stream << "\n## Notes\n\n";
    stream << "- This report records whether CommunicationSettingTest confirmed each expected server output string.\n";
    stream << "- If an item is `NOT VERIFIED`, the scenario stopped before that server message was confirmed.\n";
}

void CommunicationSettingTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" ) );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Server/pbvr_server" ) );
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        QStringLiteral( "/path/to/reg_test_data/unstruct/mej_iofiles_downsize4_step80_90/Piece/example.pfl" ) );
    m_transfer_function_path = envOrDefault(
        "PBVR_TRANSFER_FUNCTION",
        QStringLiteral( "/path/to/reg_test_data/unstruct/mej_v2.tfe" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "CommunicationSettingTest.mov" ) );
    m_summary_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "CommunicationSettingTest.md" ) );
    m_sampling_checks = {
        { QStringLiteral( "Uniform sampling" ), QStringLiteral( "GPU - Uniform sampling" ), false },
        { QStringLiteral( "Metropolis sampling" ), QStringLiteral( "GPU - Metropolis sampling" ), false },
        { QStringLiteral( "Rejection sampling" ), QStringLiteral( "GPU - Rejection sampling" ), false }
    };
    m_test_succeeded = false;

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
        QFileInfo::exists( m_transfer_function_path ),
        qPrintable( QStringLiteral( "Transfer function file not found: %1" ).arg( m_transfer_function_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );

    m_server_process.setProgram( m_server_executable );
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.setProcessChannelMode( QProcess::MergedChannels );
    m_server_process.start();

    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
}

void CommunicationSettingTest::cleanupTestCase()
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

    writeSummaryReport();
}

void CommunicationSettingTest::performs_communication_setting_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    startVideoRecording();

    MainWindow client_window( *g_test_app );
    client_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &client_window ) );

    ClientHandles client = resolveClientHandles( client_window );
    client.object_editor->show();

    logStep( QStringLiteral( "scenario: uniform begin" ) );
    configureLocalSampling( client, client.uniform_radio );
    waitForObjectAndApply( client );
    clearServerOutput();
    clickJumpAndWaitForCompletion( client );
    waitForServerSamplingMessage( QStringLiteral( "GPU - Uniform sampling" ) );
    QTest::qWait( k_short_wait_ms );

    logStep( QStringLiteral( "scenario: metropolis begin" ) );
    ensureDisconnected( client );
    configureLocalSampling( client, client.metropolis_radio );
    waitForObjectAndApply( client );
    clearServerOutput();
    clickJumpAndWaitForCompletion( client );
    waitForServerSamplingMessage( QStringLiteral( "GPU - Metropolis sampling" ) );
    QTest::qWait( k_short_wait_ms );

    logStep( QStringLiteral( "scenario: rejection begin" ) );
    ensureDisconnected( client );
    configureLocalSampling( client, client.rejection_radio );
    waitForObjectAndApply( client );
    clearServerOutput();
    clickJumpAndWaitForCompletion( client );
    waitForServerSamplingMessage( QStringLiteral( "GPU - Rejection sampling" ) );
    QTest::qWait( k_short_wait_ms );

    stopVideoRecording();
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
    ClientTests::CommunicationSettingTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
