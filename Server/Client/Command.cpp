#include <algorithm>
#include <string.h>
#include "ParticleTransferClient.h"

#include <kvs/glut/Screen>
#include <kvs/glut/Timer>

#include "ParticleMerger.h"
#include "ParticleServer.h"
#include "CoordinatepanelUI.h"
#include "Command.h"
#include "ExtendedParticleVolumeRenderer.h"
#include "Controller.h"
#include "TimerEvent.h"
#include "TransferFunctionEditorMain.h"

#include <kvs/Directory>
#include <kvs/File>

#include "timer_simple.h"

using namespace kvs::visclient;

// APPEND START fp)m.tanaka 2013.09.01
extern int subpixellevela;
extern int subpixelleveld;
extern int repeatlevela;
extern int repeatleveld;
extern int plimitlevel;

// APPEND START fp)m.takizawa 2014.05.21
float	PVBRmaxcoords[3];
float	PVBRmincoords[3];
// APPEND END fp)m.takizawa 2014.05.21

// APPEND START fp)k.yajima 2015.02.26
extern GLUI_StaticText* lbl_numParticles;
extern GLUI_StaticText* lbl_numNodes;
extern GLUI_StaticText* lbl_numElements;
extern GLUI_StaticText* lbl_elemType;
extern GLUI_StaticText* lbl_fileType;
extern GLUI_StaticText* lbl_numIngredients;
extern GLUI_StaticText* lbl_numStep;
extern GLUI_StaticText* lbl_numVolDiv;
extern GLUI_StaticText* lbl_objectCoord0;
extern GLUI_StaticText* lbl_objectCoord1;
extern GLUI_StaticText* lbl_objectCoord2;
// APPEND END fp)k.yajima 2015.02.26

// APPEND START fp)m.takizawa 2014.05.29
static int pretimestep  = -1;
extern	void	ScreenShot(  kvs::ScreenBase* screen, const int tstep );
// APPEND END fp)m.takizawa 2014.05.29

//extern auto_ptr<kvs::visclient::ProgressBar> *g_screen3_progress;
// APPEND END fp)m.tanaka 2013.09.01

static int numvol, numstep = 0;
#ifdef _TIMER_
#define TIMER_MAX_COUNT 9
static int timer_count = 0;
#endif

extern char pfi_path_server[256];
static char pre_pfi_path_server[256];
// add:start by @hira at 2016/12/01
extern char filter_parameter_filename[256];
static char pre_filter_parameter_filename[256];
// add:end by @hira at 2016/12/01
static int exportMergedParticleStartStep = -1;
extern int start_store_step;

#if 1 // FEAST modify 20160128
extern int g_curStep;
#endif // FEAST modify 20160128 end

Command::Command( ParticleServer* server ) :
    m_is_under_animation( true ),
    m_is_under_communication( true ),
    m_is_shown( true ),
    m_is_import_transfer_function_parameter( false ),
    m_is_key_frame_animation( false ),
    m_server( server ),
    m_last_sampling_params( NULL ),
    m_particle_assign_flag( false )
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
    m_controller->m_detail->setButtonColor( ::Gray );
//	m_parameter.m_time_step = 1;
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
        m_controller->abst->setButtonColor( ::Gray );
        generateDetailedParticles();
    */
    m_parameter.m_time_step = m_parameter.m_min_time_step;
    m_parameter.m_max_time_step = m_parameter.m_max_server_time_step;
    std::cout << "*** START DETAILEDANALYSIS" << std::endl;
    std::cout << "param.m_min_time_step:" << m_parameter.m_min_time_step << std::endl;
    std::cout << "param.m_max_time_step:" << m_parameter.m_max_time_step << std::endl;
}

