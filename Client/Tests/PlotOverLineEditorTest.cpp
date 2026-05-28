#include "PlotOverLineEditorTest.h"

#include <QAction>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTest>
#include <QTextStream>
#include <QTreeView>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/GlyphEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/PlotOverLineEditor.h"
#include "../Widgets/VolumeTransform.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_disconnect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_glyph_editor_ready_timeout_ms = 120000;
constexpr int k_plot_over_line_ready_timeout_ms = 120000;
constexpr int k_combo_popup_timeout_ms = 5000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_post_jump_wait_ms = 3000;
constexpr int k_capture_settle_ms = 700;
constexpr int k_button_retry_count = 3;
constexpr int k_button_retry_wait_ms = 500;
kvs::qt::Application* g_test_app = nullptr;

void logStep( const QString& message )
{
    qInfo().noquote() << message;
}

QString connectionStateSummary(
    QPushButton* connect_button,
    QPushButton* disconnect_button,
    QLineEdit* id_line_edit )
{
    return QStringLiteral( "connectEnabled=%1 disconnectEnabled=%2 id='%3'" )
        .arg( connect_button != nullptr && connect_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
        .arg( disconnect_button != nullptr && disconnect_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
        .arg( id_line_edit != nullptr ? id_line_edit->text().trimmed() : QStringLiteral( "<null>" ) );
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

QString PlotOverLineEditorTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString PlotOverLineEditorTest::repoRootPath() const
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

QString PlotOverLineEditorTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

QString PlotOverLineEditorTest::serverProcessSummary() const
{
    auto& server_process = const_cast<QProcess&>( m_server_process );
    const QString stdout_text = server_process.isOpen()
        ? QString::fromLocal8Bit( server_process.readAllStandardOutput() ).trimmed()
        : QString();
    const QString stderr_text = server_process.isOpen()
        ? QString::fromLocal8Bit( server_process.readAllStandardError() ).trimmed()
        : QString();
    return QStringLiteral( "state=%1 exitCode=%2 exitStatus=%3 stdout=\"%4\" stderr=\"%5\"" )
        .arg( static_cast<int>( m_server_process.state() ) )
        .arg( m_server_process.exitCode() )
        .arg( static_cast<int>( m_server_process.exitStatus() ) )
        .arg( stdout_text )
        .arg( stderr_text );
}

bool PlotOverLineEditorTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void PlotOverLineEditorTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void PlotOverLineEditorTest::bringGlyphEditorToFront( GlyphEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "GlyphEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void PlotOverLineEditorTest::bringPlotOverLineEditorToFront( PlotOverLineEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "PlotOverLineEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void PlotOverLineEditorTest::bringVolumeTransformToFront( VolumeTransform* editor ) const
{
    QVERIFY2( editor != nullptr, "VolumeTransform is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void PlotOverLineEditorTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    if ( !text.isEmpty() )
    {
        QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    }
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void PlotOverLineEditorTest::setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const
{
    QVERIFY2( spin_box != nullptr, "Target double spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QTRY_VERIFY( qAbs( spin_box->value() - value ) < 1e-9 );
}

void PlotOverLineEditorTest::setSpinBoxValue( QSpinBox* spin_box, int value ) const
{
    QVERIFY2( spin_box != nullptr, "Target spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QTRY_COMPARE( spin_box->value(), value );
}

void PlotOverLineEditorTest::setGroupBoxChecked( QGroupBox* group_box, bool checked, const char* object_name ) const
{
    QVERIFY2( group_box != nullptr, "Target group box was not found" );
    QVERIFY2(
        waitForCondition(
            [group_box]()
            {
                return group_box->isEnabled() && group_box->isVisible();
            },
            k_connect_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "%1 did not become available within the timeout" ).arg( object_name ) ) );

    if ( group_box->isChecked() == checked ) { return; }

    group_box->setFocus();
    group_box->setChecked( checked );
    QTRY_COMPARE( group_box->isChecked(), checked );
}

void PlotOverLineEditorTest::selectRadioButton( QRadioButton* radio_button, const char* object_name ) const
{
    QVERIFY2( radio_button != nullptr, "Target radio button was not found" );

    if ( radio_button->isChecked() ) { return; }

    QVERIFY2(
        waitForCondition(
            [radio_button]()
            {
                return radio_button->isEnabled() && radio_button->isVisible();
            },
            k_connect_timeout_ms,
            100 ),
        qPrintable( QStringLiteral( "%1 did not become clickable within the timeout" ).arg( object_name ) ) );

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        QTest::mouseClick( radio_button, Qt::LeftButton );
        if ( radio_button->isChecked() ) { return; }
        QTest::qWait( k_button_retry_wait_ms );
    }

    radio_button->click();
    QVERIFY2(
        radio_button->isChecked(),
        qPrintable( QStringLiteral( "%1 was not checked" ).arg( object_name ) ) );
}

void PlotOverLineEditorTest::selectComboBoxItem( QComboBox* combo_box, int index ) const
{
    QVERIFY2( combo_box != nullptr, "Target combo box was not found" );
    QVERIFY2( combo_box->count() > index, "Target combo box does not have the requested index" );

    combo_box->setFocus();
    combo_box->showPopup();

    QVERIFY2(
        waitForCondition(
            [combo_box]()
            {
                auto* view = combo_box->view();
                return view != nullptr && view->isVisible();
            },
            k_combo_popup_timeout_ms,
            50 ),
        "Combo box popup did not open" );

    auto* view = combo_box->view();
    QVERIFY2( view != nullptr, "Combo box view was not found" );

    const QModelIndex target_index =
        combo_box->model()->index( index, combo_box->modelColumn(), combo_box->rootModelIndex() );
    QVERIFY2( target_index.isValid(), "Requested combo box item is invalid" );

    view->scrollTo( target_index );
    combo_box->setCurrentIndex( index );
    combo_box->hidePopup();

    QTRY_COMPARE( combo_box->currentIndex(), index );
}

void PlotOverLineEditorTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void PlotOverLineEditorTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# PlotOverLineEditorTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- SPXボリュームデータ: `" << m_structured_volume_data_path << "`\n";
    stream << "- MEJボリュームデータ: `" << m_unstructured_volume_data_path << "`\n";
    stream << "- MEJ伝達関数: `" << m_transfer_function_path << "`\n";
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
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 対象ウィジェットを objectName で取得できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Communication で接続、切断、再接続できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": ObjectEditor の nameLineEdit が更新され、Apply できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": PlotOverLineEditor の座標、Resolution、Target を設定できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Jump 実行後に m_jump_push_button が再度有効になること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": スクリーンショットと Markdown レポートを指定ディレクトリへ保存できること。\n\n";

    stream << "## 目視確認対象\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "- 要確認: " << entry.caption << "\n";
    }
    if ( m_screenshots.empty() )
    {
        stream << "- NOT RUN: スクリーンショットは保存されていない。\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.caption << "\n\n";
        stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
    }

    stream << "## 未自動化・保留事項\n\n";
    stream << "- グラフ表示内容が説明どおりかどうかは、保存したスクリーンショットを目視確認する。\n";
}

void PlotOverLineEditorTest::addStep( const QString& description )
{
    m_steps.push_back( { description, false } );
}

void PlotOverLineEditorTest::markStepCompleted( const QString& description )
{
    for ( StepEntry& step : m_steps )
    {
        if ( step.description == description )
        {
            step.completed = true;
            return;
        }
    }

    m_steps.push_back( { description, true } );
}

PlotOverLineEditorTest::ClientHandles PlotOverLineEditorTest::resolveClientHandles( MainWindow& window ) const
{
    ClientHandles handles;
    const auto require = [&handles]( bool condition, const char* message )
    {
        if ( condition ) { return true; }
        QTest::qFail( message, __FILE__, __LINE__ );
        return false;
    };

    handles.main_window = &window;
    handles.communication = window.findChild<Communication*>();
    handles.object_editor = window.findChild<ObjectEditor*>();
    handles.playback_tool_bar = window.findChild<::PlayBackControlToolBar*>();
    handles.glyph_editor = window.findChild<GlyphEditor*>();
    handles.plot_over_line_editor = window.findChild<PlotOverLineEditor*>();
    handles.volume_transform = window.findChild<VolumeTransform*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.glyph_editor != nullptr, "GlyphEditor not found" ) ) { return handles; }
    if ( !require( handles.plot_over_line_editor != nullptr, "PlotOverLineEditor not found" ) ) { return handles; }
    if ( !require( handles.volume_transform != nullptr, "VolumeTransform not found" ) ) { return handles; }

    const auto actions = window.findChildren<QAction*>();
    for ( QAction* action : actions )
    {
        if ( action == nullptr ) { continue; }

        if ( action->text() == QStringLiteral( "Glyph Editor" ) )
        {
            handles.glyph_editor_action = action;
        }
        else if ( action->text() == QStringLiteral( "Plot Over Line Editor" ) )
        {
            handles.plot_over_line_editor_action = action;
        }
        else if ( action->text() == QStringLiteral( "Volume Transform" ) )
        {
            handles.volume_transform_action = action;
        }
    }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.transfer_function_path_line_edit = handles.communication->findChild<QLineEdit*>( "transferFunctionFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.is_operator_line_edit = handles.communication->findChild<QLineEdit*>( "isOperatorLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );

    handles.scale_factor_spin_box = handles.glyph_editor->findChild<QDoubleSpinBox*>( "scaleFactorDoubleSpinBox" );
    handles.size_variable_array_radio = handles.glyph_editor->findChild<QRadioButton*>( "sizeVariableArrayRadioButton" );
    handles.size_number_of_variables_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "sizeNumberOfVariablesSpinBox" );
    handles.size_variable_grid_layout = handles.glyph_editor->findChild<QGridLayout*>( "sizeVariableGridLayout" );
    handles.color_data_variable_array_radio = handles.glyph_editor->findChild<QRadioButton*>( "colorDataVariableArrayRadioButton" );
    handles.color_data_number_of_variables_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "colorDataNumberOfVariablesSpinBox" );
    handles.color_data_variable_grid_layout = handles.glyph_editor->findChild<QGridLayout*>( "colorDataVariableGridLayout" );
    handles.glyph_apply_button = handles.glyph_editor->findChild<QPushButton*>( "applyPushButton" );

    handles.plot_over_line_group_box = handles.plot_over_line_editor->findChild<QGroupBox*>( "plotOverLineGroupBox" );
    handles.target_combo_box = handles.plot_over_line_editor->findChild<QComboBox*>( "targetComboBox" );
    handles.resolution_spin_box = handles.plot_over_line_editor->findChild<QSpinBox*>( "resolutionSpinBox" );
    handles.start_coords_x_spin_box = handles.plot_over_line_editor->findChild<QDoubleSpinBox*>( "startCoordsXDoubleSpinBox" );
    handles.start_coords_y_spin_box = handles.plot_over_line_editor->findChild<QDoubleSpinBox*>( "startCoordsYDoubleSpinBox" );
    handles.start_coords_z_spin_box = handles.plot_over_line_editor->findChild<QDoubleSpinBox*>( "startCoordsZDoubleSpinBox" );
    handles.end_coords_x_spin_box = handles.plot_over_line_editor->findChild<QDoubleSpinBox*>( "endCoordsXDoubleSpinBox" );
    handles.end_coords_y_spin_box = handles.plot_over_line_editor->findChild<QDoubleSpinBox*>( "endCoordsYDoubleSpinBox" );
    handles.end_coords_z_spin_box = handles.plot_over_line_editor->findChild<QDoubleSpinBox*>( "endCoordsZDoubleSpinBox" );
    handles.plot_over_line_apply_button = handles.plot_over_line_editor->findChild<QPushButton*>( "applyPushButton" );

    handles.rotation_x_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationXAxisDoubleSpinBox" );
    handles.volume_transform_apply_button = handles.volume_transform->findChild<QPushButton*>( "applyPushButton" );

    if ( !require( handles.glyph_editor_action != nullptr, "Glyph Editor action not found" ) ) { return handles; }
    if ( !require( handles.plot_over_line_editor_action != nullptr, "Plot Over Line Editor action not found" ) ) { return handles; }
    if ( !require( handles.volume_transform_action != nullptr, "Volume Transform action not found" ) ) { return handles; }
    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_path_line_edit != nullptr, "transferFunctionFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.is_operator_line_edit != nullptr, "isOperatorLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.scale_factor_spin_box != nullptr, "scaleFactorDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.size_variable_array_radio != nullptr, "sizeVariableArrayRadioButton not found" ) ) { return handles; }
    if ( !require( handles.size_number_of_variables_spin_box != nullptr, "sizeNumberOfVariablesSpinBox not found" ) ) { return handles; }
    if ( !require( handles.size_variable_grid_layout != nullptr, "sizeVariableGridLayout not found" ) ) { return handles; }
    if ( !require( handles.color_data_variable_array_radio != nullptr, "colorDataVariableArrayRadioButton not found" ) ) { return handles; }
    if ( !require( handles.color_data_number_of_variables_spin_box != nullptr, "colorDataNumberOfVariablesSpinBox not found" ) ) { return handles; }
    if ( !require( handles.color_data_variable_grid_layout != nullptr, "colorDataVariableGridLayout not found" ) ) { return handles; }
    if ( !require( handles.glyph_apply_button != nullptr, "GlyphEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.plot_over_line_group_box != nullptr, "plotOverLineGroupBox not found" ) ) { return handles; }
    if ( !require( handles.target_combo_box != nullptr, "targetComboBox not found" ) ) { return handles; }
    if ( !require( handles.resolution_spin_box != nullptr, "resolutionSpinBox not found" ) ) { return handles; }
    if ( !require( handles.start_coords_x_spin_box != nullptr, "startCoordsXDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.start_coords_y_spin_box != nullptr, "startCoordsYDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.start_coords_z_spin_box != nullptr, "startCoordsZDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.end_coords_x_spin_box != nullptr, "endCoordsXDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.end_coords_y_spin_box != nullptr, "endCoordsYDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.end_coords_z_spin_box != nullptr, "endCoordsZDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.plot_over_line_apply_button != nullptr, "PlotOverLineEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.rotation_x_axis_spin_box != nullptr, "rotationXAxisDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.volume_transform_apply_button != nullptr, "VolumeTransform applyPushButton not found" ) ) { return handles; }

    return handles;
}

void PlotOverLineEditorTest::ensureConnected( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "ensureConnected: begin" ) );
    bringWindowToFront( client.main_window );

    const auto is_connected = [client]()
    {
        return client.disconnect_button->isEnabled() &&
               !client.connect_button->isEnabled() &&
               !client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_connected() )
    {
        logStep(
            QStringLiteral( "ensureConnected: already connected %1" )
                .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );
        return;
    }

    logStep(
        QStringLiteral( "ensureConnected: initial %1 isOperator='%2' address='%3'" )
            .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) )
            .arg( client.is_operator_line_edit != nullptr ? client.is_operator_line_edit->text() : QStringLiteral( "<null>" ) )
            .arg( client.communication->findChild<QLineEdit*>( "addressLineEdit" ) != nullptr ?
                      client.communication->findChild<QLineEdit*>( "addressLineEdit" )->text() :
                      QStringLiteral( "<null>" ) ) );
    logStep( QStringLiteral( "ensureConnected: server %1" ).arg( serverProcessSummary() ) );

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        logStep(
            QStringLiteral( "ensureConnected: attempt %1 waiting %2" )
                .arg( attempt + 1 )
                .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );

        QVERIFY2(
            waitForCondition(
                [client]()
                {
                    return client.connect_button->isEnabled();
                },
                k_connect_timeout_ms,
                100 ),
            "connectPushButton did not become enabled within the timeout" );

        bringWindowToFront( client.main_window );
        QTest::mouseClick( client.connect_button, Qt::LeftButton );

        logStep(
            QStringLiteral( "ensureConnected: attempt %1 clicked %2" )
                .arg( attempt + 1 )
                .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );
        logStep(
            QStringLiteral( "ensureConnected: attempt %1 server-after-click %2" )
                .arg( attempt + 1 )
                .arg( serverProcessSummary() ) );

        if ( waitForCondition( is_connected, k_connect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            logStep(
                QStringLiteral( "ensureConnected: completed %1 isOperator='%2'" )
                    .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) )
                    .arg( client.is_operator_line_edit->text() ) );
            return;
        }

        logStep(
            QStringLiteral( "ensureConnected: attempt %1 timed out %2 isOperator='%3'" )
                .arg( attempt + 1 )
                .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) )
                .arg( client.is_operator_line_edit->text() ) );
        logStep(
            QStringLiteral( "ensureConnected: attempt %1 server-after-timeout %2" )
                .arg( attempt + 1 )
                .arg( serverProcessSummary() ) );

        QTest::qWait( k_button_retry_wait_ms );
    }

    logStep(
        QStringLiteral( "ensureConnected: failed %1 isOperator='%2'" )
            .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) )
            .arg( client.is_operator_line_edit->text() ) );
    logStep( QStringLiteral( "ensureConnected: failed server %1" ).arg( serverProcessSummary() ) );
    QFAIL( "Client did not enter the connected state after clicking connectPushButton" );
}

