#include "TransferFunctionEditorImportInvalidFileTest.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QTest>
#include <QTextStream>
#include <QTimer>

#include <cmath>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../ExtendedQT/ColorMap.h"
#include "../ExtendedQT/OpacityMap.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TransferFunctionEditor.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_dialog_timeout_ms = 10000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_jump_button_enable_timeout_ms = 120000;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_after_jump_wait_ms = 3000;
constexpr int k_capture_settle_ms = 300;
constexpr int k_no_message_box_wait_ms = 3000;
constexpr int k_transfer_function_count = 2;
const char* const k_item_57 = "C-TFE-57";
const char* const k_item_58 = "C-TFE-58";
kvs::qt::Application* g_test_app = nullptr;

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

bool nearlyEqual( double a, double b )
{
    return std::abs( a - b ) < 1.0e-8;
}

bool opacitiesNearlyEqual( const QVector<float>& a, const QVector<float>& b )
{
    if ( a.size() != b.size() ) { return false; }
    for ( int i = 0; i < a.size(); ++i )
    {
        if ( std::abs( a.at( i ) - b.at( i ) ) > 1.0e-6f ) { return false; }
    }
    return true;
}
}

TransferFunctionEditorTest::ImportInvalidFileTest::ImportInvalidFileTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArrayLiteral( "900000" ) );
}

QString TransferFunctionEditorTest::ImportInvalidFileTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString TransferFunctionEditorTest::ImportInvalidFileTest::repoRootPath() const
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

bool TransferFunctionEditorTest::ImportInvalidFileTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, QDir::toNativeSeparators( text ) );
    QCOMPARE( line_edit->text(), QDir::toNativeSeparators( text ) );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::markStep( const QString& item_id, const QString& description, const QString& status )
{
    const QString line = QStringLiteral( "%1 %2" ).arg( item_id, description );
    m_steps.push_back( { line, status } );
    qInfo().noquote() << QStringLiteral( "[%1] %2" ).arg( status, line );
}

bool TransferFunctionEditorTest::ImportInvalidFileTest::check( ItemResult& result, bool condition, const QString& message )
{
    if ( condition ) { return true; }
    result.messages.push_back( message );
    result.status = QStringLiteral( "FAIL" );
    markStep( result.id, message, QStringLiteral( "FAIL" ) );
    return false;
}

