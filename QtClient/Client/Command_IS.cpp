#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

//HASCHANGES MANY CHANGES
#include <QOpenGLContext>

//#include "ExtendedParticleVolumeRenderer.h"

#include <QThread>
#include <QFile>
#include <algorithm>
#include <string.h>
#include "ParticleTransferClient.h"
#include "ParticleMerger.h"
#include "ParticleServer.h"
#include "Command.h"

#include "TimerEvent.h"


#include <Panels/transferfunceditor_IS.h>
#include "Panels/particlepanel.h"
#include "Panels/coordinatepanel.h"
#include "Panels/timecontrolpanel.h"
#include "Panels/systemstatuspanel.h"
#include <Panels/filterinfopanel.h>
#include <Panels/renderoptionpanel.h>

#include "QGlue/renderarea.h"

#include <kvs/Directory>
#include <kvs/File>

#include "timer_simple.h"


#include "v3defines.h"

using namespace kvs::visclient;

// APPEND START fp)m.tanaka 2013.09.01
int subpixellevela = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
int subpixelleveld = PBVR_SUB_PIXEL_LEVEL_DETAILED;



char pfi_path_server[256];
char filter_parameter_filename[256];
static char pre_pfi_path_server[256];


#ifdef _TIMER_
#define TIMER_MAX_COUNT 9
static int timer_count = 0;
#endif

extern int repeatlevela;
extern int repeatleveld;
static int pretimestep  = -1;
static int numvol, numstep = 0;
static int exportMergedParticleStartStep = -1;

float Command::PBVRmaxcoords[3]={1.0,1.0,1.0};
float Command::PBVRmincoords[3]={0.0,0.0,0.0};

Command::Command( ParticleServer* server ) :
    m_is_under_animation( true ),
    m_is_under_communication( true ),
    m_is_shown( true ),
    m_is_import_transfer_function_parameter( false ),
    m_is_key_frame_animation( false ),
    m_server( server ),
    m_last_sampling_params( NULL ),
    m_particle_assign_flag( false ),
    m_server_side_subpixel_level(1)
{
    m_parameter.m_time_step_key_frame = -1;
    m_step_key_frame = 0;
    m_previous_key_frame  = 0;
};

Command::~Command()
{
    delete m_last_sampling_params;
}

void Command::startAbstractAnalysis()
{
    m_parameter.m_transfer_type = VisualizationParameter::Abstract;
//    controller->detail->setButtonColor( ::Gray );
//	param.timeStep = 1;
    m_parameter.m_time_step = 0;
// APPEND START fp)m.tanaka 2013.09.01
    m_parameter.m_detailed_repeat_level = subpixelleveld;
    m_parameter.m_abstract_repeat_level = subpixellevela;
    std::cout << "*** START ABSTRACTANALYSIS" << std::endl;
// APPEND END   fp)m.tanaka 2013.09.01

}

void Command::startDetailedAnalysis()
{
    m_parameter.m_transfer_type = VisualizationParameter::Detailed;
    /* removed 2014.07.21
        controller->abst->setButtonColor( ::Gray );
        generateDetailedParticles();
    */
    m_parameter.m_time_step = m_parameter.m_min_time_step;
    m_parameter.m_max_time_step = m_parameter.m_max_server_time_step;
    std::cout << "*** START DETAILEDANALYSIS" << std::endl;
    std::cout << "param.minTimeStep:" << m_parameter.m_min_time_step << std::endl;
    std::cout << "param.maxTimeStep:" << m_parameter.m_max_time_step << std::endl;
}

void Command::changeShowHide()
{

    m_is_shown = !m_is_shown;
    qWarning("Command::changeShowHide() depreceated, use TimecontrolPanel::changeShowHide instead");
    exit(1);

}

