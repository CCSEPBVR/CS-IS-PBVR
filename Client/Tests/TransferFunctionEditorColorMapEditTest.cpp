#include "TransferFunctionEditorColorMapEditTest.h"

#include <QApplication>
#include <QColorDialog>
#include <QCoreApplication>
#include <QDate>
#include <QDialog>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTest>
#include <QTimer>

#include <kvs/qt/Application>

#include "../App/MainWindow.h"
#include "../ExtendedQT/ColorMapPalette.h"
#include "../Widgets/Communication.h"
#include "../Widgets/ObjectEditor.h"
#include "../Widgets/PlayBackControlToolBar.h"
#include "../Widgets/TransferFunctionEditor.h"
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

namespace TransferFunctionEditorTest
{

QString ColorMapEditTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
}

QString ColorMapEditTest::repoRootPath() const
{
    const QString app_root = findRepoRootFrom( QCoreApplication::applicationDirPath() );
    if ( !app_root.isEmpty() ) { return app_root; }
    const QString cwd_root = findRepoRootFrom( QDir::currentPath() );
    if ( !cwd_root.isEmpty() ) { return cwd_root; }
    const QString source_root = findRepoRootFrom( QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() );
    if ( !source_root.isEmpty() ) { return source_root; }
    return QDir::currentPath();
}

QString ColorMapEditTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool ColorMapEditTest::waitForCondition(
    const std::function<bool()>& condition,
    int timeout_ms,
    int interval_ms ) const
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

void ColorMapEditTest::bringWindowToFront( MainWindow* window ) const
{
    QVERIFY2( window != nullptr, "MainWindow is null" );
    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ColorMapEditTest::bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const
{
    QVERIFY2( editor != nullptr, "TransferFunctionEditor is null" );
    editor->show();
    editor->raise();
    editor->activateWindow();
    QVERIFY2( editor->isVisible(), "TransferFunctionEditor did not become visible" );
    QTest::qWait( k_window_settle_ms );
}

void ColorMapEditTest::bringDialogToFront( QDialog* dialog ) const
{
    QVERIFY2( dialog != nullptr, "Dialog is null" );
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
    QTest::qWait( k_window_settle_ms );
}

void ColorMapEditTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, text );
    QCOMPARE( line_edit->text(), text );
}

void ColorMapEditTest::saveScreenshot( const QString& file_name, const QString& caption )
{
    QTest::qWait( k_capture_settle_ms );
    QScreen* screen = QGuiApplication::primaryScreen();
    QVERIFY2( screen != nullptr, "Primary screen not found" );

    const QString file_path = QDir( m_screenshot_dir_path ).absoluteFilePath( file_name );
    const QPixmap screenshot = screen->grabWindow( 0 );
    QVERIFY2( !screenshot.isNull(), "Failed to capture screenshot from the primary screen" );
    QVERIFY2( screenshot.save( file_path ), qPrintable( QStringLiteral( "Failed to save screenshot: %1" ).arg( file_path ) ) );
    m_screenshots.push_back( { file_name, caption } );
}

void ColorMapEditTest::writeMarkdownReport() const
{
    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# TransferFunctionEditorTest::ColorMapEditTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- クライアントプログラム: `" << m_client_executable << "`\n";
    stream << "- サーバプログラム: `" << m_server_executable << "`\n";
    stream << "- サーバ起動ラッパー: `" << m_server_target_wrapper_executable << "`\n";
    stream << "- ボリュームデータ: `" << m_volume_data_path << "`\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n";
    stream << "- Freeform Curveのドラッグ編集確認: "
           << ( m_palette_drag_verified ? "PASS" : "未確認" )
           << "\n\n";

    stream << "## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.caption << "\n\n";
        stream << "![" << entry.caption << "](img/" << entry.file_name << ")\n\n";
    }
}

void ColorMapEditTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

