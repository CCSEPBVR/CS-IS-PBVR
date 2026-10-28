#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QGuiApplication>
#include <QWindow>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QTest>
#include <QtGlobal>
#include <QWidget>

#include <kvs/qt/Application>

#include "TestAppContext.h"

#ifdef PBVR_ENABLE_TEST_MENUBAR
#include "MenuBarTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_SCREEN
#include "ScreenTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_PLAYBACKCONTROLTOOLBAR
#include "PlayBackControlToolBarTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TIMESTEPCONTROLTOOLBAR
#include "TimeStepControlToolBarTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_COLORMAPSELECTORTOOLBAR
#include "ColorMapSelectorToolBarTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TOTALPARTICLESTOOLBAR
#include "TotalParticlesToolBarTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_PREFERENCE
#include "PreferenceTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION
#include "CommunicationTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_USER_INFO
#include "CommunicationUserInfoTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SETTING
#include "CommunicationSettingTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SHARE_VIEW
#include "CommunicationShareViewTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_ANIMATIONCONTROL
#include "AnimationControlTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_GLYPHEDITOR
#include "GlyphEditorTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_OBJECTEDITOR
#include "ObjectEditorTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_PLOTOVERLINEEDITOR
#include "PlotOverLineEditorTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_PLOTOVERTIMEEDITOR
#include "PlotOverTimeEditorTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_POINTSIZECONTROL
#include "PointSizeControlTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL
#include "RepetitionLevelControlTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_SHADINGCONTROL
#include "ShadingControlTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_SERVER
#include "ServerTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_INSITU_MODE
#include "InSituModeTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_ENSEMBLE_TRANSFER_FUNCTION
#include "EnsembleTransferFunctionTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL_INSITU
#include "RepetitionLevelControlInSituTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_ENSEMBLE_INSITU_MODE
#include "EnsembleInSituModeTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_VOLUMETRANSFORM
#include "VolumeTransformTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT
#include "TransferFunctionEditorImportExportTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORT_INVALID_FILE
#include "TransferFunctionEditorImportInvalidFileTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER
#include "TransferFunctionEditorChangeTransferFunctionNumberTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER
#include "TransferFunctionEditorColorFunctionSynthesizerTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER
#include "TransferFunctionEditorOpacityFunctionSynthesizerTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE
#include "TransferFunctionEditorOpacityFunctionVariableTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE
#include "TransferFunctionEditorColorFunctionVariableTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX
#include "TransferFunctionEditorColorMinMaxTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX
#include "TransferFunctionEditorOpacityMinMaxTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT
#include "TransferFunctionEditorColorMapEditTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT
#include "TransferFunctionEditorOpacityMapEditTest.h"
#endif

namespace
{
static kvs::qt::Application* g_pbvr_test_application = nullptr;

/**
 * @brief 保留中のQtイベントを処理し、遅延削除対象を解放します。
 */
void processPendingEvents()
{
    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
}

void cleanupBetweenTests()
{
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        if ( widget != nullptr )
        {
            widget->close();
        }
    }

    // ウィンドウのclose後に発生するイベントを2回処理し、子ウィンドウも確実に破棄します。
    processPendingEvents();
    processPendingEvents();
}

int qExecWithCleanup( QObject* test, int argc, char** argv )
{
    const int result = QTest::qExec( test, argc, argv );
    cleanupBetweenTests();
    return result;
}

/**
 * @brief 有効化されたテストを生成して実行し、共通の後処理を行います。
 * @tparam Test 実行するQTestテストクラス。
 * @param result これまでに実行したテストの結果を格納する変数。
 * @param has_enabled_test 有効なテストが存在するかを示す変数。
 * @param argc コマンドライン引数の数。
 * @param argv コマンドライン引数。
 */
template <typename Test>
void runEnabledTest( int& result, bool& has_enabled_test, int argc, char** argv )
{
    has_enabled_test = true;
    Test test;
    result |= qExecWithCleanup( &test, argc, argv );
}