void Command::preUpdate()
{
#ifdef _TIMER_
    if ( timer_count == 0 ) TIMER_INIT();
    timer_count++;
#endif
    PBVR_TIMER_STA( 1 );

    // for Particle Merge
    m_parameter.m_particle_merge_param.m_circuit_time = m_parameter.m_time_step;
    m_particle_merge_ui->setParam( &m_parameter.m_particle_merge_param );
    for ( int i = 0; i < 11; i++ )
    {
        if ( m_parameter.m_particle_merge_param.m_particles[i].m_enable == true )
        {
            printf( "particles[%d] = true : \n", i );
        }
        else
        {
            printf( "particles[%d] = false : \n", i );
        }

        if ( m_parameter.m_particle_merge_param.m_particles[i].m_file_path.empty() != true )
        {
            printf( "filepath = %s\n", m_parameter.m_particle_merge_param.m_particles[i].m_file_path.c_str() );
        }
        else
        {
            printf( "filepath = Empty\n" );
        }
    }

    // for KeyFrameAnimation
    if ( m_is_key_frame_animation )
    {
        m_parameter.m_time_step = m_parameter.m_time_step_key_frame;
        if ( m_parameter.m_time_step < m_parameter.m_min_time_step )
        {
            m_parameter.m_time_step = m_parameter.m_min_time_step;
        }
        if ( m_parameter.m_time_step > m_parameter.m_max_time_step )
        {
            m_parameter.m_time_step = m_parameter.m_max_time_step;
        }
    }

    const size_t maxMemory = 10 * 1024 * 1024;
    if ( getUsingMemoryByKiloByte() > maxMemory )
    {
        kvsMessageError( "memory over: %d KB / %d KB.", getUsingMemoryByKiloByte(), maxMemory );
        kvsMessageError( "communication cancel." );
        m_is_under_communication = false;
    }

    if ( m_is_under_communication )
    {
        if ( m_parameter.m_transfer_type == VisualizationParameter::Abstract )
        {
            if ( m_parameter.m_detailed_transfer_type == VisualizationParameter::Divided )
            {
                if ( m_parameter.m_repeat_level == m_parameter.m_abstract_repeat_level )
                {
                    m_parameter.m_repeat_level = 1;
                }
            }
        }
        else if ( m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
            if ( m_parameter.m_detailed_transfer_type == VisualizationParameter::Divided )
            {
                if ( m_parameter.m_repeat_level == m_parameter.m_detailed_repeat_level )
                {
                    m_parameter.m_repeat_level = 1;
                }
            }
        }
    }

    // for export Particles
    if ( m_parameter.m_particle_merge_param.m_do_export )
    {
        printf( " ### [debug]: exportMergedParticleStartStep = %d, param.timeStep = %d\n", exportMergedParticleStartStep, m_parameter.m_time_step );
        if ( exportMergedParticleStartStep < 0 )
        {
            exportMergedParticleStartStep = m_parameter.m_time_step;
        }
        else
        {
            if ( exportMergedParticleStartStep == m_parameter.m_time_step )
            {
                m_parameter.m_particle_merge_param.m_do_export = false;
                exportMergedParticleStartStep = -1;
                m_particle_merge_ui->particleExportButtonActive();
            }
        }
    }
}
/**
 * @brief Command::update_doExport export parameters to as result of setparam
 * @param param
 */
