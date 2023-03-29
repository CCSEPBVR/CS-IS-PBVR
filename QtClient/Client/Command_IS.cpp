//HASCHANGES MANY CHANGES
#include <algorithm>
#include <string.h>
#include "ParticleTransferClient.h"

#include <kvs/glut/Screen>
#include <kvs/glut/Timer>

#include "ParticleMerger.h"
#include "ParticleServer.h"
#include "Command.h"
#include "PVRenderer.h"
//#include "Controller.h"
#include "TimerEvent.h"
//#include "TFE_main.h"

//#include "Profiler.h"

#include <kvs/Directory>
#include <kvs/File>

#include "timer_simple.h"
#include "Panels/particlepanel.h"
#include <Panels/filterinfopanel.h>
#include <Panels/transferfunceditor_IS.h>
#include "Panels/particlepanel.h"
#include "Panels/coordinatepanel.h"
#include "Panels/timecontrolpanel.h"
#include "Panels/systemstatuspanel.h"
#include "Panels/renderoptionpanel.h"
#include "QGlue/renderarea.h"
#include "v3defines.h"
//#include "AnimationControl.h"

using namespace kvs::visclient;

// APPEND START fp)m.tanaka 2013.09.01
int subpixellevela = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
int subpixelleveld = PBVR_SUB_PIXEL_LEVEL_DETAILED;
extern int repeatlevela;
extern int repeatleveld;
//extern float pdensitylevel;

//int plimitlevel = PBVR_PARTICLE_LIMIT;

//float resolution_width_level;
//float resolution_height_level;

// APPEND START fp)m.takizawa 2014.05.21
float Command::PVBRmaxcoords[3]={0.0,0.0,0.0};
float Command::PVBRmincoords[3]={0.0,0.0,0.0};
// APPEND END fp)m.takizawa 2014.05.21


// APPEND START fp)m.takizawa 2014.05.29
static int pretimestep  = -1;
//extern	void	ScreenShot(  kvs::ScreenBase* screen, int tstep );
// APPEND END fp)m.takizawa 2014.05.29

//extern auto_ptr<kvs::visclient::ProgressBar> *g_screen3_progress;
// APPEND END fp)m.tanaka 2013.09.01

static int numvol, numstep = 0;
#ifdef _TIMER_
#define TIMER_MAX_COUNT 9
static int timer_count = 0;
#endif

char pfi_path_server[256];
static char pre_pfi_path_server[256];
static int exportMergedParticleStartStep = -1;
char filter_parameter_filename[256];
#if 1 // FEAST modify 20160128
//extern int TimecontrolPanel::g_curStep;
#endif // FEAST modify 20160128 end

Command::Command( ParticleServer* server ) :
    isUnderAnimation( true ),
    isUnderCommunication( true ),
    isShown( true ),
    isImportTransferFunctionParam( false ),
    isKeyFrameAnimation( false ),
    server( server ),
    lastSamplingParams( NULL ),
    m_particle_assign_flag( false )
{
    param.timeStepKeyFrame = -1;
    stepKeyFrame = 0;
    preKeyFrame  = 0;
};

Command::~Command()
{
    delete lastSamplingParams;
}

void Command::startAbstractAnalysis()
{
    param.transferType = PBVRParam::Abstract;
//    controller->detail->setButtonColor( ::Gray );
//	param.timeStep = 1;
    param.timeStep = 0;
// APPEND START fp)m.tanaka 2013.09.01
    param.detailedRepeatLevel = subpixelleveld;
    param.abstractRepeatLevel = subpixellevela;
    std::cout << "*** START ABSTRACTANALYSIS" << std::endl;
// APPEND END   fp)m.tanaka 2013.09.01

}

void Command::startDetailedAnalysis()
{
    param.transferType = PBVRParam::Detailed;
    /* removed 2014.07.21
        controller->abst->setButtonColor( ::Gray );
        generateDetailedParticles();
    */
    param.timeStep = param.minTimeStep;
    param.maxTimeStep = param.maxServerTimeStep;
    std::cout << "*** START DETAILEDANALYSIS" << std::endl;
    std::cout << "param.minTimeStep:" << param.minTimeStep << std::endl;
    std::cout << "param.maxTimeStep:" << param.maxTimeStep << std::endl;
}

void Command::changeShowHide()
{
//    if ( isShown )
//    {
//        progress->hide();
//        timeSlider->hide();
//        minTimeSlider->hide();
//        maxTimeSlider->hide();
//    }
//    else
//    {
//        timeSlider->show();
//#if 0 //delete show jupiter
//        progress->show();
//        minTimeSlider->show();
//        maxTimeSlider->show();
//#endif
//    }

    isShown = !isShown;
}

