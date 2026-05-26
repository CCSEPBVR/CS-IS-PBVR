#include "ColorMapSelectorToolBarTest.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextStream>
#include <QTest>
#include <QTimer>

#include <cmath>

#include "../App/MainWindow.h"
#include "../Widgets/ColorMapSelectorToolBar.h"
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
constexpr int k_combo_box_ready_timeout_ms = 120000;
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

QString ColorMapSelectorToolBarTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString ColorMapSelectorToolBarTest::repoRootPath() const
{
    const QString app_root = findRepoRootFrom( QCoreApplication::applicationDirPath() );
    if ( !app_root.isEmpty() ) { return app_root; }

    const QString cwd_root = findRepoRootFrom( QDir::currentPath() );
    if ( !cwd_root.isEmpty() ) { return cwd_root; }

    const QString source_root = findRepoRootFrom( QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() );
    if ( !source_root.isEmpty() ) { return source_root; }

    return QDir::currentPath();
}

QString ColorMapSelectorToolBarTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool ColorMapSelectorToolBarTest::waitForCondition(
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

void ColorMapSelectorToolBarTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ColorMapSelectorToolBarTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void ColorMapSelectorToolBarTest::bringDialogToFront( QDialog* dialog ) const
{
    QVERIFY2( dialog != nullptr, "Dialog is null" );
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ColorMapSelectorToolBarTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void ColorMapSelectorToolBarTest::setDoubleSpinBoxValue(
    QDoubleSpinBox* spin_box,
    double value,
    const char* widget_name ) const
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

void ColorMapSelectorToolBarTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void ColorMapSelectorToolBarTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# ColorMapSelectorToolBarTest\n\n";
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
        stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
    }
}

void ColorMapSelectorToolBarTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

