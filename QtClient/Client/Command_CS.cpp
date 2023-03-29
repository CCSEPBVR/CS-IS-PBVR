//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <kvs/OpenGL>
#include <QOpenGLContext>

#include "ExtendedParticleVolumeRenderer.h"

#include <QThread>
#include <QFile>
#include <algorithm>
#include <string.h>

#include "ParticleTransferClient.h"
#include "ParticleMerger.h"
#include "ParticleServer.h"
#include "Command.h"

#include "TimerEvent.h"

#include <Panels/transferfunceditor_CS.h>
#include "Panels/particlepanel.h"
#include "Panels/coordinatepanel.h"
#include "Panels/timecontrolpanel.h"
#include "Panels/systemstatuspanel.h"
#include <Panels/filterinfopanel.h>

#include <QGlue/renderarea.h>

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

#ifdef _TIMER_
#define TIMER_MAX_COUNT 9
//static int timer_count = 0;
#endif

// add:end by @hira at 2016/12/01
int start_store_step = -1;
float Command::PBVRmaxcoords[3]={0.0,0.0,0.0};
float Command::PBVRmincoords[3]={0.0,0.0,0.0};

// Kawamura debug write
int nstart = 0;

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
    qInfo("*** START ABSTRACT ANALYSIS");
    m_parameter.m_transfer_type = VisualizationParameter::Abstract;

    m_parameter.m_time_step = 0;

    m_parameter.m_detailed_repeat_level = subpixelleveld;
    m_parameter.m_abstract_repeat_level = subpixellevela;

    std::cout << "param.m_min_time_step:" << m_parameter.m_min_time_step << std::endl;
    std::cout << "param.m_max_time_step:" << m_parameter.m_max_time_step << std::endl;

}

void Command::startDetailedAnalysis()
{
    qInfo("*** START DETAILED ANALYSIS");
    m_parameter.m_transfer_type = VisualizationParameter::Detailed;
    /* removed 2014.07.21
        m_controller->abst->setButtonColor( ::Gray );
        generateDetailedParticles();
    */
    m_parameter.m_time_step = m_parameter.m_min_time_step;
    m_parameter.m_max_time_step = m_parameter.m_max_server_time_step;

    std::cout << "param.m_min_time_step:" << m_parameter.m_min_time_step << std::endl;
    std::cout << "param.m_max_time_step:" << m_parameter.m_max_time_step << std::endl;
}

void Command::changeShowHide()
{
    qWarning("Command::changeShowHide() depreceated, use TimecontrolPanel::changeShowHide instead");
    exit(1);
}

