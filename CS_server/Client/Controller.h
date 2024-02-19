#ifndef PBVR__KVS__VISCLIENT__COMMUNICATION_CONTROLLER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__COMMUNICATION_CONTROLLER_H_INCLUDE

#include "kvs/RGBColor"
#include "kvs/glut/Application"
#include "kvs/glut/Screen"
#include "kvs/glut/PushButton"
#include "kvs/glut/Slider"
#include "ProgressBar.h"

#include "kvs/glut/RadioButton"
#include "kvs/glut/RadioButtonGroup"

namespace
{
const kvs::RGBColor Gray( 100, 100, 100 );
const kvs::RGBColor Red( 255, 0, 0 );
const kvs::RGBColor Green( 0, 255, 0 );
const kvs::RGBColor Blue( 0, 0, 255 );
kvs::RGBColor Color = Gray;
}

namespace kvs
{
namespace visclient
{

class Command;

class StartDetailedAnalysisButton : public kvs::glut::PushButton
{
public:

    StartDetailedAnalysisButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::PushButton( screen ),
        m_command( command )
    {};

    void pressed();

private:
    Command* m_command;
};

class StartAbstractAnalysisButton : public kvs::glut::PushButton
{
public:

    StartAbstractAnalysisButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::PushButton( screen ),
        m_command( command )
    {};

    void pressed();

private:
    Command* m_command;
};

class StopButton : public kvs::glut::PushButton
{
private:
    Command* m_command;
public:

    StopButton( kvs::glut::Screen* screen, Command* command ) :
        kvs::glut::PushButton( screen ),
        m_command( command )
    {};

    void pressed();
};

class ShowHideButton : public kvs::glut::PushButton
{
private:
    Command* m_command;
    bool m_is_shown;
public:

    ShowHideButton( kvs::glut::Screen* screen, Command* command ) :
        kvs::glut::PushButton( screen ),
        m_command( command ),
        m_is_shown( true )
    {};

    void pressed();
};

class TimeStepSlider : public kvs::glut::Slider
{
public:

    TimeStepSlider( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::Slider( screen ),
        m_command( command )
    {};

    void sliderMoved();

private:
    Command* m_command;
};

class TimeSegmentMinSlider : public kvs::glut::Slider
{
public:

    TimeSegmentMinSlider( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::Slider( screen ),
        m_command( command ) {};

    void sliderMoved();

private:
    Command* m_command;
};

class TimeSegmentMaxSlider : public kvs::glut::Slider
{
public:

    TimeSegmentMaxSlider( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::Slider( screen ),
        m_command( command ) {};

    void sliderMoved();

private:
    Command* m_command;
};

class RandomBasedShuffleButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    RandomBasedShuffleButton( kvs::glut::Screen* screen, Command* command  ):
        kvs::glut::RadioButton( screen ),
        m_command( command ) {};

    void stateChanged();
};

class IndexBasedSingleShuffleButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    IndexBasedSingleShuffleButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command ) {};

    void stateChanged();
};

class IndexBasedCPUParallelShuffleButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    IndexBasedCPUParallelShuffleButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command ) {};

    void stateChanged();
};

class IndexBasedGPUParallelShuffleButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    IndexBasedGPUParallelShuffleButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command ) {};

    void stateChanged();
};

class SummalizedTransferButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    SummalizedTransferButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class DividedTransferButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    DividedTransferButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class UniformSamplingButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    UniformSamplingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class RejectionSamplingButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    RejectionSamplingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class MetropolisSamplingButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    MetropolisSamplingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class AllNodesButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    AllNodesButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class SlaveNodesButton : public kvs::glut::RadioButton
{
private:
    Command* m_command;

public:

    SlaveNodesButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};


    void stateChanged();
};

class MisesRenderingButton : public kvs::glut::RadioButton
{
public:

    MisesRenderingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};

    void stateChanged();

private:
    Command* m_command;
};

class UXRenderingButton : public kvs::glut::RadioButton
{
public:

    UXRenderingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};

    void stateChanged();

private:
    Command* m_command;
};

class UYRenderingButton : public kvs::glut::RadioButton
{
public:

    UYRenderingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};

    void stateChanged();

private:
    Command* m_command;
};

class UZRenderingButton : public kvs::glut::RadioButton
{
public:

    UZRenderingButton( kvs::glut::Screen* screen, Command* command ):
        kvs::glut::RadioButton( screen ),
        m_command( command )
    {};

    void stateChanged();

private:
    Command* m_command;
};

class Controller : public kvs::glut::WidgetBase
{
public:
    // MODIFIED START fp)m.tanaka 2013.09.01
//	Controller( kvs::glut::Screen* screen, Command* command );
    Controller( kvs::glut::Screen* screen, kvs::glut::Screen* screen3, Command* command );
    // MODIFIED END   fp)m.tanaka 2013.09.01

private:
    kvs::glut::Screen* m_screen;
// tantan
    kvs::glut::Screen* m_screen3;
    Command* m_command;

private:
    std::unique_ptr<StartAbstractAnalysisButton> m_abstract;
    std::unique_ptr<StopButton> m_stop;
    std::unique_ptr<ShowHideButton> m_show_hide;
    std::unique_ptr<kvs::glut::RadioButtonGroup> m_group1;
    std::unique_ptr<kvs::glut::RadioButtonGroup> m_group2;
    std::unique_ptr<RandomBasedShuffleButton> m_random_based;
    std::unique_ptr<IndexBasedSingleShuffleButton> m_index_based_single;
    std::unique_ptr<IndexBasedCPUParallelShuffleButton> m_index_based_cpu_parallel;
    std::unique_ptr<IndexBasedGPUParallelShuffleButton> m_index_based_gpu_parallel;
    std::unique_ptr<SummalizedTransferButton> m_summalized;
    std::unique_ptr<DividedTransferButton> m_divided;
    std::unique_ptr<kvs::glut::RadioButtonGroup> m_sampling_group;
    std::unique_ptr<UniformSamplingButton> m_uniform;
    std::unique_ptr<RejectionSamplingButton> m_rejection;
    std::unique_ptr<MetropolisSamplingButton> m_metropolis;
    std::unique_ptr<kvs::glut::RadioButtonGroup> m_node_group;
    std::unique_ptr<AllNodesButton> m_all_nodes;
    std::unique_ptr<SlaveNodesButton> m_slave_nodes;
    std::unique_ptr<kvs::glut::RadioButtonGroup> m_rendering_group;
    std::unique_ptr<MisesRenderingButton> m_mises_rendering_button;
    std::unique_ptr<UXRenderingButton> m_ux_rendering_button;
    std::unique_ptr<UYRenderingButton> m_uy_rendering_button;
    std::unique_ptr<UZRenderingButton> m_uz_rendering_button;

public:
    std::unique_ptr<StartDetailedAnalysisButton> m_detail;
};



}
}

#endif
