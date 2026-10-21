#include "PlotOverTimeEditorTest.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <string_view>

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSignalSpy>
#include <QSpinBox>
#include <QTabWidget>
#include <QTest>
#include <QTextStream>

#include "../App/MainWindow.h"
#include "../Widgets/Communication.h"
#include "../Widgets/GlyphEditor.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/RepetitionLevelControl.h"
#include "../Widgets/TimeStepControlToolBar.h"
#include "../Widgets/VolumeTransform.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

// Keep qcustomplot (included by PlotOverTimeEditor) after the KVS/GL headers.
#include "../Widgets/PlotOverTimeEditor.h"

namespace
{
constexpr int k_connection_timeout_ms = 15000;
constexpr int k_data_timeout_ms = 120000;
constexpr int k_calculation_timeout_ms = 300000;
constexpr int k_window_settle_ms = 500;
constexpr int k_capture_settle_ms = 700;
kvs::qt::Application* g_test_app = nullptr;

QString key( std::string_view value )
{
    return QString::fromUtf8( value );
}

QString findRepoRootFrom( const QString& start_path )
{
    QDir dir( start_path );
    do
    {
        if ( dir.exists( ".git" ) && dir.exists( "Client" ) && dir.exists( "Server" ) )
            return dir.absolutePath();
    }
    while ( dir.cdUp() );
    return QString();
}

QString repoRootPath()
{
    for ( const QString& start : { QCoreApplication::applicationDirPath(), QDir::currentPath(),
                                   QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() } )
    {
        const QString root = findRepoRootFrom( start );
        if ( !root.isEmpty() ) return root;
    }
    return QDir::currentPath();
}

QString envOrConfiguredPath( const char* name, const QString& root, const QString& fallback = QString() )
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, root, fallback ) : value;
}

bool waitForCondition( const std::function<bool()>& condition, int timeout_ms )
{
    QElapsedTimer timer;
    timer.start();
    while ( !condition() && timer.elapsed() < timeout_ms ) QTest::qWait( 50 );
    return condition();
}

bool openTool( MainWindow& window, QWidget* tool, const QString& action_text )
{
    for ( QAction* action : window.findChildren<QAction*>() )
    {
        if ( action->text() != action_text ) continue;
        if ( !waitForCondition( [action]() { return action->isEnabled(); }, k_data_timeout_ms ) ) return false;
        action->trigger();
        tool->raise();
        QTest::qWait( k_window_settle_ms );
        return tool->isVisible();
    }
    return false;
}

bool clickButton( QWidget* parent, const char* name )
{
    auto* button = parent->findChild<QPushButton*>( name );
    if ( !button || !waitForCondition( [button]() { return button->isEnabled() && button->isVisible(); },
                                     k_connection_timeout_ms ) ) return false;
    QTest::mouseClick( button, Qt::LeftButton );
    return true;
}

bool setDouble( QWidget* parent, const char* name, double value )
{
    auto* spin = parent->findChild<QDoubleSpinBox*>( name );
    if ( !spin || !spin->isEnabled() || value < spin->minimum() || value > spin->maximum() ) return false;
    spin->setFocus();
    spin->setValue( value );
    return qAbs( spin->value() - value ) < 1e-9;
}

bool setInteger( QWidget* parent, const char* name, int value )
{
    auto* spin = parent->findChild<QSpinBox*>( name );
    if ( !spin || !spin->isEnabled() || value < spin->minimum() || value > spin->maximum() ) return false;
    spin->setValue( value );
    return spin->value() == value;
}

bool selectRadio( QWidget* parent, const char* name )
{
    auto* radio = parent->findChild<QRadioButton*>( name );
    if ( !radio || !radio->isEnabled() || !radio->isVisible() ) return false;
    if ( !radio->isChecked() ) QTest::mouseClick( radio, Qt::LeftButton );
    return radio->isChecked();
}

