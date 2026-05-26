#include "PreferenceTest.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QMetaObject>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QTextStream>
#include <QTest>
#include <QTimer>
#include <QWidget>

#include "../App/MainWindow.h"
#include "../ExtendedQT/ClickableLabel.h"
#include "../Widgets/Preference.h"
#include "TestAppContext.h"
#include "TestOutputPaths.h"

namespace
{
constexpr int k_window_settle_ms = 500;
constexpr int k_short_wait_ms = 1000;
constexpr int k_long_wait_ms = 3000;
constexpr int k_combo_popup_timeout_ms = 5000;
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

namespace ClientTests
{

QString PreferenceTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? ClientTests::configuredPath( name, repoRootPath(), fallback ) : value;
}

QString PreferenceTest::repoRootPath() const
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

QString PreferenceTest::sourceTreePath( const QString& relative_path_from_repo_root ) const
{
    return QDir::cleanPath( QDir( repoRootPath() ).absoluteFilePath( relative_path_from_repo_root ) );
}

bool PreferenceTest::waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms ) const
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

void PreferenceTest::clickButtonAndWait( QPushButton* button, int wait_ms ) const
{
    QVERIFY2( button != nullptr, "Target button was not found" );
    QVERIFY2( button->isEnabled(), qPrintable( QStringLiteral( "Button is disabled: %1" ).arg( button->objectName() ) ) );
    QTest::mouseClick( button, Qt::LeftButton );
    QTest::qWait( wait_ms );
}

void PreferenceTest::setLineEditText( QLineEdit* line_edit, const QString& text ) const
{
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );
    line_edit->setFocus();
    line_edit->clear();
    line_edit->setText( text );
    QCOMPARE( line_edit->text(), text );
}

void PreferenceTest::setSpinBoxValue( QSpinBox* spin_box, int value ) const
{
    QVERIFY2( spin_box != nullptr, "Target spin box was not found" );
    spin_box->setFocus();
    spin_box->setValue( value );
    QTRY_COMPARE( spin_box->value(), value );
}

void PreferenceTest::setGroupBoxChecked( QGroupBox* group_box, bool checked ) const
{
    QVERIFY2( group_box != nullptr, "Target group box was not found" );
    group_box->setChecked( checked );
    QTRY_COMPARE( group_box->isChecked(), checked );
}

void PreferenceTest::selectComboBoxItem( QComboBox* combo_box, int index ) const
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

QColorDialog* PreferenceTest::waitForColorDialog( int timeout_ms ) const
{
    QColorDialog* dialog = nullptr;
    const bool dialog_found = waitForCondition(
        [&dialog]()
        {
            const auto widgets = QApplication::topLevelWidgets();
            for ( QWidget* widget : widgets )
            {
                auto* color_dialog = qobject_cast<QColorDialog*>( widget );
                if ( color_dialog && color_dialog->isVisible() )
                {
                    dialog = color_dialog;
                    return true;
                }
            }
            return false;
        },
        timeout_ms,
        50 );

    return dialog_found ? dialog : nullptr;
}

void PreferenceTest::selectColor( QWidget* target, const QColor& color ) const
{
    QVERIFY2( target != nullptr, "Color target widget was not found" );

    auto* clickable_label = qobject_cast<ClickableLabel*>( target );
    QVERIFY2( clickable_label != nullptr, "Color target is not ClickableLabel" );

    QTimer closer_timer;
    closer_timer.setInterval( 50 );
    closer_timer.setSingleShot( false );
    QObject::connect(
        &closer_timer,
        &QTimer::timeout,
        qApp,
        [&closer_timer, color]()
        {
            for ( QWidget* widget : QApplication::topLevelWidgets() )
            {
                auto* dialog = qobject_cast<QColorDialog*>( widget );
                if ( dialog == nullptr || !dialog->isVisible() ) { continue; }

                dialog->setCurrentColor( color );
                auto* button_box = dialog->findChild<QDialogButtonBox*>();
                if ( button_box == nullptr ) { continue; }

                auto* ok_button = button_box->button( QDialogButtonBox::Ok );
                if ( ok_button == nullptr || !ok_button->isEnabled() ) { continue; }

                QTest::mouseClick( ok_button, Qt::LeftButton );
                closer_timer.stop();
                return;
            }
        } );

    closer_timer.start();

    QMetaObject::invokeMethod( clickable_label, "doubleClicked", Qt::DirectConnection );
    closer_timer.stop();

    const QColor selected_color = clickable_label->palette().color( QPalette::Window );
    QCOMPARE( selected_color, color );
}

