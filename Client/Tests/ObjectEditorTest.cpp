#include "ObjectEditorTest.h"

#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTest>
#include <QTimer>
#include <QTreeView>
#include <QWidget>

#include <memory>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

#include <csignal>

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_dialog_timeout_ms = 15000;
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

QString ObjectEditorTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString ObjectEditorTest::repoRootPath() const
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

QString ObjectEditorTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool ObjectEditorTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void ObjectEditorTest::startVideoRecording()
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

void ObjectEditorTest::stopVideoRecording()
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

void ObjectEditorTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ObjectEditorTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
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

QString ObjectEditorTest::serverProcessSummary()
{
    const QString stdout_text = QString::fromLocal8Bit( m_server_process.readAllStandardOutput() ).trimmed();
    const QString stderr_text = QString::fromLocal8Bit( m_server_process.readAllStandardError() ).trimmed();
    return QStringLiteral( "state=%1 exitCode=%2 exitStatus=%3 stdout=\"%4\" stderr=\"%5\"" )
        .arg( static_cast<int>( m_server_process.state() ) )
        .arg( m_server_process.exitCode() )
        .arg( static_cast<int>( m_server_process.exitStatus() ) )
        .arg( stdout_text )
        .arg( stderr_text );
}

ObjectEditorTest::ClientHandles ObjectEditorTest::resolveClientHandles( MainWindow& window ) const
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
    handles.time_step_tool_bar = window.findChild<::TimeStepControlToolBar*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio = handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.browse_button = handles.object_editor->findChild<QPushButton*>( "browsePushButton" );
    handles.delete_button = handles.object_editor->findChild<QPushButton*>( "deletePushButton" );
    handles.focus_check_box = handles.object_editor->findChild<QCheckBox*>( "focusCheckBox" );
    handles.coordinate_x_line_edit = handles.object_editor->findChild<QLineEdit*>( "coordinateXLineEdit" );
    handles.coordinate_y_line_edit = handles.object_editor->findChild<QLineEdit*>( "coordinateYLineEdit" );
    handles.coordinate_z_line_edit = handles.object_editor->findChild<QLineEdit*>( "coordinateZLineEdit" );
    handles.particle_limit_spin_box = handles.object_editor->findChild<QSpinBox*>( "particleLimitSpinBox" );
    handles.opacity_double_spin_box = handles.object_editor->findChild<QDoubleSpinBox*>( "opacityDoubleSpinBox" );
    handles.color_clickable_label = handles.object_editor->findChild<QWidget*>( "colorClickableLabel" );
    handles.first_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_first_push_button" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.next_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_next_push_button" );
    handles.play_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_play_push_button" );
    handles.loop_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_loop_push_button" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );

    const QList<QSpinBox*> playback_spin_boxes = handles.time_step_tool_bar->findChildren<QSpinBox*>();
    QSpinBox* next_time_step_spin_box = nullptr;
    for ( QSpinBox* spin_box : playback_spin_boxes )
    {
        if ( spin_box == nullptr ) { continue; }
        if ( spin_box->suffix() == QStringLiteral( "ms" ) ) { continue; }

        if ( next_time_step_spin_box == nullptr )
        {
            next_time_step_spin_box = spin_box;
        }
        else if ( handles.min_limit_time_step_spin_box == nullptr )
        {
            handles.min_limit_time_step_spin_box = spin_box;
        }
        else if ( handles.max_limit_time_step_spin_box == nullptr )
        {
            handles.max_limit_time_step_spin_box = spin_box;
            break;
        }
    }

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.browse_button != nullptr, "browsePushButton not found" ) ) { return handles; }
    if ( !require( handles.delete_button != nullptr, "deletePushButton not found" ) ) { return handles; }
    if ( !require( handles.focus_check_box != nullptr, "focusCheckBox not found" ) ) { return handles; }
    if ( !require( handles.coordinate_x_line_edit != nullptr, "coordinateXLineEdit not found" ) ) { return handles; }
    if ( !require( handles.coordinate_y_line_edit != nullptr, "coordinateYLineEdit not found" ) ) { return handles; }
    if ( !require( handles.coordinate_z_line_edit != nullptr, "coordinateZLineEdit not found" ) ) { return handles; }
    if ( !require( handles.min_limit_time_step_spin_box != nullptr, "m_min_limit_time_step_spin_box not found" ) ) { return handles; }
    if ( !require( handles.max_limit_time_step_spin_box != nullptr, "m_max_limit_time_step_spin_box not found" ) ) { return handles; }
    if ( !require( handles.particle_limit_spin_box != nullptr, "particleLimitSpinBox not found" ) ) { return handles; }
    if ( !require( handles.opacity_double_spin_box != nullptr, "opacityDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.color_clickable_label != nullptr, "colorClickableLabel not found" ) ) { return handles; }
    if ( !require( handles.first_button != nullptr, "m_first_push_button not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.next_button != nullptr, "m_next_push_button not found" ) ) { return handles; }
    if ( !require( handles.play_button != nullptr, "m_play_push_button not found" ) ) { return handles; }
    if ( !require( handles.loop_button != nullptr, "m_loop_push_button not found" ) ) { return handles; }
    if ( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) { return handles; }

    return handles;
}

void ObjectEditorTest::ensureConnected( const ClientHandles& client )
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
        QStringLiteral( "ensureConnected: initial %1" )
            .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );
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
                QStringLiteral( "ensureConnected: completed %1" )
                    .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );
            return;
        }

        logStep(
            QStringLiteral( "ensureConnected: attempt %1 timed out %2" )
                .arg( attempt + 1 )
                .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );
        logStep(
            QStringLiteral( "ensureConnected: attempt %1 server-after-timeout %2" )
                .arg( attempt + 1 )
                .arg( serverProcessSummary() ) );

        QTest::qWait( k_button_retry_wait_ms );
    }

    logStep(
        QStringLiteral( "ensureConnected: failed %1" )
            .arg( connectionStateSummary( client.connect_button, client.disconnect_button, client.id_line_edit ) ) );
    logStep( QStringLiteral( "ensureConnected: failed server %1" ).arg( serverProcessSummary() ) );
    QFAIL( "Client did not enter the connected state after clicking connectPushButton" );
}

void ObjectEditorTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
        QVERIFY2(
            client.remote_viz_client_server_radio->isChecked(),
            "remoteVizClientServerRadioButton was not checked" );
    }
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, m_volume_data_path );
    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
}

void ObjectEditorTest::applyObjectEditor( const ClientHandles& client ) const
{
    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

QStandardItemModel* ObjectEditorTest::waitForObjectModel( const ClientHandles& client ) const
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

void ObjectEditorTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    logStep( QStringLiteral( "waitForObjectAndApply: waiting for ObjectEditor nameLineEdit" ) );
    QStandardItemModel* model = waitForObjectModel( client );
    QVERIFY2( model != nullptr, "Failed to resolve ObjectEditor model" );

    auto* display_item = model->item( 0, 2 );
    QVERIFY2( display_item != nullptr, "ObjectEditor display item was not found" );
    if ( display_item->checkState() != Qt::Checked )
    {
        display_item->setCheckState( Qt::Checked );
    }

    applyObjectEditor( client );
}

void ObjectEditorTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

    QTest::qWait( k_short_wait_ms );
}

void ObjectEditorTest::selectObjectRow( const ClientHandles& client, int row ) const
{
    QStandardItemModel* model = waitForObjectModel( client );
    QVERIFY2( model != nullptr, "Failed to resolve ObjectEditor model" );
    QVERIFY2( row >= 0 && row < model->rowCount(), "Requested ObjectEditor row is out of range" );

    const QModelIndex index = model->index( row, 0 );
    QVERIFY2( index.isValid(), "Requested ObjectEditor row is invalid" );

    client.object_tree_view->scrollTo( index );
    client.object_tree_view->setCurrentIndex( index );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    QTRY_COMPARE( client.object_tree_view->currentIndex().row(), row );
    QTest::qWait( k_short_wait_ms );
}

void ObjectEditorTest::setModelCheckState( const ClientHandles& client, int row, int column, bool checked, const char* item_name ) const
{
    QStandardItemModel* model = waitForObjectModel( client );
    QVERIFY2( model != nullptr, "Failed to resolve ObjectEditor model" );
    QVERIFY2( row >= 0 && row < model->rowCount(), "Requested ObjectEditor row is out of range" );

    auto* item = model->item( row, column );
    QVERIFY2( item != nullptr, item_name );

    item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
    QCOMPARE( item->checkState(), checked ? Qt::Checked : Qt::Unchecked );

    applyObjectEditor( client );
}

