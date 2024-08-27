#include "Controller.h"

#include "Command.h"

#if 1 // FEAST modify 20160128
extern int g_curStep;
#endif // FEAST modify 20160128 end

using namespace kvs::visclient;

void StartDetailedAnalysisButton::pressed()
{
    setButtonColor( ::Red );
    m_command->startDetailedAnalysis();
}

void StartAbstractAnalysisButton::pressed()
{
    setButtonColor( ::Red );
    m_command->startAbstractAnalysis();
}

void ShowHideButton::pressed()
{
    m_command->changeShowHide();
}

void StopButton::pressed()
{
    if ( g_curStep >= 0 )
    {
        //command->m_is_under_animation ? setButtonColor( ::Gray ) : setButtonColor( ::Red );
        // ボタンの色を反転
        m_command->m_is_under_animation ? setButtonColor( ::Red ) : setButtonColor( ::Gray );
        // 内部変数を反転
        m_command->m_is_under_animation = !m_command->m_is_under_animation;
        m_command->m_is_under_communication = !m_command->m_is_under_communication;
    }
}

void TimeStepSlider::sliderMoved()
{
    m_command->m_parameter.m_time_step = static_cast<size_t>( this->value() );
    // FEAST modify 20160128
    g_curStep = m_command->m_parameter.m_time_step;
}

void TimeSegmentMinSlider::sliderMoved()
{
    m_command->m_parameter.m_min_time_step = static_cast<int>( this->value() );
    if ( m_command->m_max_time_slider->value() < this->value() + 1.0f )
    {
        if ( this->value() + 1.0f > m_command->m_max_time_slider->maxValue() )
        {
            m_command->m_max_time_slider->setValue( m_command->m_max_time_slider->maxValue() );
        }
        else
        {
            m_command->m_max_time_slider->setValue( this->value() + 1.0f );
        }
    }
}

void TimeSegmentMaxSlider::sliderMoved()
{
    m_command->m_parameter.m_max_time_step = static_cast<int>( this->value() );
    if ( m_command->m_min_time_slider->value() > this->value() - 1.0f )
    {
        if ( this->value() - 1.0f < m_command->m_min_time_slider->minValue() )
        {
            m_command->m_min_time_slider->setValue( m_command->m_min_time_slider->minValue() );
        }
        else
        {
            m_command->m_min_time_slider->setValue( this->value() - 1.0f );
        }
    }
}

void RandomBasedShuffleButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_shuffle_type = VisualizationParameter::RandomBased;
    }
}

void IndexBasedSingleShuffleButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_shuffle_type = VisualizationParameter::SingleShuffleBased;
    }
}

void IndexBasedCPUParallelShuffleButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_shuffle_type = VisualizationParameter::CPUShuffleBased;
    }
}

void IndexBasedGPUParallelShuffleButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_shuffle_type = VisualizationParameter::GPUShffleBased;
    }
}

void SummalizedTransferButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_detailed_transfer_type = VisualizationParameter::Summalized;
    }
}

void DividedTransferButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_detailed_transfer_type = VisualizationParameter::Divided;
    }
}

void UniformSamplingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_sampling_type = VisualizationParameter::UniformSampling;
    }
}

void RejectionSamplingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_sampling_type = VisualizationParameter::RejectionSampling;
    }
}

void MetropolisSamplingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_sampling_type = VisualizationParameter::MetropolisSampling;
    }
}

void AllNodesButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_node_type = VisualizationParameter::AllNodes;
    }
}

void SlaveNodesButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_node_type = VisualizationParameter::SlaveNodes;
    }
}

void MisesRenderingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_rendering_id = 0;
    }
}

void UXRenderingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_rendering_id = 1;
    }
}

void UYRenderingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_rendering_id = 2;
    }
}

void UZRenderingButton::stateChanged()
{
    if ( this->state() )
    {
        m_command->m_parameter.m_rendering_id = 3;
    }
}

