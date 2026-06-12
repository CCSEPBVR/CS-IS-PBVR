#include "GlyphEditorTest.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QComboBox>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QTableWidget>
#include <QTest>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/GlyphEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TotalParticlesToolBar.h"
#include "../Widgets/VolumeTransform.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

#include <csignal>

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_disconnect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_glyph_editor_ready_timeout_ms = 120000;
constexpr int k_combo_popup_timeout_ms = 5000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 500;
constexpr int k_jump_finish_wait_ms = 1500;
constexpr int k_capture_settle_ms = 500;
constexpr int k_button_retry_count = 3;
constexpr int k_button_retry_wait_ms = 500;
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

GlyphEditorTest::GlyphEditorTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArray( "1200000" ) );
}

QString GlyphEditorTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString GlyphEditorTest::repoRootPath() const
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

QString GlyphEditorTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool GlyphEditorTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void GlyphEditorTest::startVideoRecording()
{
#ifdef Q_OS_WIN
    return;
#else
    if ( QFileInfo::exists( m_video_file_path ) )
    {
        QVERIFY2(
            QFile::remove( m_video_file_path ),
            qPrintable( QStringLiteral( "Failed to remove existing video: %1" ).arg( m_video_file_path ) ) );
    }

    m_recording_process.setProgram( QStringLiteral( "screencapture" ) );
    m_recording_process.setArguments(
        {
            QStringLiteral( "-v" ),
            QStringLiteral( "-k" ),
            QStringLiteral( "-m" ),
            QStringLiteral( "-x" ),
            m_video_file_path
        } );
    m_recording_process.start();

    QVERIFY2(
        m_recording_process.waitForStarted( 5000 ),
        qPrintable( QStringLiteral( "Failed to start video recording: %1" ).arg( m_recording_process.errorString() ) ) );
#endif
}

