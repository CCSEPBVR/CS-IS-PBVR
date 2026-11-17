#include "InSituModeTest.h"

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFileInfo>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QTest>
#include <QTreeView>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_tree_view_timeout_ms = 120000;
constexpr int k_jump_button_timeout_ms = 180000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_capture_settle_ms = 700;
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

InSituModeTest::InSituModeTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArrayLiteral( "900000" ) );
}

QString InSituModeTest::repoRootPath() const
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

bool InSituModeTest::waitForCondition(
    const std::function<bool()>& condition,
    int timeout_ms,
    int interval_ms ) const
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

void InSituModeTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void InSituModeTest::saveScreenshot() const
{
    if ( !ClientTests::screenshotsEnabled() ) { return; }

    QTest::qWait( k_capture_settle_ms );

    QScreen* screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen != nullptr, "Primary screen not found" );

    const QPixmap screenshot = screen->grabWindow( 0 );
    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the primary screen" );
    QVERIFY2(
        screenshot.save( m_screenshot_path ),
        qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( m_screenshot_path ) ) );

    qInfo().noquote() << QStringLiteral( "Screenshot: %1" ).arg( m_screenshot_path );
}

void InSituModeTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_output_dir_path = ClientTests::datedTestOutputDir(
        repoRootPath(),
        date_stamp,
        QStringLiteral( "InSituModeTest" ) );
    m_screenshot_path =
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "in_situ_mode.png" ) );

    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
}

void InSituModeTest::performs_in_situ_mode_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    auto* communication = main_window.findChild<Communication*>();
    auto* object_editor = main_window.findChild<ObjectEditor*>();
    auto* playback_tool_bar = main_window.findChild<::PlayBackControlToolBar*>();

    QVERIFY2( communication != nullptr, "Communication dock not found" );
    QVERIFY2( object_editor != nullptr, "ObjectEditor dock not found" );
    QVERIFY2( playback_tool_bar != nullptr, "PlayBackControlToolBar not found" );

    communication->show();
    object_editor->show();
    bringWindowToFront( &main_window );

    auto* connect_button = communication->findChild<QPushButton*>( "connectPushButton" );
    auto* disconnect_button = communication->findChild<QPushButton*>( "disconnectPushButton" );
    auto* remote_viz_insitu_radio = communication->findChild<QRadioButton*>( "remoteVizInsituRadioButton" );
    auto* setting_apply_button = communication->findChild<QPushButton*>( "settingApplyPushButton" );
    auto* object_tree_view = object_editor->findChild<QTreeView*>( "treeView" );
    auto* object_apply_button = object_editor->findChild<QPushButton*>( "applyPushButton" );
    auto* jump_button = playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );

    QVERIFY2( connect_button != nullptr, "connectPushButton not found" );
    QVERIFY2( disconnect_button != nullptr, "disconnectPushButton not found" );
    QVERIFY2( remote_viz_insitu_radio != nullptr, "remoteVizInsituRadioButton not found" );
    QVERIFY2( setting_apply_button != nullptr, "settingApplyPushButton not found" );
    QVERIFY2( object_tree_view != nullptr, "ObjectEditor treeView not found" );
    QVERIFY2( object_apply_button != nullptr, "ObjectEditor applyPushButton not found" );
    QVERIFY2( jump_button != nullptr, "m_jump_push_button not found" );

    QVERIFY2(
        waitForCondition( [connect_button]() { return connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "connectPushButton did not become enabled within the timeout" );
    QTest::mouseClick( connect_button, Qt::LeftButton );

    QVERIFY2(
        waitForCondition(
            [connect_button, disconnect_button]()
            {
                return !connect_button->isEnabled() && disconnect_button->isEnabled();
            },
            k_connect_timeout_ms,
            100 ),
        "Client did not enter the connected state" );

    QVERIFY2( remote_viz_insitu_radio->isEnabled(), "remoteVizInsituRadioButton is disabled" );
    QTest::mouseClick( remote_viz_insitu_radio, Qt::LeftButton );
    QVERIFY2( remote_viz_insitu_radio->isChecked(), "remoteVizInsituRadioButton was not checked" );

    QVERIFY2( setting_apply_button->isEnabled(), "settingApplyPushButton is disabled" );
    QTest::mouseClick( setting_apply_button, Qt::LeftButton );

    QVERIFY2(
        waitForCondition(
            [object_tree_view]()
            {
                return object_tree_view->model() != nullptr &&
                       object_tree_view->model()->rowCount() > 0;
            },
            k_tree_view_timeout_ms,
            100 ),
        "ObjectEditor treeView did not receive an item within the timeout" );

    QAbstractItemModel* model = object_tree_view->model();
    QVERIFY2( model != nullptr, "ObjectEditor treeView model not found" );
    const QModelIndex first_index = model->index( 0, 0 );
    QVERIFY2( first_index.isValid(), "ObjectEditor first row is invalid" );
    object_tree_view->scrollTo( first_index );
    object_tree_view->setCurrentIndex( first_index );
    QVERIFY2( object_tree_view->selectionModel() != nullptr, "ObjectEditor selection model not found" );
    object_tree_view->selectionModel()->setCurrentIndex(
        first_index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    QVERIFY2(
        waitForCondition( [object_apply_button]() { return object_apply_button->isEnabled(); }, k_tree_view_timeout_ms, 100 ),
        "ObjectEditor applyPushButton did not become enabled within the timeout" );
    QTest::mouseClick( object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition( [jump_button]() { return jump_button->isEnabled(); }, k_jump_button_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled within the timeout" );
    QTest::mouseClick( jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition( [jump_button]() { return jump_button->isEnabled(); }, k_jump_button_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled again within the timeout" );

    bringWindowToFront( &main_window );
    saveScreenshot();

    main_window.close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::InSituModeTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
