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
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTest>
#include <QTextStream>
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

namespace
{
constexpr int k_server_start_timeout_ms = 10000;
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_capture_settle_ms = 700;
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

#ifdef Q_OS_WIN
bool isWindowsDrivePathPart( const QString& part )
{
    return part.size() == 2 && part.at( 0 ).isLetter() && part.at( 1 ) == QChar( ':' );
}
#endif

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
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
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

void ObjectEditorTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void ObjectEditorTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# ObjectEditorTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- SPXポイントデータ: `" << m_point_data_path << "`\n";
    stream << "- ポリゴンデータ: `" << m_object_data_path << "`\n";
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
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 必要な UI 部品を objectName で取得できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": ファイル選択後に ObjectEditor の nameLineEdit が更新されること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Display / Keep Initial / Keep Final / Focus / Color / Opacity / Particle Limit / Coordinate の UI 値を設定できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": PlayBackControlToolBar の Jump / Next / First 操作後に対象ボタンが再度有効になること。\n";
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
    stream << "- 3D 表示内容が説明どおりかどうかは、保存したスクリーンショットを目視確認する。\n";
}

void ObjectEditorTest::addStep( const QString& description )
{
    m_steps.push_back( { description, false } );
}

void ObjectEditorTest::markStepCompleted( const QString& description )
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

QString ObjectEditorTest::serverProcessSummary()
{
    const QString stdout_text = m_server_process.isOpen()
        ? QString::fromLocal8Bit( m_server_process.readAllStandardOutput() ).trimmed()
        : QString();
    const QString stderr_text = m_server_process.isOpen()
        ? QString::fromLocal8Bit( m_server_process.readAllStandardError() ).trimmed()
        : QString();
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

void ObjectEditorTest::ensureDisconnected( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( client.connect_button->isEnabled() && !client.disconnect_button->isEnabled() )
    {
        return;
    }

    QVERIFY2( client.disconnect_button->isEnabled(), "disconnectPushButton is not enabled" );
    QTest::mouseClick( client.disconnect_button, Qt::LeftButton );
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.connect_button->isEnabled() && !client.disconnect_button->isEnabled();
            },
            k_connect_timeout_ms,
            100 ),
        "Client did not enter the disconnected state after clicking disconnectPushButton" );
    QTest::qWait( k_short_wait_ms );
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

void ObjectEditorTest::configureRemoteVisualizationWithoutVolume( const ClientHandles& client ) const
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
    setLineEditText( client.volume_data_path_line_edit, QString() );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
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

void ObjectEditorTest::clickNextAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.next_button->isEnabled();
            },
            k_jump_button_enable_timeout_ms,
            200 ),
        "m_next_push_button did not become enabled within the timeout" );

    QTest::mouseClick( client.next_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.next_button->isEnabled();
            },
            k_jump_button_enable_timeout_ms,
            200 ),
        "m_next_push_button did not become enabled again within the timeout" );

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

void ObjectEditorTest::setDisplayItemChecked( const ClientHandles& client, int row, bool checked ) const
{
    setModelCheckState( client, row, 2, checked, "ObjectEditor display item was not found" );
}

void ObjectEditorTest::setKeepInitialChecked( const ClientHandles& client, int row, bool checked ) const
{
    setModelCheckState( client, row, 3, checked, "ObjectEditor keepInitial item was not found" );
}

