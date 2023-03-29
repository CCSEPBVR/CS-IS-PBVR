//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08

#include <kvs/OpenGL>

#include <QOpenGLContext>

#include "QGlue/timer.h"
#include <kvs/DebugNew>
#include <kvs/IgnoreUnusedVariable>
#include <QDebug>

#include <Client/KeyFrameAnimation.h>
#include "Panels/animationcontrols.h"
#include "Panels/particlepanel.h"
#include "Panels/coordinatepanel.h"
#include "Panels/timecontrolpanel.h"
#include "Panels/systemstatuspanel.h"

#include "QGlue/renderarea.h"
#include "Client/ExtendedParticleVolumeRenderer.h"
//KVS2.7.0
//MOD BY)T.Osaki 2020.06.04
#include <kvs/Quaternion>

static kvs::Xform InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end );
namespace  QGlue
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Timer class.
 *  @param  msec [in] interval time in milliseconds
 */
/*===========================================================================*/
Timer::Timer( int msec ):
    m_id( 0 ),
    m_interval( msec ),
    m_is_stopped( true ),
    m_time_event( new kvs::TimeEvent ),
    m_event_listener( 0 ),
    m_event_handler( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Timer class.
 *  @param  msec [in] interval time in milliseconds
 */
/*===========================================================================*/
Timer::Timer( int msec , ExtCommand* command ):
    m_id( 0 ),
    m_interval( msec ),
    m_is_stopped( true ),
    m_time_event( new kvs::TimeEvent ),
    m_event_listener( 0 ),
    m_event_handler( 0 ),
    m_command( command ),
    m_comthread( &command->comthread ),
    m_screen(command->m_screen)
{
    m_is_key_frame_animation = 0;
    m_is_ready = 0;
    m_interpolation_counter = 0;
    m_ninterpolation = 10;
    m_xform_index = 0;
    KeyFrameAnimationInit();
    m_xforms = KeyFrameAnimationGetXforms();
    m_time_steps = KeyFrameAnimationGetTimeSteps();
}

///*===========================================================================*/
///**
// *  @brief  Constructs a new Timer class.
// *  @param  listener [in] pointer to a event listener
// *  @param  msec [in] interval time in milliseconds
// */
///*===========================================================================*/
//Timer::Timer( kvs::EventListener* listener, int msec ):
//    m_id( 0 ),
//    m_interval( msec ),
//    m_is_stopped( true ),
//    m_time_event( new kvs::TimeEvent ),
//    m_event_listener( listener ),
//    m_event_handler( 0 )
//{
//}

///*===========================================================================*/
///**
// *  @brief  Constructs a new Timer class.
// *  @param  handler [in] pointer to a event handler
// *  @param  msec [in] interval time in milliseconds
// */
///*===========================================================================*/
//Timer::Timer( kvs::EventHandler* handler, int msec ):
//    m_id( 0 ),
//    m_interval( msec ),
//    m_is_stopped( true ),
//    m_time_event( new kvs::TimeEvent ),
//    m_event_listener( 0 ),
//    m_event_handler( handler )
//{
//}

/*===========================================================================*/
/**
 *  @brief  Destructs the Timer class.
 */
/*===========================================================================*/
Timer::~Timer( void )
{
    if ( m_time_event ) delete m_time_event;
}

/*===========================================================================*/
/**
 *  @brief  Starts the timer.
 *  @param  msec [in] interval time in milliseconds
 */
/*===========================================================================*/
void Timer::start( int msec )
{
    if ( m_is_stopped )
    {
        m_interval = msec;
        m_is_stopped = false;
        QTimer::start( msec );
    }
}

/*===========================================================================*/
/**
 *  @brief  Starts the timer.
 */
/*===========================================================================*/
void Timer::start( void )
{
    this->start( m_interval );
}

///*===========================================================================*/
/**
 *  @brief  Stops the timer.
 */
/*===========================================================================*/
void Timer::stop( void )
{
    if ( !m_is_stopped )
    {
        m_is_stopped = true;
    }
    extCommand->m_is_under_communication = false;
    QTimer::stop();
}

/*===========================================================================*/
/**
 *  @brief  Sets a interval time.
 *  @param  msec [in] interval time in milliseconds
 */
/*===========================================================================*/
void Timer::setInterval( int msec )
{
    m_interval = msec;
}

/*===========================================================================*/
/**
 *  @brief  Sets an event listener.
 *  @param  listener [in] pointer to the event listener
 */
/*===========================================================================*/
void Timer::setEventListener( kvs::visclient::TimerEvent* listener )
{
    m_event_listener = listener;
}

///*===========================================================================*/
///**
// *  @brief  Sets an event handler.
// *  @param  handler [in] pointer to the event handler
// */
///*===========================================================================*/
//void Timer::setEventHandler( kvs::EventHandler* handler )
//{
//    m_event_handler = handler;
//}

/*===========================================================================*/
/**
 *  @brief  Checks whether the timer is stopped or not.
 *  @return true, if the timer is stopped
 */
/*===========================================================================*/
const bool Timer::isStopped( void ) const
{
    return( m_is_stopped );
}

/*===========================================================================*/
/**
 *  @brief  Returns the interval time.
 *  @return interval time
 */
/*===========================================================================*/
const int Timer::interval( void ) const
{
    return( m_interval );
}
/*===========================================================================*/
/**
 *  @brief  Com Thread Idle event.
 */
/*===========================================================================*/
void Timer::comThreadIdleEvent()
{
    //    qInfo(" ## QGLUETimer::Timer::comThreadIdleEvent START ###");
    m_command->preUpdate();
    m_comthread->start();
    //    qInfo(" ## QGLUETimer::Timer::comThreadIdleEvent END ###");
}

/*===========================================================================*/
/**
 *  @brief  Com Thread Exit event.
 */
/*===========================================================================*/
void Timer::comThreadExitEvent()
{
    //    qInfo(" ## QGLUETimer::Timer::comThreadExitEvent start ###");
    m_comthread->wait();
    m_comthread->quit();
    m_command->postUpdate();

    bool view_flag = false;

    if ( m_command->m_is_under_animation && m_command->m_particle_assign_flag )
    {
        kvs::PointObject* m_front_object = new kvs::PointObject();
        int sp_level=1;
        if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
            *m_front_object = *( m_command->m_detailed_particles[m_command->m_parameter.m_time_step] );
            sp_level= m_command->m_parameter.m_detailed_subpixel_level;
        }
        else
        {
            assert( false );
        }

        if ( m_command->m_parameter.m_shading_type_flag )
            m_screen->enableRendererShading();

        m_screen->attachPointObject( m_front_object,sp_level );

        delete m_front_object;

//        kvs::PointObject* m_front_object = new kvs::PointObject();
//        if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Abstract )
//        {
//            *m_front_object = *( m_command->m_abstract_particles[m_command->m_parameter.m_time_step] );
//            m_screen->setRenderSubPixelLevel(m_command->m_parameter.m_abstract_subpixel_level );
//        }
//        else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
//        {
//            *m_front_object = *( m_command->m_detailed_particles[m_command->m_parameter.m_time_step] );
//            m_screen->setRenderSubPixelLevel( m_command->m_parameter.m_detailed_subpixel_level );
//        }
//        else
//        {
//            assert( false );
//        }

//        if ( m_command->m_parameter.m_shading_type_flag )
//            m_screen->enableRendererShading();
//        m_screen->attachPointObject( m_front_object );

//        delete m_front_object;
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
//        m_screen->scene()->objectManager()->change( m_object_id, m_front_object );

        //        qInfo("Timer::comThreadExitEvent not updating m_orientation_axis");
        //        m_command->m_screen->m_orientation_axis->setObject( m_front_object );

        view_flag = true;

        m_command->m_particle_assign_flag = false;
    }

    if ( m_command->m_parameter.m_time_step == m_command->m_parameter.m_time_step_key_frame )
    {
        m_is_ready = 1;
    }

#if 1 // FEAST modify 20160128
    if ( m_command->m_is_under_communication && view_flag )
    {
        if ( m_command-> m_parameter.m_transfer_type == VisualizationParameter::Abstract )
        {
            m_command->m_timectrl_panel->setProgress(m_command->m_parameter.m_time_step);
            m_command->m_timectrl_panel->setStepValue(m_command->m_parameter.m_time_step);
            //                m_command->m_progress->setValue( static_cast<float>( m_command->param.m_time_step ) );
            //                m_command->m_time_slider->setValue( static_cast<float>( m_command->param.m_time_step ) );
        }
        else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
            if ( m_command->m_parameter.m_time_step > m_command->m_parameter.m_max_time_step )
            {
                m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
            }
            //float ratio = float( m_command->param.m_time_step - m_command->param.m_min_time_step ) / float( m_command->param.m_max_time_step - m_command->param.m_min_time_step ) * 100;
            int min_time_step = static_cast<int>( m_command->m_timectrl_panel->minValue() );
            int max_time_step = static_cast<int>( m_command->m_timectrl_panel->maxValue() );
            float ratio = float( m_command->m_parameter.m_time_step - min_time_step + 1.0 ) / float( max_time_step - min_time_step + 1.0 ) * 100;
            m_command->m_timectrl_panel->setProgress(ratio);
            m_command->m_timectrl_panel->setStepValue(m_command->m_parameter.m_time_step);
            //              m_command->m_progress->setValue( static_cast<float>( ratio ) );
            //              m_command->m_time_slider->setValue( static_cast<float>( m_command->param.m_time_step ) );
        }
    }