void GlyphEditorTest::stopVideoRecording()
{
#ifdef Q_OS_WIN
    Q_UNUSED( m_recording_process );
    return;
#else
    if ( m_recording_process.state() == QProcess::NotRunning )
    {
        QVERIFY2(
            QFileInfo::exists( m_video_file_path ),
            qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
        return;
    }

    const qint64 pid = m_recording_process.processId();
    if ( pid > 0 )
    {
        ::kill( static_cast<pid_t>( pid ), SIGINT );
    }
    else
    {
        m_recording_process.terminate();
    }

    if ( !m_recording_process.waitForFinished( k_recording_finish_timeout_ms ) )
    {
        m_recording_process.terminate();
    }
    if ( m_recording_process.state() != QProcess::NotRunning &&
         !m_recording_process.waitForFinished( 5000 ) )
    {
        m_recording_process.kill();
        m_recording_process.waitForFinished( 5000 );
    }

    QVERIFY2(
        QFileInfo::exists( m_video_file_path ),
        qPrintable( QStringLiteral( "Recorded video was not created: %1" ).arg( m_video_file_path ) ) );
#endif
}

void GlyphEditorTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void GlyphEditorTest::bringGlyphEditorToFront( GlyphEditor* glyph_editor ) const
{
    QVERIFY2( glyph_editor != nullptr, "GlyphEditor is null" );
    glyph_editor->show();
    glyph_editor->raise();
    glyph_editor->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void GlyphEditorTest::bringVolumeTransformToFront( VolumeTransform* volume_transform ) const
{
    QVERIFY2( volume_transform != nullptr, "VolumeTransform is null" );
    volume_transform->show();
    volume_transform->raise();
    volume_transform->activateWindow();
    QVERIFY2( volume_transform->isVisible(), "VolumeTransform did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void GlyphEditorTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void GlyphEditorTest::setSpinBoxValue( QSpinBox* spin_box, int value ) const
{
    QVERIFY2( spin_box != nullptr, "Target spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QTRY_COMPARE( spin_box->value(), value );
}

void GlyphEditorTest::setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const
{
    QVERIFY2( spin_box != nullptr, "Target double spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QTRY_VERIFY( qAbs( spin_box->value() - value ) < 1e-9 );
}

void GlyphEditorTest::selectRadioButton( QRadioButton* radio_button, const char* object_name ) const
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

void GlyphEditorTest::selectComboBoxItem( QComboBox* combo_box, int index ) const
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

void GlyphEditorTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void GlyphEditorTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# GlyphEditorTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバープログラム: `" << m_server_executable << "`\n";
    stream << "- MEJボリュームデータ: `" << m_unstructured_volume_data_path << "`\n";
    stream << "- MEJ伝達関数: `" << m_transfer_function_path << "`\n";
    stream << "- TORNADOボリュームデータ: `" << m_structured_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 実行手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    stream << "\n## 自動検証項目\n\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 対象ウィジェットを objectName で取得できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Jump 実行後に m_jump_push_button が再度有効になること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 指定したスクリーンショットファイルを保存できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Markdown レポートを作成できること。\n\n";

    stream << "## 目視確認対象\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "- " << entry.caption << "\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.caption << "\n\n";
        stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
    }
}

void GlyphEditorTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

GlyphEditorTest::ClientHandles GlyphEditorTest::resolveClientHandles( MainWindow& window ) const
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
    handles.total_particles_tool_bar = window.findChild<TotalParticlesToolBar*>();
    handles.glyph_editor = window.findChild<GlyphEditor*>();
    handles.volume_transform = window.findChild<VolumeTransform*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.total_particles_tool_bar != nullptr, "TotalParticlesToolBar not found" ) ) { return handles; }
    if ( !require( handles.glyph_editor != nullptr, "GlyphEditor not found" ) ) { return handles; }
    if ( !require( handles.volume_transform != nullptr, "VolumeTransform dock not found" ) ) { return handles; }

    const auto actions = window.findChildren<QAction*>();
    for ( QAction* action : actions )
    {
        if ( action && action->text() == QStringLiteral( "Glyph Editor" ) )
        {
            handles.glyph_editor_action = action;
            break;
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
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );
    handles.focus_check_box = handles.object_editor->findChild<QCheckBox*>( "focusCheckBox" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.volume_transform_apply_button = handles.volume_transform->findChild<QPushButton*>( "applyPushButton" );

    handles.type_combo_box = handles.glyph_editor->findChild<QComboBox*>( "typeComboBox" );
    handles.scale_factor_spin_box = handles.glyph_editor->findChild<QDoubleSpinBox*>( "scaleFactorDoubleSpinBox" );
    handles.direction1_combo_box = handles.glyph_editor->findChild<QComboBox*>( "direction1ComboBox" );
    handles.direction2_combo_box = handles.glyph_editor->findChild<QComboBox*>( "direction2ComboBox" );
    handles.direction3_combo_box = handles.glyph_editor->findChild<QComboBox*>( "direction3ComboBox" );
    handles.size_constant_radio = handles.glyph_editor->findChild<QRadioButton*>( "sizeConstantRadioButton" );
    handles.size_variable_array_radio = handles.glyph_editor->findChild<QRadioButton*>( "sizeVariableArrayRadioButton" );
    handles.size_number_of_variables_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "sizeNumberOfVariablesSpinBox" );
    handles.size_variable_grid_layout = handles.glyph_editor->findChild<QGridLayout*>( "sizeVariableGridLayout" );
    handles.color_data_constant_radio = handles.glyph_editor->findChild<QRadioButton*>( "colorDataConstantRadioButton" );
    handles.color_data_variable_array_radio = handles.glyph_editor->findChild<QRadioButton*>( "colorDataVariableArrayRadioButton" );
    handles.color_data_number_of_variables_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "colorDataNumberOfVariablesSpinBox" );
    handles.color_data_variable_grid_layout = handles.glyph_editor->findChild<QGridLayout*>( "colorDataVariableGridLayout" );
    handles.uniform_radio = handles.glyph_editor->findChild<QRadioButton*>( "uniformRadioButton" );
    handles.number_of_sample_points_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "numberOfSamplePointsSpinBox" );
    handles.seed_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "seedSpinBox" );
    handles.all_points_radio = handles.glyph_editor->findChild<QRadioButton*>( "allPointsRadioButton" );
    handles.every_nth_point_radio = handles.glyph_editor->findChild<QRadioButton*>( "everyNthPointRadioButton" );
    handles.stride_spin_box = handles.glyph_editor->findChild<QSpinBox*>( "strideSpinBox" );
    handles.edit_color_map_button = handles.glyph_editor->findChild<QPushButton*>( "editColorMapPushButton" );
    handles.glyph_apply_button = handles.glyph_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.rotation_x_axis_spin_box = handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationXAxisDoubleSpinBox" );

    const auto total_particle_labels = handles.total_particles_tool_bar->findChildren<QLabel*>();
    for ( QLabel* label : total_particle_labels )
    {
        if ( label && label->text() != QStringLiteral( "Total Particles : " ) )
        {
            handles.total_particles_display_label = label;
            break;
        }
    }

    if ( !require( handles.glyph_editor_action != nullptr, "Glyph Editor action not found" ) ) { return handles; }
    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_path_line_edit != nullptr, "transferFunctionFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.is_operator_line_edit != nullptr, "isOperatorLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) { return handles; }
    if ( !require( handles.focus_check_box != nullptr, "ObjectEditor focusCheckBox not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.volume_transform_apply_button != nullptr, "VolumeTransform applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.total_particles_display_label != nullptr, "m_total_particles_display not found" ) ) { return handles; }
    if ( !require( handles.type_combo_box != nullptr, "typeComboBox not found" ) ) { return handles; }
    if ( !require( handles.scale_factor_spin_box != nullptr, "scaleFactorDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.direction1_combo_box != nullptr, "direction1ComboBox not found" ) ) { return handles; }
    if ( !require( handles.direction2_combo_box != nullptr, "direction2ComboBox not found" ) ) { return handles; }
    if ( !require( handles.direction3_combo_box != nullptr, "direction3ComboBox not found" ) ) { return handles; }
    if ( !require( handles.size_constant_radio != nullptr, "sizeConstantRadioButton not found" ) ) { return handles; }
    if ( !require( handles.size_variable_array_radio != nullptr, "sizeVariableArrayRadioButton not found" ) ) { return handles; }
    if ( !require( handles.size_number_of_variables_spin_box != nullptr, "sizeNumberOfVariablesSpinBox not found" ) ) { return handles; }
    if ( !require( handles.size_variable_grid_layout != nullptr, "sizeVariableGridLayout not found" ) ) { return handles; }
    if ( !require( handles.color_data_constant_radio != nullptr, "colorDataConstantRadioButton not found" ) ) { return handles; }
    if ( !require( handles.color_data_variable_array_radio != nullptr, "colorDataVariableArrayRadioButton not found" ) ) { return handles; }
    if ( !require( handles.color_data_number_of_variables_spin_box != nullptr, "colorDataNumberOfVariablesSpinBox not found" ) ) { return handles; }
    if ( !require( handles.color_data_variable_grid_layout != nullptr, "colorDataVariableGridLayout not found" ) ) { return handles; }
    if ( !require( handles.uniform_radio != nullptr, "uniformRadioButton not found" ) ) { return handles; }
    if ( !require( handles.number_of_sample_points_spin_box != nullptr, "numberOfSamplePointsSpinBox not found" ) ) { return handles; }
    if ( !require( handles.seed_spin_box != nullptr, "seedSpinBox not found" ) ) { return handles; }
    if ( !require( handles.all_points_radio != nullptr, "allPointsRadioButton not found" ) ) { return handles; }
    if ( !require( handles.every_nth_point_radio != nullptr, "everyNthPointRadioButton not found" ) ) { return handles; }
    if ( !require( handles.stride_spin_box != nullptr, "strideSpinBox not found" ) ) { return handles; }
    if ( !require( handles.edit_color_map_button != nullptr, "editColorMapPushButton not found" ) ) { return handles; }
    if ( !require( handles.glyph_apply_button != nullptr, "GlyphEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.rotation_x_axis_spin_box != nullptr, "rotationXAxisDoubleSpinBox not found" ) ) { return handles; }

    return handles;
}

void GlyphEditorTest::ensureConnected( const ClientHandles& client ) const
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
        logStep( QStringLiteral( "ensureConnected: already connected" ) );
        return;
    }

    for ( int attempt = 0; attempt < k_button_retry_count; ++attempt )
    {
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

        if ( waitForCondition( is_connected, k_connect_timeout_ms, 100 ) )
        {
            QTest::qWait( k_short_wait_ms );
            logStep( QStringLiteral( "ensureConnected: completed" ) );
            return;
        }

        QTest::qWait( k_button_retry_wait_ms );
    }

    QFAIL( "Client did not enter the connected state after clicking connectPushButton" );
}

void GlyphEditorTest::ensureDisconnected( const ClientHandles& client ) const
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

void GlyphEditorTest::configureRemoteVisualization( const ClientHandles& client, const QString& volume_path, const QString& transfer_function_path ) const
{
    ensureConnected( client );
    waitForOperatorPrivileges( client );

    bringWindowToFront( client.main_window );
    selectRadioButton( client.remote_viz_client_server_radio, "remoteVizClientServerRadioButton" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, volume_path );
    QTest::qWait( k_short_wait_ms );

    if ( transfer_function_path.isEmpty() )
    {
        client.transfer_function_path_line_edit->clear();
        QCOMPARE( client.transfer_function_path_line_edit->text(), QString() );
    }
    else
    {
        setLineEditText( client.transfer_function_path_line_edit, transfer_function_path );
    }
    QTest::qWait( k_short_wait_ms );

    client.object_name_line_edit->clear();
    QCOMPARE( client.object_name_line_edit->text(), QString() );

    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void GlyphEditorTest::waitForOperatorPrivileges( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_apply_button->isEnabled() &&
                       client.glyph_apply_button->isEnabled();
            },
            k_connect_timeout_ms,
            100 ),
        qPrintable(
            QStringLiteral( "Operator-capable state was not reached. isOperator='%1' objectApplyEnabled=%2 glyphApplyEnabled=%3" )
                .arg( client.is_operator_line_edit->text() )
                .arg( client.object_apply_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                .arg( client.glyph_apply_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) ) ) );
}

void GlyphEditorTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForObjectAndApply: waiting for ObjectEditor rows and nameLineEdit" ) );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_apply_button->isEnabled() &&
                       client.object_tree_view->model() != nullptr &&
                       client.object_tree_view->model()->rowCount() > 0 &&
                       client.object_tree_view->currentIndex().isValid() &&
                       !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor rows/nameLineEdit were not populated within the timeout" );

    auto* model = qobject_cast<QStandardItemModel*>( client.object_tree_view->model() );
    QVERIFY2( model != nullptr, "ObjectEditor model is not a QStandardItemModel" );

    const QModelIndex first_index = model->index( 0, 0 );
    QVERIFY2( first_index.isValid(), "ObjectEditor first row is invalid" );
    client.object_tree_view->setCurrentIndex( first_index );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        first_index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    auto* display_item = model->item( 0, 2 );
    QVERIFY2( display_item != nullptr, "ObjectEditor display item was not found" );
    if ( display_item->checkState() != Qt::Checked )
    {
        display_item->setCheckState( Qt::Checked );
    }

    if ( !client.focus_check_box->isChecked() )
    {
        client.focus_check_box->setChecked( true );
    }

    logStep(
        QStringLiteral( "waitForObjectAndApply: objectName='%1' displayChecked=%2 focusChecked=%3" )
            .arg( client.object_name_line_edit->text() )
            .arg( display_item->checkState() == Qt::Checked ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
            .arg( client.focus_check_box->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) ) );

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void GlyphEditorTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_jump_finish_wait_ms );
}

