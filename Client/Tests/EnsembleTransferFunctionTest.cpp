#include "EnsembleTransferFunctionTest.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDate>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTableWidget>
#include <QTest>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>
#include <QAbstractButton>
#include <QJsonValue>
#include <QMenu>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QSettings>

#include <memory>

#include <cmath>

#include <kvs/qt/Application>
#include <kvs/PointObject>

#include "../App/MainWindow.h"
#include "../ExtendedQT/ColorMap.h"
#include "../ExtendedQT/Histogram.h"
#include "../Widgets/ColorMapSelectorToolBar.h"
#include "../ExtendedQT/OpacityMap.h"
#include "../Widgets/Communication.h"
#include "../Widgets/EnsembleTransferFunctionEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "../Widgets/TotalParticlesToolBar.h"
#include "../Widgets/VolumeTransform.h"
#include "../../Shared/JsonKeys.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace ClientTests
{
constexpr int k_connect_timeout_ms = 15000;
constexpr int k_dialog_timeout_ms = 10000;
constexpr int k_object_load_timeout_ms = 120000;
constexpr int k_data_request_timeout_ms = 180000;
constexpr int k_apply_propagation_wait_ms = 5000;
// Apply 後に生成されたステップで判定するためのマージン。Apply 後に最初に通知された最新ステップ M' に対し、
// Apply 時点で生成中だったステップは高々 M'+1 のため、M'+2 以上のステップを「Apply 後の設定で生成された」とみなす。
// (シミュレーションが古いステップ(例: 接続直後の step 12)に張り付いた応答を返しても、それを判定に使わない)
constexpr int k_ensemble_generation_margin_steps = 2;
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 500;
constexpr int k_capture_settle_ms = 500;
constexpr double k_server_range_decimal_scale = 100000000.0;

static kvs::qt::Application* g_ensemble_test_app = nullptr;

QString findRepositoryRoot( const QString& start_path )
{
    QDir dir( start_path );
    while( dir.exists() )
    {
        if( dir.exists( QStringLiteral( ".git" ) ) &&
            dir.exists( QStringLiteral( "Client" ) ) &&
            dir.exists( QStringLiteral( "Server" ) ) )
        {
            return dir.absolutePath();
        }
        if( !dir.cdUp() ) break;
    }
    return QString();
}

bool isRoundedToEightDecimalPlaces( const double value )
{
    const double rounded = std::round( value * k_server_range_decimal_scale ) /
        k_server_range_decimal_scale;
    return std::abs( value - rounded ) <= 1.0e-12;
}

namespace
{
// 定義はファイル後半(チェックシート項目用ヘルパー)にある
bool opacitiesNearlyEqual( const QVector<float>& a, const QVector<float>& b );
}

EnsembleTransferFunctionTest::EnsembleTransferFunctionTest( QObject* parent )
    : QObject( parent )
{
    qputenv( "QTEST_FUNCTION_TIMEOUT", QByteArrayLiteral( "1800000" ) );
}

bool EnsembleTransferFunctionTest::eventFilter( QObject* watched, QEvent* event )
{
    if( watched == m_render_screen_object && event && event->type() == QEvent::Paint )
    {
        ++m_render_paint_count;
    }
    return QObject::eventFilter( watched, event );
}

QString EnsembleTransferFunctionTest::repoRootPath() const
{
    const QString app_root = findRepositoryRoot( QCoreApplication::applicationDirPath() );
    if( !app_root.isEmpty() ) return app_root;

    const QString cwd_root = findRepositoryRoot( QDir::currentPath() );
    if( !cwd_root.isEmpty() ) return cwd_root;

    const QString source_root = findRepositoryRoot( QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() );
    return source_root.isEmpty() ? QDir::currentPath() : source_root;
}

bool EnsembleTransferFunctionTest::waitForCondition(
    const std::function<bool()>& condition,
    int timeout_ms,
    int interval_ms ) const
{
    QElapsedTimer timer;
    timer.start();
    while( timer.elapsed() < timeout_ms )
    {
        if( condition() ) return true;
        QTest::qWait( interval_ms );
    }
    return condition();
}

void EnsembleTransferFunctionTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void EnsembleTransferFunctionTest::bringEditorToFront( EnsembleTransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "EnsembleTransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "EnsembleTransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void EnsembleTransferFunctionTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->selectAll();
    QTest::keyClicks( line_edit, text );
    QCOMPARE( line_edit->text(), text );
}

void EnsembleTransferFunctionTest::saveWindowScreenshot( QWidget* window, const QString& file_name ) const
{
    if( !screenshotsEnabled() ) return;

    QVERIFY2( window != nullptr, "Screenshot target window is null" );
    QTest::qWait( k_capture_settle_ms );
    QScreen* screen = QGuiApplication::screenAt( window->mapToGlobal( window->rect().center() ) );
    if( !screen ) screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen != nullptr, "Screenshot target screen not found" );

    const QString path = QDir( m_screenshot_dir_path ).absoluteFilePath( file_name );
    const QPixmap screenshot = screen->grabWindow( window->winId() );
    QVERIFY2( !screenshot.isNull(), "Failed to capture the target window" );
    QVERIFY2( screenshot.save( path ), qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( path ) ) );
}

void EnsembleTransferFunctionTest::saveScreenshotPair(
    const ClientHandles& client,
    const QString& file_name_prefix,
    const QString& title,
    const QString& change_description,
    const ParticleUpdateResult& update_result )
{
    if( !screenshotsEnabled() ) return;

    const QString particle_file_name = file_name_prefix + QStringLiteral( "_particles.png" );
    const QString editor_file_name = file_name_prefix + QStringLiteral( "_editor.png" );
    bringWindowToFront( client.main_window );
    saveWindowScreenshot( client.main_window, particle_file_name );
    bringEditorToFront( client.ensemble_editor );
    saveWindowScreenshot( client.ensemble_editor, editor_file_name );
    m_screenshot_pairs.push_back(
        { title,
          change_description,
          particle_file_name,
          editor_file_name,
          update_result.request_time_step,
          update_result.request_count,
          update_result.before_fingerprint,
          update_result.after_fingerprint,
          update_result.paint_completed } );
}

void EnsembleTransferFunctionTest::writeMarkdownReport() const
{
    QFile report( m_report_path );
    if( !report.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        qWarning().noquote() << QStringLiteral( "Failed to write test report: %1" ).arg( m_report_path );
        return;
    }

    QTextStream stream( &report );
    stream << "# EnsembleTransferFunctionTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- 前提: サーバと In-Situ アンサンブル計算プログラムは起動済み\n";
    stream << "- Apply 後の粒子生成側反映待ち: 5秒\n";
    stream << "- Keep Last: 初期 Jump 後からテスト終了まで ON\n";
    stream << "- Volume Transform: 初期粒子生成後にX軸回転を-30度へ設定\n";
    stream << "- Export JSON: `" << m_export_path << "`\n";
    stream << "- 画像ピクセルの自動比較: 実施しない（粒子配列のフィンガープリントで更新を確認）\n";
    stream << "- チェックシート項目: PBVRリグレッションテストチェックシート 追加項目仕様_アンサンブル C-ETF-01〜19 / C-MB-15 / C-CMS-02（結果は末尾の表）\n";
    stream << "- マニュアル用画像: `img/manual/` 以下（Docs/ClientRegressionTestManual/img/ と同じ命名）\n";
    stream << "- IsEnsemble はサーバが Initialize 時に送る SelectedFile イベントで受信する(ObjectInfoParameter には含まれない)\n";
    stream << "- サーバの再配信(EnsembleStatisticsParameter)は uWebSockets の ws->publish が送信元を除外するため単一クライアントでは受信できず、参考情報として記録するのみ\n";
    stream << "- 基本シナリオの各ケースは失敗しても中断せず記録して続行する(結果は「基本シナリオ結果」表)\n";
    stream << "- 実行前に pbvr_server とシミュレーションを両方再起動し RESET.sh を実行すること(サーバの保持する前回の Apply 値が Initialize で default.json に書き戻され、基準状態になるため)\n";
    stream << "- 応答不一致時の診断 JSON: `diag/<ケース名>_mismatch_first.json` / `_mismatch_last.json`(期待値と応答を保存)\n";
    stream << "- Apply の判定対象は「Apply 後に生成されたステップ」に限定する。Apply 後に最初に通知された最新ステップ M'(LatestTimeStep: UpdateMaxTimeStep)に対し time step >= M'+" << k_ensemble_generation_margin_steps << " の RequestDataAt 応答だけを判定に使う(Apply 前の古いステップに張り付いた応答を誤判定しないため。RepetitionLevelControlInSituTest と同じ方式)\n\n";

    stream << "## 実施手順\n\n";
    for( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    if( m_has_baseline_settings )
    {
        stream << "\n## 基準状態（初回受信値）\n\n";
        stream << "- Variable: `" << m_baseline_variable << "`\n";
        stream << "- 取得元: 接続後に最初に受信した有効な `EnsembleStatisticsParameter`\n\n";
        for( const StatisticState& state : m_baseline_states )
        {
            stream << "### " << state.statistic << "\n\n";
            stream << "- MinMaxモード: " << ( state.use_user_range ? "User" : "Server" ) << "\n";
            stream << "- User MinMax: " << state.user_min << " / " << state.user_max << "\n";
            stream << "- Server MinMax: " << state.server_min << " / " << state.server_max << "\n";
            stream << "- Color Map要素数: " << state.colors.size() << "\n";
            stream << "- Opacity Map要素数: " << state.opacities.size() << "\n\n";
        }
    }

    writeImportedSettings( stream );

    stream << "\n## スクリーンショット\n\n";
    stream << "各見出し内の2枚は、同じ Apply とデータ要求完了後に設定を変更せず撮影した組です。"
              "左にMainWindow、右にEnsemble Transfer Function Editorを表示します。\n\n";
    for( const ScreenshotPair& pair : m_screenshot_pairs )
    {
        stream << "### " << pair.title << "\n\n";
        stream << "- 変更内容: " << pair.change_description << "\n\n";
        stream << "- RequestDataAtタイムステップ: " << pair.request_time_step << "\n";
        stream << "- 確認したKeep Last要求数: " << pair.request_count << "\n";
        stream << "- RequestDataAt設定一致: 確認済み\n";
        stream << "- 変更前粒子フィンガープリント: `" << pair.before_fingerprint.toHex() << "`\n";
        stream << "- 撮影時粒子フィンガープリント: `" << pair.after_fingerprint.toHex() << "`\n";
        stream << "- Render Screen再描画: " << ( pair.paint_completed ? "完了" : "未確認" ) << "\n\n";
        stream << "<table>\n";
        stream << "<tr>\n";
        stream << "<th width=\"60%\">MainWindow（粒子表示）</th>\n";
        stream << "<th width=\"40%\">Ensemble Transfer Function Editor</th>\n";
        stream << "</tr>\n";
        stream << "<tr>\n";
        stream << "<td><img src=\"./img/" << pair.particle_file_name
               << "\" alt=\"" << pair.title << "の粒子表示\" width=\"100%\"></td>\n";
        stream << "<td><img src=\"./img/" << pair.editor_file_name
               << "\" alt=\"" << pair.title << "のEditor\" width=\"100%\"></td>\n";
        stream << "</tr>\n";
        stream << "</table>\n\n";
    }

    stream << "## 目視確認\n\n";
    stream << "各統計の変更なし画像と、Variable、User MinMax、Server MinMax、Color Map、Opacity Mapを"
              "それぞれ単独で変更した画像を比較し、粒子分布・密度・色が設定に応じて変化していることを確認する。\n";

    writeBaseCaseSection( stream );
    writeCheckSheetSections( stream );
}

void EnsembleTransferFunctionTest::writeImportedSettings( QTextStream& stream ) const
{
    if( !m_has_import_settings ) return;

    stream << "\n## Import対象値\n\n";
    stream << "- Variable: `" << m_import_variable << "`\n";
    stream << "- Import元: `" << m_export_path << "`\n\n";
    for( const StatisticState& state : m_import_states )
    {
        stream << "### " << state.statistic << "\n\n";
        stream << "- MinMaxモード: " << ( state.use_user_range ? "User" : "Server" ) << "\n";
        stream << "- User MinMax: " << state.user_min << " / " << state.user_max << "\n";
        stream << "- Color Map要素数: " << state.colors.size() << "\n";
        stream << "- Opacity Map要素数: " << state.opacities.size() << "\n\n";
        stream << "<details><summary>Color Map値</summary>\n\n`";
        for( int index = 0; index < state.colors.size(); ++index )
        {
            if( index > 0 ) stream << ", ";
            stream << state.colors.at( index ).name( QColor::HexRgb );
        }
        stream << "`\n\n</details>\n\n";
        stream << "<details><summary>Opacity Map値</summary>\n\n`";
        for( int index = 0; index < state.opacities.size(); ++index )
        {
            if( index > 0 ) stream << ", ";
            stream << state.opacities.at( index );
        }
        stream << "`\n\n</details>\n\n";
    }
}

void EnsembleTransferFunctionTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    qInfo().noquote() << description;
}

EnsembleTransferFunctionTest::ClientHandles
EnsembleTransferFunctionTest::resolveClientHandles( MainWindow& window ) const
{
    ClientHandles handles;
    const auto require = []( bool condition, const char* message )
    {
        if( condition ) return true;
        QTest::qFail( message, __FILE__, __LINE__ );
        return false;
    };
    handles.main_window = &window;
    handles.communication = window.findChild<Communication*>();
    handles.object_editor = window.findChild<ObjectEditor*>();
    handles.playback_tool_bar = window.findChild<PlayBackControlToolBar*>();
    handles.time_step_tool_bar = window.findChild<TimeStepControlToolBar*>();
    handles.total_particles_tool_bar = window.findChild<TotalParticlesToolBar*>();
    handles.ensemble_editor = window.findChild<EnsembleTransferFunctionEditor*>();
    handles.volume_transform = window.findChild<VolumeTransform*>();
    for( QWidget* widget : window.findChildren<QWidget*>() )
    {
        if( dynamic_cast<kvs::qt::jaea::Screen*>( widget ) )
        {
            handles.render_screen = widget;
            break;
        }
    }

    if( !require( handles.communication != nullptr, "Communication dock not found" ) ) return handles;
    if( !require( handles.object_editor != nullptr, "ObjectEditor dock not found" ) ) return handles;
    if( !require( handles.playback_tool_bar != nullptr, "PlayBackControlToolBar not found" ) ) return handles;
    if( !require( handles.time_step_tool_bar != nullptr, "TimeStepControlToolBar not found" ) ) return handles;
    if( !require( handles.total_particles_tool_bar != nullptr, "TotalParticlesToolBar not found" ) ) return handles;
    if( !require( handles.ensemble_editor != nullptr, "EnsembleTransferFunctionEditor not found" ) ) return handles;
    if( !require( handles.volume_transform != nullptr, "VolumeTransform not found" ) ) return handles;
    if( !require( handles.render_screen != nullptr, "Render Screen not found" ) ) return handles;

    handles.connect_button = handles.communication->findChild<QPushButton*>( "connectPushButton" );
    handles.disconnect_button = handles.communication->findChild<QPushButton*>( "disconnectPushButton" );
    handles.setting_apply_button = handles.communication->findChild<QPushButton*>( "settingApplyPushButton" );
    handles.remote_viz_insitu_radio = handles.communication->findChild<QRadioButton*>( "remoteVizInsituRadioButton" );
    handles.id_line_edit = handles.communication->findChild<QLineEdit*>( "IDLineEdit" );
    handles.object_tree_view = handles.object_editor->findChild<QTreeView*>( "treeView" );
    handles.object_apply_button = handles.object_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.jump_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_jump_push_button" );
    handles.keep_last_button = handles.playback_tool_bar->findChild<QPushButton*>( "m_keep_last_push_button" );
    handles.ensemble_apply_button = handles.ensemble_editor->findChild<QPushButton*>( "m_apply_push_button" );
    handles.ensemble_import_button = handles.ensemble_editor->findChild<QPushButton*>( "m_import_push_button" );
    handles.ensemble_export_button = handles.ensemble_editor->findChild<QPushButton*>( "m_export_push_button" );
    handles.variable_line_edit = handles.ensemble_editor->findChild<QLineEdit*>( "m_statistics_synthesizer_line_edit" );
    handles.statistic_combo_box = handles.ensemble_editor->findChild<QComboBox*>( "m_statistics_combo_box" );
    handles.user_min_max_radio = handles.ensemble_editor->findChild<QRadioButton*>( "m_user_min_max_radio_button" );
    handles.server_min_max_radio = handles.ensemble_editor->findChild<QRadioButton*>( "m_server_min_max_radio_button" );
    handles.user_min_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_user_min_double_spin_box" );
    handles.user_max_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_user_max_double_spin_box" );
    handles.server_min_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_server_min_double_spin_box" );
    handles.server_max_spin_box = handles.ensemble_editor->findChild<QDoubleSpinBox*>( "m_server_max_double_spin_box" );
    handles.rotation_x_axis_spin_box =
        handles.volume_transform->findChild<QDoubleSpinBox*>( "rotationXAxisDoubleSpinBox" );
    handles.volume_transform_apply_button =
        handles.volume_transform->findChild<QPushButton*>( "applyPushButton" );
    handles.color_map = handles.ensemble_editor->findChild<ColorMap*>( "m_color_map" );
    handles.opacity_map = handles.ensemble_editor->findChild<OpacityMap*>( "m_opacity_map" );
    handles.histogram = handles.ensemble_editor->findChild<Histogram*>( "m_histogram" );
    if( auto* selector = window.findChild<ColorMapSelectorToolBar*>() )
    {
        handles.color_function_combo_box = selector->findChild<QComboBox*>();
    }

    const auto labels = handles.total_particles_tool_bar->findChildren<QLabel*>();
    for( QLabel* label : labels )
    {
        if( label && label->text() != QStringLiteral( "Total Particles : " ) )
        {
            handles.total_particles_display = label;
            break;
        }
    }

    if( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) return handles;
    if( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) return handles;
    if( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) return handles;
    if( !require( handles.remote_viz_insitu_radio != nullptr, "remoteVizInsituRadioButton not found" ) ) return handles;
    if( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) return handles;
    if( !require( handles.object_tree_view != nullptr, "ObjectEditor treeView not found" ) ) return handles;
    if( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) return handles;
    if( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) return handles;
    if( !require( handles.keep_last_button != nullptr, "m_keep_last_push_button not found" ) ) return handles;
    if( !require( handles.ensemble_apply_button != nullptr, "m_apply_push_button not found" ) ) return handles;
    if( !require( handles.ensemble_import_button != nullptr, "m_import_push_button not found" ) ) return handles;
    if( !require( handles.ensemble_export_button != nullptr, "m_export_push_button not found" ) ) return handles;
    if( !require( handles.variable_line_edit != nullptr, "m_statistics_synthesizer_line_edit not found" ) ) return handles;
    if( !require( handles.statistic_combo_box != nullptr, "m_statistics_combo_box not found" ) ) return handles;
    if( !require( handles.user_min_max_radio != nullptr, "m_user_min_max_radio_button not found" ) ) return handles;
    if( !require( handles.server_min_max_radio != nullptr, "m_server_min_max_radio_button not found" ) ) return handles;
    if( !require( handles.user_min_spin_box != nullptr, "m_user_min_double_spin_box not found" ) ) return handles;
    if( !require( handles.user_max_spin_box != nullptr, "m_user_max_double_spin_box not found" ) ) return handles;
    if( !require( handles.server_min_spin_box != nullptr, "m_server_min_double_spin_box not found" ) ) return handles;
    if( !require( handles.server_max_spin_box != nullptr, "m_server_max_double_spin_box not found" ) ) return handles;
    if( !require( handles.rotation_x_axis_spin_box != nullptr, "rotationXAxisDoubleSpinBox not found" ) ) return handles;
    if( !require( handles.volume_transform_apply_button != nullptr, "VolumeTransform applyPushButton not found" ) ) return handles;
    if( !require( handles.color_map != nullptr, "m_color_map not found" ) ) return handles;
    if( !require( handles.opacity_map != nullptr, "m_opacity_map not found" ) ) return handles;
    if( !require( handles.total_particles_display != nullptr, "Total Particles display label not found" ) ) return handles;
    return handles;
}

void EnsembleTransferFunctionTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2(
        waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
        "connectPushButton did not become enabled" );
    QTest::mouseClick( client.connect_button, Qt::LeftButton );
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

void EnsembleTransferFunctionTest::configureInSituVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2( client.remote_viz_insitu_radio->isEnabled(), "remoteVizInsituRadioButton is disabled" );
    if( !client.remote_viz_insitu_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_insitu_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_insitu_radio->isChecked(), "remoteVizInsituRadioButton was not checked" );
    QVERIFY2( client.setting_apply_button->isEnabled(), "settingApplyPushButton is disabled" );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
}

