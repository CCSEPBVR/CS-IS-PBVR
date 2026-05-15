#include "TransferFunctionEditorColorFunctionSynthesizerTest.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTest>
#include <QTimer>

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
constexpr int k_dialog_timeout_ms = 10000;
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

QString ColorFunctionSynthesizerTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString ColorFunctionSynthesizerTest::repoRootPath() const
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

QString ColorFunctionSynthesizerTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool ColorFunctionSynthesizerTest::waitForCondition(
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

void ColorFunctionSynthesizerTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ColorFunctionSynthesizerTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void ColorFunctionSynthesizerTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void ColorFunctionSynthesizerTest::closeUnexpectedDialogs( TransferFunctionEditor* editor ) const
{
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        auto* dialog = qobject_cast<QDialog*>( widget );
        if ( dialog == nullptr || dialog == editor || !dialog->isVisible() ) { continue; }

        dialog->reject();
        dialog->close();
    }
}

void ColorFunctionSynthesizerTest::setTransferFunctionCount( const ClientHandles& client, int value ) const
{
    QSpinBox* spin_box = client.number_of_transfer_function_spin_box;
    QVERIFY2( spin_box != nullptr, "numberOfTransferFunctionSpinBox was not found" );
    QVERIFY2( spin_box->isEnabled(), "numberOfTransferFunctionSpinBox is disabled" );
    QVERIFY2(
        value >= spin_box->minimum() && value <= spin_box->maximum(),
        qPrintable( QStringLiteral( "Target value is out of range: %1" ).arg( value ) ) );

    QTimer dialog_closer;
    connect( &dialog_closer, &QTimer::timeout, this, [this, client]() { closeUnexpectedDialogs( client.transfer_function_editor ); } );
    dialog_closer.start( 100 );

    spin_box->setFocus();
    spin_box->selectAll();
    QTest::keyClicks( spin_box, QString::number( value ) );
    QTest::keyClick( spin_box, Qt::Key_Enter );
    dialog_closer.stop();

    QCOMPARE( spin_box->value(), value );
    closeUnexpectedDialogs( client.transfer_function_editor );
    QTest::qWait( k_short_wait_ms );
}

void ColorFunctionSynthesizerTest::setComboBoxIndex( QComboBox* combo_box, int index, const char* widget_name ) const
{
    QVERIFY2( combo_box != nullptr, widget_name );
    QVERIFY2(
        index >= 0 && index < combo_box->count(),
        qPrintable(
            QStringLiteral( "%1 index %2 is out of range. item count: %3" )
                .arg( QString::fromUtf8( widget_name ) )
                .arg( index )
                .arg( combo_box->count() ) ) );

    combo_box->setFocus();
    combo_box->setCurrentIndex( index );
    QCOMPARE( combo_box->currentIndex(), index );
    QTest::qWait( k_short_wait_ms );
}

void ColorFunctionSynthesizerTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void ColorFunctionSynthesizerTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditorTest::ColorFunctionSynthesizerTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- サーバ起動ラッパー: `" << m_server_target_wrapper_executable << "`\n";
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
               << "](img/"
               << entry.file_name
               << ")\n\n";
    }
}

void ColorFunctionSynthesizerTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

ColorFunctionSynthesizerTest::ClientHandles
ColorFunctionSynthesizerTest::resolveClientHandles( MainWindow& window ) const
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
    handles.tf_color_map_edit_button = handles.transfer_function_editor->findChild<QPushButton*>( "colorMapEditPushButton" );
    handles.number_of_transfer_function_spin_box =
        handles.transfer_function_editor->findChild<QSpinBox*>( "numberOfTransferFunctionSpinBox" );
    handles.color_synthesizer_line_edit =
        handles.transfer_function_editor->findChild<QLineEdit*>( "colorSynthesizerLineEdit" );
    handles.color_function_combo_box =
        handles.transfer_function_editor->findChild<QComboBox*>( "colorFunctionComboBox" );

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
    if ( !require( handles.tf_color_map_edit_button != nullptr, "TransferFunctionEditor colorMapEditPushButton not found" ) ) { return handles; }
    if ( !require( handles.number_of_transfer_function_spin_box != nullptr, "numberOfTransferFunctionSpinBox not found" ) ) { return handles; }
    if ( !require( handles.color_synthesizer_line_edit != nullptr, "colorSynthesizerLineEdit not found" ) ) { return handles; }
    if ( !require( handles.color_function_combo_box != nullptr, "colorFunctionComboBox not found" ) ) { return handles; }

    return handles;
}