void PreferenceTest::saveScreenshot( const QString& file_name, const QString& caption )
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
    m_visual_checks.push_back( caption );
}

void PreferenceTest::writeMarkdownReport() const
{
    if ( m_report_path.isEmpty() ) { return; }

    QFile report_file( m_report_path );
    QVERIFY2(
        report_file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ),
        qPrintable( QStringLiteral( "Failed to open markdown report: %1" ).arg( m_report_path ) ) );

    QTextStream stream( &report_file );
    stream << "# PreferenceTest\n\n";
    stream << "- 結果: " << ( m_test_succeeded ? "PASS" : "FAIL" ) << "\n";
    stream << "- 出力先: `" << m_output_dir_path << "`\n";
    stream << "- スクリーンショット出力先: `" << m_screenshot_dir_path << "`\n\n";

    stream << "## 実施手順\n\n";
    for ( const StepEntry& step : m_steps )
    {
        stream << "- " << ( step.completed ? "PASS" : "NOT RUN" ) << ": " << step.description << "\n";
    }

    stream << "\n## 自動判定項目\n\n";
    for ( const CheckEntry& check : m_checks )
    {
        stream << "- " << ( check.passed ? "PASS" : "FAIL" ) << ": " << check.description << "\n";
    }

    stream << "\n## 目視確認対象\n\n";
    for ( const QString& caption : m_visual_checks )
    {
        stream << "- 要確認: " << caption << "\n";
    }

    stream << "\n## スクリーンショット\n\n";
    for ( const ScreenshotEntry& entry : m_screenshots )
    {
        stream << "### " << entry.caption << "\n\n";
        stream << "![" << entry.caption << "](./img/" << entry.file_name << ")\n\n";
    }

    stream << "## 未自動化・保留事項\n\n";
    stream << "- スクリーンショットに写るカラーマップバー、OrientationAxis、背景色、文字色、ラベル表示の視覚的な正しさは目視確認対象です。\n";
}

void PreferenceTest::markStepCompleted( const QString& description )
{
    m_steps.push_back( { description, true } );
    logStep( description );
}

void PreferenceTest::recordCheck( const QString& description, bool passed )
{
    m_checks.push_back( { description, passed } );
    QVERIFY2( passed, qPrintable( description ) );
}

void PreferenceTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        ClientTests::configuredPath( "PBVR_CLIENT_EXECUTABLE", repoRootPath() ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp, QStringLiteral( "PreferenceTest" ) ) );
    m_screenshot_dir_path = envOrDefault(
        "PBVR_SCREENSHOT_DIR",
        QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "img" ) ) );
    m_report_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "TestResult.md" ) );

    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
    QVERIFY2(
        QDir().mkpath( m_screenshot_dir_path ),
        qPrintable( QStringLiteral( "Failed to create screenshot directory: %1" ).arg( m_screenshot_dir_path ) ) );
}

void PreferenceTest::cleanupTestCase()
{
    writeMarkdownReport();
}