void EnsembleTransferFunctionTest::waitForObjectAndApply( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                return client.object_tree_view->model() != nullptr &&
                       client.object_tree_view->model()->rowCount() > 0;
            },
            k_object_load_timeout_ms,
            100 ),
        "ObjectEditor treeView did not receive an item" );

    QAbstractItemModel* model = client.object_tree_view->model();
    const QModelIndex first = model->index( 0, 0 );
    QVERIFY2( first.isValid(), "ObjectEditor first row is invalid" );
    client.object_tree_view->setCurrentIndex( first );
    QVERIFY2( client.object_tree_view->selectionModel() != nullptr, "ObjectEditor selection model not found" );
    client.object_tree_view->selectionModel()->setCurrentIndex(
        first,
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    QVERIFY2(
        waitForCondition( [client]() { return client.object_apply_button->isEnabled(); }, k_object_load_timeout_ms, 100 ),
        "ObjectEditor applyPushButton did not become enabled" );
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void EnsembleTransferFunctionTest::generateInitialParticles(
    const ClientHandles& client,
    QSignalSpy& completion_spy ) const
{
    QVERIFY2(
        waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_data_request_timeout_ms, 100 ),
        "m_jump_push_button did not become enabled" );
    completion_spy.clear();
    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTRY_VERIFY_WITH_TIMEOUT( completion_spy.count() > 0, k_data_request_timeout_ms );
    waitForPositiveParticleCount( client );
}

void EnsembleTransferFunctionTest::enableKeepLast( const ClientHandles& client ) const
{
    QVERIFY2( client.keep_last_button->isEnabled(), "m_keep_last_push_button is disabled" );
    if( !client.keep_last_button->isChecked() )
    {
        QTest::mouseClick( client.keep_last_button, Qt::LeftButton );
    }
    QVERIFY2( client.keep_last_button->isChecked(), "Keep Last was not enabled" );
}

void EnsembleTransferFunctionTest::applyVolumeRotationX( const ClientHandles& client, double value ) const
{
    QVERIFY2( client.rotation_x_axis_spin_box != nullptr, "rotationXAxisDoubleSpinBox is null" );
    QVERIFY2( client.volume_transform_apply_button != nullptr, "VolumeTransform applyPushButton is null" );
    client.rotation_x_axis_spin_box->setValue( value );
    QCOMPARE( client.rotation_x_axis_spin_box->value(), value );
    client.volume_transform_apply_button->click();
}

void EnsembleTransferFunctionTest::waitForPositiveParticleCount( const ClientHandles& client ) const
{
    QVERIFY2(
        waitForCondition(
            [client]()
            {
                bool ok = false;
                const int particles = client.total_particles_display->text().toInt( &ok );
                return ok && particles > 0;
            },
            k_data_request_timeout_ms,
            100 ),
        qPrintable(
            QStringLiteral( "Total Particles did not become positive. current='%1'" )
                .arg( client.total_particles_display->text() ) ) );
}

void EnsembleTransferFunctionTest::selectStatistic( const ClientHandles& client, int index ) const
{
    QVERIFY2( index >= 0 && index < client.statistic_combo_box->count(), "Statistic index is out of range" );
    client.statistic_combo_box->setCurrentIndex( index );
    QCOMPARE( client.statistic_combo_box->currentIndex(), index );
    QCoreApplication::processEvents();
}

void EnsembleTransferFunctionTest::setUserRange(
    const ClientHandles& client,
    double min_value,
    double max_value ) const
{
    QVERIFY2( min_value <= max_value, "User MinMax is invalid" );
    client.user_min_spin_box->setValue( min_value );
    client.user_max_spin_box->setValue( max_value );
    if( !client.user_min_max_radio->isChecked() )
    {
        QTest::mouseClick( client.user_min_max_radio, Qt::LeftButton );
    }
    QVERIFY2( client.user_min_max_radio->isChecked(), "User MinMax mode was not selected" );
    QVERIFY2( std::abs( client.user_min_spin_box->value() - min_value ) < 1.0e-7, "User Min was rounded unexpectedly" );
    QVERIFY2( std::abs( client.user_max_spin_box->value() - max_value ) < 1.0e-7, "User Max was rounded unexpectedly" );
}

void EnsembleTransferFunctionTest::setServerRange( const ClientHandles& client ) const
{
    if( !client.server_min_max_radio->isChecked() )
    {
        QTest::mouseClick( client.server_min_max_radio, Qt::LeftButton );
    }
    QVERIFY2( client.server_min_max_radio->isChecked(), "Server MinMax mode was not selected" );
    QVERIFY2(
        client.server_min_spin_box->value() <= client.server_max_spin_box->value(),
        "Server MinMax is invalid" );
}

void EnsembleTransferFunctionTest::applyColorPreset(
    const ClientHandles& client,
    const QString& preset_name ) const
{
    QTimer::singleShot(
        0,
        [this, preset_name]()
        {
            QDialog* dialog = nullptr;
            QVERIFY2(
                waitForCondition(
                    [&dialog]()
                    {
                        for( QWidget* widget : QApplication::topLevelWidgets() )
                        {
                            auto* candidate = qobject_cast<QDialog*>( widget );
                            if( candidate && candidate->objectName() == QStringLiteral( "ColorMapEditor" ) && candidate->isVisible() )
                            {
                                dialog = candidate;
                                return true;
                            }
                        }
                        return false;
                    },
                    k_dialog_timeout_ms,
                    50 ),
                "ColorMapEditor did not become visible" );

            auto* table = dialog->findChild<QTableWidget*>( "colorMapTableWidget" );
            QVERIFY2( table != nullptr, "ColorMapEditor colorMapTableWidget not found" );
            int preset_row = -1;
            int preset_column = -1;
            for( int row = 0; row < table->rowCount() && preset_row < 0; ++row )
            {
                for( int column = 0; column < table->columnCount(); ++column )
                {
                    QWidget* cell = table->cellWidget( row, column );
                    if( !cell ) continue;
                    const auto labels = cell->findChildren<QLabel*>();
                    for( QLabel* label : labels )
                    {
                        if( label && label->text() == preset_name )
                        {
                            preset_row = row;
                            preset_column = column;
                            break;
                        }
                    }
                    if( preset_row >= 0 ) break;
                }
            }
            QVERIFY2( preset_row >= 0, qPrintable( QStringLiteral( "Color preset not found: %1" ).arg( preset_name ) ) );
            QVERIFY2(
                QMetaObject::invokeMethod(
                    dialog,
                    "onPresetColorMapDoubleClicked",
                    Qt::DirectConnection,
                    Q_ARG( int, preset_row ),
                    Q_ARG( int, preset_column ) ),
                "Failed to select ColorMap preset" );
            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "ColorMapEditor applyPushButton not found" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
        } );

    QTest::mouseClick( client.color_map, Qt::LeftButton, Qt::NoModifier, client.color_map->rect().center() );
    QCoreApplication::processEvents();
}

void EnsembleTransferFunctionTest::applyOpacityExpression(
    const ClientHandles& client,
    const QString& expression ) const
{
    QTimer::singleShot(
        0,
        [this, expression]()
        {
            QDialog* dialog = nullptr;
            QVERIFY2(
                waitForCondition(
                    [&dialog]()
                    {
                        for( QWidget* widget : QApplication::topLevelWidgets() )
                        {
                            auto* candidate = qobject_cast<QDialog*>( widget );
                            if( candidate && candidate->objectName() == QStringLiteral( "OpacityMapEditor" ) && candidate->isVisible() )
                            {
                                dialog = candidate;
                                return true;
                            }
                        }
                        return false;
                    },
                    k_dialog_timeout_ms,
                    50 ),
                "OpacityMapEditor did not become visible" );

            auto* tab_widget = dialog->findChild<QTabWidget*>( "tabWidget" );
            auto* expression_page = dialog->findChild<QWidget*>( "expression" );
            QVERIFY2( tab_widget != nullptr, "OpacityMapEditor tabWidget not found" );
            QVERIFY2( expression_page != nullptr, "OpacityMapEditor expression page not found" );
            tab_widget->setCurrentWidget( expression_page );

            auto* line_edit = dialog->findChild<QLineEdit*>( "opacityLineEdit" );
            QVERIFY2( line_edit != nullptr, "OpacityMapEditor opacityLineEdit not found" );
            line_edit->setText( expression );
            QVERIFY2(
                QMetaObject::invokeMethod( dialog, "onExpressionChanged", Qt::DirectConnection ),
                "Failed to apply opacity expression" );
            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "OpacityMapEditor applyPushButton not found" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
        } );

    QTest::mouseClick( client.opacity_map, Qt::LeftButton, Qt::NoModifier, client.opacity_map->rect().center() );
    QCoreApplication::processEvents();
}

EnsembleTransferFunctionTest::StatisticState
EnsembleTransferFunctionTest::captureStatisticState( const ClientHandles& client, int index ) const
{
    selectStatistic( client, index );
    StatisticState state;
    state.statistic = client.ensemble_editor->selectedStatistic();
    state.use_user_range = client.user_min_max_radio->isChecked();
    state.user_min = client.user_min_spin_box->value();
    state.user_max = client.user_max_spin_box->value();
    state.server_min = client.server_min_spin_box->value();
    state.server_max = client.server_max_spin_box->value();
    state.colors = client.color_map->getColors();
    state.opacities = client.opacity_map->getOpacities();
    return state;
}

std::array<EnsembleTransferFunctionTest::StatisticState, 3>
EnsembleTransferFunctionTest::captureAllStatisticStates( const ClientHandles& client ) const
{
    std::array<StatisticState, 3> states;
    for( int index = 0; index < 3; ++index ) states[static_cast<size_t>( index )] = captureStatisticState( client, index );
    return states;
}

bool EnsembleTransferFunctionTest::extractSettingsFromPayload(
    const QJsonObject& payload,
    QString* variable,
    std::array<StatisticState, 3>* states ) const
{
    if( variable == nullptr || states == nullptr ) return false;

    const QJsonArray data = payload.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();
    if( data.size() != 3 ) return false;

    const std::array<QString, 3> statistic_names = {
        QStringLiteral( "average" ),
        QStringLiteral( "variance" ),
        QStringLiteral( "cv" ) };
    std::array<bool, 3> found = { false, false, false };
    QString received_variable;

    for( const QJsonValue& value : data )
    {
        if( !value.isObject() ) return false;
        const QJsonObject patch = value.toObject();
        QString statistic = patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString().trimmed().toLower();
        if( statistic == QStringLiteral( "mean" ) ) statistic = QStringLiteral( "average" );
        if( statistic == QStringLiteral( "coefficient of variation" ) ||
            statistic == QStringLiteral( "coefficient_of_variation" ) )
        {
            statistic = QStringLiteral( "cv" );
        }

        int state_index = -1;
        for( int index = 0; index < 3; ++index )
        {
            if( statistic == statistic_names[static_cast<size_t>( index )] )
            {
                state_index = index;
                break;
            }
        }
        if( state_index < 0 || found[static_cast<size_t>( state_index )] ) return false;

        const QString patch_variable =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleVariable ) ).toString().trimmed();
        if( patch_variable.isEmpty() ) return false;
        if( received_variable.isEmpty() ) received_variable = patch_variable;
        if( received_variable != patch_variable ) return false;

        const QString mode_key = QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode );
        const QString user_min_key = QString::fromUtf8( Protocol::Key::EnsembleUserRangeMin );
        const QString user_max_key = QString::fromUtf8( Protocol::Key::EnsembleUserRangeMax );
        const QString server_min_key = QString::fromUtf8( Protocol::Key::EnsembleServerRangeMin );
        const QString server_max_key = QString::fromUtf8( Protocol::Key::EnsembleServerRangeMax );
        if( !patch.value( mode_key ).isDouble() ||
            !patch.value( user_min_key ).isDouble() ||
            !patch.value( user_max_key ).isDouble() ||
            !patch.value( server_min_key ).isDouble() ||
            !patch.value( server_max_key ).isDouble() )
        {
            return false;
        }

        const int range_mode = patch.value( mode_key ).toInt();
        if( range_mode != 1 && range_mode != 2 ) return false;

        const QJsonArray color_values =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleColorMap ) ).toArray();
        const QJsonArray opacity_values =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleOpacityMap ) ).toArray();
        if( color_values.isEmpty() || opacity_values.isEmpty() ) return false;

        StatisticState state;
        state.statistic = statistic_names[static_cast<size_t>( state_index )];
        state.use_user_range = range_mode == 1;
        state.user_min = patch.value( user_min_key ).toDouble();
        state.user_max = patch.value( user_max_key ).toDouble();
        state.server_min = patch.value( server_min_key ).toDouble();
        state.server_max = patch.value( server_max_key ).toDouble();
        if( !isRoundedToEightDecimalPlaces( state.server_min ) ||
            !isRoundedToEightDecimalPlaces( state.server_max ) )
        {
            return false;
        }
        for( const QJsonValue& color_value : color_values )
        {
            const QJsonArray rgb = color_value.toArray();
            if( rgb.size() != 3 ) return false;
            state.colors.push_back( QColor( rgb.at( 0 ).toInt(), rgb.at( 1 ).toInt(), rgb.at( 2 ).toInt() ) );
        }
        for( const QJsonValue& opacity_value : opacity_values )
        {
            if( !opacity_value.isDouble() ) return false;
            state.opacities.push_back( static_cast<float>( opacity_value.toDouble() ) );
        }

        ( *states )[static_cast<size_t>( state_index )] = state;
        found[static_cast<size_t>( state_index )] = true;
    }

    if( !found[0] || !found[1] || !found[2] ) return false;
    *variable = received_variable;
    return true;
}

bool EnsembleTransferFunctionTest::waitForInitialSettings(
    QSignalSpy& parameter_spy,
    QString* variable,
    std::array<StatisticState, 3>* states ) const
{
    int inspected_count = 0;
    return waitForCondition(
        [this, &parameter_spy, &inspected_count, variable, states]()
        {
            while( inspected_count < parameter_spy.count() )
            {
                const QList<QVariant> arguments = parameter_spy.at( inspected_count++ );
                if( arguments.isEmpty() ) continue;
                const QJsonObject payload = qvariant_cast<QJsonObject>( arguments.at( 0 ) );
                if( extractSettingsFromPayload( payload, variable, states ) ) return true;
            }
            return false;
        },
        k_object_load_timeout_ms,
        50 );
}

void EnsembleTransferFunctionTest::compareStatisticState(
    const ClientHandles& client,
    int index,
    const StatisticState& expected ) const
{
    selectStatistic( client, index );
    QCOMPARE( client.ensemble_editor->selectedStatistic(), expected.statistic );
    QCOMPARE( client.user_min_max_radio->isChecked(), expected.use_user_range );
    QCOMPARE( client.user_min_spin_box->value(), expected.user_min );
    QCOMPARE( client.user_max_spin_box->value(), expected.user_max );
    QCOMPARE( client.color_map->getColors(), expected.colors );
    QCOMPARE( client.opacity_map->getOpacities(), expected.opacities );
}

void EnsembleTransferFunctionTest::restoreSettings(
    const ClientHandles& client,
    const QString& variable,
    const std::array<StatisticState, 3>& states ) const
{
    setLineEditText( client.variable_line_edit, variable );
    for( int index = 0; index < 3; ++index )
    {
        const StatisticState& state = states[static_cast<size_t>( index )];
        selectStatistic( client, index );
        client.user_min_spin_box->setValue( state.user_min );
        client.user_max_spin_box->setValue( state.user_max );
        if( state.use_user_range )
        {
            if( !client.user_min_max_radio->isChecked() )
            {
                QTest::mouseClick( client.user_min_max_radio, Qt::LeftButton );
            }
        }
        else if( !client.server_min_max_radio->isChecked() )
        {
            QTest::mouseClick( client.server_min_max_radio, Qt::LeftButton );
        }
        client.color_map->setColors( state.colors );
        client.opacity_map->setOpacities( state.opacities );
        client.color_map->update();
        client.opacity_map->update();
    }
    QCoreApplication::processEvents();
    for( int index = 0; index < 3; ++index )
    {
        compareStatisticState( client, index, states[static_cast<size_t>( index )] );
    }
    QCOMPARE( client.variable_line_edit->text(), variable );
}

QByteArray EnsembleTransferFunctionTest::particleFingerprint( const ClientHandles& client ) const
{
    QCryptographicHash hash( QCryptographicHash::Sha256 );
    QAbstractItemModel* model = client.object_tree_view->model();
    if( !model ) return QByteArray();

    int point_object_count = 0;
    for( int row = 0; row < model->rowCount(); ++row )
    {
        const QVariant value = model->index( row, 0 ).data( Qt::UserRole );
        if( !value.canConvert<ObjectInfoExtractor::ObjectInfo>() ) continue;

        const ObjectInfoExtractor::ObjectInfo info = value.value<ObjectInfoExtractor::ObjectInfo>();
        if( !info.isDisplay || !info.isEnsemble || !info.object ) continue;
        if( info.format != ObjectInfoExtractor::ClientServerPointObject &&
            info.format != ObjectInfoExtractor::InsituServerPointObject )
        {
            continue;
        }

        const auto* point_object = dynamic_cast<const kvs::PointObject*>( info.object );
        if( !point_object ) continue;

        ++point_object_count;
        hash.addData( info.uuid.data(), static_cast<qsizetype>( info.uuid.size() ) );
        const std::size_t vertex_count = point_object->numberOfVertices();
        hash.addData( reinterpret_cast<const char*>( &vertex_count ), sizeof( vertex_count ) );

        const auto add_values = [&hash]( const auto& values )
        {
            if( values.empty() ) return;
            hash.addData(
                reinterpret_cast<const char*>( values.data() ),
                static_cast<qsizetype>( values.byteSize() ) );
        };
        add_values( point_object->coords() );
        add_values( point_object->colors() );
        add_values( point_object->normals() );
        add_values( point_object->sizes() );
    }

    return point_object_count > 0 ? hash.result() : QByteArray();
}

// RequestDataAt 応答(EnsembleTransferFunctionParameter)が GUI の期待値と一致するかを判定する。
// 不一致のときは why に「どの統計のどの項目が」一致しないかを書き込む(診断用)。
bool EnsembleTransferFunctionTest::responseMatchesExpectedSettings(
    const QJsonObject& payload,
    const std::array<StatisticState, 3>& expected,
    const QString& expected_variable,
    QString* why ) const
{
    const auto fail = [why]( const QString& reason )
    {
        if( why ) *why = reason;
        return false;
    };

    const QJsonArray data = payload.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();
    if( data.size() != 3 ) return fail( QStringLiteral( "Data の要素数が 3 ではない(%1)" ).arg( data.size() ) );

    for( const StatisticState& state : expected )
    {
        QJsonObject patch;
        for( const QJsonValue& value : data )
        {
            const QJsonObject candidate = value.toObject();
            if( candidate.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString() == state.statistic )
            {
                patch = candidate;
                break;
            }
        }
        if( patch.isEmpty() ) return fail( QStringLiteral( "%1 のパッチが無い" ).arg( state.statistic ) );
        const QString variable = patch.value( QString::fromUtf8( Protocol::Key::EnsembleVariable ) ).toString();
        if( variable != expected_variable )
        {
            return fail( QStringLiteral( "%1 の Variable: 応答 `%2` / 期待 `%3`" ).arg( state.statistic, variable, expected_variable ) );
        }
        const int range_mode = patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode ) ).toInt();
        if( range_mode != ( state.use_user_range ? 1 : 2 ) )
        {
            return fail( QStringLiteral( "%1 のレンジモード: 応答 %2 / 期待 %3" ).arg( state.statistic ).arg( range_mode ).arg( state.use_user_range ? 1 : 2 ) );
        }
        const double user_min = patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMin ) ).toDouble();
        const double user_max = patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMax ) ).toDouble();
        if( std::abs( user_min - state.user_min ) > 1.0e-7 || std::abs( user_max - state.user_max ) > 1.0e-7 )
        {
            return fail( QStringLiteral( "%1 の User MinMax: 応答 %2 / %3、期待 %4 / %5" ).arg( state.statistic ).arg( user_min ).arg( user_max ).arg( state.user_min ).arg( state.user_max ) );
        }

        const QJsonValue server_min_value =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleServerRangeMin ) );
        const QJsonValue server_max_value =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleServerRangeMax ) );
        if( !server_min_value.isDouble() || !server_max_value.isDouble() )
        {
            return fail( QStringLiteral( "%1 の Server MinMax が数値ではない" ).arg( state.statistic ) );
        }
        const double server_min = server_min_value.toDouble();
        const double server_max = server_max_value.toDouble();
        if( !std::isfinite( server_min ) || !std::isfinite( server_max ) || server_min > server_max )
        {
            return fail( QStringLiteral( "%1 の Server MinMax が不正: %2 / %3" ).arg( state.statistic ).arg( server_min ).arg( server_max ) );
        }
        if( !isRoundedToEightDecimalPlaces( server_min ) ||
            !isRoundedToEightDecimalPlaces( server_max ) )
        {
            return fail( QStringLiteral( "%1 の Server MinMax が小数第 8 位に丸められていない: %2 / %3" ).arg( state.statistic ).arg( server_min, 0, 'g', 17 ).arg( server_max, 0, 'g', 17 ) );
        }

        const QJsonValue histogram = patch.value( QString::fromUtf8( Protocol::Key::EnsembleHistogram ) );
        if( !histogram.isArray() || histogram.toArray().isEmpty() )
        {
            return fail( QStringLiteral( "%1 の Histogram が無い(または空)" ).arg( state.statistic ) );
        }

        const QJsonArray colors = patch.value( QString::fromUtf8( Protocol::Key::EnsembleColorMap ) ).toArray();
        if( colors.size() != state.colors.size() )
        {
            return fail( QStringLiteral( "%1 の Color Map 要素数: 応答 %2 / 期待 %3" ).arg( state.statistic ).arg( colors.size() ).arg( state.colors.size() ) );
        }
        for( int index = 0; index < colors.size(); ++index )
        {
            const QJsonArray rgb = colors.at( index ).toArray();
            if( rgb.size() != 3 ||
                rgb.at( 0 ).toInt() != state.colors.at( index ).red() ||
                rgb.at( 1 ).toInt() != state.colors.at( index ).green() ||
                rgb.at( 2 ).toInt() != state.colors.at( index ).blue() )
            {
                return fail( QStringLiteral( "%1 の Color Map[%2] が不一致" ).arg( state.statistic ).arg( index ) );
            }
        }

        const QJsonArray opacities = patch.value( QString::fromUtf8( Protocol::Key::EnsembleOpacityMap ) ).toArray();
        if( opacities.size() != state.opacities.size() )
        {
            return fail( QStringLiteral( "%1 の Opacity Map 要素数: 応答 %2 / 期待 %3" ).arg( state.statistic ).arg( opacities.size() ).arg( state.opacities.size() ) );
        }
        for( int index = 0; index < opacities.size(); ++index )
        {
            if( std::abs( opacities.at( index ).toDouble() - state.opacities.at( index ) ) > 1.0e-6 )
            {
                return fail( QStringLiteral( "%1 の Opacity Map[%2]: 応答 %3 / 期待 %4" ).arg( state.statistic ).arg( index ).arg( opacities.at( index ).toDouble() ).arg( state.opacities.at( index ) ) );
            }
        }
    }
    if( why ) why->clear();
    return true;
}

