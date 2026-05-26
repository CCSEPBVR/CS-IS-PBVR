#include "TransferFunctionEditorOpacityMinMaxTest.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QTextStream>
#include <QTest>

#include <cmath>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TransferFunctionEditor.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_after_jump_wait_ms = 3000;
constexpr int k_capture_settle_ms = 300;
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

namespace TransferFunctionEditorTest
{

QString OpacityMinMaxTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString OpacityMinMaxTest::repoRootPath() const
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

QString OpacityMinMaxTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool OpacityMinMaxTest::waitForCondition(
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

void OpacityMinMaxTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void OpacityMinMaxTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void OpacityMinMaxTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void OpacityMinMaxTest::setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value, const char* widget_name ) const
{
    QVERIFY2( spin_box != nullptr, widget_name );
    QVERIFY2( spin_box->isEnabled(), qPrintable( QStringLiteral( "%1 is disabled" ).arg( widget_name ) ) );
    QVERIFY2(
        value >= spin_box->minimum() && value <= spin_box->maximum(),
        qPrintable( QStringLiteral( "%1 target value is out of range: %2" ).arg( widget_name ).arg( value ) ) );

    spin_box->setFocus();
    spin_box->setValue( value );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2(
        std::abs( spin_box->value() - value ) < 1.0e-8,
        qPrintable( QStringLiteral( "%1 was not set to %2" ).arg( widget_name ).arg( value ) ) );
}

void OpacityMinMaxTest::saveScreenshot( const QString& file_name, const QString& caption )
{
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

void OpacityMinMaxTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditorTest::OpacityMinMaxTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.caption << "\n\n";
        stream << "!["
               << entry.caption
               << "](./img/"
               << entry.file_name
               << ")\n\n";
    }
}

void OpacityMinMaxTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

OpacityMinMaxTest::ClientHandles OpacityMinMaxTest::resolveClientHandles( MainWindow& window ) const
{
    ClientHandles handles;
    const auto require = []( bool condition, const char* message )
    {
        if ( condition ) { return true; }
        QTest::qFail( message, __FILE__, __LINE__ );
        return false;
    };

    handles.main_window = &window;
    handles.communication = window.findChild<Communication*>();
    handles.object_editor = window.findChild<ObjectEditor*>();
    handles.playback_tool_bar = window.findChild<::PlayBackControlToolBar*>();
    handles.transfer_function_editor = window.findChild<TransferFunctionEditor*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_editor != nullptr, "TransferFunctionEditor not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.tf_apply_button = handles.transfer_function_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.opacity_user_defined_min_max_radio =
        handles.transfer_function_editor->findChild<QRadioButton*>( "opacityUserDefinedMinMaxRadioButton" );
    handles.opacity_server_side_min_max_radio =
        handles.transfer_function_editor->findChild<QRadioButton*>( "opacityServerSideMinMaxRadioButton" );
    handles.opacity_user_defined_min_spin_box =
        handles.transfer_function_editor->findChild<QDoubleSpinBox*>( "opacityUserDefinedMinDoubleSpinBox" );
    handles.opacity_user_defined_max_spin_box =
        handles.transfer_function_editor->findChild<QDoubleSpinBox*>( "opacityUserDefinedMaxDoubleSpinBox" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.tf_apply_button != nullptr, "TransferFunctionEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.opacity_user_defined_min_max_radio != nullptr, "opacityUserDefinedMinMaxRadioButton not found" ) ) { return handles; }
    if ( !require( handles.opacity_server_side_min_max_radio != nullptr, "opacityServerSideMinMaxRadioButton not found" ) ) { return handles; }
    if ( !require( handles.opacity_user_defined_min_spin_box != nullptr, "opacityUserDefinedMinDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.opacity_user_defined_max_spin_box != nullptr, "opacityUserDefinedMaxDoubleSpinBox not found" ) ) { return handles; }

    return handles;
}

void OpacityMinMaxTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "connectPushButton did not become enabled within the timeout" );

    QTest::mouseClick( client.connect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.disconnect_button->isEnabled() &&
                       !client.connect_button->isEnabled() &&
                       !client.id_line_edit->text().trimmed().isEmpty();
            },
            k_connect_timeout_ms,
            100 ),
        "Client did not enter the connected state" );
}

void OpacityMinMaxTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_client_server_radio->isChecked(), "remoteVizClientServerRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void OpacityMinMaxTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_apply_button->isEnabled() &&
                       !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor nameLineEdit was not populated within the timeout" );

    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void OpacityMinMaxTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled within the timeout" );

    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled again within the timeout" );

    QTest::qWait( k_after_jump_wait_ms );
}