void PlotOverLineEditorTest::ensureDisconnected( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "ensureDisconnected: begin" ) );
    bringWindowToFront( client.main_window );

    const auto is_disconnected = [client]()
    {
        return client.connect_button->isEnabled() &&
               !client.disconnect_button->isEnabled() &&
               client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_disconnected() )
    {
        logStep( QStringLiteral( "ensureDisconnected: already disconnected" ) );
        return;
    }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
        QVERIFY2(
            waitForCondition(
                [client]()
                {
                    return client.disconnect_button->isEnabled();
                },
                k_disconnect_timeout_ms,
                100 ),
            "disconnectPushButton did not become enabled within the timeout" );

        bringWindowToFront( client.main_window );
        QTest::mouseClick( client.disconnect_button, Qt::LeftButton );

        if ( waitForCondition( is_disconnected, k_disconnect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            logStep( QStringLiteral( "ensureDisconnected: completed" ) );
            return;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    QFAIL( "Client did not enter the disconnected state after clicking disconnectPushButton" );
}

void PlotOverLineEditorTest::waitForOperatorPrivileges( const ClientHandles& client ) const
{
    const bool ready = waitForCondition(
        [client]()
        {
            return client.object_apply_button->isEnabled() &&
                   client.plot_over_line_apply_button->isEnabled() &&
                   client.plot_over_line_group_box->isEnabled();
        },
        k_connect_timeout_ms,
        100 );

    if ( !ready )
    {
        qInfo().noquote()
            << QStringLiteral(
                   "Operator-capable state was not reached. isOperator='%1' objectApplyEnabled=%2 plotOverLineApplyEnabled=%3 plotOverLineGroupEnabled=%4" )
                   .arg( client.is_operator_line_edit->text() )
                   .arg( client.object_apply_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.plot_over_line_apply_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.plot_over_line_group_box->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    }

    QVERIFY2( ready, "Operator-capable state was not reached within the timeout" );
}

void PlotOverLineEditorTest::configureRemoteVisualization(
    const ClientHandles& client,
    const QString& volume_path,
    const QString& transfer_function_path ) const
{
    ensureConnected( client );
    waitForOperatorPrivileges( client );

    bringWindowToFront( client.main_window );
    selectRadioButton( client.remote_viz_client_server_radio, "remoteVizClientServerRadioButton" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, volume_path );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.transfer_function_path_line_edit, transfer_function_path );
    QTest::qWait( k_short_wait_ms );

    client.object_name_line_edit->clear();
    QCOMPARE( client.object_name_line_edit->text(), QString() );

    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

QStandardItemModel* PlotOverLineEditorTest::waitForObjectModel( const ClientHandles& client ) const
{
    if ( !waitForCondition(
             [client]()
             {
                 return client.object_apply_button->isEnabled() &&
                        client.object_tree_view->model() != nullptr &&
                        client.object_tree_view->model()->rowCount() > 0 &&
                        !client.object_name_line_edit->text().trimmed().isEmpty();
             },
             k_object_load_timeout_ms,
             100 ) )
    {
        return nullptr;
    }

    auto* model = qobject_cast<QStandardItemModel*>( client.object_tree_view->model() );
    if ( model == nullptr )
    {
        return nullptr;
    }

    const QModelIndex first_index = model->index( 0, 0 );
    if ( !first_index.isValid() )
    {
        return nullptr;
    }

    client.object_tree_view->scrollTo( first_index );
    client.object_tree_view->setCurrentIndex( first_index );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        first_index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    return model;
}

void PlotOverLineEditorTest::setObjectDisplayItemChecked(
    const ClientHandles& client,
    int row,
    bool checked ) const
{
    QStandardItemModel* model = waitForObjectModel( client );
    QVERIFY2( model != nullptr, "Failed to resolve ObjectEditor model" );
    QVERIFY2( row >= 0 && row < model->rowCount(), "Requested ObjectEditor row is out of range" );

    auto* display_item = model->item( row, 2 );
    QVERIFY2( display_item != nullptr, "ObjectEditor display item was not found" );

    display_item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
    QCOMPARE( display_item->checkState(), checked ? Qt::Checked : Qt::Unchecked );
}

void PlotOverLineEditorTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForObjectAndApply: waiting for ObjectEditor nameLineEdit" ) );
    QStandardItemModel* model = waitForObjectModel( client );
    QVERIFY2( model != nullptr, "Failed to resolve ObjectEditor model" );

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_post_jump_wait_ms );
}