ColorMapSelectorToolBarTest::ClientHandles ColorMapSelectorToolBarTest::resolveClientHandles( MainWindow& window ) const
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
    handles.color_map_selector_tool_bar = window.findChild<ColorMapSelectorToolBar*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_editor != nullptr, "TransferFunctionEditor not found" ) ) { return handles; }
    if ( !require( handles.color_map_selector_tool_bar != nullptr, "ColorMapSelectorToolBar not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio =
        handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.tf_apply_button = handles.transfer_function_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.tf_color_map_edit_button =
        handles.transfer_function_editor->findChild<QPushButton*>( "colorMapEditPushButton" );
    handles.color_user_defined_min_max_radio =
        handles.transfer_function_editor->findChild<QRadioButton*>( "colorUserDefinedMinMaxRadioButton" );
    handles.color_user_defined_min_spin_box =
        handles.transfer_function_editor->findChild<QDoubleSpinBox*>( "colorUserDefinedMinDoubleSpinBox" );
    handles.color_user_defined_max_spin_box =
        handles.transfer_function_editor->findChild<QDoubleSpinBox*>( "colorUserDefinedMaxDoubleSpinBox" );
    handles.number_of_transfer_function_spin_box =
        handles.transfer_function_editor->findChild<QSpinBox*>( "numberOfTransferFunctionSpinBox" );
    handles.tf_color_function_combo_box =
        handles.transfer_function_editor->findChild<QComboBox*>( "colorFunctionComboBox" );
    handles.selector_color_function_combo_box = handles.color_map_selector_tool_bar->findChild<QComboBox*>();

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
    if ( !require( handles.tf_color_map_edit_button != nullptr, "colorMapEditPushButton not found" ) ) { return handles; }
    if ( !require( handles.color_user_defined_min_max_radio != nullptr, "colorUserDefinedMinMaxRadioButton not found" ) ) { return handles; }
    if ( !require( handles.color_user_defined_min_spin_box != nullptr, "colorUserDefinedMinDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.color_user_defined_max_spin_box != nullptr, "colorUserDefinedMaxDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.number_of_transfer_function_spin_box != nullptr, "numberOfTransferFunctionSpinBox not found" ) ) { return handles; }
    if ( !require( handles.tf_color_function_combo_box != nullptr, "colorFunctionComboBox not found" ) ) { return handles; }
    if ( !require( handles.selector_color_function_combo_box != nullptr, "m_color_function_combo_box not found" ) ) { return handles; }

    return handles;
}

void ColorMapSelectorToolBarTest::connectClient( const ClientHandles& client ) const
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

void ColorMapSelectorToolBarTest::configureRemoteVisualization( const ClientHandles& client ) const
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

void ColorMapSelectorToolBarTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void ColorMapSelectorToolBarTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

void ColorMapSelectorToolBarTest::applyTransferFunction( const ClientHandles& client ) const
{
    QVERIFY2( client.tf_apply_button->isEnabled(), "TransferFunctionEditor applyPushButton is disabled" );
    QTest::mouseClick( client.tf_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ColorMapSelectorToolBarTest::selectComboBoxIndex(
    QComboBox* combo_box,
    int index,
    const char* widget_name ) const
{
    QVERIFY2( combo_box != nullptr, widget_name );
    QVERIFY2( combo_box->count() > index, qPrintable( QStringLiteral( "%1 does not have the requested item" ).arg( widget_name ) ) );

    combo_box->setFocus();
    combo_box->setCurrentIndex( index );
    QVERIFY2(
        waitForCondition( [combo_box, index]() { return combo_box->currentIndex() == index; }, 3000, 50 ),
        qPrintable( QStringLiteral( "Failed to select index %1 on %2" ).arg( index ).arg( widget_name ) ) );
    QTest::qWait( k_short_wait_ms );
}

QDialog* ColorMapSelectorToolBarTest::waitForColorMapEditor() const
{
    QDialog* dialog = nullptr;
    const bool found = waitForCondition(
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
        50 );

    if ( !found )
    {
        QTest::qFail( "ColorMapEditor did not become visible", __FILE__, __LINE__ );
        return nullptr;
    }

    bringDialogToFront( dialog );
    return dialog;
}

void ColorMapSelectorToolBarTest::doubleClickPresetColorMap( QDialog* dialog, const QString& preset_name ) const
{
    auto* table = dialog->findChild<QTableWidget*>( "colorMapTableWidget" );
    QVERIFY2( table != nullptr, "colorMapTableWidget not found" );

    for ( int row = 0; row < table->rowCount(); ++row )
    {
        for ( int column = 0; column < table->columnCount(); ++column )
        {
            QWidget* cell_widget = table->cellWidget( row, column );
            if ( cell_widget == nullptr ) { continue; }

            const QList<QLabel*> labels = cell_widget->findChildren<QLabel*>();
            for ( QLabel* label : labels )
            {
                if ( label->text() != preset_name ) { continue; }

                const QModelIndex index = table->model()->index( row, column );
                const QPoint click_pos = table->visualRect( index ).center();
                QTest::mouseDClick( table->viewport(), Qt::LeftButton, Qt::NoModifier, click_pos );
                const bool invoked = QMetaObject::invokeMethod(
                    dialog,
                    "onPresetColorMapDoubleClicked",
                    Qt::DirectConnection,
                    Q_ARG( int, row ),
                    Q_ARG( int, column ) );
                QVERIFY2( invoked, "Failed to invoke ColorMapEditor::onPresetColorMapDoubleClicked" );
                QTest::qWait( k_short_wait_ms );
                return;
            }
        }
    }

    QFAIL( qPrintable( QStringLiteral( "%1 was not found in colorMapTableWidget" ).arg( preset_name ) ) );
}

void ColorMapSelectorToolBarTest::applyPresetColorMapFromEditor(
    const ClientHandles& client,
    const QString& preset_name ) const
{
    QVERIFY2( client.tf_color_map_edit_button->isEnabled(), "colorMapEditPushButton is disabled" );

    QTimer::singleShot(
        0,
        [this, preset_name]()
        {
            QDialog* dialog = waitForColorMapEditor();
            doubleClickPresetColorMap( dialog, preset_name );

            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "ColorMapEditor applyPushButton not found" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
        } );

    QTest::mouseClick( client.tf_color_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ColorMapSelectorToolBarTest::initTestCase()
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
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "ColorMapSelectorToolBarTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

}

void ColorMapSelectorToolBarTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void ColorMapSelectorToolBarTest::performs_color_map_selector_toolbar_scenario()
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
        QStringLiteral( "00_color_map_selector_before_change.png" ),
        QStringLiteral( "変更前の色伝達関数のカラーマップと最大最小値" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 変更前の色伝達関数のカラーマップと最大最小値。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "01_tfe_before_change.png" ),
        QStringLiteral( "変更前のTransferFunctionEditor" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.uiを開き、変更前の状態を撮影しました。" ) );

    QTest::mouseClick( client.color_user_defined_min_max_radio, Qt::LeftButton );
    QVERIFY2( client.color_user_defined_min_max_radio->isChecked(), "colorUserDefinedMinMaxRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );
    saveScreenshot(
        QStringLiteral( "02_tfe_user_defined_min_max.png" ),
        QStringLiteral( "Color MapをServer Side Min MaxからUser Defined Min Maxに変更したTransferFunctionEditor" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: colorUserDefinedMinMaxRadioButtonを選択し、スクリーンショットを撮影しました。" ) );

    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "03_color_map_selector_user_defined_min_max.png" ),
        QStringLiteral( "Color MapをServer Side Min MaxからUser Defined Min Maxに変更した色伝達関数のカラーマップと最大最小値" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Max変更後の色伝達関数のカラーマップと最大最小値。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    setDoubleSpinBoxValue( client.color_user_defined_min_spin_box, 0.5, "colorUserDefinedMinDoubleSpinBox" );
    setDoubleSpinBoxValue( client.color_user_defined_max_spin_box, 0.6, "colorUserDefinedMaxDoubleSpinBox" );
    applyPresetColorMapFromEditor( client, QStringLiteral( "Warm Step" ) );
    markStepCompleted( QStringLiteral( "ColorMapEditor.ui: colorMapTableWidgetからWarm Stepをダブルクリックし、applyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "04_tfe_user_defined_0_5_0_6_warm_step.png" ),
        QStringLiteral( "Color MapのUser Defined Min Maxを0.5~0.6に設定しColor MapをWarm Stepに設定したTransferFunctionEditor" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Maxを0.5~0.6、Color MapをWarm Stepに設定したTransferFunctionEditor。" ) );

    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "05_color_map_selector_user_defined_0_5_0_6_warm_step.png" ),
        QStringLiteral( "Color MapのUser Defined Min Maxを0.5~0.6に設定しColor MapをWarm Stepに設定した色伝達関数のカラーマップと最大最小値" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: User Defined Min Maxを0.5~0.6、Color MapをWarm Stepに設定した色伝達関数のカラーマップと最大最小値。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    client.number_of_transfer_function_spin_box->setValue( 2 );
    QVERIFY2(
        waitForCondition( [client]() { return client.tf_color_function_combo_box->count() >= 2; }, 3000, 50 ),
        "colorFunctionComboBox did not get a second item" );
    selectComboBoxIndex( client.tf_color_function_combo_box, 1, "colorFunctionComboBox" );
    QTest::mouseClick( client.color_user_defined_min_max_radio, Qt::LeftButton );
    QVERIFY2( client.color_user_defined_min_max_radio->isChecked(), "colorUserDefinedMinMaxRadioButton was not checked for C2" );
    QTest::qWait( k_short_wait_ms );
    applyPresetColorMapFromEditor( client, QStringLiteral( "Traffic Lights Step" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: numberOfTransferFunctionSpinBoxを2に設定し、colorFunctionComboBoxの2番目を選択しました。" ) );
    markStepCompleted( QStringLiteral( "ColorMapEditor.ui: colorMapTableWidgetからTraffic Lights Stepをダブルクリックし、applyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "06_tfe_c2_traffic_lights_step.png" ),
        QStringLiteral( "C1からC2に変更し色伝達関数を設定したTransferFunctionEditor" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: C1からC2に変更し色伝達関数を設定したTransferFunctionEditor。" ) );

    applyTransferFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: applyPushButtonを押しました。" ) );
    client.transfer_function_editor->hide();

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.selector_color_function_combo_box->isEnabled() &&
                       client.selector_color_function_combo_box->count() >= 2;
            },
            k_combo_box_ready_timeout_ms,
            100 ),
        "m_color_function_combo_box did not become ready within the timeout" );
    selectComboBoxIndex( client.selector_color_function_combo_box, 1, "m_color_function_combo_box" );
    bringWindowToFront( client.main_window );
    saveScreenshot(
        QStringLiteral( "07_color_map_selector_c2_traffic_lights_step.png" ),
        QStringLiteral( "C1からC2に変更し色伝達関数を設定した色伝達関数のカラーマップと最大最小値" ) );
    markStepCompleted( QStringLiteral( "ColorMapSelectorToolBar.cpp: m_color_function_combo_boxの2番目を選択し、スクリーンショットを撮影しました。" ) );

    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::ColorMapSelectorToolBarTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