void GlyphEditorTest::waitForVisiblePointObject( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForVisiblePointObject: waiting for total particles > 0" ) );
    const bool visible = waitForCondition(
        [client]()
        {
            bool ok = false;
            const int total_particles = client.total_particles_display_label->text().toInt( &ok );
            return ok && total_particles > 0;
        },
        k_object_load_timeout_ms,
        200 );

    if ( !visible )
    {
        auto* model = qobject_cast<QStandardItemModel*>( client.object_tree_view->model() );
        const int row_count = model != nullptr ? model->rowCount() : -1;
        const QString display_state =
            ( model != nullptr && model->item( 0, 2 ) != nullptr && model->item( 0, 2 )->checkState() == Qt::Checked ) ?
                QStringLiteral( "checked" ) :
                QStringLiteral( "unchecked" );
        qInfo().noquote()
            << QStringLiteral(
                   "Visible point object was not created: totalParticles='%1' rows=%2 isOperator='%3' objectApplyEnabled=%4 display=%5 focus=%6 objectName='%7'" )
                   .arg( client.total_particles_display_label->text() )
                   .arg( row_count )
                   .arg( client.is_operator_line_edit->text() )
                   .arg( client.object_apply_button->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( display_state )
                   .arg( client.focus_check_box->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.object_name_line_edit->text() );
    }

    QVERIFY2(
        visible,
        qPrintable(
            QStringLiteral( "m_total_particles_display did not become > 0. current='%1'" )
                .arg( client.total_particles_display_label->text() ) ) );
}

void GlyphEditorTest::waitForGlyphEditorReady( const ClientHandles& client ) const
{
    const bool ready = waitForCondition(
        [client]()
        {
            return client.glyph_editor_action->isEnabled() &&
                   client.direction3_combo_box->count() > 0 &&
                   client.size_number_of_variables_spin_box->maximum() >= 3 &&
                   client.color_data_number_of_variables_spin_box->maximum() >= 3;
        },
        k_glyph_editor_ready_timeout_ms,
        100 );

    if ( !ready )
    {
        qInfo().noquote()
            << QStringLiteral(
                   "GlyphEditor ready state: actionEnabled=%1 direction3Count=%2 sizeMax=%3 colorDataMax=%4 objectName='%5'" )
                   .arg( client.glyph_editor_action->isEnabled() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) )
                   .arg( client.direction3_combo_box->count() )
                   .arg( client.size_number_of_variables_spin_box->maximum() )
                   .arg( client.color_data_number_of_variables_spin_box->maximum() )
                   .arg( client.object_name_line_edit->text() );
    }

    QVERIFY2( ready, "GlyphEditor did not become ready within the timeout" );
}