bool configureVariables( GlyphEditor* glyph, const char* radio_name,
                         const char* count_name, const char* layout_name )
{
    if ( !selectRadio( glyph, radio_name ) || !setInteger( glyph, count_name, 3 ) ) return false;
    auto* layout = glyph->findChild<QGridLayout*>( layout_name );
    if ( !layout ) return false;
    for ( int row = 1; row <= 2; ++row )
    {
        auto* item = layout->itemAtPosition( row, 1 );
        auto* combo = item ? qobject_cast<QComboBox*>( item->widget() ) : nullptr;
        if ( !combo || !combo->isEnabled() || combo->count() <= row ) return false;
        combo->setCurrentIndex( row );
        if ( combo->currentIndex() != row ) return false;
    }
    return true;
}

// Capture actual CS responses; no private editor state or synthetic results are used.
struct ObservationCapture
{
    QJsonObject applied;
    QMap<int, QJsonObject> samples;
    QString state;
    QString last_payload;
    bool enabled = false;

    void receive( const QJsonObject& payload )
    {
        last_payload = QString::fromUtf8( QJsonDocument( payload ).toJson( QJsonDocument::Compact ) );
        if ( payload.contains( key( Protocol::Key::AppliedObservation ) ) )
        {
            const auto observation = payload.value( key( Protocol::Key::AppliedObservation ) ).toObject();
            if ( applied != observation ) { samples.clear(); state.clear(); }
            applied = observation;
        }
        if ( payload.contains( key( Protocol::Key::Enable ) ) )
            enabled = payload.value( key( Protocol::Key::Enable ) ).toBool();
        if ( payload.value( key( Protocol::Key::Observation ) ).toObject() != applied ) return;
        if ( payload.value( key( Protocol::Key::Snapshot ) ).toBool() ) samples.clear();
        if ( payload.contains( key( Protocol::Key::CalculationState ) ) )
            state = payload.value( key( Protocol::Key::CalculationState ) ).toString();
        for ( const auto& value : payload.value( key( Protocol::Key::Samples ) ).toArray() )
        {
            const auto sample = value.toObject();
            samples[sample.value( key( Protocol::Key::TimeStep ) ).toInt()] = sample;
        }
    }

    bool matches( double z ) const
    {
        const auto coords = applied.value( key( Protocol::Key::Coords ) ).toArray();
        return !applied.value( key( Protocol::Key::DatasetId ) ).toString().isEmpty() &&
               coords.size() == 3 && coords[0].toDouble() == 0.0 && coords[1].toDouble() == 0.0 &&
               qAbs( coords[2].toDouble() - static_cast<double>( static_cast<float>( z ) ) ) < 1e-9;
    }
};

using GraphData = QVector<QPair<double, double>>;

GraphData graphData( QCustomPlot* plot )
{
    GraphData result;
    if ( !plot || plot->graphCount() == 0 ) return result;
    const auto data = plot->graph( 0 )->data();
    for ( auto it = data->constBegin(); it != data->constEnd(); ++it ) result.push_back( { it->key, it->value } );
    return result;
}

bool sameValue( double a, double b )
{
    if ( std::isnan( a ) && std::isnan( b ) ) return true;
    if ( !std::isfinite( a ) || !std::isfinite( b ) ) return false;
    return std::abs( a - b ) <= 1e-6 * std::max( 1.0, std::max( std::abs( a ), std::abs( b ) ) );
}

bool graphMatchesSamples( const GraphData& graph, const ObservationCapture& capture, int variable_index )
{
    if ( variable_index < 0 ) return false;
    if ( !capture.applied.contains( key( Protocol::Key::RangeStart ) ) ||
         !capture.applied.contains( key( Protocol::Key::RangeEnd ) ) ) return false;
    const int first = capture.applied.value( key( Protocol::Key::RangeStart ) ).toInt();
    const int last = capture.applied.value( key( Protocol::Key::RangeEnd ) ).toInt();
    const qint64 expected = qint64( last ) - first + 1;
    if ( expected <= 0 || graph.size() != expected || capture.samples.size() != expected ) return false;
    bool has_finite_value = false;
    for ( int i = 0; i < graph.size(); ++i )
    {
        const int step = static_cast<int>( qint64( first ) + i );
        if ( graph[i].first != step || !capture.samples.contains( step ) ) return false;
        const auto sample = capture.samples.value( step );
        const auto values = sample.value( key( Protocol::Key::ValueOnTime ) ).toArray();
        const QString sample_state = sample.value( key( Protocol::Key::SampleState ) ).toString();
        const bool valid = sample.value( key( Protocol::Key::Mask ) ).toBool( true ) &&
                           ( sample_state.isEmpty() || sample_state == "valid" );
        const double expected_value = valid && variable_index < values.size() && values[variable_index].isDouble()
            ? values[variable_index].toDouble() : std::numeric_limits<double>::quiet_NaN();
        if ( !sameValue( graph[i].second, expected_value ) ) return false;
        has_finite_value = has_finite_value || std::isfinite( graph[i].second );
    }
    return has_finite_value;
}

