#include "GuiScenarioTest.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLineEdit>
#include <QMainWindow>
#include <QModelIndex>
#include <QPixmap>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QTest>
#include <QTreeView>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_disable_timeout_ms = 5000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_screenshot_settle_ms = 1000;
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

QString GuiScenarioTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString GuiScenarioTest::defaultServerExecutablePath() const
{
#if defined( Q_OS_WIN )
    return sourceTreePath( QStringLiteral( "Server/x64/Release/pbvr_server.exe" ) );
#elif defined( Q_OS_MACOS )
    return sourceTreePath( QStringLiteral( "Server/pbvr_server" ) );
#else
    return sourceTreePath( QStringLiteral( "Server/pbvr_server" ) );
#endif
}

QString GuiScenarioTest::repoRootPath() const
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

QString GuiScenarioTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool GuiScenarioTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void GuiScenarioTest::initTestCase()
{
    m_server_executable = defaultServerExecutablePath();
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        QStringLiteral( "/Users/sakamoto/Work/SampleData/ucd/old/out/spx.pfl" ) );

    const QString default_screenshot =
        sourceTreePath(
            QStringLiteral( "pbvr_client_qtest_%1.png" )
                .arg( QDateTime::currentDateTime().toString( QStringLiteral( "yyyyMMdd_HHmmss" ) ) ) );
    m_screenshot_path = envOrDefault( "PBVR_SCREENSHOT_PATH", default_screenshot );

    QVERIFY2( QFileInfo::exists( m_server_executable ), qPrintable( QStringLiteral( "Server executable not found: %1" ).arg( m_server_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

    m_server_process.setProgram( m_server_executable );
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.start();

    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
}

void GuiScenarioTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }
}

void GuiScenarioTest::performs_remote_viz_jump_scenario()
{
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    main_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    auto* communication = main_window.findChild<Communication*>();
    auto* object_editor = main_window.findChild<ObjectEditor*>();
    auto* playback_tool_bar = main_window.findChild<PlayBackControlToolBar*>();

    QVERIFY2( communication != nullptr, "Communication dock not found" );
    QVERIFY2( object_editor != nullptr, "ObjectEditor dock not found" );
    QVERIFY2( playback_tool_bar != nullptr, "Playback toolbar not found" );

    communication->show();
    object_editor->show();
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( 250 );

    auto* connect_button = communication->findChild<QPushButton*>( "connectPushButton" );
    auto* volume_path_edit = communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    auto* remote_viz_radio = communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    auto* setting_apply_button = communication->findChild<QPushButton*>( "settingApplyPushButton" );

    QVERIFY2( connect_button != nullptr, "connectPushButton not found" );
    QVERIFY2( volume_path_edit != nullptr, "volumeDataFilePathLineEdit not found" );
    QVERIFY2( remote_viz_radio != nullptr, "remoteVizClientServerRadioButton not found" );
    QVERIFY2( setting_apply_button != nullptr, "settingApplyPushButton not found" );

    QTest::mouseClick( connect_button, Qt::LeftButton );

    const bool connected = waitForCondition(
        [communication]()
        {
            if ( auto* disconnect_button = communication->findChild<QPushButton*>( "disconnectPushButton" ) )
            {
                return disconnect_button->isEnabled();
            }
            return false;
        },
        k_connect_timeout_ms );
    QVERIFY2( connected, "Client did not enter the connected state" );

    volume_path_edit->setFocus();
    volume_path_edit->clear();
    QTest::keyClicks( volume_path_edit, QDir::toNativeSeparators( m_volume_data_path ) );
    QCOMPARE( volume_path_edit->text(), QDir::toNativeSeparators( m_volume_data_path ) );

    if ( !remote_viz_radio->isChecked() )
    {
        QTest::mouseClick( remote_viz_radio, Qt::LeftButton );
    }
    QVERIFY( remote_viz_radio->isChecked() );

    QTest::mouseClick( setting_apply_button, Qt::LeftButton );

    auto* tree_view = object_editor->findChild<QTreeView*>( "treeView" );
    auto* apply_button = object_editor->findChild<QPushButton*>( "applyPushButton" );

    QVERIFY2( tree_view != nullptr, "ObjectEditor treeView not found" );
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
    QVERIFY( tree_view->currentIndex().isValid() );

    QTest::mouseClick( apply_button, Qt::LeftButton );

    auto* jump_button = playback_tool_bar->jumpButton();
    QVERIFY2( jump_button != nullptr, "m_jump_push_button not found" );
    QVERIFY2( jump_button->isEnabled(), "m_jump_push_button is disabled" );

    QTest::mouseClick( jump_button, Qt::LeftButton );

    const bool became_disabled = waitForCondition(
        [jump_button]()
        {
            return !jump_button->isEnabled();
        },
        k_jump_button_disable_timeout_ms );
    QVERIFY2( became_disabled, "m_jump_push_button did not become disabled after click" );

    const bool became_enabled_again = waitForCondition(
        [jump_button]()
        {
            return jump_button->isEnabled();
        },
        k_jump_button_enable_timeout_ms,
        200 );
    QVERIFY2( became_enabled_again, "m_jump_push_button did not become enabled again within the timeout" );

    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_screenshot_settle_ms );

    QVERIFY2( main_window.windowHandle() != nullptr, "Main window has no window handle" );
    QVERIFY2( main_window.windowHandle()->screen() != nullptr, "Main window is not attached to a screen" );

    const QPixmap screenshot = main_window.grab();
    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the main window" );
    QVERIFY2( screenshot.save( m_screenshot_path ), qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( m_screenshot_path ) ) );

    qInfo() << "Saved screenshot to" << QDir::toNativeSeparators( m_screenshot_path );
}

int main( int argc, char** argv )
{
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    GuiScenarioTest test;
    return QTest::qExec( &test, argc, argv );
}