void Command::preupdate()
{
#ifdef _TIMER_
    if ( timer_count == 0 ) TIMER_INIT();
    timer_count++;
#endif
    TIMER_STA( 1 );

    // for Particle Merge
    param.particle_merge_param.crt_time = param.timeStep;
    ParticleMergeUI->setParam( &param.particle_merge_param );
    for ( int i = 0; i < 11; i++ )
    {
        if ( param.particle_merge_param.particles[i].enable == true )
        {
            printf( "particles[%d] = true : \n", i );
        }
        else
        {
            printf( "particles[%d] = false : \n", i );
        }

        if ( param.particle_merge_param.particles[i].filepath.empty() != true )
        {
            printf( "filepath = %s\n", param.particle_merge_param.particles[i].filepath.c_str() );
        }
        else
        {
            printf( "filepath = Empty\n" );
        }
    }
#if 0 // FEAST modify 20160128
    if ( isUnderAnimation )
    {
        if ( param.transferType == PBVRParam::Detailed )
        {
#ifndef CPUMODE
            if ( param.repeatLevel == param.detailedRepeatLevel )
            {
#endif
                param.timeStep++;
#ifndef CPUMODE
            }
#endif
        }
        else if ( param.transferType == PBVRParam::Abstract )
        {
#ifndef CPUMODE
            if ( param.repeatLevel == param.abstractRepeatLevel )
            {
#endif
                param.timeStep++;
#ifndef CPUMODE
            }
#endif
        }
    }
    if ( param.timeStep > maxTimeSlider->value() ) //99) {
    {
        //param.timeStep = 1;
        //param.timeStep = 0;
        //if( param.transferType == PBVRParam::Detailed ) param.timeStep = minTimeSlider->value();
// APPEND START fj) 2015.04.03
        if ( minTimeSlider->value() >= param.minTimeStep )
        {
            param.timeStep = minTimeSlider->value();
        }
        else
        {
            param.timeStep = param.minTimeStep;
        }
// APPEND END   fj) 2015.04.03
    }
#endif // FEAST modify 20160128 end

    // for KeyFrameAnimation
    if ( isKeyFrameAnimation )
    {
        param.timeStep = param.timeStepKeyFrame;
        if ( param.timeStep < param.minTimeStep )
        {
            param.timeStep = param.minTimeStep;
        }
        if ( param.timeStep > param.maxTimeStep )
        {
            param.timeStep = param.maxTimeStep;
        }
    }

    const size_t maxMemory = 10 * 1024 * 1024;
    if ( getUsingMemoryByKiloByte() > maxMemory )
    {
        kvsMessageError( "memory over: %d KB / %d KB.", getUsingMemoryByKiloByte(), maxMemory );
        kvsMessageError( "communication cancel." );
        isUnderCommunication = false;
    }

#if 0 // FEAST modify 20160128
    if ( isUnderCommunication )
    {
        if ( param.transferType == PBVRParam::Abstract )
        {
            progress->setValue( static_cast<float>( param.timeStep ) );
            timeSlider->setValue( static_cast<float>( param.timeStep ) );
        }
        else if ( param.transferType == PBVRParam::Detailed )
        {
            if ( param.timeStep > param.maxTimeStep )
            {
                param.timeStep = param.minTimeStep;
            }
            float ratio = float( param.timeStep - param.minTimeStep ) / float( param.maxTimeStep - param.minTimeStep ) * 100;
            progress->setValue( static_cast<float>( ratio ) );
            timeSlider->setValue( static_cast<float>( param.timeStep ) );
        }
    }
#endif // FEAST modify 20160128 end

    if ( isUnderCommunication )
    {
        if ( param.transferType == PBVRParam::Abstract )
        {
            if ( param.detailedTransferType == PBVRParam::Divided )
            {
                if ( param.repeatLevel == param.abstractRepeatLevel )
                {
                    param.repeatLevel = 1;
                }
            }
        }
        else if ( param.transferType == PBVRParam::Detailed )
        {
            if ( param.detailedTransferType == PBVRParam::Divided )
            {
                if ( param.repeatLevel == param.detailedRepeatLevel )
                {
                    param.repeatLevel = 1;
                }
            }
        }
    }

    // for export Particles
    if ( param.particle_merge_param.doExport )
    {
        printf( " ### [debug]: exportMergedParticleStartStep = %d, param.timeStep = %d\n", exportMergedParticleStartStep, param.timeStep );
        if ( exportMergedParticleStartStep < 0 )
        {
            exportMergedParticleStartStep = param.timeStep;
        }
        else
        {
            if ( exportMergedParticleStartStep == param.timeStep )
            {
                param.particle_merge_param.doExport = false;
                exportMergedParticleStartStep = -1;
                ParticleMergeUI->particleExportButtonActive();
            }
        }
    }
}