QComboBox* PlotOverLineEditorTest::comboBoxAtGridRow( QGridLayout* grid_layout, int row ) const
{
    if ( grid_layout == nullptr )
    {
        QTest::qFail( "Grid layout was not found", __FILE__, __LINE__ );
        return nullptr;
    }

    QLayoutItem* item = grid_layout->itemAtPosition( row, 1 );
    if ( item == nullptr )
    {
        QTest::qFail( "Grid layout item was not found", __FILE__, __LINE__ );
        return nullptr;
    }

    auto* combo_box = qobject_cast<QComboBox*>( item->widget() );
    if ( combo_box == nullptr )
    {
        QTest::qFail( "Grid layout combo box was not found", __FILE__, __LINE__ );
        return nullptr;
    }

    return combo_box;
}

void PlotOverLineEditorTest::waitForGlyphEditorReady( const ClientHandles& client ) const
{
    const bool ready = waitForCondition(
        [client]()
        {
            return client.glyph_editor_action->isEnabled() &&
                   client.scale_factor_spin_box->isEnabled() &&
                   client.size_number_of_variables_spin_box->maximum() >= 3 &&
                   client.color_data_number_of_variables_spin_box->maximum() >= 3;
        },
        k_glyph_editor_ready_timeout_ms,
        100 );

    if ( !ready )
    {
        qInfo().noquote()
            << QStringLiteral(
                   "GlyphEditor ready state: actionEnabled=%1 scaleEnabled=%2 sizeMax=%3 colorDataMax=%4 objectName='%5'" )
                   .arg( client.glyph_editor_action->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.scale_factor_spin_box->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.size_number_of_variables_spin_box->maximum() )
                   .arg( client.color_data_number_of_variables_spin_box->maximum() )
                   .arg( client.object_name_line_edit->text() );
    }

    QVERIFY2( ready, "GlyphEditor did not become ready within the timeout" );
}

