#include "MenuBarTest.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QTextStream>
#include <QTreeView>
#include <QWidget>
#include <QTest>

#include "../App/MainWindow.h"
#include "../Widgets/AnimationControl.h"
#include "../Widgets/Communication.h"
#include "../Widgets/GlyphEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/PlotOverLineEditor.h"
#include "../Widgets/PlotOverTimeEditor.h"
#include "../Widgets/PointSizeControl.h"
#include "../Widgets/Preference.h"
#include "../Widgets/RepetitionLevelControl.h"
#include "../Widgets/ShadingControl.h"
#include "../Widgets/TransferFunctionEditor.h"
#include "../Widgets/VolumeTransform.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_menu_popup_timeout_ms = 3000;
constexpr int k_window_settle_ms = 500;
constexpr int k_dialog_settle_ms = 1000;
constexpr int k_full_screen_capture_settle_ms = 300;
constexpr int k_action_enable_timeout_ms = 30000;
constexpr int k_post_connect_settle_ms = 1000;
constexpr int k_post_apply_settle_ms = 3000;
constexpr int k_top_level_widget_timeout_ms = 5000;
constexpr int k_widget_close_timeout_ms = 5000;
constexpr int k_button_retry_count = 3;
constexpr int k_button_retry_wait_ms = 500;
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

QString MenuBarTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString MenuBarTest::repoRootPath() const
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

QString MenuBarTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool MenuBarTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

QAction* MenuBarTest::findActionByText( QWidget* root, const QString& text ) const
{
    if ( root == nullptr ) { return nullptr; }

    const QList<QAction*> actions = root->findChildren<QAction*>();
    for ( QAction* action : actions )
    {
        if ( action && action->text() == text ) { return action; }
    }

    return nullptr;
}

void MenuBarTest::openMenu( QMainWindow& window, QMenu* menu ) const
{
    QVERIFY2( menu != nullptr, "Menu not found" );

    QMenuBar* menu_bar = window.menuBar();
    QVERIFY2( menu_bar != nullptr, "Menu bar not found" );

    window.raise();
    window.activateWindow();
    menu_bar->setFocus();
    QTest::qWait( k_window_settle_ms );

    const QRect action_rect = menu_bar->actionGeometry( menu->menuAction() );
    if ( action_rect.isValid() )
    {
        QTest::mouseClick( menu_bar, Qt::LeftButton, Qt::NoModifier, action_rect.center() );
    }
    else
    {
        const int x_offset = ( menu->objectName() == QStringLiteral( "menuTools" ) ) ? 140 : 24;
        const QPoint popup_pos = window.mapToGlobal( QPoint( x_offset, 36 ) );
        menu->popup( popup_pos );
    }

    const bool opened = waitForCondition(
        [menu]()
        {
            return menu->isVisible();
        },
        k_menu_popup_timeout_ms,
        50 );
    QVERIFY2( opened, qPrintable( QStringLiteral( "Menu did not open: %1" ).arg( menu->title() ) ) );
    QTest::qWait( k_dialog_settle_ms );
}

void MenuBarTest::triggerAction( QAction* action, int settle_ms ) const
{
    QVERIFY2( action != nullptr, "Action not found" );
    QVERIFY2( action->isEnabled(), qPrintable( QStringLiteral( "Action is disabled: %1" ).arg( action->text() ) ) );

    action->trigger();
    while ( QWidget* popup = QApplication::activePopupWidget() )
    {
        popup->close();
    }
    QTest::qWait( settle_ms );
}

void MenuBarTest::saveScreenshot( const QString& file_name, const QString& caption ) const
{
    QTest::qWait( k_full_screen_capture_settle_ms );

    QScreen* screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen != nullptr, "Primary screen not found" );

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );

    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the primary screen" );
    QVERIFY2(
        screenshot.save( file_path ),
        qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( file_path ) ) );

    const_cast<MenuBarTest*>( this )->m_screenshots.push_back( { file_name, caption } );
}

void MenuBarTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# MenuBarTest\n\n";
    stream << "## テスト結果\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 伝達関数ファイル: `" << m_transfer_function_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";
    stream << "## 実施内容\n\n";
    stream << "1. Communication でサーバに接続し、Remote Viz.(Client && Server) を選択しました。\n";
    stream << "2. ボリュームデータと伝達関数ファイルを設定して Apply を押しました。\n";
    stream << "3. Object Editor の nameLineEdit にテキストが入るまで待機し、Apply を押しました。\n";
    stream << "4. PlayBackControlToolBar の m_jump_push_button が有効になるまで待機し、ボタンを押しました。\n";
    stream << "5. Object Editor と Communication を閉じた後、メニュー項目を順に開いてスクリーンショットを取得しました。\n\n";
    stream << "## スクリーンショット\n\n";

    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        const QString screenshot_path =
            QDir( m_screenshot_dir_path ).absoluteFilePath( entry.file_name );
        const QString relative_screenshot_path =
            QDir( QFileInfo( m_report_path ).absolutePath() ).relativeFilePath( screenshot_path );

        stream << "### " << entry.caption << "\n\n";
        stream << "!["
               << entry.caption
               << "]("
               << relative_screenshot_path
               << ")\n\n";
    }
}

void MenuBarTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() ) );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_EXECUTABLE", repoRootPath() ) );
    m_server_target_wrapper_executable = envOrDefault(
        "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE", repoRootPath() ) );
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        ClientTests::configuredPath( "MEJ_VOLUME_DATA", repoRootPath() ) );
    m_transfer_function_path = envOrDefault(
        "MEJ_TRANSFER_FUNCTION",
        ClientTests::configuredPath( "MEJ_TRANSFER_FUNCTION", repoRootPath() ) );
    m_report_dir_path = envOrDefault(
        "PBVR_TEST_REPORT_DIR",
        ClientTests::datedTestOutputDir(
            repoRootPath(),
            date_stamp,
            QStringLiteral( "MenuBarTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_report_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_report_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );

    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QFileInfo::exists( m_transfer_function_path ),
        qPrintable( QStringLiteral( "Transfer function file not found: %1" ).arg( m_transfer_function_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_report_dir_path ),
        qPrintable( QStringLiteral( "Failed to create report directory: %1" ).arg( m_report_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

}

void MenuBarTest::cleanupTestCase()
{
    writeMarkdownReport();

    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }
}

void MenuBarTest::performs_menu_bar_scenario()
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
    auto* playback_tool_bar = main_window.findChild<PlayBackControlToolBar*>();

    QVERIFY2( communication != nullptr, "Communication dock not found" );
    QVERIFY2( object_editor != nullptr, "ObjectEditor dock not found" );
    QVERIFY2( playback_tool_bar != nullptr, "Playback toolbar not found" );

    communication->show();
    object_editor->show();
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_window_settle_ms );

    auto* connect_button = communication->findChild<QPushButton*>( "connectPushButton" );
    auto* remote_viz_radio = communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    auto* volume_path_edit = communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    auto* transfer_function_path_edit = communication->findChild<QLineEdit*>( "transferFunctionFilePathLineEdit" );
    auto* setting_apply_button = communication->findChild<QPushButton*>( "settingApplyPushButton" );
    auto* id_line_edit = communication->findChild<QLineEdit*>( "IDLineEdit" );
    auto* disconnect_button = communication->findChild<QPushButton*>( "disconnectPushButton" );

    QVERIFY2( connect_button != nullptr, "connectPushButton not found" );
    QVERIFY2( disconnect_button != nullptr, "disconnectPushButton not found" );
    QVERIFY2( remote_viz_radio != nullptr, "remoteVizClientServerRadioButton not found" );
    QVERIFY2( volume_path_edit != nullptr, "volumeDataFilePathLineEdit not found" );
    QVERIFY2( transfer_function_path_edit != nullptr, "transferFunctionFilePathLineEdit not found" );
    QVERIFY2( setting_apply_button != nullptr, "settingApplyPushButton not found" );
    QVERIFY2( id_line_edit != nullptr, "IDLineEdit not found" );

    const auto connected = [connect_button, disconnect_button, id_line_edit]()
    {
        return disconnect_button->isEnabled() &&
               !connect_button->isEnabled() &&
               !id_line_edit->text().trimmed().isEmpty();
    };

    for ( int attempt = 0; attempt < k_button_retry_count && !connected(); ++attempt )
    {
        QVERIFY2(
            waitForCondition(
                [connect_button]()
                {
                    return connect_button->isEnabled();
                },
                k_connect_timeout_ms,
                100 ),
            "connectPushButton did not become enabled within the timeout" );

        main_window.raise();
        main_window.activateWindow();
        QTest::qWait( k_window_settle_ms );
        QTest::mouseClick( connect_button, Qt::LeftButton );

        if ( connected() || waitForCondition( connected, k_connect_timeout_ms, 100 ) )
        {
            break;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }
    QVERIFY2( connected(), "Client did not enter the connected state" );
    QTest::qWait( k_post_connect_settle_ms );

    if ( !remote_viz_radio->isChecked() )
    {
        QTest::mouseClick( remote_viz_radio, Qt::LeftButton );
    }
    QVERIFY( remote_viz_radio->isChecked() );

    volume_path_edit->setFocus();
    volume_path_edit->clear();
    QTest::keyClicks( volume_path_edit, QDir::toNativeSeparators( m_volume_data_path ) );
    QCOMPARE( volume_path_edit->text(), QDir::toNativeSeparators( m_volume_data_path ) );

    transfer_function_path_edit->setFocus();
    transfer_function_path_edit->clear();
    QTest::keyClicks( transfer_function_path_edit, QDir::toNativeSeparators( m_transfer_function_path ) );
    QCOMPARE( transfer_function_path_edit->text(), QDir::toNativeSeparators( m_transfer_function_path ) );

    QTest::mouseClick( setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_post_apply_settle_ms );

    auto* name_line_edit = object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    auto* apply_button = object_editor->findChild<QPushButton*>( "applyPushButton" );

    QVERIFY2( name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" );
    QVERIFY2( apply_button != nullptr, "ObjectEditor applyPushButton not found" );

    const bool name_loaded = waitForCondition(
        [name_line_edit]()
        {
            return !name_line_edit->text().trimmed().isEmpty();
        },
        k_object_load_timeout_ms,
        100 );
    QVERIFY2( name_loaded, "ObjectEditor nameLineEdit was not populated within the timeout" );

    QTest::mouseClick( apply_button, Qt::LeftButton );

    auto* jump_button = playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    QVERIFY2( jump_button != nullptr, "m_jump_push_button not found" );

    const bool jump_enabled = waitForCondition(
        [jump_button]()
        {
            return jump_button->isEnabled();
        },
        k_jump_button_enable_timeout_ms,
        200 );
    QVERIFY2( jump_enabled, "m_jump_push_button did not become enabled within the timeout" );

    QTest::mouseClick( jump_button, Qt::LeftButton );
    QTest::qWait( k_dialog_settle_ms );

    object_editor->close();
    communication->close();
    QTest::qWait( k_window_settle_ms );

    saveScreenshot(
        QStringLiteral( "00_main_window_after_close.png" ),
        QStringLiteral( "Object Editor と Communication を閉じた状態" ) );

    QMenu* pbvr_client_menu = main_window.findChild<QMenu*>( "menuPBVRClient" );
    openMenu( main_window, pbvr_client_menu );
    saveScreenshot(
        QStringLiteral( "01_menu_pbvr_client_clicked.png" ),
        QStringLiteral( "pbvr_client をクリックしたことを表す" ) );

    QAction* preference_action = findActionByText( &main_window, QStringLiteral( "Preference" ) );
    triggerAction( preference_action, k_dialog_settle_ms );

    auto* preference = main_window.findChild<Preference*>();
    QVERIFY2( preference != nullptr, "Preference dialog not found" );
    QVERIFY2(
        waitForCondition(
            [preference]()
            {
                return preference->isVisible();
            },
            k_top_level_widget_timeout_ms,
            100 ),
        "Preference dialog did not become visible" );

    saveScreenshot(
        QStringLiteral( "02_preference_clicked.png" ),
        QStringLiteral( "Preference をクリックしたことを表す" ) );

    preference->close();
    QVERIFY2(
        waitForCondition(
            [preference]()
            {
                return !preference->isVisible();
            },
            k_widget_close_timeout_ms,
            100 ),
        "Preference dialog did not close" );

    QMenu* tools_menu = main_window.findChild<QMenu*>( "menuTools" );
    openMenu( main_window, tools_menu );
    saveScreenshot(
        QStringLiteral( "03_menu_tools_clicked.png" ),
        QStringLiteral( "Tools をクリックしたことを表す" ) );

    QAction* communication_action = findActionByText( &main_window, QStringLiteral( "Communication" ) );
    triggerAction( communication_action, k_dialog_settle_ms );

    QVERIFY2(
        waitForCondition(
            [communication]()
            {
                return communication->isVisible();
            },
            k_top_level_widget_timeout_ms,
            100 ),
        "Communication did not become visible" );

    saveScreenshot(
        QStringLiteral( "04_communication_clicked.png" ),
        QStringLiteral( "Communication をクリックしたことを表す" ) );

    communication->close();
    QVERIFY2(
        waitForCondition(
            [communication]()
            {
                return !communication->isVisible();
            },
            k_widget_close_timeout_ms,
            100 ),
        "Communication did not close" );

    struct ActionScenario
    {
        const char* text;
        const char* screenshot_file_name;
        const char* caption;
        std::function<QWidget*()> widget_lookup;
    };

    const ActionScenario scenarios[] = {
        { "Animation Control", "05_animation_control_clicked.png", "Animation Control をクリックしたことを表す", [&main_window]() { return main_window.findChild<AnimationControl*>(); } },
        { "Glyph Editor", "06_glyph_editor_clicked.png", "Glyph Editor をクリックしたことを表す", [&main_window]() { return main_window.findChild<GlyphEditor*>(); } },
        { "Object Editor", "07_object_editor_clicked.png", "Object Editor をクリックしたことを表す", [object_editor]() { return object_editor; } },
        { "Plot Over Line Editor", "08_plot_over_line_editor_clicked.png", "Plot Over Line Editor をクリックしたことを表す", [&main_window]() { return main_window.findChild<PlotOverLineEditor*>(); } },
        { "Plot Over Time Editor", "09_plot_over_time_editor_clicked.png", "Plot Over Time Editor をクリックしたことを表す", [&main_window]() { return main_window.findChild<PlotOverTimeEditor*>(); } },
        { "Point Size Control", "10_point_size_control_clicked.png", "Point Size Control をクリックしたことを表す", [&main_window]() { return main_window.findChild<PointSizeControl*>(); } },
        { "Repetition Level Control", "11_repetition_level_control_clicked.png", "Repetition Level Control をクリックしたことを表す", [&main_window]() { return main_window.findChild<RepetitionLevelControl*>(); } },
        { "Shading Control", "12_shading_control_clicked.png", "Shading Control をクリックしたことを表す", [&main_window]() { return main_window.findChild<ShadingControl*>(); } },
        { "Transfer Function Editor", "13_transfer_function_editor_clicked.png", "Transfer Function Editor をクリックしたことを表す", [&main_window]() { return main_window.findChild<TransferFunctionEditor*>(); } },
        { "Volume Transform", "14_volume_transform_clicked.png", "VolumeTransform をクリックしたことを表す", [&main_window]() { return main_window.findChild<VolumeTransform*>(); } }
    };

    for ( const ActionScenario& scenario : scenarios )
    {
        QAction* action = findActionByText( &main_window, QString::fromUtf8( scenario.text ) );
        QVERIFY2(
            waitForCondition(
                [action]()
                {
                    return action != nullptr && action->isEnabled();
                },
                k_action_enable_timeout_ms,
                100 ),
            qPrintable( QStringLiteral( "Action was not enabled in time: %1" ).arg( QString::fromUtf8( scenario.text ) ) ) );

        triggerAction( action, k_dialog_settle_ms );

        QWidget* opened_widget = scenario.widget_lookup();
        QVERIFY2( opened_widget != nullptr, qPrintable( QStringLiteral( "Widget not found for action: %1" ).arg( QString::fromUtf8( scenario.text ) ) ) );
        QVERIFY2(
            waitForCondition(
                [opened_widget]()
                {
                    return opened_widget->isVisible();
                },
                k_top_level_widget_timeout_ms,
                100 ),
            qPrintable( QStringLiteral( "Widget did not become visible for action: %1" ).arg( QString::fromUtf8( scenario.text ) ) ) );

        saveScreenshot(
            QString::fromUtf8( scenario.screenshot_file_name ),
            QString::fromUtf8( scenario.caption ) );

        opened_widget->close();
        QVERIFY2(
            waitForCondition(
                [opened_widget]()
                {
                    return !opened_widget->isVisible();
                },
                k_widget_close_timeout_ms,
                100 ),
            qPrintable( QStringLiteral( "Widget did not close for action: %1" ).arg( QString::fromUtf8( scenario.text ) ) ) );
    }

    m_test_succeeded = true;
}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    MenuBarTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