void Command::changeShowHide()
{
    if ( m_is_shown )
    {
        m_progress->hide();
        m_time_slider->hide();
        m_min_time_slider->hide();
        m_max_time_slider->hide();
    }
    else
    {
        m_progress->show();
        m_time_slider->show();
        m_min_time_slider->show();
        m_max_time_slider->show();
    }

    m_is_shown = !m_is_shown;
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
#if 0 // FEAST modify 20160128
    if ( m_is_under_animation )
    {
        if ( m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
#ifndef CPUMODE
            if ( m_parameter.m_repeat_level == m_parameter.m_detailed_repeat_level )
            {
#endif
                m_parameter.m_time_step++;
#ifndef CPUMODE
            }
#endif
        }
        else if ( m_parameter.m_transfer_type == VisualizationParameter::Abstract )
        {
#ifndef CPUMODE
            if ( m_parameter.m_repeat_level == m_parameter.m_abstract_repeat_level )
            {
#endif
                m_parameter.m_time_step++;
#ifndef CPUMODE
            }
#endif
        }
    }
    if ( m_parameter.m_time_step > m_max_time_slider->value() ) //99) {
    {
        //m_parameter.m_time_step = 1;
        //m_parameter.m_time_step = 0;
        //if( m_parameter.m_transfer_type == VisualizationParameter::Detailed ) m_parameter.m_time_step = m_min_time_slider->value();
// APPEND START fj) 2015.04.03
        if ( m_min_time_slider->value() >= m_parameter.m_min_time_step )
        {
            m_parameter.m_time_step = m_min_time_slider->value();
        }
        else
        {
            m_parameter.m_time_step = m_parameter.m_min_time_step;
        }
// APPEND END   fj) 2015.04.03
    }
#endif // FEAST modify 20160128 end

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

#if 0 // FEAST modify 20160128
    if ( m_is_under_communication )
    {
        if ( m_parameter.m_transfer_type == VisualizationParameter::Abstract )
        {
            m_progress->setValue( static_cast<float>( m_parameter.m_time_step ) );
            m_time_slider->setValue( static_cast<float>( m_parameter.m_time_step ) );
        }
        else if ( m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
            if ( m_parameter.m_time_step > m_parameter.m_max_time_step )
            {
                m_parameter.m_time_step = m_parameter.m_min_time_step;
            }
            float ratio = float( m_parameter.m_time_step - m_parameter.m_min_time_step ) / float( m_parameter.m_max_time_step - m_parameter.m_min_time_step ) * 100;
            m_progress->setValue( static_cast<float>( ratio ) );
            m_time_slider->setValue( static_cast<float>( m_parameter.m_time_step ) );
        }
    }
#endif // FEAST modify 20160128 end

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
        printf( " ### [debug]: exportMergedParticleStartStep = %d, m_parameter.m_time_step = %d\n", exportMergedParticleStartStep, m_parameter.m_time_step );
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
                m_particle_merge_ui->paricleExportBottomActive();
            }
        }
    }
}