void PlotOverLineEditorTest::openGlyphEditor( const ClientHandles& client ) const
{
    waitForGlyphEditorReady( client );

    bringWindowToFront( client.main_window );
    client.glyph_editor_action->trigger();

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.glyph_editor->isVisible();
            },
            5000,
            50 ),
        "GlyphEditor did not become visible" );

    bringGlyphEditorToFront( client.glyph_editor );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::configureGlyphEditor( const ClientHandles& client ) const
{
    bringGlyphEditorToFront( client.glyph_editor );

    setDoubleSpinBoxValue( client.scale_factor_spin_box, 0.1 );
    QTest::qWait( k_short_wait_ms );

    selectRadioButton( client.size_variable_array_radio, "sizeVariableArrayRadioButton" );
    QTest::qWait( k_short_wait_ms );

    setSpinBoxValue( client.size_number_of_variables_spin_box, 3 );
    QTest::qWait( k_short_wait_ms );

    selectComboBoxItem( comboBoxAtGridRow( client.size_variable_grid_layout, 1 ), 1 );
    QTest::qWait( k_short_wait_ms );

    selectComboBoxItem( comboBoxAtGridRow( client.size_variable_grid_layout, 2 ), 2 );
    QTest::qWait( k_short_wait_ms );

    selectRadioButton( client.color_data_variable_array_radio, "colorDataVariableArrayRadioButton" );
    QTest::qWait( k_short_wait_ms );

    setSpinBoxValue( client.color_data_number_of_variables_spin_box, 3 );
    QTest::qWait( k_short_wait_ms );

    selectComboBoxItem( comboBoxAtGridRow( client.color_data_variable_grid_layout, 1 ), 1 );
    QTest::qWait( k_short_wait_ms );

    selectComboBoxItem( comboBoxAtGridRow( client.color_data_variable_grid_layout, 2 ), 2 );
    QTest::qWait( k_short_wait_ms );

    QTest::mouseClick( client.glyph_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::openVolumeTransform( const ClientHandles& client ) const
{
    QVERIFY2( client.volume_transform_action != nullptr, "Volume Transform action is null" );

    bringWindowToFront( client.main_window );
    client.volume_transform_action->trigger();

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.volume_transform->isVisible();
            },
            5000,
            50 ),
        "VolumeTransform did not become visible" );

    bringVolumeTransformToFront( client.volume_transform );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::configureVolumeTransform( const ClientHandles& client ) const
{
    bringVolumeTransformToFront( client.volume_transform );
    setDoubleSpinBoxValue( client.rotation_x_axis_spin_box, -90.0 );
    QTest::qWait( k_short_wait_ms );

    QTest::mouseClick( client.volume_transform_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::waitForPlotOverLineEditorReady( const ClientHandles& client, int minimum_target_count ) const
{
    const bool ready = waitForCondition(
        [client, minimum_target_count]()
        {
            return client.plot_over_line_editor_action->isEnabled() &&
                   client.plot_over_line_group_box->isEnabled() &&
                   client.target_combo_box->count() >= minimum_target_count;
        },
        k_plot_over_line_ready_timeout_ms,
        100 );

    if ( !ready )
    {
        qInfo().noquote()
            << QStringLiteral(
                   "PlotOverLineEditor ready state: actionEnabled=%1 groupEnabled=%2 targetCount=%3 isOperator='%4' objectName='%5'" )
                   .arg( client.plot_over_line_editor_action->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.plot_over_line_group_box->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.target_combo_box->count() )
                   .arg( client.is_operator_line_edit->text() )
                   .arg( client.object_name_line_edit->text() );
    }

    QVERIFY2( ready, "PlotOverLineEditor did not become ready within the timeout" );
}

void PlotOverLineEditorTest::openPlotOverLineEditor( const ClientHandles& client, int minimum_target_count ) const
{
    waitForOperatorPrivileges( client );
    waitForPlotOverLineEditorReady( client, minimum_target_count );

    bringWindowToFront( client.main_window );
    client.plot_over_line_editor_action->trigger();

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.plot_over_line_editor->isVisible();
            },
            5000,
            50 ),
        "PlotOverLineEditor did not become visible" );

    bringPlotOverLineEditorToFront( client.plot_over_line_editor );
}

