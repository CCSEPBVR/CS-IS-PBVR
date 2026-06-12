#include "VolumeTransformTest.h"

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

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/VolumeTransform.h"
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

namespace ClientTests
{

QString VolumeTransformTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString VolumeTransformTest::repoRootPath() const
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

QString VolumeTransformTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool VolumeTransformTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void VolumeTransformTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void VolumeTransformTest::bringVolumeTransformToFront( VolumeTransform* control ) const
{
    QVERIFY2( control != nullptr, "VolumeTransform is null" );
    control->show();
    control->raise();
    control->activateWindow();
    QVERIFY2( control->isVisible(), "VolumeTransform did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void VolumeTransformTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void VolumeTransformTest::setSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const
{
    QVERIFY2( spin_box != nullptr, "Target double spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QCOMPARE( spin_box->value(), value );
    QTest::qWait( k_short_wait_ms );
}

void VolumeTransformTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void VolumeTransformTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# VolumeTransformTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- サーバターゲットラッパー: `" << m_server_target_wrapper_executable << "`\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    const QString auto_status = m_test_succeeded ? QStringLiteral( "PASS" ) : QStringLiteral( "FAIL" );
    stream << "\n## 自動判定項目\n\n";
    stream << "- " << auto_status << ": Communication.ui の connectPushButton / disconnectPushButton / remoteVizClientServerRadioButton / volumeDataFilePathLineEdit / settingApplyPushButton を取得できること。\n";
    stream << "- " << auto_status << ": connectPushButton 押下後に接続状態となり、IDLineEdit にIDが表示されること。\n";
    stream << "- " << auto_status << ": volumeDataFilePathLineEdit に TestPathConfig.ini の SPX_VOLUME_DATA を入力できること。\n";
    stream << "- " << auto_status << ": ObjectEditor.ui の nameLineEdit にテキストが入り、applyPushButton を押せること。\n";
    stream << "- " << auto_status << ": PlayBackControlToolBar.cpp の m_jump_push_button を押下後、再度有効になること。\n";
    stream << "- " << auto_status << ": VolumeTransform.ui の各 DoubleSpinBox に指定値を設定でき、applyPushButton を押せること。\n";
    stream << "- " << auto_status << ": Markdown レポートとスクリーンショットファイルを出力できること。\n\n";

    stream << "## 目視確認対象\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "- 要確認: " << entry.caption << "\n";
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

    stream << "## 未自動化・保留事項\n\n";
    stream << "- VolumeTransform 適用後の3D表示が期待通り変化しているかは、スクリーンショットを目視確認してください。\n";
}

void VolumeTransformTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

VolumeTransformTest::ClientHandles VolumeTransformTest::resolveClientHandles( MainWindow& window ) const
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
    handles.volume_transform = window.findChild<VolumeTransform*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.volume_transform != nullptr, "VolumeTransform dock not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.volume_transform_apply_button = handles.volume_transform->findChild<QPushButton*>( "applyPushButton" );
    handles.translation_x_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "translationXAxisDoubleSpinBox" );
    handles.translation_y_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "translationYAxisDoubleSpinBox" );
    handles.translation_z_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "translationZAxisDoubleSpinBox" );
    handles.scale_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "scaleDoubleSpinBox" );
    handles.rotation_x_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationXAxisDoubleSpinBox" );
    handles.rotation_y_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationYAxisDoubleSpinBox" );
    handles.rotation_z_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationZAxisDoubleSpinBox" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.volume_transform_apply_button != nullptr, "VolumeTransform applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.translation_x_axis_spin_box != nullptr, "translationXAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.translation_y_axis_spin_box != nullptr, "translationYAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.translation_z_axis_spin_box != nullptr, "translationZAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.scale_spin_box != nullptr, "scaleDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.rotation_x_axis_spin_box != nullptr, "rotationXAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.rotation_y_axis_spin_box != nullptr, "rotationYAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.rotation_z_axis_spin_box != nullptr, "rotationZAxisDoubleSpinBox not found" ) ) { return handles; }

    return handles;
}

void VolumeTransformTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.connect_button->isEnabled();
            },
            k_connect_timeout_ms,
            100 ),
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

void VolumeTransformTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_client_server_radio->isChecked(), "remoteVizClientServerRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void VolumeTransformTest::waitForObjectAndApply( const ClientHandles& client ) const
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

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void VolumeTransformTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_after_jump_wait_ms );
}

void VolumeTransformTest::applyVolumeTransform( const ClientHandles& client, const QString& step_description )
{
    bringVolumeTransformToFront( client.volume_transform );
    QTest::mouseClick( client.volume_transform_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( step_description );
}

void VolumeTransformTest::captureVolumeTransformState( const QString& file_name, const QString& caption )
{
    saveScreenshot( file_name, caption );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: %1" ).arg( caption ) );
}

void VolumeTransformTest::initTestCase()
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
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "VolumeTransformTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

}

void VolumeTransformTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void VolumeTransformTest::performs_volume_transform_scenario()
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
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButtonを押し、1秒待機しました。" ) );

    configureRemoteVisualization( client );
    markStepCompleted( QStringLiteral( "Communication.ui: remoteVizClientServerRadioButtonを押し、volumeDataFilePathLineEditにデータパスを書き込み、settingApplyPushButtonを押しました。" ) );

    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: nameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待ってから3秒待機しました。" ) );

    captureVolumeTransformState(
        QStringLiteral( "00_before_volume_transform.png" ),
        QStringLiteral( "VolumeTransformを変更する前のオブジェクト" ) );

    bringVolumeTransformToFront( client.volume_transform );
    markStepCompleted( QStringLiteral( "VolumeTransform.uiを開きました。" ) );

    setSpinBoxValue( client.translation_x_axis_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: translationXAxisDoubleSpinBoxに1を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "01_translation_x_axis.png" ),
        QStringLiteral( "オブジェクトがX軸方向に移動することを表す" ) );

    setSpinBoxValue( client.translation_x_axis_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: translationXAxisDoubleSpinBoxに0を入力しました。" ) );
    setSpinBoxValue( client.translation_y_axis_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: translationYAxisDoubleSpinBoxに1を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "02_translation_y_axis.png" ),
        QStringLiteral( "オブジェクトがY軸方向に移動することを表す" ) );

    setSpinBoxValue( client.translation_y_axis_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: translationYAxisDoubleSpinBoxに0を入力しました。" ) );
    setSpinBoxValue( client.translation_z_axis_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: translationZAxisDoubleSpinBoxに1を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "03_translation_z_axis.png" ),
        QStringLiteral( "オブジェクトがZ軸方向に移動することを表す" ) );

    setSpinBoxValue( client.translation_z_axis_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: translationZAxisDoubleSpinBoxに0を入力しました。" ) );
    setSpinBoxValue( client.scale_spin_box, 2.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: scaleDoubleSpinBoxに2を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "04_scale_2.png" ),
        QStringLiteral( "オブジェクトが拡大したことを表す" ) );

    setSpinBoxValue( client.scale_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: scaleDoubleSpinBoxに1を入力しました。" ) );
    setSpinBoxValue( client.rotation_x_axis_spin_box, 90.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: rotationXAxisDoubleSpinBoxに90を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "05_rotation_x_axis.png" ),
        QStringLiteral( "オブジェクトがX軸に対して回転することを表す" ) );

    setSpinBoxValue( client.rotation_x_axis_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: rotationXAxisDoubleSpinBoxに0を入力しました。" ) );
    setSpinBoxValue( client.rotation_y_axis_spin_box, 90.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: rotationYAxisDoubleSpinBoxに90を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "06_rotation_y_axis.png" ),
        QStringLiteral( "オブジェクトがY軸に対して回転することを表す" ) );

    setSpinBoxValue( client.rotation_y_axis_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: rotationYAxisDoubleSpinBoxに0を入力しました。" ) );
    setSpinBoxValue( client.rotation_z_axis_spin_box, 90.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: rotationZAxisDoubleSpinBoxに90を入力しました。" ) );
    applyVolumeTransform( client, QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
    captureVolumeTransformState(
        QStringLiteral( "07_rotation_z_axis.png" ),
        QStringLiteral( "オブジェクトがZ軸に対して回転することを表す" ) );

    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::VolumeTransformTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
