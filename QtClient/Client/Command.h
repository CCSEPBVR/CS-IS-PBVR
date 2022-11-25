#ifndef PBVR__KVS_VISCLIENT_COMMAND_H_INCLUDE
#define PBVR__KVS_VISCLIENT_COMMAND_H_INCLUDE

#include <vector>
#include <kvs/Mutex>

#include "VisualizationParameter.h"
#include "ReceivedMessage.h"
#include "ParticleServer.h"
class RenderArea;

class TransferFuncEditor;
class ParticlePanel;
class CoordinatePanel;
class TimecontrolPanel;
//class KVSQScreen;
namespace QGlue{
class Timer;
class Screen;
}

namespace kvs
{
class TransferFunction;
class PointObject;

namespace visclient
{
//struct VisualizationParameter; /* add 20140123 */
class ParticleServer;
//class PBRProxy;
struct Controller;
//class TimerEvent;
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
            m_input_directory( param.m_input_directory ),
            m_particle_limit( param.m_particle_limit ),
            m_particle_density( param.m_particle_density )
  #ifdef CS_MODE
          ,m_filter_parameter_filename( param.m_filter_parameter_filename )
  #endif
        {
        }

        const ExtendedTransferFunctionMessage m_parameter_extend_transfer_function;
        const std::string m_input_directory;
        const int32_t m_particle_limit;
        const float m_particle_density;
        const std::string m_filter_parameter_filename;
    };
    const SamplingParams* m_last_sampling_params;

private:
    //    MOD BY)T.Osaki 2020.04.28
    int m_local_subpixel_level = 0;
    //    ADD BY)T.Osaki 2020.03.16
    int m_server_subpixel_level = 0;

public:

    Command( ParticleServer* server );
    ~Command();
    bool lastStepCheckBoxState=false;
public:

    void startAbstractAnalysis();

    void startDetailedAnalysis();

    void changeShowHide();

    void preUpdate();

    void update( VisualizationParameter* param, ReceivedMessage* result );
    void update_doExport( VisualizationParameter* param);
    void update_doAbstractAnalysis(VisualizationParameter *param, ReceivedMessage* result );
    void update_doDetailedAnalysis(VisualizationParameter* param, ReceivedMessage* result );

    void postUpdate();

    void closeServer();
#ifdef IS_MODE
    void getServerParticleInfomation( VisualizationParameter* param, ReceivedMessage* result );
#else
    int getServerParticleInfomation( VisualizationParameter* param, ReceivedMessage* result );
#endif
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
    bool m_is_polygon_checkbox_current[5] = {false,false,false,false,false};
    bool m_is_polygon_checkbox_before[5] = {false,false,false,false,false};
    kvs::Real32 m_server_side_subpixel_level;
    VisualizationParameter m_parameter;
    ReceivedMessage m_result;
//    ExtendedParticleVolumeRenderer* m_renderer;
    //    TransferFunctionEditorMain* m_extend_transfer_function_editor;
    TransferFuncEditor* extransfuncedit;
    TimecontrolPanel* m_timectrl_panel;
    RenderArea* m_screen;
    //    QGlue::Screen* m_screen3;
    Controller* m_controller;
    //    ProgressBar* m_progress;
    //    StopButton* m_stop_button;
    //    ParticleMergerUI* m_particle_merge_ui;
    ParticlePanel* m_particle_merge_ui;
    //    CoordinatepanelUI* m_coord_panel_ui;
    CoordinatePanel* m_coord_panel_ui;
    QGlue::Timer* m_glut_timer = NULL;
    ParticleServer* m_server;
    int  m_step_key_frame;
    int  m_previous_key_frame;

    std::vector<kvs::PointObject*> m_detailed_particles;

    float particle_data_size_limit_input;

    int pretimestep  = -1;
    int numvol=0;
    int numstep = 0;
    char pre_filter_parameter_filename[256];
    int exportMergedParticleStartStep = -1;
    char pre_pfi_path_server[256];
#ifdef _TIMER_
    //#define TIMER_MAX_COUNT 9
    static int timer_count = 0;
#endif

    size_t m_number_nodes;
    size_t m_number_elements;
    size_t m_element_type;
    size_t m_file_type;
    size_t m_number_ingredients;
    size_t m_number_step;
    size_t m_number_volume_divide;
    float m_min_object_coord[3];
    float m_max_object_coord[3];

    static float	PBVRmaxcoords[3];
    static float	PBVRmincoords[3];
private:
    std::vector<kvs::PointObject*> m_server_particles;
    std::vector<kvs::Vector3f> m_server_coord_min;
    std::vector<kvs::Vector3f> m_server_coord_max;
    std::vector<bool> m_server_coord_flag;
    bool keeper = false;

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