// 不一致だった応答と期待値を診断用 JSON として保存する(基本シナリオの原因調査用)
void EnsembleTransferFunctionTest::writeMismatchDiagnostics(
    const QString& tag,
    int time_step,
    const QString& reason,
    const QJsonObject& payload,
    const std::array<StatisticState, 3>& expected,
    const QString& expected_variable ) const
{
    const QString dir = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "diag" ) );
    QDir().mkpath( dir );
    QJsonObject root;
    root.insert( QStringLiteral( "case" ), m_current_case_name );
    root.insert( QStringLiteral( "tag" ), tag );
    root.insert( QStringLiteral( "time_step" ), time_step );
    root.insert( QStringLiteral( "reason" ), reason );
    root.insert( QStringLiteral( "expected_variable" ), expected_variable );
    QJsonArray expected_array;
    for( const StatisticState& state : expected )
    {
        QJsonObject item;
        item.insert( QStringLiteral( "statistic" ), state.statistic );
        item.insert( QStringLiteral( "use_user_range" ), state.use_user_range );
        item.insert( QStringLiteral( "user_min" ), state.user_min );
        item.insert( QStringLiteral( "user_max" ), state.user_max );
        item.insert( QStringLiteral( "server_min" ), state.server_min );
        item.insert( QStringLiteral( "server_max" ), state.server_max );
        QJsonArray colors;
        for( const QColor& color : state.colors )
        {
            colors.append( QJsonArray{ color.red(), color.green(), color.blue() } );
        }
        QJsonArray opacities;
        for( float opacity : state.opacities ) opacities.append( static_cast<double>( opacity ) );
        item.insert( QStringLiteral( "colors" ), colors );
        item.insert( QStringLiteral( "opacities" ), opacities );
        expected_array.append( item );
    }
    root.insert( QStringLiteral( "expected" ), expected_array );
    root.insert( QStringLiteral( "response" ), payload );
    const QString file_name = QStringLiteral( "%1_%2.json" ).arg( m_current_case_name.isEmpty() ? QStringLiteral( "case" ) : m_current_case_name, tag );
    writeJsonObject( QDir( dir ).absoluteFilePath( file_name ), root );
}

// LatestTimeStep イベント(Communication::updateObjectLatestTimeStep)の UpdateMaxTimeStep を取り出す。
int EnsembleTransferFunctionTest::notifiedMaxTimeStepAt( int index ) const
{
    if( m_latest_step_spy == nullptr || index < 0 || index >= m_latest_step_spy->count() ) { return -1; }
    const QList<QVariant> arguments = m_latest_step_spy->at( index );
    if( arguments.isEmpty() ) { return -1; }
    const QJsonObject payload = qvariant_cast<QJsonObject>( arguments.at( 0 ) );
    const QString key = QString::fromUtf8(
        Protocol::Key::UpdateMaxTimeStep.data(), static_cast<int>( Protocol::Key::UpdateMaxTimeStep.size() ) );
    if( !payload.contains( key ) || !payload.value( key ).isDouble() ) { return -1; }
    return payload.value( key ).toInt();
}

int EnsembleTransferFunctionTest::latestNotifiedMaxTimeStep() const
{
    int latest = -1;
    if( m_latest_step_spy == nullptr ) { return latest; }
    for( int index = 0; index < m_latest_step_spy->count(); ++index )
    {
        latest = std::max( latest, notifiedMaxTimeStepAt( index ) );
    }
    return latest;
}

bool EnsembleTransferFunctionTest::waitForAppliedParticles(
    const ClientHandles& client,
    QSignalSpy& request_spy,
    QSignalSpy& completion_spy,
    QSignalSpy& request_parameter_spy,
    const std::array<StatisticState, 3>& expected,
    const QString& expected_variable,
    bool require_particle_change,
    ParticleUpdateResult* result )
{
    m_last_wait_failure.clear();
    if( result == nullptr || result->before_fingerprint.isEmpty() )
    {
        m_last_wait_failure = QStringLiteral( "Apply 前の粒子(アンサンブル PointObject)が ObjectEditor に無い" );
        return false;
    }
    if( !client.keep_last_button->isChecked() )
    {
        m_last_wait_failure = QStringLiteral( "Apply 前に Keep Last が OFF" );
        return false;
    }
    QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton );
    QTest::qWait( k_apply_propagation_wait_ms );
    if( !client.keep_last_button->isChecked() )
    {
        m_last_wait_failure = QStringLiteral( "Apply 後に Keep Last が OFF になった" );
        return false;
    }

    // Apply 後に最初に通知された最新ステップ M'(LatestTimeStep: UpdateMaxTimeStep)を基準に、判定対象のしきい値を決める。
    // M'+2 以上のステップだけを「Apply 後の設定で生成された」とみなし、それ未満のステップの応答は判定に使わない。
    // 通知がまだ無い場合はしきい値を無効(-1)にして従来どおり全応答を判定対象とする。
    const int notified_after_apply = latestNotifiedMaxTimeStep();
    const int threshold_step =
        notified_after_apply >= 0 ? notified_after_apply + k_ensemble_generation_margin_steps : -1;
    m_last_post_apply_threshold = threshold_step;

    request_spy.clear();
    completion_spy.clear();
    request_parameter_spy.clear();

    // 不一致が続いた場合の理由(最後に観測した応答の不一致内容)。タイムアウト時にこちらを優先して報告する。
    QString last_mismatch;
    bool first_mismatch_written = false;
    QJsonObject last_mismatch_payload;
    int last_mismatch_step = -1;
    int below_threshold_count = 0;   // しきい値未満だった応答の回数(タイムアウト報告用)
    int max_request_step_seen = -1;  // 観測した最大の要求ステップ(タイムアウト報告用)

    QElapsedTimer timeout;
    timeout.start();
    while( timeout.elapsed() < k_data_request_timeout_ms )
    {
        const int request_start = request_spy.count();
        const int parameter_start = request_parameter_spy.count();
        const int completion_start = completion_spy.count();
        const int remaining = k_data_request_timeout_ms - static_cast<int>( timeout.elapsed() );
        if( !waitForCondition( [&request_spy, request_start]() { return request_spy.count() > request_start; }, remaining, 50 ) )
        {
            if( !last_mismatch.isEmpty() )
            {
                m_last_wait_failure = QStringLiteral( "%1 秒間に %2 回の応答を確認したが一致しないままタイムアウト。最後の応答(time step %3): %4" )
                                          .arg( k_data_request_timeout_ms / 1000 ).arg( result->request_count ).arg( last_mismatch_step ).arg( last_mismatch );
                if( !last_mismatch_payload.isEmpty() )
                {
                    writeMismatchDiagnostics( QStringLiteral( "mismatch_last" ), last_mismatch_step, last_mismatch, last_mismatch_payload, expected, expected_variable );
                }
            }
            else if( threshold_step >= 0 && below_threshold_count > 0 )
            {
                m_last_wait_failure = QStringLiteral( "Apply 後に生成されたステップ(しきい値 time step >= %1、M'=%2)を %3 秒以内に受信しない。しきい値未満の応答のみ %4 回(最大要求ステップ %5)。Keep Last が最新ステップへ更新しているか、シミュレーションが新しいステップを出力しているか確認する" )
                                          .arg( threshold_step ).arg( notified_after_apply ).arg( k_data_request_timeout_ms / 1000 ).arg( below_threshold_count ).arg( max_request_step_seen );
            }
            else
            {
                m_last_wait_failure = QStringLiteral( "Keep Last の requestDataAt が %1 秒以内に発生しない(シミュレーションが新しいステップを出力していない可能性。確認済み要求数 %2)" )
                                          .arg( k_data_request_timeout_ms / 1000 ).arg( result->request_count );
            }
            return false;
        }

        const int request_time_step = request_spy.at( request_start ).at( 0 ).toInt();
        ++result->request_count;
        max_request_step_seen = std::max( max_request_step_seen, request_time_step );

        // Apply 後に生成されたことが保証されるステップ(しきい値以上)だけを判定対象にする。
        // しきい値未満のステップは Apply 前の設定で生成されている可能性があるため、応答を待たずに次の更新を待つ。
        if( threshold_step >= 0 && request_time_step < threshold_step )
        {
            ++below_threshold_count;
            continue;
        }

        const int request_remaining = k_data_request_timeout_ms - static_cast<int>( timeout.elapsed() );
        const bool response_completed = waitForCondition(
            [&request_parameter_spy, &completion_spy, parameter_start, completion_start, request_time_step]()
            {
                bool has_parameter = request_parameter_spy.count() > parameter_start;
                bool has_completion = false;
                for( int index = completion_start; index < completion_spy.count(); ++index )
                {
                    if( completion_spy.at( index ).at( 0 ).toInt() == request_time_step )
                    {
                        has_completion = true;
                        break;
                    }
                }
                return has_parameter && has_completion;
            },
            request_remaining,
            50 );
        if( !response_completed )
        {
            m_last_wait_failure = QStringLiteral( "time step %1 の RequestDataAt 応答(EnsembleTransferFunctionParameter / dataRequestCompleted)が届かない" ).arg( request_time_step );
            return false;
        }

        bool settings_match = false;
        QString mismatch_reason;
        QJsonObject mismatch_payload;
        for( int index = parameter_start; index < request_parameter_spy.count(); ++index )
        {
            const QJsonObject payload = qvariant_cast<QJsonObject>( request_parameter_spy.at( index ).at( 0 ) );
            QString why;
            if( responseMatchesExpectedSettings( payload, expected, expected_variable, &why ) )
            {
                settings_match = true;
                m_last_request_payload = payload;
                m_last_request_time_step = request_time_step;
                break;
            }
            mismatch_reason = why;
            mismatch_payload = payload;
        }

        const QByteArray fingerprint = particleFingerprint( client );
        if( fingerprint.isEmpty() )
        {
            m_last_wait_failure = QStringLiteral( "time step %1 受信後にアンサンブル PointObject が ObjectEditor に無い" ).arg( request_time_step );
            return false;
        }
        if( !settings_match || ( require_particle_change && fingerprint == result->before_fingerprint ) )
        {
            last_mismatch = settings_match
                ? QStringLiteral( "RequestDataAt 応答は GUI と一致したが粒子フィンガープリントが Apply 前から変化しない" )
                : QStringLiteral( "RequestDataAt 応答が GUI と不一致(%1)" ).arg( mismatch_reason );
            last_mismatch_step = request_time_step;
            last_mismatch_payload = settings_match ? m_last_request_payload : mismatch_payload;
            m_last_wait_failure = QStringLiteral( "time step %1: %2(タイムアウトまで次ステップを待機)" ).arg( request_time_step ).arg( last_mismatch );
            if( !first_mismatch_written && !last_mismatch_payload.isEmpty() )
            {
                writeMismatchDiagnostics( QStringLiteral( "mismatch_first" ), request_time_step, last_mismatch, last_mismatch_payload, expected, expected_variable );
                first_mismatch_written = true;
            }
            continue;
        }

        result->request_time_step = request_time_step;
        result->after_fingerprint = fingerprint;
        waitForPositiveParticleCount( client );

        const int paint_count_before_update = m_render_paint_count;
        client.render_screen->update();
        result->paint_completed = waitForCondition(
            [this, paint_count_before_update]() { return m_render_paint_count > paint_count_before_update; },
            k_data_request_timeout_ms,
            20 );
        if( !result->paint_completed )
        {
            m_last_wait_failure = QStringLiteral( "Render Screen の再描画が確認できない" );
            return false;
        }
        if( !client.keep_last_button->isChecked() )
        {
            m_last_wait_failure = QStringLiteral( "粒子受信後に Keep Last が OFF になった" );
            return false;
        }
        return true;
    }

    if( m_last_wait_failure.isEmpty() ) m_last_wait_failure = QStringLiteral( "タイムアウト(%1 秒)" ).arg( k_data_request_timeout_ms / 1000 );
    else m_last_wait_failure += QStringLiteral( " → %1 秒でタイムアウト" ).arg( k_data_request_timeout_ms / 1000 );
    return false;
}

bool EnsembleTransferFunctionTest::applyAndCaptureCase(
    const ClientHandles& client,
    QSignalSpy& request_spy,
    QSignalSpy& completion_spy,
    QSignalSpy& request_parameter_spy,
    int statistic_index,
    const QString& variable,
    const QString& file_name_prefix,
    const QString& title,
    const QString& change_description,
    bool require_particle_change )
{
    m_current_case_name = file_name_prefix;
    const std::array<StatisticState, 3> expected = captureAllStatisticStates( client );
    selectStatistic( client, statistic_index );
    ParticleUpdateResult update_result;
    update_result.before_fingerprint = particleFingerprint( client );
    if( !waitForAppliedParticles(
        client,
        request_spy,
        completion_spy,
        request_parameter_spy,
        expected,
        variable,
        require_particle_change,
        &update_result ) )
    {
        return false;
    }
    if( !update_result.paint_completed || update_result.after_fingerprint.isEmpty() )
    {
        m_last_wait_failure = QStringLiteral( "粒子置換後の再描画が確認できない(撮影をスキップ)" );
        return false;
    }
    saveScreenshotPair( client, file_name_prefix, title, change_description, update_result );
    if( !client.keep_last_button->isChecked() )
    {
        m_last_wait_failure = QStringLiteral( "撮影後に Keep Last が OFF になっている" );
        return false;
    }
    return true;
}

QFileDialog* EnsembleTransferFunctionTest::waitForFileDialog( int timeout_ms ) const
{
    QFileDialog* dialog = nullptr;
    const bool found = waitForCondition(
        [&dialog]()
        {
            for( QWidget* widget : QApplication::topLevelWidgets() )
            {
                auto* candidate = qobject_cast<QFileDialog*>( widget );
                if( candidate && candidate->isVisible() )
                {
                    dialog = candidate;
                    return true;
                }
            }
            return false;
        },
        timeout_ms,
        50 );
    return found ? dialog : nullptr;
}

void EnsembleTransferFunctionTest::selectFileFromDialog(
    const QString& file_path,
    bool require_existing_file ) const
{
    QFileDialog* dialog = waitForFileDialog( k_dialog_timeout_ms );
    QVERIFY2( dialog != nullptr, "File dialog was not shown" );

    const QFileInfo file_info( file_path );
    QVERIFY2( QDir().mkpath( file_info.absolutePath() ), "Failed to create file-dialog directory" );
    if( require_existing_file )
    {
        QVERIFY2( file_info.exists(), qPrintable( QStringLiteral( "Import file not found: %1" ).arg( file_path ) ) );
    }

    dialog->setDirectory( file_info.absolutePath() );
    QCoreApplication::processEvents();
    auto* file_name_edit = dialog->findChild<QLineEdit*>( QStringLiteral( "fileNameEdit" ) );
    if( !file_name_edit )
    {
        const auto edits = dialog->findChildren<QLineEdit*>();
        for( QLineEdit* edit : edits )
        {
            if( edit && edit->isVisible() )
            {
                file_name_edit = edit;
                break;
            }
        }
    }
    QVERIFY2( file_name_edit != nullptr, "QFileDialog fileNameEdit not found" );
    file_name_edit->setText( file_info.fileName() );
    QMetaObject::invokeMethod( dialog, "accept", Qt::QueuedConnection );
}

void EnsembleTransferFunctionTest::exportSettings(
    const ClientHandles& client,
    const QString& file_path ) const
{
    if( QFileInfo::exists( file_path ) )
    {
        QVERIFY2( QFile::remove( file_path ), "Failed to remove previous export JSON" );
    }
    bringEditorToFront( client.ensemble_editor );
    QTimer::singleShot( 0, [this, file_path]() { selectFileFromDialog( file_path, false ); } );
    QTest::mouseClick( client.ensemble_export_button, Qt::LeftButton );
    QVERIFY2(
        waitForCondition( [file_path]() { return QFileInfo::exists( file_path ); }, k_dialog_timeout_ms, 50 ),
        qPrintable( QStringLiteral( "Export JSON was not created: %1" ).arg( file_path ) ) );
}

void EnsembleTransferFunctionTest::importSettings(
    const ClientHandles& client,
    const QString& file_path ) const
{
    bringEditorToFront( client.ensemble_editor );
    QTimer::singleShot( 0, [this, file_path]() { selectFileFromDialog( file_path, true ); } );
    QTest::mouseClick( client.ensemble_import_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void EnsembleTransferFunctionTest::validateExportedJson(
    const QString& file_path,
    const QString& expected_variable,
    const std::array<StatisticState, 3>& expected ) const
{
    QFile file( file_path );
    QVERIFY2( file.open( QIODevice::ReadOnly | QIODevice::Text ), "Failed to open Export JSON" );
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson( file.readAll(), &error );
    QCOMPARE( error.error, QJsonParseError::NoError );
    QVERIFY2( document.isObject(), "Export JSON root is not an object" );

    const QJsonObject root = document.object();
    QCOMPARE( root.value( "format" ).toString(), QStringLiteral( "PBVR readable transfer function settings" ) );
    QCOMPARE( root.value( "format_version" ).toInt(), 1 );
    const QJsonObject transfer_settings =
        root.value( "settings" ).toObject().value( "transfer_function" ).toObject();
    QCOMPARE( transfer_settings.value( "transfer_function_count" ).toInt(), 1 );
    QCOMPARE( transfer_settings.value( "transfer_function_resolution" ).toInt(), 256 );

    const std::array<QString, 3> section_names = {
        QStringLiteral( "mean_transfer_functions" ),
        QStringLiteral( "variance_transfer_functions" ),
        QStringLiteral( "coefficient_of_variation_transfer_functions" ) };
    const QJsonArray normal = root.value( "transfer_functions" ).toArray();
    const QJsonArray mean = root.value( section_names[0] ).toArray();
    QCOMPARE( normal.size(), 1 );
    QCOMPARE( mean.size(), 1 );
    QCOMPARE( normal.first().toObject(), mean.first().toObject() );

    for( int index = 0; index < 3; ++index )
    {
        const QJsonArray section = root.value( section_names[static_cast<size_t>( index )] ).toArray();
        QCOMPARE( section.size(), 1 );
        const QJsonObject tf = section.first().toObject();
        const QJsonObject color = tf.value( "color" ).toObject();
        const QJsonObject opacity = tf.value( "opacity" ).toObject();
        QCOMPARE( color.value( "variable" ).toString(), expected_variable );
        QCOMPARE( opacity.value( "variable" ).toString(), expected_variable );

        const QJsonObject color_range = color.value( "range" ).toObject();
        QCOMPARE( color_range.value( "active_range" ).toString(), QStringLiteral( "user" ) );
        QVERIFY2( !color_range.contains( "server" ), "Exported color range contains Server MinMax" );
        const QJsonObject color_user = color_range.value( "user" ).toObject();
        QCOMPARE( color_user.value( "min" ).toDouble(), expected[static_cast<size_t>( index )].user_min );
        QCOMPARE( color_user.value( "max" ).toDouble(), expected[static_cast<size_t>( index )].user_max );

        const QJsonObject opacity_range = opacity.value( "range" ).toObject();
        QCOMPARE( opacity_range.value( "active_range" ).toString(), QStringLiteral( "user" ) );
        QVERIFY2( !opacity_range.contains( "server" ), "Exported opacity range contains Server MinMax" );
        const QJsonObject opacity_user = opacity_range.value( "user" ).toObject();
        QCOMPARE( opacity_user.value( "min" ).toDouble(), expected[static_cast<size_t>( index )].user_min );
        QCOMPARE( opacity_user.value( "max" ).toDouble(), expected[static_cast<size_t>( index )].user_max );

        const QJsonObject color_map = color.value( "map" ).toObject();
        const QJsonArray color_values = color_map.value( "values" ).toArray();
        QCOMPARE( color_values.size(), expected[static_cast<size_t>( index )].colors.size() * 3 );
        QCOMPARE( color_map.value( "length" ).toInt(), color_values.size() );
        for( int i = 0; i < expected[static_cast<size_t>( index )].colors.size(); ++i )
        {
            const QColor expected_color = expected[static_cast<size_t>( index )].colors.at( i );
            QCOMPARE( color_values.at( i * 3 ).toInt(), expected_color.red() );
            QCOMPARE( color_values.at( i * 3 + 1 ).toInt(), expected_color.green() );
            QCOMPARE( color_values.at( i * 3 + 2 ).toInt(), expected_color.blue() );
        }

        const QJsonObject opacity_map = opacity.value( "map" ).toObject();
        const QJsonArray opacity_values = opacity_map.value( "values" ).toArray();
        QCOMPARE( opacity_values.size(), expected[static_cast<size_t>( index )].opacities.size() );
        QCOMPARE( opacity_map.value( "length" ).toInt(), opacity_values.size() );
        for( int i = 0; i < opacity_values.size(); ++i )
        {
            QCOMPARE(
                static_cast<float>( opacity_values.at( i ).toDouble() ),
                expected[static_cast<size_t>( index )].opacities.at( i ) );
        }
    }
}

void EnsembleTransferFunctionTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_output_dir_path = datedTestOutputDir(
        repoRootPath(),
        date_stamp,
        QStringLiteral( "EnsembleTransferFunctionTest" ) );
    m_screenshot_dir_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_export_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "ensemble_transfer_function.json" ) );
    m_manual_dir_path = QDir( m_screenshot_dir_path ).absoluteFilePath( QStringLiteral( "manual" ) );
    m_item_export_path = configuredPathOrEmpty( "ENSEMBLE_TF_EXPORT_TEST" );
    if( m_item_export_path.isEmpty() )
    {
        m_item_export_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "ensemble_tf_export_C-ETF-12.json" ) );
    }
    m_vis_param_dir = configuredPathOrEmpty( "ENSEMBLE_VIS_PARAM_DIR" );
    if( !m_vis_param_dir.isEmpty() && !QDir( m_vis_param_dir ).exists() )
    {
        qWarning().noquote() << QStringLiteral( "ENSEMBLE_VIS_PARAM_DIR does not exist; server-side file checks are skipped: %1" ).arg( m_vis_param_dir );
        m_vis_param_dir.clear();
    }
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), "Failed to create EnsembleTransferFunctionTest output directory" );
    QVERIFY2( QDir().mkpath( m_manual_dir_path ), "Failed to create manual screenshot directory" );

    QFile report( m_report_path );
    QVERIFY2( report.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ), "Failed to initialize TestResult.md" );
    QTextStream stream( &report );
    stream << "# EnsembleTransferFunctionTest\n\n- 結果: RUNNING\n";
}