// MODIFIED START fp)m.tanaka 2013.09.01
//Controller::Controller( kvs::glut::Screen* screen, Command* command ) :
//	kvs::glut::WidgetBase(screen),
//		screen(screen),
//		command(command)
//{
Controller::Controller( kvs::glut::Screen* screen, kvs::glut::Screen* screen3, Command* command ) :
    kvs::glut::WidgetBase( screen ),
    m_screen( screen ),
    m_command( command )
{
// MODIFIED END   fp)m.tanaka 2013.09.01

//	screen->background()->setColor( kvs::RGBColor( 255, 0, 0 ));
// MODIFIED START fp)m.tanaka 2013.09.01
    //detail.reset( new StartDetailedAnalysisButton( screen, command ) );
    m_detail.reset( new StartDetailedAnalysisButton( screen3, command ) );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_detail->setButtonColor( ::Gray );
    m_detail->setTextColor( kvs::RGBColor( 255, 255, 255 ) );
    //detail->setButtonColor( kvs::RGBColor( 255, 0, 0 ));
// MODIFIED START fp)m.tanaka 2013.09.01
    //detail->setX( 0 );
    //detail->setY( 0 );
    m_detail->setX( 0 );
    m_detail->setY( 230 );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_detail->setWidth( 250 );
    m_detail->setMargin( 10 );
    m_detail->setCaption( "Detailed analysis" );
//	detail->show(); /* removed 2014.07.20 */

// MODIFIED START fp)m.tanaka 2013.09.01
    //abst.reset( new StartAbstractAnalysisButton( screen, command ) );
    m_abstract.reset( new StartAbstractAnalysisButton( screen3, command ) );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_abstract->setButtonColor( ::Gray );
    m_abstract->setTextColor( kvs::RGBColor( 255, 255, 255 ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //abst->setX( 0 );
    //abst->setY( detail->y() + detail->height() );
    m_abstract->setX( 245 );
    m_abstract->setY( 230 );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_abstract->setWidth( m_detail->width() );
    m_abstract->setMargin( 10 );
    m_abstract->setCaption( "Abstract analysis" );
//	abst->show(); /* removed 2014.07.20 */

    m_stop.reset( new StopButton( screen, command ) );
    m_stop->setButtonColor( ::Gray );
    m_stop->setTextColor( kvs::RGBColor( 255, 255, 255 ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    m_stop->setX( 0 );
//	stop->setY( abst->y() + abst->height() );
    m_stop->setY( 20 );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_stop->setWidth( m_abstract->width() );
    m_stop->setMargin( 10 );
    m_stop->setCaption( "Start/Stop" );
    m_stop->show();

    m_group1.reset( new kvs::glut::RadioButtonGroup() );

    m_random_based.reset( new RandomBasedShuffleButton( screen, command ) );
    m_random_based->setX( 0 );
    m_random_based->setY( m_stop->y() + m_stop->height() );
    m_random_based->setMargin( 10 );
    m_random_based->setCaption( "RandomBased" );

    m_index_based_single.reset( new IndexBasedSingleShuffleButton( screen, command ) );
    m_index_based_single->setX( 0 );
    m_index_based_single->setY( m_random_based->y() + 20 );
    m_index_based_single->setMargin( 10 );
    m_index_based_single->setCaption( "IndexBased(Single)" );

    m_index_based_cpu_parallel.reset( new IndexBasedCPUParallelShuffleButton( screen, command ) );
    m_index_based_cpu_parallel->setX( 0 );
    m_index_based_cpu_parallel->setY( m_index_based_single->y() + 20 );
    m_index_based_cpu_parallel->setMargin( 10 );
    m_index_based_cpu_parallel->setCaption( "IndexBased(CPU Parallel)" );

    m_index_based_gpu_parallel.reset( new IndexBasedGPUParallelShuffleButton( screen, command ) );
    m_index_based_gpu_parallel->setX( 0 );
    m_index_based_gpu_parallel->setY( m_index_based_cpu_parallel->y() + 20 );
    m_index_based_gpu_parallel->setMargin( 10 );
    m_index_based_gpu_parallel->setCaption( "IndexBased(GPU Parallel)" );

    m_group1->add( m_random_based.get() );
    m_group1->add( m_index_based_single.get() );
    m_group1->add( m_index_based_cpu_parallel.get() );
    m_group1->add( m_index_based_gpu_parallel.get() );

// DELETE START fp)m.tanaka 2013.09.01
    //m_group1->show();
// DELETE END   fp)m.tanaka 2013.09.01

    m_group2.reset( new kvs::glut::RadioButtonGroup() );

    const kvs::RGBAColor transferColor( 0, 122, 255 );

    m_summalized.reset( new SummalizedTransferButton( screen, command ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_summalized->setX( 255 );
    //m_summalized->setY( m_random_based->y() );
    int rd_offset_x = 0;
    m_summalized->setX( rd_offset_x );
    m_summalized->setY( m_random_based->y() + 20 );
// MODIFIED END   fp)m.tanaka 2013.09.01

    m_summalized->setMargin( 10 );
    m_summalized->setCaption( "Summalized Transfer" );
    m_summalized->setTextColor( transferColor );

    m_divided.reset( new DividedTransferButton( screen, command  ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_divided->setX( 255 );
    m_divided->setX( rd_offset_x );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_divided->setY( m_summalized->y() + 20 );
    m_divided->setMargin( 10 );
    m_divided->setCaption( "Divided Transfer" );
    m_divided->setTextColor( transferColor );

    m_group2->add( m_summalized.get() );
    m_group2->add( m_divided.get() );
    m_group2->show();

    m_sampling_group.reset( new kvs::glut::RadioButtonGroup() );

    const kvs::RGBAColor samplingColor( 122, 122, 122 );

    m_uniform.reset( new UniformSamplingButton( screen, command ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_uniform->setX( 255 );
    m_uniform->setX( rd_offset_x );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_uniform->setY( m_divided->y() + 20 );
    m_uniform->setMargin( 10 );
    m_uniform->setCaption( "Uniform Sampling" );
    m_uniform->setTextColor( samplingColor );

    m_rejection.reset( new RejectionSamplingButton( screen, command ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_rejection->setX( 255 );
    m_rejection->setX( rd_offset_x );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_rejection->setY( m_uniform->y() + 20 );
    m_rejection->setMargin( 10 );
    m_rejection->setCaption( "Rejection Sampling" );
    m_rejection->setTextColor( samplingColor );

    m_metropolis.reset( new MetropolisSamplingButton( screen, command ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_metropolis->setX( 255 );
    m_metropolis->setX( rd_offset_x );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_metropolis->setY( m_rejection->y() + 20 );
    m_metropolis->setMargin( 10 );
    m_metropolis->setCaption( "Metropolis Sampling" );
    m_metropolis->setTextColor( samplingColor );

    m_sampling_group->add( m_uniform.get() );
    m_sampling_group->add( m_rejection.get() );
    m_sampling_group->add( m_metropolis.get() );
    m_sampling_group->show();

    m_show_hide.reset( new ShowHideButton( screen, command ) );
    m_show_hide->setX( m_metropolis->x() );
    m_show_hide->setY( m_stop->y() );
    m_show_hide->setWidth( 250 );
    m_show_hide->setCaption( "Show/Hide" );
    m_show_hide->setMargin( 10 );
    m_show_hide->setButtonColor( ::Gray );
    m_show_hide->setTextColor( kvs::RGBColor( 255, 255, 255 ) );

// DELETE START fp)m.tanaka 2013.09.01
    //m_show_hide->show();
// DELETE END   fp)m.tanaka 2013.09.01

    m_node_group.reset( new kvs::glut::RadioButtonGroup() );

    kvs::RGBColor nodeColor( 255, 122, 255 );

    m_all_nodes.reset( new AllNodesButton( screen, command ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_all_nodes->setX( m_metropolis->x() );
    m_all_nodes->setX( rd_offset_x );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_all_nodes->setY( m_metropolis->y() + 20 );
    m_all_nodes->setMargin( 10 );
    m_all_nodes->setCaption( "All Nodes" );
    m_all_nodes->setTextColor( nodeColor );

    m_slave_nodes.reset( new SlaveNodesButton( screen, command ) );
// MODIFIED START fp)m.tanaka 2013.09.01
    //m_slave_nodes->setX( m_all_nodes->x() );
    m_slave_nodes->setX( rd_offset_x );
// MODIFIED END   fp)m.tanaka 2013.09.01
    m_slave_nodes->setY( m_all_nodes->y() + 20 );
    m_slave_nodes->setMargin( 10 );
    m_slave_nodes->setCaption( "Slave Nodes" );
    m_slave_nodes->setTextColor( nodeColor );

    m_node_group->add( m_all_nodes.get() );
    m_node_group->add( m_slave_nodes.get() );
//	m_node_group->show(); /* removed 2014.07.20 */

    m_rendering_group.reset( new kvs::glut::RadioButtonGroup() );

    m_mises_rendering_button.reset( new MisesRenderingButton( screen, command ) );
    m_mises_rendering_button->setX( 250 );
    m_mises_rendering_button->setY( 0 );
    m_mises_rendering_button->setCaption( "Mises Stress" );
    m_mises_rendering_button->setMargin( 10 );

    m_ux_rendering_button.reset( new UXRenderingButton( screen, command ) );
    m_ux_rendering_button->setX( 250 );
    m_ux_rendering_button->setY( 20 );
    m_ux_rendering_button->setCaption( "Ux" );
    m_ux_rendering_button->setMargin( 10 );

    m_uy_rendering_button.reset( new UYRenderingButton( screen, command ) );
    m_uy_rendering_button->setX( 250 );
    m_uy_rendering_button->setY( 40 );
    m_uy_rendering_button->setCaption( "Uy" );
    m_uy_rendering_button->setMargin( 10 );

    m_uz_rendering_button.reset( new UZRenderingButton( screen, command ) );
    m_uz_rendering_button->setX( 250 );
    m_uz_rendering_button->setY( 60 );
    m_uz_rendering_button->setCaption( "Uz" );
    m_uz_rendering_button->setMargin( 10 );

    m_rendering_group->add( m_mises_rendering_button.get() );
    m_rendering_group->add( m_ux_rendering_button.get() );
    m_rendering_group->add( m_uy_rendering_button.get() );
    m_rendering_group->add( m_uz_rendering_button.get() );

    // DELETE START fp)m.tanaka 2013.09.01
    //m_rendering_group->show();
    // DELETE END   fp)m.tanaka 2013.09.01
};
