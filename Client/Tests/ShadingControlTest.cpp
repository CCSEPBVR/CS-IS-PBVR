#include "ShadingControlTest.h"

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
#include "../Widgets/ShadingControl.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
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

QString ShadingControlTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString ShadingControlTest::repoRootPath() const
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

QString ShadingControlTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool ShadingControlTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void ShadingControlTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ShadingControlTest::bringShadingControlToFront( ShadingControl* control ) const
{
    QVERIFY2( control != nullptr, "ShadingControl is null" );
    control->show();
    control->raise();
    control->activateWindow();
    QVERIFY2( control->isVisible(), "ShadingControl did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void ShadingControlTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void ShadingControlTest::setSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const
{
    QVERIFY2( spin_box != nullptr, "Target double spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QCOMPARE( spin_box->value(), value );
    QTest::qWait( k_short_wait_ms );
}

void ShadingControlTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void ShadingControlTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# ShadingControlTest\n\n";
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
    if ( m_steps.empty() )
    {
        stream << "- NOT RUN: テスト手順は実行されていない。\n";
    }

    stream << "\n## 自動判定項目\n\n";
    if ( m_test_succeeded )
    {
        stream << "- PASS: Communication.ui の connectPushButton、remoteVizClientServerRadioButton、volumeDataFilePathLineEdit、settingApplyPushButton を取得して操作できた。\n";
        stream << "- PASS: TestPathConfig.ini の SPX_VOLUME_DATA を volumeDataFilePathLineEdit に設定できた。\n";
        stream << "- PASS: ObjectEditor.ui の nameLineEdit にテキストが入るまで待機し、applyPushButton を押せた。\n";
        stream << "- PASS: PlayBackControlToolBar.cpp の m_jump_push_button を押し、再度有効になるまで待機できた。\n";
        stream << "- PASS: ShadingControl.ui の各ラジオボタンと DoubleSpinBox を objectName で取得して操作できた。\n";
        stream << "- PASS: スクリーンショットと Markdown レポートを指定ディレクトリへ保存できた。\n";
    }
    else
    {
        stream << "- FAIL: テストが最後まで完了しなかった。QtTest の失敗箇所を確認する。\n";
    }

    stream << "\n## 目視確認対象\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "- 要確認: " << entry.caption << "\n";
    }
    if ( m_screenshots.empty() )
    {
        stream << "- NOT RUN: スクリーンショットは保存されていない。\n";
    }
    stream << "- 注意: スクリーンショットの視覚的な正しさは自動判定していない。\n";

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
    stream << "- Shading 種別および Ambient / Diffuse / Specular / Shininess 変更後の3D表示内容は、保存したスクリーンショットを目視確認する。\n";
}

void ShadingControlTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

ShadingControlTest::ClientHandles ShadingControlTest::resolveClientHandles( MainWindow& window ) const
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
    handles.shading_control = window.findChild<ShadingControl*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.shading_control != nullptr, "ShadingControl dock not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.none_radio_button = handles.shading_control->findChild<QRadioButton*>( "noneRadioButton" );
    handles.phong_radio_button = handles.shading_control->findChild<QRadioButton*>( "phongRadioButton" );
    handles.blinn_phong_radio_button = handles.shading_control->findChild<QRadioButton*>( "blinnPhongRadioButton" );
    handles.lambert_radio_button = handles.shading_control->findChild<QRadioButton*>( "lambertRadioButton" );
    handles.ambient_double_spin_box = handles.shading_control->findChild<QDoubleSpinBox*>( "AmbientDoubleSpinBox" );
    handles.diffuse_double_spin_box = handles.shading_control->findChild<QDoubleSpinBox*>( "DiffuseDoubleSpinBox" );
    handles.specular_double_spin_box = handles.shading_control->findChild<QDoubleSpinBox*>( "SpecularDoubleSpinBox" );
    handles.shininess_double_spin_box = handles.shading_control->findChild<QDoubleSpinBox*>( "ShininessDoubleSpinBox" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.none_radio_button != nullptr, "noneRadioButton not found" ) ) { return handles; }
    if ( !require( handles.phong_radio_button != nullptr, "phongRadioButton not found" ) ) { return handles; }
    if ( !require( handles.blinn_phong_radio_button != nullptr, "blinnPhongRadioButton not found" ) ) { return handles; }
    if ( !require( handles.lambert_radio_button != nullptr, "lambertRadioButton not found" ) ) { return handles; }
    if ( !require( handles.ambient_double_spin_box != nullptr, "AmbientDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.diffuse_double_spin_box != nullptr, "DiffuseDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.specular_double_spin_box != nullptr, "SpecularDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.shininess_double_spin_box != nullptr, "ShininessDoubleSpinBox not found" ) ) { return handles; }

    return handles;
}

void ShadingControlTest::connectClient( const ClientHandles& client ) const
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

void ShadingControlTest::configureRemoteVisualization( const ClientHandles& client ) const
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

void ShadingControlTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void ShadingControlTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

void ShadingControlTest::selectShadingRadioButton( const ClientHandles& client, QRadioButton* radio_button, const QString& step_description )
{
    bringShadingControlToFront( client.shading_control );
    if ( !radio_button->isChecked() )
    {
        QTest::mouseClick( radio_button, Qt::LeftButton );
    }
    QVERIFY2( radio_button->isChecked(), qPrintable( QStringLiteral( "%1 was not checked" ).arg( radio_button->objectName() ) ) );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( step_description );
}

void ShadingControlTest::captureShadingState( const QString& file_name, const QString& caption )
{
    saveScreenshot( file_name, caption );
    markStepCompleted( QStringLiteral( "%1をスクリーンショット撮影した。" ).arg( caption ) );
}

void ShadingControlTest::initTestCase()
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
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "ShadingControlTest" ) ) );
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

void ShadingControlTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void ShadingControlTest::performs_shading_control_scenario()
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

    bringShadingControlToFront( client.shading_control );
    markStepCompleted( QStringLiteral( "ShadingControl.uiを開きました。" ) );

    selectShadingRadioButton( client, client.none_radio_button, QStringLiteral( "ShadingControl.ui: noneRadioButtonを選択しました。" ) );
    captureShadingState(
        QStringLiteral( "01_none_radio_button.png" ),
        QStringLiteral( "Noneラジオボタンを押下したオブジェクト" ) );

    selectShadingRadioButton( client, client.lambert_radio_button, QStringLiteral( "ShadingControl.ui: lambertRadioButtonを選択しました。" ) );
    captureShadingState(
        QStringLiteral( "02_lambert_radio_button.png" ),
        QStringLiteral( "Lambertラジオボタンを押下したオブジェクト" ) );

    selectShadingRadioButton( client, client.phong_radio_button, QStringLiteral( "ShadingControl.ui: phongRadioButtonを選択しました。" ) );
    captureShadingState(
        QStringLiteral( "03_phong_radio_button.png" ),
        QStringLiteral( "Phongラジオボタンを押下したオブジェクト" ) );

    selectShadingRadioButton( client, client.blinn_phong_radio_button, QStringLiteral( "ShadingControl.ui: blinnPhongRadioButtonを選択しました。" ) );
    captureShadingState(
        QStringLiteral( "04_blinn_phong_radio_button.png" ),
        QStringLiteral( "BlinnPhongラジオボタンを押下したオブジェクト" ) );

    selectShadingRadioButton( client, client.lambert_radio_button, QStringLiteral( "ShadingControl.ui: lambertRadioButtonを選択しました。" ) );
    captureShadingState(
        QStringLiteral( "05_before_ambient_change.png" ),
        QStringLiteral( "Ambientを変更する前のオブジェクト" ) );

    setSpinBoxValue( client.ambient_double_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "AmbientDoubleSpinBoxに0を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "06_ambient_0.png" ),
        QStringLiteral( "Ambientを0に変更した時のオブジェクト" ) );

    setSpinBoxValue( client.ambient_double_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "AmbientDoubleSpinBoxに1を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "07_ambient_1.png" ),
        QStringLiteral( "Ambientを1に変更した時のオブジェクト" ) );

    setSpinBoxValue( client.ambient_double_spin_box, 0.4 );
    markStepCompleted( QStringLiteral( "AmbientDoubleSpinBoxに0.4を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "08_before_diffuse_change.png" ),
        QStringLiteral( "Diffuseを変更する前のオブジェクト" ) );

    setSpinBoxValue( client.diffuse_double_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "DiffuseDoubleSpinBoxに0を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "09_diffuse_0.png" ),
        QStringLiteral( "Diffuseを0に変更した時のオブジェクト" ) );

    setSpinBoxValue( client.diffuse_double_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "DiffuseDoubleSpinBoxに1を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "10_diffuse_1.png" ),
        QStringLiteral( "Diffuseを1に変更した時のオブジェクト" ) );

    setSpinBoxValue( client.diffuse_double_spin_box, 0.4 );
    markStepCompleted( QStringLiteral( "DiffuseDoubleSpinBoxに0.4を入力しました。" ) );

    selectShadingRadioButton( client, client.phong_radio_button, QStringLiteral( "ShadingControl.ui: phongRadioButtonを選択しました。" ) );
    captureShadingState(
        QStringLiteral( "11_before_specular_change.png" ),
        QStringLiteral( "Specularを変更する前のオブジェクト" ) );

    setSpinBoxValue( client.specular_double_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "SpecularDoubleSpinBoxに0を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "12_specular_0.png" ),
        QStringLiteral( "Specularを0に設定した時のオブジェクト" ) );

    setSpinBoxValue( client.specular_double_spin_box, 1.0 );
    markStepCompleted( QStringLiteral( "SpecularDoubleSpinBoxに1を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "13_specular_1.png" ),
        QStringLiteral( "Specularを1に設定した時のオブジェクト" ) );

    setSpinBoxValue( client.specular_double_spin_box, 0.6 );
    markStepCompleted( QStringLiteral( "SpecularDoubleSpinBoxに0.6を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "14_before_shininess_change.png" ),
        QStringLiteral( "Shininessを変更する前のオブジェクト" ) );

    setSpinBoxValue( client.shininess_double_spin_box, 0.0 );
    markStepCompleted( QStringLiteral( "ShininessDoubleSpinBoxに0を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "15_shininess_0.png" ),
        QStringLiteral( "Shininessを0に設定した時のオブジェクト" ) );

    setSpinBoxValue( client.shininess_double_spin_box, 100.0 );
    markStepCompleted( QStringLiteral( "ShininessDoubleSpinBoxに100を入力しました。" ) );
    captureShadingState(
        QStringLiteral( "16_shininess_100.png" ),
        QStringLiteral( "Shininessを100に設定した時のオブジェクト" ) );

    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::ShadingControlTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