void Command::update( PBVRParam& param, PBVRResult* result )
{
    // Viewer のスケール正規化用
    bool resetflag = false;
    bool server_resetflag = false;
    bool local_particle_exits = false;
    float local_crd[6], server_crd[6], crd[6];

    if ( param.CSmode == 0 && param.particle_merge_param.doExport )
    {
        ParticleMerger merger;
        merger.setParam( param.particle_merge_param, param.minServerTimeStep, param.maxServerTimeStep );
        size_t time_start = merger.getMergedInitialTimeStep();
        size_t time_end   = merger.getMergedFinalTimeStep();

        for ( size_t step = time_start; step <= time_end; step++ )
        {
            kvs::PointObject* p = NULL;
            kvs::PointObject object = *serverParticles[step];
            if ( object.nvertices() <= 1 )
            {
                p = merger.doMerge( NULL, step );
            }
            else
            {
                p = merger.doMerge( &object, step );
            }
            delete p;
        }
        param.particle_merge_param.doExport = false;
    }

    result->isUnderAnimation = false;
    if ( isUnderCommunication )
    {
        if ( param.norepsampling && isSamplingParamsKeeped( param ) )
        {
            // When Transfer Functions are not edited.
            std::cout << "*** Transfer Functions are not edited." << std::endl;
        }
        else
        {
            // When Transfer Functions are edited.
            if ( param.transferType == PBVRParam::Abstract )
            {

                if ( param.detailedTransferType == PBVRParam::Divided )
                {
                    std::cout << "*** param.detailedTransferType == PBVRParam::Divided" << std::endl;
#ifndef CPUMODE
                    std::cout << "param.repeatLevel=" << param.repeatLevel << " param.abstractRepeatLevel=" << param.abstractRepeatLevel << std::endl;
                    if ( param.repeatLevel == param.abstractRepeatLevel )
                    {
#endif
                        //delete abstractParticles[param.timeStep];
                        abstractParticles[param.timeStep] = server->getPointObjectFromServer( param, result, numvol );
#ifndef CPUMODE
                    }
                    else if ( param.repeatLevel < param.abstractRepeatLevel )
                    {
                        PointObject* dividedObject = server->getPointObjectFromServer( param, result, numvol );

                        abstractParticles[param.timeStep]->add( *dividedObject );
                        delete dividedObject;
                    }
                    else
                    {
                        assert( false );
                    }
#endif
                }
                else if ( param.detailedTransferType == PBVRParam::Summalized )
                {
                    //delete abstractParticles[param.timeStep];
                    abstractParticles[param.timeStep] = server->getPointObjectFromServer( param, result, numvol );
                }
                else
                {
                    assert( false );
                }
            }
            else if ( param.transferType == PBVRParam::Detailed )
            {

                if ( param.detailedTransferType == PBVRParam::Divided )
                {
#ifndef CPUMODE
                    if ( param.repeatLevel == param.detailedRepeatLevel )
                    {
#endif
                        //delete detailedParticles[param.timeStep];
                        detailedParticles[param.timeStep] = server->getPointObjectFromServer( param, result, numvol );
#ifndef CPUMODE
                    }
                    else if ( param.repeatLevel < param.detailedRepeatLevel )
                    {
                        PointObject* dividedObject = server->getPointObjectFromServer( param, result, numvol );
                        detailedParticles[param.timeStep]->add( *dividedObject );
                        delete dividedObject;
                    }
                    else
                    {
                        assert( false );
                    }
#endif
                }
                else if ( param.detailedTransferType == PBVRParam::Summalized ) //jupiter_branch
                {
                    kvs::PointObject* tmp_po = server->getPointObjectFromServer( param, result, numvol );
                    if ( result->serverTimeStep > -1 ) param.timeStep = result->serverTimeStep;
                    param.minServerTimeStep = result->minServerTimeStep ;
                    param.maxServerTimeStep = result->maxServerTimeStep ;
                    this->param.minServerTimeStep = param.minServerTimeStep ;
                    this->param.maxServerTimeStep = param.maxServerTimeStep ;
                    //generateAbstractParticles( ( PBVRParam& )param, result );

                    this->result = *result;
                    generateDetailedParticles();
                    detailedParticles[param.timeStep] = tmp_po;
                    FilterinfoPanel::updateFilterInfo(result);
                    SystemstatusPanel::updateSystemStatus(result->numParticle);
                }
                else
                {
                    assert( false );
                }
            }
            else
            {
                assert( false );
            }

            if ( serverParticles.size() < param.timeStep + 1 )
            {
                serverParticles.resize( param.timeStep + 1, NULL );
            }
//          delete serverParticles[param.timeStep];
            serverParticles[param.timeStep] = detailedParticles[param.timeStep];
            m_particle_assign_flag = true;
            isUnderAnimation = result->isUnderAnimation;
        }
        PVBRmaxcoords[0] = result->maxObjectCoord[0];
        PVBRmaxcoords[1] = result->maxObjectCoord[1];
        PVBRmaxcoords[2] = result->maxObjectCoord[2];

        PVBRmincoords[0] = result->minObjectCoord[0];
        PVBRmincoords[1] = result->minObjectCoord[1];
        PVBRmincoords[2] = result->minObjectCoord[2];

        // Merge Point Object
        ParticleMerger merger;
        kvs::PointObject* server_object = serverParticles[param.timeStep];
        kvs::PointObject* object;
        merger.setParam( param.particle_merge_param, param.minServerTimeStep, param.maxServerTimeStep );
        object = merger.doMerge( server_object, param.timeStep );

        result->minMergedTimeStep = merger.getMergedInitialTimeStep();
        result->maxMergedTimeStep = merger.getMergedFinalTimeStep();
        detailedParticles[param.timeStep] = object;

        // set last TransferFunction
        if ( param.timeStep == result->maxMergedTimeStep )
        {
            saveSamplingParams( param );
        }

        // Viewerの正規化用処理
        local_particle_exits = merger.CalcExternalCoords( local_crd );
        //サーバで計算されたsubpixel levelの更新.
        if ( param.CSmode == 1 )
        {
            local_subpixel_level = result->subPixelLevel;
        }
        else
        {
            local_subpixel_level = object->size( 0 );
        }

//#if 0
//        if ( maxTimeSlider->maxValue() != result->maxMergedTimeStep
//                || minTimeSlider->minValue() != result->minMergedTimeStep )
//        {
//            minTimeSlider->hide();
//            maxTimeSlider->hide();
//        }
//#endif
//        if ( maxTimeSlider->maxValue() != result->maxMergedTimeStep )
//        {
//            param.maxTimeStep = result->maxMergedTimeStep;
//            maxTimeSlider->setValue( static_cast<float>( param.maxTimeStep ) );
//        }

//        if ( minTimeSlider->minValue() != result->minMergedTimeStep )
//        {
//            param.minTimeStep = result->minMergedTimeStep;
//            minTimeSlider->setValue( static_cast<float>( param.minTimeStep ) );
//        }

//        if ( maxTimeSlider->maxValue() != result->maxMergedTimeStep
//                || minTimeSlider->minValue() != result->minMergedTimeStep )
//        {
//            maxTimeSlider->setRange( param.minTimeStep, param.maxTimeStep );
//            minTimeSlider->setRange( param.minTimeStep, param.maxTimeStep );
//            timeSlider->setRange( param.minTimeStep, param.maxTimeStep );
//#if 0 //delete show (jupiter)
//            maxTimeSlider->show();
//            minTimeSlider->show();
//#endif
//        }

////        timeSlider->hide();
//        size_t timeStep = std::max( param.minTimeStep, std::min( param.maxTimeStep, param.timeStep ) );

////      timeSlider->setCaption( "Time step" );
//        timeSlider->setRange( param.minTimeStep, param.maxTimeStep );
////        timeSlider->show();
        TimecontrolPanel::requestUpdate(&param, result);

        // reallocate abstractParticles and detailedParticles
        if ( param.maxTimeStep > detailedParticles.size() - 1 )
        {
            generateAbstractParticles( param, result );
            this->param = param;
            this->result = *result;
            generateDetailedParticles();
        }

        this->param.timeStep    = param.timeStep;
        this->param.maxTimeStep = param.maxTimeStep;
        this->param.minTimeStep = param.minTimeStep;
    }

    // printf(" [debug] current pfi file. %s & %s\n", pre_pfi_path_server, pfi_path_server);
#if 0
    if ( param.CSmode == 1 && strcmp( pre_pfi_path_server, pfi_path_server ) )
    {
        // printf(" [debug] change pfi file. %s -> %s\n", pre_pfi_path_server, pfi_path_server);
        numstep = 0;
        param.maxServerTimeStep = 0;
        // isImportTransferFunctionParam = false;
        generateAbstractParticles ( ( PBVRParam& )param, result );
        this->param = param;
        this->result = *result;
        generateDetailedParticles();

        timeSlider->hide();
        minTimeSlider->hide();
        maxTimeSlider->hide();
        timeSlider->setRange( param.minTimeStep, param.maxTimeStep );
        minTimeSlider->setRange( param.minTimeStep, param.maxTimeStep );
        maxTimeSlider->setRange( param.minTimeStep, param.maxTimeStep );
        timeSlider->setValue( static_cast<float>( param.minTimeStep ) );
        minTimeSlider->setValue( static_cast<float>( param.minTimeStep ) );
        maxTimeSlider->setValue( static_cast<float>( param.maxTimeStep ) );
        timeSlider->show();
//delete show (jupiter)
#if 0
        minTimeSlider->show();
        maxTimeSlider->show();
#endif

        // For change view (Reset scale)
        server_resetflag = true;
    }
#endif
    // For change view (Reset scale)
    if ( ParticleMergeUI->checkChangeFlag() )
    {
        // 粒子統合の表示,非表示フラグが変化した
        if ( param.CSmode != 1 )
        {
            // スタンドアローンモードで、統合粒子が指定されているならば
            if ( local_particle_exits )
            {
                // クライアントの粒子情報で設定
                crd[0] = local_crd[0];
                crd[1] = local_crd[1];
                crd[2] = local_crd[2];
                crd[3] = local_crd[3];
                crd[4] = local_crd[4];
                crd[5] = local_crd[5];
                resetflag = true;
            }
        }
        else
        {
            // クラサバモードでlocalの統合粒子も指定されているならば
            if ( local_particle_exits )
            {
                // サーバの粒子情報とlocalの粒子情報を比較して範囲が広い方を採用
                crd[0] = std::min( local_crd[0], PVBRmincoords[0] );
                crd[1] = std::min( local_crd[1], PVBRmincoords[1] );
                crd[2] = std::min( local_crd[2], PVBRmincoords[2] );
                crd[3] = std::max( local_crd[3], PVBRmaxcoords[0] );
                crd[4] = std::max( local_crd[4], PVBRmaxcoords[1] );
                crd[5] = std::max( local_crd[5], PVBRmaxcoords[2] );
                resetflag = true;
            }
            else
            {
                // localの統合粒子が１つも選択されていない状態なのでサーバの粒子情報で設定
                crd[0] = PVBRmincoords[0];
                crd[1] = PVBRmincoords[1];
                crd[2] = PVBRmincoords[2];
                crd[3] = PVBRmaxcoords[0];
                crd[4] = PVBRmaxcoords[1];
                crd[5] = PVBRmaxcoords[2];
                resetflag = true;
            }
        }
        ParticleMergeUI->particleResetChangedFlag();
    }
    else if ( server_resetflag )
    {
        // PFIファイルが切り替わった
        if ( local_particle_exits )
        {
            // サーバの粒子情報とlocalの粒子情報を比較して範囲が広い方を採用
            crd[0] = std::min( local_crd[0], PVBRmincoords[0] );
            crd[1] = std::min( local_crd[1], PVBRmincoords[1] );
            crd[2] = std::min( local_crd[2], PVBRmincoords[2] );
            crd[3] = std::max( local_crd[3], PVBRmaxcoords[0] );
            crd[4] = std::max( local_crd[4], PVBRmaxcoords[1] );
            crd[5] = std::max( local_crd[5], PVBRmaxcoords[2] );
            resetflag = true;
        }
        else
        {
            // サーバの粒子情報で設定
            crd[0] = PVBRmincoords[0];
            crd[1] = PVBRmincoords[1];
            crd[2] = PVBRmincoords[2];
            crd[3] = PVBRmaxcoords[0];
            crd[4] = PVBRmaxcoords[1];
            crd[5] = PVBRmaxcoords[2];
            resetflag = true;
        }
    }
    else
    {
        crd[0] = PVBRmincoords[0];
        crd[1] = PVBRmincoords[1];
        crd[2] = PVBRmincoords[2];
        crd[3] = PVBRmaxcoords[0];
        crd[4] = PVBRmaxcoords[1];
        crd[5] = PVBRmaxcoords[2];
        resetflag = true;
    }
    // change view
    if ( resetflag )
    {
        kvs::Vector3f min_t( crd[0], crd[1], crd[2] );
        kvs::Vector3f max_t( crd[3], crd[4], crd[5] );
        screen->objectManager()->object()->setMinMaxObjectCoords( min_t, max_t );
        screen->objectManager()->object()->setMinMaxExternalCoords( min_t, max_t );
        screen->objectManager()->updateExternalCoords();
        std::cout << " !!!!!!!!!!!!!!!!!!! Reset Viewer Scale !!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
    }

   FilterinfoPanel::updateFilterInfo();
}