#endif // FEAST modify 20160128 end


#if 1 // FEAST modify 20160128
    if ( m_command->m_is_under_animation && view_flag )
    {
        if ( TimecontrolPanel::g_curStep < 0 && m_command->m_parameter.m_client_server_mode == 1 )
        {
            m_command->m_is_under_animation     = false;
            m_command->m_is_under_communication = false;
        }
        if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
        {
                TimecontrolPanel::g_curStep = m_command->m_parameter.m_time_step;
                m_command->m_parameter.m_time_step++;
        }
        else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Abstract )
        {
                TimecontrolPanel::g_curStep = m_command->m_parameter.m_time_step;
                m_command->m_parameter.m_time_step++;
        }
    }
    if ( m_command->m_parameter.m_time_step > m_command->m_timectrl_panel->maxValue() ) //99) {
    {
        if ( m_command->m_timectrl_panel->minValue() >= m_command->m_parameter.m_min_time_step )
        {
            m_command->m_parameter.m_time_step = m_command->m_timectrl_panel->minValue();
        }
        else
        {
            m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
        }
    }
    // ユーザの、粒子の表示・非表示操作によりカレントtimeが急にminTimeより小さくなった場合の対処
    if ( m_command->m_timectrl_panel->minValue() < m_command->m_timectrl_panel->stepValue()) //99) {
    {
        if ( m_command->m_timectrl_panel->minValue() >= m_command->m_parameter.m_min_time_step )
        {
            m_command->m_parameter.m_time_step = m_command->m_timectrl_panel->minValue();
        }
        else
        {
            m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
        }
    }