void GlyphEditorTest::openGlyphEditor( const ClientHandles& client ) const
{
    waitForGlyphEditorReady( client );
    QVERIFY2( client.glyph_editor_action != nullptr, "Glyph Editor action is null" );

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
}

void GlyphEditorTest::moveGlyphEditorRight( const ClientHandles& client ) const
{
    bringGlyphEditorToFront( client.glyph_editor );
    const QPoint current_position = client.glyph_editor->pos();
    client.glyph_editor->move( current_position + QPoint( 180, 0 ) );
    QTest::qWait( k_short_wait_ms );
}

QComboBox* GlyphEditorTest::comboBoxAtGridRow( QGridLayout* grid_layout, int row ) const
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

void GlyphEditorTest::applyGlyphEditor( const ClientHandles& client )
{
    bringGlyphEditorToFront( client.glyph_editor );
    QVERIFY2( client.glyph_apply_button->isEnabled(), "GlyphEditor applyPushButton is disabled" );
    QTest::mouseClick( client.glyph_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: applyPushButtonを押しました。" ) );
}

void GlyphEditorTest::applyVolumeTransform( const ClientHandles& client )
{
    bringVolumeTransformToFront( client.volume_transform );
    QVERIFY2( client.volume_transform_apply_button->isEnabled(), "VolumeTransform applyPushButton is disabled" );
    QTest::mouseClick( client.volume_transform_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: applyPushButtonを押しました。" ) );
}

void GlyphEditorTest::applyPresetColorMap( const ClientHandles& client, const QString& preset_name )
{
    bringGlyphEditorToFront( client.glyph_editor );

    QTimer::singleShot(
        0,
        [this, preset_name]()
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
                    5000,
                    50 ),
                "ColorMapEditor did not become visible" );

            QTest::qWait( k_short_wait_ms );

            auto* table = dialog->findChild<QTableWidget*>( "colorMapTableWidget" );
            QVERIFY2( table != nullptr, "colorMapTableWidget not found" );

            int preset_row = -1;
            int preset_col = -1;
            for ( int row = 0; row < table->rowCount(); ++row )
            {
                for ( int col = 0; col < table->columnCount(); ++col )
                {
                    QWidget* cell = table->cellWidget( row, col );
                    if ( cell == nullptr ) { continue; }

                    const auto labels = cell->findChildren<QLabel*>();
                    for ( QLabel* label : labels )
                    {
                        if ( label != nullptr && label->text() == preset_name )
                        {
                            preset_row = row;
                            preset_col = col;
                            break;
                        }
                    }

                    if ( preset_row >= 0 ) { break; }
                }
                if ( preset_row >= 0 ) { break; }
            }

            QVERIFY2(
                preset_row >= 0 && preset_col >= 0,
                qPrintable( QStringLiteral( "Color map preset was not found: %1" ).arg( preset_name ) ) );
            table->setCurrentCell( preset_row, preset_col );
            table->scrollTo( table->model()->index( preset_row, preset_col ) );
            QTest::mouseDClick(
                table->viewport(),
                Qt::LeftButton,
                Qt::NoModifier,
                table->visualRect( table->model()->index( preset_row, preset_col ) ).center() );

            const bool invoked = QMetaObject::invokeMethod(
                dialog,
                "onPresetColorMapDoubleClicked",
                Qt::DirectConnection,
                Q_ARG( int, preset_row ),
                Q_ARG( int, preset_col ) );
            QVERIFY2( invoked, "Failed to invoke ColorMapEditor::onPresetColorMapDoubleClicked" );
            QTest::qWait( k_short_wait_ms );

            QVERIFY2(
                waitForCondition(
                    [dialog]()
                    {
                        auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
                        return apply_button != nullptr && apply_button->isVisible() && apply_button->isEnabled();
                    },
                    5000,
                    50 ),
                "ColorMapEditor applyPushButton did not become ready" );

            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "ColorMapEditor applyPushButton not found" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
            QTest::qWait( 3000 );
        } );

    QTest::mouseClick( client.edit_color_map_button, Qt::LeftButton );
    markStepCompleted(
        QStringLiteral( "ColorMapEditor.ui: colorMapTableWidgetから%1をダブルクリックし、Applyボタンを押しました。" )
            .arg( preset_name ) );
}

