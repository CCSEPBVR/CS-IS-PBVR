#include "PlayBackControlToolBarTest.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QTreeView>
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
    return value.isEmpty() ? fallback : value;
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

    const auto spin_boxes = tool_bar->findChildren<QSpinBox*>();
    for ( QSpinBox* spin_box : spin_boxes )
    {
        if ( spin_box && spin_box->suffix().isEmpty() && spin_box->maximum() >= 5 )
        {
            return spin_box;
        }
    }

    return nullptr;
}

void PlayBackControlToolBarTest::startVideoRecording()
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

void PlayBackControlToolBarTest::stopVideoRecording()
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

void PlayBackControlToolBarTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" ) );
    m_object_file_path = envOrDefault(
        "PBVR_OBJECT_FILE",
        QStringLiteral( "/path/to/SampleData/stl/clock/clock_00000.stl" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "PlayBackControlToolBarTest.mov" ) );

    QVERIFY2(
        QFileInfo::exists( m_client_executable ),
        qPrintable( QStringLiteral( "Client executable not found: %1" ).arg( m_client_executable ) ) );
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
    main_window.show();
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

    startVideoRecording();
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