void PlotOverLineEditorTest::configurePlotOverLine(
    const ClientHandles& client,
    double start_x,
    double start_y,
    double start_z,
    double end_x,
    double end_y,
    double end_z,
    int target_index ) const
{
    bringPlotOverLineEditorToFront( client.plot_over_line_editor );
    setGroupBoxChecked( client.plot_over_line_group_box, true, "plotOverLineGroupBox" );
    QTest::qWait( k_short_wait_ms );

    setDoubleSpinBoxValue( client.start_coords_x_spin_box, start_x );
    QTest::qWait( k_short_wait_ms );
    setDoubleSpinBoxValue( client.start_coords_y_spin_box, start_y );
    QTest::qWait( k_short_wait_ms );
    setDoubleSpinBoxValue( client.start_coords_z_spin_box, start_z );
    QTest::qWait( k_short_wait_ms );
    setDoubleSpinBoxValue( client.end_coords_x_spin_box, end_x );
    QTest::qWait( k_short_wait_ms );
    setDoubleSpinBoxValue( client.end_coords_y_spin_box, end_y );
    QTest::qWait( k_short_wait_ms );
    setDoubleSpinBoxValue( client.end_coords_z_spin_box, end_z );
    QTest::qWait( k_short_wait_ms );

    if ( target_index >= 0 )
    {
        selectComboBoxItem( client.target_combo_box, target_index );
        QTest::qWait( k_short_wait_ms );
    }

    QTest::mouseClick( client.plot_over_line_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::applyPlotOverLineResolution( const ClientHandles& client, int resolution ) const
{
    bringPlotOverLineEditorToFront( client.plot_over_line_editor );
    setSpinBoxValue( client.resolution_spin_box, resolution );
    QTest::qWait( k_short_wait_ms );

    QTest::mouseClick( client.plot_over_line_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void PlotOverLineEditorTest::prepareSecondDatasetConnection( const ClientHandles& client ) const
{
    ensureDisconnected( client );
    ensureConnected( client );

    if ( waitForCondition(
             [client]()
             {
                 return client.object_apply_button->isEnabled() &&
                        client.plot_over_line_apply_button->isEnabled();
             },
             5000,
             100 ) )
    {
        return;
    }

    logStep( QStringLiteral( "prepareSecondDatasetConnection: reconnecting the client to restore operator state" ) );
    ensureDisconnected( client );
    ensureConnected( client );
    waitForOperatorPrivileges( client );
}

void PlotOverLineEditorTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    const QString default_client_executable =
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() );

    m_client_executable = envOrDefault( "PBVR_CLIENT_EXECUTABLE", default_client_executable );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_EXECUTABLE", repoRootPath() ) );
    m_server_target_wrapper_executable = envOrDefault(
        "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE", repoRootPath() ) );
    m_structured_volume_data_path = envOrDefault(
        "SPX_VOLUME_DATA",
        ClientTests::configuredPath( "SPX_VOLUME_DATA", repoRootPath() ) );
    m_unstructured_volume_data_path = envOrDefault(
        "MEJ_VOLUME_DATA",
        ClientTests::configuredPath( "MEJ_VOLUME_DATA", repoRootPath() ) );
    m_transfer_function_path = envOrDefault(
        "MEJ_TRANSFER_FUNCTION",
        ClientTests::configuredPath( "MEJ_TRANSFER_FUNCTION", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "PlotOverLineEditorTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );

    QVERIFY2(
        QFileInfo::exists( m_structured_volume_data_path ),
        qPrintable( QStringLiteral( "Structured volume data file not found: %1" ).arg( m_structured_volume_data_path ) ) );
    QVERIFY2(
        QFileInfo::exists( m_unstructured_volume_data_path ),
        qPrintable( QStringLiteral( "Unstructured volume data file not found: %1" ).arg( m_unstructured_volume_data_path ) ) );
    QVERIFY2(
        QFileInfo::exists( m_transfer_function_path ),
        qPrintable( QStringLiteral( "Transfer function file not found: %1" ).arg( m_transfer_function_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QTest::qWait( 500 );
}

void PlotOverLineEditorTest::cleanupTestCase()
{
    writeMarkdownReport();

    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }
}

void PlotOverLineEditorTest::performs_plot_over_line_editor_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
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

    bool scenario_aborted = false;
    const auto run_step = [this, &scenario_aborted]( const QString& description, const std::function<void()>& body )
    {
        if ( scenario_aborted )
        {
            addStep( description );
            return;
        }

        addStep( description );
        logStep( QStringLiteral( "scenario: %1" ).arg( description ) );
        body();
        if ( QTest::currentTestFailed() )
        {
            scenario_aborted = true;
            return;
        }
        markStepCompleted( description );
    };

    run_step(
        QStringLiteral( "Communication で接続し、SPX_VOLUME_DATA を Remote Viz Client/Server に設定する。" ),
        [&]()
        {
            ensureConnected( client );
            configureRemoteVisualization( client, m_structured_volume_data_path, QString() );
        } );

    run_step(
        QStringLiteral( "ObjectEditor の nameLineEdit にテキストが入るまで待機し、Apply 後に Jump する。" ),
        [&]()
        {
            waitForObjectAndApply( client );
            clickJumpAndWaitForCompletion( client );
            client.object_editor->close();
            QTest::qWait( k_short_wait_ms );
        } );

    run_step(
        QStringLiteral( "PlotOverLineEditor を開き、SPX の線分座標を設定して解像度 256 のグラフを撮影する。" ),
        [&]()
        {
            openPlotOverLineEditor( client, 1 );
            setSpinBoxValue( client.resolution_spin_box, 256 );
            configurePlotOverLine( client, 4.1, 3.5, -4.5, 10.0, 3.5, -4.5 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "01_spx_resolution_256.png" ),
                QStringLiteral( "解像度を 256 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "resolutionSpinBox を 128 に設定してグラフを撮影する。" ),
        [&]()
        {
            applyPlotOverLineResolution( client, 128 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "02_spx_resolution_128.png" ),
                QStringLiteral( "解像度を 128 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "resolutionSpinBox を 32 に設定してグラフを撮影する。" ),
        [&]()
        {
            applyPlotOverLineResolution( client, 32 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "03_spx_resolution_32.png" ),
                QStringLiteral( "解像度を 32 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "resolutionSpinBox を 16 に設定してグラフを撮影する。" ),
        [&]()
        {
            applyPlotOverLineResolution( client, 16 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "04_spx_resolution_16.png" ),
                QStringLiteral( "解像度を 16 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "Communication で切断、再接続し、MEJ_VOLUME_DATA と MEJ_TRANSFER_FUNCTION を設定する。" ),
        [&]()
        {
            prepareSecondDatasetConnection( client );
            configureRemoteVisualization( client, m_unstructured_volume_data_path, m_transfer_function_path );
        } );

    run_step(
        QStringLiteral( "ObjectEditor を開き、2 行目の displayItem を OFF にして Apply 後に Jump する。" ),
        [&]()
        {
            client.object_editor->show();
            bringWindowToFront( client.main_window );
            QTest::qWait( k_short_wait_ms );
            QStandardItemModel* model = waitForObjectModel( client );
            QVERIFY2( model != nullptr, "Failed to resolve ObjectEditor model" );
            setObjectDisplayItemChecked( client, 1, false );
            QTest::qWait( k_short_wait_ms );
            QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            clickJumpAndWaitForCompletion( client );
            client.object_editor->close();
            QTest::qWait( k_short_wait_ms );
        } );

    run_step(
        QStringLiteral( "VolumeTransform を開き、rotationXAxisDoubleSpinBox に -90 を設定して Apply 後に閉じる。" ),
        [&]()
        {
            openVolumeTransform( client );
            setDoubleSpinBoxValue( client.rotation_x_axis_spin_box, -90.0 );
            QTest::qWait( k_short_wait_ms );
            QTest::mouseClick( client.volume_transform_apply_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            client.volume_transform->close();
            QTest::qWait( k_short_wait_ms );
        } );

    run_step(
        QStringLiteral( "PlotOverLineEditor で MEJ の線分座標を設定し、Target q1 のグラフを撮影する。" ),
        [&]()
        {
            openPlotOverLineEditor( client, 4 );
            configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 0 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "05_mej_target_q1.png" ),
                QStringLiteral( "Target を q1 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "targetComboBox の 2 番目を選択し、Target q2 のグラフを撮影する。" ),
        [&]()
        {
            configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 1 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "06_mej_target_q2.png" ),
                QStringLiteral( "Target を q2 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "targetComboBox の 3 番目を選択し、Target q3 のグラフを撮影する。" ),
        [&]()
        {
            configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 2 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "07_mej_target_q3.png" ),
                QStringLiteral( "Target を q3 に設定した Plot Over Line グラフ。" ) );
        } );

    run_step(
        QStringLiteral( "targetComboBox の 4 番目を選択し、Target q4 のグラフを撮影する。" ),
        [&]()
        {
            configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 3 );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "08_mej_target_q4.png" ),
                QStringLiteral( "Target を q4 に設定した Plot Over Line グラフ。" ) );
        } );

    bringWindowToFront( client.main_window );
    logStep( QStringLiteral( "scenario: completed" ) );
    m_test_succeeded = !scenario_aborted && !QTest::currentTestFailed();
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::PlotOverLineEditorTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