void Command::preUpdate()
{
#ifdef _TIMER_
    if ( timer_count == 0 ) PBVR_TIMER_INIT();
    timer_count++;
#endif
    PBVR_TIMER_STA( 1 );

    // for Particle Merge
    m_parameter.m_particle_merge_param.m_circuit_time = m_parameter.m_time_step;
    m_particle_merge_ui->setParam( &m_parameter.m_particle_merge_param );
    // for Coordinate Synthesis
    m_coord_panel_ui->setParam( &m_parameter );

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
        printf( " ### [debug]: exportMergedParticleStartStep = %d, param.m_time_step = %d\n", exportMergedParticleStartStep, m_parameter.m_time_step );
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
    float local_crd[6], crd[6];

    if ( param->m_client_server_mode == 0 && param->m_particle_merge_param.m_do_export )
    {
        ParticleMerger merger;
        merger.setParam( param->m_particle_merge_param, param->m_min_server_time_step, param->m_max_server_time_step );
        size_t time_start = merger.getMergedInitialTimeStep();
        size_t time_end   = merger.getMergedFinalTimeStep();

        for ( size_t step = time_start; step <= time_end; step++ )
        {
            kvs::PointObject* p = NULL;
            kvs::PointObject* object;
            //KVS2.7.0
            //MOD BY)T.Osaki 2020.06.17
            //if ( m_server_particles[step]->nvertices() <= 1 )
            if( object->numberOfVertices() <= 1 )
            {
                object = NULL;
            }
            else
            {
                object = m_server_particles[step];
            }
            p = merger.doMerge( object, step, false );
            delete p;
        }
        param->m_particle_merge_param.m_do_export = false;
    }

    if ( m_is_under_communication )
    {
        qInfo(" *** Command::update::2 starts *** %d",QThread::currentThreadId() );
        if ( param->m_no_repsampling && start_store_step == -2 && m_server_coord_flag[param->m_time_step] )
        {
            std::cout << "*** Display particle data on memory." << std::endl;
        }
        else
        {
            // 保存始めのステップ番号がMin Time より小さい場合はMin Timeを保存始めのステップとする.
            // If the step number at the start of saving is smaller than Min Time, set Min Time as the beginning step of saving.
            if ( start_store_step < static_cast<int>( m_timectrl_panel->minValue() ) )
                start_store_step = static_cast<int>( m_timectrl_panel->minValue() );

            if ( start_store_step == param->m_time_step )
            {   // Since it is the timing to store the particles of all the steps locally, set -2
                start_store_step = -2; // 全ステップの粒子をローカルに保存するタイミングなので-2をセット

            }
            // When Transfer Functions are edited.
            if ( param->m_transfer_type == VisualizationParameter::Abstract )
            {

                if ( param->m_detailed_transfer_type == VisualizationParameter::Divided )
                {
                    std::cout << "*** param->m_detailed_transfer_type == PBVRParam::Divided" << std::endl;
                        // サーバにないステップは対象としない 2018.12.19 start

                        int stepno;
                        if ((stepno = getServerStep(param)) >= 0){
                            delete m_abstract_particles[param->m_time_step];

                            //std::cerr << "getPointObjectFromServer() calling for abstract particle @1" << std::endl;
                            qInfo("getPointObjectFromServer() calling for abstract particle @1");

#ifdef USE_SERVER_SIDE_PBVR_COORDS
                            m_abstract_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno, PBVRmincoords, PBVRmaxcoords);
#else // USE_SERVER_SIDE_PBVR_COORDS
                            m_abstract_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
#endif // USE_SERVER_SIDE_PBVR_COORDS

//                            std::cerr << "getPointObjectFromServer() called for abstract particle @1" << std::endl;
                            qInfo("getPointObjectFromServer() called for abstract particle @1");

                            // ADD START FEAST 2016.01.07
                            // 粒子生成時にサーバー側で演算エラーが発生

                            if (m_abstract_particles[param->m_time_step] == NULL)
                            {
                                //                                m_timectrl_panel->toggleStop(true);
                                m_timectrl_panel->toggleActive();
                            }
                            // ADD END FEAST 2016.01.07
                        } // 2018.12.19
                        else if (param->m_client_server_mode == 0  ){
                            if (m_abstract_particles[param->m_time_step]) delete m_abstract_particles[param->m_time_step];
                        }
                }
                else if ( param->m_detailed_transfer_type == VisualizationParameter::Summalized )
                {
                    // サーバにないステップは対象としない 2018.12.19 start

                    int stepno;
                    if ((stepno = getServerStep(param)) >= 0 ){
                        delete m_abstract_particles[param->m_time_step];

                        //std::cerr << "getPointObjectFromServer() calling for abstract particle @2" << std::endl;
                        qInfo("getPointObjectFromServer() calling for abstract particle @2");

#ifdef USE_SERVER_SIDE_PBVR_COORDS
                        m_abstract_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno, PBVRmincoords, PBVRmaxcoords);
#else // USE_SERVER_SIDE_PBVR_COORDS
                        m_abstract_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
#endif // USE_SERVER_SIDE_PBVR_COORDS

                        //std::cerr << "getPointObjectFromServer() called for abstract particle @2" << std::endl;
                        qInfo("getPointObjectFromServer() called for abstract particle @2" );

                        // ADD START FEAST 2016.01.07

                        if (m_abstract_particles[param->m_time_step] == NULL)
                        {
                            //                            m_timectrl_panel->toggleStop(true);

                            m_timectrl_panel->toggleActive();
                        }
                        // ADD END FEAST 2016.01.07

                    } // 2018.12.19

                    else if (param->m_client_server_mode == 0){
                        if (m_abstract_particles[param->m_time_step]) delete m_abstract_particles[param->m_time_step];
                    }
                }
                else
                {
                    assert( false );
                }
            }
            else if ( param->m_transfer_type == VisualizationParameter::Detailed )
            {//Run detailed analysis
                if ( param->m_detailed_transfer_type == VisualizationParameter::Divided )
                {
                    qInfo(" *** Command::update::2::abstract::Divided starts *** %d",QThread::currentThreadId() );
                        // サーバにないステップは対象としない 2018.12.19 start

                    int stepno;
                    if ((stepno = getServerStep(param)) >= 0 ){
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        delete m_detailed_particles[m_detailed_particles.size()-1];
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        delete m_detailed_particles[param->m_time_step];
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS

                        //std::cerr << "getPointObjectFromServer() calling for detailed particle @3" << std::endl;
                        qInfo("getPointObjectFromServer() calling for detailed particle @3");

#ifdef USE_SERVER_SIDE_PBVR_COORDS
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            m_detailed_particles[m_detailed_particles.size()-1] = m_server->getPointObjectFromServer(*param, result, numvol, stepno,PBVRmincoords,PBVRmaxcoords);
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            m_detailed_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno, PBVRmincoords, PBVRmaxcoords);
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
#else // USE_SERVER_SIDE_PBVR_COORDS
                            m_detailed_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
#endif // USE_SERVER_SIDE_PBVR_COORDS

                            //std::cerr << "getPointObjectFromServer() called for detailed particle @3" << std::endl;
                            qInfo( "getPointObjectFromServer() called for detailed particle @3");

                            // ADD START FEAST 2016.01.07
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            if (m_detailed_particles[m_detailed_particles.size()-1] == NULL)
#else //  FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            if (m_detailed_particles[param->m_time_step] == NULL)
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            {
                                //                                m_timectrl_panel->toggleStop(true);
                                m_timectrl_panel->toggleActive();
                            }
                            // ADD END FEAST 2016.01.07
                        } // 2018.12.19 end
                        else if (param->m_client_server_mode == 0  ){
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                          if (m_detailed_particles[m_detailed_particles.size()-1]) delete m_detailed_particles[m_detailed_particles.size()-1];
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            if (m_detailed_particles[param->m_time_step]) delete m_detailed_particles[param->m_time_step];
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        }
                }
                else if ( param->m_detailed_transfer_type == VisualizationParameter::Summalized )
                {
                    // サーバにないステップは対象としない 2018.12.19 start

                    int stepno;
                    if ((stepno = getServerStep(param)) >= 0){
                        if (m_detailed_particles.size() > param->m_time_step){
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            if (m_detailed_particles[m_detailed_particles.size()-1])
                                delete m_detailed_particles[m_detailed_particles.size()-1];
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            if (m_detailed_particles[param->m_time_step])
                                delete m_detailed_particles[param->m_time_step];
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS

                            //std::cerr << "getPointObjectFromServer() calling for detailed particle @4" << std::endl;
                            qInfo("getPointObjectFromServer() calling for detailed particle @4");

#ifdef USE_SERVER_SIDE_PBVR_COORDS
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            m_detailed_particles[m_detailed_particles.size()-1] = m_server->getPointObjectFromServer(*param, result, numvol, stepno,PBVRmincoords,PBVRmaxcoords);

//                            std::cerr << "Command::update(): hasObjCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->hasMinMaxObjectCoords() << std::endl;
//                            std::cerr << "Command::update(): objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " ... " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//                            std::cerr << "Command::update(): extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " ... " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;

#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            m_detailed_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno, PBVRmincoords, PBVRmaxcoords);
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
#else // USE_SERVER_SIDE_PBVR_COORDS
                            m_detailed_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
#endif // USE_SERVER_SIDE_PBVR_COORDS

                            //std::cerr << "getPointObjectFromServer() called for detailed particle @4" << std::endl;
                            qInfo("getPointObjectFromServer() called for detailed particle @4");

                        }
                        else{

//                            std::cerr << "getPointObjectFromServer() calling for abstract particle @5" << std::endl;
                            qInfo("getPointObjectFromServer() calling for abstract particle @5");;

#ifdef USE_SERVER_SIDE_PBVR_COORDS
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            m_detailed_particles[m_detailed_particles.size()-1] = m_server->getPointObjectFromServer(*param, result, numvol, stepno,PBVRmincoords,PBVRmaxcoords);
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                            m_detailed_particles.push_back(m_server->getPointObjectFromServer(*param, result, numvol, stepno, PBVRmincoords, PBVRmaxcoords));
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
#else // USE_SERVER_SIDE_PBVR_COORDS
                            m_detailed_particles.push_back(m_server->getPointObjectFromServer(*param, result, numvol, stepno));
#endif // USE_SERVER_SIDE_PBVR_COORDS

                           // std::cerr << "getPointObjectFromServer() called for abstract particle @5" << std::endl;
                            qInfo("getPointObjectFromServer() called for abstract particle @5");

                        }
                        if (m_server_particles.size() < param->m_time_step + 1){
                            m_server_particles.resize(param->m_time_step + 1, NULL);
                            // サーバー生成粒子の各stepの存在範囲を格納

                            m_server_coord_min.resize(param->m_time_step + 1);
                            m_server_coord_max.resize(param->m_time_step + 1);
                            m_server_coord_flag.resize(param->m_time_step + 1, false);
                        }
                        if (m_server_particles[param->m_time_step])
                            delete m_server_particles[param->m_time_step];
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        m_server_particles[param->m_time_step] = m_detailed_particles[m_detailed_particles.size()-1];
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        m_server_particles[param->m_time_step] = m_detailed_particles[param->m_time_step];
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS

                        m_coord_panel_ui->resetChangeFlagForStoreParticle();


                        // ADD START FEAST 2016.01.07
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        if (m_detailed_particles[m_detailed_particles.size()-1] == NULL)
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        if (m_detailed_particles[param->m_time_step] == NULL)
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        {
                            //                            m_timectrl_panel->toggleStop(true);
                            m_timectrl_panel->toggleActive();
                        }
                        // ADD END FEAST 2016.01.07
                    }// 2018.12.19 end
                    else if (param->m_client_server_mode == 0){
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        if (m_detailed_particles[m_detailed_particles.size()-1]) delete m_detailed_particles[m_detailed_particles.size()-1];
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                        if (m_detailed_particles[param->m_time_step]) delete m_detailed_particles[param->m_time_step];
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
                    }
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
        }
        // サーバから送られてくる粒子について保存している存在範囲情報を消去
        //Clear saved existence range information about particles sent from server

        if ( param->m_client_server_mode == 1 && m_coord_panel_ui->getChangeCoordFlag() )
        {
            for ( int i = 0; i < m_server_coord_flag.size(); i++ )
            {
                m_server_coord_flag[i] = false;
            }
        }
        // サーバにないステップは対象としない 2018.12.19 start

        int stepno;
        if ((stepno = getServerStep(param)) >= 0){
            // サーバ側の粒子の存在範囲を保存

            if (param->m_client_server_mode == 1 && !m_server_coord_flag[param->m_time_step] && m_server_particles[param->m_time_step] != NULL)
            {
                // add by @hira at 2016/12/01
                if (m_server_particles[param->m_time_step]->coords().pointer() != NULL) {

//                    std::cerr << "Commad::update(): before call updateMinMaxCoords() : resetviewflag=" << m_coord_panel_ui->getResetViewFlag() << std::endl;
//                    std::cerr << "  objCrds= " << m_server_particles[param->m_time_step]->minObjectCoord() << " ... " << m_server_particles[param->m_time_step]->maxObjectCoord() << std::endl;
//                    std::cerr << "  extCrds= " << m_server_particles[param->m_time_step]->minExternalCoord() << " ... " << m_server_particles[param->m_time_step]->maxExternalCoord() << std::endl;


                    if(m_coord_panel_ui->getResetViewFlag()){

                        m_server_particles[param->m_time_step]->updateMinMaxCoords();
                    }

//                    std::cerr << "Commad::update(): after call updateMinMaxCoords() : " << std::endl;
//                    std::cerr << "  objCrds= " << m_server_particles[param->m_time_step]->minObjectCoord() << " ... " << m_server_particles[param->m_time_step]->maxObjectCoord() << std::endl;
//                    std::cerr << "  extCrds= " << m_server_particles[param->m_time_step]->minExternalCoord() << " ... " << m_server_particles[param->m_time_step]->maxExternalCoord() << std::endl;


                    m_server_coord_min[param->m_time_step] = m_server_particles[param->m_time_step]->minObjectCoord();
                    m_server_coord_max[param->m_time_step] = m_server_particles[param->m_time_step]->maxObjectCoord();
                    m_server_coord_flag[param->m_time_step] = true;
                }
            }
        }
        // Merge Point Object
        ParticleMerger merger;

        // サーバにないステップは対象としない 2018.12.19 start

        kvs::PointObject* server_object = NULL;
        if (stepno >= 0)
            server_object = m_server_particles[param->m_time_step];
        kvs::PointObject* object;
        merger.setParam( param->m_particle_merge_param, param->m_min_server_time_step, param->m_max_server_time_step );
#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
        if(param->m_client_server_mode != 0){

//            {
//                std::cerr << "Command::update() : before doMerge(true) :" << std::endl;
//                std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//                std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//                std::cerr << "  object->objCrds= " << object->minObjectCoord() << " .., " << object->maxObjectCoord() << std::endl;
//                std::cerr << "  object->extCrds= " << object->minExternalCoord() << " .., " << object->maxExternalCoord() << std::endl;
//            }


            object = merger.doMerge( server_object, param->m_time_step,true );
            if(server_object != nullptr){
                if(server_object->size() != m_server_side_subpixel_level){
                    m_server_side_subpixel_level = server_object->size();
                    object->setSize(m_server_side_subpixel_level);
                }
            }else{
                object->setSize(m_server_side_subpixel_level);
            }

        }else{
            object = merger.doMerge( server_object, param->m_time_step,false );
        }
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
        object = merger.doMerge( *server_object, param->m_time_step );
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS

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

        extCommand->m_screen->update();

        result->m_min_merged_time_step = merger.getMergedInitialTimeStep();
        result->m_max_merged_time_step = merger.getMergedFinalTimeStep();
        // サーバにないステップは対象としない 2018.12.19 start

//        {
//            std::cerr << "Command::update() : before let object to m_detaile_particles :" << std::endl;
//            std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//            std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//            std::cerr << "  object->objCrds= " << object->minObjectCoord() << " .., " << object->maxObjectCoord() << std::endl;
//            std::cerr << "  object->extCrds= " << object->minExternalCoord() << " .., " << object->maxExternalCoord() << std::endl;
//        }

#ifdef FIX_DETAILED_PARATICLES_HANDLING_AS_CS
        m_detailed_particles[m_detailed_particles.size()-1] = object;
#else // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
        if (m_detailed_particles.size() > param->m_time_step){
            m_detailed_particles[param->m_time_step] = object;
        }
        else {
            m_detailed_particles.push_back(object);
        }
#endif // FIX_DETAILED_PARATICLES_HANDLING_AS_CS
        // Change TransferFunction
        if ( param->m_client_server_mode == 1 && ( !isSamplingParamsKeeped( *param ) || m_coord_panel_ui->getChangeCoordFlagForStoreParticle() ) )
        {
            // 保存始めたstep番号の保存と、可視化パラメータの保存

            saveSamplingParams( *param );
            start_store_step = param->m_time_step;
            m_coord_panel_ui->resetChangeFlagForStoreParticle();
        }

        // Viewerの正規化用処理
        // Processing for viewer normalization

        local_particle_exits = merger.calculateExternalCoords( local_crd );
        // サーバで計算されたsubpixel levelの更新.
        // Update subpixel level calculated by server.

        if ( param->m_client_server_mode == 1 )
        {
            if ( m_server_particles.size() <= param->m_time_step || m_server_particles[param->m_time_step] == NULL )
            {
                m_local_subpixel_level = object->size( 0 );
            }
            else
            {
                //    ADD BY)T.Osaki 2020.03.16
                m_local_subpixel_level = result->m_subpixel_level;
                m_server_subpixel_level = result->m_subpixel_level;
            }
        }
        else
        {
            m_local_subpixel_level = object->size( 0 );
        }

//        {
//            std::cerr << "Command::update() : before request update timestep :" << std::endl;
//            std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//            std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//        }


        size_t m_time_step = std::max( param->m_min_time_step, std::min( param->m_max_time_step, param->m_time_step ) );
        param->m_time_step = m_time_step;
        TimecontrolPanel::requestUpdate(param,result);

        // 2018.12.19 不要と思われる
#if 0
        // reallocate m_abstract_particles and m_detailed_particles
        if ( param->m_max_time_step > m_detailed_particles.size() - 1 )
        {
            generateAbstractParticles( ( VisualizationParameter* )param, result );
            this->m_parameter = *param;
            this->m_result = *result;
            generateDetailedParticles();
        }
#endif
        this->m_parameter.m_max_time_step = param->m_max_time_step;
        this->m_parameter.m_min_time_step = param->m_min_time_step;

//        {
//            std::cerr << "Command::update() : before update system status :" << std::endl;
//            std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//            std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//        }


        // 粒子数の表示
        char buff[512];
        SystemstatusPanel::updateSystemStatus( object->nvertices());
        m_particle_assign_flag = true;
    }