void OpacityMinMaxTest::applyTransferFunction( const ClientHandles& client ) const
{
    QVERIFY2( client.tf_apply_button->isEnabled(), "TransferFunctionEditor applyPushButton is disabled" );
    QTest::mouseClick( client.tf_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void OpacityMinMaxTest::initTestCase()
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
        ClientTests::configuredPath( "SPX_VOLUME_DATA", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/OpacityMinMaxTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

}

void OpacityMinMaxTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void OpacityMinMaxTest::opacity_min_max()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();

    connectClient( client );
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButtonを押しました。" ) );
    configureRemoteVisualization( client );
    markStepCompleted( QStringLiteral( "Communication.ui: remoteVizClientServerRadioButtonを押し、volumeDataFilePathLineEditにデータパスを書き込み、settingApplyPushButtonを押しました。" ) );
    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: nameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "00_object_server_side_min_max.png" ),
        QStringLiteral( "Server Side Min Maxでオブジェクトを可視化した結果" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: Server Side Min Maxでオブジェクトを可視化した結果。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.opacity_server_side_min_max_radio->isChecked(), "opacityServerSideMinMaxRadioButton is not checked" );
    saveScreenshot(
        QStringLiteral( "01_tfe_server_side_min_max.png" ),
        QStringLiteral( "Server Side Min Maxを選択した状態のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: Server Side Min Maxを選択したTransferFunctionEditorの状態。" ) );

    QTest::mouseClick( client.opacity_user_defined_min_max_radio, Qt::LeftButton );
    QVERIFY2( client.opacity_user_defined_min_max_radio->isChecked(), "opacityUserDefinedMinMaxRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );
    saveScreenshot(
        QStringLiteral( "02_tfe_user_defined_min_max.png" ),
        QStringLiteral( "User Defined Min Maxを選択した状態のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: opacityUserDefinedMinMaxRadioButtonを選択し、スクリーンショットを撮影しました。" ) );
    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "03_object_user_defined_min_max.png" ),
        QStringLiteral( "User Defined Min Maxでオブジェクトを可視化した結果" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Maxでオブジェクトを可視化した結果。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    setDoubleSpinBoxValue( client.opacity_user_defined_min_spin_box, 0.2, "opacityUserDefinedMinDoubleSpinBox" );
    saveScreenshot(
        QStringLiteral( "04_tfe_user_defined_min_0_2_max_1.png" ),
        QStringLiteral( "User Defined Min Maxを0.2~1に設定した状態のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: opacityUserDefinedMinDoubleSpinBoxを0.2に設定し、スクリーンショットを撮影しました。" ) );
    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "05_object_user_defined_min_0_2_max_1.png" ),
        QStringLiteral( "User Defined Min Maxを0.2~1に設定したオブジェクトを可視化した結果" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Maxを0.2~1に設定したオブジェクトを可視化した結果。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    setDoubleSpinBoxValue( client.opacity_user_defined_min_spin_box, 0.5, "opacityUserDefinedMinDoubleSpinBox" );
    saveScreenshot(
        QStringLiteral( "06_tfe_user_defined_min_0_5_max_1.png" ),
        QStringLiteral( "User Defined Min Maxを0.5~1に設定した状態のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: opacityUserDefinedMinDoubleSpinBoxを0.5に設定し、スクリーンショットを撮影しました。" ) );
    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "07_object_user_defined_min_0_5_max_1.png" ),
        QStringLiteral( "User Defined Min Maxを0.5~1に設定したオブジェクトを可視化した結果" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Maxを0.5~1に設定したオブジェクトを可視化した結果。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    setDoubleSpinBoxValue( client.opacity_user_defined_max_spin_box, 0.6, "opacityUserDefinedMaxDoubleSpinBox" );
    saveScreenshot(
        QStringLiteral( "08_tfe_user_defined_min_0_5_max_0_6.png" ),
        QStringLiteral( "User Defined Min Maxを0.5~0.6に設定した状態のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: opacityUserDefinedMaxDoubleSpinBoxを0.6に設定し、スクリーンショットを撮影しました。" ) );
    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "09_object_user_defined_min_0_5_max_0_6.png" ),
        QStringLiteral( "User Defined Min Maxを0.5~0.6に設定したオブジェクトを可視化した結果" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Maxを0.5~0.6に設定したオブジェクトを可視化した結果。" ) );

    m_test_succeeded = true;
}

}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::OpacityMinMaxTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
