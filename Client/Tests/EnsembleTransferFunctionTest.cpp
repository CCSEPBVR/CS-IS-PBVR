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

#include <cmath>

#include <kvs/qt/Application>
#include <kvs/PointObject>

#include "../App/MainWindow.h"
#include "../ExtendedQT/ColorMap.h"
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
    stream << "- 画像ピクセルの自動比較: 実施しない（粒子配列のフィンガープリントで更新を確認）\n\n";

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

bool EnsembleTransferFunctionTest::responseMatchesExpectedSettings(
    const QJsonObject& payload,
    const std::array<StatisticState, 3>& expected,
    const QString& expected_variable ) const
{
    const QJsonArray data = payload.value( QString::fromUtf8( Protocol::Key::Data ) ).toArray();
    if( data.size() != 3 ) return false;

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
        if( patch.isEmpty() ) return false;
        if( patch.value( QString::fromUtf8( Protocol::Key::EnsembleVariable ) ).toString() != expected_variable ) return false;
        if( patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMode ) ).toInt() != ( state.use_user_range ? 1 : 2 ) ) return false;
        if( std::abs( patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMin ) ).toDouble() - state.user_min ) > 1.0e-7 ) return false;
        if( std::abs( patch.value( QString::fromUtf8( Protocol::Key::EnsembleUserRangeMax ) ).toDouble() - state.user_max ) > 1.0e-7 ) return false;

        const QJsonValue server_min_value =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleServerRangeMin ) );
        const QJsonValue server_max_value =
            patch.value( QString::fromUtf8( Protocol::Key::EnsembleServerRangeMax ) );
        if( !server_min_value.isDouble() || !server_max_value.isDouble() ) return false;
        const double server_min = server_min_value.toDouble();
        const double server_max = server_max_value.toDouble();
        if( !std::isfinite( server_min ) || !std::isfinite( server_max ) || server_min > server_max ) return false;
        if( !isRoundedToEightDecimalPlaces( server_min ) ||
            !isRoundedToEightDecimalPlaces( server_max ) )
        {
            return false;
        }

        const QJsonValue histogram = patch.value( QString::fromUtf8( Protocol::Key::EnsembleHistogram ) );
        if( !histogram.isArray() || histogram.toArray().isEmpty() ) return false;

        const QJsonArray colors = patch.value( QString::fromUtf8( Protocol::Key::EnsembleColorMap ) ).toArray();
        if( colors.size() != state.colors.size() ) return false;
        for( int index = 0; index < colors.size(); ++index )
        {
            const QJsonArray rgb = colors.at( index ).toArray();
            if( rgb.size() != 3 ||
                rgb.at( 0 ).toInt() != state.colors.at( index ).red() ||
                rgb.at( 1 ).toInt() != state.colors.at( index ).green() ||
                rgb.at( 2 ).toInt() != state.colors.at( index ).blue() )
            {
                return false;
            }
        }

        const QJsonArray opacities = patch.value( QString::fromUtf8( Protocol::Key::EnsembleOpacityMap ) ).toArray();
        if( opacities.size() != state.opacities.size() ) return false;
        for( int index = 0; index < opacities.size(); ++index )
        {
            if( std::abs( opacities.at( index ).toDouble() - state.opacities.at( index ) ) > 1.0e-6 ) return false;
        }
    }
    return true;
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
    if( result == nullptr || result->before_fingerprint.isEmpty() ) return false;
    if( !client.keep_last_button->isChecked() ) return false;
    QTest::mouseClick( client.ensemble_apply_button, Qt::LeftButton );
    QTest::qWait( k_apply_propagation_wait_ms );
    if( !client.keep_last_button->isChecked() ) return false;

    request_spy.clear();
    completion_spy.clear();
    request_parameter_spy.clear();

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
            return false;
        }

        const int request_time_step = request_spy.at( request_start ).at( 0 ).toInt();
        ++result->request_count;

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
        if( !response_completed ) return false;

        bool settings_match = false;
        for( int index = parameter_start; index < request_parameter_spy.count(); ++index )
        {
            const QJsonObject payload = qvariant_cast<QJsonObject>( request_parameter_spy.at( index ).at( 0 ) );
            if( responseMatchesExpectedSettings( payload, expected, expected_variable ) )
            {
                settings_match = true;
                break;
            }
        }

        const QByteArray fingerprint = particleFingerprint( client );
        if( fingerprint.isEmpty() ) return false;
        if( !settings_match || ( require_particle_change && fingerprint == result->before_fingerprint ) )
        {
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
        if( !result->paint_completed || !client.keep_last_button->isChecked() ) return false;
        return true;
    }

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
    if( !update_result.paint_completed || update_result.after_fingerprint.isEmpty() ) return false;
    saveScreenshotPair( client, file_name_prefix, title, change_description, update_result );
    return client.keep_last_button->isChecked();
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
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), "Failed to create EnsembleTransferFunctionTest output directory" );

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
    bringEditorToFront( client.ensemble_editor );

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
        QVERIFY2( applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_unchanged" ),
            statistic_title + QStringLiteral( " — 変更なし" ),
            QStringLiteral( "初回EnsembleStatisticsParameterで受信した基準状態（変更なし）" ),
            index != 0 ), "Unchanged case did not receive matching RequestDataAt settings and particles" );

        restoreSettings( client, initial_variable, baseline_states );
        setLineEditText( client.variable_line_edit, changed_variable );
        QVERIFY2( applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            changed_variable,
            file_name + QStringLiteral( "_variable" ),
            statistic_title + QStringLiteral( " — Variable" ),
            QStringLiteral( "Variableのみ `%1` から `%2` へ変更" ).arg( initial_variable, changed_variable ) ),
            "Variable case did not receive matching RequestDataAt settings and changed particles" );

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        const double user_min = user_min_values[static_cast<size_t>( index )];
        const double user_max = user_max_values[static_cast<size_t>( index )];
        setUserRange( client, user_min, user_max );
        QVERIFY2(
            std::abs( user_min - baseline.user_min ) > 1.0e-7 ||
                std::abs( user_max - baseline.user_max ) > 1.0e-7 ||
                !baseline.use_user_range,
            "User MinMax case did not differ from the baseline" );
        QVERIFY2( applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_user_minmax" ),
            statistic_title + QStringLiteral( " — User MinMax" ),
            QStringLiteral( "User MinMaxのみ %1 / %2 へ変更" ).arg( user_min ).arg( user_max ) ),
            "User MinMax case did not receive matching RequestDataAt settings and changed particles" );

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        QVERIFY2( baseline.use_user_range, "Server MinMax case requires a User MinMax baseline" );
        setServerRange( client );
        QVERIFY2( applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_server_minmax" ),
            statistic_title + QStringLiteral( " — Server MinMax" ),
            QStringLiteral( "MinMaxモードのみServerへ変更（%1 / %2）" ).arg( baseline.server_min ).arg( baseline.server_max ) ),
            "Server MinMax case did not receive matching RequestDataAt settings and changed particles" );

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
        QVERIFY2( client.color_map->getColors() != baseline.colors, "Color Map case did not differ from the baseline" );
        QVERIFY2( applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_color_map" ),
            statistic_title + QStringLiteral( " — Color Map" ),
            QStringLiteral( "Color Mapのみプリセット `%1` へ変更" ).arg( applied_preset ) ),
            "Color Map case did not receive matching RequestDataAt settings and changed particles" );

        restoreSettings( client, initial_variable, baseline_states );
        selectStatistic( client, index );
        const QString applied_opacity_expression = opacity_expressions[static_cast<size_t>( index )];
        applyOpacityExpression( client, applied_opacity_expression );
        QVERIFY2( client.opacity_map->getOpacities() != baseline.opacities, "Opacity Map case did not differ from the baseline" );
        QVERIFY2( applyAndCaptureCase(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            index,
            initial_variable,
            file_name + QStringLiteral( "_opacity_map" ),
            statistic_title + QStringLiteral( " — Opacity Map" ),
            QStringLiteral( "Opacity Mapのみ式 `%1` へ変更" ).arg( applied_opacity_expression ) ),
            "Opacity Map case did not receive matching RequestDataAt settings and changed particles" );
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
    QVERIFY2(
        waitForAppliedParticles(
            client,
            request_spy,
            completion_spy,
            request_parameter_spy,
            imported_states,
            changed_variable,
            true,
            &import_update_result ),
        "Import case did not receive matching RequestDataAt settings and changed particles" );
    QVERIFY2(
        import_update_result.paint_completed && !import_update_result.after_fingerprint.isEmpty(),
        "Imported particle replacement was not confirmed; screenshot capture was skipped" );
    saveScreenshotPair(
        client,
        QStringLiteral( "import_restored" ),
        QStringLiteral( "Import復元後" ),
        QStringLiteral( "Export JSONに記録した3統計の設定をImportして復元" ),
        import_update_result );
    markStepCompleted( QStringLiteral( "Import 復元値を Apply し、5秒待機後の粒子生成を確認しました。" ) );

    QVERIFY2( client.keep_last_button->isChecked(), "Keep Last was not maintained until the end of the test" );
    m_test_succeeded = true;
    client.render_screen->removeEventFilter( this );
    m_render_screen_object = nullptr;
    main_window.close();
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, k_window_settle_ms );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
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
