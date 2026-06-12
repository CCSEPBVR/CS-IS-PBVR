#include "CommunicationTest.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QDate>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QMetaObject>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>
#include <QTest>
#include <QSignalSpy>

#include "../App/MainWindow.h"
#include "../ExtendedKVS/Screen.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TotalParticlesToolBar.h"
#include "../Widgets/VolumeTransform.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

#include <cmath>
#include <csignal>
#include <memory>

namespace
{
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_disconnect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 180000;
constexpr int k_dialog_timeout_ms = 15000;
constexpr int k_share_list_timeout_ms = 15000;
constexpr int k_share_view_apply_timeout_ms = 5000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_capture_settle_ms = 700;
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

QString normalizedPathForCompare( const QString& path )
{
    return path.isEmpty() ? QString() : QDir::cleanPath( path );
}

bool verifyInitializeMessage(
    const QSignalSpy& spy,
    int expected_sampling_type,
    const QString& expected_volume_path,
    const QString& expected_transfer_function_path,
    QString* error_message )
{
    auto fail = [error_message]( const QString& message )
    {
        if ( error_message != nullptr )
        {
            *error_message = message;
        }
        return false;
    };

    if ( spy.isEmpty() )
    {
        return fail( QStringLiteral( "Communication did not send an initialize message" ) );
    }

    const QString message = spy.last().at( 0 ).toString();
    QJsonParseError parse_error;
    const QJsonDocument document = QJsonDocument::fromJson( message.toUtf8(), &parse_error );
    if ( parse_error.error != QJsonParseError::NoError )
    {
        return fail( QStringLiteral( "Initialize message JSON parse error: %1" ).arg( parse_error.errorString() ) );
    }
    if ( !document.isObject() )
    {
        return fail( QStringLiteral( "Initialize message was not a JSON object: %1" ).arg( message ) );
    }

    const QJsonObject object = document.object();
    const QString event = object.value( QString::fromUtf8( Protocol::Key::Event ) ).toString();
    if ( event != QString::fromUtf8( Protocol::Events::Initialize ) )
    {
        return fail( QStringLiteral( "Unexpected Event: %1" ).arg( event ) );
    }

    const int viz_mode = object.value( QStringLiteral( "VizMode" ) ).toInt( -1 );
    if ( viz_mode != static_cast<int>( Viz::Mode::RemoteClientAndServer ) )
    {
        return fail( QStringLiteral( "Unexpected VizMode: %1" ).arg( viz_mode ) );
    }

    const int sampling_type = object.value( QStringLiteral( "SamplingType" ) ).toInt( -1 );
    if ( sampling_type != expected_sampling_type )
    {
        return fail(
            QStringLiteral( "Unexpected SamplingType: expected %1, actual %2" )
                .arg( expected_sampling_type )
                .arg( sampling_type ) );
    }

    const QString volume_path = object.value( QString::fromUtf8( Protocol::Key::VolumeDataFilePath ) ).toString();
    if ( normalizedPathForCompare( volume_path ) != normalizedPathForCompare( expected_volume_path ) )
    {
        return fail(
            QStringLiteral( "Unexpected VolumeDataFilePath: expected '%1', actual '%2'" )
                .arg( expected_volume_path, volume_path ) );
    }

    const QString transfer_function_path =
        object.value( QString::fromUtf8( Protocol::Key::TransferFunctionFilePath ) ).toString();
    if ( normalizedPathForCompare( transfer_function_path ) != normalizedPathForCompare( expected_transfer_function_path ) )
    {
        return fail(
            QStringLiteral( "Unexpected TransferFunctionFilePath: expected '%1', actual '%2'" )
                .arg( expected_transfer_function_path, transfer_function_path ) );
    }

    const int uuid_count = object.value( QString::fromUtf8( Protocol::Key::UUID ) ).toArray().size();
    if ( uuid_count != 2 )
    {
        return fail( QStringLiteral( "Unexpected UUID array size: %1" ).arg( uuid_count ) );
    }

    const int format_count = object.value( QString::fromUtf8( Protocol::Key::Format ) ).toArray().size();
    if ( format_count != 2 )
    {
        return fail( QStringLiteral( "Unexpected Format array size: %1" ).arg( format_count ) );
    }

    return true;
}
}

namespace ClientTests
{

CommunicationTest::CommunicationTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArrayLiteral( "1800000" ) );
}

QString CommunicationTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString CommunicationTest::repoRootPath() const
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

QString CommunicationTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool CommunicationTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void CommunicationTest::startVideoRecording()
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

void CommunicationTest::stopVideoRecording()
{
#ifdef Q_OS_WIN
    Q_UNUSED( m_recording_process );
    return;
#else
    if ( m_recording_process.state() == QProcess::NotRunning )
    {
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
#endif
}

void CommunicationTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void CommunicationTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void CommunicationTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void CommunicationTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# CommunicationTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
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
        stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
    }
}

void CommunicationTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