void ObjectEditorTest::setDisplayItemChecked( const ClientHandles& client, bool checked ) const
{
    setModelCheckState( client, 0, 2, checked, "ObjectEditor display item was not found" );
}

void ObjectEditorTest::setKeepInitialChecked( const ClientHandles& client, bool checked ) const
{
    setModelCheckState( client, 0, 3, checked, "ObjectEditor keepInitial item was not found" );
}

void ObjectEditorTest::setKeepFinalChecked( const ClientHandles& client, bool checked ) const
{
    setModelCheckState( client, 0, 4, checked, "ObjectEditor keepFinal item was not found" );
}

void ObjectEditorTest::setSpinBoxValue( QSpinBox* spin_box, int value, const char* object_name ) const
{
    QVERIFY2( spin_box != nullptr, object_name );
    spin_box->setFocus();
    spin_box->setValue( value );
    QCOMPARE( spin_box->value(), value );
}

void ObjectEditorTest::setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value, const char* object_name ) const
{
    QVERIFY2( spin_box != nullptr, object_name );
    spin_box->setFocus();
    spin_box->setValue( value );
    QCOMPARE( spin_box->value(), value );
}

void ObjectEditorTest::setCheckBoxState( QCheckBox* check_box, bool checked, const char* object_name ) const
{
    QVERIFY2( check_box != nullptr, object_name );
    if ( check_box->isChecked() == checked ) { return; }

    check_box->setFocus();
    check_box->setChecked( checked );
    QCOMPARE( check_box->isChecked(), checked );
}

QFileDialog* ObjectEditorTest::waitForFileDialog( int timeout_ms ) const
{
    QFileDialog* dialog = nullptr;
    const bool dialog_found = waitForCondition(
        [&dialog]()
        {
            const auto widgets = QApplication::topLevelWidgets();
            for ( QWidget* widget : widgets )
            {
                auto* file_dialog = qobject_cast<QFileDialog*>( widget );
                if ( file_dialog && file_dialog->isVisible() )
                {
                    dialog = file_dialog;
                    return true;
                }
            }
            return false;
        },
        timeout_ms,
        50 );

    return dialog_found ? dialog : nullptr;
}

QDialog* ObjectEditorTest::waitFor3dDataDialog( int timeout_ms ) const
{
    QDialog* dialog = nullptr;
    const bool dialog_found = waitForCondition(
        [&dialog]()
        {
            const auto widgets = QApplication::topLevelWidgets();
            for ( QWidget* widget : widgets )
            {
                auto* candidate = qobject_cast<QDialog*>( widget );
                if ( candidate == nullptr || !candidate->isVisible() ) { continue; }

                if ( qobject_cast<QFileDialog*>( candidate ) != nullptr ||
                     candidate->windowTitle() == QStringLiteral( "Select 3D data files" ) )
                {
                    dialog = candidate;
                    return true;
                }
            }
            return false;
        },
        timeout_ms,
        50 );

    return dialog_found ? dialog : nullptr;
}

static QColorDialog* waitForColorDialogImpl( int timeout_ms )
{
    QColorDialog* dialog = nullptr;
    QElapsedTimer timer;
    timer.start();

    while ( timer.elapsed() < timeout_ms )
    {
        const auto widgets = QApplication::topLevelWidgets();
        for ( QWidget* widget : widgets )
        {
            auto* color_dialog = qobject_cast<QColorDialog*>( widget );
            if ( color_dialog && color_dialog->isVisible() )
            {
                dialog = color_dialog;
                return dialog;
            }
        }
        QTest::qWait( 50 );
    }

    return dialog;
}

