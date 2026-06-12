#include "TransferFunctionEditorImportExportTest.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextStream>
#include <QTest>
#include <QTimer>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
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

QString TransferFunctionEditorTest::ImportExportTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString TransferFunctionEditorTest::ImportExportTest::repoRootPath() const
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

QString TransferFunctionEditorTest::ImportExportTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool TransferFunctionEditorTest::ImportExportTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void TransferFunctionEditorTest::ImportExportTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void TransferFunctionEditorTest::ImportExportTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void TransferFunctionEditorTest::ImportExportTest::closeTransferFunctionEditor( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->close();
    QTest::qWait( k_window_settle_ms );
}

void TransferFunctionEditorTest::ImportExportTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void TransferFunctionEditorTest::ImportExportTest::saveScreenshot( const QString& file_name, const QString& caption )
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

void TransferFunctionEditorTest::ImportExportTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditor::ImportExportTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- import元: `" << m_import_tfe_path << "`\n";
    stream << "- export先: `" << m_export_tfe_path << "`\n";
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

void TransferFunctionEditorTest::ImportExportTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

QFileDialog* TransferFunctionEditorTest::ImportExportTest::waitForFileDialog( int timeout_ms ) const
{
    QFileDialog* dialog = nullptr;
    const bool dialog_found = waitForCondition(
        [&dialog]()
        {
            for ( QWidget* widget : QApplication::topLevelWidgets() )
            {
                auto* file_dialog = qobject_cast<QFileDialog*>( widget );
                if ( file_dialog != nullptr && file_dialog->isVisible() )
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

void TransferFunctionEditorTest::ImportExportTest::selectFileFromDialog( const QString& file_path, bool require_existing_file ) const
{
    QFileDialog* dialog = waitForFileDialog( k_dialog_timeout_ms );
    QVERIFY2( dialog != nullptr, "File dialog was not shown" );

    const QFileInfo file_info( file_path );
    QVERIFY2(
        QDir().mkpath( file_info.absolutePath() ),
        qPrintable( QStringLiteral( "Failed to create dialog target directory: %1" ).arg( file_info.absolutePath() ) ) );

    if ( require_existing_file )
    {
        QVERIFY2(
            file_info.exists(),
            qPrintable( QStringLiteral( "Target file does not exist: %1" ).arg( file_path ) ) );
    }

    dialog->setDirectory( file_info.absolutePath() );
    QCoreApplication::processEvents();
    QTest::qWait( 200 );

    QLineEdit* file_name_edit = dialog->findChild<QLineEdit*>( QStringLiteral( "fileNameEdit" ) );
    if ( file_name_edit == nullptr )
    {
        const auto line_edits = dialog->findChildren<QLineEdit*>();
        for ( QLineEdit* line_edit : line_edits )
        {
            if ( line_edit != nullptr && line_edit->isVisible() )
            {
                file_name_edit = line_edit;
                break;
            }
        }
    }

    QVERIFY2( file_name_edit != nullptr, "fileNameEdit was not found in QFileDialog" );
    file_name_edit->setFocus();
    file_name_edit->clear();
    QTest::keyClicks( file_name_edit, file_info.fileName() );
    QCoreApplication::processEvents();
    QCOMPARE( file_name_edit->text(), file_info.fileName() );

    QMetaObject::invokeMethod( dialog, "accept", Qt::QueuedConnection );
}

TransferFunctionEditorTest::ImportExportTest::ClientHandles
TransferFunctionEditorTest::ImportExportTest::resolveClientHandles( MainWindow& window ) const
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
    handles.tf_import_button = handles.transfer_function_editor->findChild<QPushButton*>( "importPushButton" );
    handles.tf_export_button = handles.transfer_function_editor->findChild<QPushButton*>( "exportPushButton" );
    handles.tf_apply_button = handles.transfer_function_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.tf_color_map_edit_button = handles.transfer_function_editor->findChild<QPushButton*>( "colorMapEditPushButton" );
    handles.tf_color_function_combo_box = handles.transfer_function_editor->findChild<QComboBox*>( "colorFunctionComboBox" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.tf_import_button != nullptr, "TransferFunctionEditor importPushButton not found" ) ) { return handles; }
    if ( !require( handles.tf_export_button != nullptr, "TransferFunctionEditor exportPushButton not found" ) ) { return handles; }
    if ( !require( handles.tf_apply_button != nullptr, "TransferFunctionEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.tf_color_map_edit_button != nullptr, "TransferFunctionEditor colorMapEditPushButton not found" ) ) { return handles; }
    if ( !require( handles.tf_color_function_combo_box != nullptr, "TransferFunctionEditor colorFunctionComboBox not found" ) ) { return handles; }

    return handles;
}

void TransferFunctionEditorTest::ImportExportTest::connectClient( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportExportTest::disconnectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition( [client]() { return client.disconnect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "disconnectPushButton did not become enabled within the timeout" );

    QTest::mouseClick( client.disconnect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.connect_button->isEnabled() && !client.disconnect_button->isEnabled();
            },
            k_connect_timeout_ms,
            100 ),
        "Client did not enter the disconnected state" );
}

void TransferFunctionEditorTest::ImportExportTest::configureRemoteVisualization( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportExportTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportExportTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportExportTest::importTransferFunction( const ClientHandles& client, const QString& file_path )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_import_button->isEnabled(), "importPushButton is disabled" );

    QTimer::singleShot( 0, [this, file_path]() { selectFileFromDialog( file_path, true ); } );
    QTest::mouseClick( client.tf_import_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QTest::mouseClick( client.tf_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void TransferFunctionEditorTest::ImportExportTest::exportTransferFunction( const ClientHandles& client, const QString& file_path )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_export_button->isEnabled(), "exportPushButton is disabled" );

    if ( QFileInfo::exists( file_path ) )
    {
        QVERIFY2(
            QFile::remove( file_path ),
            qPrintable( QStringLiteral( "Failed to remove existing export file: %1" ).arg( file_path ) ) );
    }

    QTimer::singleShot( 0, [this, file_path]() { selectFileFromDialog( file_path, false ); } );
    QTest::mouseClick( client.tf_export_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    QVERIFY2(
        QFileInfo::exists( file_path ),
        qPrintable( QStringLiteral( "Exported transfer function was not created: %1" ).arg( file_path ) ) );
}

void TransferFunctionEditorTest::ImportExportTest::selectThirdColorFunction( const ClientHandles& client ) const
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_color_function_combo_box != nullptr, "colorFunctionComboBox was not found" );
    QVERIFY2(
        client.tf_color_function_combo_box->count() >= 3,
        qPrintable( QStringLiteral( "colorFunctionComboBox has only %1 items" ).arg( client.tf_color_function_combo_box->count() ) ) );

    client.tf_color_function_combo_box->setFocus();
    client.tf_color_function_combo_box->setCurrentIndex( 2 );
    QCOMPARE( client.tf_color_function_combo_box->currentIndex(), 2 );
    QTest::qWait( k_short_wait_ms );
}

bool TransferFunctionEditorTest::ImportExportTest::findPresetCell( QTableWidget* table, const QString& preset_name, int* row, int* column ) const
{
    if ( table == nullptr || row == nullptr || column == nullptr ) { return false; }

    for ( int r = 0; r < table->rowCount(); ++r )
    {
        for ( int c = 0; c < table->columnCount(); ++c )
        {
            QWidget* cell = table->cellWidget( r, c );
            if ( cell == nullptr ) { continue; }

            const auto labels = cell->findChildren<QLabel*>();
            for ( QLabel* label : labels )
            {
                if ( label != nullptr && label->text() == preset_name )
                {
                    *row = r;
                    *column = c;
                    return true;
                }
            }
        }
    }

    return false;
}

void TransferFunctionEditorTest::ImportExportTest::applyCoolToWarmColorMap( const ClientHandles& client )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );

    QTimer::singleShot(
        0,
        [this]()
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

            QTest::qWait( k_short_wait_ms );

            auto* tab_widget = dialog->findChild<QTabWidget*>( "tabWidget" );
            QVERIFY2( tab_widget != nullptr, "ColorMapEditor tabWidget not found" );
            for ( int index = 0; index < tab_widget->count(); ++index )
            {
                if ( tab_widget->tabText( index ) == QStringLiteral( "Preset" ) )
                {
                    tab_widget->setCurrentIndex( index );
                    break;
                }
            }

            auto* table = dialog->findChild<QTableWidget*>( "colorMapTableWidget" );
            QVERIFY2( table != nullptr, "colorMapTableWidget not found" );

            int preset_row = -1;
            int preset_col = -1;
            QVERIFY2(
                findPresetCell( table, QStringLiteral( "Cool to Warm" ), &preset_row, &preset_col ),
                "Cool to Warm preset was not found" );

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

            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "ColorMapEditor applyPushButton not found" );
            QVERIFY2( apply_button->isEnabled(), "ColorMapEditor applyPushButton is disabled" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
        } );

    QTest::mouseClick( client.tf_color_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void TransferFunctionEditorTest::ImportExportTest::initTestCase()
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
        ClientTests::configuredPath( "GT5D_VOLUME_DATA", repoRootPath() ) );
    m_import_tfe_path = envOrDefault(
        "GT5D_TRANSFER_FUNCTION",
        ClientTests::configuredPath( "GT5D_TRANSFER_FUNCTION", repoRootPath() ) );
    m_export_tfe_path = envOrDefault(
        "GT5D_TRANSFER_FUNCTION_EXPORT_TEST",
        ClientTests::configuredPath( "GT5D_TRANSFER_FUNCTION_EXPORT_TEST", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/ImportExportTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );

    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );
    QVERIFY2( QFileInfo::exists( m_import_tfe_path ), qPrintable( QStringLiteral( "Import transfer function file not found: %1" ).arg( m_import_tfe_path ) ) );

}