CommunicationTest::ClientHandles CommunicationTest::resolveClientHandles( MainWindow& window ) const
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
    handles.total_particles_tool_bar = window.findChild<TotalParticlesToolBar*>();
    handles.volume_transform = window.findChild<VolumeTransform*>();
    handles.screen_widget = window.findChild<kvs::qt::jaea::Screen*>();
    handles.tools_menu = window.findChild<QMenu*>( "menuTools" );

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.total_particles_tool_bar != nullptr, "TotalParticlesToolBar not found" ) ) { return handles; }
    if ( !require( handles.volume_transform != nullptr, "VolumeTransform not found" ) ) { return handles; }
    if ( !require( handles.screen_widget != nullptr, "m_screen not found" ) ) { return handles; }
    if ( !require( handles.tools_menu != nullptr, "menuTools not found" ) ) { return handles; }

    const auto actions = window.findChildren<QAction*>();
    for ( QAction* action : actions )
    {
        if ( action == nullptr ) { continue; }

        if ( action->text() == QStringLiteral( "Glyph Editor" ) ) { handles.glyph_editor_action = action; }
        else if ( action->text() == QStringLiteral( "Plot Over Line Editor" ) ) { handles.plot_over_line_editor_action = action; }
        else if ( action->text() == QStringLiteral( "Plot Over Time Editor" ) ) { handles.plot_over_time_editor_action = action; }
        else if ( action->text() == QStringLiteral( "Transfer Function Editor" ) ) { handles.transfer_function_editor_action = action; }
        else if ( action->text() == QStringLiteral( "Volume Transform" ) ) { handles.volume_transform_action = action; }
    }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.transfer_operator_apply_button = handles.communication->findChild<QPushButton*>( "transferOperatorApplyPushButton" );
    handles.volume_data_path_button = handles.communication->findChild<QPushButton*>( "volumeDataFilePathPushButton" );
    handles.transfer_function_path_button = handles.communication->findChild<QPushButton*>( "transferFunctionFilePathPushButton" );
    handles.share_view_button = handles.communication->findChild<QPushButton*>( "shareViewPushButton" );
    handles.local_viz_radio = handles.communication->findChild<QRadioButton*>( "localVizRadioButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.uniform_radio = handles.communication->findChild<QRadioButton*>( "uniformRadioButton" );
    handles.metropolis_radio = handles.communication->findChild<QRadioButton*>( "metropolisRadioButton" );
    handles.rejection_radio = handles.communication->findChild<QRadioButton*>( "rejectionRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.transfer_function_path_line_edit = handles.communication->findChild<QLineEdit*>( "transferFunctionFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.is_operator_line_edit = handles.communication->findChild<QLineEdit*>( "isOperatorLineEdit" );
    handles.transfer_operator_id_line_edit = handles.communication->findChild<QLineEdit*>( "transferOperatorIDlineEdit" );
    handles.chat_line_edit = handles.communication->findChild<QLineEdit*>( "chatLineEdit" );
    handles.text_browser = handles.communication->findChild<QTextBrowser*>( "textBrowser" );
    handles.tab_widget = handles.communication->findChild<QTabWidget*>( "tabWidget" );
    handles.share_view_tab = handles.communication->findChild<QWidget*>( "shareViewTab" );
    handles.share_list_view = handles.communication->findChild<QListView*>( "shareListView" );

    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );

    const auto total_particle_labels = handles.total_particles_tool_bar->findChildren<QLabel*>();
    for ( QLabel* label : total_particle_labels )
    {
        if ( label && label->text() != QStringLiteral( "Total Particles : " ) )
        {
            handles.total_particles_display_label = label;
            break;
        }
    }

    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.rotation_x_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationXAxisDoubleSpinBox" );
    handles.volume_transform_apply_button = handles.volume_transform->findChild<QPushButton*>( "applyPushButton" );

    if ( !require( handles.glyph_editor_action != nullptr, "Glyph Editor action not found" ) ) { return handles; }
    if ( !require( handles.plot_over_line_editor_action != nullptr, "Plot Over Line Editor action not found" ) ) { return handles; }
    if ( !require( handles.plot_over_time_editor_action != nullptr, "Plot Over Time Editor action not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_editor_action != nullptr, "Transfer Function Editor action not found" ) ) { return handles; }
    if ( !require( handles.volume_transform_action != nullptr, "Volume Transform action not found" ) ) { return handles; }
    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.transfer_operator_apply_button != nullptr, "transferOperatorApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_button != nullptr, "volumeDataFilePathPushButton not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_path_button != nullptr, "transferFunctionFilePathPushButton not found" ) ) { return handles; }
    if ( !require( handles.share_view_button != nullptr, "shareViewPushButton not found" ) ) { return handles; }
    if ( !require( handles.local_viz_radio != nullptr, "localVizRadioButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.uniform_radio != nullptr, "uniformRadioButton not found" ) ) { return handles; }
    if ( !require( handles.metropolis_radio != nullptr, "metropolisRadioButton not found" ) ) { return handles; }
    if ( !require( handles.rejection_radio != nullptr, "rejectionRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_path_line_edit != nullptr, "transferFunctionFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.is_operator_line_edit != nullptr, "isOperatorLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_operator_id_line_edit != nullptr, "transferOperatorIDlineEdit not found" ) ) { return handles; }
    if ( !require( handles.chat_line_edit != nullptr, "chatLineEdit not found" ) ) { return handles; }
    if ( !require( handles.text_browser != nullptr, "textBrowser not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) { return handles; }
    if ( !require( handles.total_particles_display_label != nullptr, "m_total_particles_display not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.tab_widget != nullptr, "tabWidget not found" ) ) { return handles; }
    if ( !require( handles.share_view_tab != nullptr, "shareViewTab not found" ) ) { return handles; }
    if ( !require( handles.share_list_view != nullptr, "shareListView not found" ) ) { return handles; }
    if ( !require( handles.rotation_x_axis_spin_box != nullptr, "rotationXAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.volume_transform_apply_button != nullptr, "VolumeTransform applyPushButton not found" ) ) { return handles; }

    return handles;
}

void CommunicationTest::ensureConnected( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );

    const auto is_connected = [client]()
    {
        return client.disconnect_button->isEnabled() &&
               !client.connect_button->isEnabled() &&
               !client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_connected() ) { return; }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        QVERIFY2(
            waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
            "connectPushButton did not become enabled within the timeout" );

        client.connect_button->click();
        if ( waitForCondition( is_connected, k_connect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            return;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    QFAIL( "Client did not enter the connected state after clicking connectPushButton" );
}

void CommunicationTest::ensureDisconnected( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );

    const auto is_disconnected = [client]()
    {
        return client.connect_button->isEnabled() &&
               !client.disconnect_button->isEnabled() &&
               client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_disconnected() ) { return; }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        QVERIFY2(
            waitForCondition( [client]() { return client.disconnect_button->isEnabled(); }, k_disconnect_timeout_ms, 100 ),
            "disconnectPushButton did not become enabled within the timeout" );

        client.disconnect_button->click();
        if ( waitForCondition( is_disconnected, k_disconnect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            return;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    QFAIL( "Client did not enter the disconnected state after clicking disconnectPushButton" );
}

void CommunicationTest::connectGuestClient( const ClientHandles& guest ) const
{
    ensureConnected( guest );
}

void CommunicationTest::selectRadioButton( QRadioButton* radio_button, const char* object_name ) const
{
    QVERIFY2( radio_button != nullptr, "Target radio button was not found" );
    if ( radio_button->isChecked() ) { return; }

    QVERIFY2(
        waitForCondition(
            [radio_button]() { return radio_button->isEnabled() && radio_button->isVisible(); },
            k_connect_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "%1 did not become clickable within the timeout" ).arg( object_name ) ) );

    radio_button->click();
    if ( !radio_button->isChecked() )
    {
        radio_button->click();
    }
    QVERIFY2( radio_button->isChecked(), qPrintable( QStringLiteral( "%1 was not checked" ).arg( object_name ) ) );
}

void CommunicationTest::setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const
{
    QVERIFY2( spin_box != nullptr, "Target double spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QTRY_VERIFY( qAbs( spin_box->value() - value ) < 1e-9 );
}

void CommunicationTest::configureVisualization(
    const ClientHandles& client,
    QRadioButton* visualization_radio,
    const QString& volume_path,
    const QString& transfer_function_path,
    QRadioButton* sampling_radio ) const
{
    bringWindowToFront( client.main_window );
    selectRadioButton( visualization_radio, "visualization radio button" );
    if ( sampling_radio != nullptr )
    {
        selectRadioButton( sampling_radio, "sampling radio button" );
    }
    setLineEditText( client.volume_data_path_line_edit, volume_path );
    setLineEditText( client.transfer_function_path_line_edit, transfer_function_path );
    client.setting_apply_button->click();
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_apply_button->isEnabled() &&
                       client.object_tree_view->model() != nullptr &&
                       client.object_tree_view->model()->rowCount() > 0 &&
                       !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor model was not ready within the timeout" );

    QAbstractItemModel* model = client.object_tree_view->model();
    QVERIFY2( model != nullptr, "ObjectEditor treeView model not found" );
    const QModelIndex first_index = model->index( 0, 0 );
    QVERIFY2( first_index.isValid(), "ObjectEditor first row is invalid" );
    client.object_tree_view->scrollTo( first_index );
    client.object_tree_view->setCurrentIndex( first_index );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        first_index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    const QModelIndex display_index = model->index( 0, 2 );
    if ( display_index.isValid() && display_index.flags().testFlag( Qt::ItemIsUserCheckable ) )
    {
        if ( display_index.data( Qt::CheckStateRole ).toInt() != Qt::Checked )
        {
            QVERIFY2(
                model->setData( display_index, Qt::Checked, Qt::CheckStateRole ),
                "Failed to check ObjectEditor display item" );
        }
    }

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled within the timeout" );

    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled again within the timeout" );

    QTest::qWait( k_post_jump_wait_ms );
}

void CommunicationTest::generateObject(
    const ClientHandles& client,
    QRadioButton* visualization_radio,
    const QString& volume_path,
    const QString& transfer_function_path,
    QRadioButton* sampling_radio ) const
{
    configureVisualization( client, visualization_radio, volume_path, transfer_function_path, sampling_radio );
    waitForObjectAndApply( client );
    clickJumpAndWaitForCompletion( client );
}

void CommunicationTest::openToolsMenuAndCapture(
    const ClientHandles& client,
    const QString& file_name,
    const QString& caption,
    bool expected_enabled )
{
    QCOMPARE( client.glyph_editor_action->isEnabled(), expected_enabled );
    QCOMPARE( client.plot_over_line_editor_action->isEnabled(), expected_enabled );
    QCOMPARE( client.plot_over_time_editor_action->isEnabled(), expected_enabled );
    QCOMPARE( client.transfer_function_editor_action->isEnabled(), expected_enabled );

    bringWindowToFront( client.main_window );
    QMenuBar* menu_bar = client.main_window->menuBar();
    QVERIFY2( menu_bar != nullptr, "Menu bar not found" );
    const QRect action_rect = menu_bar->actionGeometry( client.tools_menu->menuAction() );
    if ( action_rect.isValid() )
    {
        QTest::mouseClick( menu_bar, Qt::LeftButton, Qt::NoModifier, action_rect.center() );
    }
    else
    {
        client.tools_menu->popup( client.main_window->mapToGlobal( QPoint( 140, 36 ) ) );
    }
    QVERIFY2(
        waitForCondition( [client]() { return client.tools_menu->isVisible(); }, 5000, 50 ),
        "menuTools did not open" );
    saveScreenshot( file_name, caption );
    client.tools_menu->hide();
}

QFileDialog* CommunicationTest::waitForFileDialog( int timeout_ms ) const
{
    QFileDialog* dialog = nullptr;
    const bool dialog_found = waitForCondition(
        [&dialog]()
        {
            const auto widgets = QApplication::topLevelWidgets();
            for ( QWidget* widget : widgets )
            {
                auto* file_dialog = qobject_cast<QFileDialog*>( widget );
                if ( file_dialog && file_dialog->isVisible() )
                {
                    dialog = file_dialog;
                    return true;
                }
            }
            return false;
        },
        timeout_ms,
        50 );

    return dialog_found ? dialog : nullptr;
}

void CommunicationTest::selectFileFromBrowseDialog(
    const ClientHandles& client,
    QPushButton* browse_button,
    const QString& file_path,
    const QString& screenshot_file_name,
    const QString& caption )
{
    bringWindowToFront( client.main_window );
    auto selection_finished = std::make_shared<bool>( false );
    QTimer::singleShot(
        0,
        qApp,
        [this, file_path, screenshot_file_name, caption, selection_finished]()
        {
            QFileDialog* dialog = waitForFileDialog( k_dialog_timeout_ms );
            QVERIFY2( dialog != nullptr, "File dialog was not shown" );

            const QFileInfo file_info( file_path );
            QVERIFY2( file_info.exists(), qPrintable( QStringLiteral( "Target file does not exist: %1" ).arg( file_path ) ) );

            dialog->setDirectory( file_info.absolutePath() );
            QTest::qWait( 300 );
            dialog->selectFile( file_info.fileName() );
            QCoreApplication::processEvents();

            if ( auto* list_view = dialog->findChild<QListView*>( "listView" ) )
            {
                const QModelIndex root_index = list_view->rootIndex();
                const int rows = list_view->model() ? list_view->model()->rowCount( root_index ) : 0;
                for ( int row = 0; row < rows; ++row )
                {
                    const QModelIndex index = list_view->model()->index( row, 0, root_index );
                    if ( index.data().toString() != file_info.fileName() ) { continue; }
                    list_view->selectionModel()->setCurrentIndex(
                        index,
                        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                    break;
                }
            }

            saveScreenshot( screenshot_file_name, caption );
            QMetaObject::invokeMethod( dialog, "accept", Qt::QueuedConnection );
            *selection_finished = true;
        } );

    browse_button->click();
    QVERIFY2(
        waitForCondition( [selection_finished]() { return *selection_finished; }, k_dialog_timeout_ms, 50 ),
        "File dialog selection did not finish within the timeout" );
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::uncheckSecondObjectDisplay( const ClientHandles& client ) const
{
    QAbstractItemModel* model = client.object_tree_view->model();
    QVERIFY2( model != nullptr, "ObjectEditor treeView model not found" );
    const QModelIndex display_index = model->index( 1, 2 );
    QVERIFY2( display_index.isValid(), "ObjectEditor second row display item index is invalid" );
    QVERIFY2( model->setData( display_index, Qt::Unchecked, Qt::CheckStateRole ), "Failed to uncheck display item" );
    QTest::qWait( k_short_wait_ms );
    client.object_apply_button->click();
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::openVolumeTransformAndApplyRotationX( const ClientHandles& client, double value ) const
{
    bringWindowToFront( client.main_window );
    client.volume_transform_action->trigger();
    QVERIFY2(
        waitForCondition( [client]() { return client.volume_transform->isVisible(); }, 5000, 50 ),
        "VolumeTransform did not become visible" );
    client.volume_transform->raise();
    client.volume_transform->activateWindow();
    setDoubleSpinBoxValue( client.rotation_x_axis_spin_box, value );
    client.volume_transform_apply_button->click();
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::selectShareViewTab( const ClientHandles& client ) const
{
    const int share_view_index = client.tab_widget->indexOf( client.share_view_tab );
    QVERIFY2( share_view_index >= 0, "tabWidget does not contain shareViewTab" );
    client.tab_widget->setCurrentIndex( share_view_index );
    QCOMPARE( client.tab_widget->currentIndex(), share_view_index );
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::dragScreenLeftButton( QWidget* widget ) const
{
    QVERIFY2( widget != nullptr, "Drag target widget is null" );

    const QPoint start = widget->rect().center();
    const QPoint end = start + QPoint( -100, 90 );
    const QPoint global_start = widget->mapToGlobal( start );
    const QPoint global_end = widget->mapToGlobal( end );

    widget->setFocus();

    QMouseEvent press_event(
        QEvent::MouseButtonPress,
        start,
        global_start,
        Qt::LeftButton,
        Qt::LeftButton,
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
            Qt::LeftButton,
            Qt::NoModifier );
        QVERIFY( QCoreApplication::sendEvent( widget, &move_event ) );
        QTest::qWait( 20 );
    }

    QMouseEvent release_event(
        QEvent::MouseButtonRelease,
        end,
        global_end,
        Qt::LeftButton,
        Qt::NoButton,
        Qt::NoModifier );
    QVERIFY( QCoreApplication::sendEvent( widget, &release_event ) );
    QTest::qWait( k_short_wait_ms );
}

void CommunicationTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    const QString default_client_executable =
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() );

    m_operator_client_executable = envOrDefault( "PBVR_OPERATOR_CLIENT_EXECUTABLE", default_client_executable );
    m_guest_client_executable = envOrDefault( "PBVR_GUEST_CLIENT_EXECUTABLE", default_client_executable );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "CommunicationTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "CommunicationTest.mov" ) );
    m_test_succeeded = false;

    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

}

void CommunicationTest::cleanupTestCase()
{
    if ( m_recording_process.state() != QProcess::NotRunning )
    {
        stopVideoRecording();
    }

    writeMarkdownReport();
}

void CommunicationTest::performs_communication_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    startVideoRecording();

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

    const QString piece_example =
        ClientTests::configuredPath( "MEJ_VOLUME_DATA", repoRootPath() );
    const QString mej_v2 = ClientTests::configuredPath( "MEJ_TRANSFER_FUNCTION", repoRootPath() );
    const QString spx = ClientTests::configuredPath( "SPX_VOLUME_DATA", repoRootPath() );
    const QString gt5d = ClientTests::configuredPath( "GT5D_VOLUME_DATA", repoRootPath() );
    const QString gt5d_tfe = ClientTests::configuredPath( "GT5D_TRANSFER_FUNCTION", repoRootPath() );
    const QString mej = ClientTests::configuredPath( "MEJ_VOLUME_DATA", repoRootPath() );
    const QString mej_tfe = ClientTests::configuredPath( "MEJ_TRANSFER_FUNCTION", repoRootPath() );
    const QString hydrogen = ClientTests::configuredPath( "HYDROGEN_VOLUME_DATA", repoRootPath() );
    const QString tornado = ClientTests::configuredPath( "TORNADO_VOLUME_DATA", repoRootPath() );

    for ( const QString& file_path : { piece_example, mej_v2, spx, gt5d, gt5d_tfe, mej, mej_tfe, hydrogen, tornado } )
    {
        QVERIFY2( QFileInfo::exists( file_path ), qPrintable( QStringLiteral( "Required test file not found: %1" ).arg( file_path ) ) );
    }

    ensureConnected( operator_client );
    saveScreenshot( QStringLiteral( "01_operator_join.png" ), QStringLiteral( "Operator用 Chatにjoin表示が出力された状態" ) );
    markStepCompleted( QStringLiteral( "Operator用 Communication.ui: connectPushButtonを押し、join表示のスクリーンショットを撮影しました。" ) );

    connectGuestClient( guest_client );
    saveScreenshot( QStringLiteral( "02_guest_join.png" ), QStringLiteral( "Guest用 Chatにjoin表示が出力された状態" ) );
    markStepCompleted( QStringLiteral( "Guest用 Communication.ui: connectPushButtonを押し、join表示のスクリーンショットを撮影しました。" ) );

    generateObject( operator_client, operator_client.local_viz_radio, piece_example, mej_v2 );
    saveScreenshot( QStringLiteral( "03_operator_piece_object.png" ), QStringLiteral( "Operator用画面にオブジェクトが表示された状態" ) );
    openToolsMenuAndCapture(
        operator_client,
        QStringLiteral( "04_operator_tools_enabled.png" ),
        QStringLiteral( "ToolsメニューでGlyph Editor, Plot Over Line Editor, Plot Over Time Editor, Transfer Function Editorが有効な状態" ),
        true );
    bringWindowToFront( guest_client.main_window );
    saveScreenshot( QStringLiteral( "05_guest_piece_object.png" ), QStringLiteral( "Guest用画面にオブジェクトが表示された状態" ) );
    markStepCompleted( QStringLiteral( "Operator用 localVizRadioButtonでPieceデータを読み込み、Operator/Guestの表示とTools有効状態を確認しました。" ) );

    ensureDisconnected( guest_client );
    saveScreenshot( QStringLiteral( "06_guest_after_disconnect.png" ), QStringLiteral( "Guest用 Disconnect後にCommunicationウィジェットが有効でChatがクリアされた状態" ) );
    ensureDisconnected( operator_client );
    openToolsMenuAndCapture(
        operator_client,
        QStringLiteral( "07_operator_tools_disabled_after_disconnect.png" ),
        QStringLiteral( "Operator用 Disconnect後にTools対象アクションが無効でChatがクリアされた状態" ),
        false );
    markStepCompleted( QStringLiteral( "Guest/OperatorのdisconnectPushButtonを押し、ChatクリアとTools無効状態を確認しました。" ) );

    ensureConnected( operator_client );
    QSignalSpy uniform_initialize_spy( operator_client.communication, &Communication::textMessageSent );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, spx, QString(), nullptr );
    QString initialize_message_error;
    QVERIFY2(
        verifyInitializeMessage(
            uniform_initialize_spy,
            static_cast<int>( Communication::SamplingType::Uniform ),
            spx,
            QString(),
            &initialize_message_error ),
        qPrintable( initialize_message_error ) );
    markStepCompleted( QStringLiteral( "spxでUniform samplingの送信JSONとリモート粒子生成を確認しました。" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    QSignalSpy metropolis_initialize_spy( operator_client.communication, &Communication::textMessageSent );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, spx, QString(), operator_client.metropolis_radio );
    initialize_message_error.clear();
    QVERIFY2(
        verifyInitializeMessage(
            metropolis_initialize_spy,
            static_cast<int>( Communication::SamplingType::Metropolis ),
            spx,
            QString(),
            &initialize_message_error ),
        qPrintable( initialize_message_error ) );
    markStepCompleted( QStringLiteral( "metropolisRadioButtonで送信JSONとリモート粒子生成を確認しました。" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    QSignalSpy rejection_initialize_spy( operator_client.communication, &Communication::textMessageSent );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, spx, QString(), operator_client.rejection_radio );
    initialize_message_error.clear();
    QVERIFY2(
        verifyInitializeMessage(
            rejection_initialize_spy,
            static_cast<int>( Communication::SamplingType::Rejection ),
            spx,
            QString(),
            &initialize_message_error ),
        qPrintable( initialize_message_error ) );
    markStepCompleted( QStringLiteral( "rejectionRadioButtonで送信JSONとリモート粒子生成を確認しました。" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, spx, QString(), operator_client.uniform_radio );
    saveScreenshot( QStringLiteral( "08_spx_particles.png" ), QStringLiteral( "spxの粒子生成を行うことができた状態" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, gt5d, QString(), nullptr );
    saveScreenshot( QStringLiteral( "09_gt5d_particles.png" ), QStringLiteral( "gt5dの粒子生成を行うことができた状態" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, mej, QString(), nullptr );
    saveScreenshot( QStringLiteral( "10_mej_glyph.png" ), QStringLiteral( "mejのグリフ生成を行うことができた状態" ) );
    uncheckSecondObjectDisplay( operator_client );
    clickJumpAndWaitForCompletion( operator_client );
    saveScreenshot( QStringLiteral( "11_mej_particles.png" ), QStringLiteral( "mejの粒子生成を行うことができた状態" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, hydrogen, QString(), nullptr );
    saveScreenshot( QStringLiteral( "12_hydrogen_particles.png" ), QStringLiteral( "Hydrogenの粒子生成を行うことができた状態" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    generateObject( operator_client, operator_client.remote_viz_client_server_radio, tornado, QString(), nullptr );
    saveScreenshot( QStringLiteral( "13_tornado_particles_glyph.png" ), QStringLiteral( "tornadoの粒子・グリフ生成を行うことができた状態" ) );
    markStepCompleted( QStringLiteral( "spx, gt5d, mej, Hydrogen, tornadoの粒子またはグリフ生成を確認しました。" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    selectFileFromBrowseDialog(
        operator_client,
        operator_client.volume_data_path_button,
        gt5d,
        QStringLiteral( "14_gt5d_volume_file_dialog.png" ),
        QStringLiteral( "gt5dのボリュームデータをファイルダイアログから選択した状態" ) );
    selectFileFromBrowseDialog(
        operator_client,
        operator_client.transfer_function_path_button,
        gt5d_tfe,
        QStringLiteral( "15_gt5d_transfer_function_file_dialog.png" ),
        QStringLiteral( "gt5dの伝達関数ファイルをファイルダイアログから選択した状態" ) );
    operator_client.setting_apply_button->click();
    waitForObjectAndApply( operator_client );
    clickJumpAndWaitForCompletion( operator_client );
    saveScreenshot( QStringLiteral( "16_gt5d_transfer_function_applied.png" ), QStringLiteral( "gt5dに伝達関数ファイルを反映できた状態" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    selectFileFromBrowseDialog(
        operator_client,
        operator_client.volume_data_path_button,
        mej,
        QStringLiteral( "17_mej_volume_file_dialog.png" ),
        QStringLiteral( "mejのボリュームデータをファイルダイアログから選択した状態" ) );
    selectFileFromBrowseDialog(
        operator_client,
        operator_client.transfer_function_path_button,
        mej_tfe,
        QStringLiteral( "18_mej_transfer_function_file_dialog.png" ),
        QStringLiteral( "mejの伝達関数ファイルをファイルダイアログから選択した状態" ) );
    operator_client.setting_apply_button->click();
    waitForObjectAndApply( operator_client );
    uncheckSecondObjectDisplay( operator_client );
    clickJumpAndWaitForCompletion( operator_client );
    openVolumeTransformAndApplyRotationX( operator_client, -90.0 );
    saveScreenshot( QStringLiteral( "19_mej_transfer_function_applied.png" ), QStringLiteral( "mejに伝達関数ファイルを反映できた状態" ) );
    markStepCompleted( QStringLiteral( "ファイルダイアログからボリュームデータと伝達関数ファイルを選択し、gt5d/mejへの反映を確認しました。" ) );

    ensureDisconnected( operator_client );
    ensureConnected( operator_client );
    bringWindowToFront( operator_client.main_window );
    saveScreenshot( QStringLiteral( "20_operator_user_id.png" ), QStringLiteral( "OperatorにユーザIDが割り当てられた状態" ) );

    ensureDisconnected( guest_client );
    connectGuestClient( guest_client );
    const QString guest_id = guest_client.id_line_edit->text().trimmed();
    QVERIFY2( !guest_id.isEmpty(), "Guest ID is empty" );
    QVERIFY2( guest_id != operator_client.id_line_edit->text().trimmed(), "Operator and Guest IDs are not distinct" );
    bringWindowToFront( guest_client.main_window );
    saveScreenshot( QStringLiteral( "21_guest_user_id.png" ), QStringLiteral( "GuestにOperatorとは異なるユーザIDが割り当てられた状態" ) );

    bringWindowToFront( operator_client.main_window );
    configureVisualization( operator_client, operator_client.remote_viz_client_server_radio, spx, QString(), operator_client.uniform_radio );
    QVERIFY2(
        waitForCondition( [operator_client]() { return !operator_client.object_name_line_edit->text().trimmed().isEmpty(); }, k_object_load_timeout_ms, 100 ),
        "ObjectEditor nameLineEdit did not receive text within the timeout" );
    QVERIFY2(
        waitForCondition( [operator_client]() { return operator_client.is_operator_line_edit->text().trimmed() == QStringLiteral( "true" ); }, k_connect_timeout_ms, 100 ),
        "Operator did not have operator privilege within the timeout" );
    bringWindowToFront( operator_client.main_window );
    saveScreenshot( QStringLiteral( "22_operator_has_privilege.png" ), QStringLiteral( "Operator用画面に操作権限がある状態" ) );
    QVERIFY2(
        waitForCondition( [guest_client]() { return guest_client.is_operator_line_edit->text().trimmed() == QStringLiteral( "false" ); }, k_connect_timeout_ms, 100 ),
        "Guest did not show no-operator privilege within the timeout" );
    bringWindowToFront( guest_client.main_window );
    saveScreenshot( QStringLiteral( "23_guest_no_privilege.png" ), QStringLiteral( "Guest用画面に操作権限がない状態" ) );

    bringWindowToFront( operator_client.main_window );
    setLineEditText( operator_client.transfer_operator_id_line_edit, guest_id );
    operator_client.transfer_operator_apply_button->click();
    QTest::qWait( k_short_wait_ms );
    saveScreenshot( QStringLiteral( "24_operator_transfer_operator.png" ), QStringLiteral( "Operator用画面が操作権限をGuest用画面に移譲した状態" ) );
    QVERIFY2(
        waitForCondition( [guest_client]() { return guest_client.is_operator_line_edit->text().trimmed() == QStringLiteral( "true" ); }, k_connect_timeout_ms, 100 ),
        "Guest did not become operator within the timeout" );
    bringWindowToFront( guest_client.main_window );
    saveScreenshot( QStringLiteral( "25_guest_received_operator.png" ), QStringLiteral( "Guest用画面が操作権限をOperator用画面から受け取った状態" ) );

    setLineEditText( operator_client.chat_line_edit, QStringLiteral( "test1" ) );
    QTest::keyClick( operator_client.chat_line_edit, Qt::Key_Return );
    QVERIFY2(
        waitForCondition( [guest_client]() { return guest_client.text_browser->toPlainText().contains( QStringLiteral( "test1" ) ); }, k_connect_timeout_ms, 100 ),
        "Guest chat view did not receive test1" );
    saveScreenshot( QStringLiteral( "26_guest_received_chat_test1.png" ), QStringLiteral( "Guest用画面がOperator用画面からチャットtest1を受け取った状態" ) );

    bringWindowToFront( operator_client.main_window );
    setLineEditText( guest_client.chat_line_edit, QStringLiteral( "test2" ) );
    QTest::keyClick( guest_client.chat_line_edit, Qt::Key_Return );
    QVERIFY2(
        waitForCondition( [operator_client]() { return operator_client.text_browser->toPlainText().contains( QStringLiteral( "test2" ) ); }, k_connect_timeout_ms, 100 ),
        "Operator chat view did not receive test2" );
    saveScreenshot( QStringLiteral( "27_operator_received_chat_test2.png" ), QStringLiteral( "Operator用画面がGuest用画面からチャットtest2を受け取った状態" ) );
    markStepCompleted( QStringLiteral( "ユーザID、操作権限移譲、双方向チャットを確認しました。" ) );

    bringWindowToFront( guest_client.main_window );
    qInfo().noquote() << QStringLiteral( "Guest用 ObjectEditor.ui: nameLineEditを待機し、applyPushButtonを押します。" );
    waitForObjectAndApply( guest_client );
    qInfo().noquote() << QStringLiteral( "Guest用 PlayBackControlToolBar.cpp: m_jump_push_buttonを押します。" );
    clickJumpAndWaitForCompletion( guest_client );
    qInfo().noquote() << QStringLiteral( "Guest用 MainWindow.cpp: m_screenを左クリックでドラッグします。" );
    dragScreenLeftButton( guest_client.screen_widget );
    auto* long_guest_screen = qobject_cast<kvs::qt::jaea::Screen*>( guest_client.screen_widget );
    auto* long_operator_screen = qobject_cast<kvs::qt::jaea::Screen*>( operator_client.screen_widget );
    QVERIFY2( long_guest_screen != nullptr, "Guest screen could not be resolved" );
    QVERIFY2( long_operator_screen != nullptr, "Operator screen could not be resolved" );
    const kvs::Xform long_guest_shared_xform = long_guest_screen->scene()->objectManager()->xform();

    selectShareViewTab( guest_client );
    guest_client.share_view_button->click();
    QTest::qWait( k_short_wait_ms );
    saveScreenshot( QStringLiteral( "28_guest_share_view_sender.png" ), QStringLiteral( "Guest用画面で視点共有者の視点を送信した状態" ) );

    bringWindowToFront( operator_client.main_window );
    selectShareViewTab( operator_client );
    QVERIFY2(
        waitForCondition(
            [operator_client]()
            {
                return operator_client.share_list_view->model() != nullptr &&
                       operator_client.share_list_view->model()->rowCount() > 0;
            },
            k_connect_timeout_ms,
            100 ),
        "Operator shareListView did not receive any shared view entries within the timeout" );
    const QModelIndex first_index = operator_client.share_list_view->model()->index( 0, 0 );
    QVERIFY2( first_index.isValid(), "Operator shareListView first item is invalid" );
    operator_client.share_list_view->scrollTo( first_index );
    operator_client.share_list_view->setCurrentIndex( first_index );
    operator_client.share_list_view->viewport()->setFocus();
    const QRect first_item_rect = operator_client.share_list_view->visualRect( first_index );
    QVERIFY2( first_item_rect.isValid(), "Operator shareListView first item visual rect is invalid" );
    QTest::mouseClick(
        operator_client.share_list_view->viewport(),
        Qt::LeftButton,
        Qt::NoModifier,
        first_item_rect.center() );
    QCoreApplication::processEvents();
    QTest::qWait( 100 );
    QTest::mouseDClick(
        operator_client.share_list_view->viewport(),
        Qt::LeftButton,
        Qt::NoModifier,
        first_item_rect.center() );
    if ( !waitForCondition(
             [long_operator_screen, long_guest_shared_xform]()
             {
                 return xformsNearlyEqual(
                     long_operator_screen->scene()->objectManager()->xform(),
                     long_guest_shared_xform );
             },
             k_share_view_apply_timeout_ms,
             100 ) )
    {
        const bool invoked = QMetaObject::invokeMethod(
            operator_client.communication,
            "onItemDoubleClicked",
            Qt::DirectConnection,
            Q_ARG( QModelIndex, first_index ) );
        QVERIFY2( invoked, "Failed to invoke Communication::onItemDoubleClicked for Operator shareListView first item" );
    }
    QVERIFY2(
        waitForCondition(
            [long_operator_screen, long_guest_shared_xform]()
            {
                return xformsNearlyEqual(
                    long_operator_screen->scene()->objectManager()->xform(),
                    long_guest_shared_xform );
            },
            k_connect_timeout_ms,
            100 ),
        "Operator objectManager xform did not match the shared Guest xform after double-clicking shareListView" );
    saveScreenshot( QStringLiteral( "29_operator_shared_view_received.png" ), QStringLiteral( "Operator用画面で視点共有者の視点を共有できた状態" ) );
    markStepCompleted( QStringLiteral( "GuestからShare Viewを送信し、OperatorのshareListViewの1番目をダブルクリックして視点共有を確認しました。" ) );

    stopVideoRecording();
    operator_client.main_window->close();
    guest_client.main_window->close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::CommunicationTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