bool graphChanged( const GraphData& before, const GraphData& after )
{
    // Compare the same steps, treating two missing values as equal.
    if ( before.size() != after.size() || before.isEmpty() ) return false;
    bool changed = false;
    for ( int i = 0; i < before.size(); ++i )
    {
        if ( before[i].first != after[i].first ) return false;
        changed = changed || !sameValue( before[i].second, after[i].second );
    }
    return changed;
}
}

namespace ClientTests
{
void PlotOverTimeEditorTest::initTestCase()
{
    const QString root = repoRootPath();
    m_volume_data_path = envOrConfiguredPath( "MEJ_VOLUME_DATA", root );
    m_transfer_function_path = envOrConfiguredPath( "MEJ_TRANSFER_FUNCTION", root );
    const QString date = QDate::currentDate().toString( "yyyyMMdd" );
    m_output_dir_path = envOrConfiguredPath( "PBVR_TEST_OUTPUT_DIR", root,
        datedTestOutputDir( root, date, "PlotOverTimeEditorTest" ) );
    m_screenshot_dir_path = envOrConfiguredPath( "PBVR_SCREENSHOT_DIR", root,
        QDir( m_output_dir_path ).absoluteFilePath( "img" ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( "TestResult.md" );
    QVERIFY2( QDir().mkpath( m_output_dir_path ), "Failed to create test output directory" );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), "Failed to create screenshot directory" );
    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( "MEJ_VOLUME_DATA not found: " + m_volume_data_path ) );
    QVERIFY2( QFileInfo::exists( m_transfer_function_path ), qPrintable( "MEJ_TRANSFER_FUNCTION not found: " + m_transfer_function_path ) );
}

void PlotOverTimeEditorTest::cleanupTestCase()
{
    if ( !m_report_path.isEmpty() && QDir( m_output_dir_path ).exists() ) writeMarkdownReport();
}

void PlotOverTimeEditorTest::saveScreenshot( const QString& file_name, const QString& caption )
{
    if ( !screenshotsEnabled() ) return;
    QTest::qWait( k_capture_settle_ms );
    auto* screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen, "Primary screen not found" );
    const QPixmap screenshot = screen->grabWindow( 0 );
    QVERIFY2( !screenshot.isNull(), "Failed to capture primary screen" );
    QVERIFY2( screenshot.save( QDir( m_screenshot_dir_path ).absoluteFilePath( file_name ) ),
              "Failed to save screenshot" );
    m_screenshots.push_back( { file_name, caption } );
}

void PlotOverTimeEditorTest::writeMarkdownReport() const
{
    QFile file( m_report_path );
    QVERIFY2( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ), "Failed to open test report" );
    QTextStream out( &file );
    out << "# PlotOverTimeEditorTest\n\n";
    out << "- 自動テスト結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    out << "- モード: CS（サーバーは事前起動）\n";
    out << "- MEJ_VOLUME_DATA: `" << m_volume_data_path << "`\n";
    out << "- MEJ_TRANSFER_FUNCTION: `" << m_transfer_function_path << "`\n";
    out << "- スクリーンショット: " << ( screenshotsEnabled() ? "有効" : "無効" ) << "\n\n";
    out << "## 実施手順・自動判定\n\n";
    for ( const auto& step : m_steps ) out << "- " << step.result << ": " << step.description << "\n";
    if ( m_steps.isEmpty() ) out << "- NOT RUN: シナリオ開始前に終了。\n";
    out << "\n## 目視確認\n\n";
    out << "黄色の球の表示・移動方向とグラフの見え方は、以下の画像で確認してください。\n";
    out << "自動テストのPASSは、目視確認の完了を意味しません。\n\n";
    for ( const auto& shot : m_screenshots )
    {
        const QString path = QDir( m_output_dir_path ).relativeFilePath(
            QDir( m_screenshot_dir_path ).absoluteFilePath( shot.file_name ) );
        out << "### 要確認: " << shot.caption << "\n\n";
        out << "![" << shot.caption << "](" << QDir::fromNativeSeparators( path ) << ")\n\n";
    }
    if ( m_screenshots.isEmpty() ) out << "スクリーンショットは保存されていません。\n";
}