#ifdef _DEBUG
    if (param->m_hasfin) {
        printf(" [debug] current filter parameter file. %s & %s\n", pre_filter_parameter_filename, filter_parameter_filename);
    }
    else {
        printf(" [debug] current pfi file. %s & %s\n", pre_pfi_path_server, pfi_path_server);
    }
#endif

    bool is_change = false;
    if ( param->m_client_server_mode == 1) {
        qInfo(" *** Command::update::3 starts *** %d",QThread::currentThreadId() );
        if (param->m_hasfin) {
            if (strcmp( pre_filter_parameter_filename, filter_parameter_filename ) != 0) is_change = true;
        }
        else {
            if (strcmp( pre_pfi_path_server, pfi_path_server ) != 0) is_change = true;
        }
    }

//    {
//        std::cerr << "Command::update() : before check is_change :is_change = " <<  is_change << std::endl;
//        std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//        std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//    }


    if (is_change )
    {
        qInfo(" *** Command::update::4 starts *** %d",QThread::currentThreadId() );
#ifdef _DEBUG
        if (param->m_hasfin) {
            printf(" [debug] change filter parameter file. %s & %s\n", pre_filter_parameter_filename, filter_parameter_filename);
        }
        else {
            printf(" [debug] change pfi file. %s -> %s\n", pre_pfi_path_server, pfi_path_server);
        }
#endif
        qInfo(" *** Command::update::5 starts *** %d",QThread::currentThreadId() );
        numstep = 0;
        param->m_max_server_time_step = 0;
        // m_is_import_transfer_function_parameter = false;
        generateAbstractParticles ( ( VisualizationParameter* )param, result );
        this->m_parameter = *param;
        this->m_result = *result;
        generateDetailedParticles();
        qInfo(" *** Command::update::6 starts *** %d",QThread::currentThreadId() );
        TimecontrolPanel::requestUpdate(param,result);
        // For change view (Reset scale)
        server_resetflag = true;
    }