void TransferFunctionEditorTest::ImportInvalidFileTest::skipItem( ItemResult& result, const QString& reason )
{
    result.status = QStringLiteral( "SKIP" );
    result.messages.push_back( reason );
    markStep( result.id, reason, QStringLiteral( "SKIP" ) );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::finishItem( ItemResult& result )
{
    if ( result.status == QStringLiteral( "NOT RUN" ) ) { result.status = QStringLiteral( "PASS" ); }
    m_item_results.push_back( result );
    markStep( result.id, QStringLiteral( "%1: %2" ).arg( result.title, result.status ), result.status );
}

// 既存テスト(ImportExportTest)と同じ方法(プライマリスクリーン全体)で撮影し、
// manual_rel_path が指定されていればマニュアル用のコピー(img/manual/ 以下)も出力する。
void TransferFunctionEditorTest::ImportInvalidFileTest::saveScreenshot(
    ItemResult& result,
    int seq,
    const QString& content,
    const QString& caption,
    const QString& manual_rel_path )
{
    ScreenshotEntry entry;
    entry.item_id = result.id;
    entry.seq = seq;
    entry.file_name = QStringLiteral( "%1_%2_%3.png" ).arg( result.id ).arg( seq ).arg( content );
    entry.caption = caption;
    entry.manual_rel_path = manual_rel_path;

    if ( !ClientTests::screenshotsEnabled() )
    {
        m_screenshots.push_back( entry );
        markStep( result.id, QStringLiteral( "スクリーンショット(連番 %1)は撮影無効のため SKIP" ).arg( seq ), QStringLiteral( "SKIP" ) );
        return;
    }

    QTest::qWait( k_capture_settle_ms );
    QScreen* screen = QGuiApplication::primaryScreen();
    if ( !check( result, screen != nullptr, QStringLiteral( "プライマリスクリーンが見つからない(連番 %1)" ).arg( seq ) ) )
    {
        m_screenshots.push_back( entry );
        return;
    }

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( entry.file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );
    if ( !check( result, !screenshot.isNull(), QStringLiteral( "スクリーンショットの取得に失敗(連番 %1)" ).arg( seq ) ) )
    {
        m_screenshots.push_back( entry );
        return;
    }
    entry.captured = screenshot.save( file_path );
    check( result, entry.captured, QStringLiteral( "スクリーンショットの保存に失敗: %1" ).arg( file_path ) );
    if ( entry.captured && !manual_rel_path.isEmpty() )
    {
        const QString destination = QDir( m_manual_dir_path ).absoluteFilePath( manual_rel_path );
        QDir().mkpath( QFileInfo( destination ).absolutePath() );
        if ( QFileInfo::exists( destination ) ) { QFile::remove( destination ); }
        entry.copied = QFile::copy( file_path, destination );
        check( result, entry.copied, QStringLiteral( "マニュアル用コピーに失敗: %1" ).arg( manual_rel_path ) );
    }
    m_screenshots.push_back( entry );
    if ( manual_rel_path.isEmpty() )
    {
        markStep( result.id, QStringLiteral( "スクリーンショットを撮影: %1" ).arg( caption ) );
    }
    else
    {
        markStep( result.id, QStringLiteral( "スクリーンショットを撮影: %1 → manual: img/manual/%2" ).arg( caption, manual_rel_path ) );
    }
}

void TransferFunctionEditorTest::ImportInvalidFileTest::writeStateTable( QTextStream& stream, const QString& title, const EditorState& state ) const
{
    stream << "### " << title << "\n\n";
    stream << "- numberOfTransferFunctionSpinBox: " << state.count << "\n";
    stream << "- colorSynthesizerLineEdit: `" << state.color_synthesizer << "`\n";
    stream << "- opacitySynthesizerLineEdit: `" << state.opacity_synthesizer << "`\n\n";
    stream << "| 伝達関数 | レンジモード | User Min | User Max | マップ要素数 |\n|---|---|---|---|---|\n";
    for ( int i = 0; i < state.colors.size(); ++i )
    {
        const RangeEntry& entry = state.colors.at( i );
        stream << "| C" << ( i + 1 ) << " | " << ( entry.user_range ? "User" : "Server" ) << " | " << entry.min << " | " << entry.max
               << " | " << entry.colors.size() << " |\n";
    }
    for ( int i = 0; i < state.opacities.size(); ++i )
    {
        const RangeEntry& entry = state.opacities.at( i );
        stream << "| O" << ( i + 1 ) << " | " << ( entry.user_range ? "User" : "Server" ) << " | " << entry.min << " | " << entry.max
               << " | " << entry.opacities.size() << " |\n";
    }
    stream << "\n";
}

void TransferFunctionEditorTest::ImportInvalidFileTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    if ( !report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        qWarning().noquote() << QStringLiteral( "Failed to write test report: %1" ).arg( m_report_path );
        return;
    }

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditor::ImportInvalidFileTest\n\n";
    stream << "- 対応項目: PBVRリグレッションテストチェックシート Client シート TransferFunctionEditor 57 / 58(追加項目仕様_アンサンブル C-TFE-57 / C-TFE-58)\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- モード: 通常モード(CS)。サーバ(pbvr_server)は既存の ImportExportTest と同じく起動済みであることを前提とする\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- ボリュームデータ(SPX_VOLUME_DATA): `" << m_volume_data_path << "`\n";
    stream << "- テスト用 .tfe(テスト内で Export から生成):\n";
    stream << "    - base.tfe: `" << m_base_tfe_path << "`\n";
    stream << "    - unreadable.tfe(chmod 000): `" << m_unreadable_tfe_path << "`\n";
    stream << "    - no_tf_number.tfe(TF_NUMBER= 行を削除、TF_NAME1_MIN_C=0.2 / TF_NAME1_MAX_C=0.8): `" << m_no_tf_number_tfe_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`(マニュアル用コピー: `img/manual/`)\n\n";

    stream << "## 項目結果\n\n";
    stream << "| 項目ID | 内容 | 結果 | 備考 |\n|---|---|---|---|\n";
    for ( const ItemResult& item : m_item_results )
    {
        QString notes = item.messages.join( QStringLiteral( "<br>" ) );
        notes.replace( QLatin1Char( '|' ), QStringLiteral( "\\|" ) );
        stream << "| " << item.id << " | " << item.title << " | " << item.status << " | " << notes << " |\n";
    }

    stream << "\n## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << step.status << ": " << step.description << "\n";
    }

    stream << "\n## TransferFunctionEditor の状態\n\n";
    if ( m_has_baseline ) { writeStateTable( stream, QStringLiteral( "Import 前(base.tfe を Export した時点)" ), m_baseline_state ); }
    if ( m_has_state_57 ) { writeStateTable( stream, QStringLiteral( "C-TFE-57 unreadable.tfe Import 後(変化なしが期待値)" ), m_state_after_57 ); }
    if ( m_has_state_58 ) { writeStateTable( stream, QStringLiteral( "C-TFE-58 no_tf_number.tfe Import 後(C1 のみ 0.2 / 0.8 が期待値)" ), m_state_after_58 ); }

    stream << "## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.item_id << "-" << entry.seq << " " << entry.caption << "\n\n";
        if ( !entry.manual_rel_path.isEmpty() )
        {
            stream << "- マニュアル用コピー先: `img/manual/" << entry.manual_rel_path << "`("
                   << ( entry.copied ? "出力済み" : ( entry.captured ? "コピー失敗" : "SKIP" ) ) << ")\n\n";
        }
        if ( entry.captured )
        {
            stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
        }
    }

    stream << "## マニュアル用画像\n\n";
    stream << "| 項目ID | 連番 | テスト用ファイル | マニュアル用コピー先 | 状態 |\n|---|---|---|---|---|\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        if ( entry.manual_rel_path.isEmpty() ) { continue; }
        stream << "| " << entry.item_id << " | " << entry.seq << " | `img/" << entry.file_name << "` | `img/manual/" << entry.manual_rel_path
               << "` | " << ( entry.copied ? "出力済み" : "未出力(SKIP)" ) << " |\n";
    }

    stream << "\n## 備考(アンサンブル側 Import との違い)\n\n";
    stream << "- TransferFunctionEditor::onImport は KEY=VALUE 行(.tfe)を順に処理し、開けないファイル・未知キー・欠損キーはいずれも無言で扱う(警告ダイアログなし)。"
              "TF_NUMBER 行が無い場合は伝達関数数を変更せず、存在する TF_NAME<n>_* 行だけを現在の伝達関数に上書きする。\n";
    stream << "- EnsembleTransferFunctionEditor::onImport は JSON を読み込み、不正な JSON では「Invalid JSON file.」、キー欠損では"
              "「Some default.json keys were missing. Existing GUI values were kept for those fields.」、開けない場合は「Could not open import file.」の警告ダイアログを表示する(C-ETF-17 / 18 で確認)。"
              "両者は実装が別であり、本テストは通常モード側の現状仕様(警告なし)を確認する。\n";
    stream << "- 現在の伝達関数数を超える TF_NAME<n> を含むファイルでは TransferFunction::at() が std::out_of_range を投げて異常終了するため、本テストでは意図的にそのケースを含めない(修正候補として別途管理)。\n";
}