void EnsembleTransferFunctionTest::cleanupTestCase()
{
    writeMarkdownReport();
}

void EnsembleTransferFunctionTest::performs_ensemble_transfer_function_scenario()
{
    if( !g_ensemble_test_app ) g_ensemble_test_app = pbvrTestApplication();
    QVERIFY2( g_ensemble_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_ensemble_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );
    const ClientHandles client = resolveClientHandles( main_window );
    m_render_screen_object = client.render_screen;
    m_render_paint_count = 0;
    client.render_screen->installEventFilter( this );
    client.communication->show();
    client.object_editor->show();

    QSignalSpy request_spy( client.time_step_tool_bar, &TimeStepControlToolBar::requestDataAt );
    QSignalSpy completion_spy( client.time_step_tool_bar, &TimeStepControlToolBar::dataRequestCompleted );
    QSignalSpy parameter_spy( client.communication, &Communication::receiveEnsembleStatisticsParameter );
    QSignalSpy request_parameter_spy(
        client.communication,
        &Communication::receiveRequestDataAtEnsembleStatisticsParameter );
    QVERIFY2( request_spy.isValid(), "requestDataAt QSignalSpy is invalid" );
    QVERIFY2( completion_spy.isValid(), "dataRequestCompleted QSignalSpy is invalid" );
    QVERIFY2( parameter_spy.isValid(), "EnsembleStatisticsParameter QSignalSpy is invalid" );
    QVERIFY2( request_parameter_spy.isValid(), "RequestDataAt EnsembleStatisticsParameter QSignalSpy is invalid" );
    QSignalSpy selected_file_spy( client.communication, &Communication::receiveSelectedFile );
    QVERIFY2( selected_file_spy.isValid(), "SelectedFile QSignalSpy is invalid" );
    QSignalSpy latest_step_spy( client.communication, &Communication::updateObjectLatestTimeStep );
    QVERIFY2( latest_step_spy.isValid(), "updateObjectLatestTimeStep QSignalSpy is invalid" );
    m_latest_step_spy = &latest_step_spy;

    connectClient( client );
    configureInSituVisualization( client );
    QString initial_variable;
    std::array<StatisticState, 3> baseline_states;
    QVERIFY2(
        waitForInitialSettings( parameter_spy, &initial_variable, &baseline_states ),
        "Initial EnsembleStatisticsParameter with three valid statistic patches was not received" );
    m_baseline_variable = initial_variable;
    m_baseline_states = baseline_states;
    m_has_baseline_settings = true;
    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "サーバへ接続し、In-Situ アンサンブルオブジェクトを選択して Apply しました。" ) );

    ItemContext ctx;
    ctx.client = client;
    ctx.request_spy = &request_spy;
    ctx.completion_spy = &completion_spy;
    ctx.parameter_spy = &parameter_spy;
    ctx.request_parameter_spy = &request_parameter_spy;
    ctx.selected_file_spy = &selected_file_spy;
    ctx.baseline_variable = initial_variable;
    ctx.baseline_states = baseline_states;
    runItemMenuBar15( ctx, main_window );

    generateInitialParticles( client, completion_spy );
    enableKeepLast( client );
    markStepCompleted( QStringLiteral( "初期 Jump で粒子を生成し、Keep Last を ON にしました。" ) );

    applyVolumeRotationX( client, -30.0 );
    markStepCompleted( QStringLiteral( "初期粒子生成後、Volume TransformのX軸回転を-30度に設定しました。" ) );

    QVERIFY2(
        waitForCondition(
            [&main_window]()
            {
                for( QAction* action : main_window.findChildren<QAction*>() )
                {
                    if( action->text().contains( QStringLiteral( "Ensemble Transfer Function Editor" ) ) )
                    {
                        return action->isEnabled();
                    }
                }
                return false;
            },
            k_object_load_timeout_ms,
            100 ),
        "Ensemble Transfer Function Editor action did not become enabled" );
    runItemColorMapSelector02( ctx );
    bringEditorToFront( client.ensemble_editor );
    runItemEditorOpened15_2( ctx );

    const QString changed_variable =
        initial_variable == QStringLiteral( "q1*2" ) ? QStringLiteral( "q1" ) : QStringLiteral( "q1*2" );
    restoreSettings( client, initial_variable, baseline_states );

    const std::array<QString, 3> presets = {
        QStringLiteral( "Traffic Lights Step" ),
        QStringLiteral( "Grayscale" ),
        QStringLiteral( "Rainbow Uniform" ) };
    const std::array<QString, 3> opacity_expressions = {
        QStringLiteral( "0.5 * x" ),
        QStringLiteral( "0.5 * x" ),
        QStringLiteral( "0.5 * x" ) };
    const std::array<double, 3> user_min_values = { 0.0, 0.0, 0.0 };
    const std::array<double, 3> user_max_values = { 122.0, 122.0, 2.5 };
    const std::array<QString, 3> statistic_titles = {
        QStringLiteral( "Average" ),
        QStringLiteral( "Variance" ),
        QStringLiteral( "Coefficient of Variation" ) };
    const std::array<QString, 3> statistic_file_names = {
        QStringLiteral( "average" ),
        QStringLiteral( "variance" ),
        QStringLiteral( "coefficient_variation" ) };

    for( int index = 0; index < 3; ++index )
    {
        const StatisticState& baseline = baseline_states[static_cast<size_t>( index )];
        const QString statistic_title = statistic_titles[static_cast<size_t>( index )];
        const QString file_name = statistic_file_names[static_cast<size_t>( index )];

        restoreSettings( client, initial_variable, baseline_states );
        {
            // 引数の評価順序に依存しないよう、先にケースを実行してから結果と理由を記録する
            const bool case_ok = applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_unchanged" ),
            statistic_title + QStringLiteral( " — 変更なし" ),
            QStringLiteral( "初回EnsembleStatisticsParameterで受信した基準状態（変更なし）" ),
            index != 0 );
            recordBaseCase( statistic_title + QStringLiteral( " — 変更なし" ), case_ok, QStringLiteral( "Unchanged case did not receive matching RequestDataAt settings and particles" ) + QStringLiteral( ": " ) + m_last_wait_failure );
        }

        restoreSettings( client, initial_variable, baseline_states );
        setLineEditText( client.variable_line_edit, changed_variable );
        {
            // 引数の評価順序に依存しないよう、先にケースを実行してから結果と理由を記録する
            const bool case_ok = applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            changed_variable,
            file_name + QStringLiteral( "_variable" ),
            statistic_title + QStringLiteral( " — Variable" ),
            QStringLiteral( "Variableのみ `%1` から `%2` へ変更" ).arg( initial_variable, changed_variable ) );
            recordBaseCase( statistic_title + QStringLiteral( " — Variable" ), case_ok, QStringLiteral( "Variable case did not receive matching RequestDataAt settings and changed particles" ) + QStringLiteral( ": " ) + m_last_wait_failure );
        }

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        const double user_min = user_min_values[static_cast<size_t>( index )];
        const double user_max = user_max_values[static_cast<size_t>( index )];
        setUserRange( client, user_min, user_max );
        const bool user_minmax_differs =
            std::abs( user_min - baseline.user_min ) > 1.0e-7 ||
            std::abs( user_max - baseline.user_max ) > 1.0e-7 ||
            !baseline.use_user_range;
        if( !user_minmax_differs )
        {
            recordBaseCase( statistic_title + QStringLiteral( " — User MinMax" ), false,
                            QStringLiteral( "User MinMax の設定値が基準状態と同じで変更ケースにならない" ) );
        }
        else
        {
            // 引数の評価順序に依存しないよう、先にケースを実行してから結果と理由を記録する
            const bool case_ok = applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_user_minmax" ),
            statistic_title + QStringLiteral( " — User MinMax" ),
            QStringLiteral( "User MinMaxのみ %1 / %2 へ変更" ).arg( user_min ).arg( user_max ) );
            recordBaseCase( statistic_title + QStringLiteral( " — User MinMax" ), case_ok, QStringLiteral( "User MinMax case did not receive matching RequestDataAt settings and changed particles" ) + QStringLiteral( ": " ) + m_last_wait_failure );
        }

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        if( !baseline.use_user_range )
        {
            recordBaseCase( statistic_title + QStringLiteral( " — Server MinMax" ), false,
                            QStringLiteral( "基準状態が既に Server MinMax のため変更ケースにならない" ) );
        }
        else
        {
        setServerRange( client );
        {
            // 引数の評価順序に依存しないよう、先にケースを実行してから結果と理由を記録する
            const bool case_ok = applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_server_minmax" ),
            statistic_title + QStringLiteral( " — Server MinMax" ),
            QStringLiteral( "MinMaxモードのみServerへ変更（%1 / %2）" ).arg( baseline.server_min ).arg( baseline.server_max ) );
            recordBaseCase( statistic_title + QStringLiteral( " — Server MinMax" ), case_ok, QStringLiteral( "Server MinMax case did not receive matching RequestDataAt settings and changed particles" ) + QStringLiteral( ": " ) + m_last_wait_failure );
        }
        }

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        QString applied_preset = presets[static_cast<size_t>( index )];
        applyColorPreset( client, applied_preset );
        if( client.color_map->getColors() == baseline.colors )
        {
            applied_preset = applied_preset == QStringLiteral( "Traffic Lights Step" )
                ? QStringLiteral( "Grayscale" )
                : QStringLiteral( "Traffic Lights Step" );
            applyColorPreset( client, applied_preset );
        }
        if( client.color_map->getColors() == baseline.colors )
        {
            recordBaseCase( statistic_title + QStringLiteral( " — Color Map" ), false,
                            QStringLiteral( "Color Map プリセットを適用しても基準状態と同じ" ) );
        }
        else
        {
            // 引数の評価順序に依存しないよう、先にケースを実行してから結果と理由を記録する
            const bool case_ok = applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_color_map" ),
            statistic_title + QStringLiteral( " — Color Map" ),
            QStringLiteral( "Color Mapのみプリセット `%1` へ変更" ).arg( applied_preset ) );
            recordBaseCase( statistic_title + QStringLiteral( " — Color Map" ), case_ok, QStringLiteral( "Color Map case did not receive matching RequestDataAt settings and changed particles" ) + QStringLiteral( ": " ) + m_last_wait_failure );
        }

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        // 前回テストの Apply 値がサーバに残っていると基準状態が既に `0.5 * x` のことがあるため、
        // 基準と同じになった場合は別の式(`1 - x`)に切り替える(Color Map のプリセット切替と同じ扱い)
        QString applied_opacity_expression = opacity_expressions[static_cast<size_t>( index )];
        applyOpacityExpression( client, applied_opacity_expression );
        if( opacitiesNearlyEqual( client.opacity_map->getOpacities(), baseline.opacities ) )
        {
            applied_opacity_expression = QStringLiteral( "1 - x" );
            applyOpacityExpression( client, applied_opacity_expression );
        }
        if( opacitiesNearlyEqual( client.opacity_map->getOpacities(), baseline.opacities ) )
        {
            recordBaseCase( statistic_title + QStringLiteral( " — Opacity Map" ), false,
                            QStringLiteral( "Opacity Map の式(0.5 * x / 1 - x)を適用しても基準状態と同じ" ) );
        }
        else
        {
            // 引数の評価順序に依存しないよう、先にケースを実行してから結果と理由を記録する
            const bool case_ok = applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_opacity_map" ),
            statistic_title + QStringLiteral( " — Opacity Map" ),
            QStringLiteral( "Opacity Mapのみ式 `%1` へ変更" ).arg( applied_opacity_expression ) );
            recordBaseCase( statistic_title + QStringLiteral( " — Opacity Map" ), case_ok, QStringLiteral( "Opacity Map case did not receive matching RequestDataAt settings and changed particles" ) + QStringLiteral( ": " ) + m_last_wait_failure );
        }
    }
    markStepCompleted(
        QStringLiteral( "3統計について変更なしと5項目の単独変更をApplyし、粒子表示とEditorを18組撮影しました。" ) );

    restoreSettings( client, changed_variable, baseline_states );
    for( int index = 0; index < 3; ++index )
    {
        selectStatistic( client, index );
        const double user_min = user_min_values[static_cast<size_t>( index )];
        const double user_max = user_max_values[static_cast<size_t>( index )];
        setUserRange( client, user_min, user_max );
        applyColorPreset( client, presets[static_cast<size_t>( index )] );
        applyOpacityExpression( client, opacity_expressions[static_cast<size_t>( index )] );
    }
    const std::array<StatisticState, 3> exported_states = captureAllStatisticStates( client );
    const std::array<StatisticState, 3> imported_states = exported_states;
    m_import_variable = changed_variable;
    m_import_states = imported_states;
    m_has_import_settings = true;
    exportSettings( client, m_export_path );
    validateExportedJson( m_export_path, changed_variable, exported_states );
    QVERIFY2( client.keep_last_button->isChecked(), "Keep Last was disabled during Export" );
    markStepCompleted( QStringLiteral( "3統計を JSON へ Export し、ファイル構造と値を検証しました。" ) );

    const QString mutated_variable = changed_variable == QStringLiteral( "q1" ) ? QStringLiteral( "q1*2" ) : QStringLiteral( "q1" );
    setLineEditText( client.variable_line_edit, mutated_variable );
    for( int index = 0; index < 3; ++index )
    {
        selectStatistic( client, index );
        setUserRange(
            client,
            exported_states[static_cast<size_t>( index )].user_min + 0.25,
            exported_states[static_cast<size_t>( index )].user_max + 0.5 );
    }
    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "KVS Default" ) );
    selectStatistic( client, 1 );
    applyOpacityExpression( client, QStringLiteral( "1-x" ) );
    selectStatistic( client, 1 );
    const int selected_before_import = client.statistic_combo_box->currentIndex();

    importSettings( client, m_export_path );
    QCOMPARE( client.statistic_combo_box->currentIndex(), selected_before_import );
    QCOMPARE( client.variable_line_edit->text(), changed_variable );
    for( int index = 0; index < 3; ++index )
    {
        compareStatisticState( client, index, imported_states[static_cast<size_t>( index )] );
    }
    QVERIFY2( client.keep_last_button->isChecked(), "Keep Last was disabled during Import" );
    markStepCompleted( QStringLiteral( "GUI 値を変更後に Export JSON を Import し、3統計の値が復元されることを確認しました。" ) );

    selectStatistic( client, selected_before_import );
    ParticleUpdateResult import_update_result;
    import_update_result.before_fingerprint = particleFingerprint( client );
    m_current_case_name = QStringLiteral( "import_restored" );
    const bool import_applied = waitForAppliedParticles(
        client,
        request_spy,
        completion_spy,
        request_parameter_spy,
        imported_states,
        changed_variable,
        true,
        &import_update_result );
    const bool import_confirmed =
        import_applied && import_update_result.paint_completed && !import_update_result.after_fingerprint.isEmpty();
    recordBaseCase(
        QStringLiteral( "Import 復元値の Apply" ),
        import_confirmed,
        QStringLiteral( "Import case did not receive matching RequestDataAt settings and changed particles: " ) + m_last_wait_failure );
    if( import_confirmed )
    {
        saveScreenshotPair(
            client,
            QStringLiteral( "import_restored" ),
            QStringLiteral( "Import復元後" ),
            QStringLiteral( "Export JSONに記録した3統計の設定をImportして復元" ),
            import_update_result );
        markStepCompleted( QStringLiteral( "Import 復元値を Apply し、5秒待機後の粒子生成を確認しました。" ) );
    }

    // ---- チェックシート項目(C-ETF-01〜19 / C-ETF-16 / C-ETF-19)----
    runCheckSheetItems( ctx, main_window );
    bool any_item_failed = false;
    for( const ItemResult& item : m_item_results )
    {
        if( item.status == QStringLiteral( "FAIL" ) ) any_item_failed = true;
    }
    for( const ItemResult& item : m_base_results )
    {
        if( item.status == QStringLiteral( "FAIL" ) ) any_item_failed = true;
    }
    markStepCompleted( QStringLiteral( "チェックシート項目 %1 件を実施しました(FAIL: %2)。" )
                           .arg( m_item_results.size() )
                           .arg( any_item_failed ? QStringLiteral( "あり" ) : QStringLiteral( "なし" ) ) );

    QVERIFY2( client.keep_last_button->isChecked(), "Keep Last was not maintained until the end of the test" );
    m_test_succeeded = !any_item_failed;
    client.render_screen->removeEventFilter( this );
    m_render_screen_object = nullptr;
    m_latest_step_spy = nullptr; // スコープを抜ける latest_step_spy への参照を残さない
    main_window.close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    QVERIFY2( !any_item_failed, "One or more base-scenario cases or check-sheet items failed. See TestResult.md" );
}

// ===========================================================================
// チェックシート項目(C-ETF-01〜19 / C-MB-15 / C-CMS-02)用ヘルパー
// ===========================================================================

namespace
{
constexpr int k_warning_timeout_ms = 20000;
constexpr int k_no_send_wait_ms = 5000;
constexpr int k_no_warning_wait_ms = 1500;

const QString k_manual_root = QStringLiteral( "Client/EnsembleTransferFunctionEditor/" );

QString manualEtf( int item, int seq )
{
    return k_manual_root + QStringLiteral( "%1_%2.png" ).arg( item ).arg( seq );
}

QJsonObject removeSection( QJsonObject root, const QString& section )
{
    root.remove( section );
    return root;
}

QJsonObject removeVarianceColorRange( QJsonObject root )
{
    QJsonArray section = root.value( QStringLiteral( "variance_transfer_functions" ) ).toArray();
    if( section.isEmpty() ) return root;
    QJsonObject tf = section.first().toObject();
    QJsonObject color = tf.value( QStringLiteral( "color" ) ).toObject();
    color.remove( QStringLiteral( "range" ) );
    tf[QStringLiteral( "color" )] = color;
    section[0] = tf;
    root[QStringLiteral( "variance_transfer_functions" )] = section;
    return root;
}

QVector<QColor> colorsFromDefaultJsonMap( const QJsonObject& map )
{
    QVector<QColor> colors;
    const QJsonArray values = map.value( QStringLiteral( "values" ) ).toArray();
    for( int i = 0; i + 2 < values.size(); i += 3 )
    {
        colors.append( QColor( values.at( i ).toInt(), values.at( i + 1 ).toInt(), values.at( i + 2 ).toInt() ) );
    }
    return colors;
}

QVector<float> opacitiesFromDefaultJsonMap( const QJsonObject& map )
{
    QVector<float> opacities;
    const QJsonArray values = map.value( QStringLiteral( "values" ) ).toArray();
    for( const QJsonValue& value : values ) opacities.append( static_cast<float>( value.toDouble() ) );
    return opacities;
}

bool opacitiesNearlyEqual( const QVector<float>& a, const QVector<float>& b )
{
    if( a.size() != b.size() ) return false;
    for( int i = 0; i < a.size(); ++i )
    {
        if( std::abs( a.at( i ) - b.at( i ) ) > 1.0e-6f ) return false;
    }
    return true;
}

std::vector<int> histogramFromJson( const QJsonValue& value )
{
    std::vector<int> histogram;
    for( const QJsonValue& entry : value.toArray() ) histogram.push_back( entry.toInt() );
    return histogram;
}
} // namespace

