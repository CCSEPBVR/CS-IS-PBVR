#include "ServerTest.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSettings>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTextStream>
#include <QTest>
#include <QTreeView>
#include <QVariant>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../Widgets/ColorMapSelectorToolBar.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/RepetitionLevelControl.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "../Widgets/TransferFunctionEditor.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_disconnect_timeout_ms = 15000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_time_step_ready_timeout_ms = 120000;
constexpr int k_transfer_function_ready_timeout_ms = 30000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_after_jump_wait_ms = 3000;
constexpr int k_after_repetition_apply_wait_ms = 3000;
constexpr int k_capture_settle_ms = 300;
constexpr int k_ensight_test_particle_limit = 500000;
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

bool containsWildcard( const QString& path )
{
    return path.contains( QLatin1Char( '*' ) ) || path.contains( QLatin1Char( '?' ) );
}

bool configFlagIsEnabled( const QString& value )
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral( "1" ) ||
           normalized == QStringLiteral( "true" ) ||
           normalized == QStringLiteral( "yes" ) ||
           normalized == QStringLiteral( "on" );
}

bool serverTestUsesRemoteDataPaths( const QString& repo_root_path )
{
    QSettings settings( ClientTests::testPathConfigFilePath( repo_root_path ), QSettings::IniFormat );
    settings.beginGroup( QStringLiteral( "paths" ) );
    const QString value = settings.value( QStringLiteral( "SERVER_TEST_REMOTE_DATA" ) ).toString();
    settings.endGroup();

    return configFlagIsEnabled( value );
}

QString samplingModeName( ClientTests::ServerTest::SamplingMode sampling_mode )
{
    switch ( sampling_mode )
    {
    case ClientTests::ServerTest::SamplingMode::Uniform:
        return QStringLiteral( "Uniform" );
    case ClientTests::ServerTest::SamplingMode::Metropolis:
        return QStringLiteral( "Metropolis" );
    case ClientTests::ServerTest::SamplingMode::Rejection:
        return QStringLiteral( "Rejection" );
    }

    return QStringLiteral( "Unknown" );
}
}

namespace ClientTests
{

ServerTest::ServerTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArray( "2700000" ) );
}

QString ServerTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString ServerTest::repoRootPath() const
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

bool ServerTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

bool ServerTest::configuredPathExists( const QString& path ) const
{
    if ( path.trimmed().isEmpty() ) { return false; }

    if ( !containsWildcard( path ) )
    {
        return QFileInfo::exists( path );
    }

    const QFileInfo file_info( path );
    const QDir dir( file_info.absolutePath() );
    if ( !dir.exists() ) { return false; }

    const QStringList entries =
        dir.entryList( QStringList() << file_info.fileName(), QDir::Files | QDir::NoSymLinks );
    return !entries.isEmpty();
}

ServerTest::Dataset ServerTest::dataset( const QString& key ) const
{
    return { key, ClientTests::configuredPath( key.toUtf8().constData(), repoRootPath() ) };
}

void ServerTest::verifyDatasets() const
{
    for ( const Dataset& data : m_datasets )
    {
        QVERIFY2(
            !data.path.trimmed().isEmpty(),
            qPrintable( QStringLiteral( "TestPathConfig.ini key is empty: %1" ).arg( data.key ) ) );
        if ( m_uses_remote_data_paths ) { continue; }

        QVERIFY2(
            configuredPathExists( data.path ),
            qPrintable(
                QStringLiteral( "Configured data path was not found. key=%1 path=%2" )
                    .arg( data.key, data.path ) ) );
    }
}

void ServerTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ServerTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void ServerTest::bringRepetitionLevelControlToFront( RepetitionLevelControl* control ) const
{
    QVERIFY2( control != nullptr, "RepetitionLevelControl is null" );
    control->show();
    control->raise();
    control->activateWindow();
    QVERIFY2( control->isVisible(), "RepetitionLevelControl did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void ServerTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, text );
    QCOMPARE( line_edit->text(), text );
}

void ServerTest::setSpinBoxValue( QSpinBox* spin_box, int value, const char* widget_name ) const
{
    QVERIFY2( spin_box != nullptr, widget_name );
    QVERIFY2( spin_box->isEnabled(), qPrintable( QStringLiteral( "%1 is disabled" ).arg( widget_name ) ) );
    QVERIFY2(
        value >= spin_box->minimum() && value <= spin_box->maximum(),
        qPrintable(
            QStringLiteral( "%1 value %2 is out of range [%3, %4]" )
                .arg( widget_name )
                .arg( value )
                .arg( spin_box->minimum() )
                .arg( spin_box->maximum() ) ) );

    spin_box->setFocus();
    spin_box->setValue( value );
    QCOMPARE( spin_box->value(), value );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::selectRadioButton( QRadioButton* radio_button, const char* object_name ) const
{
    QVERIFY2( radio_button != nullptr, object_name );

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

    for ( int attempt = 0; attempt < 3; ++attempt )
    {
        QTest::mouseClick( radio_button, Qt::LeftButton );
        if ( radio_button->isChecked() ) { return; }
        QTest::qWait( 200 );
    }

    radio_button->click();
    QVERIFY2(
        radio_button->isChecked(),
        qPrintable( QStringLiteral( "%1 was not checked" ).arg( object_name ) ) );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::selectComboBoxIndex( QComboBox* combo_box, int index, const char* widget_name ) const
{
    QVERIFY2( combo_box != nullptr, widget_name );
    QVERIFY2( combo_box->isEnabled(), qPrintable( QStringLiteral( "%1 is disabled" ).arg( widget_name ) ) );
    QVERIFY2(
        index >= 0 && index < combo_box->count(),
        qPrintable(
            QStringLiteral( "%1 index %2 is out of range. item count: %3" )
                .arg( widget_name )
                .arg( index )
                .arg( combo_box->count() ) ) );

    combo_box->setCurrentIndex( index );
    QCOMPARE( combo_box->currentIndex(), index );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::saveScreenshot( const QString& case_id, const QString& file_name, const QString& caption )
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

    m_screenshots.push_back( { case_id, file_name, caption } );
}

void ServerTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# ServerTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n";
    stream << "- サーバ: 事前に起動済みのサーバへ接続する前提\n";
    stream << "- データパス: " << ( m_uses_remote_data_paths ? "リモートサーバ上のパスとして扱う" : "ローカルパスとして存在確認する" ) << "\n";
    stream << "- 画像内容の自動比較: 実施しない\n\n";

    stream << "## 実施ケース\n\n";
    stream << "| ID | 状態 | データキー | 入力パス | 説明 |\n";
    stream << "| --- | --- | --- | --- | --- |\n";
    for ( const CaseEntry& entry : m_cases )
    {
        stream << "| " << entry.case_id
               << " | " << entry.status
               << " | `" << entry.data_key << "`"
               << " | `" << entry.data_path << "`"
               << " | " << entry.description
               << " |\n";
    }

    stream << "\n## 自動判定項目\n\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 指定したUI部品を取得し、操作できること。\n";
    if ( m_uses_remote_data_paths )
    {
        stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": TestPathConfig.ini の Server Test 用パスが設定されていること。ローカル存在確認は SERVER_TEST_REMOTE_DATA によりスキップする。\n";
    }
    else
    {
        stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": TestPathConfig.ini の Server Test 用パスが存在すること。ワイルドカード付きパスは1件以上一致すること。\n";
    }
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": 各ケースで m_jump_push_button を押し、再度有効になるまで待機できること。\n";
    stream << "- " << ( m_test_succeeded ? "PASS" : "FAIL" ) << ": Markdown レポートとスクリーンショットを出力できること。\n";
    stream << "- 注意: 3D表示の見た目、粒子密度、描画差分の妥当性は目視確認する。\n\n";

    stream << "## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.case_id << " " << entry.caption << "\n\n";
        stream << "!["
               << entry.caption
               << "](./img/"
               << entry.file_name
               << ")\n\n";
    }
}

ServerTest::ClientHandles ServerTest::resolveClientHandles( MainWindow& window ) const
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
    handles.time_step_tool_bar = window.findChild<TimeStepControlToolBar*>();
    handles.color_map_selector_tool_bar = window.findChild<ColorMapSelectorToolBar*>();
    handles.transfer_function_editor = window.findChild<TransferFunctionEditor*>();
    handles.repetition_level_control = window.findChild<RepetitionLevelControl*>();

    if ( !require( handles.communication != nullptr, "Communication dock not found" ) ) { return handles; }
    if ( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) { return handles; }
    if ( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" ) ) { return handles; }
    if ( !require( handles.color_map_selector_tool_bar != nullptr, "ColorMapSelectorToolBar not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_editor != nullptr, "TransferFunctionEditor not found" ) ) { return handles; }
    if ( !require( handles.repetition_level_control != nullptr, "RepetitionLevelControl dock not found" ) ) { return handles; }

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_client_server_radio =
        handles.communication->findChild<QRadioButton*>( "remoteVizClientServerRadioButton" );
    handles.uniform_radio = handles.communication->findChild<QRadioButton*>( "uniformRadioButton" );
    handles.metropolis_radio = handles.communication->findChild<QRadioButton*>( "metropolisRadioButton" );
    handles.rejection_radio = handles.communication->findChild<QRadioButton*>( "rejectionRadioButton" );
    handles.volume_data_path_line_edit = handles.communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
    handles.transfer_function_path_line_edit =
        handles.communication->findChild<QLineEdit*>( "transferFunctionFilePathLineEdit" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );

    handles.object_name_line_edit = handles.object_editor->findChild<QLineEdit*>( "nameLineEdit" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );
    handles.focus_check_box = handles.object_editor->findChild<QCheckBox*>( "focusCheckBox" );
    handles.particle_limit_spin_box = handles.object_editor->findChild<QSpinBox*>( "particleLimitSpinBox" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );

    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.next_time_step_spin_box =
        handles.time_step_tool_bar->findChild<QSpinBox*>( "m_next_time_step_spin_box" );
    handles.selector_color_function_combo_box =
        handles.color_map_selector_tool_bar->findChild<QComboBox*>();

    handles.number_of_transfer_function_spin_box =
        handles.transfer_function_editor->findChild<QSpinBox*>( "numberOfTransferFunctionSpinBox" );
    handles.color_synthesizer_line_edit =
        handles.transfer_function_editor->findChild<QLineEdit*>( "colorSynthesizerLineEdit" );
    handles.opacity_synthesizer_line_edit =
        handles.transfer_function_editor->findChild<QLineEdit*>( "opacitySynthesizerLineEdit" );
    handles.transfer_function_apply_button =
        handles.transfer_function_editor->findChild<QPushButton*>( "applyPushButton" );

    handles.new_repetition_level_spin_box =
        handles.repetition_level_control->findChild<QSpinBox*>( "spinBoxNewRepetitionLevel" );
    handles.repetition_apply_button =
        handles.repetition_level_control->findChild<QPushButton*>( "applyPushButton" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.uniform_radio != nullptr, "uniformRadioButton not found" ) ) { return handles; }
    if ( !require( handles.metropolis_radio != nullptr, "metropolisRadioButton not found" ) ) { return handles; }
    if ( !require( handles.rejection_radio != nullptr, "rejectionRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_path_line_edit != nullptr, "transferFunctionFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) { return handles; }
    if ( !require( handles.focus_check_box != nullptr, "ObjectEditor focusCheckBox not found" ) ) { return handles; }
    if ( !require( handles.particle_limit_spin_box != nullptr, "particleLimitSpinBox not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.next_time_step_spin_box != nullptr, "m_next_time_step_spin_box not found" ) ) { return handles; }
    if ( !require( handles.selector_color_function_combo_box != nullptr, "m_color_function_combo_box not found" ) ) { return handles; }
    if ( !require( handles.number_of_transfer_function_spin_box != nullptr, "numberOfTransferFunctionSpinBox not found" ) ) { return handles; }
    if ( !require( handles.color_synthesizer_line_edit != nullptr, "colorSynthesizerLineEdit not found" ) ) { return handles; }
    if ( !require( handles.opacity_synthesizer_line_edit != nullptr, "opacitySynthesizerLineEdit not found" ) ) { return handles; }
    if ( !require( handles.transfer_function_apply_button != nullptr, "TransferFunctionEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.new_repetition_level_spin_box != nullptr, "spinBoxNewRepetitionLevel not found" ) ) { return handles; }
    if ( !require( handles.repetition_apply_button != nullptr, "RepetitionLevelControl applyPushButton not found" ) ) { return handles; }

    return handles;
}

void ServerTest::ensureDisconnected( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );

    const auto is_disconnected = [client]()
    {
        return client.connect_button->isEnabled() &&
               !client.disconnect_button->isEnabled() &&
               client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_disconnected() ) { return; }

    QVERIFY2(
        waitForCondition( [client]() { return client.disconnect_button->isEnabled(); }, k_disconnect_timeout_ms, 100 ),
        "disconnectPushButton did not become enabled within the timeout" );
    QTest::mouseClick( client.disconnect_button, Qt::LeftButton );

    QVERIFY2(
        waitForCondition( is_disconnected, k_disconnect_timeout_ms, 100 ),
        "Client did not enter the disconnected state" );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::ensureConnected( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );

    const auto is_connected = [client]()
    {
        return client.disconnect_button->isEnabled() &&
               !client.connect_button->isEnabled() &&
               !client.id_line_edit->text().trimmed().isEmpty();
    };

    if ( is_connected() ) { return; }

    QVERIFY2(
        waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "connectPushButton did not become enabled within the timeout" );
    QTest::mouseClick( client.connect_button, Qt::LeftButton );

    QVERIFY2(
        waitForCondition( is_connected, k_connect_timeout_ms, 100 ),
        "Client did not enter the connected state" );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::selectSamplingMode( const ClientHandles& client, SamplingMode sampling_mode ) const
{
    switch ( sampling_mode )
    {
    case SamplingMode::Uniform:
        selectRadioButton( client.uniform_radio, "uniformRadioButton" );
        break;
    case SamplingMode::Metropolis:
        selectRadioButton( client.metropolis_radio, "metropolisRadioButton" );
        break;
    case SamplingMode::Rejection:
        selectRadioButton( client.rejection_radio, "rejectionRadioButton" );
        break;
    }
}

void ServerTest::loadDataset(
    const ClientHandles& client,
    const Dataset& data,
    SamplingMode sampling_mode,
    const QString& transfer_function_path )
{
    if ( m_has_connected_once )
    {
        ensureDisconnected( client );
    }
    ensureConnected( client );
    m_has_connected_once = true;

    bringWindowToFront( client.main_window );
    selectRadioButton( client.remote_viz_client_server_radio, "remoteVizClientServerRadioButton" );
    selectSamplingMode( client, sampling_mode );
    setLineEditText( client.volume_data_path_line_edit, data.path );
    if ( transfer_function_path.isEmpty() )
    {
        client.transfer_function_path_line_edit->clear();
        QCOMPARE( client.transfer_function_path_line_edit->text(), QString() );
    }
    else
    {
        setLineEditText( client.transfer_function_path_line_edit, transfer_function_path );
    }

    client.object_name_line_edit->clear();
    QCOMPARE( client.object_name_line_edit->text(), QString() );

    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );

    logStep(
        QStringLiteral( "Dataset loaded: key=%1 sampling=%2 path=%3" )
            .arg( data.key, samplingModeName( sampling_mode ), data.path ) );
}

void ServerTest::waitForObjectAndApply( const ClientHandles& client, bool hide_glyph )
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_apply_button->isEnabled() &&
                       client.object_tree_view->model() != nullptr &&
                       client.object_tree_view->model()->rowCount() > 0 &&
                       !client.object_name_line_edit->text().trimmed().isEmpty();
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor rows/nameLineEdit were not populated within the timeout" );

    auto* model = qobject_cast<QStandardItemModel*>( client.object_tree_view->model() );
    QVERIFY2( model != nullptr, "ObjectEditor model is not a QStandardItemModel" );

    const auto is_glyph_row = [model]( int row )
    {
        QStandardItem* name_item = model->item( row, 0 );
        if ( name_item == nullptr ) { return false; }

        const QVariant var = name_item->data( Qt::UserRole );
        if ( !var.canConvert<ObjectInfoExtractor::ObjectInfo>() ) { return false; }

        const ObjectInfoExtractor::ObjectInfo info = var.value<ObjectInfoExtractor::ObjectInfo>();
        return info.format == ObjectInfoExtractor::Format::ServerGlyphObject;
    };

    if ( hide_glyph )
    {
        QVERIFY2(
            waitForCondition(
                [model, is_glyph_row]()
                {
                    for ( int row = 0; row < model->rowCount(); ++row )
                    {
                        if ( is_glyph_row( row ) ) { return true; }
                    }
                    return false;
                },
                k_object_load_timeout_ms,
                100 ),
            "ObjectEditor ServerGlyphObject row was not populated within the timeout" );
    }

    const QModelIndex first_index = model->index( 0, 0 );
    QVERIFY2( first_index.isValid(), "ObjectEditor first row is invalid" );
    client.object_tree_view->setCurrentIndex( first_index );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        first_index,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    QStandardItem* display_item = model->item( 0, 2 );
    if ( display_item != nullptr && display_item->checkState() != Qt::Checked )
    {
        display_item->setCheckState( Qt::Checked );
    }

    if ( hide_glyph )
    {
        for ( int row = 0; row < model->rowCount(); ++row )
        {
            if ( !is_glyph_row( row ) ) { continue; }

            QStandardItem* glyph_display_item = model->item( row, 2 );
            QVERIFY2( glyph_display_item != nullptr, "ObjectEditor glyph display item was not found" );
            glyph_display_item->setCheckState( Qt::Unchecked );
            QCOMPARE( glyph_display_item->checkState(), Qt::Unchecked );
        }
    }

    if ( !client.focus_check_box->isChecked() )
    {
        client.focus_check_box->setChecked( true );
    }

    QTest::qWait( k_short_wait_ms );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

void ServerTest::applyRepetitionLevel( const ClientHandles& client, int repetition_level ) const
{
    bringRepetitionLevelControlToFront( client.repetition_level_control );
    setSpinBoxValue( client.new_repetition_level_spin_box, repetition_level, "spinBoxNewRepetitionLevel" );
    QVERIFY2( client.repetition_apply_button->isEnabled(), "RepetitionLevelControl applyPushButton is disabled" );
    QTest::mouseClick( client.repetition_apply_button, Qt::LeftButton );
    QTest::qWait( k_after_repetition_apply_wait_ms );
}

void ServerTest::setTimeStepAndJump( const ClientHandles& client, int time_step ) const
{
    QVERIFY2(
        waitForCondition(
            [client, time_step]()
            {
                return client.next_time_step_spin_box->isEnabled() &&
                       client.next_time_step_spin_box->maximum() >= time_step;
            },
            k_time_step_ready_timeout_ms,
            100 ),
        qPrintable(
            QStringLiteral( "m_next_time_step_spin_box did not become ready for time step %1. current maximum=%2" )
                .arg( time_step )
                .arg( client.next_time_step_spin_box->maximum() ) ) );

    setSpinBoxValue( client.next_time_step_spin_box, time_step, "m_next_time_step_spin_box" );
    clickJumpAndWaitForCompletion( client );
}

void ServerTest::applyTransferFunctionSynthesizer(
    const ClientHandles& client,
    int required_transfer_function_count,
    const QString& color_synthesizer,
    const QString& opacity_synthesizer ) const
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2(
        waitForCondition(
            [client, required_transfer_function_count]()
            {
                return client.number_of_transfer_function_spin_box->isEnabled() &&
                       client.number_of_transfer_function_spin_box->value() >= required_transfer_function_count;
            },
            k_transfer_function_ready_timeout_ms,
            100 ),
        qPrintable(
            QStringLiteral(
                "numberOfTransferFunctionSpinBox is smaller than required. current=%1 required=%2" )
                .arg( client.number_of_transfer_function_spin_box->value() )
                .arg( required_transfer_function_count ) ) );

    setLineEditText( client.color_synthesizer_line_edit, color_synthesizer );
    setLineEditText( client.opacity_synthesizer_line_edit, opacity_synthesizer );
    QVERIFY2( client.transfer_function_apply_button->isEnabled(), "TransferFunctionEditor applyPushButton is disabled" );
    QTest::mouseClick( client.transfer_function_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    client.transfer_function_editor->hide();
}

void ServerTest::selectColorFunction( const ClientHandles& client, int one_based_index ) const
{
    selectComboBoxIndex(
        client.selector_color_function_combo_box,
        one_based_index - 1,
        "m_color_function_combo_box" );
}

void ServerTest::setParticleLimitAndApply( const ClientHandles& client, int particle_limit ) const
{
    setSpinBoxValue( client.particle_limit_spin_box, particle_limit, "particleLimitSpinBox" );
    QVERIFY2( client.object_apply_button->isEnabled(), "ObjectEditor applyPushButton is disabled" );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ServerTest::runCase(
    const QString& case_id,
    const QString& description,
    const QString& data_key,
    const QString& data_path,
    const std::function<void()>& body )
{
    m_cases.push_back( { case_id, description, data_key, data_path, QStringLiteral( "未実行" ) } );
    CaseEntry& entry = m_cases.back();

    logStep( QStringLiteral( "%1: %2" ).arg( case_id, description ) );
    body();

    entry.status = QStringLiteral( "PASS" );
}

void ServerTest::captureCase(
    const QString& case_id,
    const QString& caption,
    int repetition_level,
    const ClientHandles* client )
{
    if ( repetition_level > 0 )
    {
        QVERIFY2( client != nullptr, "Client handles are required to apply Repetition Level" );
        applyRepetitionLevel( *client, repetition_level );
    }

    if ( client != nullptr )
    {
        bringWindowToFront( client->main_window );
    }

    const QString file_name = QStringLiteral( "%1.png" ).arg( case_id );
    saveScreenshot( case_id, file_name, caption );
}

void ServerTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_output_dir_path = QDir( repoRootPath() ).absoluteFilePath(
        QStringLiteral( "Client/output-tests/%1/ServerTest" ).arg( date_stamp ) );
    m_screenshot_dir_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;
    m_has_connected_once = false;
    m_uses_remote_data_paths = serverTestUsesRemoteDataPaths( repoRootPath() );
    m_mej_transfer_function_path = envOrDefault( "MEJ_TRANSFER_FUNCTION", QString() );

    m_datasets = {
        dataset( QStringLiteral( "ASCII_PFI_STRUCTURED_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "BINARY_PFI_STRUCTURED_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "ASCII_PFI_UNSTRUCTURED_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "BINARY_PFI_UNSTRUCTURED_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "VTI_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "TETRA_AND_HEX_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "UNSTRUCTURED_MULTI_BLOCK_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "PVTU_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "ENSIGHT_GOLD_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "LARGE_VTI_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "LARGE_TETRA_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "LARGE_HEXA_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "LARGE_PRISM_VOLUME_DATA" ) ),
        dataset( QStringLiteral( "LARGE_PYRAMID_VOLUME_DATA" ) ),
    };
    verifyDatasets();
    QVERIFY2(
        !m_mej_transfer_function_path.trimmed().isEmpty(),
        "TestPathConfig.ini key is empty: MEJ_TRANSFER_FUNCTION" );
    if ( !m_uses_remote_data_paths )
    {
        QVERIFY2(
            configuredPathExists( m_mej_transfer_function_path ),
            qPrintable(
                QStringLiteral( "Configured transfer function path was not found. key=MEJ_TRANSFER_FUNCTION path=%1" )
                    .arg( m_mej_transfer_function_path ) ) );
    }

    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
}

void ServerTest::cleanupTestCase()
{
    writeMarkdownReport();
}

void ServerTest::performs_server_scenario()
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

    ensureDisconnected( client );

    const Dataset ascii_structured = dataset( QStringLiteral( "ASCII_PFI_STRUCTURED_VOLUME_DATA" ) );
    const Dataset binary_structured = dataset( QStringLiteral( "BINARY_PFI_STRUCTURED_VOLUME_DATA" ) );
    const Dataset ascii_unstructured = dataset( QStringLiteral( "ASCII_PFI_UNSTRUCTURED_VOLUME_DATA" ) );
    const Dataset binary_unstructured = dataset( QStringLiteral( "BINARY_PFI_UNSTRUCTURED_VOLUME_DATA" ) );
    const Dataset vti = dataset( QStringLiteral( "VTI_VOLUME_DATA" ) );
    const Dataset tetra_and_hex = dataset( QStringLiteral( "TETRA_AND_HEX_VOLUME_DATA" ) );
    const Dataset multiblock = dataset( QStringLiteral( "UNSTRUCTURED_MULTI_BLOCK_VOLUME_DATA" ) );
    const Dataset pvtu = dataset( QStringLiteral( "PVTU_VOLUME_DATA" ) );
    const Dataset ensight = dataset( QStringLiteral( "ENSIGHT_GOLD_VOLUME_DATA" ) );
    const Dataset large_vti = dataset( QStringLiteral( "LARGE_VTI_VOLUME_DATA" ) );
    const Dataset large_tetra = dataset( QStringLiteral( "LARGE_TETRA_VOLUME_DATA" ) );
    const Dataset large_hexa = dataset( QStringLiteral( "LARGE_HEXA_VOLUME_DATA" ) );
    const Dataset large_prism = dataset( QStringLiteral( "LARGE_PRISM_VOLUME_DATA" ) );
    const Dataset large_pyramid = dataset( QStringLiteral( "LARGE_PYRAMID_VOLUME_DATA" ) );

    auto load_apply_jump_capture =
        [this, &client](
            const QString& case_id,
            const Dataset& data,
            SamplingMode sampling,
            const QString& caption,
            const CaseOptions& options )
        {
            loadDataset( client, data, sampling, options.transfer_function_path );
            waitForObjectAndApply( client, options.hide_glyph );
            clickJumpAndWaitForCompletion( client );
            captureCase( case_id, caption, options.repetition_level, &client );
        };

    runCase(
        QStringLiteral( "01_ascii_pfi_structured" ),
        QStringLiteral( "ASCII PFI structured volume dataをUniformで表示する。" ),
        ascii_structured.key,
        ascii_structured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "01_ascii_pfi_structured" ), ascii_structured, SamplingMode::Uniform, QStringLiteral( "ASCII pfi file Structured Volume Data" ), CaseOptions() ); } );

    runCase(
        QStringLiteral( "02_binary_pfi_structured" ),
        QStringLiteral( "Binary PFI structured volume dataをUniformで表示する。" ),
        binary_structured.key,
        binary_structured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "02_binary_pfi_structured" ), binary_structured, SamplingMode::Uniform, QStringLiteral( "Binary pfi file Structured Volume Data" ), CaseOptions() ); } );

    runCase(
        QStringLiteral( "03_ascii_pfl_unstructured" ),
        QStringLiteral( "ASCII PFL unstructured volume dataをUniformで表示する。" ),
        ascii_unstructured.key,
        ascii_unstructured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "03_ascii_pfl_unstructured" ), ascii_unstructured, SamplingMode::Uniform, QStringLiteral( "ASCII pfl file Unstructured Volume Data" ), CaseOptions( -1, true, m_mej_transfer_function_path ) ); } );

    runCase(
        QStringLiteral( "04_binary_pfl_unstructured" ),
        QStringLiteral( "Binary PFL unstructured volume dataをUniformで表示する。" ),
        binary_unstructured.key,
        binary_unstructured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "04_binary_pfl_unstructured" ), binary_unstructured, SamplingMode::Uniform, QStringLiteral( "Binary pfl file Unstructured Volume Data" ), CaseOptions( -1, true, m_mej_transfer_function_path ) ); } );

    runCase(
        QStringLiteral( "05_uniform_structured" ),
        QStringLiteral( "ASCII PFI structured volume dataをUniformで表示する。" ),
        ascii_structured.key,
        ascii_structured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "05_uniform_structured" ), ascii_structured, SamplingMode::Uniform, QStringLiteral( "Uniform Structured Volume Data" ), CaseOptions() ); } );

    runCase(
        QStringLiteral( "06_metropolis_structured" ),
        QStringLiteral( "ASCII PFI structured volume dataをMetropolisで表示する。" ),
        ascii_structured.key,
        ascii_structured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "06_metropolis_structured" ), ascii_structured, SamplingMode::Metropolis, QStringLiteral( "Metropolis Structured Volume Data" ), CaseOptions() ); } );

    runCase(
        QStringLiteral( "07_rejection_structured" ),
        QStringLiteral( "ASCII PFI structured volume dataをRejectionで表示する。" ),
        ascii_structured.key,
        ascii_structured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "07_rejection_structured" ), ascii_structured, SamplingMode::Rejection, QStringLiteral( "Rejection Structured Volume Data" ), CaseOptions() ); } );

    runCase(
        QStringLiteral( "08_uniform_unstructured" ),
        QStringLiteral( "ASCII PFL unstructured volume dataをUniformで表示する。" ),
        ascii_unstructured.key,
        ascii_unstructured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "08_uniform_unstructured" ), ascii_unstructured, SamplingMode::Uniform, QStringLiteral( "Uniform Unstructured Volume Data" ), CaseOptions( -1, true, m_mej_transfer_function_path ) ); } );

    runCase(
        QStringLiteral( "09_metropolis_unstructured" ),
        QStringLiteral( "ASCII PFL unstructured volume dataをMetropolisで表示する。" ),
        ascii_unstructured.key,
        ascii_unstructured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "09_metropolis_unstructured" ), ascii_unstructured, SamplingMode::Metropolis, QStringLiteral( "Metropolis Unstructured Volume Data" ), CaseOptions( -1, true, m_mej_transfer_function_path ) ); } );

    runCase(
        QStringLiteral( "10_rejection_unstructured" ),
        QStringLiteral( "ASCII PFL unstructured volume dataをRejectionで表示する。" ),
        ascii_unstructured.key,
        ascii_unstructured.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "10_rejection_unstructured" ), ascii_unstructured, SamplingMode::Rejection, QStringLiteral( "Rejection Unstructured Volume Data" ), CaseOptions( -1, true, m_mej_transfer_function_path ) ); } );

    runCase(
        QStringLiteral( "11_vti_t0_q1_rl32" ),
        QStringLiteral( "VTI volume dataのtime step 0、q1、Repetition Level 32を表示する。" ),
        vti.key,
        vti.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "11_vti_t0_q1_rl32" ), vti, SamplingMode::Uniform, QStringLiteral( "Vti/union_*.vti, time step:0, q1, Repetition Level:32" ), CaseOptions( 32 ) ); } );

    runCase(
        QStringLiteral( "12_tetra_hex_t0_q1_rl32" ),
        QStringLiteral( "TetraAndHexのtime step 0、q1、Repetition Level 32を表示する。" ),
        tetra_and_hex.key,
        tetra_and_hex.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "12_tetra_hex_t0_q1_rl32" ), tetra_and_hex, SamplingMode::Uniform, QStringLiteral( "TetraAndHex/tetra_and_hex_*.vtu, time step:0, q1, Repetition Level:32" ), CaseOptions( 32 ) ); } );

    runCase(
        QStringLiteral( "13_tetra_hex_t3_q1_rl32" ),
        QStringLiteral( "TetraAndHexのtime step 3、q1、Repetition Level 32を表示する。" ),
        tetra_and_hex.key,
        tetra_and_hex.path,
        [&]()
        {
            setTimeStepAndJump( client, 3 );
            captureCase( QStringLiteral( "13_tetra_hex_t3_q1_rl32" ), QStringLiteral( "TetraAndHex/tetra_and_hex_*.vtu, time step:3, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "14_tetra_hex_t0_q2_rl32" ),
        QStringLiteral( "TetraAndHexのtime step 0、q2、Repetition Level 32を表示する。" ),
        tetra_and_hex.key,
        tetra_and_hex.path,
        [&]()
        {
            applyTransferFunctionSynthesizer( client, 2, QStringLiteral( "C2" ), QStringLiteral( "O2" ) );
            setTimeStepAndJump( client, 0 );
            selectColorFunction( client, 2 );
            captureCase( QStringLiteral( "14_tetra_hex_t0_q2_rl32" ), QStringLiteral( "TetraAndHex/tetra_and_hex_*.vtu, time step:0, q2, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "15_tetra_hex_t3_q2_rl32" ),
        QStringLiteral( "TetraAndHexのtime step 3、q2、Repetition Level 32を表示する。" ),
        tetra_and_hex.key,
        tetra_and_hex.path,
        [&]()
        {
            setTimeStepAndJump( client, 3 );
            captureCase( QStringLiteral( "15_tetra_hex_t3_q2_rl32" ), QStringLiteral( "TetraAndHex/tetra_and_hex_*.vtu, time step:3, q2, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "16_multiblock_t0_q1_rl32" ),
        QStringLiteral( "UnstructuredMultiBlockのtime step 0、q1、Repetition Level 32を表示する。" ),
        multiblock.key,
        multiblock.path,
        [&]()
        {
            loadDataset( client, multiblock, SamplingMode::Uniform );
            waitForObjectAndApply( client );
            clickJumpAndWaitForCompletion( client );
            selectColorFunction( client, 1 );
            captureCase( QStringLiteral( "16_multiblock_t0_q1_rl32" ), QStringLiteral( "UnstructuredMultiBlock/multiblock_*.vtm, time step:0, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "17_multiblock_t8_q1_rl32" ),
        QStringLiteral( "UnstructuredMultiBlockのtime step 8、q1、Repetition Level 32を表示する。" ),
        multiblock.key,
        multiblock.path,
        [&]()
        {
            setTimeStepAndJump( client, 8 );
            captureCase( QStringLiteral( "17_multiblock_t8_q1_rl32" ), QStringLiteral( "UnstructuredMultiBlock/multiblock_*.vtm, time step:8, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "18_multiblock_t0_q2_rl32" ),
        QStringLiteral( "UnstructuredMultiBlockのtime step 0、q2、Repetition Level 32を表示する。" ),
        multiblock.key,
        multiblock.path,
        [&]()
        {
            applyTransferFunctionSynthesizer( client, 2, QStringLiteral( "C2" ), QStringLiteral( "O2" ) );
            setTimeStepAndJump( client, 0 );
            selectColorFunction( client, 2 );
            captureCase( QStringLiteral( "18_multiblock_t0_q2_rl32" ), QStringLiteral( "UnstructuredMultiBlock/multiblock_*.vtm, time step:0, q2, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "19_multiblock_t8_q2_rl32" ),
        QStringLiteral( "UnstructuredMultiBlockのtime step 8、q2、Repetition Level 32を表示する。" ),
        multiblock.key,
        multiblock.path,
        [&]()
        {
            setTimeStepAndJump( client, 8 );
            captureCase( QStringLiteral( "19_multiblock_t8_q2_rl32" ), QStringLiteral( "UnstructuredMultiBlock/multiblock_*.vtm, time step:8, q2, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "20_pvtu_t0_q1_rl32" ),
        QStringLiteral( "PVTUのtime step 0、q1、Repetition Level 32を表示する。" ),
        pvtu.key,
        pvtu.path,
        [&]()
        {
            loadDataset( client, pvtu, SamplingMode::Uniform );
            waitForObjectAndApply( client );
            clickJumpAndWaitForCompletion( client );
            selectColorFunction( client, 1 );
            captureCase( QStringLiteral( "20_pvtu_t0_q1_rl32" ), QStringLiteral( "Pvtu/example_*.pvtu, time step:0, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "21_pvtu_t5_q1_rl32" ),
        QStringLiteral( "PVTUのtime step 5、q1、Repetition Level 32を表示する。" ),
        pvtu.key,
        pvtu.path,
        [&]()
        {
            setTimeStepAndJump( client, 5 );
            captureCase( QStringLiteral( "21_pvtu_t5_q1_rl32" ), QStringLiteral( "Pvtu/example_*.pvtu, time step:5, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "22_pvtu_t0_q2_rl32" ),
        QStringLiteral( "PVTUのtime step 0、q2、Repetition Level 32を表示する。" ),
        pvtu.key,
        pvtu.path,
        [&]()
        {
            applyTransferFunctionSynthesizer( client, 2, QStringLiteral( "C2" ), QStringLiteral( "O2" ) );
            setTimeStepAndJump( client, 0 );
            selectColorFunction( client, 2 );
            captureCase( QStringLiteral( "22_pvtu_t0_q2_rl32" ), QStringLiteral( "Pvtu/example_*.pvtu, time step:0, q2, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "23_pvtu_t5_q2_rl32" ),
        QStringLiteral( "PVTUのtime step 5、q2、Repetition Level 32を表示する。" ),
        pvtu.key,
        pvtu.path,
        [&]()
        {
            setTimeStepAndJump( client, 5 );
            captureCase( QStringLiteral( "23_pvtu_t5_q2_rl32" ), QStringLiteral( "Pvtu/example_*.pvtu, time step:5, q2, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "24_ensight_t7_q6_rl4" ),
        QStringLiteral( "EnSight Goldのtime step 7、q6、Repetition Level 4を表示する。" ),
        ensight.key,
        ensight.path,
        [&]()
        {
            loadDataset( client, ensight, SamplingMode::Uniform );
            waitForObjectAndApply( client, true );
            clickJumpAndWaitForCompletion( client );
            applyTransferFunctionSynthesizer( client, 6, QStringLiteral( "C6" ), QStringLiteral( "O6" ) );
            setTimeStepAndJump( client, 7 );
            selectColorFunction( client, 6 );
            captureCase( QStringLiteral( "24_ensight_t7_q6_rl4" ), QStringLiteral( "depthCharge3D_EnsightGold/depthCharge3D.case, time step:7, q6, Repetition Level:4" ), 4, &client );
        } );

    runCase(
        QStringLiteral( "25_ensight_t9_q6_rl4" ),
        QStringLiteral( "EnSight Goldのtime step 9、q6、Repetition Level 4、Particle Limit 500000を表示する。" ),
        ensight.key,
        ensight.path,
        [&]()
        {
            loadDataset( client, ensight, SamplingMode::Uniform );
            waitForObjectAndApply( client, true );
            setParticleLimitAndApply( client, k_ensight_test_particle_limit );
            clickJumpAndWaitForCompletion( client );
            applyTransferFunctionSynthesizer( client, 6, QStringLiteral( "C6" ), QStringLiteral( "O6" ) );
            setTimeStepAndJump( client, 9 );
            selectColorFunction( client, 6 );
            captureCase( QStringLiteral( "25_ensight_t9_q6_rl4" ), QStringLiteral( "depthCharge3D_EnsightGold/depthCharge3D.case, time step:9, q6, Repetition Level:4, Particle Limit:500000" ), 4, &client );
        } );

    runCase(
        QStringLiteral( "26_large_vti_t0_q1_rl32" ),
        QStringLiteral( "LargeVtiのtime step 0、q1、Repetition Level 32を表示する。" ),
        large_vti.key,
        large_vti.path,
        [&]()
        {
            loadDataset( client, large_vti, SamplingMode::Uniform );
            waitForObjectAndApply( client );
            setParticleLimitAndApply( client, 1000000 );
            clickJumpAndWaitForCompletion( client );
            captureCase( QStringLiteral( "26_large_vti_t0_q1_rl32" ), QStringLiteral( "Large/LargeVti/union.*.vtm, time step:0, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "27_large_tetra_t0_q1_rl32" ),
        QStringLiteral( "LargeTetraのtime step 0、q1、Repetition Level 32を表示する。" ),
        large_tetra.key,
        large_tetra.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "27_large_tetra_t0_q1_rl32" ), large_tetra, SamplingMode::Uniform, QStringLiteral( "Large/LargeTetra/large_tetra.*.vtm, time step:0, q1, Repetition Level:32" ), CaseOptions( 32 ) ); } );

    runCase(
        QStringLiteral( "28_large_tetra_t7_q1_rl32" ),
        QStringLiteral( "LargeTetraのtime step 7、q1、Repetition Level 32を表示する。" ),
        large_tetra.key,
        large_tetra.path,
        [&]()
        {
            setTimeStepAndJump( client, 7 );
            captureCase( QStringLiteral( "28_large_tetra_t7_q1_rl32" ), QStringLiteral( "Large/LargeTetra/large_tetra.*.vtm, time step:7, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "29_large_hexa_t0_q1_rl32" ),
        QStringLiteral( "LargeHexaのtime step 0、q1、Repetition Level 32を表示する。" ),
        large_hexa.key,
        large_hexa.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "29_large_hexa_t0_q1_rl32" ), large_hexa, SamplingMode::Uniform, QStringLiteral( "Large/LargeHexa/large_hexa.*.vtm, time step:0, q1, Repetition Level:32" ), CaseOptions( 32 ) ); } );

    runCase(
        QStringLiteral( "30_large_hexa_t7_q1_rl32" ),
        QStringLiteral( "LargeHexaのtime step 7、q1、Repetition Level 32を表示する。" ),
        large_hexa.key,
        large_hexa.path,
        [&]()
        {
            setTimeStepAndJump( client, 7 );
            captureCase( QStringLiteral( "30_large_hexa_t7_q1_rl32" ), QStringLiteral( "Large/LargeHexa/large_hexa.*.vtm, time step:7, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "31_large_prism_t0_q1_rl32" ),
        QStringLiteral( "LargePrismのtime step 0、q1、Repetition Level 32を表示する。" ),
        large_prism.key,
        large_prism.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "31_large_prism_t0_q1_rl32" ), large_prism, SamplingMode::Uniform, QStringLiteral( "Large/LargePrism/large_prism.*.vtm, time step:0, q1, Repetition Level:32" ), CaseOptions( 32 ) ); } );

    runCase(
        QStringLiteral( "32_large_prism_t7_q1_rl32" ),
        QStringLiteral( "LargePrismのtime step 7、q1、Repetition Level 32を表示する。" ),
        large_prism.key,
        large_prism.path,
        [&]()
        {
            setTimeStepAndJump( client, 7 );
            captureCase( QStringLiteral( "32_large_prism_t7_q1_rl32" ), QStringLiteral( "Large/LargePrism/large_prism.*.vtm, time step:7, q1, Repetition Level:32" ), 32, &client );
        } );

    runCase(
        QStringLiteral( "33_large_pyramid_t0_q1_rl32" ),
        QStringLiteral( "LargePyramidのtime step 0、q1、Repetition Level 32を表示する。" ),
        large_pyramid.key,
        large_pyramid.path,
        [&]() { load_apply_jump_capture( QStringLiteral( "33_large_pyramid_t0_q1_rl32" ), large_pyramid, SamplingMode::Uniform, QStringLiteral( "Large/LargePyramid/large_pyramid.*.vtm, time step:0, q1, Repetition Level:32" ), CaseOptions( 32 ) ); } );

    runCase(
        QStringLiteral( "34_large_pyramid_t7_q1_rl32" ),
        QStringLiteral( "LargePyramidのtime step 7、q1、Repetition Level 32を表示する。" ),
        large_pyramid.key,
        large_pyramid.path,
        [&]()
        {
            setTimeStepAndJump( client, 7 );
            captureCase( QStringLiteral( "34_large_pyramid_t7_q1_rl32" ), QStringLiteral( "Large/LargePyramid/large_pyramid.*.vtm, time step:7, q1, Repetition Level:32" ), 32, &client );
        } );

    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::ServerTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