void ObjectEditorTest::setKeepFinalChecked( const ClientHandles& client, int row, bool checked ) const
{
    setModelCheckState( client, row, 4, checked, "ObjectEditor keepFinal item was not found" );
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

    const QStringList name_filters = dialog->nameFilters();
    for ( const QString& filter : name_filters )
    {
        if ( filter.contains( QStringLiteral( "All Files" ), Qt::CaseInsensitive ) ||
             filter.contains( QStringLiteral( "*.*" ) ) )
        {
            dialog->selectNameFilter( filter );
            break;
        }
    }
    dialog->setDirectory( file_info.absolutePath() );
    QTest::qWait( 300 );
    dialog->selectFile( file_info.absoluteFilePath() );
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

    QStringList path_parts = QDir::cleanPath( file_path ).split( '/', Qt::SkipEmptyParts );
#ifdef Q_OS_WIN
    if ( !path_parts.isEmpty() && isWindowsDrivePathPart( path_parts.first() ) )
    {
        const QString target_drive = path_parts.first().toUpper();
        logStep( QStringLiteral( "remote dialog: skipping local drive '%1'" ).arg( target_drive ) );
        path_parts.removeFirst();
    }
#endif
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
                tree_view->setCurrentIndex( index );
                tree_view->selectionModel()->setCurrentIndex(
                    index,
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
                QCoreApplication::processEvents();

                if ( is_last )
                {
                    logStep( QStringLiteral( "remote dialog: selecting file '%1'" ).arg( part ) );
                    const bool invoked = QMetaObject::invokeMethod( dialog, "onOk", Qt::DirectConnection );
                    if ( !invoked )
                    {
                        QTest::mouseClick( ok_button, Qt::LeftButton );
                    }
                    QVERIFY2(
                        waitForCondition(
                            [dialog]()
                            {
                                return !dialog->isVisible();
                            },
                            k_dialog_timeout_ms,
                            50 ),
                        "Remote file dialog did not close after selecting the file" );
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
        300,
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
    m_point_data_path = envOrDefault(
        "SPX_POINT_DATA",
        ClientTests::configuredPath( "SPX_POINT_DATA", repoRootPath() ) );
    m_object_data_path = envOrDefault(
        "CLOCK_POLYGON_DATA",
        ClientTests::configuredPath( "CLOCK_POLYGON_DATA", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "ObjectEditorTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );

    QVERIFY2(
        QFileInfo::exists( m_volume_data_path ),
        qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2(
        QFileInfo::exists( m_point_data_path ),
        qPrintable( QStringLiteral( "Point data file not found: %1" ).arg( m_point_data_path ) ) );
    QVERIFY2(
        QFileInfo::exists( m_object_data_path ),
        qPrintable( QStringLiteral( "Object data file not found: %1" ).arg( m_object_data_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QTest::qWait( 500 );
    logStep( QStringLiteral( "initTestCase: server %1" ).arg( serverProcessSummary() ) );
}

void ObjectEditorTest::cleanupTestCase()
{
    writeMarkdownReport();

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
        QStringLiteral( "CLOCK_POLYGON_DATA を ObjectEditor へ読み込んで Display ON の状態を撮影する。" ),
        [&]()
        {
            browseAndLoadObject( client, m_object_data_path, 1 );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "01_display_on.png" ),
                QStringLiteral( "Display が ON のときにオブジェクトが表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "ObjectEditor の 1 行目 displayItem を OFF にして Display OFF の状態を撮影する。" ),
        [&]()
        {
            setDisplayItemChecked( client, 0, false );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "02_display_off.png" ),
                QStringLiteral( "Display が OFF のときにオブジェクトが表示されないこと。" ) );
        } );

    run_step(
        QStringLiteral( "SPX_POINT_DATA を追加し、spx の Keep Initial ON / Keep Final OFF / Current Time Step 0 を撮影する。" ),
        [&]()
        {
            browseAndLoadObject( client, m_point_data_path, 2 );
            QVERIFY2(
                waitForCondition(
                    [client]()
                    {
                        return client.object_name_line_edit->text().contains( QStringLiteral( "spx" ), Qt::CaseInsensitive );
                    },
                    k_object_load_timeout_ms,
                    100 ),
                "ObjectEditor nameLineEdit did not contain 'spx'" );
            setDisplayItemChecked( client, 0, true );
            setKeepInitialChecked( client, 1, true );
            setKeepFinalChecked( client, 1, false );
            selectObjectRow( client, 1 );
            setCheckBoxState( client.focus_check_box, true, "focusCheckBox not found" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "03_spx_keep_initial_t0.png" ),
                QStringLiteral( "spx の Keep Initial が ON、Keep Final が OFF、Current Time Step が 0 のときに clock と spx のオブジェクトが表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "spx の Keep Initial ON / Keep Final OFF / Current Time Step 1 を撮影する。" ),
        [&]()
        {
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "04_spx_keep_initial_t1.png" ),
                QStringLiteral( "spx の Keep Initial が ON、Keep Final が OFF、Current Time Step が 1 のときに clock と spx のオブジェクトが表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "spx の Keep Initial ON / Keep Final OFF / Current Time Step 2 を撮影する。" ),
        [&]()
        {
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "05_spx_keep_initial_t2.png" ),
                QStringLiteral( "spx の Keep Initial が ON、Keep Final が OFF、Current Time Step が 2 のときに clock のオブジェクトのみ表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "spx を Keep Initial OFF / Keep Final ON に変更し、Current Time Step 0 を撮影する。" ),
        [&]()
        {
            setKeepInitialChecked( client, 1, false );
            setKeepFinalChecked( client, 1, true );
            applyObjectEditor( client );
            clickFirstAndWait( client );
            saveScreenshot(
                QStringLiteral( "06_spx_keep_final_t0.png" ),
                QStringLiteral( "spx の Keep Initial が OFF、Keep Final が ON、Current Time Step が 0 のときに clock のオブジェクトのみ表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "spx の Keep Initial OFF / Keep Final ON / Current Time Step 1 を撮影する。" ),
        [&]()
        {
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "07_spx_keep_final_t1.png" ),
                QStringLiteral( "spx の Keep Initial が OFF、Keep Final が ON、Current Time Step が 1 のときに clock と spx のオブジェクトが表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "spx の Keep Initial OFF / Keep Final ON / Current Time Step 2 を撮影する。" ),
        [&]()
        {
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "08_spx_keep_final_t2.png" ),
                QStringLiteral( "spx の Keep Initial が OFF、Keep Final が ON、Current Time Step が 2 のときに clock と spx のオブジェクトが表示されること。" ) );
        } );

    run_step(
        QStringLiteral( "オフラインのポリゴンオブジェクト色をグリーンへ変更して撮影する。" ),
        [&]()
        {
            selectObjectRow( client, 0 );
            auto color_selected = std::make_shared<bool>( false );
            QTimer::singleShot(
                0,
                qApp,
                [this, color_selected]()
                {
                    chooseColorFromDialog( QColor( Qt::green ) );
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
            applyObjectEditor( client );
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "09_offline_polygon_green.png" ),
                QStringLiteral( "オフラインでポリゴンオブジェクトの色を変更できること。" ) );
        } );

    run_step(
        QStringLiteral( "オフラインのポリゴンオブジェクト opacity を 0.1 に下げて撮影する。" ),
        [&]()
        {
            setDoubleSpinBoxValue( client.opacity_double_spin_box, 0.1, "opacityDoubleSpinBox not found" );
            applyObjectEditor( client );
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "10_offline_polygon_opacity_01.png" ),
                QStringLiteral( "オフラインでポリゴンオブジェクトの opacity を下げられること。" ) );
        } );

    run_step(
        QStringLiteral( "オフラインのポリゴンオブジェクト opacity を 1 に上げて撮影する。" ),
        [&]()
        {
            setDoubleSpinBoxValue( client.opacity_double_spin_box, 1.0, "opacityDoubleSpinBox not found" );
            applyObjectEditor( client );
            clickNextAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "11_offline_polygon_opacity_1.png" ),
                QStringLiteral( "オフラインでポリゴンオブジェクトの opacity を上げられること。" ) );
        } );

    run_step(
        QStringLiteral( "オンラインで CLOCK_POLYGON_DATA を読み込み、opacity変更前を撮影する。" ),
        [&]()
        {
            ensureConnected( client );
            configureRemoteVisualizationWithoutVolume( client );
            browseAndLoadObject( client, m_object_data_path, 1 );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "12_online_polygon_opacity_before.png" ),
                QStringLiteral( "オンラインでポリゴンオブジェクトの opacity 変更前を撮影する。" ) );
        } );

    run_step(
        QStringLiteral( "オンラインのポリゴンオブジェクト opacity を 0.1 に下げて撮影する。" ),
        [&]()
        {
            setDoubleSpinBoxValue( client.opacity_double_spin_box, 0.1, "opacityDoubleSpinBox not found" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "13_online_polygon_opacity_01.png" ),
                QStringLiteral( "オンラインでポリゴンオブジェクトの opacity を下げられること。" ) );
        } );

    run_step(
        QStringLiteral( "オンラインのポリゴンオブジェクト opacity を 1 に上げて撮影する。" ),
        [&]()
        {
            setDoubleSpinBoxValue( client.opacity_double_spin_box, 1.0, "opacityDoubleSpinBox not found" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "14_online_polygon_opacity_1.png" ),
                QStringLiteral( "オンラインでポリゴンオブジェクトの opacity を上げられること。" ) );
        } );

    run_step(
        QStringLiteral( "再接続して SPX_VOLUME_DATA を読み込み、Particle Limit 変更前を撮影する。" ),
        [&]()
        {
            ensureDisconnected( client );
            ensureConnected( client );
            configureRemoteVisualization( client );
            waitForObjectAndApply( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "15_particle_limit_before.png" ),
                QStringLiteral( "Particle Limit を変更する前のオブジェクト。" ) );
        } );

    run_step(
        QStringLiteral( "Particle Limit を 5000 に変更して撮影する。" ),
        [&]()
        {
            setSpinBoxValue( client.particle_limit_spin_box, 5000, "particleLimitSpinBox not found" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "16_particle_limit_5000.png" ),
                QStringLiteral( "Particle Limit を小さくしたオブジェクト。" ) );
        } );

    run_step(
        QStringLiteral( "Particle Limit を 100000000 に変更して撮影する。" ),
        [&]()
        {
            setSpinBoxValue( client.particle_limit_spin_box, 100000000, "particleLimitSpinBox not found" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "17_particle_limit_100000000.png" ),
                QStringLiteral( "Particle Limit を大きくしたオブジェクト。" ) );
        } );

    run_step(
        QStringLiteral( "Coordinate の X に X*2 を設定して撮影する。" ),
        [&]()
        {
            setSpinBoxValue( client.particle_limit_spin_box, 10000000, "particleLimitSpinBox not found" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            setLineEditText( client.coordinate_x_line_edit, QStringLiteral( "X*2" ) );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "18_coordinate_x_x2.png" ),
                QStringLiteral( "Coordinate の X に X*2 と設定したオブジェクト。" ) );
        } );

    run_step(
        QStringLiteral( "Coordinate の Y に Y*2 を設定して撮影する。" ),
        [&]()
        {
            setLineEditText( client.coordinate_x_line_edit, QStringLiteral( "" ) );
            setLineEditText( client.coordinate_y_line_edit, QStringLiteral( "Y*2" ) );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "19_coordinate_y_y2.png" ),
                QStringLiteral( "Coordinate の Y に Y*2 と設定したオブジェクト。" ) );
        } );

    run_step(
        QStringLiteral( "Coordinate の Z に Z*2 を設定して撮影する。" ),
        [&]()
        {
            setLineEditText( client.coordinate_y_line_edit, QStringLiteral( "" ) );
            setLineEditText( client.coordinate_z_line_edit, QStringLiteral( "Z*2" ) );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "20_coordinate_z_z2.png" ),
                QStringLiteral( "Coordinate の Z に Z*2 と設定したオブジェクト。" ) );
        } );

    run_step(
        QStringLiteral( "オフラインで browsePushButton からファイルダイアログを開けることを撮影する。" ),
        [&]()
        {
            ensureDisconnected( client );
            auto dialog_handled = std::make_shared<bool>( false );
            QTimer::singleShot(
                300,
                qApp,
                [this, dialog_handled]()
                {
                    QDialog* dialog = waitFor3dDataDialog( k_dialog_timeout_ms );
                    QVERIFY2( dialog != nullptr, "Offline 3D data selection dialog was not shown" );
                    saveScreenshot(
                        QStringLiteral( "21_offline_file_dialog.png" ),
                        QStringLiteral( "... ボタンからオフラインでのファイルダイアログを開くことができること。" ) );
                    dialog->reject();
                    *dialog_handled = true;
                } );
            QTest::mouseClick( client.browse_button, Qt::LeftButton );
            QVERIFY2(
                waitForCondition(
                    [dialog_handled]()
                    {
                        return *dialog_handled;
                    },
                    k_dialog_timeout_ms,
                    50 ),
                "Offline file dialog was not captured and closed" );
            QTest::qWait( k_short_wait_ms );
        } );

    run_step(
        QStringLiteral( "オンラインで browsePushButton からファイルダイアログを開けることを撮影し、CLOCK_POLYGON_DATA を読み込む。" ),
        [&]()
        {
            ensureConnected( client );
            configureRemoteVisualizationWithoutVolume( client );
            auto dialog_handled = std::make_shared<bool>( false );
            QTimer::singleShot(
                300,
                qApp,
                [this, dialog_handled]()
                {
                    QDialog* dialog = waitFor3dDataDialog( k_dialog_timeout_ms );
                    QVERIFY2( dialog != nullptr, "Online 3D data selection dialog was not shown" );
                    saveScreenshot(
                        QStringLiteral( "22_online_file_dialog.png" ),
                        QStringLiteral( "... ボタンからオンラインでのファイルダイアログを開くことができること。" ) );
                    if ( qobject_cast<QFileDialog*>( dialog ) != nullptr )
                    {
                        selectFileFromDialog( m_object_data_path );
                    }
                    else
                    {
                        selectFileFromRemoteDialog( dialog, m_object_data_path );
                    }
                    *dialog_handled = true;
                } );
            QTest::mouseClick( client.browse_button, Qt::LeftButton );
            QVERIFY2(
                waitForCondition(
                    [dialog_handled]()
                    {
                        return *dialog_handled;
                    },
                    k_dialog_timeout_ms,
                    50 ),
                "Online file dialog was not captured and selected" );
            QVERIFY2(
                waitForCondition(
                    [client]()
                    {
                        return client.object_tree_view->model() != nullptr &&
                               client.object_tree_view->model()->rowCount() > 0 &&
                               !client.object_name_line_edit->text().trimmed().isEmpty();
                    },
                    k_object_load_timeout_ms,
                    100 ),
                "ObjectEditor did not load CLOCK_POLYGON_DATA" );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "23_online_before_delete.png" ),
                QStringLiteral( "オンラインで Delete ボタンを押す前の状態。" ) );
        } );

    run_step(
        QStringLiteral( "オンラインで 1 行目を選択して Delete ボタンを押した後を撮影する。" ),
        [&]()
        {
            selectObjectRow( client, 0 );
            QTest::mouseClick( client.delete_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot(
                QStringLiteral( "24_online_after_delete.png" ),
                QStringLiteral( "オンラインで Delete ボタンを押した後の状態。" ) );
        } );

    run_step(
        QStringLiteral( "オフラインで CLOCK_POLYGON_DATA を読み込み、Delete ボタンを押す前を撮影する。" ),
        [&]()
        {
            ensureDisconnected( client );
            browseAndLoadObject( client, m_object_data_path, 1 );
            applyObjectEditor( client );
            clickJumpAndWaitForCompletion( client );
            saveScreenshot(
                QStringLiteral( "25_offline_before_delete.png" ),
                QStringLiteral( "オフラインで Delete ボタンを押す前の状態。" ) );
        } );

    run_step(
        QStringLiteral( "オフラインで 1 行目を選択して Delete ボタンを押した後を撮影する。" ),
        [&]()
        {
            selectObjectRow( client, 0 );
            QTest::mouseClick( client.delete_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot(
                QStringLiteral( "26_offline_after_delete.png" ),
                QStringLiteral( "オフラインで Delete ボタンを押した後の状態。" ) );
        } );

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
    ClientTests::ObjectEditorTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