QString EnsembleTransferFunctionTest::configuredValue( const char* name ) const
{
    const QString env = QProcessEnvironment::systemEnvironment().value( QString::fromLatin1( name ) );
    if( !env.trimmed().isEmpty() ) return env.trimmed();

    QSettings settings( testPathConfigFilePath( repoRootPath() ), QSettings::IniFormat );
    settings.beginGroup( QStringLiteral( "paths" ) );
    const QString value = settings.value( QString::fromLatin1( name ) ).toString();
    settings.endGroup();
    return value.trimmed();
}

QString EnsembleTransferFunctionTest::configuredPathOrEmpty( const char* name ) const
{
    if( configuredValue( name ).isEmpty() ) return QString();
    return configuredPath( name, repoRootPath() );
}

bool EnsembleTransferFunctionTest::check( ItemResult& result, bool condition, const QString& message ) const
{
    if( condition ) return true;
    result.messages.push_back( message );
    result.status = QStringLiteral( "FAIL" );
    qWarning().noquote() << QStringLiteral( "[%1] FAIL: %2" ).arg( result.id, message );
    return false;
}

void EnsembleTransferFunctionTest::skipItem( ItemResult& result, const QString& reason )
{
    result.status = QStringLiteral( "SKIP" );
    result.messages.push_back( reason );
    qInfo().noquote() << QStringLiteral( "[%1] SKIP: %2" ).arg( result.id, reason );
}

void EnsembleTransferFunctionTest::finishItem( ItemResult& result )
{
    if( result.status == QStringLiteral( "NOT RUN" ) ) result.status = QStringLiteral( "PASS" );
    m_item_results.push_back( result );
    markStepCompleted( QStringLiteral( "%1 %2: %3" ).arg( result.id, result.title, result.status ) );
}

bool EnsembleTransferFunctionTest::captureManual(
    ItemResult& result,
    QWidget* window,
    int seq,
    const QString& content,
    const QString& manual_rel_path,
    int time_step )
{
    ManualImage image;
    image.item_id = result.id;
    image.seq = seq;
    image.test_file_name = QStringLiteral( "%1_%2_%3.png" ).arg( result.id ).arg( seq ).arg( content );
    image.manual_rel_path = manual_rel_path;
    image.time_step = time_step;

    if( !screenshotsEnabled() )
    {
        m_manual_images.push_back( image );
        return true;
    }
    if( !window )
    {
        check( result, false, QStringLiteral( "撮影対象ウィンドウが null(連番 %1)" ).arg( seq ) );
        m_manual_images.push_back( image );
        return false;
    }

    saveWindowScreenshot( window, image.test_file_name );
    const QString source = QDir( m_screenshot_dir_path ).absoluteFilePath( image.test_file_name );
    const QString destination = QDir( m_manual_dir_path ).absoluteFilePath( manual_rel_path );
    bool copied = false;
    if( QFileInfo::exists( source ) )
    {
        QDir().mkpath( QFileInfo( destination ).absolutePath() );
        if( QFileInfo::exists( destination ) ) QFile::remove( destination );
        copied = QFile::copy( source, destination );
    }
    image.copied = copied;
    check( result, copied, QStringLiteral( "マニュアル用コピーに失敗: %1" ).arg( manual_rel_path ) );
    result.images.push_back( image.test_file_name );
    m_manual_images.push_back( image );
    return copied;
}

QMenu* EnsembleTransferFunctionTest::findToolsMenu( MainWindow* window ) const
{
    if( !window ) return nullptr;
    QMenu* menu = window->findChild<QMenu*>( QStringLiteral( "menuTools" ) );
    if( menu ) return menu;
    for( QMenu* candidate : window->findChildren<QMenu*>() )
    {
        if( candidate->title().remove( QLatin1Char( '&' ) ) == QStringLiteral( "Tools" ) ) return candidate;
    }
    return nullptr;
}

QAction* EnsembleTransferFunctionTest::findAction( MainWindow* window, const QString& text, bool exclude_ensemble ) const
{
    if( !window ) return nullptr;
    for( QAction* action : window->findChildren<QAction*>() )
    {
        const QString label = action->text().remove( QLatin1Char( '&' ) );
        if( !label.contains( text ) ) continue;
        if( exclude_ensemble && label.contains( QStringLiteral( "Ensemble" ) ) ) continue;
        return action;
    }
    return nullptr;
}

QMessageBox* EnsembleTransferFunctionTest::findVisibleMessageBox() const
{
    for( QWidget* widget : QApplication::topLevelWidgets() )
    {
        auto* box = qobject_cast<QMessageBox*>( widget );
        if( box && box->isVisible() ) return box;
    }
    return nullptr;
}

EnsembleTransferFunctionTest::WarningCapture EnsembleTransferFunctionTest::expectWarning(
    ItemResult& result,
    const std::function<void()>& trigger,
    const QString& expected_text,
    int seq,
    const QString& content,
    const QString& manual_rel_path,
    const std::function<void()>& inspect_while_open )
{
    // モーダルダイアログ(QMessageBox::warning)の表示中にイベントループから呼ばれる監視処理。
    // 監視は本関数の終了後にも発火し得るため、状態は shared_ptr で保持し、終了後は何もしない。
    struct PollState
    {
        bool active = true;
        bool shown = false;
        QString text;
        QString title;
        QElapsedTimer timer;
    };
    auto state = std::make_shared<PollState>();
    state->timer.start();
    ItemResult* result_ptr = &result;
    const std::function<void()> inspect = inspect_while_open;

    auto poll = std::make_shared<std::function<void()>>();
    std::weak_ptr<std::function<void()>> weak_poll = poll;
    *poll = [this, state, result_ptr, seq, content, manual_rel_path, inspect, weak_poll]()
    {
        if( !state->active ) return;
        QMessageBox* box = findVisibleMessageBox();
        if( !box )
        {
            if( state->timer.elapsed() < k_warning_timeout_ms )
            {
                if( auto next = weak_poll.lock() ) QTimer::singleShot( 50, *next );
            }
            return;
        }
        state->shown = true;
        state->text = box->text();
        state->title = box->windowTitle();
        QTest::qWait( k_capture_settle_ms );
        if( seq > 0 ) captureManual( *result_ptr, box, seq, content, manual_rel_path );
        if( inspect ) inspect();
        QAbstractButton* ok = box->button( QMessageBox::Ok );
        if( ok ) QTest::mouseClick( ok, Qt::LeftButton );
        else box->accept();
    };
    QTimer::singleShot( 0, *poll );

    trigger();
    QTest::qWait( k_short_wait_ms );
    state->active = false;

    // 念のため、閉じ損ねたダイアログが残っていれば閉じる
    if( QMessageBox* remaining = findVisibleMessageBox() ) remaining->accept();

    WarningCapture capture;
    capture.shown = state->shown;
    capture.text = state->text;
    capture.title = state->title;
    check( result, capture.shown, QStringLiteral( "警告ダイアログが表示されなかった(期待: %1)" ).arg( expected_text ) );
    if( capture.shown )
    {
        check(
            result,
            capture.text == expected_text,
            QStringLiteral( "警告文が異なる。期待: 「%1」 実際: 「%2」" ).arg( expected_text, capture.text ) );
#if defined( Q_OS_MACOS ) || defined( Q_OS_MAC )
        // macOS では Qt の QMessageBox がウィンドウタイトルを持たない(windowTitle() が空)ためタイトルは判定しない
        if( capture.title.isEmpty() )
        {
            result.messages.push_back( QStringLiteral( "警告ダイアログのタイトル判定は macOS のため省略(QMessageBox はタイトルを表示しない)" ) );
        }
        else
#endif
        check(
            result,
            capture.title == QStringLiteral( "Ensemble Transfer Function Editor" ),
            QStringLiteral( "警告ダイアログのタイトルが異なる: %1" ).arg( capture.title ) );
    }
    return capture;
}

bool EnsembleTransferFunctionTest::verifyNoWarning( ItemResult& result, int wait_ms ) const
{
    bool shown = false;
    QElapsedTimer timer;
    timer.start();
    while( timer.elapsed() < wait_ms )
    {
        if( QMessageBox* box = findVisibleMessageBox() )
        {
            shown = true;
            result.messages.push_back( QStringLiteral( "予期しない警告: %1" ).arg( box->text() ) );
            box->accept();
            break;
        }
        QTest::qWait( 50 );
    }
    return check( result, !shown, QStringLiteral( "警告ダイアログが表示されない想定だが表示された" ) );
}

bool EnsembleTransferFunctionTest::applyAndWait(
    ItemContext& ctx,
    int statistic_index,
    bool require_particle_change,
    ParticleUpdateResult* update_result )
{
    const std::array<StatisticState, 3> expected = captureAllStatisticStates( ctx.client );
    const QString variable = ctx.client.variable_line_edit->text().trimmed();
    selectStatistic( ctx.client, statistic_index );
    update_result->before_fingerprint = particleFingerprint( ctx.client );
    // チェックシート項目からの Apply。診断 JSON 名は項目結果表の通し番号(item_N)で対応付ける
    m_current_case_name = QStringLiteral( "item_%1" ).arg( m_item_results.size() + 1 );
    return waitForAppliedParticles(
        ctx.client,
        *ctx.request_spy,
        *ctx.completion_spy,
        *ctx.request_parameter_spy,
        expected,
        variable,
        require_particle_change,
        update_result );
}

QJsonObject EnsembleTransferFunctionTest::latestRequestPayload( const ItemContext& ctx ) const
{
    // Keep Last により Apply 後も RequestDataAt 応答は進むため、突合には最新の応答を使う
    if( ctx.request_parameter_spy && ctx.request_parameter_spy->count() > 0 )
    {
        const QList<QVariant> arguments = ctx.request_parameter_spy->at( ctx.request_parameter_spy->count() - 1 );
        if( !arguments.isEmpty() ) return qvariant_cast<QJsonObject>( arguments.at( 0 ) );
    }
    return m_last_request_payload;
}

QJsonObject EnsembleTransferFunctionTest::patchForStatistic( const QJsonObject& payload, const QString& statistic ) const
{
    const QJsonArray data = payload.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();
    for( const QJsonValue& value : data )
    {
        const QJsonObject patch = value.toObject();
        QString name = patch.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString().trimmed().toLower();
        if( name == QStringLiteral( "mean" ) ) name = QStringLiteral( "average" );
        if( name == statistic ) return patch;
    }
    return QJsonObject();
}

bool EnsembleTransferFunctionTest::waitForNoParameter( QSignalSpy& spy, int wait_ms ) const
{
    const int start = spy.count();
    QTest::qWait( wait_ms );
    return spy.count() == start;
}

QJsonObject EnsembleTransferFunctionTest::readJsonObject( const QString& path, bool* ok ) const
{
    if( ok ) *ok = false;
    QFile file( path );
    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return QJsonObject();
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson( file.readAll(), &error );
    if( error.error != QJsonParseError::NoError || !document.isObject() ) return QJsonObject();
    if( ok ) *ok = true;
    return document.object();
}

bool EnsembleTransferFunctionTest::writeJsonObject( const QString& path, const QJsonObject& root ) const
{
    QDir().mkpath( QFileInfo( path ).absolutePath() );
    QFile file( path );
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) ) return false;
    file.write( QJsonDocument( root ).toJson( QJsonDocument::Indented ) );
    return true;
}

QString EnsembleTransferFunctionTest::latestHistoryJsonPath() const
{
    if( m_vis_param_dir.isEmpty() ) return QString();
    QDir dir( m_vis_param_dir );
    QStringList names = dir.entryList( QStringList() << QStringLiteral( "history_*.json" ), QDir::Files, QDir::Name );
    if( names.isEmpty() ) return QString();
    return dir.absoluteFilePath( names.last() );
}

QString EnsembleTransferFunctionTest::defaultJsonPath() const
{
    if( m_vis_param_dir.isEmpty() ) return QString();
    const QString tf_name = configuredValue( "TF_NAME" );
    return QDir( m_vis_param_dir ).absoluteFilePath( ( tf_name.isEmpty() ? QStringLiteral( "default" ) : tf_name ) + QStringLiteral( ".json" ) );
}

bool EnsembleTransferFunctionTest::statesEqual( const StatisticState& a, const StatisticState& b, QString* why ) const
{
    if( a.use_user_range != b.use_user_range ) { if( why ) *why = QStringLiteral( "レンジモード" ); return false; }
    if( std::abs( a.user_min - b.user_min ) > 1.0e-7 ) { if( why ) *why = QStringLiteral( "User Min" ); return false; }
    if( std::abs( a.user_max - b.user_max ) > 1.0e-7 ) { if( why ) *why = QStringLiteral( "User Max" ); return false; }
    if( a.colors != b.colors ) { if( why ) *why = QStringLiteral( "Color Map" ); return false; }
    if( !opacitiesNearlyEqual( a.opacities, b.opacities ) ) { if( why ) *why = QStringLiteral( "Opacity Map" ); return false; }
    return true;
}

void EnsembleTransferFunctionTest::restoreBaseline( ItemContext& ctx ) const
{
    bringEditorToFront( ctx.client.ensemble_editor );
    restoreSettings( ctx.client, ctx.baseline_variable, ctx.baseline_states );
}

int EnsembleTransferFunctionTest::totalParticles( const ClientHandles& client ) const
{
    bool ok = false;
    const int value = client.total_particles_display->text().toInt( &ok );
    return ok ? value : -1;
}

// 基本シナリオの 1 ケースを記録する。失敗しても QVERIFY で中断せず、後続のケースとチェックシート項目を続行する。
void EnsembleTransferFunctionTest::recordBaseCase( const QString& title, bool ok, const QString& failure_message )
{
    ItemResult result;
    result.id = QStringLiteral( "基本シナリオ" );
    result.title = title;
    if( ok )
    {
        result.status = QStringLiteral( "PASS" );
    }
    else
    {
        result.status = QStringLiteral( "FAIL" );
        result.messages.push_back( failure_message );
        qWarning().noquote() << QStringLiteral( "[基本シナリオ] FAIL: %1: %2" ).arg( title, failure_message );
    }
    m_base_results.push_back( result );
    markStepCompleted( QStringLiteral( "基本シナリオ %1: %2" ).arg( title, result.status ) );
}

void EnsembleTransferFunctionTest::writeBaseCaseSection( QTextStream& stream ) const
{
    if( m_base_results.empty() ) return;
    stream << "\n## 基本シナリオ結果(3統計 × 変更なし/Variable/User MinMax/Server MinMax/Color Map/Opacity Map、Import 復元)\n\n";
    stream << "| ケース | 結果 | 失敗理由 |\n|---|---|---|\n";
    for( const ItemResult& item : m_base_results )
    {
        QString notes = item.messages.join( QStringLiteral( "<br>" ) );
        notes.replace( QLatin1Char( '|' ), QStringLiteral( "\\|" ) );
        stream << "| " << item.title << " | " << item.status << " | " << notes << " |\n";
    }
}

// ===========================================================================
// C-MB-15 メニューバー: Ensemble Transfer Function Editor をクリックする
// ===========================================================================
void EnsembleTransferFunctionTest::runItemMenuBar15( ItemContext& ctx, MainWindow& window )
{
    ItemResult result;
    result.id = QStringLiteral( "C-MB-15" );
    result.title = QStringLiteral( "Ensemble Transfer Function Editorをクリックする" );

    // IsEnsemble == true はサーバが Initialize 時に送る SelectedFile イベント(ObjectInfo)で受信する
    // (ObjectInfoParameter には含まれない)。受信が遅れる場合に備えて待機する。
    const bool is_ensemble = waitForCondition(
        [&ctx]()
        {
            for( int i = 0; i < ctx.selected_file_spy->count(); ++i )
            {
                const QJsonObject payload = qvariant_cast<QJsonObject>( ctx.selected_file_spy->at( i ).at( 0 ) );
                if( payload.value( QString::fromUtf8( Protocol::Key::IsEnsemble ) ).toBool( false ) ) return true;
            }
            return false;
        },
        k_object_load_timeout_ms,
        100 );
    check( result, is_ensemble, QStringLiteral( "SelectedFile(Initialize の ObjectInfo)の IsEnsemble が true ではない" ) );

    QAction* ensemble_action = findAction( &window, QStringLiteral( "Ensemble Transfer Function Editor" ), false );
    QAction* tfe_action = findAction( &window, QStringLiteral( "Transfer Function Editor" ), true );
    check( result, ensemble_action != nullptr, QStringLiteral( "QAction「Ensemble Transfer Function Editor」が見つからない" ) );
    check( result, tfe_action != nullptr, QStringLiteral( "QAction「Transfer Function Editor」が見つからない" ) );
    if( ensemble_action )
    {
        check(
            result,
            waitForCondition( [ensemble_action]() { return ensemble_action->isEnabled(); }, k_object_load_timeout_ms, 100 ),
            QStringLiteral( "「Ensemble Transfer Function Editor」が有効にならない" ) );
    }
    if( tfe_action )
    {
        check( result, !tfe_action->isEnabled(), QStringLiteral( "「Transfer Function Editor」が無効になっていない(排他)" ) );
    }

    // Tools メニューを展開して撮影
    QMenu* tools = findToolsMenu( &window );
    check( result, tools != nullptr, QStringLiteral( "Tools メニュー(menuTools)が見つからない" ) );
    if( tools )
    {
        bringWindowToFront( &window );
        tools->popup( window.mapToGlobal( QPoint( 40, 40 ) ) );
        QTest::qWait( k_window_settle_ms );
        captureManual( result, tools, 1, QStringLiteral( "tools_menu" ), QStringLiteral( "Client/MainWindow/MenuBar_15_1.png" ) );
        tools->hide();
        QCoreApplication::processEvents();
    }
    finishItem( result );
}

// C-MB-15(2): ダイアログを開いた状態
void EnsembleTransferFunctionTest::runItemEditorOpened15_2( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-MB-15" );
    result.title = QStringLiteral( "Ensemble Transfer Function Editor ダイアログ表示" );
    bringEditorToFront( ctx.client.ensemble_editor );
    check( result, ctx.client.ensemble_editor->isVisible(), QStringLiteral( "ダイアログが表示されていない" ) );
    check(
        result,
        ctx.client.ensemble_editor->windowTitle() == QStringLiteral( "Ensemble Transfer Function Editor" ),
        QStringLiteral( "ダイアログのタイトルが異なる: %1" ).arg( ctx.client.ensemble_editor->windowTitle() ) );
    captureManual( result, ctx.client.ensemble_editor, 2, QStringLiteral( "editor_initial" ), QStringLiteral( "Client/MainWindow/MenuBar_15_2.png" ) );
    finishItem( result );
}

// ===========================================================================
// C-CMS-02 ColorMapSelectorToolBar: アンサンブルモードで Color Function を操作する
// ===========================================================================
void EnsembleTransferFunctionTest::runItemColorMapSelector02( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-CMS-02" );
    result.title = QStringLiteral( "アンサンブルモードでColor Functionを操作する" );

    QComboBox* combo = ctx.client.color_function_combo_box;
    if( !check( result, combo != nullptr, QStringLiteral( "ColorMapSelectorToolBar のコンボボックスが見つからない" ) ) )
    {
        finishItem( result );
        return;
    }
    const QStringList expected_labels = {
        QStringLiteral( "Average" ), QStringLiteral( "Variance" ), QStringLiteral( "Coefficient of Variation" ) };
    QStringList labels;
    for( int i = 0; i < combo->count(); ++i ) labels << combo->itemText( i );
    check( result, labels == expected_labels, QStringLiteral( "コンボボックスの項目が異なる: %1" ).arg( labels.join( ", " ) ) );

    const int previous = combo->currentIndex();
    const std::array<QString, 3> contents = { QStringLiteral( "legend_average" ), QStringLiteral( "legend_variance" ), QStringLiteral( "legend_cv" ) };
    for( int i = 0; i < 3 && i < combo->count(); ++i )
    {
        combo->setCurrentIndex( i );
        QCoreApplication::processEvents();
        QTest::qWait( k_short_wait_ms );
        bringWindowToFront( ctx.client.main_window );
        captureManual(
            result,
            ctx.client.main_window,
            i + 1,
            contents[static_cast<size_t>( i )],
            QStringLiteral( "Client/ColorMapSelectorToolBar/2_%1.png" ).arg( i + 1 ) );
    }
    combo->setCurrentIndex( previous < 0 ? 0 : previous );
    QCoreApplication::processEvents();
    result.messages.push_back( QStringLiteral( "凡例が選択統計に追従する仕様は実装確認中のため、表示内容はスクリーンショットで確認する(FAIL 判定にしない)" ) );
    finishItem( result );
}