#endif // FEAST modify 20160128 end

    //    qInfo(" ## QGLUETimer::Timer::comThreadExitEvent stp ###");
}

/*===========================================================================*/
/**
 *  @brief  Timer animation.
 */
/*===========================================================================*/
void Timer::animate()
{
    //    qInfo(" ## QGLUETimer::Timer::animate start ###");
    m_ninterpolation = AnimationControls::getInterpolationValue();
    if ( m_interpolation_counter >= m_ninterpolation )
    {
        m_interpolation_counter = 0;
        m_xform_index++;
    }
    if ( m_xform_index < ( int )m_xforms->size() - 1 )
    {
        int i = m_xform_index;
        int t = m_interpolation_counter;
        float delta = ( float )( m_time_steps->at( i + 1 ) - m_time_steps->at( i ) ) / ( float )m_ninterpolation;
        float f = delta * ( float )t;
        m_command->m_parameter.m_time_step_key_frame = ( int )( ( float )( m_time_steps->at( i ) ) + f );

        if ( m_command->m_parameter.m_client_server_mode == 1 )
        {
            if ( m_command->m_parameter.m_time_step != m_command->m_parameter.m_time_step_key_frame )
            {
                m_is_ready = 0;
            }
            if ( m_is_ready )
            {
                if ( m_interpolation_counter < m_ninterpolation )
                {
                    kvs::Xform Xform_new = InterpolateXform( t, m_ninterpolation, m_xforms->at( i ), m_xforms->at( i + 1 ) );
                    m_screen->setPointObjectXform(Xform_new);
                    m_command->m_step_key_frame++;
                    t++;
                    m_interpolation_counter = t;
                }
            }
        }
        else
        {
            if ( m_command->m_parameter.m_time_step != m_command->m_parameter.m_time_step_key_frame )
            {
                m_is_ready = 0;
            }
            if ( m_is_ready )
            {
                if ( m_interpolation_counter < m_ninterpolation )
                {
                    kvs::Xform Xform_new = InterpolateXform( t, m_ninterpolation, m_xforms->at( i ), m_xforms->at( i + 1 ) );
                    m_screen->setPointObjectXform(Xform_new);
                    m_command->m_step_key_frame++;
                    t++;
                    m_interpolation_counter = t;
                }
            }
        }
    }
    else
    {
        m_xform_index = 0;
        m_interpolation_counter = 0;
        m_is_key_frame_animation = 0;
        m_command->m_is_key_frame_animation = false;
        m_command->m_step_key_frame = 0;
        m_command->m_previous_key_frame = 0;
    }
    //    qInfo(" ## QGLUETimer::Timer::animate start ###");
}