//    {
//        std::cerr << "Command::update() : before check change|reserview flag :ChangeFlag = " <<  m_particle_merge_ui->checkChangeFlag() << ", reserview= " << m_coord_panel_ui->getResetViewFlag() << std::endl;
//        std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//        std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//    }


    // For change view (Reset scale)
    if ( m_particle_merge_ui->checkChangeFlag() || m_coord_panel_ui->getResetViewFlag() )
    {
        qInfo(" *** Command::update::7a starts *** %d",QThread::currentThreadId() );
        // 粒子統合の表示,非表示フラグが変化した
        // サーバ表示粒子範囲による正規化処理のボタンが押された
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
            qInfo(" *** Command::update::7b starts *** %d",QThread::currentThreadId() );
            // クラサバモードでlocalの統合粒子も指定されているならば
            if ( local_particle_exits )
            {
                resetflag = false;
                if ( m_coord_panel_ui->getResetViewFlag() )
                {
                    // サーバの粒子情報とlocalの粒子情報を比較して範囲が広い方を採用
                    for ( int i = 0; i < m_server_coord_flag.size(); i++ )
                    {
                        if ( m_server_coord_flag[i] ) // サーバの粒子存在情報があるなら
                        {
                            // サーバの存在情報とlocalの粒子情報を比較して範囲が広い方を採用
                            crd[0] = std::min( local_crd[0], m_server_coord_min[i][0] );
                            crd[1] = std::min( local_crd[1], m_server_coord_min[i][1] );
                            crd[2] = std::min( local_crd[2], m_server_coord_min[i][2] );
                            crd[3] = std::max( local_crd[3], m_server_coord_max[i][0] );
                            crd[4] = std::max( local_crd[4], m_server_coord_max[i][1] );
                            crd[5] = std::max( local_crd[5], m_server_coord_max[i][2] );
                            resetflag = true;
                        }
                    }
                }

                // サーバの粒子存在情報無かったか、正規化ボタンが押されずにここに来た場合
                if ( resetflag == false )
                {

                    {
                        // 20220703 yodo@meshman.jp append for debug
                        float *min_crd = PBVRmincoords;
                        float *max_crd = PBVRmaxcoords;
                        fprintf (stderr, "DEBUG: Command::update(): change or resertview & csmode=1 & local_exists & resetflag=true\n");
                        fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
                        fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
                        fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
                        fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
                    }


                    // サーバの粒子情報とlocalの粒子情報を比較して範囲が広い方を採用
                    crd[0] = std::min( local_crd[0], PBVRmincoords[0] );
                    crd[1] = std::min( local_crd[1], PBVRmincoords[1] );
                    crd[2] = std::min( local_crd[2], PBVRmincoords[2] );
                    crd[3] = std::max( local_crd[3], PBVRmaxcoords[0] );
                    crd[4] = std::max( local_crd[4], PBVRmaxcoords[1] );
                    crd[5] = std::max( local_crd[5], PBVRmaxcoords[2] );
                    resetflag = true;
                }
            }
            else
            {
                // localの統合粒子が１つも選択されていない状態なのでサーバの粒子情報で設定
                resetflag = false;
                if ( m_coord_panel_ui->getResetViewFlag() )
                {
                    for ( int i = 0; i < m_server_coord_flag.size(); i++ )
                    {
                        if ( m_server_coord_flag[i] ) // サーバの粒子存在情報があるなら
                        {
                            if ( resetflag )
                            {
                                crd[0] = std::min( crd[0], m_server_coord_min[i][0] );
                                crd[1] = std::min( crd[1], m_server_coord_min[i][1] );
                                crd[2] = std::min( crd[2], m_server_coord_min[i][2] );
                                crd[3] = std::max( crd[3], m_server_coord_max[i][0] );
                                crd[4] = std::max( crd[4], m_server_coord_max[i][1] );
                                crd[5] = std::max( crd[5], m_server_coord_max[i][2] );
                            }
                            else
                            {
                                crd[0] = m_server_coord_min[i][0];
                                crd[1] = m_server_coord_min[i][1];
                                crd[2] = m_server_coord_min[i][2];
                                crd[3] = m_server_coord_max[i][0];
                                crd[4] = m_server_coord_max[i][1];
                                crd[5] = m_server_coord_max[i][2];
                                resetflag = true;
                            }
                        }
                    }
                }

                // サーバの粒子存在情報無かったか、正規化ボタンが押されずにここに来た場合
                if ( resetflag == false )
                {

                    {
                        // 20220703 yodo@meshman.jp append for debug
                        float *min_crd = PBVRmincoords;
                        float *max_crd = PBVRmaxcoords;
                        fprintf (stderr, "DEBUG: Command::update(): change or resertview & csmode!=1 & resetflag=false\n");
                        fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
                        fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
                        //fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
                        //fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
                    }

                    crd[0] = PBVRmincoords[0];
                    crd[1] = PBVRmincoords[1];
                    crd[2] = PBVRmincoords[2];
                    crd[3] = PBVRmaxcoords[0];
                    crd[4] = PBVRmaxcoords[1];
                    crd[5] = PBVRmaxcoords[2];
                    resetflag = true;
                }
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
        qInfo(" *** Command::update::8 starts *** %d",QThread::currentThreadId() );
        // PFIファイルが切り替わった
        if ( local_particle_exits )
        {
            {
                // 20220703 yodo@meshman.jp append for debug
                float *min_crd = PBVRmincoords;
                float *max_crd = PBVRmaxcoords;
                fprintf (stderr, "DEBUG: Command::update(): server_restflag & local_exists\n");
                fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
                fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
                fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
                fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
            }

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
            {
                // 20220703 yodo@meshman.jp append for debug
                float *min_crd = PBVRmincoords;
                float *max_crd = PBVRmaxcoords;
                fprintf (stderr, "DEBUG: Command::update(): server_restflag & local_exists\n");
                fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
                fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
                //fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
                //fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
            }

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

//    {
//        std::cerr << "Command::update() : before check resetflag: resetflag= " << resetflag << std::endl;
//        std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//        std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//    }


    // change view
    if ( resetflag )
    {
        keeper = true;

        qInfo(" *** Command::update::9 starts *** %d",QThread::currentThreadId() );
        ((RenderArea*)m_screen)->setCoordinateBoundaries(crd);
    }

//    {
//        std::cerr << "Command::update() : before check keeper: keeper= " << keeper << std::endl;
//        std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//        std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//    }


    // 20230113 yodo imported from CS-PBVR 1.17.3
    if(keeper == true){
        if(param->m_client_server_mode != 0){
        m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxObjectCoords(kvs::Vector3f(m_server_coord_min[1][0],m_server_coord_min[1][1],m_server_coord_min[1][2]),kvs::Vector3f(m_server_coord_max[1][0],m_server_coord_max[1][1],m_server_coord_max[1][2]));
        m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxExternalCoords(kvs::Vector3f(m_server_coord_min[1][0],m_server_coord_min[1][1],m_server_coord_min[1][2]),kvs::Vector3f(m_server_coord_max[1][0],m_server_coord_max[1][1],m_server_coord_max[1][2]));
        }
        if(local_particle_exits == true){
            m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxObjectCoords(kvs::Vector3f(local_crd[0],local_crd[1],local_crd[2]),kvs::Vector3f(local_crd[3],local_crd[4],local_crd[5]));
            m_detailed_particles[m_detailed_particles.size()-1]->setMinMaxExternalCoords(kvs::Vector3f(local_crd[0],local_crd[1],local_crd[2]),kvs::Vector3f(local_crd[3],local_crd[4],local_crd[5]));
        }

    }

//    {
//        std::cerr << "Command::update() : last " << std::endl;
//        std::cerr << "  objCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minObjectCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxObjectCoord() << std::endl;
//        std::cerr << "  extCrds= " << m_detailed_particles[m_detailed_particles.size()-1]->minExternalCoord() << " .., " << m_detailed_particles[m_detailed_particles.size()-1]->maxExternalCoord() << std::endl;
//    }


    qInfo(" *** Command::update ends *** %d\n",QThread::currentThreadId() );
}

void Command::postUpdate()
{
    qInfo(" ### Command::postUpdate() starts ### %d",QThread::currentThreadId() );
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
    if(m_server_subpixel_level != m_local_subpixel_level){
        m_parameter.m_detailed_subpixel_level = m_server_subpixel_level;
    }else{
        m_parameter.m_detailed_subpixel_level = m_local_subpixel_level;
    }

    m_screen->setRenderSubPixelLevel(m_parameter.m_detailed_subpixel_level);
//    m_screen->recreateRenderImageBuffer();

    PBVR_TIMER_STA( 150 );


    if ( pretimestep != TimecontrolPanel::g_curStep )
    {
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        RenderArea::ScreenShot( m_screen->scene(), TimecontrolPanel::g_curStep );

        pretimestep = TimecontrolPanel::g_curStep;
    }

    PBVR_TIMER_END( 150 );
    if ( m_is_under_animation )
    {
#if !(defined(_DEBUG) && defined(WIN32))
#endif
    }
    if ( m_is_import_transfer_function_parameter == false )
    {
        apply_variable_range( m_result.m_var_range );
    }
    extransfuncedit->updateRangeView();

    PBVR_TIMER_END( 1 );
#ifdef _TIMER_
    if ( timer_count == TIMER_MAX_COUNT ) PBVR_TIMER_FIN();
#endif
    qInfo(" ### Command::postUpdate() ends ###");

    //        extCommand->param->paramExTransFunc.ExTransFuncParaDump();
}

int Command::getServerParticleInfomation( VisualizationParameter* param, ReceivedMessage* result )
{
    kvs::Vector3f minObjectCoord;
    kvs::Vector3f maxObjectCoord;
    if ( param->m_client_server_mode == 1 )
    {
        if ( param->m_max_server_time_step == 0 )
        {
            jpv::ParticleTransferClient client( param->m_hostname, param->m_port );
            jpv::ParticleTransferClientMessage message;
            jpv::ParticleTransferServerMessage reply;

            // modify by @hira at 2016/12/01 : 接続エラー発生の場合、終了する
            int init = client.initClient();
            if (init < 0) {
                return init;
            }

            // Send inputDir
            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
            message.m_initialize_parameter = -3;
            // message.m_input_directory = param->m_input_directory;
            message.m_input_directory = std::string( pfi_path_server );
            strcpy( pre_pfi_path_server, pfi_path_server );
            // add by @hira at 2016/12/01
            message.m_filter_parameter_filename = std::string( filter_parameter_filename );
            strcpy( pre_filter_parameter_filename, filter_parameter_filename );

            // transfer function
            param->m_parameter_extend_transfer_function.applyToClientMessage( &message );

            message.m_message_size = message.byteSize();
            client.sendMessage( message );

            // Recive server condition
            client.recvMessage( &reply );
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

//            {
//                // 20220703 yodo@meshman.jp append for debug
//                float *min_crd = PBVRmincoords;
//                float *max_crd = PBVRmaxcoords;
//                fprintf (stderr, "DEBUG: Command::getServerParticleInformation():\n");
//                fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
//                fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
//                //fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
//                //fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
//            }


            float minValue = reply.m_min_value;
            float maxValue = reply.m_max_value;
            std::cout << "minValue: " << minValue
                      << " maxValue: " << maxValue << std::endl;
            std::cout << "minObjectCoord: " << minObjectCoord << std::endl;
            std::cout << "maxObjectCoord: " << maxObjectCoord << std::endl;

            if ( m_is_import_transfer_function_parameter == false )
            {
                apply_variable_range( reply.m_variable_range );
            }

            m_number_nodes = reply.m_number_nodes;
            m_number_elements = reply.m_number_elements;
            m_element_type = reply.m_element_type;
            m_file_type = reply.m_file_type;
            m_number_ingredients = reply.m_number_ingredients;
            m_number_step = reply.m_number_step;
            m_number_volume_divide = reply.m_number_volume_divide;

            m_min_object_coord[0]=reply.m_min_object_coord[0];
            m_min_object_coord[1]=reply.m_min_object_coord[1];
            m_min_object_coord[2]=reply.m_min_object_coord[2];

            m_max_object_coord[0]= reply.m_max_object_coord[0];
            m_max_object_coord[1]= reply.m_max_object_coord[1];
            m_max_object_coord[2]= reply.m_max_object_coord[2];

            /* delete for 140521 server */
            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
            message.m_initialize_parameter = -1;
            message.m_message_size = message.byteSize();
            client.sendMessage( message );
            client.recvMessage( &reply );
            client.termClient();
            return 1; // added by noda@JAEA, 2017/8/8
        }
    }
    return 0;
}

void Command::initializeAbstractParticles( VisualizationParameter* param, ReceivedMessage* result, const int localminstep, const int localmaxstep )
{
    // stepの範囲を示すparamのメンバー変数に設定を行う
    param->m_min_time_step = localminstep;
    param->m_max_time_step = localmaxstep;

    std::cout << "Command::initializeAbstractParticles numvol " << numvol << std::endl;
    std::cout << "Command::initializeAbstractParticles numstep " << param->m_max_server_time_step << std::endl;
    std::cout << "Command::initializeAbstractParticles minstep " << param->m_min_time_step << std::endl;
    std::cout << "Command::initializeAbstractParticles maxstep " << param->m_max_time_step << std::endl;

    for ( size_t i = 0; i < m_abstract_particles.size(); ++i )
    {
        delete m_abstract_particles[i];
    }

    m_abstract_particles.clear();

    {
        size_t mt = ( param->m_max_time_step > param->m_max_server_time_step ) ? param->m_max_time_step : param->m_max_server_time_step;
        for ( size_t i = 0; i < mt + 1; ++i )
        {
            m_abstract_particles.push_back( m_server->getPointObjectFromLocal() );
        }
    }

    // サーバの情報が無い場合のViewer正規化処理
    // Viewer正規化処理をする
    if ( param->m_client_server_mode == 1 )
    {
        kvs::Vector3f minObjectCoord;
        kvs::Vector3f maxObjectCoord;
        minObjectCoord.set( PBVRmincoords );
        maxObjectCoord.set( PBVRmaxcoords );

        {
            // 20220703 yodo@meshman.jp append for debug
            float *min_crd = PBVRmincoords;
            float *max_crd = PBVRmaxcoords;
            fprintf (stderr, "DEBUG: Command::initializeAbstractParticles(): csmode=1 : PBVR->objcrd for abstract particles\n");
            fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
            fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
            //fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
            //fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
        }


        m_abstract_particles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
        m_abstract_particles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
    }
    else
    {
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.06.17
        //m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step );
        kvs::Vector3f minObjectCoord;
        kvs::Vector3f maxObjectCoord;
        std::cout << PBVRmincoords << std::endl;
        std::cout << PBVRmaxcoords << std::endl;
        minObjectCoord.set(PBVRmincoords);
        maxObjectCoord.set(PBVRmaxcoords);

//        {
//            // 20220703 yodo@meshman.jp append for debug
//            float *min_crd = PBVRmincoords;
//            float *max_crd = PBVRmaxcoords;
//            fprintf (stderr, "DEBUG: Command::initializeAbstractParticles(): csmode!=1 : PBVR->objcrd for abstract particles (?)\n");
//            fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
//            fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
//            //fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
//            //fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
//        }
    }
}


void Command::generateAbstractParticles ( VisualizationParameter* param, ReceivedMessage* result )
{
    kvs::Vector3f minObjectCoord;
    kvs::Vector3f maxObjectCoord;
    if ( param->m_client_server_mode == 1 )
    {
        if ( param->m_max_server_time_step == 0 )
        {
            jpv::ParticleTransferClient client( param->m_hostname, param->m_port );
            jpv::ParticleTransferClientMessage message;
            jpv::ParticleTransferServerMessage reply;

            client.initClient();

            // Send inputDir
            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
            message.m_initialize_parameter = -3;
            // message.m_input_directory = param->m_input_directory;
            message.m_input_directory = std::string( pfi_path_server );
            strcpy( pre_pfi_path_server, pfi_path_server );
            // add by @hira at 2016/12/01
            message.m_filter_parameter_filename = std::string( "filter_parameter_filename" );
            strcpy( pre_filter_parameter_filename, filter_parameter_filename );

            // transfer function
            param->m_parameter_extend_transfer_function.applyToClientMessage( &message );

            message.m_message_size = message.byteSize();
            client.sendMessage( message );

            // Recive server condition
            client.recvMessage( &reply );
            numvol = reply.m_number_volume_divide;
            param->m_min_server_time_step = reply.m_start_step;
            param->m_max_server_time_step = reply.m_end_step;
            param->m_min_time_step = reply.m_start_step;
            param->m_max_time_step = reply.m_end_step;
            param->m_time_step = reply.m_time_step;

            // APPEND START fp)m.takizawa 2014.05.21

            minObjectCoord = kvs::Vector3f( reply.m_min_object_coord );
            maxObjectCoord = kvs::Vector3f( reply.m_max_object_coord );
            PBVRmaxcoords[0] = reply.m_max_object_coord[0];
            PBVRmaxcoords[1] = reply.m_max_object_coord[1];
            PBVRmaxcoords[2] = reply.m_max_object_coord[2];

            PBVRmincoords[0] = reply.m_min_object_coord[0];
            PBVRmincoords[1] = reply.m_min_object_coord[1];
            PBVRmincoords[2] = reply.m_min_object_coord[2];

            {
                // 20220703 yodo@meshman.jp append for debug
                float *min_crd = PBVRmincoords;
                float *max_crd = PBVRmaxcoords;
                fprintf (stderr, "DEBUG: Command::generateAbstractParticles(): csmode=1 & max_step=0\n");
                fprintf (stderr, "DEBUG:   PBVRmincoords: (% 16f, % 16f, % 16f)\n", min_crd[0], min_crd[1], min_crd[2]);
                fprintf (stderr, "DEBUG:   PBVRmaxcoords: (% 16f, % 16f, % 16f)\n", max_crd[0], max_crd[1], max_crd[2]);
                //fprintf (stderr, "DEBUG:  local_crd(min): (% 16f, % 16f, % 16f)\n", local_crd[0], local_crd[1], local_crd[2]);
                //fprintf (stderr, "DEBUG:  local_crd(max): (% 16f, % 16f, % 16f)\n", local_crd[3], local_crd[4], local_crd[5]);
            }


            // APPEND END fp)m.takizawa 2014.05.21

            float minValue = reply.m_min_value;
            float maxValue = reply.m_max_value;
            std::cout << "minValue: " << minValue
                      << " maxValue: " << maxValue << std::endl;
            std::cout << "minObjectCoord: " << minObjectCoord << std::endl;
            std::cout << "maxObjectCoord: " << maxObjectCoord << std::endl;

            if ( m_is_import_transfer_function_parameter == false )
            {
                apply_variable_range( reply.m_variable_range );
            }

            // APPEND START fp)k.yajima 2015.06.26
            //            char buff[512];
            //            sprintf( buff, "All node: %d", reply.m_number_nodes );
            //            lbl_numNodes->name = buff;
            //            sprintf( buff, "All element: %d", reply.m_number_elements );
            //            lbl_numElements->name = buff;
            //            // 	sprintf(buff, "Element type: %c", reply.m_element_type);
            //            sprintf( buff, "Element type: %d", reply.m_element_type ); // MODIFY fj 2015.03.15
            //            lbl_elemType->name = buff;
            //            // 	sprintf(buff, "File type: %c", reply.m_file_type);
            //            sprintf( buff, "File type: %d", reply.m_file_type ); // MODIFY fj 2015.03.15
            //            lbl_fileType->name = buff;
            //            sprintf( buff, "Vector num: %d", reply.m_number_ingredients );
            //            lbl_numIngredients->name = buff;
            //            sprintf( buff, "Step num: %d", reply.m_number_step );
            //            lbl_numStep->name = buff;
            //            sprintf( buff, "SUB volume: %d", reply.numVolDiv );
            //            lbl_numVolDiv->name = buff;
            //            sprintf( buff, " X-Axis: min=%f, max=%f", reply.m_min_object_coord[0], reply.m_max_object_coord[0] );
            //            lbl_objectCoord0->name = buff;
            //            sprintf( buff, " Y-Axis: min=%f, max=%f", reply.m_min_object_coord[1], reply.m_max_object_coord[1] );
            //            lbl_objectCoord1->name = buff;
            //            sprintf( buff, " Z-Axis: min=%f, max=%f", reply.m_min_object_coord[2], reply.m_max_object_coord[2] );
            //            lbl_objectCoord2->name = buff;

            FilterinfoPanel::updateFilterInfo(reply);
            // APPEND END fp)k.yajima 2015.06.26

            /* delete for 140521 server */
            strncpy( message.m_header, "JPTP /1.0\r\n", 11 );
            message.m_initialize_parameter = -1;
            message.m_message_size = message.byteSize();
            client.sendMessage( message );
            client.recvMessage( &reply );
            client.termClient();
        }

    }

    /*
    if ( localminstep > -1 && localmaxstep > -1 )
    {
        // stepの範囲を示すparamのメンバー変数に設定を行う
        param->m_min_time_step = localminstep;
        param->m_max_time_step = localmaxstep;
    }
    */
    std::cout << "#Command::generateAbstractParticles numvol " << numvol << std::endl;
    std::cout << "#Command::generateAbstractParticles numstep " << param->m_max_server_time_step << std::endl;
    std::cout << "#Command::generateAbstractParticles minstep " << param->m_min_time_step << std::endl;
    std::cout << "#Command::generateAbstractParticles maxstep " << param->m_max_time_step << std::endl;

    for ( size_t i = 0; i < m_abstract_particles.size(); ++i )
    {
        delete m_abstract_particles[i];
    }

    m_abstract_particles.clear();

    {
        size_t mt = ( param->m_max_time_step > param->m_max_server_time_step ) ? param->m_max_time_step : param->m_max_server_time_step;
        for ( size_t i = 0; i < mt + 1; ++i )
        {
            m_abstract_particles.push_back( m_server->getPointObjectFromLocal() );
        }
    }

    if ( param->m_client_server_mode == 1 )
    {
        m_abstract_particles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
        m_abstract_particles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
    }
    else
    {

        qInfo("getPointObjectFromServer() calling for abstract particle @6");

        //std::cerr << "getPointObjectFromServer() calling for abstract particle @6" << std::endl;

#ifdef USE_SERVER_SIDE_PBVR_COORDS
        m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step, PBVRmincoords, PBVRmaxcoords );
#else // USE_SERVER_SIDE_PBVR_COORDS
        m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step );
#endif // USE_SERVER_SIDE_PBVR_COORDS

        qInfo("getPointObjectFromServer() called for abstract particle @6");
        //std::cerr << "getPointObjectFromServer() called for abstract particle @6" << std::endl;

        // ADD START FEAST 2016.01.07
        if ( m_abstract_particles.front() == NULL )
        {
            //            m_stop_button->pressed();
            m_timectrl_panel->toggleActive();
        }
        // ADD END FEAST 2016.01.07
    }
}

