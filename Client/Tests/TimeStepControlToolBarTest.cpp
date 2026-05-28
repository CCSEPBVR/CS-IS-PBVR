#include "TimeStepControlToolBarTest.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <windows.h>
#endif

#include <QDebug>
#include <QEventLoop>
#include <QGuiApplication>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QStandardPaths>
#include <QString>
#include <QtGlobal>
#include <QWindow>

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QPoint>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
#include <QWidget>
#include <QTest>

#include <algorithm>
#include <csignal>

#include "../App/MainWindow.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_button_enable_timeout_ms = 120000;
constexpr int k_file_dialog_timeout_ms = 5000;
constexpr int k_window_settle_ms = 500;
constexpr int k_post_connect_settle_ms = 1000;
constexpr int k_post_apply_settle_ms = 3000;
constexpr int k_action_wait_ms = 2000;
constexpr int k_long_action_wait_ms = 5000;
constexpr int k_request_complete_delay_ms = 300;
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

QString TimeStepControlToolBarTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString TimeStepControlToolBarTest::repoRootPath() const
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

QString TimeStepControlToolBarTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool TimeStepControlToolBarTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

QFileDialog* TimeStepControlToolBarTest::waitForFileDialog( int timeout_ms ) const
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

void TimeStepControlToolBarTest::selectFileFromDialog( const QString& file_path ) const
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

void TimeStepControlToolBarTest::clickButtonAndWait( QPushButton* button, int wait_ms ) const
{
    QVERIFY2( button != nullptr, "Target button was not found" );
    QVERIFY2(
        waitForCondition(
            [button]()
            {
                return button->isEnabled();
            },
            k_button_enable_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "Button did not become enabled: %1" ).arg( button->objectName() ) ) );

    QTest::mouseClick( button, Qt::LeftButton );
    QTest::qWait( wait_ms );
}

QList<QSpinBox*> TimeStepControlToolBarTest::findSpinBoxes( QToolBar* tool_bar ) const
{
    QList<QSpinBox*> spin_boxes;
    if ( tool_bar == nullptr ) { return spin_boxes; }

    const auto candidates = tool_bar->findChildren<QSpinBox*>();
    for ( QSpinBox* spin_box : candidates )
    {
        if ( spin_box )
        {
            spin_boxes.push_back( spin_box );
        }
    }

    std::sort(
        spin_boxes.begin(),
        spin_boxes.end(),
        []( QSpinBox* lhs, QSpinBox* rhs )
        {
            const QPoint lhs_pos = lhs->mapToGlobal( QPoint( 0, 0 ) );
            const QPoint rhs_pos = rhs->mapToGlobal( QPoint( 0, 0 ) );
            if ( lhs_pos.y() != rhs_pos.y() ) { return lhs_pos.y() < rhs_pos.y(); }
            return lhs_pos.x() < rhs_pos.x();
        } );

    return spin_boxes;
}

void TimeStepControlToolBarTest::startVideoRecording( QWidget* target_window )
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

void TimeStepControlToolBarTest::stopVideoRecording()
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
    m_recording_process.write( "q\n" );
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
#ifdef Q_OS_WIN
        QVERIFY2(
            false,
            qPrintable( QStringLiteral( "Video recorder did not finish after quit request: %1" )
                            .arg( m_recording_process.errorString() ) ) );
#else
        m_recording_process.terminate();
#endif
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

