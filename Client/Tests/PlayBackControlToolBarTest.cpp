#include "PlayBackControlToolBarTest.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <windows.h>
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QPoint>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QRect>
#include <QScreen>
#include <QString>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QtGlobal>
#include <QWindow>
#include <QWidget>
#include <QTest>

#include <csignal>
#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_file_dialog_timeout_ms = 5000;
constexpr int k_window_settle_ms = 500;
constexpr int k_post_connect_settle_ms = 1000;
constexpr int k_post_apply_settle_ms = 3000;
constexpr int k_action_wait_ms = 2000;
constexpr int k_recording_finish_timeout_ms = 15000;
kvs::qt::Application* g_test_app = nullptr;

#ifdef Q_OS_WIN
QRect physicalWindowGeometryForRecording( QWidget* target_window, QScreen* fallback_screen )
{
    if ( target_window != nullptr && target_window->windowHandle() != nullptr )
    {
        const HWND hwnd = reinterpret_cast<HWND>( target_window->windowHandle()->winId() );
        RECT window_rect;
        if ( hwnd != nullptr && GetWindowRect( hwnd, &window_rect ) )
        {
            QRect physical_window_geometry(
                window_rect.left,
                window_rect.top,
                window_rect.right - window_rect.left,
                window_rect.bottom - window_rect.top );

            const HMONITOR monitor = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST );
            if ( monitor != nullptr )
            {
                MONITORINFO monitor_info;
                monitor_info.cbSize = sizeof( MONITORINFO );
                if ( GetMonitorInfo( monitor, &monitor_info ) )
                {
                    const RECT monitor_rect = monitor_info.rcMonitor;
                    const QRect physical_monitor_geometry(
                        monitor_rect.left,
                        monitor_rect.top,
                        monitor_rect.right - monitor_rect.left,
                        monitor_rect.bottom - monitor_rect.top );
                    physical_window_geometry = physical_window_geometry.intersected( physical_monitor_geometry );
                }
            }

            physical_window_geometry.setWidth( physical_window_geometry.width() & ~1 );
            physical_window_geometry.setHeight( physical_window_geometry.height() & ~1 );
            return physical_window_geometry;
        }
    }

    QScreen* screen = fallback_screen;
    if ( target_window != nullptr && target_window->windowHandle() != nullptr &&
         target_window->windowHandle()->screen() != nullptr )
    {
        screen = target_window->windowHandle()->screen();
    }

    if ( screen == nullptr )
    {
        return QRect();
    }

    const QRect logical_geometry =
        target_window != nullptr ? target_window->frameGeometry() : screen->geometry();
    const qreal device_pixel_ratio = screen->devicePixelRatio();

    // Qt reports window geometry in logical pixels while ffmpeg gdigrab uses
    // physical desktop pixels. Convert explicitly for scaled displays such as
    // Windows 150% scaling.
    QRect physical_geometry(
        qRound( logical_geometry.x() * device_pixel_ratio ),
        qRound( logical_geometry.y() * device_pixel_ratio ),
        qRound( logical_geometry.width() * device_pixel_ratio ),
        qRound( logical_geometry.height() * device_pixel_ratio ) );
    physical_geometry.setWidth( physical_geometry.width() & ~1 );
    physical_geometry.setHeight( physical_geometry.height() & ~1 );
    return physical_geometry;
}
#endif

#ifdef Q_OS_LINUX
QRect physicalWindowGeometryForRecording( QWidget* target_window, QScreen* fallback_screen )
{
    QScreen* screen = fallback_screen;
    if ( target_window != nullptr && target_window->windowHandle() != nullptr &&
         target_window->windowHandle()->screen() != nullptr )
    {
        screen = target_window->windowHandle()->screen();
    }

    if ( screen == nullptr )
    {
        return QRect();
    }

    const QRect logical_geometry =
        target_window != nullptr ? target_window->frameGeometry() : screen->geometry();
    const qreal device_pixel_ratio = screen->devicePixelRatio();

    QRect physical_geometry(
        qRound( logical_geometry.x() * device_pixel_ratio ),
        qRound( logical_geometry.y() * device_pixel_ratio ),
        qRound( logical_geometry.width() * device_pixel_ratio ),
        qRound( logical_geometry.height() * device_pixel_ratio ) );
    physical_geometry.setWidth( physical_geometry.width() & ~1 );
    physical_geometry.setHeight( physical_geometry.height() & ~1 );
    return physical_geometry;
}
#endif

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