/*===========================================================================*/
/**
 *  @brief  Timer event.
 */
/*===========================================================================*/
void Timer::timerEvent( QTimerEvent* event )
{
    //    qInfo("### QGLUETimer::timerEvent start ###");
    const ComThread::ComStatus comstatus = m_comthread->getStatus();
    if ( !m_is_stopped )
    {
        switch (comstatus){
        case ComThread::Idle:
            //comThreadIdleEvent();
//            m_event_listener->onEvent( m_time_event );
            m_event_listener->update(m_time_event);
            break;
        case ComThread::Exit:
            //            comThreadExitEvent();
//            m_event_listener->onEvent( m_time_event );
            m_event_listener->update(m_time_event);
            if(oneShot){stop(); oneShot=false; qInfo("ENDING SINGLESHOT");}
            break;
        case ComThread::Running:
            // if ( srvstat == ParticleServer::Recieving ) UpdateRecieving();
            break;
        default:
            qInfo("### QGLUETimer::timerEvent NOOP ###\n ");
            exit(1);
        }
        if (m_is_key_frame_animation){

        }
    }
    else
    {
        //        qInfo(" ## EventHandler::notify STOP ##");
        this->stop();
    }
}

} // end of namesapce QGLUE

/// kvs::stuff
static float Sign( const float x )
{
    return ( x >= 0.0f ) ? +1.0f : -1.0f;
}

static float Norm( const float a, const float b, const float c, const float d )
{
    return sqrt( a * a + b * b + c * c + d * d );
}

//High precision conversion of Rotation Matrix to Quaternion
//KVS2.7.0
//MOD BY)T.Osaki 2020.06.04
//static kvs::Quaternion<float> RtoQ( const kvs::Matrix33f& R )
/*===========================================================================*/
/**
 * @brief RtoQ  Transform-Matrix to Quaternion conversion.
 * @param R     Transform-Matrix
 * @return kvs::Quaternion representing transform.
 */
