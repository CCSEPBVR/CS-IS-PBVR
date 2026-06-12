#include "TransferFunctionEditorOpacityMapEditTest.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QDir>
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
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTest>
#include <QTimer>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../ExtendedQT/OpacityMapPalette.h"
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

QString OpacityMapEditTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString OpacityMapEditTest::repoRootPath() const
{
    const QString app_root = findRepoRootFrom( QCoreApplication::applicationDirPath() );
    if ( !app_root.isEmpty() ) { return app_root; }
    const QString cwd_root = findRepoRootFrom( QDir::currentPath() );
    if ( !cwd_root.isEmpty() ) { return cwd_root; }
    const QString source_root = findRepoRootFrom( QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() );
    if ( !source_root.isEmpty() ) { return source_root; }
    return QDir::currentPath();
}

QString OpacityMapEditTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool OpacityMapEditTest::waitForCondition(
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

void OpacityMapEditTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void OpacityMapEditTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void OpacityMapEditTest::bringDialogToFront( QDialog* dialog ) const
{
    QVERIFY2( dialog != nullptr, "Dialog is null" );
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void OpacityMapEditTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, text );
    QCOMPARE( line_edit->text(), text );
}

void OpacityMapEditTest::saveScreenshot( const QString& file_name, const QString& caption )
{
    if ( !ClientTests::screenshotsEnabled() ) { return; }

    QTest::qWait( k_capture_settle_ms );

    QScreen* screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen != nullptr, "Primary screen not found" );

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );
    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the primary screen" );
    QVERIFY2( screenshot.save( file_path ), qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( file_path ) ) );
    m_screenshots.push_back( { file_name, caption } );
}

void OpacityMapEditTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditorTest::OpacityMapEditTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n";
    stream << "- Freeform Curveのドラッグ編集確認: "
           << ( m_palette_drag_verified ? "PASS" : "未確認" )
           << "\n\n";

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

void OpacityMapEditTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

OpacityMapEditTest::ClientHandles OpacityMapEditTest::resolveClientHandles( MainWindow& window ) const
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
    handles.tf_opacity_map_edit_button = handles.transfer_function_editor->findChild<QPushButton*>( "opacityMapEditPushButton" );

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
    if ( !require( handles.tf_opacity_map_edit_button != nullptr, "TransferFunctionEditor opacityMapEditPushButton not found" ) ) { return handles; }
    return handles;
}

void OpacityMapEditTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2( waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
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

void OpacityMapEditTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_client_server_radio->isChecked(), "remoteVizClientServerRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, QDir::toNativeSeparators( m_volume_data_path ) );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void OpacityMapEditTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void OpacityMapEditTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2( waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
              "m_jump_push_button did not become enabled within the timeout" );
    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2( waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
              "m_jump_push_button did not become enabled again within the timeout" );
    QTest::qWait( k_after_jump_wait_ms );
}