void TimeStepControlToolBarTest::initTestCase()
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
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TimeStepControlToolBarTest.mp4" ) );

    QVERIFY2(
        QFileInfo::exists( m_object_file_path ),
        qPrintable( QStringLiteral( "Object file not found: %1" ).arg( m_object_file_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
}

void TimeStepControlToolBarTest::cleanupTestCase()
{
    if ( m_recording_process.state() != QProcess::NotRunning )
    {
        stopVideoRecording();
    }
}

void TimeStepControlToolBarTest::performs_time_step_control_toolbar_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    QCoreApplication::processEvents();

    auto* object_editor = main_window.findChild<ObjectEditor*>();
    auto* playback_tool_bar = main_window.findChild<::PlayBackControlToolBar*>();
    auto* time_step_tool_bar = main_window.findChild<TimeStepControlToolBar*>();

    QVERIFY2( object_editor != nullptr, "ObjectEditor dock not found" );
    QVERIFY2( playback_tool_bar != nullptr, "PlayBackControlToolBar not found" );
    QVERIFY2( time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" );

    QObject::connect(
        time_step_tool_bar,
        &TimeStepControlToolBar::requestDataAt,
        &main_window,
        [time_step_tool_bar]( int request_time_step )
        {
            QTimer::singleShot(
                k_request_complete_delay_ms,
                time_step_tool_bar,
                [time_step_tool_bar, request_time_step]()
                {
                    time_step_tool_bar->onDataRequestCompleted( request_time_step );
                } );
        } );

    object_editor->show();
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_window_settle_ms );

    object_editor->onOperatorStateUpdate( true );
    time_step_tool_bar->onOperatorStateUpdate( true );
    playback_tool_bar->onOperatorStateUpdate( true );
    time_step_tool_bar->onUpdateTotalTimeStepRange( 0, 11, false );
    time_step_tool_bar->onDataRequestCompleted( 0 );
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
    auto* first_button = playback_tool_bar->findChild<QPushButton*>( "m_first_push_button" );
    auto* last_button = playback_tool_bar->findChild<QPushButton*>( "m_last_push_button" );
    auto* loop_button = playback_tool_bar->findChild<QPushButton*>( "m_loop_push_button" );
    auto* play_button = playback_tool_bar->findChild<QPushButton*>( "m_play_push_button" );

    QVERIFY2( jump_button != nullptr, "m_jump_push_button not found" );
    QVERIFY2( first_button != nullptr, "m_first_push_button not found" );
    QVERIFY2( last_button != nullptr, "m_last_push_button not found" );
    QVERIFY2( loop_button != nullptr, "m_loop_push_button not found" );
    QVERIFY2( play_button != nullptr, "m_play_push_button not found" );

    const QList<QSpinBox*> spin_boxes = findSpinBoxes( time_step_tool_bar );
    QVERIFY2( spin_boxes.size() >= 4, "TimeStepControlToolBar spin boxes were not found" );

    QSpinBox* next_time_step_spin_box = nullptr;
    QSpinBox* min_limit_time_step_spin_box = nullptr;
    QSpinBox* max_limit_time_step_spin_box = nullptr;
    QSpinBox* update_interval_spin_box = nullptr;

    for ( QSpinBox* spin_box : spin_boxes )
    {
        if ( spin_box->suffix() == QStringLiteral( "ms" ) )
        {
            update_interval_spin_box = spin_box;
            continue;
        }

        if ( next_time_step_spin_box == nullptr )
        {
            next_time_step_spin_box = spin_box;
        }
        else if ( min_limit_time_step_spin_box == nullptr )
        {
            min_limit_time_step_spin_box = spin_box;
        }
        else if ( max_limit_time_step_spin_box == nullptr )
        {
            max_limit_time_step_spin_box = spin_box;
        }
    }

    QVERIFY2( next_time_step_spin_box != nullptr, "m_next_time_step_spin_box not found" );
    QVERIFY2( min_limit_time_step_spin_box != nullptr, "m_min_limit_time_step_spin_box not found" );
    QVERIFY2( max_limit_time_step_spin_box != nullptr, "m_max_limit_time_step_spin_box not found" );
    QVERIFY2( update_interval_spin_box != nullptr, "m_update_interval_spin_box not found" );

    clickButtonAndWait( jump_button, k_request_complete_delay_ms );

    const bool jump_reenabled = waitForCondition(
        [jump_button]()
        {
            return jump_button->isEnabled();
        },
        k_button_enable_timeout_ms,
        100 );
    QVERIFY2( jump_reenabled, "m_jump_push_button did not become enabled within the timeout" );

    startVideoRecording( &main_window );

    QTest::qWait( k_action_wait_ms );
    next_time_step_spin_box->setValue( 5 );
    QCOMPARE( next_time_step_spin_box->value(), 5 );

    QTest::qWait( k_action_wait_ms );
    min_limit_time_step_spin_box->setValue( 3 );
    QTRY_COMPARE( min_limit_time_step_spin_box->value(), 3 );

    QTest::qWait( k_action_wait_ms );
    max_limit_time_step_spin_box->setValue( 8 );
    QTRY_COMPARE( max_limit_time_step_spin_box->value(), 8 );

    QTest::qWait( k_action_wait_ms );
    clickButtonAndWait( jump_button, k_request_complete_delay_ms );
    QVERIFY2(
        waitForCondition(
            [jump_button]()
            {
                return jump_button->isEnabled();
            },
            k_button_enable_timeout_ms,
            100 ),
        "m_jump_push_button did not become enabled after jump" );

    QTest::qWait( k_action_wait_ms );
    clickButtonAndWait( first_button, k_request_complete_delay_ms );

    QTest::qWait( k_action_wait_ms );
    clickButtonAndWait( last_button, k_request_complete_delay_ms );

    QTest::qWait( k_action_wait_ms );
    min_limit_time_step_spin_box->setValue( 0 );
    QTRY_COMPARE( min_limit_time_step_spin_box->value(), 0 );

    QTest::qWait( k_action_wait_ms );
    max_limit_time_step_spin_box->setValue( 11 );
    QTRY_COMPARE( max_limit_time_step_spin_box->value(), 11 );

    QTest::qWait( k_action_wait_ms );
    clickButtonAndWait( loop_button, k_action_wait_ms );

    QTest::qWait( k_action_wait_ms );
    clickButtonAndWait( play_button, k_action_wait_ms );

    QTest::qWait( k_action_wait_ms );
    update_interval_spin_box->setValue( 50 );
    QTRY_COMPARE( update_interval_spin_box->value(), 50 );

    QTest::qWait( k_long_action_wait_ms );
    update_interval_spin_box->setValue( 100 );
    QTRY_COMPARE( update_interval_spin_box->value(), 100 );

    QTest::qWait( k_long_action_wait_ms );
    clickButtonAndWait( play_button, k_action_wait_ms );

    QTest::qWait( k_action_wait_ms );
    clickButtonAndWait( loop_button, k_action_wait_ms );

    QTest::qWait( k_action_wait_ms );
    stopVideoRecording();
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::TimeStepControlToolBarTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