void TransferFunctionEditorTest::ImportExportTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    writeMarkdownReport();
}

void TransferFunctionEditorTest::ImportExportTest::performs_import_export_scenario()
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

    bringWindowToFront( client.main_window );
    saveScreenshot( QStringLiteral( "00_object_before_import.png" ), QStringLiteral( "伝達関数をimportする前のオブジェクトの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数をimportする前のオブジェクトの状態" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( QStringLiteral( "01_tfe_before_import.png" ), QStringLiteral( "伝達関数をimportする前のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.uiを開き、import前の状態を撮影しました。" ) );
    importTransferFunction( client, m_import_tfe_path );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: importPushButtonでdemo.tfeを選択し、applyPushButtonを押しました。" ) );
    saveScreenshot( QStringLiteral( "02_tfe_after_import.png" ), QStringLiteral( "伝達関数をimportした後のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数をimportした後のTransferFunctionEditorの状態" ) );

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: import後にm_jump_push_buttonを押し、有効化を待ってから3秒待機しました。" ) );
    closeTransferFunctionEditor( client.transfer_function_editor );
    bringWindowToFront( client.main_window );
    saveScreenshot( QStringLiteral( "03_object_after_import.png" ), QStringLiteral( "伝達関数をimportした後のオブジェクトの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数をimportした後のオブジェクトの状態" ) );
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QTest::qWait( k_short_wait_ms );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QTest::qWait( k_short_wait_ms );
    selectThirdColorFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: colorFunctionComboBoxの3番目を選択し、1秒待機しました。" ) );
    applyCoolToWarmColorMap( client );
    markStepCompleted( QStringLiteral( "ColorMapEditor.ui: PresetタブでCool to Warmをダブルクリックし、applyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( QStringLiteral( "04_tfe_before_export.png" ), QStringLiteral( "伝達関数をexportする前のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: 伝達関数をexportする前のTransferFunctionEditorの状態" ) );
    QTest::qWait( k_short_wait_ms );
    exportTransferFunction( client, m_export_tfe_path );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: exportPushButtonを押し、demo_test.tfeとして保存しました。" ) );
    QTest::qWait( k_short_wait_ms );

    disconnectClient( client );
    markStepCompleted( QStringLiteral( "Communication.ui: disconnectPushButtonを押し、1秒待機しました。" ) );
    connectClient( client );
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButtonを押し、1秒待機しました。" ) );
    bringWindowToFront( client.main_window );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( QStringLiteral( "Communication.ui: settingApplyPushButtonを押し、1秒待機しました。" ) );

    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: 再接続後にnameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: 再接続後にm_jump_push_buttonを押し、有効化を待ってから3秒待機しました。" ) );

    bringWindowToFront( client.main_window );
    saveScreenshot( QStringLiteral( "05_object_before_reimport_exported.png" ), QStringLiteral( "exportした伝達関数をimportする前のオブジェクトの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: exportした伝達関数をimportする前のオブジェクトの状態" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( QStringLiteral( "06_tfe_before_reimport_exported.png" ), QStringLiteral( "exportした伝達関数をimportする前のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.uiを開き、exportした伝達関数をimportする前の状態を撮影しました。" ) );
    importTransferFunction( client, m_export_tfe_path );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: importPushButtonでdemo_test.tfeを選択し、applyPushButtonを押しました。" ) );
    selectThirdColorFunction( client );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: exportした伝達関数をimport後、colorFunctionComboBoxの3番目を選択し、1秒待機しました。" ) );
    saveScreenshot( QStringLiteral( "07_tfe_after_reimport_exported.png" ), QStringLiteral( "exportした伝達関数をimportした後のTransferFunctionEditorの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: exportした伝達関数をimportした後のTransferFunctionEditorの状態" ) );

    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: exportした伝達関数のimport後にm_jump_push_buttonを押し、有効化を待ってから3秒待機しました。" ) );
    closeTransferFunctionEditor( client.transfer_function_editor );
    bringWindowToFront( client.main_window );
    saveScreenshot( QStringLiteral( "08_object_after_reimport_exported.png" ), QStringLiteral( "exportした伝達関数をimportした後のオブジェクトの状態" ) );
    markStepCompleted( QStringLiteral( "スクリーンショットを撮影: exportした伝達関数をimportした後のオブジェクトの状態" ) );
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QTest::qWait( k_short_wait_ms );

    m_test_succeeded = true;
}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::ImportExportTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