void Command::update( VisualizationParameter* param, ReceivedMessage* result )
{
    // Viewer のスケール正規化用.
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
            if ( m_server_particles[step]->nvertices() <= 1 )
            {
                object = NULL;
            }
            else
            {
                object = m_server_particles[step];
            }
            p = merger.doMerge( *object, step );
            delete p;
        }
        param->m_particle_merge_param.m_do_export = false;
    }

    if ( m_is_under_communication )
    {
        if ( param->m_no_repsampling && start_store_step == -2 && m_server_coord_flag[param->m_time_step] )
        {
            std::cout << "*** Display particle data on memory." << std::endl;
        }
        else
        {
            // 保存始めのステップ番号がMin Time より小さい場合はMin Timeを保存始めのステップとする.
            if ( start_store_step < static_cast<int>( m_min_time_slider->value() ) )
                 start_store_step = static_cast<int>( m_min_time_slider->value() );

            if ( start_store_step == param->m_time_step )
            {
                start_store_step = -2; // 全ステップの粒子をローカルに保存するタイミングなので-2をセット
            }
            // When Transfer Functions are edited.
            if ( param->m_transfer_type == VisualizationParameter::Abstract )
            {

                if ( param->m_detailed_transfer_type == VisualizationParameter::Divided )
                {
                    std::cout << "*** param->m_detailed_transfer_type == VisualizationParameter::Divided" << std::endl;
#ifndef CPUMODE
                    std::cout << "param->m_repeat_level=" << param->m_repeat_level << " param->m_abstract_repeat_level=" << param->m_abstract_repeat_level << std::endl;
                    if ( param->m_repeat_level == param->m_abstract_repeat_level )
                    {
#endif
						// サーバにないステップは対象としない 2018.12.19 start
						int stepno;
						if ((stepno = getServerStep(param)) >= 0){
							delete m_abstract_particles[param->m_time_step];
							m_abstract_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
							// ADD START FEAST 2016.01.07
							// 粒子生成時にサーバー側で演算エラーが発生
							if (m_abstract_particles[param->m_time_step] == NULL)
							{
								m_stop_button->pressed();
							}
							// ADD END FEAST 2016.01.07
						} // 2018.12.19
						else if (param->m_client_server_mode == 0  ){
							if (m_abstract_particles[param->m_time_step]) delete m_abstract_particles[param->m_time_step];
						}
#ifndef CPUMODE
                    }
                    else if ( param->m_repeat_level < param->m_abstract_repeat_level )
                    {
                        PointObject* m_dividedObject = server->getPointObjectFromServer( param, result, numvol );
                        // ADD START FEAST 2016.01.07
                        if ( m_dividedObject == NULL )
                        {
                            m_stop_button->pressed();
                        }
                        else
                        {
                            m_abstract_particles[param->m_time_step]->add( *m_dividedObject );
                            delete m_dividedObject;
                        }
                        // ADD END FEAST 2016.01.07
                    }
                    else
                    {
                        assert( false );
                    }
#endif
                }
                else if ( param->m_detailed_transfer_type == VisualizationParameter::Summalized )
                {
					// サーバにないステップは対象としない 2018.12.19 start
					int stepno;
					if ((stepno = getServerStep(param)) >= 0 ){
						delete m_abstract_particles[param->m_time_step];
						m_abstract_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
						// ADD START FEAST 2016.01.07
						if (m_abstract_particles[param->m_time_step] == NULL)
						{
							m_stop_button->pressed();
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
            {

                if ( param->m_detailed_transfer_type == VisualizationParameter::Divided )
                {
#ifndef CPUMODE
                    if ( param->m_repeat_level == param->m_detailed_repeat_level )
                    {
#endif
						// サーバにないステップは対象としない 2018.12.19 start
						int stepno;
						if ((stepno = getServerStep(param)) >= 0 ){
							delete m_detailed_particles[param->m_time_step];
							m_detailed_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
							// ADD START FEAST 2016.01.07
							if (m_detailed_particles[param->m_time_step] == NULL)
							{
								m_stop_button->pressed();
							}
							// ADD END FEAST 2016.01.07
						} // 2018.12.19 end
						else if (param->m_client_server_mode == 0  ){
							if (m_detailed_particles[param->m_time_step]) delete m_detailed_particles[param->m_time_step];
						}
#ifndef CPUMODE
                    }
                    else if ( param->m_repeat_level < param->m_detailed_repeat_level )
                    {
                        PointObject* m_dividedObject = server->getPointObjectFromServer( param, result, numvol );
                        // ADD START FEAST 2016.01.07
                        if ( m_dividedObject == NULL )
                        {
                            m_stop_button->pressed();
                        }
                        else
                        {
                            m_detailed_particles[param->m_time_step]->add( *m_dividedObject );
                            delete m_dividedObject;
                        }
                        // ADD END FEAST 2016.01.07
                    }
                    else
                    {
                        assert( false );
                    }
#endif
                }
                else if ( param->m_detailed_transfer_type == VisualizationParameter::Summalized )
                {
					// サーバにないステップは対象としない 2018.12.19 start
					int stepno;
					if ((stepno = getServerStep(param)) >= 0){
						if (m_detailed_particles.size() > param->m_time_step){
							if (m_detailed_particles[param->m_time_step])
								delete m_detailed_particles[param->m_time_step];
							m_detailed_particles[param->m_time_step] = m_server->getPointObjectFromServer(*param, result, numvol, stepno);
						}
						else{
							m_detailed_particles.push_back(m_server->getPointObjectFromServer(*param, result, numvol, stepno));
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
						m_server_particles[param->m_time_step] = m_detailed_particles[param->m_time_step];

						m_coord_panel_ui->resetChangeFlagForStoreParticle();

						
						// ADD START FEAST 2016.01.07
						if (m_detailed_particles[param->m_time_step] == NULL)
						{
							m_stop_button->pressed();
						}
						// ADD END FEAST 2016.01.07
					}// 2018.12.19 end
					else if (param->m_client_server_mode == 0){
						if (m_detailed_particles[param->m_time_step]) delete m_detailed_particles[param->m_time_step];
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
#if 0
			// サーバにないステップは対象としない 2018.12.19 start
			int stepno;
			if ((stepno = getServerStep(param)) >= 0){
				if (m_server_particles.size() < param->m_time_step + 1)
				{
					m_server_particles.resize(param->m_time_step + 1, NULL);
					// サーバー生成粒子の各stepの存在範囲を格納
					m_server_coord_min.resize(param->m_time_step + 1);
					m_server_coord_max.resize(param->m_time_step + 1);
					m_server_coord_flag.resize(param->m_time_step + 1, false);
				}
				if (m_server_particles[param->m_time_step])
					delete m_server_particles[param->m_time_step];
				m_server_particles[param->m_time_step] = m_detailed_particles[param->m_time_step];

				m_coord_panel_ui->resetChangeFlagForStoreParticle();
			}
#endif
		}

        // サーバから送られてくる粒子について保存している存在範囲情報を消去
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
					m_server_particles[param->m_time_step]->updateMinMaxCoords();
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
        object = merger.doMerge( *server_object, param->m_time_step );
        result->m_min_merged_time_step = merger.getMergedInitialTimeStep();
        result->m_max_merged_time_step = merger.getMergedFinalTimeStep();
		// サーバにないステップは対象としない 2018.12.19 start
		if (m_detailed_particles.size() > param->m_time_step){
			m_detailed_particles[param->m_time_step] = object;
		}
		else {
			m_detailed_particles.push_back(object);
		}
        // Change TransferFunction
        if ( param->m_client_server_mode == 1 && ( !isSamplingParamsKeeped( *param ) || m_coord_panel_ui->getChangeCoordFlagForStoreParticle() ) )
        {
            // 保存始めたstep番号の保存と、可視化パラメータの保存
            saveSamplingParams( *param );
            start_store_step = param->m_time_step;
            m_coord_panel_ui->resetChangeFlagForStoreParticle();
        }

        // Viewerの正規化用処理
        local_particle_exits = merger.calculateExternalCoords( local_crd );
        //サーバで計算されたsubpixel levelの更新.
        if ( param->m_client_server_mode == 1 )
        {
			if (m_server_particles.size() <= param->m_time_step || m_server_particles[param->m_time_step] == NULL)
            {
                m_local_subpixel_level = object->size( 0 );
            }
            else
            {
                m_local_subpixel_level = result->m_subpixel_level;
            }
        }
        else
        {
            m_local_subpixel_level = object->size( 0 );
        }

        if ( m_max_time_slider->maxValue() != result->m_max_merged_time_step
                || m_min_time_slider->minValue() != result->m_min_merged_time_step )
        {
            m_min_time_slider->hide();
            m_max_time_slider->hide();
        }

        if ( m_max_time_slider->maxValue() != result->m_max_merged_time_step )
        {
            param->m_max_time_step = result->m_max_merged_time_step;
            m_max_time_slider->setValue( static_cast<float>( param->m_max_time_step ) );
        }

        if ( m_min_time_slider->minValue() != result->m_min_merged_time_step )
        {
            param->m_min_time_step = result->m_min_merged_time_step;
            m_min_time_slider->setValue( static_cast<float>( param->m_min_time_step ) );
        }

        if ( m_max_time_slider->maxValue() != result->m_max_merged_time_step
                || m_min_time_slider->minValue() != result->m_min_merged_time_step )
        {
            m_max_time_slider->setRange( param->m_min_time_step, param->m_max_time_step );
            m_min_time_slider->setRange( param->m_min_time_step, param->m_max_time_step );
            m_time_slider->setRange( param->m_min_time_step, param->m_max_time_step );
            m_max_time_slider->show();
            m_min_time_slider->show();
        }

        m_time_slider->hide();
        size_t m_time_step = std::max( param->m_min_time_step, std::min( param->m_max_time_step, param->m_time_step ) );
        param->m_time_step = m_time_step;
        m_time_slider->show();

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

        // 粒子数の表示
        char buff[512];
        //sprintf( buff, "Display Particle Number : %d", object->nvertices() );
		sprintf(buff, "Display Particle Number : %zd", object->nvertices());
		lbl_numParticles->name = buff;
#ifdef __APPLE__
        lbl_numParticles->redraw();
#endif
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
        if (param->m_hasfin) {
            if (strcmp( pre_filter_parameter_filename, filter_parameter_filename ) != 0) is_change = true;
        }
        else {
            if (strcmp( pre_pfi_path_server, pfi_path_server ) != 0) is_change = true;
        }
    }

    if (is_change )
    {
#ifdef _DEBUG
        if (param->m_hasfin) {
            printf(" [debug] change filter parameter file. %s & %s\n", pre_filter_parameter_filename, filter_parameter_filename);
        }
        else {
            printf(" [debug] change pfi file. %s -> %s\n", pre_pfi_path_server, pfi_path_server);
        }
#endif
        numstep = 0;
        param->m_max_server_time_step = 0;
        // m_is_import_transfer_function_parameter = false;
        generateAbstractParticles ( ( VisualizationParameter* )param, result );
        this->m_parameter = *param;
        this->m_result = *result;
        generateDetailedParticles();

        m_time_slider->hide();
        m_min_time_slider->hide();
        m_max_time_slider->hide();
        m_time_slider->setRange( param->m_min_time_step, param->m_max_time_step );
        m_min_time_slider->setRange( param->m_min_time_step, param->m_max_time_step );
        m_max_time_slider->setRange( param->m_min_time_step, param->m_max_time_step );
        m_time_slider->setValue( static_cast<float>( param->m_min_time_step ) );
        m_min_time_slider->setValue( static_cast<float>( param->m_min_time_step ) );
        m_max_time_slider->setValue( static_cast<float>( param->m_max_time_step ) );
        m_time_slider->show();
        m_min_time_slider->show();
        m_max_time_slider->show();

        // For change view (Reset scale)
        server_resetflag = true;
    }

    // For change view (Reset scale)
    if ( m_particle_merge_ui->checkChengeFlag() || m_coord_panel_ui->getResetViewFlag() )
    {
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
                    // サーバの粒子情報とlocalの粒子情報を比較して範囲が広い方を採用
                    crd[0] = std::min( local_crd[0], PVBRmincoords[0] );
                    crd[1] = std::min( local_crd[1], PVBRmincoords[1] );
                    crd[2] = std::min( local_crd[2], PVBRmincoords[2] );
                    crd[3] = std::max( local_crd[3], PVBRmaxcoords[0] );
                    crd[4] = std::max( local_crd[4], PVBRmaxcoords[1] );
                    crd[5] = std::max( local_crd[5], PVBRmaxcoords[2] );
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
                    crd[0] = PVBRmincoords[0];
                    crd[1] = PVBRmincoords[1];
                    crd[2] = PVBRmincoords[2];
                    crd[3] = PVBRmaxcoords[0];
                    crd[4] = PVBRmaxcoords[1];
                    crd[5] = PVBRmaxcoords[2];
                    resetflag = true;
                }
            }

            if ( m_coord_panel_ui->getResetViewFlag() )
            {
                // 正規化フラグをfalseに戻す
                m_coord_panel_ui->resetResetViewFlag();
            }
        }
        m_particle_merge_ui->paricleResetChangedFlag();
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

    // change view
    if ( resetflag )
    {
        kvs::Vector3f min_t( crd[0], crd[1], crd[2] );
        kvs::Vector3f max_t( crd[3], crd[4], crd[5] );
        m_screen->objectManager()->object()->setMinMaxObjectCoords( min_t, max_t );
        m_screen->objectManager()->object()->setMinMaxExternalCoords( min_t, max_t );
        m_screen->objectManager()->updateExternalCoords();
        std::cout << " !!!!!!!!!!!!!!!!!!! Reset Viewer Scale !!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
    }
}

void Command::postUpdate()
{
    if ( m_parameter.m_client_server_mode == 0 )
    {
        m_particle_merge_ui->paricleExportBottomActive();
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
    m_parameter.m_detailed_subpixel_level = m_local_subpixel_level;
#ifdef CPUMODE
    m_renderer->setSubpixelLevel( m_parameter.m_detailed_subpixel_level );
    m_renderer->recreateImageBuffer();
#endif
    PBVR_TIMER_STA( 150 );

#ifndef CPUMODE
    if ( m_parameter.m_transfer_type == VisualizationParameter::Abstract )
    {
#ifndef CPUMODE
        m_renderer->setRepetitionLevel( m_parameter.m_abstract_repeat_level );
#endif
        m_renderer->setSubpixelLevel( m_parameter.m_abstract_subpixel_level );
        PointObject* object = m_abstract_particles[m_parameter.m_time_step];
        m_renderer->changePointObject( object );
    }
    else if ( m_parameter.m_transfer_type == VisualizationParameter::Detailed )
    {
#ifndef CPUMODE
        m_renderer->setRepetitionLevel( m_parameter.m_repeat_level );
#endif
        m_renderer->setSubpixelLevel( m_parameter.m_detailed_subpixel_level );
        PointObject* object = m_detailed_particles[m_parameter.m_time_step];
        m_renderer->changePointObject( object );
    }
    else
    {
        assert( false );
    }
#endif

#if 0  // FEAST modify 20160128
    if ( pretimestep != m_parameter.m_time_step )
    {
        ScreenShot( screen, m_parameter.m_time_step );

        pretimestep = m_parameter.m_time_step;
    }
#else
    if ( pretimestep != g_curStep )
    {
        ScreenShot( m_screen, g_curStep );

        pretimestep = g_curStep;
    }
#endif  // FEAST modify 20160128 end

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
    m_extend_transfer_function_editor->updateRangeView();

    PBVR_TIMER_END( 1 );
#ifdef _TIMER_
    if ( timer_count == TIMER_MAX_COUNT ) PBVR_TIMER_FIN();
#endif

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
            PVBRmaxcoords[0] = reply.m_max_object_coord[0];
            PVBRmaxcoords[1] = reply.m_max_object_coord[1];
            PVBRmaxcoords[2] = reply.m_max_object_coord[2];

            PVBRmincoords[0] = reply.m_min_object_coord[0];
            PVBRmincoords[1] = reply.m_min_object_coord[1];
            PVBRmincoords[2] = reply.m_min_object_coord[2];

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

            char buff[512];
            sprintf( buff, "All node: %d", reply.m_number_nodes );
            lbl_numNodes->name = buff;
            sprintf( buff, "All element: %d", reply.m_number_elements );
            lbl_numElements->name = buff;
            sprintf( buff, "Element type: %d", reply.m_element_type ); // MODIFY fj 2015.03.15
            lbl_elemType->name = buff;
            sprintf( buff, "File type: %d", reply.m_file_type ); // MODIFY fj 2015.03.15
            lbl_fileType->name = buff;
            sprintf( buff, "Vector num: %d", reply.m_number_ingredients );
            lbl_numIngredients->name = buff;
            sprintf( buff, "Step num: %d", reply.m_number_step );
            lbl_numStep->name = buff;
            sprintf( buff, "SUB volume: %d", reply.m_number_volume_divide );
            lbl_numVolDiv->name = buff;
            sprintf( buff, " X-Axis: min=%f, max=%f", reply.m_min_object_coord[0], reply.m_max_object_coord[0] );
            lbl_objectCoord0->name = buff;
            sprintf( buff, " Y-Axis: min=%f, max=%f", reply.m_min_object_coord[1], reply.m_max_object_coord[1] );
            lbl_objectCoord1->name = buff;
            sprintf( buff, " Z-Axis: min=%f, max=%f", reply.m_min_object_coord[2], reply.m_max_object_coord[2] );
            lbl_objectCoord2->name = buff;

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

    std::cout << "Command::generateAbstractParticles numvol " << numvol << std::endl;
    std::cout << "Command::generateAbstractParticles numstep " << param->m_max_server_time_step << std::endl;
    std::cout << "Command::generateAbstractParticles minstep " << param->m_min_time_step << std::endl;
    std::cout << "Command::generateAbstractParticles maxstep " << param->m_max_time_step << std::endl;

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
        minObjectCoord.set( PVBRmincoords );
        maxObjectCoord.set( PVBRmaxcoords );

        m_abstract_particles.front()->setMinMaxObjectCoords( minObjectCoord, maxObjectCoord );
        m_abstract_particles.front()->setMinMaxExternalCoords( minObjectCoord, maxObjectCoord );
    }
    else
    {
        m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step );
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

// APPEND START fp)m.takizawa 2014.05.21

            minObjectCoord = kvs::Vector3f( reply.m_min_object_coord );
            maxObjectCoord = kvs::Vector3f( reply.m_max_object_coord );
            PVBRmaxcoords[0] = reply.m_max_object_coord[0];
            PVBRmaxcoords[1] = reply.m_max_object_coord[1];
            PVBRmaxcoords[2] = reply.m_max_object_coord[2];

            PVBRmincoords[0] = reply.m_min_object_coord[0];
            PVBRmincoords[1] = reply.m_min_object_coord[1];
            PVBRmincoords[2] = reply.m_min_object_coord[2];

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
            char buff[512];
            sprintf( buff, "All node: %d", reply.m_number_nodes );
            lbl_numNodes->name = buff;
            sprintf( buff, "All element: %d", reply.m_number_elements );
            lbl_numElements->name = buff;
// 	sprintf(buff, "Element type: %c", reply.m_element_type);
            sprintf( buff, "Element type: %d", reply.m_element_type ); // MODIFY fj 2015.03.15
            lbl_elemType->name = buff;
// 	sprintf(buff, "File type: %c", reply.m_file_type);
            sprintf( buff, "File type: %d", reply.m_file_type ); // MODIFY fj 2015.03.15
            lbl_fileType->name = buff;
            sprintf( buff, "Vector num: %d", reply.m_number_ingredients );
            lbl_numIngredients->name = buff;
            sprintf( buff, "Step num: %d", reply.m_number_step );
            lbl_numStep->name = buff;
            sprintf( buff, "SUB volume: %d", reply.m_number_volume_divide );
            lbl_numVolDiv->name = buff;
            sprintf( buff, " X-Axis: min=%f, max=%f", reply.m_min_object_coord[0], reply.m_max_object_coord[0] );
            lbl_objectCoord0->name = buff;
            sprintf( buff, " Y-Axis: min=%f, max=%f", reply.m_min_object_coord[1], reply.m_max_object_coord[1] );
            lbl_objectCoord1->name = buff;
            sprintf( buff, " Z-Axis: min=%f, max=%f", reply.m_min_object_coord[2], reply.m_max_object_coord[2] );
            lbl_objectCoord2->name = buff;
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
    std::cout << "Command::generateAbstractParticles numvol " << numvol << std::endl;
    std::cout << "Command::generateAbstractParticles numstep " << param->m_max_server_time_step << std::endl;
    std::cout << "Command::generateAbstractParticles minstep " << param->m_min_time_step << std::endl;
    std::cout << "Command::generateAbstractParticles maxstep " << param->m_max_time_step << std::endl;

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
        m_abstract_particles.front() = m_server->getPointObjectFromServer( *param, result, numvol, param->m_time_step );
        // ADD START FEAST 2016.01.07
        if ( m_abstract_particles.front() == NULL )
        {
            m_stop_button->pressed();
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
    m_screen3->redraw();
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
        res &= ( p->m_parameter_crop == param.m_parameter_crop );
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
    ExtendedTransferFunctionMessage& doc = m_extend_transfer_function_editor->doc();

    bool f = false;

    for ( size_t i = 0; i < doc.m_color_transfer_function.size(); i++ )
    {
        if ( !doc.m_color_transfer_function[i].m_range_initialized )
        {
            const std::string& nm = doc.m_color_transfer_function[i].m_name;
            const std::string tag_c = nm + "_var_c";
            doc.m_color_transfer_function[i].m_color_variable_min   = range.min( tag_c );
            doc.m_color_transfer_function[i].m_color_variable_max   = range.max( tag_c );
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
            doc.m_opacity_transfer_function[i].m_range_initialized = true;
            f = true;
        }
    }

    if ( f && m_extend_transfer_function_editor )
    {
        m_extend_transfer_function_editor->updateRangeEdit();
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
//end