void ColorFunctionSynthesizerTest::connectClient( const ClientHandles& client ) const
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

void ColorFunctionSynthesizerTest::configureRemoteVisualization( const ClientHandles& client ) const
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

void ColorFunctionSynthesizerTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void ColorFunctionSynthesizerTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

void ColorFunctionSynthesizerTest::applyControlPointColorMap(
    const ClientHandles& client,
    const QVector<QVector<double>>& rows )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_color_map_edit_button->isEnabled(), "colorMapEditPushButton is disabled" );

    QTimer::singleShot(
        0,
        [this, rows]()
        {
            QDialog* dialog = nullptr;
            QVERIFY2(
                waitForCondition(
                    [&dialog]()
                    {
                        for ( QWidget* widget : QApplication::topLevelWidgets() )
                        {
                            auto* candidate = qobject_cast<QDialog*>( widget );
                            if ( candidate == nullptr ) { continue; }
                            if ( candidate->objectName() != QStringLiteral( "ColorMapEditor" ) ) { continue; }
                            if ( !candidate->isVisible() ) { continue; }
                            dialog = candidate;
                            return true;
                        }
                        return false;
                    },
                    k_dialog_timeout_ms,
                    50 ),
                "ColorMapEditor did not become visible" );

            QTest::qWait( k_window_settle_ms );

            auto* tab_widget = dialog->findChild<QTabWidget*>( "tabWidget" );
            QVERIFY2( tab_widget != nullptr, "ColorMapEditor tabWidget not found" );
            int control_points_index = -1;
            for ( int index = 0; index < tab_widget->count(); ++index )
            {
                QWidget* tab = tab_widget->widget( index );
                if ( tab != nullptr && tab->objectName() == QStringLiteral( "controlPoints" ) )
                {
                    control_points_index = index;
                    break;
                }
            }
            QVERIFY2( control_points_index >= 0, "ColorMapEditor controlPoints tab was not found" );
            tab_widget->setCurrentIndex( control_points_index );

            auto* spin_box = dialog->findChild<QSpinBox*>( "numberOfControlPointsSpinBox" );
            QVERIFY2( spin_box != nullptr, "numberOfControlPointsSpinBox not found" );
            spin_box->setValue( rows.size() );

            auto* table = dialog->findChild<QTableWidget*>( "controlPointsTableWidget" );
            QVERIFY2( table != nullptr, "controlPointsTableWidget not found" );
            QCOMPARE( table->rowCount(), rows.size() );
            QVERIFY2( table->columnCount() >= 4, "controlPointsTableWidget has fewer than four columns" );

            for ( int row = 0; row < rows.size(); ++row )
            {
                QCOMPARE( rows[row].size(), 4 );
                for ( int column = 0; column < 4; ++column )
                {
                    auto* item = new QTableWidgetItem( QString::number( rows[row][column], 'g', 12 ) );
                    table->setItem( row, column, item );
                }
            }

            const bool invoked = QMetaObject::invokeMethod( dialog, "onControlPointChanged", Qt::DirectConnection );
            QVERIFY2( invoked, "Failed to invoke ColorMapEditor::onControlPointChanged" );
            QTest::qWait( k_short_wait_ms );

            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "ColorMapEditor applyPushButton not found" );
            QVERIFY2( apply_button->isEnabled(), "ColorMapEditor applyPushButton is disabled" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
        } );

    QTest::mouseClick( client.tf_color_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ColorFunctionSynthesizerTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" ) );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Server/pbvr_server" ) );
    m_server_target_wrapper_executable = envOrDefault(
        "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE",
        sourceTreePath( QStringLiteral( "server_target_wrapper.sh" ) ) );
    m_volume_data_path = envOrDefault(
        "PBVR_VOLUME_DATA",
        QStringLiteral( "/path/to/SampleData/ucd/old/out/spx.pfl" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/ColorFunctionSynthesizerTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QVERIFY2( QFileInfo::exists( m_client_executable ), qPrintable( QStringLiteral( "Client executable not found: %1" ).arg( m_client_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_server_executable ), qPrintable( QStringLiteral( "Server executable not found: %1" ).arg( m_server_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_server_target_wrapper_executable ), qPrintable( QStringLiteral( "Server target wrapper executable not found: %1" ).arg( m_server_target_wrapper_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

    m_server_process.setProgram( m_server_target_wrapper_executable );
    m_server_process.setArguments( { m_server_executable } );
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.start();

    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
}

void ColorFunctionSynthesizerTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void ColorFunctionSynthesizerTest::synthesize_color_functions()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    main_window.show();
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
        QStringLiteral( "00_object_before_color_function_synthesis.png" ),
        QStringLiteral( "伝達関数のカラーマップを合成する前に表示したオブジェクトの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数のカラーマップを合成する前に表示したオブジェクトの状態。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    setTransferFunctionCount( client, 2 );
    setLineEditText( client.color_synthesizer_line_edit, QStringLiteral( "C1+C2" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.uiを開き、numberOfTransferFunctionSpinBoxに2を入力し、colorSynthesizerLineEditにC1+C2を入力しました。" ) );

    setComboBoxIndex( client.color_function_combo_box, 0, "colorFunctionComboBox" );
    applyControlPointColorMap(
        client,
        {
            { 0.0, 1.0, 0.0, 0.0 },
            { 1.0, 1.0, 0.0, 0.0 },
        } );
    markStepCompleted( QStringLiteral( "ColorMapEditor.ui: controlPointsタブを選択し、1番目のカラーマップを赤に設定してapplyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "01_tfe_first_color_map_red.png" ),
        QStringLiteral( "伝達関数の1番目のカラーマップを赤に変更したTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数の1番目のカラーマップを赤に変更したTransferFunctionEditorの状態。" ) );

    setComboBoxIndex( client.color_function_combo_box, 1, "colorFunctionComboBox" );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: colorFunctionComboBoxの2番目を選択しました。" ) );
    applyControlPointColorMap(
        client,
        {
            { 0.0, 0.0, 0.0, 1.0 },
            { 1.0, 0.0, 0.0, 1.0 },
        } );
    markStepCompleted( QStringLiteral( "ColorMapEditor.ui: controlPointsタブを選択し、2番目のカラーマップを青に設定してapplyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "02_tfe_second_color_map_blue.png" ),
        QStringLiteral( "伝達関数の2番目のカラーマップを青に変更したTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数の2番目のカラーマップを青に変更したTransferFunctionEditorの状態。" ) );

    QVERIFY2( client.tf_apply_button->isEnabled(), "TransferFunctionEditor applyPushButton is disabled" );
    QTest::mouseClick( client.tf_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    client.transfer_function_editor->close();
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押し、TransferFunctionEditor.uiを閉じました。" ) );

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );

    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "03_object_after_color_function_synthesis.png" ),
        QStringLiteral( "伝達関数のカラーマップを合成して表示したオブジェクトの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数のカラーマップを合成して表示したオブジェクトの状態。" ) );

    m_test_succeeded = true;
}

}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::ColorFunctionSynthesizerTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
