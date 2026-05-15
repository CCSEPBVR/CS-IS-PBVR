#include <QCoreApplication>
#include <QDebug>
#include <QTest>

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

int runEnabledTests( int argc, char** argv )
{
    int result = 0;
    bool has_enabled_test = false;

#ifdef PBVR_ENABLE_TEST_MENUBAR
    {
        has_enabled_test = true;
        MenuBarTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_SCREEN
    {
        has_enabled_test = true;
        ScreenTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_PLAYBACKCONTROLTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::PlayBackControlToolBarTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TIMESTEPCONTROLTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::TimeStepControlToolBarTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COLORMAPSELECTORTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::ColorMapSelectorToolBarTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TOTALPARTICLESTOOLBAR
    {
        has_enabled_test = true;
        ClientTests::TotalParticlesToolBarTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_PREFERENCE
    {
        has_enabled_test = true;
        ClientTests::PreferenceTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION
    {
        has_enabled_test = true;
        ClientTests::CommunicationTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_USER_INFO
    {
        has_enabled_test = true;
        ClientTests::CommunicationUserInfoTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SETTING
    {
        has_enabled_test = true;
        ClientTests::CommunicationSettingTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_COMMUNICATION_SHARE_VIEW
    {
        has_enabled_test = true;
        ClientTests::CommunicationShareViewTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_ANIMATIONCONTROL
    {
        has_enabled_test = true;
        ClientTests::AnimationControlTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_GLYPHEDITOR
    {
        has_enabled_test = true;
        ClientTests::GlyphEditorTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_OBJECTEDITOR
    {
        has_enabled_test = true;
        ClientTests::ObjectEditorTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_PLOTOVERLINEEDITOR
    {
        has_enabled_test = true;
        ClientTests::PlotOverLineEditorTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_POINTSIZECONTROL
    {
        has_enabled_test = true;
        ClientTests::PointSizeControlTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_REPETITIONLEVELCONTROL
    {
        has_enabled_test = true;
        ClientTests::RepetitionLevelControlTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_SHADINGCONTROL
    {
        has_enabled_test = true;
        ClientTests::ShadingControlTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_VOLUMETRANSFORM
    {
        has_enabled_test = true;
        ClientTests::VolumeTransformTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_IMPORTEXPORT
    {
        has_enabled_test = true;
        TransferFunctionEditor::ImportExportTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_CHANGE_TRANSFER_FUNCTION_NUMBER
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ChangeTransferFunctionNumberTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_SYNTHESIZER
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorFunctionSynthesizerTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_SYNTHESIZER
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityFunctionSynthesizerTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_FUNCTION_VARIABLE
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityFunctionVariableTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_FUNCTION_VARIABLE
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorFunctionVariableTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MIN_MAX
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorMinMaxTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MIN_MAX
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityMinMaxTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_COLOR_MAP_EDIT
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::ColorMapEditTest test;
        result |= QTest::qExec( &test, argc, argv );
    }
#endif

#ifdef PBVR_ENABLE_TEST_TRANSFERFUNCTIONEDITOR_OPACITY_MAP_EDIT
    {
        has_enabled_test = true;
        TransferFunctionEditorTest::OpacityMapEditTest test;
        result |= QTest::qExec( &test, argc, argv );
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

int main( int argc, char** argv )
{
    QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs );
    kvs::qt::Application app( argc, argv );
    g_pbvr_test_application = &app;
    return runEnabledTests( argc, argv );
}