void PlotOverTimeEditorTest::performs_plot_over_time_editor_cs_scenario()
{
    if ( !g_test_app ) g_test_app = pbvrTestApplication();
    QVERIFY2( g_test_app, "Test application is not initialized" );
    MainWindow window( *g_test_app );
    showTestWindowCentered( &window );
    QVERIFY( QTest::qWaitForWindowExposed( &window ) );

    auto* communication = window.findChild<Communication*>();
    auto* object = window.findChild<ObjectEditor*>();
    auto* glyph = window.findChild<GlyphEditor*>();
    auto* repetition = window.findChild<RepetitionLevelControl*>();
    auto* transform = window.findChild<VolumeTransform*>();
    auto* plot_editor = window.findChild<PlotOverTimeEditor*>();
    auto* playback = window.findChild<::PlayBackControlToolBar*>();
    auto* time_control = window.findChild<TimeStepControlToolBar*>();
    QVERIFY( communication && object && glyph && repetition && transform && plot_editor && playback && time_control );
    auto* plot = plot_editor->findChild<QCustomPlot*>();
    QVERIFY( plot );
    auto* coords_x = plot_editor->findChild<QDoubleSpinBox*>( "pointCoordsXDoubleSpinBox" );
    auto* coords_y = plot_editor->findChild<QDoubleSpinBox*>( "pointCoordsYDoubleSpinBox" );
    auto* coords_z = plot_editor->findChild<QDoubleSpinBox*>( "pointCoordsZDoubleSpinBox" );
    QVERIFY( coords_x && coords_y && coords_z );

    ObservationCapture capture;
    // Destroy this receiver before capture so the lambda cannot outlive its data.
    QObject receiver;
    connect( communication, &Communication::receivePlotOverTimeParameter, &receiver,
             [&capture]( const QJsonObject& payload ) { capture.receive( payload ); } );
    GraphData graph_z05;
    GraphData graph_z02_q1;
    bool aborted = false;
    const auto run_step = [this, &aborted]( const QString& description, const std::function<void()>& body )
    {
        m_steps.push_back( { description, "NOT RUN" } );
        if ( aborted ) return;
        qInfo().noquote() << description;
        body();
        aborted = QTest::currentTestFailed();
        m_steps.last().result = aborted ? "FAIL" : "PASS";
    };
    const auto jump_zero = [&]()
    {
        auto* next = time_control->findChild<QSpinBox*>( "m_next_time_step_spin_box" );
        QVERIFY( next );
        QVERIFY( waitForCondition( [playback]() { return playback->jumpButton()->isEnabled(); }, k_data_timeout_ms ) );
        QVERIFY( next->minimum() <= 0 && next->maximum() >= 0 );
        next->setValue( 0 );
        QCOMPARE( next->value(), 0 );
        QSignalSpy requested( time_control, &TimeStepControlToolBar::requestDataAt );
        QSignalSpy completed( object, &ObjectEditor::dataRequestCompleted );
        QVERIFY( requested.isValid() && completed.isValid() );
        QTest::mouseClick( playback->jumpButton(), Qt::LeftButton );
        QCOMPARE( requested.count(), 1 );
        QCOMPARE( requested.at( 0 ).at( 0 ).toInt(), 0 );
        QVERIFY2( waitForCondition( [&completed]() { return !completed.isEmpty(); }, k_calculation_timeout_ms ),
                  "Step 0 generation (including server-side POT calculation) did not complete" );
        QCOMPARE( completed.last().at( 0 ).toInt(), 0 );
        QVERIFY( waitForCondition( [playback]() { return playback->jumpButton()->isEnabled(); }, k_data_timeout_ms ) );
    };
    // POT is computed on the server before the requested particle generation finishes.
    const auto wait_for_graph = [&]( double z )
    {
        const bool finished = waitForCondition( [&capture, z]()
        {
            return capture.matches( z ) && ( capture.state == "complete" || capture.state == "failed" );
        }, k_calculation_timeout_ms );
        QVERIFY2( finished, qPrintable( "POT did not finish for the requested coordinates. Last response: " + capture.last_payload ) );
        QCOMPARE( capture.state, QStringLiteral( "complete" ) );
        auto* variable = plot_editor->findChild<QComboBox*>( "displayGraphComboBox" );
        QVERIFY( variable );
        QCOMPARE( variable->currentIndex(), 0 );
        QCOMPARE( variable->currentText(), QStringLiteral( "q1" ) );
        QVERIFY2( graphMatchesSamples( graphData( plot ), capture, 0 ),
                  "q1 graph must contain all observation steps, match received samples and have a finite value" );
    };

    run_step( QStringLiteral( "Communication: 接続し、CSモードとMEJデータ・伝達関数を設定してApplyする。" ), [&]()
    {
        communication->show();
        auto* tabs = communication->findChild<QTabWidget*>( "tabWidget" );
        auto* connect_tab = communication->findChild<QWidget*>( "connectTab" );
        QVERIFY( tabs && connect_tab );
        tabs->setCurrentWidget( connect_tab );
        QVERIFY( clickButton( communication, "connectPushButton" ) );
        auto* disconnect = communication->findChild<QPushButton*>( "disconnectPushButton" );
        auto* id = communication->findChild<QLineEdit*>( "IDLineEdit" );
        auto* apply = communication->findChild<QPushButton*>( "settingApplyPushButton" );
        QVERIFY( disconnect && id && apply );
        QVERIFY2( waitForCondition( [=]() { return disconnect->isEnabled() && !id->text().trimmed().isEmpty() && apply->isEnabled(); },
                                   k_connection_timeout_ms ), "Connection/operator state was not established; start the server before testing" );
        QVERIFY( selectRadio( communication, "remoteVizClientServerRadioButton" ) );
        auto* volume_path = communication->findChild<QLineEdit*>( "volumeDataFilePathLineEdit" );
        auto* tf_path = communication->findChild<QLineEdit*>( "transferFunctionFilePathLineEdit" );
        QVERIFY( volume_path && tf_path );
        volume_path->setText( QDir::toNativeSeparators( m_volume_data_path ) );
        tf_path->setText( QDir::toNativeSeparators( m_transfer_function_path ) );
        object->show();
        auto* name = object->findChild<QLineEdit*>( "nameLineEdit" );
        QVERIFY( name );
        name->clear();
        QVERIFY( clickButton( communication, "settingApplyPushButton" ) );
    } );

    run_step( QStringLiteral( "ObjectEditor: Nameへの反映を待ち、Applyする。" ), [&]()
    {
        auto* name = object->findChild<QLineEdit*>( "nameLineEdit" );
        auto* apply = object->findChild<QPushButton*>( "applyPushButton" );
        QVERIFY( name && apply );
        QVERIFY( waitForCondition( [=]() { return !name->text().trimmed().isEmpty() && apply->isEnabled(); }, k_data_timeout_ms ) );
        QVERIFY( clickButton( object, "applyPushButton" ) );
        object->close();
        communication->close();
    } );

    run_step( QStringLiteral( "GlyphEditor: Scale Factor=0.1、Size/Color DataのVariable Arrayを3成分にしてApplyする。" ), [&]()
    {
        QVERIFY( openTool( window, glyph, "Glyph Editor" ) );
        auto* count = glyph->findChild<QSpinBox*>( "sizeNumberOfVariablesSpinBox" );
        QVERIFY( count );
        QVERIFY( waitForCondition( [count]() { return count->maximum() >= 3; }, k_data_timeout_ms ) );
        QVERIFY( setDouble( glyph, "scaleFactorDoubleSpinBox", 0.1 ) );
        QVERIFY( configureVariables( glyph, "sizeVariableArrayRadioButton", "sizeNumberOfVariablesSpinBox", "sizeVariableGridLayout" ) );
        QVERIFY( configureVariables( glyph, "colorDataVariableArrayRadioButton", "colorDataNumberOfVariablesSpinBox", "colorDataVariableGridLayout" ) );
        QVERIFY( clickButton( glyph, "applyPushButton" ) );
        glyph->close();
    } );

    run_step( QStringLiteral( "Jump: 0ステップ目を生成し、完了通知を確認する。" ), jump_zero );

    run_step( QStringLiteral( "Repetition Level Control: New Repetition Level=16をApplyする。" ), [&]()
    {
        QVERIFY( openTool( window, repetition, "Repetition Level Control" ) );
        QVERIFY( setInteger( repetition, "spinBoxNewRepetitionLevel", 16 ) );
        QVERIFY( clickButton( repetition, "applyPushButton" ) );
        auto* current = repetition->findChild<QLabel*>( "dispCurrentRepetitionLevel" );
        QVERIFY( current );
        QCOMPARE( current->text().toInt(), 16 );
        repetition->close();
    } );

    run_step( QStringLiteral( "Plot Over Time: 有効化し、Coords=(0,0,0.5)の黄色い球を撮影する。" ), [&]()
    {
        QVERIFY( openTool( window, plot_editor, "Plot Over Time Editor" ) );
        QVERIFY( waitForCondition( [&capture]() { return !capture.applied.isEmpty(); }, k_data_timeout_ms ) );
        auto* enabled = plot_editor->findChild<QCheckBox*>( "plotOverTimeCheckBox" );
        QVERIFY( enabled && enabled->isEnabled() );
        if ( !enabled->isChecked() ) QTest::mouseClick( enabled, Qt::LeftButton );
        QVERIFY( enabled->isChecked() );
        QVERIFY( setDouble( plot_editor, "pointCoordsXDoubleSpinBox", 0.0 ) );
        QVERIFY( setDouble( plot_editor, "pointCoordsYDoubleSpinBox", 0.0 ) );
        QVERIFY( setDouble( plot_editor, "pointCoordsZDoubleSpinBox", 0.5 ) );
        saveScreenshot( "01_point_z05.png", QStringLiteral( "Coords=(0,0,0.5): 黄色の球が表示されること。" ) );
    } );

    run_step( QStringLiteral( "CoordsのXを0.2に変更して撮影する。" ), [&]()
    {
        QVERIFY( setDouble( plot_editor, "pointCoordsXDoubleSpinBox", 0.2 ) );
        saveScreenshot( "02_point_x02.png", QStringLiteral( "Coords=(0.2,0,0.5): 球がX軸方向へ移動すること。" ) );
    } );
    run_step( QStringLiteral( "CoordsのYを0.2に変更して撮影する。" ), [&]()
    {
        QVERIFY( setDouble( plot_editor, "pointCoordsYDoubleSpinBox", 0.2 ) );
        saveScreenshot( "03_point_y02.png", QStringLiteral( "Coords=(0.2,0.2,0.5): 球がY軸方向へ移動すること。" ) );
    } );
    run_step( QStringLiteral( "CoordsのX・Yを0へ戻してApplyし、観測座標の応答を確認する。" ), [&]()
    {
        QVERIFY( setDouble( plot_editor, "pointCoordsXDoubleSpinBox", 0.0 ) );
        QVERIFY( setDouble( plot_editor, "pointCoordsYDoubleSpinBox", 0.0 ) );
        QVERIFY( clickButton( plot_editor, "applyPushButton" ) );
        QVERIFY2( waitForCondition( [&capture]() { return capture.enabled && capture.matches( 0.5 ); }, k_connection_timeout_ms ),
                  qPrintable( capture.last_payload ) );
    } );

    run_step( QStringLiteral( "Volume Transform: Rotation XAxis=-90をApplyする。" ), [&]()
    {
        QVERIFY( openTool( window, transform, "Volume Transform" ) );
        QVERIFY( setDouble( transform, "rotationXAxisDoubleSpinBox", -90.0 ) );
        QVERIFY( clickButton( transform, "applyPushButton" ) );
        transform->close();
        QCOMPARE( coords_x->value(), 0.0 );
        QCOMPARE( coords_y->value(), 0.0 );
        QCOMPARE( coords_z->value(), 0.5 );
    } );
    run_step( QStringLiteral( "Jump: 0ステップ目を再生成する。" ), jump_zero );
    run_step( QStringLiteral( "Z=0.5: 計算完了と全時刻のq1グラフデータを検証して撮影する。" ), [&]()
    {
        wait_for_graph( 0.5 );
        if ( QTest::currentTestFailed() ) return;
        graph_z05 = graphData( plot );
        plot_editor->raise();
        saveScreenshot( "04_graph_z05.png", QStringLiteral( "Coords=(0,0,0.5): 回転後の球とPlot Over Timeグラフ。" ) );
    } );

    run_step( QStringLiteral( "Zを0.2に変更してApplyし、新しい観測座標の応答を確認する。" ), [&]()
    {
        QVERIFY( setDouble( plot_editor, "pointCoordsZDoubleSpinBox", 0.2 ) );
        QVERIFY( clickButton( plot_editor, "applyPushButton" ) );
        QVERIFY2( waitForCondition( [&capture]() { return capture.enabled && capture.matches( 0.2 ); }, k_connection_timeout_ms ),
                  qPrintable( capture.last_payload ) );
        QVERIFY2( graphData( plot ).isEmpty(), "Changing the applied CS observation must clear the old graph" );
    } );
    run_step( QStringLiteral( "Jump: 新しい観測点で0ステップ目を再生成する。" ), jump_zero );
    run_step( QStringLiteral( "Z=0.2: 計算完了とq1グラフの変化を検証し、球とグラフを撮影する。" ), [&]()
    {
        wait_for_graph( 0.2 );
        if ( QTest::currentTestFailed() ) return;
        QVERIFY2( graphChanged( graph_z05, graphData( plot ) ), "q1 graph did not change after Z moved from 0.5 to 0.2" );
        QCOMPARE( coords_x->value(), 0.0 );
        QCOMPARE( coords_y->value(), 0.0 );
        QCOMPARE( coords_z->value(), 0.2 );
        graph_z02_q1 = graphData( plot );
        saveScreenshot( "05_graph_z02.png", QStringLiteral( "Coords=(0,0,0.2): グラフが変わり、球がZ軸方向へ移動すること。" ) );
    } );
    run_step( QStringLiteral( "Z=0.2: 表示変数をq1からq2へ切り替え、全時刻のデータとグラフの変化を検証して撮影する。" ), [&]()
    {
        auto* variable = plot_editor->findChild<QComboBox*>( "displayGraphComboBox" );
        QVERIFY( variable && variable->isEnabled() && variable->isVisible() );
        QVERIFY2( variable->count() >= 2, "Display graph combo box must contain q2" );
        QCOMPARE( variable->currentIndex(), 0 );
        QCOMPARE( variable->currentText(), QStringLiteral( "q1" ) );
        QVERIFY( !graph_z02_q1.isEmpty() );

        // Switching variables redraws received samples without Apply or Jump.
        variable->setFocus();
        variable->setCurrentIndex( 1 );
        QCOMPARE( variable->currentIndex(), 1 );
        QCOMPARE( variable->currentText(), QStringLiteral( "q2" ) );
        QVERIFY2( waitForCondition( [&]() { return graphMatchesSamples( graphData( plot ), capture, 1 ); },
                                   k_connection_timeout_ms ),
                  "q2 graph must contain all observation steps, match received q2 samples and have a finite value" );
        QVERIFY2( graphChanged( graph_z02_q1, graphData( plot ) ),
                  "Graph values did not change after switching from q1 to q2 at Z=0.2" );
        QCOMPARE( coords_x->value(), 0.0 );
        QCOMPARE( coords_y->value(), 0.0 );
        QCOMPARE( coords_z->value(), 0.2 );
        saveScreenshot( "06_graph_z02_q2.png", QStringLiteral( "Coords=(0,0,0.2): q1からq2への切り替えでグラフが更新されること（05のq1画像と比較）。" ) );
    } );
    m_test_succeeded = !aborted && !QTest::currentTestFailed();
}
}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::PlotOverTimeEditorTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