QString PlayBackControlToolBarTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString PlayBackControlToolBarTest::repoRootPath() const
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

QString PlayBackControlToolBarTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool PlayBackControlToolBarTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

QFileDialog* PlayBackControlToolBarTest::waitForFileDialog( int timeout_ms ) const
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

void PlayBackControlToolBarTest::selectFileFromDialog( const QString& file_path ) const
{
    QFileDialog* dialog = waitForFileDialog( k_file_dialog_timeout_ms );
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
            if ( index.data().toString() == file_info.fileName() )
            {
                list_view->selectionModel()->setCurrentIndex(
                    index,
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                break;
            }
        }
    }

    if ( auto* tree_view = dialog->findChild<QTreeView*>() )
    {
        const QModelIndex root_index = tree_view->rootIndex();
        const int rows = tree_view->model() ? tree_view->model()->rowCount( root_index ) : 0;
        for ( int row = 0; row < rows; ++row )
        {
            const QModelIndex index = tree_view->model()->index( row, 0, root_index );
            if ( index.data().toString() == file_info.fileName() )
            {
                tree_view->selectionModel()->setCurrentIndex(
                    index,
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                break;
            }
        }
    }

    QMetaObject::invokeMethod( dialog, "accept", Qt::QueuedConnection );
}

void PlayBackControlToolBarTest::clickButtonAndWait( QPushButton* button, int wait_ms ) const
{
    QVERIFY2( button != nullptr, "Target button was not found" );
    QVERIFY2(
        waitForCondition(
            [button]()
            {
                return button->isEnabled();
            },
            k_jump_button_enable_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "Button did not become enabled: %1" ).arg( button->objectName() ) ) );

    QTest::mouseClick( button, Qt::LeftButton );
    QTest::qWait( wait_ms );
}

QSpinBox* PlayBackControlToolBarTest::findNextTimeStepSpinBox( QToolBar* tool_bar ) const
{
    if ( tool_bar == nullptr ) { return nullptr; }

    return tool_bar->findChild<QSpinBox*>( "m_next_time_step_spin_box" );
}