void Command::postupdate()
{
    if ( param.CSmode == 0 )
    {
        ParticleMergeUI->particleExportButtonActive();
    }

    if ( isUnderCommunication )
    {
        if ( param.transferType == PBVRParam::Abstract )
        {
            if ( param.detailedTransferType == PBVRParam::Divided )
            {
                if ( param.repeatLevel < param.abstractRepeatLevel )
                {
                    param.repeatLevel++;
                }
            }
            else if ( param.detailedTransferType == PBVRParam::Summalized )
            {
                param.repeatLevel = param.abstractRepeatLevel;
            }
        }
        else if ( param.transferType == PBVRParam::Detailed )
        {
            if ( param.detailedTransferType == PBVRParam::Divided )
            {
                if ( param.repeatLevel < param.detailedRepeatLevel )
                {
                    param.repeatLevel++;
                }
            }
            else if ( param.detailedTransferType == PBVRParam::Summalized )
            {
                param.repeatLevel = param.detailedRepeatLevel;
            }
        }
    }
    param.detailedSubpixelLevel = local_subpixel_level;
#ifdef CPUMODE
    renderer->setSubpixelLevel( param.detailedSubpixelLevel );
    renderer->recreateImageBuffer();
#endif
//  Profiler::get()->start( "rendering time" );
    TIMER_STA( 150 );

#ifndef CPUMODE
    if ( param.transferType == PBVRParam::Abstract )
    {
#ifndef CPUMODE
        renderer->setRepetitionLevel( param.abstractRepeatLevel );
#endif
        renderer->setSubpixelLevel( param.abstractSubpixelLevel );
        PointObject* object = abstractParticles[param.timeStep];
        renderer->changePointObject( object );
    }
    else if ( param.transferType == PBVRParam::Detailed )
    {
#ifndef CPUMODE
        renderer->setRepetitionLevel( param.repeatLevel );
#endif
        renderer->setSubpixelLevel( param.detailedSubpixelLevel );
        PointObject* object = detailedParticles[param.timeStep];
        renderer->changePointObject( object );
    }
    else
    {
        assert( false );
    }
#endif

#if 0  // FEAST modify 20160128
    if ( pretimestep != param.timeStep )
    {
        ScreenShot( screen, param.timeStep );

        pretimestep = param.timeStep;
    }
#else
    if ( pretimestep != TimecontrolPanel::g_curStep )
    {
        RenderArea::ScreenShot( screen, TimecontrolPanel::g_curStep );

        pretimestep = TimecontrolPanel::g_curStep;
    }
#endif  // FEAST modify 20160128 end

    TIMER_END( 150 );
//  Profiler::get()->end( "rendering time" );
    if ( isUnderAnimation )
    {
//      Profiler::get()->writeToTerminal();
#if !(defined(_DEBUG) && defined(WIN32))
//      Profiler::get()->writeToCSV( param.timeStep, "profile.csv" );
#endif
    }
    if ( isImportTransferFunctionParam == false )
    {
        apply_variable_range( result.varRange );
    }
    extransfuncedit->updateRangeView();

    TIMER_END( 1 );
#ifdef _TIMER_
    if ( timer_count == TIMER_MAX_COUNT ) TIMER_FIN();
#endif

}

