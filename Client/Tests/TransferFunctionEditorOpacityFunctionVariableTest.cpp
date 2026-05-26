#include "TransferFunctionEditorOpacityFunctionVariableTest.h"

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
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QTextStream>
#include <QTest>
#include <QTimer>
#include <QTreeView>

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

QString OpacityFunctionVariableTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString OpacityFunctionVariableTest::repoRootPath() const
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

QString OpacityFunctionVariableTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool OpacityFunctionVariableTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void OpacityFunctionVariableTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void OpacityFunctionVariableTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void OpacityFunctionVariableTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void OpacityFunctionVariableTest::closeUnexpectedDialogs( TransferFunctionEditor* editor ) const
{
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        auto* dialog = qobject_cast<QDialog*>( widget );
        if ( dialog == nullptr || dialog == editor || !dialog->isVisible() ) { continue; }

        dialog->reject();
        dialog->close();
    }
}

void OpacityFunctionVariableTest::setTransferFunctionCount( const ClientHandles& client, int value ) const
{
    QSpinBox* spin_box = client.number_of_transfer_function_spin_box;
    QVERIFY2( spin_box != nullptr, "numberOfTransferFunctionSpinBox was not found" );
    QVERIFY2( spin_box->isEnabled(), "numberOfTransferFunctionSpinBox is disabled" );
    spin_box->setFocus();

    QVERIFY2(
        value >= spin_box->minimum() && value <= spin_box->maximum(),
        qPrintable( QStringLiteral( "Target value is out of range: %1" ).arg( value ) ) );

    QTimer dialog_closer;
    connect( &dialog_closer, &QTimer::timeout, this, [this, client]() { closeUnexpectedDialogs( client.transfer_function_editor ); } );
    dialog_closer.start( 100 );

    spin_box->selectAll();
    QTest::keyClicks( spin_box, QString::number( value ) );
    QTest::keyClick( spin_box, Qt::Key_Enter );
    dialog_closer.stop();

    QCOMPARE( spin_box->value(), value );
    closeUnexpectedDialogs( client.transfer_function_editor );
}

void OpacityFunctionVariableTest::setComboBoxIndex( QComboBox* combo_box, int index, const char* widget_name ) const
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
}

void OpacityFunctionVariableTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void OpacityFunctionVariableTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditorTest::OpacityFunctionVariableTest\n\n";
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

void OpacityFunctionVariableTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

OpacityFunctionVariableTest::ClientHandles
OpacityFunctionVariableTest::resolveClientHandles( MainWindow& window ) const
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
    handles.number_of_transfer_function_spin_box =
        handles.transfer_function_editor->findChild<QSpinBox*>( "numberOfTransferFunctionSpinBox" );
    handles.opacity_function_combo_box =
        handles.transfer_function_editor->findChild<QComboBox*>( "opacityFunctionComboBox" );
    handles.opacity_function_variable_editor_button =
        handles.transfer_function_editor->findChild<QPushButton*>( "opacityFunctionVariableEditorPushButton" );
    handles.opacity_function_variable_line_edit =
        handles.transfer_function_editor->findChild<QLineEdit*>( "opacityFunctionVariableLineEdit" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.number_of_transfer_function_spin_box != nullptr, "numberOfTransferFunctionSpinBox not found" ) ) { return handles; }
    if ( !require( handles.opacity_function_combo_box != nullptr, "opacityFunctionComboBox not found" ) ) { return handles; }
    if ( !require( handles.opacity_function_variable_editor_button != nullptr, "opacityFunctionVariableEditorPushButton not found" ) ) { return handles; }
    if ( !require( handles.opacity_function_variable_line_edit != nullptr, "opacityFunctionVariableLineEdit not found" ) ) { return handles; }

    return handles;
}

void OpacityFunctionVariableTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "connectPushButton did not become enabled within the timeout" );

    QTest::mouseClick( client.connect_button, Qt::LeftButton );

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

void OpacityFunctionVariableTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_client_server_radio->isChecked(), "remoteVizClientServerRadioButton was not checked" );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
}

void OpacityFunctionVariableTest::waitForObjectAndApply( const ClientHandles& client ) const
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
}

void OpacityFunctionVariableTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled within the timeout" );

    QTest::mouseClick( client.jump_button, Qt::LeftButton );

    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
        "m_jump_push_button did not become enabled again within the timeout" );
}

QDialog* OpacityFunctionVariableTest::findVisibleVariableEditor() const
{
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        auto* dialog = qobject_cast<QDialog*>( widget );
        if ( dialog != nullptr &&
             dialog->isVisible() &&
             dialog->objectName() == QStringLiteral( "VariableEditor" ) )
        {
            return dialog;
        }
    }

    return nullptr;
}

void OpacityFunctionVariableTest::setVariableEditorRows( QDialog* dialog, const QStringList& variables ) const
{
    QVERIFY2( dialog != nullptr, "VariableEditor dialog is null" );
    auto* tree_view = dialog->findChild<QTreeView*>( "treeView" );
    QVERIFY2( tree_view != nullptr, "VariableEditor treeView was not found" );

    auto* model = qobject_cast<QStandardItemModel*>( tree_view->model() );
    QVERIFY2( model != nullptr, "VariableEditor treeView model was not found" );
    QVERIFY2(
        model->rowCount() >= variables.size(),
        qPrintable(
            QStringLiteral( "VariableEditor row count %1 is less than requested variable count %2" )
                .arg( model->rowCount() )
                .arg( variables.size() ) ) );

    for ( int row = 0; row < variables.size(); ++row )
    {
        QStandardItem* item = model->item( row, 1 );
        QVERIFY2( item != nullptr, "VariableEditor variable item was not found" );
        item->setText( variables.at( row ) );
        QCOMPARE( item->text(), variables.at( row ) );
    }
}