void PreferenceTest::performs_preference_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_window_settle_ms );
    markStepCompleted( QStringLiteral( "クライアントプログラムを起動した。" ) );

    auto* preference = main_window.findChild<Preference*>();
    QVERIFY2( preference != nullptr, "Preference dialog was not found" );
    QVERIFY2( preference->screen() != nullptr, "Preference screen is not initialized" );
    QVERIFY2( preference->colorMapBar() != nullptr, "Preference color map bar is not initialized" );
    QVERIFY2( preference->orientationAxis() != nullptr, "Preference orientation axis is not initialized" );
    QVERIFY2( preference->fpsLabel() != nullptr, "Preference fps label is not initialized" );
    QVERIFY2( preference->timeStepLabel() != nullptr, "Preference time step label is not initialized" );

    preference->show();
    preference->raise();
    preference->activateWindow();
    QVERIFY( QTest::qWaitForWindowExposed( preference ) );
    QTest::qWait( k_window_settle_ms );
    markStepCompleted( QStringLiteral( "Preference.uiを開いた。" ) );

    auto* default_button = preference->findChild<QPushButton*>( "defaultPushButton" );
    auto* apply_button = preference->findChild<QPushButton*>( "applyPushButton" );
    auto* cancel_button = preference->findChild<QPushButton*>( "cancelPushButton" );
    auto* ok_button = preference->findChild<QPushButton*>( "okPushButton" );
    auto* color_map_bar_group_box = preference->findChild<QGroupBox*>( "colorMapBarGroupBox" );
    auto* caption_line_edit = preference->findChild<QLineEdit*>( "captionLineEdit" );
    auto* orientation_type_combo_box = preference->findChild<QComboBox*>( "orientationTypeComboBox" );
    auto* orientation_axis_group_box = preference->findChild<QGroupBox*>( "orientationAxisGroupBox" );
    auto* axis_type_combo_box = preference->findChild<QComboBox*>( "axisTypeComboBox" );
    auto* box_type_combo_box = preference->findChild<QComboBox*>( "boxTypeComboBox" );
    auto* background_color_label = preference->findChild<QWidget*>( "backGroundColorClickableLabel" );
    auto* width_spin_box = preference->findChild<QSpinBox*>( "widthSpinBox" );
    auto* height_spin_box = preference->findChild<QSpinBox*>( "heightSpinBox" );
    auto* show_fps_combo_box = preference->findChild<QComboBox*>( "showFPSComboBox" );
    auto* show_time_step_combo_box = preference->findChild<QComboBox*>( "showTimeStepComboBox" );
    auto* font_color_label = preference->findChild<QWidget*>( "fontColorClickableLabel" );

    QVERIFY2( default_button != nullptr, "defaultPushButton not found" );
    QVERIFY2( apply_button != nullptr, "applyPushButton not found" );
    QVERIFY2( cancel_button != nullptr, "cancelPushButton not found" );
    QVERIFY2( ok_button != nullptr, "okPushButton not found" );
    QVERIFY2( color_map_bar_group_box != nullptr, "colorMapBarGroupBox not found" );
    QVERIFY2( caption_line_edit != nullptr, "captionLineEdit not found" );
    QVERIFY2( orientation_type_combo_box != nullptr, "orientationTypeComboBox not found" );
    QVERIFY2( orientation_axis_group_box != nullptr, "orientationAxisGroupBox not found" );
    QVERIFY2( axis_type_combo_box != nullptr, "axisTypeComboBox not found" );
    QVERIFY2( box_type_combo_box != nullptr, "boxTypeComboBox not found" );
    QVERIFY2( background_color_label != nullptr, "backGroundColorClickableLabel not found" );
    QVERIFY2( width_spin_box != nullptr, "widthSpinBox not found" );
    QVERIFY2( height_spin_box != nullptr, "heightSpinBox not found" );
    QVERIFY2( show_fps_combo_box != nullptr, "showFPSComboBox not found" );
    QVERIFY2( show_time_step_combo_box != nullptr, "showTimeStepComboBox not found" );
    QVERIFY2( font_color_label != nullptr, "fontColorClickableLabel not found" );

    clickButtonAndWait( default_button, k_short_wait_ms );
    recordCheck( QStringLiteral( "defaultPushButtonでcaptionLineEditが空になる。" ), caption_line_edit->text().isEmpty() );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    markStepCompleted( QStringLiteral( "defaultPushButtonとapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "01_default_state.png" ), QStringLiteral( "デフォルトの状態を表す。" ) );

    setGroupBoxChecked( color_map_bar_group_box, false );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "colorMapBarGroupBoxのチェックを外せる。" ), !color_map_bar_group_box->isChecked() );
    markStepCompleted( QStringLiteral( "colorMapBarGroupBoxのチェックを外してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "02_color_map_bar_hidden.png" ), QStringLiteral( "colorMapBarが非表示になることを表す。" ) );

    setGroupBoxChecked( color_map_bar_group_box, true );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "colorMapBarGroupBoxをチェックできる。" ), color_map_bar_group_box->isChecked() );
    markStepCompleted( QStringLiteral( "colorMapBarGroupBoxをチェックしてapplyPushButtonを押した。" ) );

    setLineEditText( caption_line_edit, QStringLiteral( "test" ) );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "captionLineEditにtestを入力できる。" ), caption_line_edit->text() == QStringLiteral( "test" ) );
    markStepCompleted( QStringLiteral( "captionLineEditにtestを書き込み、applyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "03_color_map_bar_caption.png" ), QStringLiteral( "カラーマップバーのcaptionが表示されることを表す。" ) );

    selectComboBoxItem( orientation_type_combo_box, 1 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "orientationTypeComboBoxの2番目を選択できる。" ), orientation_type_combo_box->currentIndex() == 1 );
    markStepCompleted( QStringLiteral( "orientationTypeComboBoxの2番目を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "04_color_map_bar_vertical.png" ), QStringLiteral( "カラーマップバーが垂直に表示されることを表す。" ) );

    setGroupBoxChecked( orientation_axis_group_box, false );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "orientationAxisGroupBoxのチェックを外せる。" ), !orientation_axis_group_box->isChecked() );
    markStepCompleted( QStringLiteral( "orientationAxisGroupBoxのチェックを外してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "05_orientation_axis_hidden.png" ), QStringLiteral( "orientationAxisが非表示になることを表す。" ) );

    setGroupBoxChecked( orientation_axis_group_box, true );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "orientationAxisGroupBoxをチェックできる。" ), orientation_axis_group_box->isChecked() );
    markStepCompleted( QStringLiteral( "orientationAxisGroupBoxをチェックしてapplyPushButtonを押した。" ) );

    selectComboBoxItem( axis_type_combo_box, 1 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "axisTypeComboBoxの2番目を選択できる。" ), axis_type_combo_box->currentIndex() == 1 );
    markStepCompleted( QStringLiteral( "axisTypeComboBoxの2番目を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "06_orientation_axis_centered_axis.png" ), QStringLiteral( "orientationAxisのAxisTypeがCenteredAxisになることを表す。" ) );

    selectComboBoxItem( axis_type_combo_box, 2 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "axisTypeComboBoxの3番目を選択できる。" ), axis_type_combo_box->currentIndex() == 2 );
    markStepCompleted( QStringLiteral( "axisTypeComboBoxの3番目を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "07_orientation_axis_none_axis.png" ), QStringLiteral( "orientationAxisのAxisTypeがNoneAxisになることを表す。" ) );

    selectComboBoxItem( axis_type_combo_box, 0 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "axisTypeComboBoxを1番目に戻せる。" ), axis_type_combo_box->currentIndex() == 0 );
    markStepCompleted( QStringLiteral( "axisTypeComboBoxの1番目を選択してapplyPushButtonを押した。" ) );

    selectComboBoxItem( box_type_combo_box, 0 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "boxTypeComboBoxの1番目を選択できる。" ), box_type_combo_box->currentIndex() == 0 );
    markStepCompleted( QStringLiteral( "boxTypeComboBoxの1番目を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "08_orientation_axis_wired_box.png" ), QStringLiteral( "orientationAxisのBoxTypeがWiredBoxになることを表す。" ) );

    selectComboBoxItem( box_type_combo_box, 1 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "boxTypeComboBoxの2番目を選択できる。" ), box_type_combo_box->currentIndex() == 1 );
    markStepCompleted( QStringLiteral( "boxTypeComboBoxの2番目を選択してapplyPushButtonを押した。" ) );

    selectColor( background_color_label, Qt::white );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck(
            QStringLiteral( "backGroundColorClickableLabelで白を選択できる。" ),
            background_color_label->palette().color( QPalette::Window ) == QColor( Qt::white ) );
    markStepCompleted( QStringLiteral( "backGroundColorClickableLabelで白を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "09_background_white.png" ), QStringLiteral( "スクリーンの背景色が白になることを表す。" ) );

    setSpinBoxValue( width_spin_box, 500 );
    setSpinBoxValue( height_spin_box, 500 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "widthSpinBoxとheightSpinBoxを500に設定できる。" ), width_spin_box->value() == 500 && height_spin_box->value() == 500 );
    markStepCompleted( QStringLiteral( "widthSpinBoxを500、heightSpinBoxを500に設定してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "10_resolution_500x500.png" ), QStringLiteral( "スクリーンの解像度が500x500になることを表す。" ) );

    selectComboBoxItem( show_fps_combo_box, 0 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "showFPSComboBoxの1番目を選択できる。" ), show_fps_combo_box->currentIndex() == 0 );
    markStepCompleted( QStringLiteral( "showFPSComboBoxの1番目を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "11_fps_label_shown.png" ), QStringLiteral( "スクリーンのFPSラベルが表示されることを表す。" ) );

    selectComboBoxItem( show_time_step_combo_box, 0 );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "showTimeStepComboBoxの1番目を選択できる。" ), show_time_step_combo_box->currentIndex() == 0 );
    markStepCompleted( QStringLiteral( "showTimeStepComboBoxの1番目を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "12_time_step_label_shown.png" ), QStringLiteral( "スクリーンのTimeStepラベルが表示されることを表す。" ) );

    selectColor( font_color_label, Qt::blue );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    recordCheck(
            QStringLiteral( "fontColorClickableLabelで青を選択できる。" ),
            font_color_label->palette().color( QPalette::Window ) == QColor( Qt::blue ) );
    markStepCompleted( QStringLiteral( "fontColorClickableLabelで青を選択してapplyPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "13_font_blue.png" ), QStringLiteral( "スクリーンの文字の色が青になることを表す。" ) );

    clickButtonAndWait( default_button, k_short_wait_ms );
    recordCheck( QStringLiteral( "defaultPushButtonでcaptionLineEditが再び空になる。" ), caption_line_edit->text().isEmpty() );
    clickButtonAndWait( apply_button, k_long_wait_ms );
    markStepCompleted( QStringLiteral( "defaultPushButtonとapplyPushButtonを押してデフォルト設定に戻した。" ) );
    saveScreenshot( QStringLiteral( "14_default_restored.png" ), QStringLiteral( "デフォルトの設定に戻ることを表す。" ) );

    setLineEditText( caption_line_edit, QStringLiteral( "test" ) );
    recordCheck( QStringLiteral( "cancel前にcaptionLineEditへtestを入力できる。" ), caption_line_edit->text() == QStringLiteral( "test" ) );
    markStepCompleted( QStringLiteral( "captionLineEditにtestを書き込んだ。" ) );
    saveScreenshot( QStringLiteral( "15_caption_entered_before_cancel.png" ), QStringLiteral( "ColorMapBarのCaptionにtestと書き込んだことを表す。" ) );

    clickButtonAndWait( cancel_button, k_short_wait_ms );
    recordCheck( QStringLiteral( "cancelPushButtonでPreference.uiが閉じる。" ), !preference->isVisible() );
    markStepCompleted( QStringLiteral( "cancelPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "16_cancel_closed.png" ), QStringLiteral( "修正した変更が反映されずPreference.uiが閉じたことを表す。" ) );

    preference->show();
    preference->raise();
    preference->activateWindow();
    QVERIFY( QTest::qWaitForWindowExposed( preference ) );
    QTest::qWait( k_window_settle_ms );
    markStepCompleted( QStringLiteral( "Preference.uiを再度開いた。" ) );

    setLineEditText( caption_line_edit, QStringLiteral( "test" ) );
    recordCheck( QStringLiteral( "ok前にcaptionLineEditへtestを入力できる。" ), caption_line_edit->text() == QStringLiteral( "test" ) );
    markStepCompleted( QStringLiteral( "captionLineEditにtestを書き込んだ。" ) );
    saveScreenshot( QStringLiteral( "17_caption_entered_before_ok.png" ), QStringLiteral( "ColorMapBarのCaptionにtestと書き込んだことを表す。" ) );

    clickButtonAndWait( ok_button, k_long_wait_ms );
    recordCheck( QStringLiteral( "okPushButtonでPreference.uiが閉じる。" ), !preference->isVisible() );
    markStepCompleted( QStringLiteral( "okPushButtonを押した。" ) );
    saveScreenshot( QStringLiteral( "18_ok_closed_applied.png" ), QStringLiteral( "修正した変更を反映してPreference.uiが閉じたことを表す。" ) );

    main_window.hide();
    QTest::qWait( k_window_settle_ms );
    showTestWindowCentered( &main_window );
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_long_wait_ms );
    markStepCompleted( QStringLiteral( "クライアントプログラムを再起動した。" ) );
    saveScreenshot( QStringLiteral( "19_after_restart.png" ), QStringLiteral( "クライアントプログラムを再起動しても修正した内容が保存されていることを確認する。" ) );

    m_test_succeeded = true;
}

} // namespace ClientTests

#ifndef PBVR_TEST_NO_MAIN
int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_test_app = &app;
    ClientTests::PreferenceTest test;
    return QTest::qExec( &test, argc, argv );
}
#endif