void Command::getServerParticleInfomation( PBVRParam& param, PBVRResult* result)
{
//    std::cout <<"IN Command::getServerParticleInfomation"<<std::endl;
    kvs::Vector3f minObjectCoord;
    kvs::Vector3f maxObjectCoord;
    if (param.CSmode == 1)
    {
        if (param.maxServerTimeStep == 0)
        {
            jpv::ParticleTransferClient client( param.hostname, param.port );
            jpv::ParticleTransferClientMessage message;
            jpv::ParticleTransferServerMessage reply;

            client.initClient();

            reply.camera = new kvs::Camera();

            // Send inputDir
            strncpy( message.header, "JPTP /1.0\r\n", 11 );
            message.initParam = -3;
            // message.inputDir = param.inputdir;
            message.inputDir = std::string( pfi_path_server );
            strcpy( pre_pfi_path_server, pfi_path_server );
            // transfer function
            param.paramExTransFunc.applyToClientMessage( &message );

            message.messageSize = message.byteSize();
            client.sendMessage( message );

            // Recive server condition
            client.recvMessage( reply );
            numvol = reply.numVolDiv;
            param.minServerTimeStep = reply.staStep;
            param.maxServerTimeStep = reply.endStep;
            param.minTimeStep = reply.staStep;
            param.maxTimeStep = reply.endStep;
            param.timeStep = reply.timeStep;

            minObjectCoord = kvs::Vector3f( reply.minObjectCoord );
            maxObjectCoord = kvs::Vector3f( reply.maxObjectCoord );
            PVBRmaxcoords[0] = reply.maxObjectCoord[0];
            PVBRmaxcoords[1] = reply.maxObjectCoord[1];
            PVBRmaxcoords[2] = reply.maxObjectCoord[2];

            PVBRmincoords[0] = reply.minObjectCoord[0];
            PVBRmincoords[1] = reply.minObjectCoord[1];
            PVBRmincoords[2] = reply.minObjectCoord[2];

            RenderoptionPanel::plimitlevel      = reply.particle_limit;
            RenderoptionPanel::pdensitylevel    = reply.particle_density;
            particle_data_size_limit_input = reply.particle_data_size_limit;
            RenderoptionPanel::resolution_width_level = reply.camera->windowWidth();
            RenderoptionPanel::resolution_height_level= reply.camera->windowHeight();

            float minValue = reply.minValue;
            float maxValue = reply.maxValue;
            std::cout << "minValue: " << minValue
                << " maxValue: " << maxValue << std::endl;
            std::cout << "minObjectCoord: " << minObjectCoord << std::endl;
            std::cout << "maxObjectCoord: " << maxObjectCoord << std::endl;

            param.paramExTransFunc.importFromServerMessage( reply );
            if (isImportTransferFunctionParam == false)
            {
                apply_variable_range( reply.varRange );
            }


            /* delete for 140521 server */
            strncpy( message.header, "JPTP /1.0\r\n", 11 );
            message.initParam = -1;
            message.messageSize = message.byteSize();
            client.sendMessage( message );
            client.recvMessage( reply );
            client.termClient();
            delete reply.camera;
        }
    }
//    std::cout<<" END Command::getServerParticleInfomation"<<std::endl;

}