void Command::generateDetailedParticles()
{
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
}

size_t Command::getUsingMemoryByKiloByte()
{
    size_t memory = 0;
#ifndef CPUMODE
    for ( size_t i = 0; i < m_abstract_particles.size(); ++i )
    {
        memory += m_abstract_particles[i]->coords().size() * sizeof( kvs::Real32 ) + m_abstract_particles[i]->normals().size() * sizeof( kvs::Real32 ) + m_abstract_particles[i]->colors().size() * sizeof( kvs::UInt8 );
    }
    for ( size_t i = 0; i < m_detailed_particles.size(); ++i )
    {
        memory += m_detailed_particles[i]->coords().size() * sizeof( kvs::Real32 ) + m_detailed_particles[i]->normals().size() * sizeof( kvs::Real32 ) + m_detailed_particles[i]->colors().size() * sizeof( kvs::UInt8 );
    }
#endif
    return memory / 1024;
}

void Command::reDraw()
{
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
        res &= ( p->m_particle_limit == param.m_particle_limit );
        res &= ( p->m_particle_density == param.m_particle_density );
        res &= ( p->m_filter_parameter_filename == param.m_filter_parameter_filename );
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
    m_mutex.lock();
}

void Command::unlock()
{
    m_mutex.unlock();
}

void Command::closeServer()
{
    m_server->close( m_parameter );
}