void GlyphEditorTest::captureGlyphState( const ClientHandles& client, const QString& file_name, const QString& caption )
{
    bringWindowToFront( client.main_window );
    if ( client.glyph_editor->isVisible() )
    {
        client.glyph_editor->raise();
    }
    QTest::qWait( k_short_wait_ms );
    saveScreenshot( file_name, caption );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: %1" ).arg( caption ) );
}

QString GlyphEditorTest::comboBoxItemsText( const QComboBox* combo_box ) const
{
    if ( combo_box == nullptr ) { return QStringLiteral( "<null>" ); }

    QStringList items;
    for ( int i = 0; i < combo_box->count(); ++i )
    {
        items << QStringLiteral( "[%1]=%2" ).arg( i ).arg( combo_box->itemText( i ) );
    }

    return items.join( QStringLiteral( ", " ) );
}

void GlyphEditorTest::initTestCase()
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArray( "1200000" ) );

    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() ) );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_SERVER_EXECUTABLE", repoRootPath() ) );
    m_structured_volume_data_path = envOrDefault(
        "TORNADO_VOLUME_DATA",
        ClientTests::configuredPath( "TORNADO_VOLUME_DATA", repoRootPath() ) );
    m_unstructured_volume_data_path = envOrDefault(
        "MEJ_VOLUME_DATA",
        ClientTests::configuredPath( "MEJ_VOLUME_DATA", repoRootPath() ) );
    m_transfer_function_path = envOrDefault(
        "MEJ_TRANSFER_FUNCTION",
        ClientTests::configuredPath( "MEJ_TRANSFER_FUNCTION", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "GlyphEditorTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "GlyphEditorTest.mov" ) );
    m_test_succeeded = false;

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

}

void GlyphEditorTest::cleanupTestCase()
{
    if ( m_recording_process.state() != QProcess::NotRunning )
    {
        stopVideoRecording();
    }

    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void GlyphEditorTest::performs_glyph_editor_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    startVideoRecording();
    logStep( QStringLiteral( "scenario: recording started" ) );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();

    logStep( QStringLiteral( "scenario: MEJ dataset begin" ) );
    ensureConnected( client );
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButtonを押しました。" ) );
    configureRemoteVisualization( client, m_unstructured_volume_data_path, m_transfer_function_path );
    markStepCompleted( QStringLiteral( "Communication.ui: remoteVizClientServerRadioButtonを押し、volumeDataFilePathLineEditにMEJ_VOLUME_DATA、transferFunctionFilePathLineEditにMEJ_TRANSFER_FUNCTIONを書き込み、settingApplyPushButtonを押しました。" ) );
    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: nameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    waitForVisiblePointObject( client );

    openGlyphEditor( client );
    markStepCompleted( QStringLiteral( "GlyphEditor.uiを開きました。" ) );
    moveGlyphEditorRight( client );
    markStepCompleted( QStringLiteral( "GlyphEditor.uiを少し右にずらしてスクリーンが見えるようにしました。" ) );
    setDoubleSpinBoxValue( client.scale_factor_spin_box, 0.05 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: scaleFactorDoubleSpinBoxを0.05に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );

    bringVolumeTransformToFront( client.volume_transform );
    markStepCompleted( QStringLiteral( "VolumeTransform.uiを開きました。" ) );
    setDoubleSpinBoxValue( client.rotation_x_axis_spin_box, -90.0 );
    markStepCompleted( QStringLiteral( "VolumeTransform.ui: rotationXAxisDoubleSpinBoxに-90を入力しました。" ) );
    applyVolumeTransform( client );
    captureGlyphState( client, QStringLiteral( "01_glyph_type_arrow.png" ), QStringLiteral( "Glyph TypeをArrowに指定したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.type_combo_box, 1 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: typeComboBoxの2番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "02_glyph_type_diamond.png" ), QStringLiteral( "Glyph TypeをDiamondに指定したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.type_combo_box, 2 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: typeComboBoxの3番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "03_glyph_type_sphere.png" ), QStringLiteral( "Glyph TypeをSphereに指定したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.type_combo_box, 0 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: typeComboBoxの1番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "04_glyph_scale_factor_005.png" ), QStringLiteral( "Glyph Scale Factorを0.05に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setDoubleSpinBoxValue( client.scale_factor_spin_box, 0.1 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: scaleFactorDoubleSpinBoxを0.1に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "05_glyph_scale_factor_010.png" ), QStringLiteral( "Glyph Scale Factorを0.1に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setDoubleSpinBoxValue( client.scale_factor_spin_box, 0.2 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: scaleFactorDoubleSpinBoxを0.2に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "06_glyph_scale_factor_020.png" ), QStringLiteral( "Glyph Scale Factorを0.2に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setDoubleSpinBoxValue( client.scale_factor_spin_box, 0.05 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: scaleFactorDoubleSpinBoxを0.05に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "07_direction_q1_q2_q3.png" ), QStringLiteral( "Directionの(x,y,z)を(q1,q2,q3)に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.direction1_combo_box, 1 );
    selectComboBoxItem( client.direction2_combo_box, 0 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: direction1ComboBoxの2番目、direction2ComboBoxの1番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "08_direction_q2_q1_q3.png" ), QStringLiteral( "Directionの(x,y,z)を(q2,q1,q3)に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.direction1_combo_box, 2 );
    selectComboBoxItem( client.direction2_combo_box, 0 );
    selectComboBoxItem( client.direction3_combo_box, 1 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: direction1ComboBoxの3番目、direction2ComboBoxの1番目、direction3ComboBoxの2番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "09_direction_q3_q1_q2.png" ), QStringLiteral( "Directionの(x,y,z)を(q3,q1,q2)に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.direction2_combo_box, 1 );
    selectComboBoxItem( client.direction3_combo_box, 0 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: direction2ComboBoxの2番目、direction3ComboBoxの1番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "10_direction_q3_q2_q1.png" ), QStringLiteral( "Directionの(x,y,z)を(q3,q2,q1)に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectComboBoxItem( client.direction1_combo_box, 0 );
    selectComboBoxItem( client.direction2_combo_box, 1 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: direction1ComboBoxの1番目、direction2ComboBoxの2番目を選択しました。direction3ComboBoxはGlyphEditorの重複防止処理により3番目を維持します。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "11_size_constant.png" ), QStringLiteral( "SizeをConstantに指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectRadioButton( client.size_variable_array_radio, "sizeVariableArrayRadioButton" );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: sizeVariableArrayRadioButtonを選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "12_size_q1.png" ), QStringLiteral( "Sizeをq1に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setSpinBoxValue( client.size_number_of_variables_spin_box, 2 );
    selectComboBoxItem( comboBoxAtGridRow( client.size_variable_grid_layout, 1 ), 1 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: sizeNumberOfVariablesSpinBoxを2に設定し、sizeVariableGridLayoutの2番目のコンボボックスの2番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "13_size_q1_q2.png" ), QStringLiteral( "Sizeをq1,q2に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setSpinBoxValue( client.size_number_of_variables_spin_box, 3 );
    selectComboBoxItem( comboBoxAtGridRow( client.size_variable_grid_layout, 2 ), 2 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: sizeNumberOfVariablesSpinBoxを3に設定し、sizeVariableGridLayoutの3番目のコンボボックスの3番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "14_size_q1_q2_q3.png" ), QStringLiteral( "Sizeをq1,q2,q3に指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setSpinBoxValue( client.size_number_of_variables_spin_box, 4 );
    selectComboBoxItem( comboBoxAtGridRow( client.size_variable_grid_layout, 3 ), 3 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: sizeNumberOfVariablesSpinBoxを4に設定し、sizeVariableGridLayoutの4番目のコンボボックスの4番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "15_size_q1_q2_q3_q4.png" ), QStringLiteral( "Sizeをq1,q2,q3,q4に指定していること" ) );

    logStep( QStringLiteral( "scenario: TORNADO dataset begin" ) );
    ensureDisconnected( client );
    markStepCompleted( QStringLiteral( "Communication.ui: disconnectPushButtonを押しました。" ) );
    ensureConnected( client );
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButtonを押しました。" ) );
    configureRemoteVisualization( client, m_structured_volume_data_path, QString() );
    markStepCompleted( QStringLiteral( "Communication.ui: remoteVizClientServerRadioButtonを押し、volumeDataFilePathLineEditにTORNADO_VOLUME_DATA、transferFunctionFilePathLineEditを空欄にして、settingApplyPushButtonを押しました。" ) );
    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: nameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    waitForVisiblePointObject( client );
    captureGlyphState( client, QStringLiteral( "16_distribution_uniform.png" ), QStringLiteral( "DistributionをUniformに指定していること" ) );

    openGlyphEditor( client );
    markStepCompleted( QStringLiteral( "GlyphEditor.uiを開きました。" ) );
    moveGlyphEditorRight( client );
    markStepCompleted( QStringLiteral( "GlyphEditor.uiを少し右にずらしてスクリーンが見えるようにしました。" ) );
    selectRadioButton( client.all_points_radio, "allPointsRadioButton" );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: allPointsRadioButtonを選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "17_distribution_all_points.png" ), QStringLiteral( "DistributionをAll Pointsに指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectRadioButton( client.every_nth_point_radio, "everyNthPointRadioButton" );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: everyNthPointRadioButtonを選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "18_distribution_every_nth_point.png" ), QStringLiteral( "DistributionをEvery Nth Pointに指定していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectRadioButton( client.uniform_radio, "uniformRadioButton" );
    setSpinBoxValue( client.number_of_sample_points_spin_box, 500 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: uniformRadioButtonを選択し、numberOfSamplePointsSpinBoxを500に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "19_number_of_sample_points_500.png" ), QStringLiteral( "Number of Sample Pointsを1000から500に設定を変更していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setSpinBoxValue( client.seed_spin_box, 2 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: seedSpinBoxを2に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "20_seed_2.png" ), QStringLiteral( "Seedを1から2に設定を変更していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectRadioButton( client.every_nth_point_radio, "everyNthPointRadioButton" );
    setSpinBoxValue( client.stride_spin_box, 4 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: strideSpinBoxを4に設定しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "21_stride_4.png" ), QStringLiteral( "Strideを3から4に設定を変更していること" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectRadioButton( client.uniform_radio, "uniformRadioButton" );
    applyPresetColorMap( client, QStringLiteral( "Black-Body Radiation" ) );
    selectRadioButton( client.color_data_variable_array_radio, "colorDataVariableArrayRadioButton" );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: colorDataVariableArrayRadioButtonを押しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "22_color_map_black_body_radiation.png" ), QStringLiteral( "Color MapをBlack-Body Radiationに変更したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    applyPresetColorMap( client, QStringLiteral( "Cool to Warm" ) );
    selectRadioButton( client.color_data_constant_radio, "colorDataConstantRadioButton" );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: colorDataConstantRadioButtonを押しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "23_color_data_constant.png" ), QStringLiteral( "Color DataをConstantに設定したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    selectRadioButton( client.color_data_variable_array_radio, "colorDataVariableArrayRadioButton" );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: colorDataVariableArrayRadioButtonを押しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "24_color_q1.png" ), QStringLiteral( "Colorをq1に設定したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setSpinBoxValue( client.color_data_number_of_variables_spin_box, 2 );
    selectComboBoxItem( comboBoxAtGridRow( client.color_data_variable_grid_layout, 1 ), 1 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: colorDataNumberOfVariablesSpinBoxを2に設定し、colorDataVariableGridLayoutの2番目のコンボボックスの2番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "25_color_q1_q2.png" ), QStringLiteral( "Colorをq1,q2に設定したこと" ) );

    bringGlyphEditorToFront( client.glyph_editor );
    setSpinBoxValue( client.color_data_number_of_variables_spin_box, 3 );
    selectComboBoxItem( comboBoxAtGridRow( client.color_data_variable_grid_layout, 2 ), 2 );
    markStepCompleted( QStringLiteral( "GlyphEditor.ui: colorDataNumberOfVariablesSpinBoxを3に設定し、colorDataVariableGridLayoutの3番目のコンボボックスの3番目を選択しました。" ) );
    applyGlyphEditor( client );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );
    captureGlyphState( client, QStringLiteral( "26_color_q1_q2_q3.png" ), QStringLiteral( "Colorをq1,q2,q3に設定したこと" ) );

    m_test_succeeded = true;
    stopVideoRecording();
    logStep( QStringLiteral( "scenario: completed" ) );
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::GlyphEditorTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