void Command::initializeAbstractParticles( PBVRParam& param, PBVRResult* result, int localminstep, int localmaxstep )
{
    // stepの範囲を示すparamのメンバー変数に設定を行う
    param.minTimeStep = localminstep;
    param.maxTimeStep = localmaxstep;

    std::cout << "Command::generateAbstractParticles numvol " << numvol << std::endl;
    std::cout << "Command::generateAbstractParticles numstep " << param.maxServerTimeStep << std::endl;
    std::cout << "Command::generateAbstractParticles minstep " << param.minTimeStep << std::endl;
    std::cout << "Command::generateAbstractParticles maxstep " << param.maxTimeStep << std::endl;

    for (size_t i = 0; i < abstractParticles.size(); ++i)
    {
        delete abstractParticles[i];
    }

    abstractParticles.clear();

    {
        size_t mt = (param.maxTimeStep > param.maxServerTimeStep) ? param.maxTimeStep : param.maxServerTimeStep;
        for (size_t i = 0; i < mt + 1; ++i)
        {
            abstractParticles.push_back( server->getPointObjectFromLocal() );
        }
    }

    // サーバの情報が無い場合のViewer正規化処理
    // Viewer正規化処理をする
    if (param.CSmode == 1)
    {
        kvs::Vector3f minObjectCoord;
        kvs::Vector3f maxObjectCoord;
        minObjectCoord.set( PVBRmincoords );
        maxObjectCoord.set( PVBRmaxcoords );

        abstractParticles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
        abstractParticles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
    }else
    {
        abstractParticles.front() = server->getPointObjectFromServer( param, result, numvol );
    }
}