/*===========================================================================*/
static kvs::Quaternion RtoQ( const kvs::Matrix33f& R )
{
    float r11 = R[0][0];
    float r12 = R[0][1];
    float r13 = R[0][2];
    float r21 = R[1][0];
    float r22 = R[1][1];
    float r23 = R[1][2];
    float r31 = R[2][0];
    float r32 = R[2][1];
    float r33 = R[2][2];

    float q0 = ( r11 + r22 + r33 + 1.0f ) / 4.0f;
    float q1 = ( r11 - r22 - r33 + 1.0f ) / 4.0f;
    float q2 = ( -r11 + r22 - r33 + 1.0f ) / 4.0f;
    float q3 = ( -r11 - r22 + r33 + 1.0f ) / 4.0f;
    if ( q0 < 0.0f ) q0 = 0.0f;
    if ( q1 < 0.0f ) q1 = 0.0f;
    if ( q2 < 0.0f ) q2 = 0.0f;
    if ( q3 < 0.0f ) q3 = 0.0f;
    q0 = sqrt( q0 );
    q1 = sqrt( q1 );
    q2 = sqrt( q2 );
    q3 = sqrt( q3 );
    if ( q0 >= q1 && q0 >= q2 && q0 >= q3 )
    {
        q0 *= +1.0f;
        q1 *= Sign( r32 - r23 );
        q2 *= Sign( r13 - r31 );
        q3 *= Sign( r21 - r12 );
    }
    else if ( q1 >= q0 && q1 >= q2 && q1 >= q3 )
    {
        q0 *= Sign( r32 - r23 );
        q1 *= +1.0f;
        q2 *= Sign( r21 + r12 );
        q3 *= Sign( r13 + r31 );
    }
    else if ( q2 >= q0 && q2 >= q1 && q2 >= q3 )
    {
        q0 *= Sign( r13 - r31 );
        q1 *= Sign( r21 + r12 );
        q2 *= +1.0f;
        q3 *= Sign( r32 + r23 );
    }
    else if ( q3 >= q0 && q3 >= q1 && q3 >= q2 )
    {
        q0 *= Sign( r21 - r12 );
        q1 *= Sign( r31 + r13 );
        q2 *= Sign( r32 + r23 );
        q3 *= +1.0f;
    }
    else
    {
        printf( "coding error\n" );
    }
    float r = Norm( q0, q1, q2, q3 );
    q0 /= r;
    q1 /= r;
    q2 /= r;
    q3 /= r;

    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    //return kvs::Quaternion<float>( q1, q2, q3, q0 );
    return kvs::Quaternion( q1, q2, q3, q0 );
}
/*===========================================================================*/
/**
 * @brief InterpolateXform Interpolate Xform between start and end based on
 *        number of frames and interpolation step.
 *
 * @param interp_step   Current interpolation step
 * @param num_frame     Number of frames
 * @param start         start XForm
 * @param end           end XForm
 * @return kvs::Xform   the interpolated XForm
 */
/*===========================================================================*/
static kvs::Xform InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end )
{
    // range of the interpolation parametar t = [0,1].
    float delta = 1.0 / ( float )num_frame;
    float t = delta * ( float )interp_step;

    kvs::Matrix33f rotation_0( start.rotation() );
    kvs::Vector3f translation_0( start.translation() );
    kvs::Vector3f scaling_0( start.scaling() );

    kvs::Matrix33f rotation_1( end.rotation() );
    kvs::Vector3f translation_1( end.translation() );
    kvs::Vector3f scaling_1( end.scaling() );

    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    //    kvs::Quaternion<float> q_0 = RtoQ( rotation_0 );
    //    kvs::Quaternion<float> q_1 = RtoQ( rotation_1 );
    //    kvs::Quaternion<float> q =
    //            kvs::Quaternion<float>::sphericalLinearInterpolation( q_0, q_1, t, true, true );
        kvs::Quaternion q_0 = RtoQ( rotation_0 );
        kvs::Quaternion q_1 = RtoQ( rotation_1 );
        kvs::Quaternion q =
                kvs::Quaternion::sphericalLinearInterpolation( q_0, q_1, t, true, true );
    kvs::Matrix33f rotation = q.toMatrix();

    kvs::Vector3f translation = translation_1 * t + translation_0 * ( 1 - t );

    kvs::Vector3f scaling = scaling_1 * t + scaling_0 * ( 1 - t );

    kvs::Xform xform( translation, scaling, rotation );
    return xform;
}
