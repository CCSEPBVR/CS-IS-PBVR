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
#include <QGroupBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QTest>
#include <QTimer>
#include <QWidget>

#include <csignal>

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
constexpr int k_recording_finish_timeout_ms = 15000;
constexpr int k_dialog_timeout_ms = 5000;
constexpr int k_combo_popup_timeout_ms = 5000;
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
}

namespace ClientTests
{

QString PreferenceTest::envOrDefault( const char* name, const QString& fallback ) const
{
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? fallback : value;
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

void PreferenceTest::startVideoRecording()
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

void PreferenceTest::stopVideoRecording()
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
    Q_UNUSED( color );

    auto* clickable_label = qobject_cast<ClickableLabel*>( target );
    QVERIFY2( clickable_label != nullptr, "Color target is not ClickableLabel" );

    QTimer closer_timer;
    closer_timer.setInterval( 50 );
    closer_timer.setSingleShot( false );
    QObject::connect(
        &closer_timer,
        &QTimer::timeout,
        qApp,
        [&closer_timer]()
        {
            for ( QWidget* widget : QApplication::topLevelWidgets() )
            {
                auto* dialog = qobject_cast<QColorDialog*>( widget );
                if ( dialog == nullptr || !dialog->isVisible() ) { continue; }

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
}

void PreferenceTest::initTestCase()
{
    const QString date_stamp = QDate::currentDate().toString( QStringLiteral( "yyyyMMdd" ) );
    m_client_executable = envOrDefault(
        "PBVR_CLIENT_EXECUTABLE",
        QStringLiteral( "/path/to/CS-IS-PBVR/Client/build/Qt_6_11_0_for_macOS-Release/App/pbvr_client.app/Contents/MacOS/pbvr_client" ) );
    m_output_dir_path = envOrDefault(
        "PBVR_TEST_OUTPUT_DIR",
        ClientTests::datedTestOutputDir( repoRootPath(), date_stamp ) );
    m_video_file_path = QDir( m_output_dir_path ).absoluteFilePath( QStringLiteral( "PreferenceTest.mov" ) );

    QVERIFY2(
        QFileInfo::exists( m_client_executable ),
        qPrintable( QStringLiteral( "Client executable not found: %1" ).arg( m_client_executable ) ) );
    QVERIFY2(
        QDir().mkpath( m_output_dir_path ),
        qPrintable( QStringLiteral( "Failed to create output directory: %1" ).arg( m_output_dir_path ) ) );
}

void PreferenceTest::cleanupTestCase()
{
    if ( m_recording_process.state() != QProcess::NotRunning )
    {
        stopVideoRecording();
    }
}

void PreferenceTest::performs_preference_scenario()
{
    if ( g_test_app == nullptr )
    {
        g_test_app = pbvrTestApplication();
    }
    QVERIFY2( g_test_app != nullptr, "Test application is not initialized" );

    MainWindow main_window( *g_test_app );
    main_window.show();
    QVERIFY( QTest::qWaitForWindowExposed( &main_window ) );
    main_window.raise();
    main_window.activateWindow();
    QTest::qWait( k_window_settle_ms );

    auto* preference = main_window.findChild<Preference*>();
    QVERIFY2( preference != nullptr, "Preference dialog was not found" );
    QVERIFY2( preference->screen() != nullptr, "Preference screen is not initialized" );
    QVERIFY2( preference->colorMapBar() != nullptr, "Preference color map bar is not initialized" );
    QVERIFY2( preference->orientationAxis() != nullptr, "Preference orientation axis is not initialized" );
    QVERIFY2( preference->fpsLabel() != nullptr, "Preference fps label is not initialized" );
    QVERIFY2( preference->timeStepLabel() != nullptr, "Preference time step label is not initialized" );

    startVideoRecording();
    QTest::qWait( k_short_wait_ms );

    preference->show();
    preference->raise();
    preference->activateWindow();
    QVERIFY( QTest::qWaitForWindowExposed( preference ) );
    QTest::qWait( k_window_settle_ms );

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
    QCOMPARE( caption_line_edit->text(), QString() );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setGroupBoxChecked( color_map_bar_group_box, false );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setGroupBoxChecked( color_map_bar_group_box, true );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setLineEditText( caption_line_edit, QStringLiteral( "test" ) );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectComboBoxItem( orientation_type_combo_box, 1 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setGroupBoxChecked( orientation_axis_group_box, false );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setGroupBoxChecked( orientation_axis_group_box, true );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectComboBoxItem( axis_type_combo_box, 1 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectComboBoxItem( axis_type_combo_box, 2 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectComboBoxItem( box_type_combo_box, 0 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectColor( background_color_label, Qt::white );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setSpinBoxValue( width_spin_box, 500 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setSpinBoxValue( height_spin_box, 500 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectComboBoxItem( show_fps_combo_box, 0 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectComboBoxItem( show_time_step_combo_box, 0 );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    selectColor( font_color_label, Qt::white );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    clickButtonAndWait( default_button, k_short_wait_ms );
    QCOMPARE( caption_line_edit->text(), QString() );
    clickButtonAndWait( apply_button, k_long_wait_ms );

    setLineEditText( caption_line_edit, QStringLiteral( "test" ) );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( cancel_button, k_short_wait_ms );

    preference->show();
    preference->raise();
    preference->activateWindow();
    QVERIFY( QTest::qWaitForWindowExposed( preference ) );
    QTest::qWait( k_window_settle_ms );

    setLineEditText( caption_line_edit, QStringLiteral( "test" ) );
    QTest::qWait( k_short_wait_ms );
    clickButtonAndWait( ok_button, k_long_wait_ms );

    stopVideoRecording();
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