TransferFunctionEditorTest::ImportInvalidFileTest::ClientHandles
TransferFunctionEditorTest::ImportInvalidFileTest::resolveClientHandles( MainWindow& window ) const
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

    TransferFunctionEditor* editor = handles.transfer_function_editor;
    handles.tf_import_button = editor->findChild<QPushButton*>( "importPushButton" );
    handles.tf_export_button = editor->findChild<QPushButton*>( "exportPushButton" );
    handles.tf_apply_button = editor->findChild<QPushButton*>( "applyPushButton" );
    handles.tf_number_spin_box = editor->findChild<QSpinBox*>( "numberOfTransferFunctionSpinBox" );
    handles.tf_color_synthesizer_line_edit = editor->findChild<QLineEdit*>( "colorSynthesizerLineEdit" );
    handles.tf_opacity_synthesizer_line_edit = editor->findChild<QLineEdit*>( "opacitySynthesizerLineEdit" );
    handles.tf_color_function_combo_box = editor->findChild<QComboBox*>( "colorFunctionComboBox" );
    handles.tf_opacity_function_combo_box = editor->findChild<QComboBox*>( "opacityFunctionComboBox" );
    handles.tf_color_user_min_max_radio = editor->findChild<QRadioButton*>( "colorUserDefinedMinMaxRadioButton" );
    handles.tf_color_user_min_spin_box = editor->findChild<QDoubleSpinBox*>( "colorUserDefinedMinDoubleSpinBox" );
    handles.tf_color_user_max_spin_box = editor->findChild<QDoubleSpinBox*>( "colorUserDefinedMaxDoubleSpinBox" );
    handles.tf_opacity_user_min_max_radio = editor->findChild<QRadioButton*>( "opacityUserDefinedMinMaxRadioButton" );
    handles.tf_opacity_user_min_spin_box = editor->findChild<QDoubleSpinBox*>( "opacityUserDefinedMinDoubleSpinBox" );
    handles.tf_opacity_user_max_spin_box = editor->findChild<QDoubleSpinBox*>( "opacityUserDefinedMaxDoubleSpinBox" );
    handles.tf_color_map = editor->findChild<ColorMap*>( "colorMap" );
    handles.tf_opacity_map = editor->findChild<OpacityMap*>( "opacityMap" );

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
    if ( !require( handles.tf_number_spin_box != nullptr, "numberOfTransferFunctionSpinBox not found" ) ) { return handles; }
    if ( !require( handles.tf_color_synthesizer_line_edit != nullptr, "colorSynthesizerLineEdit not found" ) ) { return handles; }
    if ( !require( handles.tf_opacity_synthesizer_line_edit != nullptr, "opacitySynthesizerLineEdit not found" ) ) { return handles; }
    if ( !require( handles.tf_color_function_combo_box != nullptr, "colorFunctionComboBox not found" ) ) { return handles; }
    if ( !require( handles.tf_opacity_function_combo_box != nullptr, "opacityFunctionComboBox not found" ) ) { return handles; }
    if ( !require( handles.tf_color_user_min_max_radio != nullptr, "colorUserDefinedMinMaxRadioButton not found" ) ) { return handles; }
    if ( !require( handles.tf_color_user_min_spin_box != nullptr, "colorUserDefinedMinDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.tf_color_user_max_spin_box != nullptr, "colorUserDefinedMaxDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.tf_opacity_user_min_max_radio != nullptr, "opacityUserDefinedMinMaxRadioButton not found" ) ) { return handles; }
    if ( !require( handles.tf_opacity_user_min_spin_box != nullptr, "opacityUserDefinedMinDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.tf_opacity_user_max_spin_box != nullptr, "opacityUserDefinedMaxDoubleSpinBox not found" ) ) { return handles; }
    if ( !require( handles.tf_color_map != nullptr, "TransferFunctionEditor colorMap not found" ) ) { return handles; }
    if ( !require( handles.tf_opacity_map != nullptr, "TransferFunctionEditor opacityMap not found" ) ) { return handles; }

    return handles;
}

void TransferFunctionEditorTest::ImportInvalidFileTest::connectClient( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::disconnectClient( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::configureRemoteVisualization( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::waitForObjectAndApply( const ClientHandles& client ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
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

QFileDialog* TransferFunctionEditorTest::ImportInvalidFileTest::waitForFileDialog( int timeout_ms ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::selectFileFromDialog( const QString& file_path, bool require_existing_file ) const
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

void TransferFunctionEditorTest::ImportInvalidFileTest::exportTransferFunction( const ClientHandles& client, const QString& file_path )
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

// importPushButton → ファイルダイアログで選択(Apply は押さない)
void TransferFunctionEditorTest::ImportInvalidFileTest::importTransferFunctionWithoutApply( const ClientHandles& client, const QString& file_path )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_import_button->isEnabled(), "importPushButton is disabled" );

    QTimer::singleShot( 0, [this, file_path]() { selectFileFromDialog( file_path, true ); } );
    QTest::mouseClick( client.tf_import_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

QMessageBox* TransferFunctionEditorTest::ImportInvalidFileTest::findVisibleMessageBox() const
{
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        auto* box = qobject_cast<QMessageBox*>( widget );
        if ( box != nullptr && box->isVisible() ) { return box; }
    }
    return nullptr;
}

// wait_ms の間 QMessageBox が表示されないことを確認する(表示された場合は閉じて FAIL)
bool TransferFunctionEditorTest::ImportInvalidFileTest::verifyNoMessageBox( ItemResult& result, int wait_ms )
{
    bool shown = false;
    QString text;
    QElapsedTimer timer;
    timer.start();
    while ( timer.elapsed() < wait_ms )
    {
        if ( QMessageBox* box = findVisibleMessageBox() )
        {
            shown = true;
            text = box->text();
            box->accept();
            break;
        }
        QTest::qWait( 50 );
    }
    if ( check( result, !shown, QStringLiteral( "QMessageBox が表示された(現状仕様では表示されない想定): %1" ).arg( text ) ) )
    {
        markStep( result.id, QStringLiteral( "%1 秒待機し、その間に QMessageBox が表示されないことを確認した" ).arg( wait_ms / 1000 ) );
        return true;
    }
    return false;
}

void TransferFunctionEditorTest::ImportInvalidFileTest::selectColorFunction( const ClientHandles& client, int index ) const
{
    QVERIFY2( client.tf_color_function_combo_box->count() > index,
              qPrintable( QStringLiteral( "colorFunctionComboBox has only %1 items" ).arg( client.tf_color_function_combo_box->count() ) ) );
    client.tf_color_function_combo_box->setFocus();
    client.tf_color_function_combo_box->setCurrentIndex( index );
    QCOMPARE( client.tf_color_function_combo_box->currentIndex(), index );
    QCoreApplication::processEvents();
    QTest::qWait( 200 );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::selectOpacityFunction( const ClientHandles& client, int index ) const
{
    QVERIFY2( client.tf_opacity_function_combo_box->count() > index,
              qPrintable( QStringLiteral( "opacityFunctionComboBox has only %1 items" ).arg( client.tf_opacity_function_combo_box->count() ) ) );
    client.tf_opacity_function_combo_box->setFocus();
    client.tf_opacity_function_combo_box->setCurrentIndex( index );
    QCOMPARE( client.tf_opacity_function_combo_box->currentIndex(), index );
    QCoreApplication::processEvents();
    QTest::qWait( 200 );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value, const char* widget_name ) const
{
    QVERIFY2( spin_box != nullptr, widget_name );
    QVERIFY2( spin_box->isEnabled(), qPrintable( QStringLiteral( "%1 is disabled" ).arg( widget_name ) ) );
    QVERIFY2(
        value >= spin_box->minimum() && value <= spin_box->maximum(),
        qPrintable( QStringLiteral( "%1 target value is out of range: %2" ).arg( widget_name ).arg( value ) ) );

    spin_box->setFocus();
    spin_box->setValue( value );
    QCoreApplication::processEvents();
    QTest::qWait( 200 );
    QVERIFY2(
        nearlyEqual( spin_box->value(), value ),
        qPrintable( QStringLiteral( "%1 was not set to %2 (actual %3)" ).arg( widget_name ).arg( value ).arg( spin_box->value() ) ) );
}

// numberOfTransferFunctionSpinBox / Synthesizer / C1..Cn・O1..On の User Min/Max、カラーマップ、オパシティマップを記録する
TransferFunctionEditorTest::ImportInvalidFileTest::EditorState
TransferFunctionEditorTest::ImportInvalidFileTest::captureEditorState( const ClientHandles& client ) const
{
    EditorState state;
    state.count = client.tf_number_spin_box->value();
    state.color_synthesizer = client.tf_color_synthesizer_line_edit->text();
    state.opacity_synthesizer = client.tf_opacity_synthesizer_line_edit->text();

    const int color_index = client.tf_color_function_combo_box->currentIndex();
    const int opacity_index = client.tf_opacity_function_combo_box->currentIndex();
    for ( int index = 0; index < state.count; ++index )
    {
        if ( client.tf_color_function_combo_box->count() > index )
        {
            client.tf_color_function_combo_box->setCurrentIndex( index );
            QCoreApplication::processEvents();
            RangeEntry entry;
            entry.user_range = client.tf_color_user_min_max_radio->isChecked();
            entry.min = client.tf_color_user_min_spin_box->value();
            entry.max = client.tf_color_user_max_spin_box->value();
            entry.colors = client.tf_color_map->getColors();
            state.colors.push_back( entry );
        }
        if ( client.tf_opacity_function_combo_box->count() > index )
        {
            client.tf_opacity_function_combo_box->setCurrentIndex( index );
            QCoreApplication::processEvents();
            RangeEntry entry;
            entry.user_range = client.tf_opacity_user_min_max_radio->isChecked();
            entry.min = client.tf_opacity_user_min_spin_box->value();
            entry.max = client.tf_opacity_user_max_spin_box->value();
            entry.opacities = client.tf_opacity_map->getOpacities();
            state.opacities.push_back( entry );
        }
    }
    if ( color_index >= 0 ) { client.tf_color_function_combo_box->setCurrentIndex( color_index ); }
    if ( opacity_index >= 0 ) { client.tf_opacity_function_combo_box->setCurrentIndex( opacity_index ); }
    QCoreApplication::processEvents();
    return state;
}

bool TransferFunctionEditorTest::ImportInvalidFileTest::rangeEntryEqual(
    const RangeEntry& a,
    const RangeEntry& b,
    const QString& label,
    QStringList* differences ) const
{
    bool equal = true;
    if ( a.user_range != b.user_range ) { equal = false; if ( differences ) { *differences << label + QStringLiteral( " レンジモード" ); } }
    if ( !nearlyEqual( a.min, b.min ) ) { equal = false; if ( differences ) { *differences << QStringLiteral( "%1 User Min(%2 → %3)" ).arg( label ).arg( a.min ).arg( b.min ); } }
    if ( !nearlyEqual( a.max, b.max ) ) { equal = false; if ( differences ) { *differences << QStringLiteral( "%1 User Max(%2 → %3)" ).arg( label ).arg( a.max ).arg( b.max ); } }
    if ( a.colors != b.colors ) { equal = false; if ( differences ) { *differences << label + QStringLiteral( " カラーマップ" ); } }
    if ( !opacitiesNearlyEqual( a.opacities, b.opacities ) ) { equal = false; if ( differences ) { *differences << label + QStringLiteral( " オパシティマップ" ); } }
    return equal;
}

bool TransferFunctionEditorTest::ImportInvalidFileTest::statesEqual( const EditorState& a, const EditorState& b, QStringList* differences ) const
{
    bool equal = true;
    if ( a.count != b.count ) { equal = false; if ( differences ) { *differences << QStringLiteral( "numberOfTransferFunctionSpinBox(%1 → %2)" ).arg( a.count ).arg( b.count ); } }
    if ( a.color_synthesizer != b.color_synthesizer ) { equal = false; if ( differences ) { *differences << QStringLiteral( "colorSynthesizerLineEdit" ); } }
    if ( a.opacity_synthesizer != b.opacity_synthesizer ) { equal = false; if ( differences ) { *differences << QStringLiteral( "opacitySynthesizerLineEdit" ); } }
    if ( a.colors.size() != b.colors.size() ) { equal = false; if ( differences ) { *differences << QStringLiteral( "Color 伝達関数の数" ); } }
    if ( a.opacities.size() != b.opacities.size() ) { equal = false; if ( differences ) { *differences << QStringLiteral( "Opacity 伝達関数の数" ); } }
    for ( int i = 0; i < std::min( a.colors.size(), b.colors.size() ); ++i )
    {
        if ( !rangeEntryEqual( a.colors.at( i ), b.colors.at( i ), QStringLiteral( "C%1" ).arg( i + 1 ), differences ) ) { equal = false; }
    }
    for ( int i = 0; i < std::min( a.opacities.size(), b.opacities.size() ); ++i )
    {
        if ( !rangeEntryEqual( a.opacities.at( i ), b.opacities.at( i ), QStringLiteral( "O%1" ).arg( i + 1 ), differences ) ) { equal = false; }
    }
    return equal;
}

// base.tfe から unreadable.tfe(chmod 000)と no_tf_number.tfe を生成する
bool TransferFunctionEditorTest::ImportInvalidFileTest::generateTestFiles( ItemResult& result )
{
    QFile base( m_base_tfe_path );
    if ( !check( result, base.open( QIODevice::ReadOnly | QIODevice::Text ), QStringLiteral( "base.tfe を開けない: %1" ).arg( m_base_tfe_path ) ) )
    {
        return false;
    }
    const QStringList lines = QString::fromUtf8( base.readAll() ).split( QLatin1Char( '\n' ) );
    base.close();

    bool has_number = false;
    bool has_min_c1 = false;
    bool has_max_c1 = false;
    for ( const QString& line : lines )
    {
        if ( line.startsWith( QStringLiteral( "TF_NUMBER=" ) ) ) { has_number = line.trimmed() == QStringLiteral( "TF_NUMBER=%1" ).arg( k_transfer_function_count ); }
        if ( line.startsWith( QStringLiteral( "TF_NAME1_MIN_C=" ) ) ) { has_min_c1 = true; }
        if ( line.startsWith( QStringLiteral( "TF_NAME1_MAX_C=" ) ) ) { has_max_c1 = true; }
    }
    check( result, has_number, QStringLiteral( "base.tfe に TF_NUMBER=%1 の行が無い" ).arg( k_transfer_function_count ) );
    check( result, has_min_c1 && has_max_c1, QStringLiteral( "base.tfe に TF_NAME1_MIN_C= / TF_NAME1_MAX_C= の行が無い" ) );

    // unreadable.tfe
    if ( QFileInfo::exists( m_unreadable_tfe_path ) )
    {
        QFile::setPermissions( m_unreadable_tfe_path, QFileDevice::ReadOwner | QFileDevice::WriteOwner );
        QFile::remove( m_unreadable_tfe_path );
    }
    if ( !check( result, QFile::copy( m_base_tfe_path, m_unreadable_tfe_path ), QStringLiteral( "unreadable.tfe の作成に失敗: %1" ).arg( m_unreadable_tfe_path ) ) )
    {
        return false;
    }
#ifndef Q_OS_WIN
    check( result, QFile::setPermissions( m_unreadable_tfe_path, QFileDevice::Permissions() ), QStringLiteral( "unreadable.tfe に chmod 000 を適用できない" ) );
#endif

    // no_tf_number.tfe
    QStringList modified;
    for ( const QString& line : lines )
    {
        if ( line.startsWith( QStringLiteral( "TF_NUMBER=" ) ) ) { continue; }
        if ( line.startsWith( QStringLiteral( "TF_NAME1_MIN_C=" ) ) ) { modified << QStringLiteral( "TF_NAME1_MIN_C=0.2" ); continue; }
        if ( line.startsWith( QStringLiteral( "TF_NAME1_MAX_C=" ) ) ) { modified << QStringLiteral( "TF_NAME1_MAX_C=0.8" ); continue; }
        modified << line;
    }
    QFile no_number( m_no_tf_number_tfe_path );
    if ( !check( result, no_number.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ), QStringLiteral( "no_tf_number.tfe を作成できない: %1" ).arg( m_no_tf_number_tfe_path ) ) )
    {
        return false;
    }
    no_number.write( modified.join( QLatin1Char( '\n' ) ).toUtf8() );
    no_number.close();
    markStep( result.id, QStringLiteral( "base.tfe から unreadable.tfe(chmod 000)と no_tf_number.tfe(TF_NUMBER= 行削除、TF_NAME1_MIN_C=0.2、TF_NAME1_MAX_C=0.8)を生成した" ) );
    return true;
}

// ===========================================================================
// C-TFE-57 開けない伝達関数ファイルの Import
// ===========================================================================
void TransferFunctionEditorTest::ImportInvalidFileTest::runItem57UnreadableFile(
    ItemResult& result,
    const ClientHandles& client,
    const EditorState& baseline )
{
#ifdef Q_OS_WIN
    Q_UNUSED( client );
    Q_UNUSED( baseline );
    skipItem( result, QStringLiteral( "Windows では chmod 000 相当の読み取り不可ファイルを作成できないため SKIP" ) );
    return;
#else
    {
        QFile probe( m_unreadable_tfe_path );
        if ( probe.open( QIODevice::ReadOnly ) )
        {
            probe.close();
            skipItem( result, QStringLiteral( "chmod 000 を適用しても unreadable.tfe を読めてしまう(root 実行など)ため SKIP" ) );
            return;
        }
    }

    importTransferFunctionWithoutApply( client, m_unreadable_tfe_path );
    markStep( result.id, QStringLiteral( "TransferFunctionEditor.ui: importPushButton を押し、ファイルダイアログで unreadable.tfe を選択した" ) );
    verifyNoMessageBox( result, k_no_message_box_wait_ms );
    if ( check( result, client.main_window->isVisible(), QStringLiteral( "MainWindow が表示されていない(アプリケーションが終了した可能性)" ) ) )
    {
        markStep( result.id, QStringLiteral( "アプリケーションが終了していないこと(MainWindow が isVisible)を確認した" ) );
    }

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    m_state_after_57 = captureEditorState( client );
    m_has_state_57 = true;
    QStringList differences;
    if ( check( result, statesEqual( baseline, m_state_after_57, &differences ),
                QStringLiteral( "Import 前と値が異なる: %1" ).arg( differences.join( QStringLiteral( ", " ) ) ) ) )
    {
        markStep( result.id, QStringLiteral( "numberOfTransferFunctionSpinBox / Synthesizer / C1・C2・O1・O2 の全値が Import 前と同一であることを確認した" ) );
    }

    selectColorFunction( client, 0 );
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( result, 1, QStringLiteral( "unreadable_after_import" ),
                    QStringLiteral( "開けないファイルを Import した後のエディタ(変化なし)" ),
                    QStringLiteral( "Client/TransferFunctionEditor/57_1.png" ) );
#endif
}

// ===========================================================================
// C-TFE-58 TF_NUMBER 行が無い伝達関数ファイルの Import
// ===========================================================================
void TransferFunctionEditorTest::ImportInvalidFileTest::runItem58NoTfNumber(
    ItemResult& result,
    const ClientHandles& client,
    const EditorState& baseline )
{
    importTransferFunctionWithoutApply( client, m_no_tf_number_tfe_path );
    markStep( result.id, QStringLiteral( "TransferFunctionEditor.ui: importPushButton を押し、ファイルダイアログで no_tf_number.tfe を選択した" ) );
    verifyNoMessageBox( result, k_no_message_box_wait_ms );
    if ( check( result, client.main_window->isVisible(), QStringLiteral( "MainWindow が表示されていない(アプリケーションが終了した可能性)" ) ) )
    {
        markStep( result.id, QStringLiteral( "アプリケーションが終了していないことを確認した" ) );
    }

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    if ( check( result, client.tf_number_spin_box->value() == k_transfer_function_count,
                QStringLiteral( "numberOfTransferFunctionSpinBox が %1 ではない: %2" ).arg( k_transfer_function_count ).arg( client.tf_number_spin_box->value() ) ) )
    {
        markStep( result.id, QStringLiteral( "numberOfTransferFunctionSpinBox の値が %1 のまま(Import 前と同一)であることを確認した" ).arg( k_transfer_function_count ) );
    }

    // C1: 0.2 / 0.8
    selectColorFunction( client, 0 );
    const double c1_min = client.tf_color_user_min_spin_box->value();
    const double c1_max = client.tf_color_user_max_spin_box->value();
    if ( check( result, nearlyEqual( c1_min, 0.2 ) && nearlyEqual( c1_max, 0.8 ),
                QStringLiteral( "C1 の User Min/Max が 0.2 / 0.8 ではない: %1 / %2" ).arg( c1_min ).arg( c1_max ) ) )
    {
        markStep( result.id, QStringLiteral( "colorFunctionComboBox で C1 を選択し、colorUserDefinedMinDoubleSpinBox が 0.2、colorUserDefinedMaxDoubleSpinBox が 0.8 であることを確認した" ) );
    }

    // C2: Import 前と同一(0.3 / 0.7)
    selectColorFunction( client, 1 );
    const double c2_min = client.tf_color_user_min_spin_box->value();
    const double c2_max = client.tf_color_user_max_spin_box->value();
    const RangeEntry& baseline_c2 = baseline.colors.size() > 1 ? baseline.colors.at( 1 ) : RangeEntry();
    if ( check( result, nearlyEqual( c2_min, baseline_c2.min ) && nearlyEqual( c2_max, baseline_c2.max ) && nearlyEqual( c2_min, 0.3 ) && nearlyEqual( c2_max, 0.7 ),
                QStringLiteral( "C2 の User Min/Max が 0.3 / 0.7(Import 前と同一)ではない: %1 / %2" ).arg( c2_min ).arg( c2_max ) ) )
    {
        markStep( result.id, QStringLiteral( "colorFunctionComboBox で C2 を選択し、User Min/Max が 0.3 / 0.7(Import 前と同一)であることを確認した" ) );
    }

    // その他(O1・O2、オパシティマップ、Synthesizer、カラーマップ)は Import 前と同一
    m_state_after_58 = captureEditorState( client );
    m_has_state_58 = true;
    EditorState expected = baseline;
    if ( expected.colors.size() > 0 )
    {
        expected.colors[0].min = 0.2;
        expected.colors[0].max = 0.8;
    }
    QStringList differences;
    if ( check( result, statesEqual( expected, m_state_after_58, &differences ),
                QStringLiteral( "C1 の Min/Max 以外に Import 前と異なる値がある: %1" ).arg( differences.join( QStringLiteral( ", " ) ) ) ) )
    {
        markStep( result.id, QStringLiteral( "O1・O2 の User Min/Max とオパシティマップ、Synthesizer、カラーマップが Import 前と同一であることを確認した" ) );
    }

    selectColorFunction( client, 0 );
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( result, 1, QStringLiteral( "no_tf_number_c1" ),
                    QStringLiteral( "C1: Min 0.2 / Max 0.8 が反映" ),
                    QStringLiteral( "Client/TransferFunctionEditor/58_1.png" ) );
    selectColorFunction( client, 1 );
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( result, 2, QStringLiteral( "no_tf_number_c2" ),
                    QStringLiteral( "C2: Import 前の値のまま" ),
                    QStringLiteral( "Client/TransferFunctionEditor/58_2.png" ) );
}

void TransferFunctionEditorTest::ImportInvalidFileTest::initTestCase()
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
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/ImportInvalidFileTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_manual_dir_path = QDir( m_screenshot_dir_path ).absoluteFilePath( QStringLiteral( "manual" ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_base_tfe_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "base.tfe" ) );
    m_unreadable_tfe_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "unreadable.tfe" ) );
    m_no_tf_number_tfe_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "no_tf_number.tfe" ) );
    m_test_succeeded = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_manual_dir_path ), qPrintable( QStringLiteral( "Failed to create manual directory: %1" ).arg( m_manual_dir_path ) ) );
    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

    QFile report( m_report_path );
    QVERIFY2( report.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ), "Failed to initialize TestResult.md" );
    QTextStream stream( &report );
    stream << "# TransferFunctionEditor::ImportInvalidFileTest\n\n- 結果: RUNNING\n";
}

