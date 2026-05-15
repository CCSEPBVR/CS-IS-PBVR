#include "AnimationControlTest.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QWidget>
#include <QTest>

#include "../App/MainWindow.h"
#include "../Widgets/AnimationControl.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TotalParticlesToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

#include <csignal>

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_file_dialog_timeout_ms = 15000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_three_second_wait_ms = 3000;
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

QString AnimationControlTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString AnimationControlTest::repoRootPath() const
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

QString AnimationControlTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool AnimationControlTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

QFileDialog* AnimationControlTest::waitForFileDialog( int timeout_ms ) const
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

void AnimationControlTest::selectFileFromDialog( const QString& file_path, bool require_existing_file ) const
{
    QFileDialog* dialog = waitForFileDialog( k_file_dialog_timeout_ms );
    QVERIFY2( dialog != nullptr, "File dialog was not shown" );

    const QFileInfo file_info( file_path );
    QVERIFY2(
        QDir().mkpath( file_info.absolutePath() ),
        qPrintable( QStringLiteral( "Failed to create dialog target directory: %1" ).arg( file_info.absolutePath() ) ) );

    if ( require_existing_file )
    {
        QVERIFY2(
            file_info.exists(),
            qPrintable( QStringLiteral( "Target file does not exist: %1" ).arg( file_path ) ) );
    }

    dialog->setDirectory( file_info.absolutePath() );
    QCoreApplication::processEvents();
    QTest::qWait( 200 );

    QLineEdit* file_name_edit = dialog->findChild<QLineEdit*>( QStringLiteral( "fileNameEdit" ) );
    if ( file_name_edit == nullptr )
    {
        const auto line_edits = dialog->findChildren<QLineEdit*>();
        for ( QLineEdit* line_edit : line_edits )
        {
            if ( line_edit && line_edit->isVisible() )
            {
                file_name_edit = line_edit;
                break;
            }
        }
    }

    QVERIFY2( file_name_edit != nullptr, "fileNameEdit was not found in QFileDialog" );
    file_name_edit->setFocus();
    file_name_edit->clear();
    QTest::keyClicks( file_name_edit, require_existing_file ? file_info.fileName() : file_info.completeBaseName() );
    QCoreApplication::processEvents();
    QCOMPARE( file_name_edit->text(), require_existing_file ? file_info.fileName() : file_info.completeBaseName() );

    QMetaObject::invokeMethod( dialog, "accept", Qt::QueuedConnection );
}

void AnimationControlTest::startVideoRecording()
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

void AnimationControlTest::stopVideoRecording()
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
        m_recording_process.kill();
        m_recording_process.waitForFinished( 5000 );
    }

    QVERIFY2(
        QFileInfo::exists( m_video_file_path ),
        qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
}

void AnimationControlTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void AnimationControlTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

AnimationControlTest::ClientHandles AnimationControlTest::resolveClientHandles( MainWindow& window ) const
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
    handles.animation_control = window.findChild<AnimationControl*>();
    handles.screen_widget = window.findChild<kvs::qt::jaea::Screen*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.total_particles_tool_bar != nullptr, "TotalParticlesToolBar not found" ) ) { return handles; }
    if ( !require( handles.animation_control != nullptr, "AnimationControl dock not found" ) ) { return handles; }
    if ( !require( handles.screen_widget != nullptr, "m_screen not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.capture_combo_box = handles.animation_control->findChild<QComboBox*>( "captureComboBox" );
    handles.image_file_line_edit = handles.animation_control->findChild<QLineEdit*>( "imageFileLineEdit" );
    handles.interpolation_spin_box = handles.animation_control->findChild<QSpinBox*>( "interpolationSpinBox" );
    handles.total_key_frames_display_label = handles.animation_control->findChild<QLabel*>( "totalKeyFramesDisplayLabel" );
    const auto total_particle_labels = handles.total_particles_tool_bar->findChildren<QLabel*>();
    for ( QLabel* label : total_particle_labels )
    {
        if ( label && label->text() != QStringLiteral( "Total Particles : " ) )
        {
            handles.total_particles_display_label = label;
            break;
        }
    }

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.capture_combo_box != nullptr, "captureComboBox not found" ) ) { return handles; }
    if ( !require( handles.image_file_line_edit != nullptr, "imageFileLineEdit not found" ) ) { return handles; }
    if ( !require( handles.interpolation_spin_box != nullptr, "interpolationSpinBox not found" ) ) { return handles; }
    if ( !require( handles.total_key_frames_display_label != nullptr, "totalKeyFramesDisplayLabel not found" ) ) { return handles; }
    if ( !require( handles.total_particles_display_label != nullptr, "m_total_particles_display not found" ) ) { return handles; }

    return handles;
}

void AnimationControlTest::ensureConnected( const ClientHandles& client ) const
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

void AnimationControlTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void AnimationControlTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_three_second_wait_ms );
}

void AnimationControlTest::waitForVisibleObject( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForVisibleObject: waiting for total particles > 0" ) );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                bool ok = false;
                const int total_particles = client.total_particles_display_label->text().toInt( &ok );
                return ok && total_particles > 0;
            },
            k_object_load_timeout_ms,
            200 ),
        qPrintable(
            QStringLiteral( "m_total_particles_display did not become > 0. current='%1'" )
                .arg( client.total_particles_display_label->text() ) ) );
}

void AnimationControlTest::focusScreen( QWidget* widget ) const
{
    QVERIFY2( widget != nullptr, "Screen widget is null" );
    widget->show();
    widget->raise();
    widget->activateWindow();
    widget->setFocus( Qt::OtherFocusReason );
    QCoreApplication::processEvents();
    QTest::qWait( 100 );
}

void AnimationControlTest::dragScreenLeftButton( QWidget* widget ) const
{
    QVERIFY2( widget != nullptr, "Drag target widget is null" );

    const QPoint start = widget->rect().center() + QPoint( -60, 40 );
    const QPoint end = start + QPoint( 140, -100 );
    const QPoint global_start = widget->mapToGlobal( start );
    const QPoint global_end = widget->mapToGlobal( end );

    focusScreen( widget );

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
}

void AnimationControlTest::pressScreenKey( QWidget* widget, Qt::Key key, Qt::KeyboardModifiers modifiers ) const
{
    focusScreen( widget );
    QTest::keyClick( widget, key, modifiers );
    QCoreApplication::processEvents();
}

int AnimationControlTest::screenshotCount() const
{
    const QDir capture_dir( m_capture_dir_path );
    const QString pattern = QStringLiteral( "%1_*.bmp" ).arg( m_capture_base_name );
    return capture_dir.entryInfoList( QStringList() << pattern, QDir::Files, QDir::Name ).size();
}

void AnimationControlTest::removeExistingScreenshots() const
{
    QDir capture_dir( m_capture_dir_path );
    const QString pattern = QStringLiteral( "%1_*.bmp" ).arg( m_capture_base_name );
    const QFileInfoList screenshots =
        capture_dir.entryInfoList( QStringList() << pattern, QDir::Files, QDir::Name );
    for ( const QFileInfo& file_info : screenshots )
    {
        QVERIFY2(
            capture_dir.remove( file_info.fileName() ),
            qPrintable( QStringLiteral( "Failed to remove existing screenshot: %1" ).arg( file_info.absoluteFilePath() ) ) );
    }
}

void AnimationControlTest::initTestCase()
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
        QStringLiteral( "/path/to/SampleData/ucd/old/out/spx.pfl" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "AnimationControlTest.mov" ) );
    m_anim_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "Test.anim" ) );
    m_capture_base_name = QStringLiteral( "Test" );
    m_capture_dir_path = QFileInfo( m_client_executable ).absolutePath();
    m_original_current_path = QDir::currentPath();

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
        QDir::setCurrent( m_capture_dir_path ),
        qPrintable( QStringLiteral( "Failed to change current directory to: %1" ).arg( m_capture_dir_path ) ) );

    if ( QFileInfo::exists( m_anim_file_path ) )
    {
        QVERIFY2(
            QFile::remove( m_anim_file_path ),
            qPrintable( QStringLiteral( "Failed to remove existing animation file: %1" ).arg( m_anim_file_path ) ) );
    }
    removeExistingScreenshots();

    m_server_process.setProgram( m_server_executable );
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.setProcessChannelMode( QProcess::MergedChannels );
    m_server_process.start();

    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
}

void AnimationControlTest::cleanupTestCase()
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

    if ( !m_original_current_path.isEmpty() )
    {
        QDir::setCurrent( m_original_current_path );
    }

}