QDialog* OpacityMapEditTest::waitForOpacityMapEditor() const
{
    QDialog* dialog = nullptr;
    const bool found = waitForCondition(
        [&dialog]()
        {
            for ( QWidget* widget : QApplication::topLevelWidgets() )
            {
                auto* candidate = qobject_cast<QDialog*>( widget );
                if ( candidate == nullptr ) { continue; }
                if ( candidate->objectName() != QStringLiteral( "OpacityMapEditor" ) ) { continue; }
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
        QTest::qFail( "OpacityMapEditor did not become visible", __FILE__, __LINE__ );
        return nullptr;
    }
    bringDialogToFront( dialog );
    return dialog;
}

int OpacityMapEditTest::tabIndexByObjectName( QTabWidget* tab_widget, const QString& object_name ) const
{
    if ( tab_widget == nullptr )
    {
        QTest::qFail( "OpacityMapEditor tabWidget not found", __FILE__, __LINE__ );
        return -1;
    }
    for ( int index = 0; index < tab_widget->count(); ++index )
    {
        QWidget* tab = tab_widget->widget( index );
        if ( tab != nullptr && tab->objectName() == object_name ) { return index; }
    }
    return -1;
}

void OpacityMapEditTest::selectTab( QDialog* dialog, const QString& object_name ) const
{
    auto* tab_widget = dialog->findChild<QTabWidget*>( "tabWidget" );
    const int index = tabIndexByObjectName( tab_widget, object_name );
    QVERIFY2( index >= 0, qPrintable( QStringLiteral( "OpacityMapEditor tab not found: %1" ).arg( object_name ) ) );
    tab_widget->setCurrentIndex( index );
    QCOMPARE( tab_widget->currentIndex(), index );
    QTest::qWait( k_window_settle_ms );
}

QVector<float> OpacityMapEditTest::opacityMapPaletteOpacities( QDialog* dialog ) const
{
    QWidget* palette_widget = dialog->findChild<QWidget*>( "opacityMapPalette" );
    auto* palette = static_cast<OpacityMapPalette*>( palette_widget );
    if ( palette == nullptr )
    {
        QTest::qFail( "opacityMapPalette not found", __FILE__, __LINE__ );
        return {};
    }

    return palette->getOpacities();
}

void OpacityMapEditTest::dragPalette( QDialog* dialog, const QPoint& start, const QPoint& end )
{
    QWidget* palette_widget = dialog->findChild<QWidget*>( "opacityMapPalette" );
    auto* palette = static_cast<OpacityMapPalette*>( palette_widget );
    QVERIFY2( palette != nullptr, "opacityMapPalette not found" );
    const QVector<float> before = palette->getOpacities();

    QTest::mousePress( palette, Qt::LeftButton, Qt::NoModifier, start );
    QTest::mouseMove( palette, end, 20 );
    QTest::mouseRelease( palette, Qt::LeftButton, Qt::NoModifier, end );
    QTest::qWait( k_short_wait_ms );

    const QVector<float> after = palette->getOpacities();
    QVERIFY2( before != after, "opacityMapPalette did not change after drag editing" );
    m_palette_drag_verified = true;
}

void OpacityMapEditTest::setControlPointRows( QDialog* dialog, const QVector<QVector<double>>& rows ) const
{
    auto* spin_box = dialog->findChild<QSpinBox*>( "numberOfControlPointsSpinBox" );
    auto* table = dialog->findChild<QTableWidget*>( "controlPointsTableWidget" );
    QVERIFY2( spin_box != nullptr, "numberOfControlPointsSpinBox not found" );
    QVERIFY2( table != nullptr, "controlPointsTableWidget not found" );
    spin_box->setValue( rows.size() );
    QCOMPARE( table->rowCount(), rows.size() );

    for ( int row = 0; row < rows.size(); ++row )
    {
        QCOMPARE( rows[row].size(), 2 );
        for ( int column = 0; column < 2; ++column )
        {
            auto* item = new QTableWidgetItem( QString::number( rows[row][column], 'g', 12 ) );
            table->setItem( row, column, item );
        }
    }

    const bool invoked = QMetaObject::invokeMethod( dialog, "onControlPointChanged", Qt::DirectConnection );
    QVERIFY2( invoked, "Failed to invoke OpacityMapEditor::onControlPointChanged" );
    QTest::qWait( k_short_wait_ms );
}

void OpacityMapEditTest::runOpacityMapEditorScenario( const ClientHandles& client )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_opacity_map_edit_button->isEnabled(), "opacityMapEditPushButton is disabled" );

    QTimer::singleShot(
        0,
        [this]()
        {
            QDialog* dialog = waitForOpacityMapEditor();
            const QVector<float> initial_opacities = opacityMapPaletteOpacities( dialog );
            saveScreenshot( QStringLiteral( "01_opacity_palette_before_edit.png" ), QStringLiteral( "編集前のopacityMapPaletteの状態" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: 編集前のopacityMapPaletteを撮影しました。" ) );

            dragPalette( dialog, QPoint( 50, 85 ), QPoint( 260, 15 ) );
            saveScreenshot( QStringLiteral( "02_opacity_palette_edited_first.png" ), QStringLiteral( "opacityMapPaletteをドラッグ編集した状態(1回目)" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: opacityMapPaletteを左クリックしながらドラッグして1回目の編集を行いました。" ) );

            dragPalette( dialog, QPoint( 300, 20 ), QPoint( 520, 80 ) );
            saveScreenshot( QStringLiteral( "03_opacity_palette_edited_second.png" ), QStringLiteral( "opacityMapPaletteをドラッグ編集した状態(2回目)" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: opacityMapPaletteを左クリックしながらドラッグして2回目の編集を行いました。" ) );

            auto* undo_button = dialog->findChild<QPushButton*>( "undoPushButton" );
            auto* redo_button = dialog->findChild<QPushButton*>( "redoPushButton" );
            QVERIFY2( undo_button != nullptr, "undoPushButton not found" );
            QVERIFY2( redo_button != nullptr, "redoPushButton not found" );
            QTest::mouseClick( undo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "04_opacity_palette_undo_first.png" ), QStringLiteral( "opacityMapPaletteのUndo(1回目)を行った状態" ) );
            QTest::mouseClick( undo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "05_opacity_palette_undo_second.png" ), QStringLiteral( "opacityMapPaletteのUndo(2回目)を行った状態" ) );
            QTest::mouseClick( redo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "06_opacity_palette_redo_first.png" ), QStringLiteral( "opacityMapPaletteのRedo(1回目)を行った状態" ) );
            QTest::mouseClick( redo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "07_opacity_palette_redo_second.png" ), QStringLiteral( "opacityMapPaletteのRedo(2回目)を行った状態" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: undoPushButtonとredoPushButtonを2回ずつ押しました。" ) );

            auto* reset_button = dialog->findChild<QPushButton*>( "resetPushButton" );
            QVERIFY2( reset_button != nullptr, "resetPushButton not found" );
            QTest::mouseClick( reset_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            QCOMPARE( opacityMapPaletteOpacities( dialog ), initial_opacities );
            saveScreenshot( QStringLiteral( "08_opacity_palette_after_reset.png" ), QStringLiteral( "編集したopacityMapPaletteが編集前の状態に戻った状態" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: resetPushButtonで編集前のopacityMapPaletteに戻しました。" ) );

            selectTab( dialog, QStringLiteral( "expression" ) );
            saveScreenshot( QStringLiteral( "09_expression_tab.png" ), QStringLiteral( "Expressionタブが表示されている状態" ) );
            auto* opacity_line_edit = dialog->findChild<QLineEdit*>( "opacityLineEdit" );
            const QVector<float> before_expression = opacityMapPaletteOpacities( dialog );
            setLineEditText( opacity_line_edit, QStringLiteral( "sin(x^x)*0.5" ) );
            QTest::qWait( k_short_wait_ms );
            QVERIFY2( opacityMapPaletteOpacities( dialog ) != before_expression, "opacityMapPalette did not change after editing opacityLineEdit" );
            saveScreenshot( QStringLiteral( "10_expression_opacity_sin_x_pow_x.png" ), QStringLiteral( "伝達関数式でOpacityMapが変更された状態" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: opacityLineEditにsin(x^x)*0.5を入力しました。" ) );

            selectTab( dialog, QStringLiteral( "controlPoints" ) );
            saveScreenshot( QStringLiteral( "11_control_points_tab.png" ), QStringLiteral( "controlPointsタブが開けた状態" ) );
            auto* spin_box = dialog->findChild<QSpinBox*>( "numberOfControlPointsSpinBox" );
            QVERIFY2( spin_box != nullptr, "numberOfControlPointsSpinBox not found" );
            spin_box->setValue( 3 );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "12_control_points_count_3.png" ), QStringLiteral( "制御点の数を3に増やした状態" ) );
            spin_box->setValue( 2 );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "13_control_points_count_2.png" ), QStringLiteral( "制御点の数を2に減らした状態" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: numberOfControlPointsSpinBoxで制御点数を増減しました。" ) );

            setControlPointRows( dialog, { { 0, 1 }, { 0.3, 0 }, { 0.7, 0 }, { 1, 1 } } );
            saveScreenshot( QStringLiteral( "14_control_points_set.png" ), QStringLiteral( "制御点を設定できた状態" ) );
            setControlPointRows( dialog, { { 0, 1 }, { 0.3, 0.5 }, { 0.7, 0.5 }, { 1, 1 } } );
            saveScreenshot( QStringLiteral( "15_control_points_edited.png" ), QStringLiteral( "制御点を編集できた状態" ) );
            markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: controlPointsTableWidgetの制御点を設定・編集しました。" ) );

            auto* cancel_button = dialog->findChild<QPushButton*>( "cancelPushButton" );
            QVERIFY2( cancel_button != nullptr, "cancelPushButton not found" );
            QTest::mouseClick( cancel_button, Qt::LeftButton );
        } );

    QTest::mouseClick( client.tf_opacity_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void OpacityMapEditTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() ) );
    m_server_executable = envOrDefault( "PBVR_SERVER_EXECUTABLE", ClientTests::configuredPath( "PBVR_SERVER_EXECUTABLE", repoRootPath() ) );
    m_server_target_wrapper_executable = envOrDefault( "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE", ClientTests::configuredPath( "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE", repoRootPath() ) );
    m_volume_data_path = envOrDefault( "PBVR_VOLUME_DATA", ClientTests::configuredPath( "SPX_VOLUME_DATA", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/OpacityMapEditTest" ) ) );
    m_screenshot_dir_path = envOrDefault( "PBVR_SCREENSHOT_DIR", QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;
    m_palette_drag_verified = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

}

void OpacityMapEditTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }
    writeMarkdownReport();
}

void OpacityMapEditTest::edit_opacity_map()
{
    if ( g_test_app == nullptr ) { g_test_app = pbvrTestApplication(); }
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
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.uiを開きました。" ) );
    runOpacityMapEditorScenario( client );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "16_tfe_after_opacity_cancel.png" ),
        QStringLiteral( "cancelPushButtonを押すとOpacityMapEditor.uiで編集した内容が反映されないことを表す状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: cancelPushButtonを押すとOpacityMapEditor.uiで編集した内容が反映されないことを表す状態を撮影しました。" ) );

    QTimer::singleShot(
        0,
        [this]()
        {
            QDialog* dialog = waitForOpacityMapEditor();
            selectTab( dialog, QStringLiteral( "expression" ) );
            auto* opacity_line_edit = dialog->findChild<QLineEdit*>( "opacityLineEdit" );
            setLineEditText( opacity_line_edit, QStringLiteral( "x*0.5" ) );
            QTest::qWait( k_short_wait_ms );
            selectTab( dialog, QStringLiteral( "expression" ) );
            bringDialogToFront( dialog );
            saveScreenshot( QStringLiteral( "17_expression_opacity_x_half.png" ), QStringLiteral( "OpacityMapの修正内容を表す状態" ) );
            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "OpacityMapEditor applyPushButton not found" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
        } );
    QTest::mouseClick( client.tf_opacity_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( QStringLiteral( "OpacityMapEditor.ui: Expressionタブを開き、opacityLineEditにx*0.5を入力し、applyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "18_tfe_after_opacity_apply.png" ),
        QStringLiteral( "OpacityMapEditor.uiで編集した内容がTransferFunctionEditorに反映された状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: apply後に編集内容が反映された状態を撮影しました。" ) );

    m_test_succeeded = true;
}

}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::OpacityMapEditTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