// ===========================================================================
// C-ETF-01 Variable
// ===========================================================================
void EnsembleTransferFunctionTest::runItem01Variable( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-01" );
    result.title = QStringLiteral( "Variableを編集する" );
    restoreBaseline( ctx );

    const QString changed = ctx.baseline_variable == QStringLiteral( "q1*2" ) ? QStringLiteral( "q1" ) : QStringLiteral( "q1*2" );
    setLineEditText( ctx.client.variable_line_edit, changed );
    selectStatistic( ctx.client, 0 );
    captureManual( result, ctx.client.ensemble_editor, 1, QStringLiteral( "editor" ), manualEtf( 1, 1 ) );

    ParticleUpdateResult update;
    const bool applied = applyAndWait( ctx, 0, true, &update );
    check( result, applied, QStringLiteral( "Apply 後の RequestDataAt 応答が設定と一致しない、または粒子が変化しない" ) );
    if( applied )
    {
        const QJsonArray data = m_last_request_payload.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();
        bool all_match = data.size() == 3;
        for( const QJsonValue& value : data )
        {
            if( value.toObject().value( QString::fromUtf8( Protocol::Key::EnsembleVariable ) ).toString() != changed ) all_match = false;
        }
        check( result, all_match, QStringLiteral( "RequestDataAt 応答の EnsembleVariable が 3統計すべて「%1」ではない" ).arg( changed ) );
        bringWindowToFront( ctx.client.main_window );
        captureManual( result, ctx.client.main_window, 2, QStringLiteral( "particles" ), manualEtf( 1, 2 ), update.request_time_step );
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-02 Statistic コンボボックス
// ===========================================================================
void EnsembleTransferFunctionTest::runItem02Statistic( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-02" );
    result.title = QStringLiteral( "Statisticコンボボックスを操作する" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;

    // Average: Color Map を Grayscale に
    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "Grayscale" ) );
    if( client.color_map->getColors() == ctx.baseline_states[0].colors )
    {
        applyColorPreset( client, QStringLiteral( "Traffic Lights Step" ) );
    }
    const QVector<QColor> average_colors = client.color_map->getColors();
    check( result, average_colors != ctx.baseline_states[0].colors, QStringLiteral( "Average の Color Map が変更できなかった" ) );

    // Variance: User MinMax
    selectStatistic( client, 1 );
    setUserRange( client, 0.0, 122.0 );

    // CV: Opacity Map(基準が既に 0.5*x のときは 1-x に切り替える)
    selectStatistic( client, 2 );
    applyOpacityExpression( client, QStringLiteral( "0.5*x" ) );
    if( opacitiesNearlyEqual( client.opacity_map->getOpacities(), ctx.baseline_states[2].opacities ) )
    {
        applyOpacityExpression( client, QStringLiteral( "1-x" ) );
    }
    const QVector<float> cv_opacities = client.opacity_map->getOpacities();
    check( result, !opacitiesNearlyEqual( cv_opacities, ctx.baseline_states[2].opacities ), QStringLiteral( "Coefficient of Variation の Opacity Map が変更できなかった(0.5*x / 1-x とも基準と同じ)" ) );

    // 切替で保持されていること
    selectStatistic( client, 0 );
    check( result, client.color_map->getColors() == average_colors, QStringLiteral( "Average の Color Map が統計切替後に保持されていない" ) );
    captureManual( result, client.ensemble_editor, 1, QStringLiteral( "average_editor" ), manualEtf( 2, 1 ) );
    selectStatistic( client, 1 );
    check(
        result,
        client.user_min_max_radio->isChecked() &&
            std::abs( client.user_min_spin_box->value() - 0.0 ) < 1.0e-7 &&
            std::abs( client.user_max_spin_box->value() - 122.0 ) < 1.0e-7,
        QStringLiteral( "Variance の User MinMax が統計切替後に保持されていない" ) );
    captureManual( result, client.ensemble_editor, 2, QStringLiteral( "variance_editor" ), manualEtf( 2, 2 ) );
    selectStatistic( client, 2 );
    check( result, client.opacity_map->getOpacities() == cv_opacities, QStringLiteral( "Coefficient of Variation の Opacity Map が統計切替後に保持されていない" ) );
    captureManual( result, client.ensemble_editor, 3, QStringLiteral( "cv_editor" ), manualEtf( 2, 3 ) );

    // Variance を選択して Apply → サーバ再配信の Statistic == "variance"、粒子が変化
    const int parameter_start = ctx.parameter_spy->count();
    ParticleUpdateResult update;
    const bool applied = applyAndWait( ctx, 1, true, &update );
    check( result, applied, QStringLiteral( "Variance を Apply 後、RequestDataAt 応答が一致しないか粒子が変化しない" ) );
    // サーバの再配信(ws->publish)は uWebSockets の仕様で送信元クライアントには届かないため参考情報として記録する
    bool variance_echoed = false;
    for( int i = parameter_start; i < ctx.parameter_spy->count(); ++i )
    {
        const QJsonObject payload = qvariant_cast<QJsonObject>( ctx.parameter_spy->at( i ).at( 0 ) );
        if( payload.value( QString::fromUtf8( Protocol::Key::Statistic ) ).toString() == QStringLiteral( "variance" ) ) variance_echoed = true;
    }
    result.messages.push_back( variance_echoed
        ? QStringLiteral( "サーバ再配信(Statistic=variance)を受信(参考)" )
        : QStringLiteral( "サーバ再配信(Statistic=variance)は未受信(参考: 単一クライアントでは送信元に配信されない仕様。2 クライアント時に確認)" ) );
    if( applied )
    {
        bringWindowToFront( client.main_window );
        captureManual( result, client.main_window, 4, QStringLiteral( "variance_particles" ), manualEtf( 2, 4 ), update.request_time_step );
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-03 Color Map
// ===========================================================================
void EnsembleTransferFunctionTest::runItem03ColorMap( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-03" );
    result.title = QStringLiteral( "Color Mapをクリックする" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;
    selectStatistic( client, 0 );

    // ColorMapEditor が開いた状態を撮影(applyColorPreset と同じ手順でプリセットを選んでから撮影して Apply)
    QTimer::singleShot(
        0,
        [this, &result]()
        {
            QDialog* dialog = nullptr;
            const bool visible = waitForCondition(
                [&dialog]()
                {
                    for( QWidget* widget : QApplication::topLevelWidgets() )
                    {
                        auto* candidate = qobject_cast<QDialog*>( widget );
                        if( candidate && candidate->objectName() == QStringLiteral( "ColorMapEditor" ) && candidate->isVisible() )
                        {
                            dialog = candidate;
                            return true;
                        }
                    }
                    return false;
                },
                k_dialog_timeout_ms,
                50 );
            if( !check( result, visible, QStringLiteral( "ColorMapEditor が表示されない" ) ) ) return;

            auto* table = dialog->findChild<QTableWidget*>( "colorMapTableWidget" );
            int preset_row = -1;
            int preset_column = -1;
            if( table )
            {
                for( int row = 0; row < table->rowCount() && preset_row < 0; ++row )
                {
                    for( int column = 0; column < table->columnCount(); ++column )
                    {
                        QWidget* cell = table->cellWidget( row, column );
                        if( !cell ) continue;
                        for( QLabel* label : cell->findChildren<QLabel*>() )
                        {
                            if( label && label->text() == QStringLiteral( "Cool to Warm" ) )
                            {
                                preset_row = row;
                                preset_column = column;
                                break;
                            }
                        }
                        if( preset_row >= 0 ) break;
                    }
                }
            }
            if( check( result, preset_row >= 0, QStringLiteral( "プリセット「Cool to Warm」が見つからない" ) ) )
            {
                QMetaObject::invokeMethod( dialog, "onPresetColorMapDoubleClicked", Qt::DirectConnection,
                                           Q_ARG( int, preset_row ), Q_ARG( int, preset_column ) );
            }
            QTest::qWait( k_short_wait_ms );
            captureManual( result, dialog, 1, QStringLiteral( "colormap_editor" ), manualEtf( 3, 1 ) );
            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            if( check( result, apply_button != nullptr, QStringLiteral( "ColorMapEditor の applyPushButton が見つからない" ) ) )
            {
                QTest::mouseClick( apply_button, Qt::LeftButton );
            }
            else dialog->reject();
        } );
    QTest::mouseClick( client.color_map, Qt::LeftButton, Qt::NoModifier, client.color_map->rect().center() );
    QCoreApplication::processEvents();
    QTest::qWait( k_short_wait_ms );

    check( result, client.color_map->getColors() != ctx.baseline_states[0].colors, QStringLiteral( "Color Map が基準状態から変化していない" ) );
    captureManual( result, client.ensemble_editor, 2, QStringLiteral( "editor" ), manualEtf( 3, 2 ) );

    ParticleUpdateResult update;
    const bool applied = applyAndWait( ctx, 0, true, &update );
    check( result, applied, QStringLiteral( "Apply 後の RequestDataAt 応答(EnsembleColorMap)が GUI と一致しないか粒子が変化しない" ) );
    if( applied )
    {
        const QJsonArray colors = patchForStatistic( m_last_request_payload, QStringLiteral( "average" ) )
                                      .value( QString::fromUtf8( Protocol::Key::EnsembleColorMap ) ).toArray();
        check( result, colors.size() == 256, QStringLiteral( "EnsembleColorMap の要素数が 256 ではない: %1" ).arg( colors.size() ) );
        bringWindowToFront( client.main_window );
        captureManual( result, client.main_window, 3, QStringLiteral( "particles" ), manualEtf( 3, 3 ), update.request_time_step );
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-04 Opacity Map
// ===========================================================================
void EnsembleTransferFunctionTest::runItem04OpacityMap( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-04" );
    result.title = QStringLiteral( "Opacity Mapをクリックする" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;
    selectStatistic( client, 0 );

    QTimer::singleShot(
        0,
        [this, &result]()
        {
            QDialog* dialog = nullptr;
            const bool visible = waitForCondition(
                [&dialog]()
                {
                    for( QWidget* widget : QApplication::topLevelWidgets() )
                    {
                        auto* candidate = qobject_cast<QDialog*>( widget );
                        if( candidate && candidate->objectName() == QStringLiteral( "OpacityMapEditor" ) && candidate->isVisible() )
                        {
                            dialog = candidate;
                            return true;
                        }
                    }
                    return false;
                },
                k_dialog_timeout_ms,
                50 );
            if( !check( result, visible, QStringLiteral( "OpacityMapEditor が表示されない" ) ) ) return;

            auto* tab_widget = dialog->findChild<QTabWidget*>( "tabWidget" );
            auto* expression_page = dialog->findChild<QWidget*>( "expression" );
            auto* line_edit = dialog->findChild<QLineEdit*>( "opacityLineEdit" );
            if( check( result, tab_widget && expression_page && line_edit, QStringLiteral( "OpacityMapEditor の expression ページが見つからない" ) ) )
            {
                tab_widget->setCurrentWidget( expression_page );
                line_edit->setText( QStringLiteral( "0.5*x" ) );
                QMetaObject::invokeMethod( dialog, "onExpressionChanged", Qt::DirectConnection );
            }
            QTest::qWait( k_short_wait_ms );
            captureManual( result, dialog, 1, QStringLiteral( "opacitymap_editor" ), manualEtf( 4, 1 ) );
            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            if( check( result, apply_button != nullptr, QStringLiteral( "OpacityMapEditor の applyPushButton が見つからない" ) ) )
            {
                QTest::mouseClick( apply_button, Qt::LeftButton );
            }
            else dialog->reject();
        } );
    QTest::mouseClick( client.opacity_map, Qt::LeftButton, Qt::NoModifier, client.opacity_map->rect().center() );
    QCoreApplication::processEvents();
    QTest::qWait( k_short_wait_ms );

    check( result, client.opacity_map->getOpacities() != ctx.baseline_states[0].opacities, QStringLiteral( "Opacity Map が基準状態から変化していない" ) );
    captureManual( result, client.ensemble_editor, 2, QStringLiteral( "editor" ), manualEtf( 4, 2 ) );

    ParticleUpdateResult update;
    const bool applied = applyAndWait( ctx, 0, true, &update );
    check( result, applied, QStringLiteral( "Apply 後の RequestDataAt 応答(EnsembleOpacityMap)が GUI と一致しないか粒子が変化しない" ) );
    if( applied )
    {
        const QJsonArray opacities = patchForStatistic( m_last_request_payload, QStringLiteral( "average" ) )
                                         .value( QString::fromUtf8( Protocol::Key::EnsembleOpacityMap ) ).toArray();
        check( result, opacities.size() == 256, QStringLiteral( "EnsembleOpacityMap の要素数が 256 ではない: %1" ).arg( opacities.size() ) );
        bringWindowToFront( client.main_window );
        captureManual( result, client.main_window, 3, QStringLiteral( "particles" ), manualEtf( 4, 3 ), update.request_time_step );
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-05 Histogram
// ===========================================================================
void EnsembleTransferFunctionTest::runItem05Histogram( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-05" );
    result.title = QStringLiteral( "Histogramを確認する" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;
    if( !check( result, client.histogram != nullptr, QStringLiteral( "m_histogram が見つからない" ) ) )
    {
        finishItem( result );
        return;
    }

    const std::array<QString, 3> names = { QStringLiteral( "average" ), QStringLiteral( "variance" ), QStringLiteral( "cv" ) };
    const std::array<QString, 3> history_keys = {
        QStringLiteral( "average" ), QStringLiteral( "variance" ), QStringLiteral( "coefficient_of_variation" ) };
    std::array<std::vector<int>, 3> gui_histograms;
    // Keep Last により比較中にも応答が届いて GUI が更新されるため、統計ごとに比較直前の応答を取り直し、
    // 直近数件の応答のいずれかと一致すれば PASS とする(1 回だけ待って再試行)
    constexpr int k_recent_responses = 5;
    const auto matches_recent_response = [this, &ctx]( const QString& statistic, const std::vector<int>& gui, int* matched_step )
    {
        QSignalSpy* spy = ctx.request_parameter_spy;
        if( !spy ) return false;
        const int count = static_cast<int>( spy->count() );
        const int begin = std::max( 0, count - k_recent_responses );
        for( int index = count - 1; index >= begin; --index )
        {
            const QJsonObject payload = qvariant_cast<QJsonObject>( spy->at( index ).at( 0 ) );
            const QJsonObject patch = patchForStatistic( payload, statistic );
            if( patch.isEmpty() ) continue;
            if( histogramFromJson( patch.value( QString::fromUtf8( Protocol::Key::EnsembleHistogram ) ) ) == gui )
            {
                if( matched_step ) *matched_step = index;
                return true;
            }
        }
        return false;
    };
    for( int i = 0; i < 3; ++i )
    {
        selectStatistic( client, i );
        gui_histograms[static_cast<size_t>( i )] = client.histogram->getDatas();
        check(
            result,
            gui_histograms[static_cast<size_t>( i )].size() == 256,
            QStringLiteral( "%1 の Histogram 要素数が 256 ではない: %2" ).arg( names[static_cast<size_t>( i )] ).arg( gui_histograms[static_cast<size_t>( i )].size() ) );
        int matched = -1;
        bool histogram_matched = matches_recent_response( names[static_cast<size_t>( i )], gui_histograms[static_cast<size_t>( i )], &matched );
        if( !histogram_matched )
        {
            // 応答到着と GUI 更新のタイミング差を吸収するため、少し待って GUI を読み直す
            QTest::qWait( k_short_wait_ms );
            gui_histograms[static_cast<size_t>( i )] = client.histogram->getDatas();
            histogram_matched = matches_recent_response( names[static_cast<size_t>( i )], gui_histograms[static_cast<size_t>( i )], &matched );
        }
        check(
            result,
            histogram_matched,
            QStringLiteral( "%1 の Histogram が直近 %2 件の RequestDataAt 応答のいずれとも一致しない" ).arg( names[static_cast<size_t>( i )] ).arg( k_recent_responses ) );
        captureManual( result, client.ensemble_editor, i + 1, QStringLiteral( "histogram_" ) + names[static_cast<size_t>( i )], manualEtf( 5, i + 1 ) );
    }
    check(
        result,
        gui_histograms[0] != gui_histograms[1] || gui_histograms[1] != gui_histograms[2],
        QStringLiteral( "3統計の Histogram がすべて同一" ) );

    const QString history_path = latestHistoryJsonPath();
    if( history_path.isEmpty() )
    {
        result.messages.push_back( QStringLiteral( "history_XXXXX.json との突合: SKIP(ENSEMBLE_VIS_PARAM_DIR 未設定または未検出)" ) );
    }
    else
    {
        bool ok = false;
        const QJsonObject history = readJsonObject( history_path, &ok );
        if( check( result, ok, QStringLiteral( "history json を読めない: %1" ).arg( history_path ) ) )
        {
            const QJsonObject statistics = history.value( QStringLiteral( "statistics" ) ).toObject();
            for( int i = 0; i < 3; ++i )
            {
                const QJsonObject o1 = statistics.value( history_keys[static_cast<size_t>( i )] ).toObject().value( QStringLiteral( "O1" ) ).toObject();
                const std::vector<int> file_histogram = histogramFromJson( o1.value( QStringLiteral( "histogram" ) ) );
                check(
                    result,
                    file_histogram == gui_histograms[static_cast<size_t>( i )],
                    QStringLiteral( "%1 の Histogram が %2 と一致しない(タイムステップ差の可能性あり)" ).arg( names[static_cast<size_t>( i )], QFileInfo( history_path ).fileName() ) );
            }
            result.messages.push_back( QStringLiteral( "突合ファイル: %1" ).arg( history_path ) );
        }
    }
    finishItem( result );
}

// ===========================================================================
// C-ETF-08 / C-ETF-06 レンジモード(Server → User)
// ===========================================================================
void EnsembleTransferFunctionTest::runItem06And08RangeMode( ItemContext& ctx )
{
    const ClientHandles& client = ctx.client;

    ItemResult result08;
    result08.id = QStringLiteral( "C-ETF-08" );
    result08.title = QStringLiteral( "Server Side Min Maxラジオボタンを押下する" );
    restoreBaseline( ctx );
    selectStatistic( client, 0 );
    setServerRange( client );
    captureManual( result08, client.ensemble_editor, 1, QStringLiteral( "editor_server" ), manualEtf( 8, 1 ) );
    ParticleUpdateResult update_server;
    const bool applied_server = applyAndWait( ctx, 0, true, &update_server );
    check( result08, applied_server, QStringLiteral( "Server Side Min Max で Apply 後、応答が一致しないか粒子が変化しない" ) );
    if( applied_server )
    {
        const int mode = patchForStatistic( m_last_request_payload, QStringLiteral( "average" ) )
                             .value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode ) ).toInt();
        check( result08, mode == 2, QStringLiteral( "EnsembleUserRangeMode が 2 ではない: %1" ).arg( mode ) );
        bringWindowToFront( client.main_window );
        captureManual( result08, client.main_window, 2, QStringLiteral( "particles_server" ), manualEtf( 8, 2 ), update_server.request_time_step );
    }
    const QByteArray server_fingerprint = particleFingerprint( client );
    finishItem( result08 );

    ItemResult result06;
    result06.id = QStringLiteral( "C-ETF-06" );
    result06.title = QStringLiteral( "User Defined Min Maxラジオボタンを押下する" );
    bringEditorToFront( client.ensemble_editor );
    selectStatistic( client, 0 );
    if( !client.user_min_max_radio->isChecked() ) QTest::mouseClick( client.user_min_max_radio, Qt::LeftButton );
    check( result06, client.user_min_max_radio->isChecked(), QStringLiteral( "User Defined Min Max が選択できない" ) );
    captureManual( result06, client.ensemble_editor, 1, QStringLiteral( "editor_user" ), manualEtf( 6, 1 ) );
    ParticleUpdateResult update_user;
    const bool applied_user = applyAndWait( ctx, 0, true, &update_user );
    check( result06, applied_user, QStringLiteral( "User Defined Min Max で Apply 後、応答が一致しないか粒子が変化しない" ) );
    if( applied_user )
    {
        const int mode = patchForStatistic( m_last_request_payload, QStringLiteral( "average" ) )
                             .value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode ) ).toInt();
        check( result06, mode == 1, QStringLiteral( "EnsembleUserRangeMode が 1 ではない: %1" ).arg( mode ) );
        check( result06, update_user.after_fingerprint != server_fingerprint, QStringLiteral( "粒子が Server モード時と同一" ) );
        bringWindowToFront( client.main_window );
        captureManual( result06, client.main_window, 2, QStringLiteral( "particles_user" ), manualEtf( 6, 2 ), update_user.request_time_step );
    }
    restoreBaseline( ctx );
    finishItem( result06 );
}

// ===========================================================================
// C-ETF-07 User Defined Min Max の編集
// ===========================================================================
void EnsembleTransferFunctionTest::runItem07UserMinMax( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-07" );
    result.title = QStringLiteral( "User Defined Min Maxを編集する" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;

    struct Case { int index; double min; double max; QString name; int seq_editor; int seq_particles; };
    const std::array<Case, 2> cases = { {
        { 0, 0.0, 122.0, QStringLiteral( "average" ), 1, 2 },
        { 2, 0.0, 2.5, QStringLiteral( "cv" ), 3, 4 } } };
    for( const Case& c : cases )
    {
        selectStatistic( client, c.index );
        setUserRange( client, c.min, c.max );
        captureManual( result, client.ensemble_editor, c.seq_editor, QStringLiteral( "editor_" ) + c.name, manualEtf( 7, c.seq_editor ) );
        ParticleUpdateResult update;
        const bool applied = applyAndWait( ctx, c.index, true, &update );
        check( result, applied, QStringLiteral( "%1: Apply 後の応答が一致しないか粒子が変化しない" ).arg( c.name ) );
        if( applied )
        {
            const QJsonObject patch = patchForStatistic( m_last_request_payload, c.name );
            check(
                result,
                std::abs( patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMin ) ).toDouble() - c.min ) < 1.0e-7 &&
                    std::abs( patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMax ) ).toDouble() - c.max ) < 1.0e-7,
                QStringLiteral( "%1: EnsembleUserRangeMin/Max が %2 / %3 ではない" ).arg( c.name ).arg( c.min ).arg( c.max ) );
            bringWindowToFront( client.main_window );
            captureManual( result, client.main_window, c.seq_particles, QStringLiteral( "particles_" ) + c.name, manualEtf( 7, c.seq_particles ), update.request_time_step );
        }
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-09 Server Side Min Max の表示
// ===========================================================================
void EnsembleTransferFunctionTest::runItem09ServerMinMax( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-09" );
    result.title = QStringLiteral( "Server Side Min Maxを確認する" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;
    const std::array<QString, 3> names = { QStringLiteral( "average" ), QStringLiteral( "variance" ), QStringLiteral( "cv" ) };
    const std::array<QString, 3> history_keys = {
        QStringLiteral( "average" ), QStringLiteral( "variance" ), QStringLiteral( "coefficient_of_variation" ) };

    check( result, client.server_min_spin_box->isReadOnly() && client.server_max_spin_box->isReadOnly(), QStringLiteral( "Server Side Min/Max が readOnly ではない" ) );
    const QString history_path = latestHistoryJsonPath();
    bool history_ok = false;
    QJsonObject history;
    if( !history_path.isEmpty() ) history = readJsonObject( history_path, &history_ok );

    const QJsonObject latest_payload = latestRequestPayload( ctx );
    for( int i = 0; i < 3; ++i )
    {
        selectStatistic( client, i );
        const double gui_min = client.server_min_spin_box->value();
        const double gui_max = client.server_max_spin_box->value();
        const QJsonObject patch = patchForStatistic( latest_payload, names[static_cast<size_t>( i )] );
        if( !patch.isEmpty() )
        {
            const double rx_min = patch.value( QString::fromUtf8( Protocol::Key::EnsembleServerRangeMin ) ).toDouble();
            const double rx_max = patch.value( QString::fromUtf8( Protocol::Key::EnsembleServerRangeMax ) ).toDouble();
            check(
                result,
                std::abs( gui_min - rx_min ) < 1.0e-7 && std::abs( gui_max - rx_max ) < 1.0e-7,
                QStringLiteral( "%1: GUI の Server MinMax(%2 / %3)が応答(%4 / %5)と一致しない" )
                    .arg( names[static_cast<size_t>( i )] ).arg( gui_min ).arg( gui_max ).arg( rx_min ).arg( rx_max ) );
        }
        if( history_ok )
        {
            const QJsonObject o1 = history.value( QStringLiteral( "statistics" ) ).toObject()
                                       .value( history_keys[static_cast<size_t>( i )] ).toObject()
                                       .value( QStringLiteral( "O1" ) ).toObject();
            const double file_min = o1.value( QStringLiteral( "min" ) ).toDouble();
            const double file_max = o1.value( QStringLiteral( "max" ) ).toDouble();
            check(
                result,
                std::abs( gui_min - file_min ) < 1.0e-6 && std::abs( gui_max - file_max ) < 1.0e-6,
                QStringLiteral( "%1: Server MinMax が %2 の O1.min/max と一致しない(タイムステップ差の可能性あり)" )
                    .arg( names[static_cast<size_t>( i )], QFileInfo( history_path ).fileName() ) );
        }
        captureManual( result, client.ensemble_editor, i + 1, QStringLiteral( "server_minmax_" ) + names[static_cast<size_t>( i )], manualEtf( 9, i + 1 ) );
    }
    if( !history_ok ) result.messages.push_back( QStringLiteral( "history_XXXXX.json との突合: SKIP(ENSEMBLE_VIS_PARAM_DIR 未設定または未検出)" ) );
    finishItem( result );
}

// ===========================================================================
// C-ETF-10 Apply(3統計同時変更)
// ===========================================================================
void EnsembleTransferFunctionTest::runItem10Apply( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-10" );
    result.title = QStringLiteral( "Applyボタンを押下する" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;

    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "Traffic Lights Step" ) );
    selectStatistic( client, 1 );
    applyOpacityExpression( client, QStringLiteral( "0.5*x" ) );
    selectStatistic( client, 2 );
    setUserRange( client, 0.0, 2.5 );
    selectStatistic( client, 0 );
    captureManual( result, client.ensemble_editor, 1, QStringLiteral( "editor" ), manualEtf( 10, 1 ) );

    const std::array<StatisticState, 3> sent = captureAllStatisticStates( client );
    selectStatistic( client, 0 );
    ParticleUpdateResult update;
    // Apply(waitForAppliedParticles 内で押下)の直後に警告が出ないことを確認するため、押下前に監視を仕込む
    bool warning_seen = false;
    QTimer::singleShot( 300, [this, &warning_seen]() { if( findVisibleMessageBox() ) warning_seen = true; } );
    const bool applied = applyAndWait( ctx, 0, true, &update );
    check( result, !warning_seen, QStringLiteral( "Apply 時に警告ダイアログが表示された" ) );
    check( result, applied, QStringLiteral( "Apply 後、RequestDataAt 応答の 3統計が GUI と一致しないか粒子が変化しない" ) );
    check( result, client.keep_last_button->isChecked(), QStringLiteral( "Keep Last が OFF になった" ) );

    const QString default_path = defaultJsonPath();
    if( default_path.isEmpty() || !QFileInfo::exists( default_path ) )
    {
        result.messages.push_back( QStringLiteral( "default.json との突合: SKIP(ENSEMBLE_VIS_PARAM_DIR 未設定または未検出)" ) );
    }
    else
    {
        bool ok = false;
        const QJsonObject root = readJsonObject( default_path, &ok );
        if( check( result, ok, QStringLiteral( "default.json を読めない: %1" ).arg( default_path ) ) )
        {
            const std::array<QString, 3> sections = {
                QStringLiteral( "mean_transfer_functions" ),
                QStringLiteral( "variance_transfer_functions" ),
                QStringLiteral( "coefficient_of_variation_transfer_functions" ) };
            for( int i = 0; i < 3; ++i )
            {
                const QJsonArray section = root.value( sections[static_cast<size_t>( i )] ).toArray();
                if( !check( result, !section.isEmpty(), QStringLiteral( "default.json に %1 が無い" ).arg( sections[static_cast<size_t>( i )] ) ) ) continue;
                const QJsonObject color = section.first().toObject().value( QStringLiteral( "color" ) ).toObject();
                const QJsonObject opacity = section.first().toObject().value( QStringLiteral( "opacity" ) ).toObject();
                const QJsonObject user = color.value( QStringLiteral( "range" ) ).toObject().value( QStringLiteral( "user" ) ).toObject();
                const StatisticState& state = sent[static_cast<size_t>( i )];
                check(
                    result,
                    std::abs( user.value( QStringLiteral( "min" ) ).toDouble() - state.user_min ) < 1.0e-6 &&
                        std::abs( user.value( QStringLiteral( "max" ) ).toDouble() - state.user_max ) < 1.0e-6,
                    QStringLiteral( "default.json %1 の range.user が送信値と一致しない" ).arg( sections[static_cast<size_t>( i )] ) );
                check(
                    result,
                    colorsFromDefaultJsonMap( color.value( QStringLiteral( "map" ) ).toObject() ) == state.colors,
                    QStringLiteral( "default.json %1 の color.map が送信値と一致しない" ).arg( sections[static_cast<size_t>( i )] ) );
                check(
                    result,
                    opacitiesNearlyEqual( opacitiesFromDefaultJsonMap( opacity.value( QStringLiteral( "map" ) ).toObject() ), state.opacities ),
                    QStringLiteral( "default.json %1 の opacity.map が送信値と一致しない" ).arg( sections[static_cast<size_t>( i )] ) );
            }
            result.messages.push_back( QStringLiteral( "突合ファイル: %1" ).arg( default_path ) );
        }
    }
    if( applied )
    {
        bringWindowToFront( client.main_window );
        captureManual( result, client.main_window, 2, QStringLiteral( "particles" ), manualEtf( 10, 2 ), update.request_time_step );
    }
    // C-ETF-12 で同じ状態を使うため、ここでは基準状態へ戻さない
    finishItem( result );
}

// ===========================================================================
// C-ETF-11 他クライアントへの反映(2クライアント)
// ===========================================================================
void EnsembleTransferFunctionTest::runItem11SecondClient( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-11" );
    result.title = QStringLiteral( "Apply結果が他クライアントへ反映されることを確認する" );

    const QString flag = configuredValue( "ENSEMBLE_TWO_CLIENTS" ).toLower();
    if( flag != QStringLiteral( "1" ) && flag != QStringLiteral( "true" ) && flag != QStringLiteral( "yes" ) )
    {
        skipItem( result, QStringLiteral( "同一プロセスで 2 台目の MainWindow を生成する検証は ENSEMBLE_TWO_CLIENTS=1 のときのみ実行する(手動撮影に切り替える)" ) );
        m_item_results.push_back( result );
        markStepCompleted( QStringLiteral( "%1 %2: SKIP" ).arg( result.id, result.title ) );
        return;
    }

    const ClientHandles& client = ctx.client;
    MainWindow second_window( *g_ensemble_test_app );
    showTestWindowCentered( &second_window, 80 );
    if( !QTest::qWaitForWindowExposed( &second_window ) )
    {
        result.messages.push_back( QStringLiteral( "2台目の MainWindow の表示待ちがタイムアウト" ) );
    }
    const ClientHandles second = resolveClientHandles( second_window );
    if( !check( result, second.communication != nullptr && second.ensemble_editor != nullptr, QStringLiteral( "2台目のハンドル解決に失敗" ) ) )
    {
        finishItem( result );
        return;
    }
    QSignalSpy second_spy( second.communication, &Communication::receiveEnsembleStatisticsParameter );
    second.communication->show();
    connectClient( second );
    if( second.remote_viz_insitu_radio->isEnabled() && second.setting_apply_button->isEnabled() )
    {
        configureInSituVisualization( second );
    }
    QTest::qWait( k_short_wait_ms );

    // 1台目: Variance + Grayscale を Apply
    bringEditorToFront( client.ensemble_editor );
    selectStatistic( client, 1 );
    applyColorPreset( client, QStringLiteral( "Grayscale" ) );
    if( client.color_map->getColors() == ctx.baseline_states[1].colors ) applyColorPreset( client, QStringLiteral( "Traffic Lights Step" ) );
    const QVector<QColor> first_colors = client.color_map->getColors();
    captureManual( result, client.ensemble_editor, 1, QStringLiteral( "client1_editor" ), manualEtf( 11, 1 ) );
    const int second_start = second_spy.count();
    QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton );

    const bool received = waitForCondition( [&second_spy, second_start]() { return second_spy.count() > second_start; }, k_object_load_timeout_ms, 100 );
    check( result, received, QStringLiteral( "2台目が EnsembleStatisticsParameter を受信しない" ) );
    if( received )
    {
        bringEditorToFront( second.ensemble_editor );
        check( result, second.statistic_combo_box->currentIndex() == 1, QStringLiteral( "2台目の Statistic が Variance ではない: %1" ).arg( second.statistic_combo_box->currentIndex() ) );
        selectStatistic( second, 1 );
        check( result, second.color_map->getColors() == first_colors, QStringLiteral( "2台目の Variance Color Map が1台目と一致しない" ) );
        captureManual( result, second.ensemble_editor, 2, QStringLiteral( "client2_editor" ), manualEtf( 11, 2 ) );
    }
    // 2台目を切断して閉じる
    if( second.disconnect_button->isEnabled() ) QTest::mouseClick( second.disconnect_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    second_window.close();
    QCoreApplication::processEvents();

    // 1台目の粒子側も同期(次ステップ待ち)しておく
    ParticleUpdateResult update;
    applyAndWait( ctx, 1, false, &update );
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-12 Export
// ===========================================================================
void EnsembleTransferFunctionTest::runItem12Export( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-12" );
    result.title = QStringLiteral( "Exportボタンを押下する" );
    const ClientHandles& client = ctx.client;

    // C-ETF-10 と同じ状態にする
    restoreBaseline( ctx );
    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "Traffic Lights Step" ) );
    selectStatistic( client, 1 );
    applyOpacityExpression( client, QStringLiteral( "0.5*x" ) );
    selectStatistic( client, 2 );
    setUserRange( client, 0.0, 2.5 );
    const std::array<StatisticState, 3> states = captureAllStatisticStates( client );
    const QString variable = client.variable_line_edit->text().trimmed();
    selectStatistic( client, 0 );
    captureManual( result, client.ensemble_editor, 1, QStringLiteral( "editor_export" ), manualEtf( 12, 1 ) );

    exportSettings( client, m_item_export_path );
    const bool exists = QFileInfo::exists( m_item_export_path );
    if( check( result, exists, QStringLiteral( "Export JSON が作成されない: %1" ).arg( m_item_export_path ) ) )
    {
        validateExportedJson( m_item_export_path, variable, states );
        bool ok = false;
        const QJsonObject root = readJsonObject( m_item_export_path, &ok );
        const QJsonObject tf_settings = root.value( QStringLiteral( "settings" ) ).toObject().value( QStringLiteral( "transfer_function" ) ).toObject();
        check( result, ok, QStringLiteral( "Export JSON をパースできない" ) );
        check( result, root.value( QStringLiteral( "format" ) ).toString() == QStringLiteral( "PBVR readable transfer function settings" ), QStringLiteral( "format が異なる" ) );
        check( result, root.value( QStringLiteral( "format_version" ) ).toInt() == 1, QStringLiteral( "format_version が 1 ではない" ) );
        check( result, tf_settings.value( QStringLiteral( "transfer_function_count" ) ).toInt() == 1, QStringLiteral( "transfer_function_count が 1 ではない" ) );
        check( result, tf_settings.value( QStringLiteral( "transfer_function_resolution" ) ).toInt() == 256, QStringLiteral( "transfer_function_resolution が 256 ではない" ) );
        check( result, tf_settings.value( QStringLiteral( "color_synthesis" ) ).toString() == QStringLiteral( "C1" ), QStringLiteral( "color_synthesis が C1 ではない" ) );
        check( result, tf_settings.value( QStringLiteral( "opacity_synthesis" ) ).toString() == QStringLiteral( "O1" ), QStringLiteral( "opacity_synthesis が O1 ではない" ) );
        const std::array<QString, 4> sections = {
            QStringLiteral( "transfer_functions" ),
            QStringLiteral( "mean_transfer_functions" ),
            QStringLiteral( "variance_transfer_functions" ),
            QStringLiteral( "coefficient_of_variation_transfer_functions" ) };
        for( const QString& section : sections )
        {
            const QJsonArray array = root.value( section ).toArray();
            if( !check( result, array.size() == 1, QStringLiteral( "%1 の要素数が 1 ではない" ).arg( section ) ) ) continue;
            const QJsonObject tf = array.first().toObject();
            const QJsonObject color = tf.value( QStringLiteral( "color" ) ).toObject();
            const QJsonObject opacity = tf.value( QStringLiteral( "opacity" ) ).toObject();
            check( result, color.value( QStringLiteral( "variable" ) ).toString() == variable && opacity.value( QStringLiteral( "variable" ) ).toString() == variable,
                   QStringLiteral( "%1 の variable が Variable と一致しない" ).arg( section ) );
            check( result, color.value( QStringLiteral( "range" ) ).toObject().value( QStringLiteral( "active_range" ) ).toString() == QStringLiteral( "user" ),
                   QStringLiteral( "%1 の active_range が user ではない" ).arg( section ) );
            check( result, color.value( QStringLiteral( "map" ) ).toObject().value( QStringLiteral( "length" ) ).toInt() == 768,
                   QStringLiteral( "%1 の color.map.length が 768 ではない" ).arg( section ) );
            check( result, opacity.value( QStringLiteral( "map" ) ).toObject().value( QStringLiteral( "length" ) ).toInt() == 256,
                   QStringLiteral( "%1 の opacity.map.length が 256 ではない" ).arg( section ) );
            const QJsonObject user = color.value( QStringLiteral( "range" ) ).toObject().value( QStringLiteral( "user" ) ).toObject();
            result.messages.push_back( QStringLiteral( "%1: user.min=%2 user.max=%3 color.map.length=%4 opacity.map.length=%5" )
                                           .arg( section )
                                           .arg( user.value( QStringLiteral( "min" ) ).toDouble() )
                                           .arg( user.value( QStringLiteral( "max" ) ).toDouble() )
                                           .arg( color.value( QStringLiteral( "map" ) ).toObject().value( QStringLiteral( "length" ) ).toInt() )
                                           .arg( opacity.value( QStringLiteral( "map" ) ).toObject().value( QStringLiteral( "length" ) ).toInt() ) );
        }
    }
    m_import_variable = variable;
    m_import_states = states;
    m_has_import_settings = true;
    finishItem( result );
}

// ===========================================================================
// C-ETF-13 Import(復元)
// ===========================================================================
void EnsembleTransferFunctionTest::runItem13Import( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-13" );
    result.title = QStringLiteral( "Importボタンを押下する" );
    const ClientHandles& client = ctx.client;
    if( !QFileInfo::exists( m_item_export_path ) )
    {
        skipItem( result, QStringLiteral( "C-ETF-12 の Export JSON が無い" ) );
        m_item_results.push_back( result );
        return;
    }
    const std::array<StatisticState, 3> exported = m_import_states;
    const QString exported_variable = m_import_variable;

    // GUI を別の値に変更
    const QString mutated = exported_variable == QStringLiteral( "q1" ) ? QStringLiteral( "q1*2" ) : QStringLiteral( "q1" );
    setLineEditText( client.variable_line_edit, mutated );
    for( int i = 0; i < 3; ++i )
    {
        selectStatistic( client, i );
        setUserRange( client, exported[static_cast<size_t>( i )].user_min + 0.25, exported[static_cast<size_t>( i )].user_max + 0.5 );
    }
    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "KVS Default" ) );
    selectStatistic( client, 1 );
    applyOpacityExpression( client, QStringLiteral( "1-x" ) );
    selectStatistic( client, 1 );
    const int index_before = client.statistic_combo_box->currentIndex();
    const double server_min_before = client.server_min_spin_box->value();
    const double server_max_before = client.server_max_spin_box->value();

    importSettings( client, m_item_export_path );
    verifyNoWarning( result, k_no_warning_wait_ms );

    check( result, client.statistic_combo_box->currentIndex() == index_before, QStringLiteral( "Import で Statistic の選択が変わった" ) );
    check( result, client.variable_line_edit->text() == exported_variable, QStringLiteral( "Variable が Export 時の値に復元されない" ) );
    check(
        result,
        std::abs( client.server_min_spin_box->value() - server_min_before ) < 1.0e-9 && std::abs( client.server_max_spin_box->value() - server_max_before ) < 1.0e-9,
        QStringLiteral( "Server Side Min Max が Import で変化した" ) );
    for( int i = 0; i < 3; ++i )
    {
        const StatisticState state = captureStatisticState( client, i );
        QString why;
        check( result, statesEqual( state, exported[static_cast<size_t>( i )], &why ), QStringLiteral( "%1 が Export 時の値に復元されない(%2)" ).arg( state.statistic, why ) );
    }
    selectStatistic( client, index_before );
    captureManual( result, client.ensemble_editor, 1, QStringLiteral( "editor_imported" ), manualEtf( 13, 1 ) );

    ParticleUpdateResult update;
    const bool applied = applyAndWait( ctx, index_before, true, &update );
    check( result, applied, QStringLiteral( "Import 後の Apply で RequestDataAt 応答が Export JSON の値と一致しないか粒子が変化しない" ) );
    if( applied )
    {
        bringWindowToFront( client.main_window );
        captureManual( result, client.main_window, 2, QStringLiteral( "particles_imported" ), manualEtf( 13, 2 ), update.request_time_step );
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-14 Variable 空文字の警告
// ===========================================================================
void EnsembleTransferFunctionTest::runItem14EmptyVariable( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-14" );
    result.title = QStringLiteral( "Variableを空にしてApplyする" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;

    client.variable_line_edit->clear();
    QCoreApplication::processEvents();
    check( result, client.variable_line_edit->text().isEmpty(), QStringLiteral( "Variable を空にできない" ) );

    const int parameter_start = ctx.parameter_spy->count();
    expectWarning(
        result,
        [client]() { QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton ); },
        QStringLiteral( "Synthesizer expression is empty." ),
        1, QStringLiteral( "warning_empty_variable" ), manualEtf( 14, 1 ) );
    check( result, waitForNoParameter( *ctx.parameter_spy, k_no_send_wait_ms ) && ctx.parameter_spy->count() == parameter_start,
           QStringLiteral( "空の Variable で Apply したのにサーバへ送信された(再配信を受信)" ) );

    expectWarning(
        result,
        [client]() { QTest::mouseClick( client.ensemble_export_button, Qt::LeftButton ); },
        QStringLiteral( "Synthesizer expression is empty." ),
        0, QString(), QString() );

    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-15 Min > Max の警告
// ===========================================================================
void EnsembleTransferFunctionTest::runItem15MinMaxInvalid( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-15" );
    result.title = QStringLiteral( "Min > MaxでApplyする" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;

    selectStatistic( client, 0 );
    if( !client.user_min_max_radio->isChecked() ) QTest::mouseClick( client.user_min_max_radio, Qt::LeftButton );
    client.user_min_spin_box->setValue( 1.0 );
    client.user_max_spin_box->setValue( 0.0 );
    QCoreApplication::processEvents();
    check( result, client.user_min_spin_box->value() > client.user_max_spin_box->value(), QStringLiteral( "Min > Max を設定できない" ) );

    expectWarning(
        result,
        [client]() { QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton ); },
        QStringLiteral( "Average MinMax is invalid." ),
        1, QStringLiteral( "warning_apply_minmax" ), manualEtf( 15, 1 ) );
    expectWarning(
        result,
        [client]() { QTest::mouseClick( client.ensemble_export_button, Qt::LeftButton ); },
        QStringLiteral( "Average User MinMax is invalid." ),
        2, QStringLiteral( "warning_export_minmax" ), manualEtf( 15, 2 ) );

    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-17 不正な JSON の Import
// ===========================================================================
void EnsembleTransferFunctionTest::runItem17InvalidJson( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-17" );
    result.title = QStringLiteral( "不正なJSONをImportする" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;

    QString invalid_path = configuredPathOrEmpty( "ENSEMBLE_TF_INVALID_JSON" );
    if( invalid_path.isEmpty() || !QFileInfo::exists( invalid_path ) )
    {
        invalid_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "invalid.json" ) );
        QFile file( invalid_path );
        if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) ) file.write( "{ this is not json ]\n" );
    }
    QString array_path = configuredPathOrEmpty( "ENSEMBLE_TF_ARRAY_JSON" );
    if( array_path.isEmpty() || !QFileInfo::exists( array_path ) )
    {
        array_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "array.json" ) );
        QFile file( array_path );
        if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) ) file.write( "[1, 2, 3]\n" );
    }

    selectStatistic( client, 1 );
    const QString variable_before = client.variable_line_edit->text();
    const int index_before = client.statistic_combo_box->currentIndex();
    std::array<StatisticState, 3> before = captureAllStatisticStates( client );
    selectStatistic( client, index_before );

    const auto verify_unchanged = [&]( const QString& label )
    {
        check( result, client.variable_line_edit->text() == variable_before, QStringLiteral( "%1: Variable が変化した" ).arg( label ) );
        check( result, client.statistic_combo_box->currentIndex() == index_before, QStringLiteral( "%1: Statistic の選択が変化した" ).arg( label ) );
        for( int i = 0; i < 3; ++i )
        {
            const StatisticState after = captureStatisticState( client, i );
            QString why;
            check( result, statesEqual( after, before[static_cast<size_t>( i )], &why ) &&
                               std::abs( after.server_min - before[static_cast<size_t>( i )].server_min ) < 1.0e-9 &&
                               std::abs( after.server_max - before[static_cast<size_t>( i )].server_max ) < 1.0e-9,
                   QStringLiteral( "%1: %2 の GUI 値が変化した(%3)" ).arg( label, after.statistic, why ) );
        }
        selectStatistic( client, index_before );
    };

    const int parameter_start = ctx.parameter_spy->count();
    expectWarning(
        result,
        [this, client, invalid_path]() { importSettings( client, invalid_path ); },
        QStringLiteral( "Invalid JSON file." ),
        1, QStringLiteral( "warning_invalid_json" ), manualEtf( 17, 1 ) );
    verify_unchanged( QStringLiteral( "不正 JSON" ) );
    captureManual( result, client.ensemble_editor, 2, QStringLiteral( "editor_unchanged" ), manualEtf( 17, 2 ) );

    expectWarning(
        result,
        [this, client, array_path]() { importSettings( client, array_path ); },
        QStringLiteral( "Invalid JSON file." ),
        0, QString(), QString() );
    verify_unchanged( QStringLiteral( "配列 JSON" ) );
    check( result, waitForNoParameter( *ctx.parameter_spy, k_no_send_wait_ms ) && ctx.parameter_spy->count() == parameter_start,
           QStringLiteral( "Import 失敗時にサーバへ送信された(再配信を受信)" ) );
    finishItem( result );
}