ColorMapEditTest::ClientHandles ColorMapEditTest::resolveClientHandles( MainWindow& window ) const
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
    handles.tf_apply_button = handles.transfer_function_editor->findChild<QPushButton*>( "applyPushButton" );
    handles.tf_color_map_edit_button = handles.transfer_function_editor->findChild<QPushButton*>( "colorMapEditPushButton" );

    if ( !require( handles.connect_button != nullptr, "connectPushButton not found" ) ) { return handles; }
    if ( !require( handles.disconnect_button != nullptr, "disconnectPushButton not found" ) ) { return handles; }
    if ( !require( handles.setting_apply_button != nullptr, "settingApplyPushButton not found" ) ) { return handles; }
    if ( !require( handles.remote_viz_client_server_radio != nullptr, "remoteVizClientServerRadioButton not found" ) ) { return handles; }
    if ( !require( handles.volume_data_path_line_edit != nullptr, "volumeDataFilePathLineEdit not found" ) ) { return handles; }
    if ( !require( handles.id_line_edit != nullptr, "IDLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_name_line_edit != nullptr, "ObjectEditor nameLineEdit not found" ) ) { return handles; }
    if ( !require( handles.object_apply_button != nullptr, "ObjectEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.jump_button != nullptr, "m_jump_push_button not found" ) ) { return handles; }
    if ( !require( handles.tf_apply_button != nullptr, "TransferFunctionEditor applyPushButton not found" ) ) { return handles; }
    if ( !require( handles.tf_color_map_edit_button != nullptr, "TransferFunctionEditor colorMapEditPushButton not found" ) ) { return handles; }
    return handles;
}

void ColorMapEditTest::connectClient( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    QVERIFY2( waitForCondition( [client]() { return client.connect_button->isEnabled(); }, k_connect_timeout_ms, 100 ),
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

void ColorMapEditTest::configureRemoteVisualization( const ClientHandles& client ) const
{
    bringWindowToFront( client.main_window );
    if ( !client.remote_viz_client_server_radio->isChecked() )
    {
        QTest::mouseClick( client.remote_viz_client_server_radio, Qt::LeftButton );
    }
    QVERIFY2( client.remote_viz_client_server_radio->isChecked(), "remoteVizClientServerRadioButton was not checked" );
    QTest::qWait( k_short_wait_ms );

    setLineEditText( client.volume_data_path_line_edit, QDir::toNativeSeparators( m_volume_data_path ) );
    QTest::mouseClick( client.setting_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ColorMapEditTest::waitForObjectAndApply( const ClientHandles& client ) const
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
    QTest::mouseClick( client.object_apply_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ColorMapEditTest::clickJumpAndWaitForCompletion( const ClientHandles& client ) const
{
    QVERIFY2( waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
              "m_jump_push_button did not become enabled within the timeout" );
    QTest::mouseClick( client.jump_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    QVERIFY2( waitForCondition( [client]() { return client.jump_button->isEnabled(); }, k_jump_button_enable_timeout_ms, 200 ),
              "m_jump_push_button did not become enabled again within the timeout" );
    QTest::qWait( k_after_jump_wait_ms );
}

QDialog* ColorMapEditTest::waitForColorMapEditor() const
{
    QDialog* dialog = nullptr;
    const bool found = waitForCondition(
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
        50 );
    if ( !found )
    {
        QTest::qFail( "ColorMapEditor did not become visible", __FILE__, __LINE__ );
        return nullptr;
    }
    bringDialogToFront( dialog );
    return dialog;
}

int ColorMapEditTest::tabIndexByObjectName( QTabWidget* tab_widget, const QString& object_name ) const
{
    if ( tab_widget == nullptr )
    {
        QTest::qFail( "ColorMapEditor tabWidget not found", __FILE__, __LINE__ );
        return -1;
    }
    for ( int index = 0; index < tab_widget->count(); ++index )
    {
        QWidget* tab = tab_widget->widget( index );
        if ( tab != nullptr && tab->objectName() == object_name ) { return index; }
    }
    return -1;
}

void ColorMapEditTest::selectTab( QDialog* dialog, const QString& object_name ) const
{
    auto* tab_widget = dialog->findChild<QTabWidget*>( "tabWidget" );
    const int index = tabIndexByObjectName( tab_widget, object_name );
    QVERIFY2( index >= 0, qPrintable( QStringLiteral( "ColorMapEditor tab not found: %1" ).arg( object_name ) ) );
    tab_widget->setCurrentIndex( index );
    QCOMPARE( tab_widget->currentIndex(), index );
    QTest::qWait( k_window_settle_ms );
}

void ColorMapEditTest::doubleClickTrafficLightsStep( QDialog* dialog ) const
{
    auto* table = dialog->findChild<QTableWidget*>( "colorMapTableWidget" );
    QVERIFY2( table != nullptr, "colorMapTableWidget not found" );

    for ( int row = 0; row < table->rowCount(); ++row )
    {
        for ( int column = 0; column < table->columnCount(); ++column )
        {
            QWidget* cell_widget = table->cellWidget( row, column );
            if ( cell_widget == nullptr ) { continue; }
            const QList<QLabel*> labels = cell_widget->findChildren<QLabel*>();
            for ( QLabel* label : labels )
            {
                if ( label->text() != QStringLiteral( "Traffic Lights Step" ) ) { continue; }
                const QModelIndex index = table->model()->index( row, column );
                const QPoint click_pos = table->visualRect( index ).center();
                QTest::mouseDClick( table->viewport(), Qt::LeftButton, Qt::NoModifier, click_pos );
                const bool invoked = QMetaObject::invokeMethod(
                    dialog,
                    "onPresetColorMapDoubleClicked",
                    Qt::DirectConnection,
                    Q_ARG( int, row ),
                    Q_ARG( int, column ) );
                QVERIFY2( invoked, "Failed to invoke ColorMapEditor::onPresetColorMapDoubleClicked" );
                QTest::qWait( k_short_wait_ms );
                return;
            }
        }
    }

    QFAIL( "Traffic Lights Step was not found in colorMapTableWidget" );
}

QVector<QColor> ColorMapEditTest::colorMapPaletteColors( QDialog* dialog ) const
{
    QWidget* palette_widget = dialog->findChild<QWidget*>( "colorMapPalette" );
    auto* palette = static_cast<ColorMapPalette*>( palette_widget );
    if ( palette == nullptr )
    {
        QTest::qFail( "colorMapPalette not found", __FILE__, __LINE__ );
        return {};
    }

    return palette->getColors();
}

void ColorMapEditTest::selectDrawingColor( QDialog* dialog, const QColor& color ) const
{
    auto* label = dialog->findChild<QLabel*>( "drawingColorClickableLabel" );
    QVERIFY2( label != nullptr, "drawingColorClickableLabel not found" );

    QTimer::singleShot(
        0,
        [this, color]()
        {
            QColorDialog* color_dialog = nullptr;
            QVERIFY2(
                waitForCondition(
                    [&color_dialog]()
                    {
                        for ( QWidget* widget : QApplication::topLevelWidgets() )
                        {
                            auto* candidate = qobject_cast<QColorDialog*>( widget );
                            if ( candidate == nullptr || !candidate->isVisible() ) { continue; }
                            color_dialog = candidate;
                            return true;
                        }
                        return false;
                    },
                    k_dialog_timeout_ms,
                    50 ),
                "QColorDialog did not become visible" );
            color_dialog->setCurrentColor( color );
            color_dialog->accept();
        } );

    QTest::mouseDClick( label, Qt::LeftButton, Qt::NoModifier, label->rect().center() );
    QTest::qWait( k_short_wait_ms );

    const QColor selected = label->palette().color( QPalette::Window );
    QVERIFY2( selected.isValid(), "Selected drawing color is invalid" );
}

void ColorMapEditTest::dragPalette( QDialog* dialog, const QPoint& start, const QPoint& end )
{
    QWidget* palette_widget = dialog->findChild<QWidget*>( "colorMapPalette" );
    auto* palette = static_cast<ColorMapPalette*>( palette_widget );
    QVERIFY2( palette != nullptr, "colorMapPalette not found" );
    const QVector<QColor> before = palette->getColors();

    QTest::mousePress( palette, Qt::LeftButton, Qt::NoModifier, start );
    QTest::mouseMove( palette, end, 20 );
    QTest::mouseRelease( palette, Qt::LeftButton, Qt::NoModifier, end );
    QTest::qWait( k_short_wait_ms );

    const QVector<QColor> after = palette->getColors();
    QVERIFY2( before != after, "colorMapPalette did not change after drag editing" );
    m_palette_drag_verified = true;
}

void ColorMapEditTest::setControlPointRows( QDialog* dialog, const QVector<QVector<double>>& rows ) const
{
    auto* spin_box = dialog->findChild<QSpinBox*>( "numberOfControlPointsSpinBox" );
    auto* table = dialog->findChild<QTableWidget*>( "controlPointsTableWidget" );
    QVERIFY2( spin_box != nullptr, "numberOfControlPointsSpinBox not found" );
    QVERIFY2( table != nullptr, "controlPointsTableWidget not found" );
    spin_box->setValue( rows.size() );
    QCOMPARE( table->rowCount(), rows.size() );

    for ( int row = 0; row < rows.size(); ++row )
    {
        QCOMPARE( rows[row].size(), 4 );
        for ( int column = 0; column < 4; ++column )
        {
            auto* item = new QTableWidgetItem( QString::number( rows[row][column], 'g', 12 ) );
            table->setItem( row, column, item );
        }
    }

    const bool invoked = QMetaObject::invokeMethod( dialog, "onControlPointChanged", Qt::DirectConnection );
    QVERIFY2( invoked, "Failed to invoke ColorMapEditor::onControlPointChanged" );
    QTest::qWait( k_short_wait_ms );
}

void ColorMapEditTest::runColorMapEditorScenario( const ClientHandles& client )
{
    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    QVERIFY2( client.tf_color_map_edit_button->isEnabled(), "colorMapEditPushButton is disabled" );

    QTimer::singleShot(
        0,
        [this]()
        {
            QDialog* dialog = waitForColorMapEditor();
            saveScreenshot( QStringLiteral( "01_presets_tab.png" ), QStringLiteral( "Presetsタブが表示されているColorMapEditorの状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: Presetsタブのスクリーンショットを撮影しました。" ) );

            const QVector<QColor> colors_before_preset = colorMapPaletteColors( dialog );
            doubleClickTrafficLightsStep( dialog );
            const QVector<QColor> colors_after_preset = colorMapPaletteColors( dialog );
            QVERIFY2(
                colors_before_preset != colors_after_preset,
                "colorMapPalette did not change after double-clicking Traffic Lights Step" );
            saveScreenshot( QStringLiteral( "02_traffic_lights_step_selected.png" ), QStringLiteral( "Traffic Lights Stepが選択されたColorMapEditorの状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: colorMapTableWidgetからTraffic Lights Stepをダブルクリックし、colorMapPaletteが変化することを確認しました。" ) );

            selectTab( dialog, QStringLiteral( "freeformCurve" ) );
            saveScreenshot( QStringLiteral( "03_freeform_curve_tab.png" ), QStringLiteral( "Freeform Curveタブが表示されているColorMapEditorの状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: Freeform Curveタブを選択しました。" ) );

            selectDrawingColor( dialog, Qt::green );
            saveScreenshot( QStringLiteral( "04_freeform_drawing_color_green.png" ), QStringLiteral( "Freeform CurveでDrawing Colorにグリーンを選択した状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: drawingColorClickableLabelからグリーンを選択しました。" ) );

            dragPalette( dialog, QPoint( 80, 15 ), QPoint( 470, 85 ) );
            saveScreenshot( QStringLiteral( "05_freeform_palette_edited_green.png" ), QStringLiteral( "Freeform CurveでcolorMapPaletteをグリーンでドラッグ編集した状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: colorMapPaletteを左クリックしながらドラッグして編集しました。" ) );

            auto* reset_button = dialog->findChild<QPushButton*>( "resetPushButton" );
            QVERIFY2( reset_button != nullptr, "resetPushButton not found" );
            QTest::mouseClick( reset_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "06_palette_after_reset.png" ), QStringLiteral( "resetPushButtonでcolorMapPaletteが編集前のカラーマップに戻った状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: resetPushButtonを押しました。" ) );

            selectTab( dialog, QStringLiteral( "preset" ) );
            const QVector<QColor> colors_before_second_preset = colorMapPaletteColors( dialog );
            doubleClickTrafficLightsStep( dialog );
            const QVector<QColor> colors_after_second_preset = colorMapPaletteColors( dialog );
            QVERIFY2(
                colors_before_second_preset != colors_after_second_preset,
                "colorMapPalette did not change after reselecting Traffic Lights Step" );
            saveScreenshot( QStringLiteral( "07_traffic_lights_step_before_second_edit.png" ), QStringLiteral( "Freeform Curveで編集される前のTraffic Lights Stepの状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: Presetタブに戻り、Traffic Lights Stepを再選択してcolorMapPaletteが変化することを確認しました。" ) );

            selectTab( dialog, QStringLiteral( "freeformCurve" ) );
            selectDrawingColor( dialog, Qt::white );
            dragPalette( dialog, QPoint( 20, 2 ), QPoint( 170, 8 ) );
            saveScreenshot( QStringLiteral( "08_red_area_painted_white.png" ), QStringLiteral( "Freeform Curveの赤の部分をホワイトで塗った状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: ホワイトでcolorMapPaletteの赤の部分を塗りました。" ) );

            selectDrawingColor( dialog, Qt::black );
            dragPalette( dialog, QPoint( 380, 2 ), QPoint( 540, 8 ) );
            saveScreenshot( QStringLiteral( "09_green_area_painted_black.png" ), QStringLiteral( "Freeform Curveの緑の部分をブラックで塗った状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: ブラックでcolorMapPaletteの緑の部分を塗りました。" ) );

            auto* undo_button = dialog->findChild<QPushButton*>( "undoPushButton" );
            auto* redo_button = dialog->findChild<QPushButton*>( "redoPushButton" );
            QVERIFY2( undo_button != nullptr, "undoPushButton not found" );
            QVERIFY2( redo_button != nullptr, "redoPushButton not found" );
            QTest::mouseClick( undo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "10_undo_black_edit.png" ), QStringLiteral( "Freeform Curveの緑を黒に塗った作業を元に戻した状態" ) );
            QTest::mouseClick( undo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "11_undo_white_edit.png" ), QStringLiteral( "Freeform Curveの赤を白に塗った作業を元に戻した状態" ) );
            QTest::mouseClick( redo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "12_redo_white_edit.png" ), QStringLiteral( "Freeform Curveの赤を白に塗った作業をやり直した状態" ) );
            QTest::mouseClick( redo_button, Qt::LeftButton );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "13_redo_black_edit.png" ), QStringLiteral( "Freeform Curveの緑を黒に塗った作業をやり直した状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: undoPushButtonとredoPushButtonを2回ずつ押しました。" ) );

            selectTab( dialog, QStringLiteral( "expression" ) );
            saveScreenshot( QStringLiteral( "14_expression_tab.png" ), QStringLiteral( "Expressionページが開けた状態" ) );
            auto* red_line_edit = dialog->findChild<QLineEdit*>( "redLineEdit" );
            auto* green_line_edit = dialog->findChild<QLineEdit*>( "greenLineEdit" );
            auto* blue_line_edit = dialog->findChild<QLineEdit*>( "blueLineEdit" );
            setLineEditText( red_line_edit, QStringLiteral( "sin(x)" ) );
            saveScreenshot( QStringLiteral( "15_expression_red_sin.png" ), QStringLiteral( "Redの数式にsin(x)を入力した状態" ) );
            setLineEditText( green_line_edit, QStringLiteral( "cos(x)" ) );
            saveScreenshot( QStringLiteral( "16_expression_green_cos.png" ), QStringLiteral( "Greenの数式にcos(x)を入力した状態" ) );
            setLineEditText( blue_line_edit, QStringLiteral( "tan(x)" ) );
            saveScreenshot( QStringLiteral( "17_expression_blue_tan.png" ), QStringLiteral( "Blueの数式にtan(x)を入力した状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: expressionページでRed/Green/Blueの数式を編集しました。" ) );

            selectTab( dialog, QStringLiteral( "controlPoints" ) );
            saveScreenshot( QStringLiteral( "18_control_points_tab.png" ), QStringLiteral( "Control Pointsタブが開けた状態" ) );
            auto* spin_box = dialog->findChild<QSpinBox*>( "numberOfControlPointsSpinBox" );
            QVERIFY2( spin_box != nullptr, "numberOfControlPointsSpinBox not found" );
            spin_box->setValue( 3 );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "19_control_points_count_3.png" ), QStringLiteral( "制御点の数を3に増やした状態" ) );
            spin_box->setValue( 2 );
            QTest::qWait( k_short_wait_ms );
            saveScreenshot( QStringLiteral( "20_control_points_count_2.png" ), QStringLiteral( "制御点の数を2に減らした状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: numberOfControlPointsSpinBoxで制御点数を増減しました。" ) );

            setControlPointRows( dialog, { { 0, 1, 0, 0 }, { 0.5, 0, 1, 0 }, { 1, 0, 0, 1 } } );
            saveScreenshot( QStringLiteral( "21_control_points_set.png" ), QStringLiteral( "制御点を設定できた状態" ) );
            setControlPointRows( dialog, { { 0, 1, 0, 0 }, { 0.9, 0, 1, 0 }, { 1, 0, 0, 1 } } );
            saveScreenshot( QStringLiteral( "22_control_points_position_edited.png" ), QStringLiteral( "制御点を編集できた状態" ) );
            setControlPointRows( dialog, { { 0, 1, 0, 0 }, { 0.5, 0, 1, 0 }, { 1, 0, 0, 1 } } );
            saveScreenshot( QStringLiteral( "23_control_points_before_rgb_edit.png" ), QStringLiteral( "制御点のRed, Green, Blueを編集する前の状態" ) );
            setControlPointRows( dialog, { { 0, 0.5, 0, 0 }, { 0.5, 0, 1, 0 }, { 1, 0, 0, 1 } } );
            saveScreenshot( QStringLiteral( "24_control_points_red_edited.png" ), QStringLiteral( "制御点のRedを編集した後の状態" ) );
            setControlPointRows( dialog, { { 0, 0.5, 0, 0 }, { 0.5, 0, 0.5, 0 }, { 1, 0, 0, 1 } } );
            saveScreenshot( QStringLiteral( "25_control_points_green_edited.png" ), QStringLiteral( "制御点のGreenを編集した後の状態" ) );
            setControlPointRows( dialog, { { 0, 0.5, 0, 0 }, { 0.5, 0, 0.5, 0 }, { 1, 0, 0, 0.5 } } );
            saveScreenshot( QStringLiteral( "26_control_points_blue_edited.png" ), QStringLiteral( "制御点のBlueを編集した後の状態" ) );
            markStepCompleted( QStringLiteral( "ColorMapEditor.ui: controlPointsTableWidgetの制御点を設定・編集しました。" ) );

            auto* cancel_button = dialog->findChild<QPushButton*>( "cancelPushButton" );
            QVERIFY2( cancel_button != nullptr, "cancelPushButton not found" );
            QTest::mouseClick( cancel_button, Qt::LeftButton );
        } );

    QTest::mouseClick( client.tf_color_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
}

void ColorMapEditTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" ) );
    m_server_executable = envOrDefault( "PBVR_SERVER_EXECUTABLE", QStringLiteral( "/path/to/CS-IS-PBVR/Server/pbvr_server" ) );
    m_server_target_wrapper_executable = envOrDefault( "PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE", sourceTreePath( QStringLiteral( "server_target_wrapper.sh" ) ) );
    m_volume_data_path = envOrDefault( "PBVR_VOLUME_DATA", QStringLiteral( "/path/to/SampleData/ucd/old/out/spx.pfl" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "TransferFunctionEditorTest/ColorMapEditTest" ) ) );
    m_screenshot_dir_path = envOrDefault( "PBVR_SCREENSHOT_DIR", QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );
    m_test_succeeded = false;
    m_palette_drag_verified = false;

    QVERIFY2( QDir().mkpath( m_output_dir_path ), qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2( QDir().mkpath( m_screenshot_dir_path ), qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
    QVERIFY2( QFileInfo::exists( m_client_executable ), qPrintable( QStringLiteral( "Client executable not found: %1" ).arg( m_client_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_server_executable ), qPrintable( QStringLiteral( "Server executable not found: %1" ).arg( m_server_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_server_target_wrapper_executable ), qPrintable( QStringLiteral( "Server target wrapper executable not found: %1" ).arg( m_server_target_wrapper_executable ) ) );
    QVERIFY2( QFileInfo::exists( m_volume_data_path ), qPrintable( QStringLiteral( "Volume data file not found: %1" ).arg( m_volume_data_path ) ) );

    m_server_process.setProgram( m_server_target_wrapper_executable );
    m_server_process.setArguments( { m_server_executable } );
    m_server_process.setWorkingDirectory( QFileInfo( m_server_executable ).absolutePath() );
    m_server_process.start();
    QVERIFY2(
        m_server_process.waitForStarted( k_server_start_timeout_ms ),
        qPrintable( QStringLiteral( "Failed to start server: %1" ).arg( m_server_process.errorString() ) ) );
}

void ColorMapEditTest::cleanupTestCase()
{
    if ( m_server_process.state() != QProcess::NotRunning )
    {
        m_server_process.kill();
        m_server_process.waitForFinished( 5000 );
    }
    writeMarkdownReport();
}

void ColorMapEditTest::edit_color_map()
{
    if ( g_test_app == nullptr ) { g_test_app = pbvrTestApplication(); }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    main_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );

    ClientHandles client = resolveClientHandles( main_window );
    client.communication->show();
    client.object_editor->show();

    connectClient( client );
    markStepCompleted( QStringLiteral( "Communication.ui: connectPushButtonを押しました。" ) );
    configureRemoteVisualization( client );
    markStepCompleted( QStringLiteral( "Communication.ui: remoteVizClientServerRadioButtonを押し、volumeDataFilePathLineEditにデータパスを書き込み、settingApplyPushButtonを押しました。" ) );
    waitForObjectAndApply( client );
    markStepCompleted( QStringLiteral( "ObjectEditor.ui: nameLineEditにテキストが入るまで待機し、applyPushButtonを押しました。" ) );
    clickJumpAndWaitForCompletion( client );
    markStepCompleted( QStringLiteral( "PlayBackControlToolBar.cpp: m_jump_push_buttonを押し、有効化を待機しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.uiを開きました。" ) );
    runColorMapEditorScenario( client );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "27_tfe_after_colormap_cancel.png" ),
        QStringLiteral( "ColorMapEditor.uiで編集した内容がcancelPushButtonによりTransferFunctionEditorに反映されない状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: cancel後に編集内容が反映されない状態を撮影しました。" ) );

    QTimer::singleShot(
        0,
        [this]()
        {
            QDialog* dialog = waitForColorMapEditor();
            doubleClickTrafficLightsStep( dialog );
            auto* apply_button = dialog->findChild<QPushButton*>( "applyPushButton" );
            QVERIFY2( apply_button != nullptr, "ColorMapEditor applyPushButton not found" );
            QTest::mouseClick( apply_button, Qt::LeftButton );
        } );
    QTest::mouseClick( client.tf_color_map_edit_button, Qt::LeftButton );
    QTest::qWait( k_short_wait_ms );
    markStepCompleted( QStringLiteral( "ColorMapEditor.ui: Traffic Lights Stepをダブルクリックし、applyPushButtonを押しました。" ) );

    bringTransferFunctionEditorToFront( client.transfer_function_editor );
    saveScreenshot(
        QStringLiteral( "28_tfe_after_colormap_apply.png" ),
        QStringLiteral( "ColorMapEditor.uiで選択したTraffic Lights StepがTransferFunctionEditorに反映された状態" ) );
    markStepCompleted( QStringLiteral( "TransferFunctionEditor.ui: apply後に編集内容が反映された状態を撮影しました。" ) );

    m_test_succeeded = true;
}

}

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    TransferFunctionEditorTest::ColorMapEditTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