void PlayBackControlToolBarTest::startVideoRecording( QWidget* target_window )
{
    if ( QFileInfo::exists( m_video_file_path ) )
    {
        QVERIFY2(
            QFile::remove( m_video_file_path ),
            qPrintable( QStringLiteral( "Failed to remove existing video: %1" ).arg( m_video_file_path ) ) );
    }

#if defined( Q_OS_WIN ) || defined( Q_OS_LINUX )
    if ( target_window != nullptr )
    {
        target_window->raise();
        target_window->activateWindow();
        QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
    }

    const QString ffmpeg_path = envOrDefault(
        "PBVR_FFMPEG_EXECUTABLE",
        QStandardPaths::findExecutable( QStringLiteral( "ffmpeg" ) ) );
    if ( ffmpeg_path.isEmpty() )
    {
        qWarning().noquote()
            << QStringLiteral( "Video recording skipped: ffmpeg was not found. "
                               "Set PBVR_FFMPEG_EXECUTABLE or add ffmpeg to PATH." );
        m_video_recording_available = false;
        return;
    }

    QScreen* target_screen = nullptr;
    if ( target_window != nullptr )
    {
        if ( target_window->windowHandle() != nullptr )
        {
            target_screen = target_window->windowHandle()->screen();
        }
        if ( target_screen == nullptr )
        {
            const QPoint window_center = target_window->frameGeometry().center();
            target_screen = QGuiApplication::screenAt( window_center );
        }
        if ( target_screen == nullptr )
        {
            target_screen = target_window->screen();
        }
    }
    if ( target_screen == nullptr )
    {
        target_screen = QGuiApplication::primaryScreen();
    }

    QVERIFY2( target_screen != nullptr, "Screen for video recording was not found" );

    const QRect available_geometry = target_screen->availableGeometry();
    if ( target_window != nullptr )
    {
        QRect frame = target_window->frameGeometry();
        if ( !available_geometry.contains( frame ) )
        {
            QSize target_size = target_window->size();
            const int frame_extra_width = frame.width() - target_window->width();
            const int frame_extra_height = frame.height() - target_window->height();
            target_size.setWidth(
                qMin( target_size.width(), qMax( 320, available_geometry.width() - frame_extra_width - 40 ) ) );
            target_size.setHeight(
                qMin( target_size.height(), qMax( 240, available_geometry.height() - frame_extra_height - 40 ) ) );
            target_window->resize( target_size );
            QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );

            frame = target_window->frameGeometry();
            QPoint top_left = available_geometry.center() - QPoint( frame.width() / 2, frame.height() / 2 );
            const int max_x = qMax( available_geometry.left(), available_geometry.right() - frame.width() + 1 );
            const int max_y = qMax( available_geometry.top(), available_geometry.bottom() - frame.height() + 1 );
            top_left.setX( qBound( available_geometry.left(), top_left.x(), max_x ) );
            top_left.setY( qBound( available_geometry.top(), top_left.y(), max_y ) );
            target_window->move( top_left );
            QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
        }
    }

    const QRect recording_geometry =
        physicalWindowGeometryForRecording( target_window, target_screen );
    QVERIFY2( recording_geometry.isValid(), "Window geometry for video recording was not found" );
    qInfo().noquote()
        << QStringLiteral( "Recording window logical geometry: %1,%2 %3x%4 DPR:%5 physical geometry: %6,%7 %8x%9" )
               .arg( target_window != nullptr ? target_window->frameGeometry().x() : target_screen->geometry().x() )
               .arg( target_window != nullptr ? target_window->frameGeometry().y() : target_screen->geometry().y() )
               .arg( target_window != nullptr ? target_window->frameGeometry().width() : target_screen->geometry().width() )
               .arg( target_window != nullptr ? target_window->frameGeometry().height() : target_screen->geometry().height() )
               .arg( target_screen->devicePixelRatio() )
               .arg( recording_geometry.x() )
               .arg( recording_geometry.y() )
               .arg( recording_geometry.width() )
               .arg( recording_geometry.height() );

    m_recording_process.setProgram( ffmpeg_path );
#ifdef Q_OS_WIN
    m_recording_process.setArguments(
        {
            QStringLiteral( "-y" ),
            QStringLiteral( "-f" ),
            QStringLiteral( "gdigrab" ),
            QStringLiteral( "-framerate" ),
            QStringLiteral( "30" ),
            QStringLiteral( "-offset_x" ),
            QString::number( recording_geometry.x() ),
            QStringLiteral( "-offset_y" ),
            QString::number( recording_geometry.y() ),
            QStringLiteral( "-video_size" ),
            QStringLiteral( "%1x%2" ).arg( recording_geometry.width() ).arg( recording_geometry.height() ),
            QStringLiteral( "-i" ),
            QStringLiteral( "desktop" ),
            QStringLiteral( "-vcodec" ),
            QStringLiteral( "libx264" ),
            QStringLiteral( "-pix_fmt" ),
            QStringLiteral( "yuv420p" ),
            QStringLiteral( "-movflags" ),
            QStringLiteral( "+faststart" ),
            m_video_file_path
        } );
#else
    const QString display = qEnvironmentVariable(
        "PBVR_FFMPEG_X11_DISPLAY",
        qEnvironmentVariable( "DISPLAY" ) );
    QVERIFY2(
        !display.isEmpty(),
        "DISPLAY was not set. Linux video recording with ffmpeg requires an X11 display." );

    m_recording_process.setArguments(
        {
            QStringLiteral( "-y" ),
            QStringLiteral( "-f" ),
            QStringLiteral( "x11grab" ),
            QStringLiteral( "-framerate" ),
            QStringLiteral( "30" ),
            QStringLiteral( "-video_size" ),
            QStringLiteral( "%1x%2" ).arg( recording_geometry.width() ).arg( recording_geometry.height() ),
            QStringLiteral( "-i" ),
            QStringLiteral( "%1+%2,%3" ).arg( display ).arg( recording_geometry.x() ).arg( recording_geometry.y() ),
            QStringLiteral( "-vcodec" ),
            QStringLiteral( "libx264" ),
            QStringLiteral( "-pix_fmt" ),
            QStringLiteral( "yuv420p" ),
            QStringLiteral( "-movflags" ),
            QStringLiteral( "+faststart" ),
            m_video_file_path
        } );