void Command::generateAbstractParticles( PBVRParam& param, PBVRResult* result)
{
//   std::cout<<" IN Command::generateAbstractParticles"<<std::endl;
    kvs::Vector3f minObjectCoord;
    kvs::Vector3f maxObjectCoord;
    if ( param.CSmode == 1 )
    {
        if ( param.maxServerTimeStep == 0 )
        {
            jpv::ParticleTransferClient client( param.hostname, param.port );
            jpv::ParticleTransferClientMessage message;
            jpv::ParticleTransferServerMessage reply;

            reply.camera = new kvs::Camera();

            client.initClient();

            // Send inputDir
            strncpy( message.header, "JPTP /1.0\r\n", 11 );
            message.initParam = -3;
            // message.inputDir = param.inputdir;
            message.inputDir = std::string( pfi_path_server );
            strcpy( pre_pfi_path_server, pfi_path_server );
            // transfer function
            param.paramExTransFunc.applyToClientMessage( &message );

            message.messageSize = message.byteSize();
            client.sendMessage( message );

            // Recive server condition
            client.recvMessage( reply );
            numvol = reply.numVolDiv;
            param.minServerTimeStep = reply.staStep;
            param.maxServerTimeStep = reply.endStep;
            param.minTimeStep = reply.staStep;
            param.maxTimeStep = reply.endStep;
            param.timeStep = reply.timeStep;

// APPEND START fp)m.takizawa 2014.05.21

            minObjectCoord = kvs::Vector3f( reply.minObjectCoord );
            maxObjectCoord = kvs::Vector3f( reply.maxObjectCoord );
            PVBRmaxcoords[0] = reply.maxObjectCoord[0];
            PVBRmaxcoords[1] = reply.maxObjectCoord[1];
            PVBRmaxcoords[2] = reply.maxObjectCoord[2];

            PVBRmincoords[0] = reply.minObjectCoord[0];
            PVBRmincoords[1] = reply.minObjectCoord[1];
            PVBRmincoords[2] = reply.minObjectCoord[2];

// APPEND END fp)m.takizawa 2014.05.21

            float minValue = reply.minValue;
            float maxValue = reply.maxValue;
            std::cout << "minValue: " << minValue
                      << " maxValue: " << maxValue << std::endl;
            std::cout << "minObjectCoord: " << minObjectCoord << std::endl;
            std::cout << "maxObjectCoord: " << maxObjectCoord << std::endl;

            if ( isImportTransferFunctionParam == false )
            {
                apply_variable_range( reply.varRange );
            }

            /* delete for 140521 server */
            strncpy( message.header, "JPTP /1.0\r\n", 11 );
            message.initParam = -1;
            message.messageSize = message.byteSize();
            client.sendMessage( message );
            client.recvMessage( reply );
            client.termClient();
            delete reply.camera;
        }

    }

    /*
    if ( localminstep > -1 && localmaxstep > -1 )
    {
        // stepの範囲を示すparamのメンバー変数に設定を行う
        param.minTimeStep = localminstep;
        param.maxTimeStep = localmaxstep;
    }
    */
    std::cout << "Command::generateAbstractParticles numvol " << numvol << std::endl;
    std::cout << "Command::generateAbstractParticles numstep " << param.maxServerTimeStep << std::endl;
    std::cout << "Command::generateAbstractParticles minstep " << param.minTimeStep << std::endl;
    std::cout << "Command::generateAbstractParticles maxstep " << param.maxTimeStep << std::endl;

    for ( size_t i = 0; i < abstractParticles.size(); ++i )
    {
        delete abstractParticles[i];
    }

    abstractParticles.clear();

    {
        size_t mt = ( param.maxTimeStep > param.maxServerTimeStep ) ? param.maxTimeStep : param.maxServerTimeStep;
        for ( size_t i = 0; i < mt + 1; ++i )
        {
            abstractParticles.push_back( server->getPointObjectFromLocal() );
        }
    }

    if ( param.CSmode == 1 )
    {
        abstractParticles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
        abstractParticles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
    }
    else
    {
        abstractParticles.front() = server->getPointObjectFromServer( param, result, numvol );
    }
//    std::cout<<"END Command::generateAbstractParticles"<<std::endl;

}