// ===========================================================================
// C-ETF-18 キー欠損 JSON の Import(セクション欠損 / 統計内の部分反映 / フォールバック)
// ===========================================================================
void EnsembleTransferFunctionTest::runItem18MissingKeys( ItemContext& ctx )
{
    ItemResult result;
    result.id = QStringLiteral( "C-ETF-18" );
    result.title = QStringLiteral( "キーが欠損したJSONをImportする" );
    restoreBaseline( ctx );
    const ClientHandles& client = ctx.client;
    const QString warning_text = QStringLiteral( "Some default.json keys were missing. Existing GUI values were kept for those fields." );

    bool ok = false;
    const QJsonObject export_root = readJsonObject( m_item_export_path, &ok );
    if( !check( result, ok, QStringLiteral( "C-ETF-12 の Export JSON を読めない: %1" ).arg( m_item_export_path ) ) )
    {
        finishItem( result );
        return;
    }
    const QJsonObject mean_tf = export_root.value( QStringLiteral( "mean_transfer_functions" ) ).toArray().first().toObject();
    const QJsonObject variance_tf = export_root.value( QStringLiteral( "variance_transfer_functions" ) ).toArray().first().toObject();
    const QJsonObject cv_tf = export_root.value( QStringLiteral( "coefficient_of_variation_transfer_functions" ) ).toArray().first().toObject();
    const QVector<QColor> json_mean_colors = colorsFromDefaultJsonMap( mean_tf.value( QStringLiteral( "color" ) ).toObject().value( QStringLiteral( "map" ) ).toObject() );
    const QVector<QColor> json_variance_colors = colorsFromDefaultJsonMap( variance_tf.value( QStringLiteral( "color" ) ).toObject().value( QStringLiteral( "map" ) ).toObject() );
    const QVector<float> json_variance_opacities = opacitiesFromDefaultJsonMap( variance_tf.value( QStringLiteral( "opacity" ) ).toObject().value( QStringLiteral( "map" ) ).toObject() );
    const QVector<QColor> json_cv_colors = colorsFromDefaultJsonMap( cv_tf.value( QStringLiteral( "color" ) ).toObject().value( QStringLiteral( "map" ) ).toObject() );

    // 入力ファイルの用意
    QString partial_path = configuredPathOrEmpty( "ENSEMBLE_TF_PARTIAL_JSON" );
    if( partial_path.isEmpty() || !QFileInfo::exists( partial_path ) )
    {
        partial_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "partial_no_variance.json" ) );
        writeJsonObject( partial_path, removeSection( export_root, QStringLiteral( "variance_transfer_functions" ) ) );
    }
    QString partial_range_path = configuredPathOrEmpty( "ENSEMBLE_TF_PARTIAL_RANGE_JSON" );
    if( partial_range_path.isEmpty() || !QFileInfo::exists( partial_range_path ) )
    {
        partial_range_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "partial_no_variance_range.json" ) );
        writeJsonObject( partial_range_path, removeVarianceColorRange( export_root ) );
    }
    const QString fallback_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "partial_no_mean.json" ) );
    writeJsonObject( fallback_path, removeSection( export_root, QStringLiteral( "mean_transfer_functions" ) ) );

    // ---- (A) セクション欠損 ----
    selectStatistic( client, 1 );
    setUserRange( client, 0.1, 0.9 );
    const StatisticState variance_before = captureStatisticState( client, 1 );
    selectStatistic( client, 1 );
    bool inspected_during_warning = false;
    bool average_updated_during_warning = false;
    expectWarning(
        result,
        [this, client, partial_path]() { importSettings( client, partial_path ); },
        warning_text,
        1, QStringLiteral( "warning_missing_section" ), manualEtf( 18, 1 ),
        [&]()
        {
            // 警告表示中に既に値が反映されていること(Average を選択して GUI を確認)
            inspected_during_warning = true;
            const int current = client.statistic_combo_box->currentIndex();
            client.statistic_combo_box->setCurrentIndex( 0 );
            QCoreApplication::processEvents();
            average_updated_during_warning = client.color_map->getColors() == json_mean_colors;
            client.statistic_combo_box->setCurrentIndex( current );
            QCoreApplication::processEvents();
        } );
    check( result, inspected_during_warning && average_updated_during_warning, QStringLiteral( "(A) 警告表示中に Average が JSON の値に更新されていない" ) );
    {
        const StatisticState variance_after = captureStatisticState( client, 1 );
        QString why;
        check( result, statesEqual( variance_after, variance_before, &why ), QStringLiteral( "(A) Variance の GUI 値が維持されていない(%1)" ).arg( why ) );
        const StatisticState average_after = captureStatisticState( client, 0 );
        check( result, average_after.colors == json_mean_colors, QStringLiteral( "(A) Average の Color Map が JSON の値に復元されていない" ) );
        const StatisticState cv_after = captureStatisticState( client, 2 );
        check( result, cv_after.colors == json_cv_colors, QStringLiteral( "(A) Coefficient of Variation の Color Map が JSON の値に復元されていない" ) );
        selectStatistic( client, 1 );
        captureManual( result, client.ensemble_editor, 2, QStringLiteral( "variance_kept" ), manualEtf( 18, 2 ) );
    }

    // ---- (B) 統計内の部分反映(variance の color.range 欠損) ----
    selectStatistic( client, 1 );
    setUserRange( client, 0.1, 0.9 );
    applyColorPreset( client, QStringLiteral( "KVS Default" ) );
    if( client.color_map->getColors() == json_variance_colors ) applyColorPreset( client, QStringLiteral( "Grayscale" ) );
    check( result, client.color_map->getColors() != json_variance_colors, QStringLiteral( "(B) Variance の Color Map を JSON と異なる値にできない" ) );
    expectWarning(
        result,
        [this, client, partial_range_path]() { importSettings( client, partial_range_path ); },
        warning_text,
        3, QStringLiteral( "warning_missing_range" ), manualEtf( 18, 3 ) );
    {
        const StatisticState variance_after = captureStatisticState( client, 1 );
        check( result, variance_after.colors == json_variance_colors, QStringLiteral( "(B) Variance の Color Map が JSON の値に復元されていない" ) );
        check( result, opacitiesNearlyEqual( variance_after.opacities, json_variance_opacities ), QStringLiteral( "(B) Variance の Opacity Map が JSON の値に復元されていない" ) );
        check(
            result,
            std::abs( variance_after.user_min - 0.1 ) < 1.0e-7 && std::abs( variance_after.user_max - 0.9 ) < 1.0e-7,
            QStringLiteral( "(B) Variance の User MinMax が 0.1 / 0.9 のまま維持されていない(%1 / %2)" ).arg( variance_after.user_min ).arg( variance_after.user_max ) );
        const StatisticState average_after = captureStatisticState( client, 0 );
        check( result, average_after.colors == json_mean_colors, QStringLiteral( "(B) Average が JSON の値になっていない" ) );
        const StatisticState cv_after = captureStatisticState( client, 2 );
        check( result, cv_after.colors == json_cv_colors, QStringLiteral( "(B) Coefficient of Variation が JSON の値になっていない" ) );
        selectStatistic( client, 1 );
        captureManual( result, client.ensemble_editor, 4, QStringLiteral( "variance_partial" ), manualEtf( 18, 4 ) );
    }

    // ---- (C) フォールバック(mean_transfer_functions 欠損 → transfer_functions) ----
    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "Grayscale" ) );
    if( client.color_map->getColors() == json_mean_colors ) applyColorPreset( client, QStringLiteral( "KVS Default" ) );
    importSettings( client, fallback_path );
    verifyNoWarning( result, k_no_warning_wait_ms );
    {
        const StatisticState average_after = captureStatisticState( client, 0 );
        const QJsonObject normal_tf = export_root.value( QStringLiteral( "transfer_functions" ) ).toArray().first().toObject();
        const QVector<QColor> json_normal_colors = colorsFromDefaultJsonMap( normal_tf.value( QStringLiteral( "color" ) ).toObject().value( QStringLiteral( "map" ) ).toObject() );
        check( result, average_after.colors == json_normal_colors, QStringLiteral( "(C) Average が transfer_functions[0] の値になっていない(フォールバック)" ) );
    }
    restoreBaseline( ctx );
    finishItem( result );
}

