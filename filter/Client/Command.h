#ifndef PBVR__KVS_VISCLIENT_COMMAND_H_INCLUDE
#define PBVR__KVS_VISCLIENT_COMMAND_H_INCLUDE

#include <vector>
#include <kvs/Mutex>

#include "VisualizationParameter.h"
#include "ReceivedMessage.h"
#include "ParticleServer.h"

namespace kvs
{
class TransferFunction;
class PointObject;
namespace glut
{
class Screen;
class Timer;
}
namespace visclient
{
//struct VisualizationParameter; /* add 20140123 */
class ParticleServer;
class ExtendedParticleVolumeRenderer;
class Controller;
class TimerEvent;
class ProgressBar;
class TimeStepSlider;
class TimeSegmentMinSlider;
class TimeSegmentMaxSlider;
class StopButton;
class TransferFunctionEditorMain;
class ParticleMergerUI;
class CoordinatepanelUI;

class Command
{
private:
    kvs::Mutex m_mutex;

    struct SamplingParams
    {
        SamplingParams( const VisualizationParameter& param ) :
            m_parameter_extend_transfer_function( param.m_parameter_extend_transfer_function ),
            m_parameter_crop( param.m_parameter_crop ),
            m_input_directory( param.m_input_directory ),
            m_particle_limit( param.m_particle_limit ),
            m_particle_density( param.m_particle_density ),
            m_filter_parameter_filename( param.m_filter_parameter_filename )
        {
        }

        const ExtendedTransferFunctionMessage m_parameter_extend_transfer_function;
        const CropParameter m_parameter_crop;
        const std::string m_input_directory;
        const int32_t m_particle_limit;
        const float m_particle_density;
        const std::string m_filter_parameter_filename;
    };
    const SamplingParams* m_last_sampling_params;

private:
    int m_local_subpixel_level;

public:

    Command( ParticleServer* server );

    ~Command();

public:

    void startAbstractAnalysis();

    void startDetailedAnalysis();

    void changeShowHide();

    void preUpdate();

    void update( VisualizationParameter* param, ReceivedMessage* result );

    void postUpdate();

    void closeServer();

    int getServerParticleInfomation( VisualizationParameter* param, ReceivedMessage* result );

    void initializeAbstractParticles( VisualizationParameter* param, ReceivedMessage* result, const int localminstep, const int localmaxstep );

//    void generateAbstractParticles( PBVRParam& param, PBVRResult* result, int localminstep = -1, int localmaxstep = -1 );
    void generateAbstractParticles( VisualizationParameter* param, ReceivedMessage* result );

    void generateDetailedParticles();

    void reDraw();

    bool isSamplingParamsKeeped( const VisualizationParameter& param );

    void saveSamplingParams( const VisualizationParameter& param );

private:
    bool m_is_shown;

public:
    bool m_is_under_animation;
    bool m_is_under_communication;
    bool m_is_key_frame_animation;
    bool m_is_import_transfer_function_parameter;
    bool m_particle_assign_flag;
    VisualizationParameter m_parameter;
    ReceivedMessage m_result;
    ExtendedParticleVolumeRenderer* m_renderer;
    TransferFunctionEditorMain* m_extend_transfer_function_editor;
    TimeSegmentMinSlider* m_min_time_slider;
    TimeSegmentMaxSlider* m_max_time_slider;
    TimeStepSlider* m_time_slider;
    kvs::glut::Screen* m_screen;
    kvs::glut::Screen* m_screen3;
    Controller* m_controller;
    ProgressBar* m_progress;
    StopButton* m_stop_button;
    ParticleMergerUI* m_particle_merge_ui;
    CoordinatepanelUI* m_coord_panel_ui;
    kvs::glut::Timer* m_glut_timer;
    ParticleServer* m_server;
    int  m_step_key_frame;
    int  m_previous_key_frame;

    std::vector<kvs::PointObject*> m_abstract_particles;
    std::vector<kvs::PointObject*> m_detailed_particles;

private:
    std::vector<kvs::PointObject*> m_server_particles;
    std::vector<kvs::Vector3f> m_server_coord_min;
    std::vector<kvs::Vector3f> m_server_coord_max;
    std::vector<bool> m_server_coord_flag;

public:
    size_t getUsingMemoryByKiloByte();
    void lock();
    void unlock();

private:	// コピー禁止．
    Command( const Command& );
    Command operator=( const Command& );

	//2018.12.19 start
	bool isExitServerStep(VisualizationParameter* param);
	int getServerStep(VisualizationParameter* param);
	//2018.12.19 end

protected:
    void apply_variable_range( const VariableRange& range );

};

}
}

#endif