void TransferFunctionEditorTest::ImportInvalidFileTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }

    // chmod 000 のままだと出力ディレクトリの削除・コピーに支障が出るため、権限を戻しておく
    if ( !m_unreadable_tfe_path.isEmpty() && QFileInfo::exists( m_unreadable_tfe_path ) )
    {
        QFile::setPermissions( m_unreadable_tfe_path, QFileDevice::ReadOwner | QFileDevice::WriteOwner );
    }

    writeMarkdownReport();
}

void TransferFunctionEditorTest::ImportInvalidFileTest::performs_import_invalid_file_scenario()
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

    ItemResult setup;
    setup.id = QStringLiteral( "C-TFE-57/58" );
    setup.title = QStringLiteral( "共通手順(接続・伝達関数設定・Export・テストファイル生成)" );

    // ---- Communication.ui / ObjectEditor.ui / PlayBackControlToolBar ----
    connectClient( client );
    markStep( setup.id, QStringLiteral( "Communication.ui: connectPushButton を押し、接続状態になった" ) );
    configureRemoteVisualization( client );
    markStep( setup.id, QStringLiteral( "Communication.ui: remoteVizClientServerRadioButton を選択し、volumeDataFilePathLineEdit に SPX_VOLUME_DATA を書き込み、settingApplyPushButton を押した" ) );
    waitForObjectAndApply( client );
    markStep( setup.id, QStringLiteral( "ObjectEditor.ui: nameLineEdit にテキストが入るまで待機し、applyPushButton を押した" ) );
    clickJumpAndWaitForCompletion( client );
    markStep( setup.id, QStringLiteral( "PlayBackControlToolBar: m_jump_push_button を押し、有効になるまで待機した" ) );

    // ---- TransferFunctionEditor.ui: 伝達関数数 2、C1 0.1/0.9、C2 0.3/0.7 ----
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    client.tf_number_spin_box->setFocus();
    client.tf_number_spin_box->setValue( k_transfer_function_count );
    QCoreApplication::processEvents();
    QTest::qWait( k_short_wait_ms );
    QVERIFY2( client.tf_number_spin_box->value() == k_transfer_function_count, "numberOfTransferFunctionSpinBox was not set to 2" );
    markStep( setup.id, QStringLiteral( "TransferFunctionEditor.ui: numberOfTransferFunctionSpinBox を %1 に設定した" ).arg( k_transfer_function_count ) );

    selectColorFunction( client, 0 );
    if ( !client.tf_color_user_min_max_radio->isChecked() )
    {
        QTest::mouseClick( client.tf_color_user_min_max_radio, Qt::LeftButton );
        QTest::qWait( 200 );
    }
    QVERIFY2( client.tf_color_user_min_max_radio->isChecked(), "colorUserDefinedMinMaxRadioButton was not checked" );
    setDoubleSpinBoxValue( client.tf_color_user_min_spin_box, 0.1, "colorUserDefinedMinDoubleSpinBox" );
    setDoubleSpinBoxValue( client.tf_color_user_max_spin_box, 0.9, "colorUserDefinedMaxDoubleSpinBox" );
    markStep( setup.id, QStringLiteral( "TransferFunctionEditor.ui: colorFunctionComboBox で C1 を選択し、colorUserDefinedMinMaxRadioButton を選択、User Min/Max を 0.1 / 0.9 に設定した" ) );

    selectColorFunction( client, 1 );
    if ( !client.tf_color_user_min_max_radio->isChecked() )
    {
        QTest::mouseClick( client.tf_color_user_min_max_radio, Qt::LeftButton );
        QTest::qWait( 200 );
    }
    setDoubleSpinBoxValue( client.tf_color_user_min_spin_box, 0.3, "colorUserDefinedMinDoubleSpinBox" );
    setDoubleSpinBoxValue( client.tf_color_user_max_spin_box, 0.7, "colorUserDefinedMaxDoubleSpinBox" );
    markStep( setup.id, QStringLiteral( "TransferFunctionEditor.ui: colorFunctionComboBox で C2 を選択し、User Min/Max を 0.3 / 0.7 に設定した" ) );

    // 設定が保持されていることを確認
    selectColorFunction( client, 0 );
    QVERIFY2( nearlyEqual( client.tf_color_user_min_spin_box->value(), 0.1 ) && nearlyEqual( client.tf_color_user_max_spin_box->value(), 0.9 ), "C1 User Min/Max was not kept (0.1 / 0.9)" );
    selectColorFunction( client, 1 );
    QVERIFY2( nearlyEqual( client.tf_color_user_min_spin_box->value(), 0.3 ) && nearlyEqual( client.tf_color_user_max_spin_box->value(), 0.7 ), "C2 User Min/Max was not kept (0.3 / 0.7)" );
    selectColorFunction( client, 0 );

    // ---- Export → base.tfe、派生ファイル生成 ----
    exportTransferFunction( client, m_base_tfe_path );
    markStep( setup.id, QStringLiteral( "TransferFunctionEditor.ui: exportPushButton を押し、ファイルダイアログで base.tfe として保存した" ) );
    const bool files_ok = generateTestFiles( setup );

    // ---- Import 前の状態を記録・撮影 ----
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    m_baseline_state = captureEditorState( client );
    m_has_baseline = true;
    selectColorFunction( client, 0 );
    markStep( setup.id, QStringLiteral( "Import 前の状態を記録した: numberOfTransferFunctionSpinBox=%1、Synthesizer、C1・C2 の User Min/Max、カラーマップ、O1・O2 の User Min/Max、オパシティマップ" ).arg( m_baseline_state.count ) );
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot( setup, 0, QStringLiteral( "before_import" ), QStringLiteral( "Import 前の TransferFunctionEditor の状態" ), QString() );
    finishItem( setup );

    if ( files_ok )
    {
        // ---- C-TFE-57 ----
        ItemResult item57;
        item57.id = QString::fromLatin1( k_item_57 );
        item57.title = QStringLiteral( "開けない伝達関数ファイルの Import" );
        runItem57UnreadableFile( item57, client, m_baseline_state );
        finishItem( item57 );

        // ---- C-TFE-58 ----
        ItemResult item58;
        item58.id = QString::fromLatin1( k_item_58 );
        item58.title = QStringLiteral( "TF_NUMBER 行が無い伝達関数ファイルの Import" );
        runItem58NoTfNumber( item58, client, m_baseline_state );
        finishItem( item58 );
    }
    else
    {
        ItemResult item57;
        item57.id = QString::fromLatin1( k_item_57 );
        item57.title = QStringLiteral( "開けない伝達関数ファイルの Import" );
        skipItem( item57, QStringLiteral( "テスト用 .tfe の生成に失敗したため SKIP" ) );
        finishItem( item57 );
        ItemResult item58;
        item58.id = QString::fromLatin1( k_item_58 );
        item58.title = QStringLiteral( "TF_NUMBER 行が無い伝達関数ファイルの Import" );
        skipItem( item58, QStringLiteral( "テスト用 .tfe の生成に失敗したため SKIP" ) );
        finishItem( item58 );
    }

    // ---- Communication.ui: 切断 ----
    client.transfer_function_editor->close();
    disconnectClient( client );
    markStep( QStringLiteral( "C-TFE-57/58" ), QStringLiteral( "Communication.ui: disconnectPushButton を押し、未接続状態になった" ) );

    bool any_failed = false;
    for ( const ItemResult& item : m_item_results )
    {
        if ( item.status == QStringLiteral( "FAIL" ) ) { any_failed = true; }
    }
    m_test_succeeded = !any_failed;

    main_window.close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );

    QVERIFY2( !any_failed, "One or more check-sheet items (C-TFE-57 / C-TFE-58) failed. See TestResult.md" );
}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::ImportInvalidFileTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