int runEnabledTests( int argc, char** argv )
{
    int result = 0;
    bool has_enabled_test = false;
    cleanupBetweenTests();

#ifdef PBVR_ENABLE_TEST_MENUBAR
    runEnabledTest<MenuBarTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_SCREEN
    runEnabledTest<ScreenTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_PLAYBACKCONTROLTOOLBAR
    runEnabledTest<ClientTests::PlayBackControlToolBarTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TIMESTEPCONTROLTOOLBAR
    runEnabledTest<ClientTests::TimeStepControlToolBarTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_COLORMAPSELECTORTOOLBAR
    runEnabledTest<ClientTests::ColorMapSelectorToolBarTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TOTALPARTICLESTOOLBAR
    runEnabledTest<ClientTests::TotalParticlesToolBarTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_PREFERENCE
    runEnabledTest<ClientTests::PreferenceTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION
    runEnabledTest<ClientTests::CommunicationTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_USER_INFO
    runEnabledTest<ClientTests::CommunicationUserInfoTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SETTING
    runEnabledTest<ClientTests::CommunicationSettingTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SHARE_VIEW
    runEnabledTest<ClientTests::CommunicationShareViewTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_ANIMATIONCONTROL
    runEnabledTest<ClientTests::AnimationControlTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_GLYPHEDITOR
    runEnabledTest<ClientTests::GlyphEditorTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_OBJECTEDITOR
    runEnabledTest<ClientTests::ObjectEditorTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_PLOTOVERLINEEDITOR
    runEnabledTest<ClientTests::PlotOverLineEditorTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_PLOTOVERTIMEEDITOR
    {
        has_enabled_test = true;
        ClientTests::PlotOverTimeEditorTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_POINTSIZECONTROL
    runEnabledTest<ClientTests::PointSizeControlTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL
    runEnabledTest<ClientTests::RepetitionLevelControlTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_SHADINGCONTROL
    runEnabledTest<ClientTests::ShadingControlTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_SERVER
    runEnabledTest<ClientTests::ServerTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_INSITU_MODE
    {
        has_enabled_test = true;
        ClientTests::InSituModeTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_ENSEMBLE_TRANSFER_FUNCTION
    {
        has_enabled_test = true;
        ClientTests::EnsembleTransferFunctionTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL_INSITU
    {
        has_enabled_test = true;
        ClientTests::RepetitionLevelControlInSituTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_ENSEMBLE_INSITU_MODE
    {
        has_enabled_test = true;
        ClientTests::EnsembleInSituModeTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_VOLUMETRANSFORM
    runEnabledTest<ClientTests::VolumeTransformTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT
    runEnabledTest<TransferFunctionEditorTest::ImportExportTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORT_INVALID_FILE
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ImportInvalidFileTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER
    runEnabledTest<TransferFunctionEditorTest::ChangeTransferFunctionNumberTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER
    runEnabledTest<TransferFunctionEditorTest::ColorFunctionSynthesizerTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER
    runEnabledTest<TransferFunctionEditorTest::OpacityFunctionSynthesizerTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE
    runEnabledTest<TransferFunctionEditorTest::OpacityFunctionVariableTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE
    runEnabledTest<TransferFunctionEditorTest::ColorFunctionVariableTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX
    runEnabledTest<TransferFunctionEditorTest::ColorMinMaxTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX
    runEnabledTest<TransferFunctionEditorTest::OpacityMinMaxTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT
    runEnabledTest<TransferFunctionEditorTest::ColorMapEditTest>( result, has_enabled_test, argc, argv );
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT
    runEnabledTest<TransferFunctionEditorTest::OpacityMapEditTest>( result, has_enabled_test, argc, argv );
#endif

    if ( !has_enabled_test )
    {
        qInfo() << "No tests are enabled. Edit Client/Tests/TestsConfig.pri and set one of "
                   "TEST_ENABLE_MENUBAR, TEST_ENABLE_SCREEN, or "
                   "TEST_ENABLE_PLAYBACKCONTROLTOOLBAR, or "
                   "TEST_ENABLE_TIMESTEPCONTROLTOOLBAR, or "
                   "TEST_ENABLE_COLORMAPSELECTORTOOLBAR, or "
                   "TEST_ENABLE_TOTALPARTICLESTOOLBAR, or "
                   "TEST_ENABLE_PREFERENCE, or "
                   "TEST_ENABLE_COMMUNICATION, or "
                   "TEST_ENABLE_COMMUNICATION_USER_INFO, or "
                   "TEST_ENABLE_COMMUNICATION_SETTING, or "
                   "TEST_ENABLE_COMMUNICATION_SHARE_VIEW, or "
                   "TEST_ENABLE_ANIMATIONCONTROL, or "
                   "TEST_ENABLE_GLYPHEDITOR, or "
                   "TEST_ENABLE_OBJECTEDITOR, or "
                   "TEST_ENABLE_PLOTOVERLINEEDITOR, "
                   "TEST_ENABLE_PLOTOVERTIMEEDITOR, or "
                   "TEST_ENABLE_POINTSIZECONTROL, or "
                   "TEST_ENABLE_REPETITIONLEVELCONTROL, or "
                   "TEST_ENABLE_SHADINGCONTROL, or "
                   "TEST_ENABLE_SERVER, or "
                   "TEST_ENABLE_INSITU_MODE, or "
                   "TEST_ENABLE_ENSEMBLE_TRANSFER_FUNCTION, or "
                   "TEST_ENABLE_VOLUMETRANSFORM, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT, or "
                   "TEST_ENABLE_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT to 1.";
    }

    return result;
}
}

kvs::qt::Application* pbvrTestApplication()
{
    return g_pbvr_test_application;
}

void showTestWindowCentered( QWidget* window, int horizontal_offset )
{
    if ( window == nullptr )
    {
        return;
    }

    QScreen* screen = QGuiApplication::primaryScreen();
    if ( screen == nullptr )
    {
        window->show();
        return;
    }

    const QRect available = screen->availableGeometry();
    window->adjustSize();

    QSize window_size = window->size();
    if ( !window_size.isValid() || window_size.isEmpty() )
    {
        window_size = window->sizeHint();
    }

    const QSize max_window_size(
        qMax( 320, available.width() - 80 ),
        qMax( 240, available.height() - 80 ) );
    if ( window_size.width() > max_window_size.width() ||
         window_size.height() > max_window_size.height() )
    {
        window_size.setWidth( qMin( window_size.width(), max_window_size.width() ) );
        window_size.setHeight( qMin( window_size.height(), max_window_size.height() ) );
        window->resize( window_size );
    }

    QPoint top_left = available.center() - QPoint( window_size.width() / 2, window_size.height() / 2 );
    top_left.rx() += horizontal_offset;
    const int max_x = qMax( available.left(), available.right() - window_size.width() + 1 );
    const int max_y = qMax( available.top(), available.bottom() - window_size.height() + 1 );
    top_left.setX( qBound( available.left(), top_left.x(), max_x ) );
    top_left.setY( qBound( available.top(), top_left.y(), max_y ) );

    window->move( top_left );
    window->show();
    QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );

    if ( window->windowHandle() != nullptr )
    {
        window->windowHandle()->setScreen( screen );
    }

    QRect frame = window->frameGeometry();
    if ( frame.width() > max_window_size.width() ||
         frame.height() > max_window_size.height() )
    {
        const int frame_extra_width = frame.width() - window->width();
        const int frame_extra_height = frame.height() - window->height();
        QSize adjusted_size = window->size();
        adjusted_size.setWidth(
            qMin( adjusted_size.width(), qMax( 320, max_window_size.width() - frame_extra_width ) ) );
        adjusted_size.setHeight(
            qMin( adjusted_size.height(), qMax( 240, max_window_size.height() - frame_extra_height ) ) );
        window->resize( adjusted_size );
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
        frame = window->frameGeometry();
    }

    top_left = available.center() - QPoint( frame.width() / 2, frame.height() / 2 );
    top_left.rx() += horizontal_offset;
    const int max_frame_x = qMax( available.left(), available.right() - frame.width() + 1 );
    const int max_frame_y = qMax( available.top(), available.bottom() - frame.height() + 1 );
    top_left.setX( qBound( available.left(), top_left.x(), max_frame_x ) );
    top_left.setY( qBound( available.top(), top_left.y(), max_frame_y ) );
    window->move( top_left );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
}

int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_pbvr_test_application = &app;
    return runEnabledTests( argc, argv );
}