void Command::generateDetailedParticles()
{
//    std::cout<<" IN Command::generateDetailedParticles"<<std::endl;

    for ( size_t i = 0; i < detailedParticles.size(); ++i )
    {
        delete detailedParticles[i];
    }

    detailedParticles.clear();

    {
        size_t mt = ( param.maxTimeStep > param.maxServerTimeStep ) ? param.maxTimeStep : param.maxServerTimeStep;
        for ( size_t i = 0; i < mt + 1; ++i )
        {
            detailedParticles.push_back( server->getPointObjectFromLocal() );
        }
    }
//    std::cout<<"END Command::generateDetailedParticles"<<std::endl;

}

size_t Command::getUsingMemoryByKiloByte()
{
    size_t memory = 0;
#ifndef CPUMODE
    for ( size_t i = 0; i < abstractParticles.size(); ++i )
    {
        memory += abstractParticles[i]->coords().size() * sizeof( kvs::Real32 ) + abstractParticles[i]->normals().size() * sizeof( kvs::Real32 ) + abstractParticles[i]->colors().size() * sizeof( kvs::UInt8 );
    }
    for ( size_t i = 0; i < detailedParticles.size(); ++i )
    {
        memory += detailedParticles[i]->coords().size() * sizeof( kvs::Real32 ) + detailedParticles[i]->normals().size() * sizeof( kvs::Real32 ) + detailedParticles[i]->colors().size() * sizeof( kvs::UInt8 );
    }
#endif
    return memory / 1024;
}

void Command::reDraw()
{
//    screen3->redraw();
    screen->redraw();
}

bool Command::isSamplingParamsKeeped( const PBVRParam& param )
{
    bool res = lastSamplingParams;
    if ( res )
    {
        const SamplingParams* p = lastSamplingParams;
        res &= ( p->paramExTransFunc == param.paramExTransFunc );
        res &= ( p->inputdir == param.inputdir );
    }
    return res;
}

void Command::saveSamplingParams( const PBVRParam& param )
{
    delete lastSamplingParams;
    lastSamplingParams = NULL;
    lastSamplingParams = new SamplingParams( param );
}

void Command::lock()
{
//    mutex.lock();
}

void Command::unlock()
{
//    mutex.unlock();
}

void Command::closeServer()
{
    std::cout<<"Command_IS::closeServer()"<<std::endl;
    server->close( param );
}

void Command::apply_variable_range( const VariableRange& range )
{
    ParamExTransFunc& doc = extransfuncedit->doc();
    for ( size_t i = 0; i < doc.transferFunction.size(); i++ )
    {
        bool f = false;
        if ( !doc.range_initialized[i] )
        {
            const std::string& nm = doc.transferFunction[i].Name;
            const std::string tag_c = nm + "_var_c";
            const std::string tag_o = nm + "_var_o";
            doc.transferFunction[i].ColorVarMin   = range.min( tag_c );
            doc.transferFunction[i].ColorVarMax   = range.max( tag_c );
            doc.transferFunction[i].OpacityVarMin = range.min( tag_o );
            doc.transferFunction[i].OpacityVarMax = range.max( tag_o );
            doc.range_initialized[i] = true;
            f = true;
        }
        if ( f && extransfuncedit )
        {
            extransfuncedit->updateRangeEdit();
        }
    }
}

