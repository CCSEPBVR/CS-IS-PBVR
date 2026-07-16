#include <QCoreApplication>
#include <QEvent>
#include <QEventLoop>
#include <QDebug>
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

#ifdef PBVR_ENABLE_TEST_VOLUMETRANSFORM
#include "VolumeTransformTest.h"
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT
#include "TransferFunctionEditorImportExportTest.h"
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

void cleanupBetweenTests()
{
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        if ( widget != nullptr )
        {
            widget->close();
        }
    }

    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );

    QCoreApplication::sendPostedEvents( nullptr, 0 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
    QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 200 );
}

int qExecWithCleanup( QObject* test, int argc, char** argv )
{
    const int result = QTest::qExec( test, argc, argv );
    cleanupBetweenTests();
    return result;
}

int runEnabledTests( int argc, char** argv )
{
    int result = 0;
    bool has_enabled_test = false;
    cleanupBetweenTests();

#ifdef PBVR_ENABLE_TEST_MENUBAR
    {
        has_enabled_test = true;
        MenuBarTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_SCREEN
    {
        has_enabled_test = true;
        ScreenTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_PLAYBACKCONTROLTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::PlayBackControlToolBarTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TIMESTEPCONTROLTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::TimeStepControlToolBarTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COLORMAPSELECTORTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::ColorMapSelectorToolBarTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TOTALPARTICLESTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::TotalParticlesToolBarTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_PREFERENCE
    {
        has_enabled_test = true;
        ClientTests::PreferenceTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION
    {
        has_enabled_test = true;
        ClientTests::CommunicationTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_USER_INFO
    {
        has_enabled_test = true;
        ClientTests::CommunicationUserInfoTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SETTING
    {
        has_enabled_test = true;
        ClientTests::CommunicationSettingTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SHARE_VIEW
    {
        has_enabled_test = true;
        ClientTests::CommunicationShareViewTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_ANIMATIONCONTROL
    {
        has_enabled_test = true;
        ClientTests::AnimationControlTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_GLYPHEDITOR
    {
        has_enabled_test = true;
        ClientTests::GlyphEditorTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_OBJECTEDITOR
    {
        has_enabled_test = true;
        ClientTests::ObjectEditorTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_PLOTOVERLINEEDITOR
    {
        has_enabled_test = true;
        ClientTests::PlotOverLineEditorTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_POINTSIZECONTROL
    {
        has_enabled_test = true;
        ClientTests::PointSizeControlTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL
    {
        has_enabled_test = true;
        ClientTests::RepetitionLevelControlTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_SHADINGCONTROL
    {
        has_enabled_test = true;
        ClientTests::ShadingControlTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_SERVER
    {
        has_enabled_test = true;
        ClientTests::ServerTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
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

#ifdef PBVR_ENABLE_TEST_VOLUMETRANSFORM
    {
        has_enabled_test = true;
        ClientTests::VolumeTransformTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ImportExportTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ChangeTransferFunctionNumberTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorFunctionSynthesizerTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityFunctionSynthesizerTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityFunctionVariableTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorFunctionVariableTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorMinMaxTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityMinMaxTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorMapEditTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityMapEditTest test;
        result |= qExecWithCleanup( &test, argc, argv );
    }
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
                   "TEST_ENABLE_PLOTOVERLINEEDITOR, or "
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