// ===========================================================================
// C-ETF-19 / C-ETF-16 切断・再接続とリセット、未接続 Apply
// ===========================================================================
void EnsembleTransferFunctionTest::runItem19And16DisconnectReconnect( ItemContext& ctx, MainWindow& window )
{
    ItemResult result19;
    result19.id = QStringLiteral( "C-ETF-19" );
    result19.title = QStringLiteral( "切断・再接続時にリセットされることを確認する" );
    ItemResult result16;
    result16.id = QStringLiteral( "C-ETF-16" );
    result16.title = QStringLiteral( "未接続状態でApplyする" );
    const ClientHandles& client = ctx.client;

    // C-ETF-10 の状態で Apply し、サーバ再配信 payload を記録
    restoreBaseline( ctx );
    selectStatistic( client, 0 );
    applyColorPreset( client, QStringLiteral( "Traffic Lights Step" ) );
    selectStatistic( client, 1 );
    applyOpacityExpression( client, QStringLiteral( "0.5*x" ) );
    selectStatistic( client, 2 );
    setUserRange( client, 0.0, 2.5 );
    const std::array<StatisticState, 3> applied_states = captureAllStatisticStates( client );
    const QString applied_variable = client.variable_line_edit->text().trimmed();
    ParticleUpdateResult update;
    const bool applied = applyAndWait( ctx, 0, false, &update );
    check( result19, applied, QStringLiteral( "切断前の Apply が反映されない" ) );

    // 切断
    QAction* ensemble_action = findAction( &window, QStringLiteral( "Ensemble Transfer Function Editor" ), false );
    bringWindowToFront( client.main_window );
    QTest::mouseClick( client.disconnect_button, Qt::LeftButton );
    const bool disconnected = waitForCondition( [client]() { return client.connect_button->isEnabled() && !client.disconnect_button->isEnabled(); }, k_connect_timeout_ms, 100 );
    check( result19, disconnected, QStringLiteral( "Disconnect 後に未接続状態にならない" ) );
    QTest::qWait( k_short_wait_ms );
    check( result19, !client.ensemble_editor->isVisible(), QStringLiteral( "切断後もエディタが表示されたまま" ) );
    check( result19, ensemble_action == nullptr || !ensemble_action->isEnabled(), QStringLiteral( "切断後も「Ensemble Transfer Function Editor」が有効" ) );

    // reset 直後の状態(エディタを show して確認)
    client.ensemble_editor->show();
    client.ensemble_editor->raise();
    QTest::qWait( k_short_wait_ms );
    check( result19, client.variable_line_edit->text() == QStringLiteral( "q1" ), QStringLiteral( "reset 後の Variable が q1 ではない: %1" ).arg( client.variable_line_edit->text() ) );
    for( int i = 0; i < 3; ++i )
    {
        selectStatistic( client, i );
        check( result19,
               std::abs( client.user_min_spin_box->value() - 0.0 ) < 1.0e-9 && std::abs( client.user_max_spin_box->value() - 1.0 ) < 1.0e-9 &&
                   std::abs( client.server_min_spin_box->value() - 0.0 ) < 1.0e-9 && std::abs( client.server_max_spin_box->value() - 1.0 ) < 1.0e-9,
               QStringLiteral( "reset 後の統計 %1 の MinMax が 0/1 ではない" ).arg( i ) );
        if( client.histogram ) check( result19, client.histogram->getDatas().empty(), QStringLiteral( "reset 後の統計 %1 の Histogram が空ではない" ).arg( i ) );
    }
    selectStatistic( client, 0 );

    // C-ETF-16: 未接続で Apply
    expectWarning(
        result16,
        [client]() { QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton ); },
        QStringLiteral( "WebSocket is not connected." ),
        1, QStringLiteral( "warning_not_connected" ), manualEtf( 16, 1 ) );
    client.ensemble_editor->close();
    finishItem( result16 );

    // 再接続
    const int parameter_start = ctx.parameter_spy->count();
    connectClient( client );
    configureInSituVisualization( client );
    QString variable_after;
    std::array<StatisticState, 3> states_after;
    bool received = false;
    {
        int inspected = parameter_start;
        received = waitForCondition(
            [&]()
            {
                while( inspected < ctx.parameter_spy->count() )
                {
                    const QJsonObject payload = qvariant_cast<QJsonObject>( ctx.parameter_spy->at( inspected++ ).at( 0 ) );
                    if( extractSettingsFromPayload( payload, &variable_after, &states_after ) ) return true;
                }
                return false;
            },
            k_object_load_timeout_ms,
            100 );
    }
    check( result19, received, QStringLiteral( "再接続後に Initialize の EnsembleStatisticsParameter を受信しない" ) );
    waitForObjectAndApply( client );
    if( ensemble_action )
    {
        check( result19, waitForCondition( [ensemble_action]() { return ensemble_action->isEnabled(); }, k_object_load_timeout_ms, 100 ),
               QStringLiteral( "再接続後に「Ensemble Transfer Function Editor」が有効にならない" ) );
    }
    bringEditorToFront( client.ensemble_editor );
    check( result19, client.variable_line_edit->text().trimmed() == applied_variable, QStringLiteral( "再接続後の Variable が切断前の Apply 値と一致しない" ) );
    for( int i = 0; i < 3; ++i )
    {
        const StatisticState state = captureStatisticState( client, i );
        QString why;
        check( result19, statesEqual( state, applied_states[static_cast<size_t>( i )], &why ), QStringLiteral( "再接続後の %1 が切断前の Apply 値と一致しない(%2)" ).arg( state.statistic, why ) );
    }
    selectStatistic( client, 0 );
    captureManual( result19, client.ensemble_editor, 1, QStringLiteral( "editor_reconnected" ), manualEtf( 19, 1 ) );

    // 後続のために粒子を再生成し Keep Last を戻す
    generateInitialParticles( client, *ctx.completion_spy );
    enableKeepLast( client );
    restoreBaseline( ctx );
    finishItem( result19 );
}

// ===========================================================================
// 項目の実行順序
// ===========================================================================
void EnsembleTransferFunctionTest::runCheckSheetItems( ItemContext& ctx, MainWindow& window )
{
    runItem01Variable( ctx );
    runItem02Statistic( ctx );
    runItem03ColorMap( ctx );
    runItem04OpacityMap( ctx );
    runItem05Histogram( ctx );
    runItem06And08RangeMode( ctx );
    runItem07UserMinMax( ctx );
    runItem09ServerMinMax( ctx );
    runItem10Apply( ctx );
    runItem11SecondClient( ctx );
    runItem12Export( ctx );
    runItem13Import( ctx );
    runItem14EmptyVariable( ctx );
    runItem15MinMaxInvalid( ctx );
    runItem17InvalidJson( ctx );
    runItem18MissingKeys( ctx );
    runItem19And16DisconnectReconnect( ctx, window );
}

void EnsembleTransferFunctionTest::writeCheckSheetSections( QTextStream& stream ) const
{
    if( m_item_results.empty() ) return;

    stream << "\n## チェックシート項目結果\n\n";
    stream << "- 使用データ: アンサンブルシミュレーション(ens_Hydrogen_unstruct)、IS モード。VIS_PARAM_DIR: `"
           << ( m_vis_param_dir.isEmpty() ? QStringLiteral( "(未設定)" ) : m_vis_param_dir ) << "`\n";
    stream << "- Export JSON(C-ETF-12): `" << m_item_export_path << "`\n\n";
    stream << "| 項目ID | 内容 | 結果 | 備考 |\n|---|---|---|---|\n";
    for( const ItemResult& item : m_item_results )
    {
        QString notes = item.messages.join( QStringLiteral( "<br>" ) );
        notes.replace( QLatin1Char( '|' ), QStringLiteral( "\\|" ) );
        stream << "| " << item.id << " | " << item.title << " | " << item.status << " | " << notes << " |\n";
    }

    stream << "\n## マニュアル用画像\n\n";
    stream << "| 項目ID | 連番 | テスト用ファイル | マニュアル用コピー先 | タイムステップ | 状態 |\n|---|---|---|---|---|---|\n";
    for( const ManualImage& image : m_manual_images )
    {
        stream << "| " << image.item_id << " | " << image.seq << " | `img/" << image.test_file_name << "` | `img/manual/"
               << image.manual_rel_path << "` | " << ( image.time_step >= 0 ? QString::number( image.time_step ) : QStringLiteral( "-" ) )
               << " | " << ( image.copied ? "出力済み" : "未出力" ) << " |\n";
    }
    QStringList missing;
    for( const ManualImage& image : m_manual_images )
    {
        if( !image.copied ) missing << image.manual_rel_path;
    }
    stream << "\n- 出力できなかった(SKIP した)マニュアル用画像: " << ( missing.isEmpty() ? QStringLiteral( "なし" ) : missing.join( QStringLiteral( ", " ) ) ) << "\n";
    stream << "- 対応表にあるが本テストでは撮影しない画像(2クライアント等の SKIP 項目)は手動で撮影して同名で配置する\n";
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    ClientTests::g_ensemble_test_app = &app;
    ClientTests::EnsembleTransferFunctionTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