void AnimationControlTest::performs_animation_control_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    startVideoRecording();
    logStep( QStringLiteral( "scenario: recording started" ) );

    MainWindow main_window( *g_test_app );
    main_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();
    client.animation_control->show();

    ensureConnected( client );

    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
        QVERIFY2(
            client.remote_viz_client_server_radio->isChecked(),
            "remoteVizClientServerRadioButton was not checked" );
    }
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );

    waitForObjectAndApply( client );
    clickJumpAndWaitForCompletion( client );
    waitForVisibleObject( client );

    client.capture_combo_box->setCurrentIndex( 0 );
    QCOMPARE( client.capture_combo_box->currentIndex(), 0 );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.image_file_line_edit, m_capture_base_name );
    QTest::qWait( k_short_wait_ms );

    pressScreenKey( client.screen_widget, Qt::Key_X );
    QTest::qWait( k_short_wait_ms );
    dragScreenLeftButton( client.screen_widget );
    QTest::qWait( k_short_wait_ms );
    pressScreenKey( client.screen_widget, Qt::Key_X );
    QTest::qWait( k_short_wait_ms );
    dragScreenLeftButton( client.screen_widget );
    QTest::qWait( k_short_wait_ms );
    pressScreenKey( client.screen_widget, Qt::Key_X );
    QTest::qWait( k_three_second_wait_ms );
    QCOMPARE( client.total_key_frames_display_label->text(), QStringLiteral( "3" ) );

    pressScreenKey( client.screen_widget, Qt::Key_D );
    QTest::qWait( k_three_second_wait_ms );
    QCOMPARE( client.total_key_frames_display_label->text(), QStringLiteral( "2" ) );

    pressScreenKey( client.screen_widget, Qt::Key_D, Qt::ShiftModifier );
    QTest::qWait( k_short_wait_ms );
    QCOMPARE( client.total_key_frames_display_label->text(), QStringLiteral( "0" ) );

    pressScreenKey( client.screen_widget, Qt::Key_X );
    QTest::qWait( k_short_wait_ms );
    dragScreenLeftButton( client.screen_widget );
    QTest::qWait( k_short_wait_ms );
    pressScreenKey( client.screen_widget, Qt::Key_X );
    QTest::qWait( k_short_wait_ms );
    dragScreenLeftButton( client.screen_widget );
    QTest::qWait( k_short_wait_ms );
    pressScreenKey( client.screen_widget, Qt::Key_X );
    QTest::qWait( k_three_second_wait_ms );
    QCOMPARE( client.total_key_frames_display_label->text(), QStringLiteral( "3" ) );

    logStep( QStringLiteral( "scenario: play animation" ) );
    QVERIFY2(
        QMetaObject::invokeMethod( client.animation_control, "onPlayKeyFrame", Qt::DirectConnection ),
        "Failed to invoke AnimationControl::onPlayKeyFrame" );
    QTest::qWait( k_three_second_wait_ms );
    QVERIFY2( screenshotCount() > 0, "No bitmap screenshots were created after playback" );

    logStep( QStringLiteral( "scenario: save animation file" ) );
    QTimer::singleShot(
        300,
        client.main_window,
        [this]()
        {
            selectFileFromDialog( m_anim_file_path, false );
        } );
    QVERIFY2(
        QMetaObject::invokeMethod( client.animation_control, "onSaveKeyFrameFile", Qt::DirectConnection ),
        "Failed to invoke AnimationControl::onSaveKeyFrameFile" );
    QTest::qWait( k_three_second_wait_ms );
    QVERIFY2(
        QFileInfo::exists( m_anim_file_path ),
        qPrintable( QStringLiteral( "Animation file was not created: %1" ).arg( m_anim_file_path ) ) );

    logStep( QStringLiteral( "scenario: clear animation keyframes" ) );
    QVERIFY2(
        QMetaObject::invokeMethod( client.animation_control, "onClearKeyFrame", Qt::DirectConnection ),
        "Failed to invoke AnimationControl::onClearKeyFrame" );
    QTest::qWait( k_three_second_wait_ms );
    QCOMPARE( client.total_key_frames_display_label->text(), QStringLiteral( "0" ) );

    logStep( QStringLiteral( "scenario: load animation file" ) );
    QTimer::singleShot(
        300,
        client.main_window,
        [this]()
        {
            selectFileFromDialog( m_anim_file_path, true );
        } );
    QVERIFY2(
        QMetaObject::invokeMethod( client.animation_control, "onLoadKeyFrameFile", Qt::DirectConnection ),
        "Failed to invoke AnimationControl::onLoadKeyFrameFile" );
    QTest::qWait( k_three_second_wait_ms );
    QCOMPARE( client.total_key_frames_display_label->text(), QStringLiteral( "3" ) );

    logStep( QStringLiteral( "scenario: replay animation" ) );
    QVERIFY2(
        QMetaObject::invokeMethod( client.animation_control, "onPlayKeyFrame", Qt::DirectConnection ),
        "Failed to invoke AnimationControl::onPlayKeyFrame" );
    QTest::qWait( k_three_second_wait_ms );
    client.interpolation_spin_box->setValue( 30 );
    QCOMPARE( client.interpolation_spin_box->value(), 30 );
    QTest::qWait( k_three_second_wait_ms );
    QVERIFY2(
        QMetaObject::invokeMethod( client.animation_control, "onPlayKeyFrame", Qt::DirectConnection ),
        "Failed to invoke AnimationControl::onPlayKeyFrame" );
    QTest::qWait( k_three_second_wait_ms );

    QVERIFY2(
        waitForCondition(
            [this]()
            {
                return screenshotCount() > 0;
            },
            5000,
            100 ),
        qPrintable( QStringLiteral( "No %1_*.bmp files were created in %2" ).arg( m_capture_base_name, m_capture_dir_path ) ) );
}
}