#endif
#else
    m_recording_process.setProgram( QStringLiteral( "screencapture" ) );
    m_recording_process.setArguments(
        {
            QStringLiteral( "-v" ),
            QStringLiteral( "-k" ),
            QStringLiteral( "-m" ),
            QStringLiteral( "-x" ),
            m_video_file_path
        } );
#endif

    m_video_recording_available = true;
    m_recording_process.setProcessChannelMode( QProcess::MergedChannels );
    m_recording_process.start();

    if ( !m_recording_process.waitForStarted( 5000 ) )
    {
        m_video_recording_available = false;
        QVERIFY2(
            false,
            qPrintable( QStringLiteral( "Failed to start video recording: %1" ).arg( m_recording_process.errorString() ) ) );
    }
}

void PlayBackControlToolBarTest::stopVideoRecording()
{
#if defined( Q_OS_WIN ) || defined( Q_OS_LINUX )
    if ( !m_video_recording_available )
    {
        return;
    }
#endif

    if ( m_recording_process.state() == QProcess::NotRunning )
    {
        const QFileInfo video_file_info( m_video_file_path );
        QVERIFY2(
            video_file_info.exists() && video_file_info.size() > 0,
            qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
        return;
    }

#ifdef Q_OS_WIN
    m_recording_process.write( "q" );
    m_recording_process.closeWriteChannel();
#else
    const qint64 pid = m_recording_process.processId();
    if ( pid > 0 )
    {
        ::kill( static_cast<pid_t>( pid ), SIGINT );
    }
    else
    {
        m_recording_process.terminate();
    }
#endif

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

    const QString recorder_output = QString::fromLocal8Bit( m_recording_process.readAll() ).trimmed();
    if ( !recorder_output.isEmpty() )
    {
        qInfo().noquote() << recorder_output;
    }

    const QFileInfo video_file_info( m_video_file_path );
    QVERIFY2(
        video_file_info.exists() && video_file_info.size() > 0,
        qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
}

void PlayBackControlToolBarTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() ) );
    m_object_file_path = envOrDefault(
        "CLOCK_POLYGON_DATA",
        ClientTests::configuredPath( "CLOCK_POLYGON_DATA", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "PlayBackControlToolBarTest.mp4" ) );

    QVERIFY2(
        QFileInfo::exists( m_object_file_path ),
        qPrintable( QStringLiteral( "Object file not found: %1" ).arg( m_object_file_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
}

void PlayBackControlToolBarTest::cleanupTestCase()
{
    if ( m_recording_process.state() != QProcess::NotRunning )
    {
        stopVideoRecording();
    }
}

void PlayBackControlToolBarTest::performs_playback_control_toolbar_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    auto* object_editor = main_window.findChild<ObjectEditor*>();
    auto* playback_tool_bar = main_window.findChild<::PlayBackControlToolBar*>();
    auto* time_step_tool_bar = main_window.findChild<TimeStepControlToolBar*>();

    QVERIFY2( object_editor != nullptr, "ObjectEditor dock not found" );
    QVERIFY2( playback_tool_bar != nullptr, "PlayBackControlToolBar not found" );
    QVERIFY2( time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" );

    object_editor->show();
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_window_settle_ms );

    object_editor->onOperatorStateUpdate( true );
    time_step_tool_bar->onOperatorStateUpdate( true );
    playback_tool_bar->onOperatorStateUpdate( true );
    time_step_tool_bar->onUpdateTotalTimeStepRange( 0, 5, false );
    playback_tool_bar->onDataRequestCompleted();
    QTest::qWait( k_post_connect_settle_ms );

    auto* browse_button = object_editor->findChild<QPushButton*>( "browsePushButton" );
    auto* name_line_edit = object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    auto* apply_button = object_editor->findChild<QPushButton*>( "applyPushButton" );

    QVERIFY2( browse_button != nullptr, "browsePushButton not found" );
    QVERIFY2( name_line_edit != nullptr, "nameLineEdit not found" );
    QVERIFY2( apply_button != nullptr, "applyPushButton not found" );

    QTimer::singleShot(
        0,
        &main_window,
        [this]()
        {
            selectFileFromDialog( m_object_file_path );
        } );
    QTest::mouseClick( browse_button, Qt::LeftButton );

    const bool name_loaded = waitForCondition(
        [name_line_edit]()
        {
            return !name_line_edit->text().trimmed().isEmpty();
        },
        k_object_load_timeout_ms,
        100 );
    QVERIFY2( name_loaded, "ObjectEditor nameLineEdit was not populated within the timeout" );

    QTest::mouseClick( apply_button, Qt::LeftButton );
    QTest::qWait( k_post_apply_settle_ms );

    auto* jump_button = playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    auto* last_button = playback_tool_bar->findChild<QPushButton*>( "m_last_push_button" );
    auto* first_button = playback_tool_bar->findChild<QPushButton*>( "m_first_push_button" );
    auto* next_button = playback_tool_bar->findChild<QPushButton*>( "m_next_push_button" );
    auto* previous_button = playback_tool_bar->findChild<QPushButton*>( "m_previous_push_button" );
    auto* play_button = playback_tool_bar->findChild<QPushButton*>( "m_play_push_button" );
    auto* reverse_button = playback_tool_bar->findChild<QPushButton*>( "m_reverse_push_button" );
    auto* loop_button = playback_tool_bar->findChild<QPushButton*>( "m_loop_push_button" );
    auto* next_time_step_spin_box = findNextTimeStepSpinBox( time_step_tool_bar );

    QVERIFY2( jump_button != nullptr, "m_jump_push_button not found" );
    QVERIFY2( last_button != nullptr, "m_last_push_button not found" );
    QVERIFY2( first_button != nullptr, "m_first_push_button not found" );
    QVERIFY2( next_button != nullptr, "m_next_push_button not found" );
    QVERIFY2( previous_button != nullptr, "m_previous_push_button not found" );
    QVERIFY2( play_button != nullptr, "m_play_push_button not found" );
    QVERIFY2( reverse_button != nullptr, "m_reverse_push_button not found" );
    QVERIFY2( loop_button != nullptr, "m_loop_push_button not found" );
    QVERIFY2( next_time_step_spin_box != nullptr, "m_next_time_step_spin_box not found" );

    const bool jump_enabled = waitForCondition(
        [jump_button]()
        {
            return jump_button->isEnabled();
        },
        k_jump_button_enable_timeout_ms,
        200 );
    QVERIFY2( jump_enabled, "m_jump_push_button did not become enabled within the timeout" );

    clickButtonAndWait( jump_button, k_action_wait_ms );

    startVideoRecording( &main_window );
    QTest::qWait( k_action_wait_ms );

    clickButtonAndWait( last_button, k_action_wait_ms );
    clickButtonAndWait( first_button, k_action_wait_ms );
    clickButtonAndWait( next_button, k_action_wait_ms );
    clickButtonAndWait( previous_button, k_action_wait_ms );
    clickButtonAndWait( play_button, k_action_wait_ms );
    clickButtonAndWait( play_button, k_action_wait_ms );
    clickButtonAndWait( reverse_button, k_action_wait_ms );
    clickButtonAndWait( reverse_button, k_action_wait_ms );

    QVERIFY2(
        waitForCondition(
            [next_time_step_spin_box]()
            {
                return next_time_step_spin_box->isEnabled() && next_time_step_spin_box->maximum() >= 5;
            },
            k_jump_button_enable_timeout_ms,
            100 ),
        "m_next_time_step_spin_box did not become ready" );
    next_time_step_spin_box->setValue( 5 );
    QCOMPARE( next_time_step_spin_box->value(), 5 );
    QTest::qWait( k_action_wait_ms );

    clickButtonAndWait( jump_button, k_action_wait_ms );
    clickButtonAndWait( loop_button, k_action_wait_ms );
    clickButtonAndWait( play_button, k_action_wait_ms );
    clickButtonAndWait( play_button, k_action_wait_ms );
    clickButtonAndWait( loop_button, k_action_wait_ms );

    stopVideoRecording();

    QVERIFY2(
        waitForCondition(
            [jump_button]()
            {
                return jump_button->isEnabled();
            },
            k_jump_button_enable_timeout_ms,
            100 ),
        "m_jump_push_button did not become enabled before teardown" );

    main_window.close();
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
    ClientTests::PlayBackControlToolBarTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