void OpacityFunctionVariableTest::editOpacityVariablesWithDialog(
    const ClientHandles& client,
    const QStringList& variables,
    const QString& file_name,
    const QString& caption,
    bool accept_dialog )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.opacity_function_variable_editor_button->isEnabled(), "opacityFunctionVariableEditorPushButton is disabled" );

    bool dialog_handled = false;
    QTimer::singleShot(
        0,
        this,
        [this, variables, file_name, caption, accept_dialog, &dialog_handled]()
        {
            QDialog* dialog = nullptr;
            QVERIFY2(
                waitForCondition(
                    [this, &dialog]()
                    {
                        dialog = findVisibleVariableEditor();
                        return dialog != nullptr;
                    },
                    k_dialog_timeout_ms,
                    100 ),
                "VariableEditor did not open within the timeout" );

            dialog->raise();
            dialog->activateWindow();
            QTest::qWait( k_window_settle_ms );
            setVariableEditorRows( dialog, variables );
            saveScreenshot( file_name, caption );

            const char* button_name = accept_dialog ? "OKPushButton" : "cancelPushButton";
            auto* button = dialog->findChild<QPushButton*>( button_name );
            QVERIFY2( button != nullptr, qPrintable( QStringLiteral( "%1 was not found" ).arg( button_name ) ) );
            QVERIFY2( button->isEnabled(), qPrintable( QStringLiteral( "%1 is disabled" ).arg( button_name ) ) );
            QTest::mouseClick( button, Qt::LeftButton );
            dialog_handled = true;
        } );

    QTest::mouseClick( client.opacity_function_variable_editor_button, Qt::LeftButton );
    QVERIFY2( dialog_handled, "VariableEditor was not handled" );
    QTest::qWait( k_window_settle_ms );
}

void OpacityFunctionVariableTest::captureTransferFunctionEditorState(
    const ClientHandles& client,
    const QString& file_name,
    const QString& caption,
    int opacity_combo_index )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    if ( opacity_combo_index >= 0 )
    {
        setComboBoxIndex( client.opacity_function_combo_box, opacity_combo_index, "opacityFunctionComboBox" );
    }

    saveScreenshot( file_name, caption );
}

void OpacityFunctionVariableTest::initTestCase()
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
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/OpacityFunctionVariableTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

}

void OpacityFunctionVariableTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void OpacityFunctionVariableTest::opacity_function_variable()
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

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    setTransferFunctionCount( client, 2 );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: numberOfTransferFunctionSpinBoxに2を入力しました。" ) );

    const QStringList variables = { QStringLiteral( "q1*0.5" ), QStringLiteral( "q1^2" ) };
    editOpacityVariablesWithDialog(
        client,
        variables,
        QStringLiteral( "01_variable_editor_before_cancel.png" ),
        QStringLiteral( "VariableEditor.uiでFunction Variableを一括編集できることを確認" ),
        false );
    markStepCompleted( QStringLiteral( "VariableEditor.ui: treeViewの2列目の1行目にq1*0.5、2行目にq1^2を入力し、スクリーンショットを撮影してcancelPushButtonを押しました。" ) );

    captureTransferFunctionEditorState(
        client,
        QStringLiteral( "02_transfer_function_editor_after_cancel.png" ),
        QStringLiteral( "VariableEditor.uiでの編集がTransferFunctionEditor.uiに反映されなかったことを確認" ) );
    QCOMPARE( client.opacity_function_variable_line_edit->text(), QStringLiteral( "q1" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: キャンセル後にFunction Variableが反映されていない状態をスクリーンショット撮影しました。" ) );

    editOpacityVariablesWithDialog(
        client,
        variables,
        QStringLiteral( "03_variable_editor_before_ok.png" ),
        QStringLiteral( "VariableEditor.uiでFunction Variableを再度編集したことを確認" ),
        true );
    markStepCompleted( QStringLiteral( "VariableEditor.ui: treeViewの2列目の1行目にq1*0.5、2行目にq1^2を再度入力し、スクリーンショットを撮影してOKPushButtonを押しました。" ) );

    captureTransferFunctionEditorState(
        client,
        QStringLiteral( "04_transfer_function_editor_o1_after_ok.png" ),
        QStringLiteral( "VariableEditor.uiでO1のFunction Variable編集が反映されたことを確認" ) );
    QCOMPARE( client.opacity_function_variable_line_edit->text(), QStringLiteral( "q1*0.5" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: O1のFunction Variable編集が反映された状態をスクリーンショット撮影しました。" ) );

    captureTransferFunctionEditorState(
        client,
        QStringLiteral( "05_transfer_function_editor_o2_after_ok.png" ),
        QStringLiteral( "VariableEditor.uiでO2のFunction Variable編集が反映されたことを確認" ),
        1 );
    QCOMPARE( client.opacity_function_variable_line_edit->text(), QStringLiteral( "q1^2" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: opacityFunctionComboBoxの2番目を選択し、O2のFunction Variable編集が反映された状態をスクリーンショット撮影しました。" ) );

    m_test_succeeded = true;
}

}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::OpacityFunctionVariableTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