void Command::apply_variable_range( const VariableRange& range )
{
    qInfo("Command::apply_variable_range >>>>>>>>>>>>>>>>>>>>>>>>>>>");
    ExtendedTransferFunctionMessage& doc = extransfuncedit->doc();
    bool f = false;

    for ( size_t i = 0; i < doc.m_color_transfer_function.size(); i++ )
    {
        if ( !doc.m_color_transfer_function[i].m_range_initialized )
        {
            const std::string& nm = doc.m_color_transfer_function[i].m_name;
            const std::string tag_c = nm + "_var_c";
            doc.m_color_transfer_function[i].m_color_variable_min   = range.min( tag_c );
            doc.m_color_transfer_function[i].m_color_variable_max   = range.max( tag_c );
            qInfo("Command::apply_variable_range >>>>> %s %f %f" ,tag_c.c_str(),
                  doc.m_color_transfer_function[i].m_color_variable_min,
                  doc.m_color_transfer_function[i].m_color_variable_max );
            doc.m_color_transfer_function[i].m_range_initialized = true;
            f = true;
        }
    }

    for ( size_t i = 0; i < doc.m_opacity_transfer_function.size(); i++ )
    {
        if ( !doc.m_opacity_transfer_function[i].m_range_initialized )
        {
            const std::string& nm = doc.m_opacity_transfer_function[i].m_name;
            const std::string tag_o = nm + "_var_o";
            doc.m_opacity_transfer_function[i].m_opacity_variable_min   = range.min( tag_o );
            doc.m_opacity_transfer_function[i].m_opacity_variable_max   = range.max( tag_o );
            qInfo("Command::apply_variable_range >>>>> %s %f %f" ,tag_o.c_str(),
                  doc.m_opacity_transfer_function[i].m_opacity_variable_min,
                  doc.m_opacity_transfer_function[i].m_opacity_variable_max );
            doc.m_opacity_transfer_function[i].m_range_initialized = true;
            f = true;
        }
    }

    if ( f && extransfuncedit )
    {
        extransfuncedit->updateRangeEdit();
    }

    return;
}
// 2018.12.19
//カレントのステップがサーバに存在するかのチェック
//start
bool Command::isExitServerStep(VisualizationParameter* param)
{
    if (param->m_time_step >= param->m_min_server_time_step && param->m_time_step <= param->m_max_server_time_step) return true;
    else return false;
}
int Command::getServerStep(VisualizationParameter* param )
{
    if (isExitServerStep(param)) return param->m_time_step;
    if (param->m_particle_merge_param.m_particles[0].m_keep_initial_step) return param->m_min_server_time_step;
    if (param->m_particle_merge_param.m_particles[0].m_keep_final_step) return param->m_max_server_time_step;

    return -1;
}