void Command::update( VisualizationParameter* param, ReceivedMessage* result )
{
    // Viewer のスケール正規化用.
    qInfo(" *** Command::update starts *** %d",QThread::currentThreadId() );
    bool resetflag = false;
    bool server_resetflag = false;
    bool local_particle_exits = false;
    float local_crd[6], server_crd[6], crd[6];

    // 20230515 yodo disabled
    // this block is expected to export partcile data in stand alone mode, but not performed as expedted by bug. 
    // if ( param->m_client_server_mode == 0 && param->m_particle_merge_param.m_do_export )
    // {
    //     ParticleMerger merger;
    //     merger.setParam( param->m_particle_merge_param, param->m_min_server_time_step, param->m_max_server_time_step );
    //     size_t time_start = merger.getMergedInitialTimeStep();
    //     size_t time_end   = merger.getMergedFinalTimeStep();

    //     for ( size_t step = time_start; step <= time_end; step++ )
    //     {
    //         kvs::PointObject* p = NULL;
    //         kvs::PointObject* object;
    //         if ( object->nvertices() <= 1 )
    //         {
    //             object = NULL;
    //         }
    //         else
    //         {
    //             object = m_server_particles[step];
    //         }
    //         p = merger.doMerge( object, step, false );
    //         delete p;
    //     }
    //     param->m_particle_merge_param.m_do_export = false;
    // }

    result->isUnderAnimation = false;
    if ( m_is_under_communication )
    {
        if ( param->m_no_repsampling && isSamplingParamsKeeped( *param ) )
        {
            // When Transfer Functions are not edited.
            std::cout << "*** Transfer Functions are not edited." << std::endl;
        }
        else
        {
            // When Transfer Functions are edited.
            if ( param->m_transfer_type == VisualizationParameter::Detailed )
            {

                if ( param->m_detailed_transfer_type == VisualizationParameter::Divided )
                {
#ifndef CPUMODE
                    if ( param->m_repeat_level == param->m_detailed_repeat_level )
                    {
#endif
                        //delete detailedParticles[param.timeStep];
                        m_detailed_particles[m_detailed_particles.size()-1] = m_server->getPointObjectFromServer(*param, result, numvol, param->m_time_step ,PBVRmincoords,PBVRmaxcoords);
//                        m_detailed_particles[m_detailed_particles.size()-1] = m_server->getPointObjectFromServer(*param, result, numvol, param->m_time_step);
#ifndef CPUMODE
                    }
                    else if (  param->m_repeat_level < param->m_detailed_repeat_level )
                    {
                        PointObject* dividedObject = m_server->getPointObjectFromServer(*param, result, numvol, param->m_time_step ,PBVRmincoords,PBVRmaxcoords);
//                        PointObject* dividedObject = m_server->getPointObjectFromServer(*param, result, numvol, param->m_time_step);
                        m_detailed_particles[m_detailed_particles.size()-1]->add( *dividedObject );
                        delete dividedObject;
                    }
                    else
                    {
                        assert( false );
                    }
#endif
                }
                else if ( param->m_detailed_transfer_type == VisualizationParameter::Summalized ) //jupiter_branch
                {
                    kvs::PointObject* tmp_po = m_server->getPointObjectFromServer(*param, result, numvol, param->m_time_step ,PBVRmincoords,PBVRmaxcoords);
//                    kvs::PointObject* tmp_po = m_server->getPointObjectFromServer(*param, result, numvol, param->m_time_step);
                    if ( result->serverTimeStep > -1 ) param->m_time_step = result->serverTimeStep;
                    param->m_min_server_time_step = result->minServerTimeStep ;
                    param->m_max_server_time_step = result->maxServerTimeStep ;
                    this->m_parameter.m_min_server_time_step = param->m_min_server_time_step ;
                    this->m_parameter.m_max_server_time_step = param->m_max_server_time_step ;
                    //generateAbstractParticles( ( PBVRParam& )param, result );

                    this->m_result = *result;
                    generateDetailedParticles();
                    m_detailed_particles[m_detailed_particles.size()-1] = tmp_po;
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

            if ( m_server_particles.size() < param->m_time_step + 1 )
            {
                m_server_particles.resize( param->m_time_step + 1, NULL );
            }
//          delete serverParticles[param.timeStep];
            m_server_particles[param->m_time_step] = m_detailed_particles[m_detailed_particles.size()-1];
            m_particle_assign_flag = true;
            m_is_under_animation = result->isUnderAnimation;
        }
        PBVRmaxcoords[0] = result->maxObjectCoord[0];
        PBVRmaxcoords[1] = result->maxObjectCoord[1];
        PBVRmaxcoords[2] = result->maxObjectCoord[2];

        PBVRmincoords[0] = result->minObjectCoord[0];
        PBVRmincoords[1] = result->minObjectCoord[1];
        PBVRmincoords[2] = result->minObjectCoord[2];

        // Merge Point Object
        ParticleMerger merger;
        kvs::PointObject* server_object = m_server_particles[param->m_time_step];
        kvs::PointObject* object;
        merger.setParam( param->m_particle_merge_param, param->m_min_server_time_step, param->m_max_server_time_step );
        object = merger.doMerge( server_object, param->m_time_step ,true);
#ifdef GPU_MODE
        m_is_polygon_checkbox_current[0] = merger.isPolygonEnable(6);
        m_is_polygon_checkbox_current[1] = merger.isPolygonEnable(7);
        m_is_polygon_checkbox_current[2] = merger.isPolygonEnable(8);
        m_is_polygon_checkbox_current[3] = merger.isPolygonEnable(9);
        m_is_polygon_checkbox_current[4] = merger.isPolygonEnable(10);
        if(     m_is_polygon_checkbox_current[0] != m_is_polygon_checkbox_before[0]||
                m_is_polygon_checkbox_current[1] != m_is_polygon_checkbox_before[1]||
                m_is_polygon_checkbox_current[2] != m_is_polygon_checkbox_before[2]||
                m_is_polygon_checkbox_current[3] != m_is_polygon_checkbox_before[3]||
                m_is_polygon_checkbox_current[4] != m_is_polygon_checkbox_before[4])
        {
            for(int i = 6; i < 11; i++)
            {
                extCommand->deletePolygonModel(i);
                m_is_polygon_checkbox_before[i - 6] = m_is_polygon_checkbox_current[i - 6];
            }
        }

        for(int i = 6; i < 11;i++)
        {
            if(merger.isPolygonEnable(i) == true)
            {
                size_t filestep = param->m_time_step;
                if(merger.getLocalObjectIsEnableKeepFinal(i) == false && merger.getLocalObjectIsEnableKeepInitial(i) == false){
                    if(merger.getLocalObjectFinalStep(i) < filestep){
                    }else if(merger.getLocalObjectInitialStep(i) <= filestep){
                    }
                }

                if(merger.getLocalObjectIsEnableKeepFinal(i) == true && merger.getLocalObjectIsEnableKeepInitial(i) == false){
                    if(merger.getLocalObjectFinalStep(i) < filestep){
                        filestep = merger.getLocalObjectFinalStep(i);
                    }else if(merger.getLocalObjectInitialStep(i) <= filestep){
                    }
                }

                if(merger.getLocalObjectIsEnableKeepFinal(i) == false && merger.getLocalObjectIsEnableKeepInitial(i) == true){
                    if(merger.getLocalObjectInitialStep(i) > filestep){
                        filestep = merger.getLocalObjectInitialStep(i);
                    }else if(merger.getLocalObjectFinalStep(i) >= filestep){
                    }
                }

                if(merger.getLocalObjectIsEnableKeepFinal(i) == true && merger.getLocalObjectIsEnableKeepInitial(i) == true){
                    if(merger.getLocalObjectFinalStep(i) < filestep){
                        filestep = merger.getLocalObjectFinalStep(i);
                    }else if(merger.getLocalObjectInitialStep(i) <= filestep){
                    }else if(merger.getLocalObjectInitialStep(i) > filestep){
                        filestep = merger.getLocalObjectInitialStep(i);
                    }else{
                    }
                }

                std::stringstream polygon_file_tmp;
                std::stringstream polygon_file_tmp_stl;
                polygon_file_tmp << merger.getPolygonFilePath(i) << '_' << std::setw(5) << std::setfill( '0' ) << filestep << ".kvsml";
                std::string polygon_file = polygon_file_tmp.str();
                std::cout << __LINE__ << " : " << polygon_file << std::endl;
                QFile file(QString::fromStdString(polygon_file));
                bool isSTL = false;

                if(file.exists() == false)
                {
                    std::cout << __LINE__ << std::endl;
//                    std::cout << merger.getPolygonFilePath(i) << std::endl;
//                    polygon_file_tmp << merger.getPolygonFilePath(i) << '_' << std::setw(5) << std::setfill( '0' ) << filestep << ".stl";
                    polygon_file_tmp_stl << merger.getPolygonFilePath(i) << '_' << std::setw(5) << std::setfill( '0' ) << filestep << ".stl";
                    polygon_file = polygon_file_tmp_stl.str();
                    std::cout << __LINE__ << " : " << polygon_file_tmp_stl.str() << std::endl;
                    std::cout << __LINE__ << " : " << polygon_file << std::endl;
                    file.setFileName(QString::fromStdString(polygon_file));
                    std::cout << polygon_file << std::endl;
                    isSTL = true;
                }

                std::stringstream polygon_file_empty_tmp;
                if(isSTL == false)
                {
                    std::cout << "it is kvsml" << std::endl;
                    polygon_file_empty_tmp << merger.getPolygonFilePath(i) << '_' << std::setw(5) << std::setfill( '0' ) << merger.getLocalObjectInitialStep(i) << ".kvsml";
                }else
                {
                    std::cout << "it is stl" << std::endl;
                    polygon_file_empty_tmp << merger.getPolygonFilePath(i) << '_' << std::setw(5) << std::setfill( '0' ) << merger.getLocalObjectInitialStep(i) << ".stl";
                }

                std::string polygon_file_empty = polygon_file_empty_tmp.str();

                std::cout << __LINE__ << " : " << polygon_file << std::endl;
                if(file.exists() == true)
                {
                    extCommand->registerPolygonModel(polygon_file,
                                                     i,
                                                     merger.getPolygon_opacity(i),
                                                     merger.getPolygonColor(i),
                                                     isSTL);
                }
                else
                {
                    extCommand->registerEmptyPolygonModel(polygon_file_empty,
                                                        i,
                                                        merger.getPolygon_opacity(i),
                                                        merger.getPolygonColor(i));
                }


            }else{
                extCommand->deletePolygonModel(i);
            }
        }
#endif

        extCommand->m_screen->update();

        result->m_min_merged_time_step = merger.getMergedInitialTimeStep();
        result->m_max_merged_time_step = merger.getMergedFinalTimeStep();
        m_detailed_particles[m_detailed_particles.size()-1] = object;

        // set last TransferFunction
        if ( param->m_time_step == result->m_max_merged_time_step )
        {
            saveSamplingParams( *param );
        }

        // Viewerの正規化用処理
        local_particle_exits = merger.calculateExternalCoords( local_crd );
        //サーバで計算されたsubpixel levelの更新.
        if ( param->m_client_server_mode == 1 )
        {
            m_local_subpixel_level = result->m_subpixel_level;
        }
        else
        {
            m_local_subpixel_level = object->size( 0 );
        }






        TimecontrolPanel::requestUpdate(param, result);



        // reallocate abstractParticles and detailedParticles
        if ( param->m_max_time_step > m_detailed_particles.size() - 1 )
        {
//            generateAbstractParticles( param, result );
            this->m_parameter = *param;
            this->m_result = *result;
            generateDetailedParticles();
        }

        this->m_parameter.m_time_step    = param->m_time_step;
        this->m_parameter.m_max_time_step = param->m_max_time_step;
        this->m_parameter.m_min_time_step = param->m_min_time_step;
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
    if ( m_particle_merge_ui->checkChangeFlag() || m_coord_panel_ui->getResetViewFlag() )
    {
        // 粒子統合の表示,非表示フラグが変化した
        if ( param->m_client_server_mode != 1 )
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
                crd[0] = std::min( local_crd[0], PBVRmincoords[0] );
                crd[1] = std::min( local_crd[1], PBVRmincoords[1] );
                crd[2] = std::min( local_crd[2], PBVRmincoords[2] );
                crd[3] = std::max( local_crd[3], PBVRmaxcoords[0] );
                crd[4] = std::max( local_crd[4], PBVRmaxcoords[1] );
                crd[5] = std::max( local_crd[5], PBVRmaxcoords[2] );
                resetflag = true;
            }
            else
            {
                // localの統合粒子が１つも選択されていない状態なのでサーバの粒子情報で設定
                crd[0] = PBVRmincoords[0];
                crd[1] = PBVRmincoords[1];
                crd[2] = PBVRmincoords[2];
                crd[3] = PBVRmaxcoords[0];
                crd[4] = PBVRmaxcoords[1];
                crd[5] = PBVRmaxcoords[2];
                resetflag = true;
            }
            if ( m_coord_panel_ui->getResetViewFlag() )
            {
                // 正規化フラグをfalseに戻す
                m_coord_panel_ui->resetResetViewFlag();
                if(keeper == true){
                    keeper = false;
                    resetflag = false;
                }
            }
        }
        m_particle_merge_ui->particleResetChangedFlag();
    }
    else if ( server_resetflag )
    {
        // PFIファイルが切り替わった
        if ( local_particle_exits )
        {
            // サーバの粒子情報とlocalの粒子情報を比較して範囲が広い方を採用
            crd[0] = std::min( local_crd[0], PBVRmincoords[0] );
            crd[1] = std::min( local_crd[1], PBVRmincoords[1] );
            crd[2] = std::min( local_crd[2], PBVRmincoords[2] );
            crd[3] = std::max( local_crd[3], PBVRmaxcoords[0] );
            crd[4] = std::max( local_crd[4], PBVRmaxcoords[1] );
            crd[5] = std::max( local_crd[5], PBVRmaxcoords[2] );
            resetflag = true;
        }
        else
        {
            // サーバの粒子情報で設定
            crd[0] = PBVRmincoords[0];
            crd[1] = PBVRmincoords[1];
            crd[2] = PBVRmincoords[2];
            crd[3] = PBVRmaxcoords[0];
            crd[4] = PBVRmaxcoords[1];
            crd[5] = PBVRmaxcoords[2];
            resetflag = true;
        }
    }
    else
    {
        crd[0] = PBVRmincoords[0];
        crd[1] = PBVRmincoords[1];
        crd[2] = PBVRmincoords[2];
        crd[3] = PBVRmaxcoords[0];
        crd[4] = PBVRmaxcoords[1];
        crd[5] = PBVRmaxcoords[2];
        resetflag = true;
    }
    // change view
    if ( resetflag )
    {
        keeper = true;
        qInfo(" *** Command::update::9 starts *** %d",QThread::currentThreadId() );
        ((RenderArea*)m_screen)->setCoordinateBoundaries(crd);
    }
//    if(keeper == true){
//        if(param->m_client_server_mode != 0){
//        m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxObjectCoords(kvs::Vector3f(m_server_coord_min[1][0],m_server_coord_min[1][1],m_server_coord_min[1][2]),kvs::Vector3f(m_server_coord_max[1][0],m_server_coord_max[1][1],m_server_coord_max[1][2]));
//        m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxExternalCoords(kvs::Vector3f(m_server_coord_min[1][0],m_server_coord_min[1][1],m_server_coord_min[1][2]),kvs::Vector3f(m_server_coord_max[1][0],m_server_coord_max[1][1],m_server_coord_max[1][2]));
//        }
//        if(local_particle_exits == true){
//            m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxObjectCoords(kvs::Vector3f(local_crd[0],local_crd[1],local_crd[2]),kvs::Vector3f(local_crd[3],local_crd[4],local_crd[5]));
//            m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxExternalCoords(kvs::Vector3f(local_crd[0],local_crd[1],local_crd[2]),kvs::Vector3f(local_crd[3],local_crd[4],local_crd[5]));
//        }

//    }

    FilterinfoPanel::updateFilterInfo();
    qInfo(" *** Command::update ends *** %d\n",QThread::currentThreadId() );
}

void Command::postUpdate()
{
    if ( m_parameter.m_client_server_mode == 0 )
    {
        m_particle_merge_ui->particleExportButtonActive();
    }

    if ( m_is_under_communication )
    {
        if ( m_parameter.m_transfer_type == VisualizationParameter::Abstract )
        {
            if ( m_parameter.m_detailed_transfer_type == VisualizationParameter::Divided )
            {
                if ( m_parameter.m_repeat_level < m_parameter.m_abstract_repeat_level )
                {
                    m_parameter.m_repeat_level++;
                }
            }
            else if ( m_parameter.m_detailed_transfer_type == VisualizationParameter::Summalized )
            {
                m_parameter.m_repeat_level = m_parameter.m_abstract_repeat_level;
            }
        }
        else if ( m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
            if ( m_parameter.m_detailed_transfer_type == VisualizationParameter::Divided )
            {
                if ( m_parameter.m_repeat_level < m_parameter.m_detailed_repeat_level )
                {
                    m_parameter.m_repeat_level++;
                }
            }
            else if ( m_parameter.m_detailed_transfer_type == VisualizationParameter::Summalized )
            {
                m_parameter.m_repeat_level = m_parameter.m_detailed_repeat_level;
            }
        }
    }
    //    ADD BY)T.Osaki 2020.03.16
    /*if(m_server_subpixel_level != m_local_subpixel_level){
        m_parameter.m_detailed_subpixel_level = m_server_subpixel_level;
    }else{
        m_parameter.m_detailed_subpixel_level = m_local_subpixel_level;
    }
    if(m_parameter.m_client_server_mode != 0){
        m_screen->setRenderSubPixelLevel(m_parameter.m_detailed_subpixel_level);
    }*/
    m_parameter.m_detailed_subpixel_level =  m_local_subpixel_level;
    //    m_screen->recreateRenderImageBuffer(m_parameter.m_detailed_subpixel_level);
#ifdef CPUMODE
    m_screen->setRenderSubPixelLevel(m_parameter.m_detailed_subpixel_level );
    m_screen->recreateRenderImageBuffer( );
#endif
//  Profiler::get()->start( "rendering time" );
    PBVR_TIMER_STA( 150 );


//#ifndef CPUMODE
//    if ( m_parameter.m_transfer_type == VisualizationParameter::Abstract)
//    {
//#ifndef CPUMODE
//        m_screen->setRenderRepetionlLevel(m_parameter.m_abstract_repeat_level);
//#endif
//        m_screen->setRenderSubPixelLevel( m_parameter.m_abstract_subpixel_level );
//        PointObject* object = m_abstract_particles[m_parameter.m_time_step];
//        m_screen->attachPointObject( object );
//    }
    if ( m_parameter.m_transfer_type == VisualizationParameter::Detailed )
    {
//#ifndef CPUMODE
//        m_screen->setRenderRepetionlLevel( m_parameter.m_detailed_repeat_level );
//#endif
//        m_screen->setRenderSubPixelLevel( m_parameter.m_detailed_subpixel_level );
//        PointObject* object = m_detailed_particles[m_parameter.m_time_step];
//        m_screen->attachPointObject( object );
//    }
//    else
//    {
//        assert( false );
//    }
//#endif

#if 0  // FEAST modify 20160128
    if ( pretimestep != param.timeStep )
    {
        ScreenShot( screen, param.timeStep );

        pretimestep = param.timeStep;
    }
#else
    if ( pretimestep != TimecontrolPanel::g_curStep )
    {
        RenderArea::ScreenShot( m_screen->scene(), TimecontrolPanel::g_curStep );

        pretimestep = TimecontrolPanel::g_curStep;
    }
#endif  // FEAST modify 20160128 end

    PBVR_TIMER_END( 150 );
//  Profiler::get()->end( "rendering time" );
    if ( m_is_under_animation )
    {
//      Profiler::get()->writeToTerminal();
#if !(defined(_DEBUG) && defined(WIN32))
//      Profiler::get()->writeToCSV( param.timeStep, "profile.csv" );
#endif
    }
    if ( m_is_import_transfer_function_parameter == false )
    {
        apply_variable_range( m_result.m_var_range );
    }
    extransfuncedit->updateRangeView();

    PBVR_TIMER_END( 1 );
#ifdef _TIMER_
    if ( timer_count == TIMER_MAX_COUNT ) TIMER_FIN();
#endif

}
}

void Command::getServerParticleInfomation( VisualizationParameter* param, ReceivedMessage* result)
{
//    std::cout <<"IN Command::getServerParticleInfomation"<<std::endl;
    kvs::Vector3f minObjectCoord;
    kvs::Vector3f maxObjectCoord;
    if (param->m_client_server_mode == 1)
    {
        if (param->m_max_server_time_step == 0)
        {
            jpv::ParticleTransferClient client( param->m_hostname, param->m_port );
            jpv::ParticleTransferClientMessage message;
            jpv::ParticleTransferServerMessage reply;

            client.initClient();



            reply.camera = new kvs::Camera();

            // Send inputDir
            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
            message.m_initialize_parameter = -3;
            // message.inputDir = param.inputdir;
            message.m_input_directory = std::string( pfi_path_server );
            strcpy( pre_pfi_path_server, pfi_path_server );
            // transfer function
            param->m_parameter_extend_transfer_function.applyToClientMessage( &message );

            message.m_message_size = message.byteSize();
            client.sendMessage( message );

            // Recive server condition
            client.recvMessage( reply );
            numvol = reply.m_number_volume_divide;
            param->m_min_server_time_step = reply.m_start_step;
            param->m_max_server_time_step = reply.m_end_step;
            param->m_min_time_step = reply.m_start_step;
            param->m_max_time_step = reply.m_end_step;
            param->m_time_step = reply.m_time_step;

            minObjectCoord = kvs::Vector3f( reply.m_min_object_coord );
            maxObjectCoord = kvs::Vector3f( reply.m_max_object_coord );
            PBVRmaxcoords[0] = reply.m_max_object_coord[0];
            PBVRmaxcoords[1] = reply.m_max_object_coord[1];
            PBVRmaxcoords[2] = reply.m_max_object_coord[2];

            PBVRmincoords[0] = reply.m_min_object_coord[0];
            PBVRmincoords[1] = reply.m_min_object_coord[1];
            PBVRmincoords[2] = reply.m_min_object_coord[2];

            RenderoptionPanel::plimitlevel      = reply.particle_limit;
            RenderoptionPanel::pdensitylevel    = reply.particle_density;
            particle_data_size_limit_input = reply.particle_data_size_limit;
            RenderoptionPanel::resolution_width_level = reply.camera->windowWidth();
            RenderoptionPanel::resolution_height_level= reply.camera->windowHeight();

            float minValue = reply.m_min_value;
            float maxValue = reply.m_max_value;
            std::cout << "minValue: " << minValue
                << " maxValue: " << maxValue << std::endl;
            std::cout << "minObjectCoord: " << minObjectCoord << std::endl;
            std::cout << "maxObjectCoord: " << maxObjectCoord << std::endl;

            param->m_parameter_extend_transfer_function.importFromServerMessage( reply );
            if (m_is_import_transfer_function_parameter == false)
            {
                apply_variable_range( reply.m_variable_range );
            }


            /* delete for 140521 server */
            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
            message.m_initialize_parameter = -1;
            message.m_message_size = message.byteSize();
            client.sendMessage( message );
            client.recvMessage( reply );
            client.termClient();
            delete reply.camera;
        }
    }
//    std::cout<<" END Command::getServerParticleInfomation"<<std::endl;

}

//void Command::initializeAbstractParticles( VisualizationParameter* param, ReceivedMessage* result, int localminstep, int localmaxstep )
//{
//    // stepの範囲を示すparamのメンバー変数に設定を行う
//    param->m_min_time_step = localminstep;
//    param->m_max_time_step = localmaxstep;

//    std::cout << "Command::generateAbstractParticles numvol " << numvol << std::endl;
//    std::cout << "Command::generateAbstractParticles numstep " << param->m_max_server_time_step << std::endl;
//    std::cout << "Command::generateAbstractParticles minstep " << param->m_min_time_step << std::endl;
//    std::cout << "Command::generateAbstractParticles maxstep " << param->m_max_time_step << std::endl;

//    for (size_t i = 0; i < m_abstract_particles.size(); ++i)
//    {
//        delete m_abstract_particles[i];
//    }

//    m_abstract_particles.clear();

//    {
//        size_t mt = (param->m_max_time_step > param->m_max_server_time_step) ? param->m_max_time_step : param->m_max_server_time_step;
//        for (size_t i = 0; i < mt + 1; ++i)
//        {
//            m_abstract_particles.push_back( m_server->getPointObjectFromLocal() );
//        }
//    }

//    // サーバの情報が無い場合のViewer正規化処理
//    // Viewer正規化処理をする
//    if (param->m_client_server_mode == 1)
//    {
//        kvs::Vector3f minObjectCoord;
//        kvs::Vector3f maxObjectCoord;
//        minObjectCoord.set( PBVRmincoords );
//        maxObjectCoord.set( PBVRmaxcoords );

//        m_abstract_particles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
//        m_abstract_particles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
//    }else
//    {
//        m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step );
//    }
//}


//void Command::generateAbstractParticles( VisualizationParameter* param, ReceivedMessage* result)
//{
////   std::cout<<" IN Command::generateAbstractParticles"<<std::endl;
//    kvs::Vector3f minObjectCoord;
//    kvs::Vector3f maxObjectCoord;
//    if ( param->m_client_server_mode == 1 )
//    {
//        if ( param->m_max_server_time_step == 0 )
//        {
//            jpv::ParticleTransferClient client( param->m_hostname, param->m_port );
//            jpv::ParticleTransferClientMessage message;
//            jpv::ParticleTransferServerMessage reply;

//            reply.camera = new kvs::Camera();

//            client.initClient();

//            // Send inputDir
//            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
//            message.m_initialize_parameter = -3;
//            // message.inputDir = param.inputdir;
//            message.m_input_directory = std::string( pfi_path_server );
//            strcpy( pre_pfi_path_server, pfi_path_server );
//            // transfer function
//            param->m_parameter_extend_transfer_function.applyToClientMessage( &message );

//            message.m_message_size = message.byteSize();
//            client.sendMessage( message );

//            // Recive server condition
//            client.recvMessage( reply );
//            numvol = reply.m_number_volume_divide;
//            param->m_min_server_time_step = reply.m_start_step;
//            param->m_max_server_time_step = reply.m_end_step;
//            param->m_min_time_step = reply.m_start_step;
//            param->m_max_time_step = reply.m_end_step;
//            param->m_time_step = reply.m_time_step;

//// APPEND START fp)m.takizawa 2014.05.21

//            minObjectCoord = kvs::Vector3f( reply.m_min_object_coord );
//            maxObjectCoord = kvs::Vector3f( reply.m_max_object_coord );
//            PBVRmaxcoords[0] = reply.m_max_object_coord[0];
//            PBVRmaxcoords[1] = reply.m_max_object_coord[1];
//            PBVRmaxcoords[2] = reply.m_max_object_coord[2];

//            PBVRmincoords[0] = reply.m_min_object_coord[0];
//            PBVRmincoords[1] = reply.m_min_object_coord[1];
//            PBVRmincoords[2] = reply.m_min_object_coord[2];

//// APPEND END fp)m.takizawa 2014.05.21

//            float minValue = reply.m_min_value;
//            float maxValue = reply.m_max_value;
//            std::cout << "minValue: " << minValue
//                      << " maxValue: " << maxValue << std::endl;
//            std::cout << "minObjectCoord: " << minObjectCoord << std::endl;
//            std::cout << "maxObjectCoord: " << maxObjectCoord << std::endl;

//            if ( m_is_import_transfer_function_parameter == false )
//            {
//                apply_variable_range( reply.m_variable_range );
//            }

//            /* delete for 140521 server */
//            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
//            message.m_initialize_parameter = -1;
//            message.m_message_size = message.byteSize();
//            client.sendMessage( message );
//            client.recvMessage( reply );
//            client.termClient();
//            delete reply.camera;
//        }

//    }

//    /*
//    if ( localminstep > -1 && localmaxstep > -1 )
//    {
//        // stepの範囲を示すparamのメンバー変数に設定を行う
//        param.minTimeStep = localminstep;
//        param.maxTimeStep = localmaxstep;
//    }
//    */
//    std::cout << "Command::generateAbstractParticles numvol " << numvol << std::endl;
//    std::cout << "Command::generateAbstractParticles numstep " << param->m_max_server_time_step << std::endl;
//    std::cout << "Command::generateAbstractParticles minstep " << param->m_min_time_step << std::endl;
//    std::cout << "Command::generateAbstractParticles maxstep " << param->m_max_time_step << std::endl;

//    for ( size_t i = 0; i < m_abstract_particles.size(); ++i )
//    {
//        delete m_abstract_particles[i];
//    }

//    m_abstract_particles.clear();

//    {
//        size_t mt = ( param->m_max_time_step > param->m_max_server_time_step ) ? param->m_max_time_step : param->m_max_server_time_step;
//        for ( size_t i = 0; i < mt + 1; ++i )
//        {
//            m_abstract_particles.push_back( m_server->getPointObjectFromLocal() );
//        }
//    }

//    if ( param->m_client_server_mode == 1 )
//    {
//        m_abstract_particles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
//        m_abstract_particles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
//    }
//    else
//    {
//        m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step );
//    }
////    std::cout<<"END Command::generateAbstractParticles"<<std::endl;

//}

void Command::generateDetailedParticles()
{
//    std::cout<<" IN Command::generateDetailedParticles"<<std::endl;

    for ( size_t i = 0; i < m_detailed_particles.size(); ++i )
    {
        delete m_detailed_particles[i];
    }

    m_detailed_particles.clear();

    {
        size_t mt = ( m_parameter.m_max_time_step > m_parameter.m_max_server_time_step ) ? m_parameter.m_max_time_step : m_parameter.m_max_server_time_step;
        for ( size_t i = 0; i < mt + 1; ++i )
        {
            m_detailed_particles.push_back( m_server->getPointObjectFromLocal() );
        }
    }
//    std::cout<<"END Command::generateDetailedParticles"<<std::endl;

}

size_t Command::getUsingMemoryByKiloByte()
{
    size_t memory = 0;
#ifndef CPUMODE
//    for ( size_t i = 0; i < m_abstract_particles.size(); ++i )
//    {
//        memory += m_abstract_particles[i]->coords().size() * sizeof( kvs::Real32 ) + m_abstract_particles[i]->normals().size() * sizeof( kvs::Real32 ) + m_abstract_particles[i]->colors().size() * sizeof( kvs::UInt8 );
//    }
    for ( size_t i = 0; i < m_detailed_particles.size(); ++i )
    {
        memory += m_detailed_particles[i]->coords().size() * sizeof( kvs::Real32 ) + m_detailed_particles[i]->normals().size() * sizeof( kvs::Real32 ) + m_detailed_particles[i]->colors().size() * sizeof( kvs::UInt8 );
    }
#endif
    return memory / 1024;
}

void Command::reDraw()
{
//    screen3->redraw();
    m_screen->redraw();
}

bool Command::isSamplingParamsKeeped( const VisualizationParameter& param )
{
    bool res = m_last_sampling_params;
    if ( res )
    {
        const SamplingParams* p = m_last_sampling_params;
        res &= ( p->m_parameter_extend_transfer_function == param.m_parameter_extend_transfer_function );
        res &= ( p->m_input_directory == param.m_input_directory );
    }
    return res;
}

void Command::saveSamplingParams( const VisualizationParameter& param )
{
    delete m_last_sampling_params;
    m_last_sampling_params = NULL;
    m_last_sampling_params = new SamplingParams( param );
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
    m_server->close( m_parameter );
}

void Command::apply_variable_range( const VariableRange& range )
{
    ExtendedTransferFunctionMessage& doc = extransfuncedit->doc();

    bool f = false;

    for (size_t i = 0; i < doc.m_color_transfer_function.size(); i++)
    {
        if (!doc.m_color_transfer_function[i].m_range_initialized)
        {
            //const std::string& nm = doc.m_color_transfer_function[i].Name;
            std::stringstream ss;
            ss << "t" << (i + 1);
            const std::string tag_c = ss.str() + "_var_c";
            doc.m_color_transfer_function[i].m_color_variable_min = range.min(tag_c);
            doc.m_color_transfer_function[i].m_color_variable_max = range.max(tag_c);
            doc.m_color_transfer_function[i].m_range_initialized = true;
            f = true;
        }
    }

    for (size_t i = 0; i < doc.m_opacity_transfer_function.size(); i++)
    {
        if (!doc.m_opacity_transfer_function[i].m_range_initialized)
        {
            //const std::string& nm = doc.m_opacity_transfer_function[i].Name;
            std::stringstream ss;
            ss << "t" << (i + 1);
            const std::string tag_o = ss.str() + "_var_o";
            doc.m_opacity_transfer_function[i].m_opacity_variable_min = range.min(tag_o);
            doc.m_opacity_transfer_function[i].m_opacity_variable_max = range.max(tag_o);
            doc.m_opacity_transfer_function[i].m_range_initialized = true;
            f = true;
        }
    }

    if (f && extransfuncedit)
    {
        extransfuncedit->updateRangeEdit();
    }

    return;

}