void ObjectEditorTest::selectFileFromDialog( const QString& file_path ) const
{
    QFileDialog* dialog = waitForFileDialog( k_dialog_timeout_ms );
    QVERIFY2( dialog != nullptr, "File dialog was not shown" );

    const QFileInfo file_info( file_path );
    QVERIFY2( file_info.exists(), qPrintable( QStringLiteral( "Target file does not exist: %1" ).arg( file_path ) ) );

    dialog->setDirectory( file_info.absolutePath() );
    QTest::qWait( 300 );
    dialog->selectFile( file_info.fileName() );
    QCoreApplication::processEvents();

    if ( auto* list_view = dialog->findChild<QListView*>( "listView" ) )
    {
        const QModelIndex root_index = list_view->rootIndex();
        const int rows = list_view->model() ? list_view->model()->rowCount( root_index ) : 0;
        for ( int row = 0; row < rows; ++row )
        {
            const QModelIndex index = list_view->model()->index( row, 0, root_index );
            if ( index.data().toString() != file_info.fileName() ) { continue; }

            list_view->selectionModel()->setCurrentIndex(
                index,
                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
            break;
        }
    }

    if ( auto* tree_view = dialog->findChild<QTreeView*>() )
    {
        const QModelIndex root_index = tree_view->rootIndex();
        const int rows = tree_view->model() ? tree_view->model()->rowCount( root_index ) : 0;
        for ( int row = 0; row < rows; ++row )
        {
            const QModelIndex index = tree_view->model()->index( row, 0, root_index );
            if ( index.data().toString() != file_info.fileName() ) { continue; }

            tree_view->selectionModel()->setCurrentIndex(
                index,
                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
            break;
        }
    }

    QMetaObject::invokeMethod( dialog, "accept", Qt::QueuedConnection );
}

void ObjectEditorTest::selectFileFromRemoteDialog( QDialog* dialog, const QString& file_path ) const
{
    QVERIFY2( dialog != nullptr, "3D data selection dialog was not shown" );

    const QFileInfo file_info( file_path );
    QVERIFY2( file_info.exists(), qPrintable( QStringLiteral( "Target file does not exist: %1" ).arg( file_path ) ) );

    auto* tree_view = dialog->findChild<QTreeView*>();
    QVERIFY2( tree_view != nullptr, "Remote file dialog tree view was not found" );

    const auto find_button = [dialog]( const QString& text ) -> QPushButton*
    {
        const auto buttons = dialog->findChildren<QPushButton*>();
        for ( QPushButton* button : buttons )
        {
            if ( button && button->text() == text ) { return button; }
        }
        return nullptr;
    };

    auto* next_button = find_button( QStringLiteral( "Next ->" ) );
    auto* ok_button = find_button( QStringLiteral( "OK" ) );
    QVERIFY2( ok_button != nullptr, "Remote file dialog OK button was not found" );

    const QStringList path_parts = QDir::cleanPath( file_path ).split( '/', Qt::SkipEmptyParts );
    QString current_path = QStringLiteral( "/" );

    for ( int part_index = 0; part_index < path_parts.size(); ++part_index )
    {
        const bool is_last = ( part_index == path_parts.size() - 1 );
        const QString& part = path_parts.at( part_index );
        bool found = false;
        logStep( QStringLiteral( "remote dialog: seeking '%1'" ).arg( part ) );

        for ( int page = 0; page < 50 && !found; ++page )
        {
            QVERIFY2(
                waitForCondition(
                    [tree_view]()
                    {
                        return tree_view->model() != nullptr && tree_view->model()->rowCount( tree_view->rootIndex() ) > 0;
                    },
                    k_dialog_timeout_ms,
                    100 ),
                "Remote file dialog did not populate the file list" );

            const QModelIndex root_index = tree_view->rootIndex();
            const int rows = tree_view->model()->rowCount( root_index );
            for ( int row = 0; row < rows; ++row )
            {
                const QModelIndex index = tree_view->model()->index( row, 0, root_index );
                if ( index.data().toString() != part ) { continue; }

                tree_view->scrollTo( index );
                tree_view->selectionModel()->setCurrentIndex(
                    index,
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                QCoreApplication::processEvents();

                if ( is_last )
                {
                    logStep( QStringLiteral( "remote dialog: selecting file '%1'" ).arg( part ) );
                    QTest::mouseClick( ok_button, Qt::LeftButton );
                }
                else
                {
                    logStep( QStringLiteral( "remote dialog: entering directory '%1'" ).arg( part ) );
                    const QString previous_path = current_path;
                    const int previous_rows = rows;
                    QMetaObject::invokeMethod(
                        tree_view,
                        "doubleClicked",
                        Qt::DirectConnection,
                        Q_ARG( QModelIndex, index ) );
                    current_path = current_path == QStringLiteral( "/" )
                        ? QStringLiteral( "/%1" ).arg( part )
                        : QStringLiteral( "%1/%2" ).arg( current_path, part );
                    const QString expected_label = QStringLiteral( "Path: %1" ).arg( current_path );
                    QVERIFY2(
                        waitForCondition(
                            [dialog, tree_view, expected_label, previous_path, previous_rows]()
                            {
                                const auto labels = dialog->findChildren<QLabel*>();
                                bool label_matches = false;
                                for ( QLabel* label : labels )
                                {
                                    if ( label && label->text() == expected_label )
                                    {
                                        label_matches = true;
                                        break;
                                    }
                                }

                                if ( !label_matches ) { return false; }

                                const auto tree_labels = dialog->findChildren<QLabel*>();
                                Q_UNUSED( tree_labels );
                                if ( tree_view->model() == nullptr ) { return false; }
                                const int new_rows = tree_view->model()->rowCount( tree_view->rootIndex() );
                                return new_rows != previous_rows || expected_label != QStringLiteral( "Path: %1" ).arg( previous_path );
                            },
                            k_dialog_timeout_ms,
                            100 ),
                        "Remote file dialog did not navigate to the expected path" );
                }

                found = true;
                break;
            }

            if ( found ) { break; }
            if ( next_button == nullptr || !next_button->isEnabled() ) { break; }

            QTest::mouseClick( next_button, Qt::LeftButton );
            QTest::qWait( 300 );
        }

        QVERIFY2( found, qPrintable( QStringLiteral( "Failed to find '%1' in remote file dialog" ).arg( part ) ) );
    }
}

void ObjectEditorTest::chooseColorFromDialog( const QColor& color ) const
{
    auto* color_dialog = waitForColorDialogImpl( k_dialog_timeout_ms );
    QVERIFY2( color_dialog != nullptr, "Color dialog was not shown" );

    color_dialog->setCurrentColor( color );
    QCOMPARE( color_dialog->currentColor(), color );

    auto* button_box = color_dialog->findChild<QDialogButtonBox*>();
    QVERIFY2( button_box != nullptr, "QColorDialog button box was not found" );
    auto* ok_button = button_box->button( QDialogButtonBox::Ok );
    QVERIFY2( ok_button != nullptr, "QColorDialog OK button was not found" );
    QTest::mouseClick( ok_button, Qt::LeftButton );
}

void ObjectEditorTest::browseAndLoadObject( const ClientHandles& client, const QString& file_path, int expected_row_count ) const
{
    bringWindowToFront( client.main_window );
    auto selection_finished = std::make_shared<bool>( false );
    QTimer::singleShot(
        0,
        qApp,
        [this, file_path, selection_finished]()
        {
            QDialog* dialog = waitFor3dDataDialog( k_dialog_timeout_ms );
            QVERIFY2( dialog != nullptr, "3D data selection dialog was not shown" );

            if ( qobject_cast<QFileDialog*>( dialog ) != nullptr )
            {
                selectFileFromDialog( file_path );
            }
            else
            {
                selectFileFromRemoteDialog( dialog, file_path );
            }

            *selection_finished = true;
        } );

    QTest::mouseClick( client.browse_button, Qt::LeftButton );
    QVERIFY2(
        waitForCondition(
            [selection_finished]()
            {
                return *selection_finished;
            },
            k_dialog_timeout_ms,
            50 ),
        "3D data selection did not complete" );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2(
        waitForCondition(
            [client, expected_row_count]()
            {
                return client.object_tree_view->model() != nullptr &&
                       client.object_tree_view->model()->rowCount() >= expected_row_count &&
                       !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor did not load the browsed object" );
    QTest::qWait( k_short_wait_ms );
    applyObjectEditor( client );
}

void ObjectEditorTest::clickFirstAndWait( const ClientHandles& client ) const
{
    QVERIFY2( client.first_button->isEnabled(), "m_first_push_button is not enabled" );
    QTest::mouseClick( client.first_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ObjectEditorTest::advanceTimeSteps( const ClientHandles& client, int repeat_count ) const
{
    QVERIFY2( client.next_button->isEnabled(), "m_next_push_button is not enabled" );

    for ( int i = 0; i < repeat_count; ++i )
    {
        QTest::mouseClick( client.next_button, Qt::LeftButton );
        QTest::qWait( k_short_wait_ms );
    }

    QTest::qWait( k_short_wait_ms );
}

void ObjectEditorTest::initTestCase()
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
        QStringLiteral( "/path/to/reg_test_data/unstruct/ucd/spx/spx.pfi" ) );
    m_object_data_path = envOrDefault(
        "PBVR_OBJECT_DATA",
        QStringLiteral( "/path/to/SampleData/stl/clock/clock_00000.stl" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "ObjectEditorTest.mov" ) );

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
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QFileInfo::exists( m_object_data_path ),
        qPrintable( QStringLiteral( "Object data file not found: %1" ).arg( m_object_data_path ) ) );
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
    logStep( QStringLiteral( "initTestCase: server %1" ).arg( serverProcessSummary() ) );
}

void ObjectEditorTest::cleanupTestCase()
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

void ObjectEditorTest::performs_object_editor_scenario()
{
    logStep( QStringLiteral( "scenario: start" ) );
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

    startVideoRecording();
    logStep( QStringLiteral( "scenario: recording started" ) );

    ensureConnected( client );
    configureRemoteVisualization( client );
    waitForObjectAndApply( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: uncheck display and re-apply" ) );
    setDisplayItemChecked( client, false );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: check display and re-apply" ) );
    setDisplayItemChecked( client, true );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: set particle limit" ) );
    setSpinBoxValue( client.particle_limit_spin_box, 1000000, "particleLimitSpinBox not found" );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: set coordinate x" ) );
    setLineEditText( client.coordinate_x_line_edit, QStringLiteral( "7" ) );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: clear coordinate x" ) );
    setLineEditText( client.coordinate_x_line_edit, QString() );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: set coordinate y" ) );
    setLineEditText( client.coordinate_y_line_edit, QStringLiteral( "3" ) );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: clear coordinate y" ) );
    setLineEditText( client.coordinate_y_line_edit, QString() );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: set coordinate z" ) );
    setLineEditText( client.coordinate_z_line_edit, QStringLiteral( "-4" ) );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: clear coordinate z" ) );
    setLineEditText( client.coordinate_z_line_edit, QString() );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: browse object file" ) );
    browseAndLoadObject( client, m_object_data_path, 2 );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: set focus on second row" ) );
    selectObjectRow( client, 1 );
    setCheckBoxState( client.focus_check_box, true, "focusCheckBox not found" );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: set time step limits" ) );
    setSpinBoxValue( client.min_limit_time_step_spin_box, 0, "m_min_limit_time_step_spin_box not found" );
    QTest::qWait( k_short_wait_ms );
    setSpinBoxValue( client.max_limit_time_step_spin_box, 11, "m_max_limit_time_step_spin_box not found" );
    QTest::qWait( k_short_wait_ms );

    logStep( QStringLiteral( "scenario: set keep initial" ) );
    setKeepInitialChecked( client, true );
    advanceTimeSteps( client, 10 );

    logStep( QStringLiteral( "scenario: set keep final" ) );
    setKeepInitialChecked( client, false );
    setKeepFinalChecked( client, true );
    clickFirstAndWait( client );
    advanceTimeSteps( client, 10 );
    clickFirstAndWait( client );

    logStep( QStringLiteral( "scenario: change second row appearance" ) );
    selectObjectRow( client, 1 );
    setDoubleSpinBoxValue( client.opacity_double_spin_box, 1.0, "opacityDoubleSpinBox not found" );
    auto color_selected = std::make_shared<bool>( false );
    QTimer::singleShot(
        0,
        qApp,
        [this, color_selected]()
        {
            chooseColorFromDialog( QColor( Qt::red ) );
            *color_selected = true;
        } );
    QMetaObject::invokeMethod( client.color_clickable_label, "doubleClicked", Qt::DirectConnection );
    QVERIFY2(
        waitForCondition(
            [color_selected]()
            {
                return *color_selected;
            },
            k_dialog_timeout_ms,
            50 ),
        "Color selection did not complete" );
    QTest::qWait( k_short_wait_ms );
    applyObjectEditor( client );
    clickJumpAndWaitForCompletion( client );

    logStep( QStringLiteral( "scenario: delete selected object" ) );
    QTest::mouseClick( client.delete_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

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
    ClientTests::ObjectEditorTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
