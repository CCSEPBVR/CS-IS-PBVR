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
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTest>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/GlyphEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/PlotOverLineEditor.h"
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
constexpr int k_plot_over_line_ready_timeout_ms = 120000;
constexpr int k_combo_popup_timeout_ms = 5000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_post_jump_wait_ms = 3000;
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
    return value.isEmpty() ? fallback : value;
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
    const QString stdout_text = QString::fromLocal8Bit( server_process.readAllStandardOutput() ).trimmed();
    const QString stderr_text = QString::fromLocal8Bit( server_process.readAllStandardError() ).trimmed();
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

void PlotOverLineEditorTest::startVideoRecording()
{
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
}

void PlotOverLineEditorTest::stopVideoRecording()
{
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

void PlotOverLineEditorTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForObjectAndApply: waiting for ObjectEditor nameLineEdit" ) );
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
        QStringLiteral( "/path/to/CS-IS-PBVR/Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" );

    m_client_executable = envOrDefault( "PBVR_CLIENT_EXECUTABLE", default_client_executable );
    m_server_executable = envOrDefault(
        "PBVR_SERVER_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Server/pbvr_server" ) );
    m_server_target_wrapper_executable = envOrDefault(
        "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE",
        sourceTreePath( QStringLiteral( "server_target_wrapper.sh" ) ) );
    m_structured_volume_data_path = envOrDefault(
        "PBVR_PLOT_OVER_LINE_STRUCTURED_VOLUME_DATA",
        QStringLiteral( "/path/to/SampleData/ucd/old/out/spx.pfl" ) );
    m_unstructured_volume_data_path = envOrDefault(
        "PBVR_PLOT_OVER_LINE_UNSTRUCTURED_VOLUME_DATA",
        QStringLiteral( "/path/to/reg_test_data/unstruct/mej_iofiles_downsize4_step80_90/Piece/example.pfl" ) );
    m_transfer_function_path = envOrDefault(
        "PBVR_PLOT_OVER_LINE_TRANSFER_FUNCTION",
        QStringLiteral( "/path/to/reg_test_data/unstruct/mej_v2.tfe" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "PlotOverLineEditorTest.mov" ) );

    QVERIFY2(
        QFileInfo::exists( m_client_executable ),
        qPrintable( QStringLiteral( "Client executable not found: %1" ).arg( m_client_executable ) ) );
    QVERIFY2(
        QFileInfo::exists( m_server_executable ),
        qPrintable( QStringLiteral( "Server executable not found: %1" ).arg( m_server_executable ) ) );
    QVERIFY2(
        QFileInfo::exists( m_server_target_wrapper_executable ),
        qPrintable( QStringLiteral( "Server target wrapper executable not found: %1" ).arg( m_server_target_wrapper_executable ) ) );
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

    m_server_process.setProgram( m_server_target_wrapper_executable );
    m_server_process.setArguments( { m_server_executable } );
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.setProcessChannelMode( QProcess::SeparateChannels );
    m_server_process.start();

    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
    QTest::qWait( 500 );
}

void PlotOverLineEditorTest::cleanupTestCase()
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
}

void PlotOverLineEditorTest::performs_plot_over_line_editor_scenario()
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
    main_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();

    ensureConnected( client );

    logStep( QStringLiteral( "scenario: structured dataset begin" ) );
    configureRemoteVisualization( client, m_structured_volume_data_path, QString() );
    waitForObjectAndApply( client );
    clickJumpAndWaitForCompletion( client );

    client.object_editor->close();
    QTest::qWait( k_short_wait_ms );

    openPlotOverLineEditor( client, 1 );
    configurePlotOverLine( client, 4.1, 3.5, -4.5, 10.0, 3.5, -4.5 );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: unstructured dataset begin" ) );
    prepareSecondDatasetConnection( client );
    configureRemoteVisualization( client, m_unstructured_volume_data_path, m_transfer_function_path );

    client.object_editor->show();
    bringWindowToFront( client.main_window );
    QTest::qWait( k_short_wait_ms );
    waitForObjectAndApply( client );
    clickJumpAndWaitForCompletion( client );

    client.object_editor->close();
    QTest::qWait( k_short_wait_ms );

    openGlyphEditor( client );
    configureGlyphEditor( client );
    client.glyph_editor->close();
    QTest::qWait( k_short_wait_ms );

    clickJumpAndWaitForCompletion( client );

    openVolumeTransform( client );
    configureVolumeTransform( client );
    client.volume_transform->close();
    QTest::qWait( k_short_wait_ms );

    openPlotOverLineEditor( client, 4 );
    configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7 );
    clickJumpAndWaitForCompletion( client );

    configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 1 );
    clickJumpAndWaitForCompletion( client );

    configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 2 );
    clickJumpAndWaitForCompletion( client );

    configurePlotOverLine( client, 0.0, 0.0, 0.0, 0.0, 0.0, 0.7, 3 );
    clickJumpAndWaitForCompletion( client );

    bringWindowToFront( client.main_window );
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
    ClientTests::PlotOverLineEditorTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
