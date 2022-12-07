#include "kvs/glut/OrientationAxis"
#include "TimerEvent.h"
#include "Command.h"
#include "sighandler.h"
#include "KeyFrameAnimation.h"
#include "ParticleMerger.h"
#include "ParticleServer.h"
#include "ExtendedParticleVolumeRenderer.h"
#include "Controller.h"
#include "TransferFunctionEditorMain.h"

void UpdateSending();
void UpdateRecieving();
void UpdateGPUMemory();
static kvs::Xform InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end );
static float Sign( const float x );
static float Norm( const float a, const float b, const float c, const float d );
static kvs::Quaternion<float> RtoQ( const kvs::Matrix33f& R );
extern kvs::glut::OrientationAxis* g_orientation_axis;
extern int ValueInterpolation;
extern      void  ScreenShotKeyFrame( kvs::ScreenBase* screen, const int tstep );

#if 1 // FEAST modify 20160128
extern int g_curStep;
#endif // FEAST modify 20160128 end


using namespace kvs::visclient;

TimerEvent::TimerEvent( Command* command, ComThread* comthread ):
    m_command( command ),
    m_comthread( comthread )
{
    m_front_object = NULL;
    m_is_key_frame_animation = 0;
    m_is_ready = 0;
    m_interpolation_counter = 0;
    m_ninterpolation = 10;
    m_xform_index = 0;
    KeyFrameAnimationInit();
    m_xforms = KeyFrameAnimationGetXforms();
    m_time_steps = KeyFrameAnimationGetTimeSteps();
}

#ifdef CPUMODE
void TimerEvent::setObject( const int& id )
{
    m_object_id = id;
}

void TimerEvent::setRenderer( kvs::ParticleVolumeRenderer* renderer )
{
    m_renderer = renderer;
}
TimerEvent::~TimerEvent()
{
    if ( m_back_object  != NULL ) delete m_back_object;
    if ( m_front_object != NULL ) delete m_front_object;
}
#endif

void TimerEvent::update( kvs::TimeEvent* event )
{
    bool view_flag = false;
    const ComThread::ComStatus comstatus = m_comthread->getStatus();
    if ( comstatus == ComThread::Idle )
    {
        m_command->preUpdate();
        m_comthread->start();
    }
    else if ( comstatus == ComThread::Exit )
    {
        m_comthread->wait();
        m_comthread->quit();
        m_command->postUpdate();
#ifdef CPUMODE

        if ( m_command->m_is_under_animation && m_command->m_particle_assign_flag )
        {
            m_front_object = new PointObject();
            if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Abstract )
            {
                *m_front_object = *( m_command->m_abstract_particles[m_command->m_parameter.m_time_step] );
                m_renderer->setSubpixelLevel( m_command->m_parameter.m_abstract_subpixel_level );
            }
            else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
            {
                *m_front_object = *( m_command->m_detailed_particles[m_command->m_parameter.m_time_step] );
                m_renderer->setSubpixelLevel( m_command->m_parameter.m_detailed_subpixel_level );
            }
            else
            {
                assert( false );
            }
            if ( m_command->m_parameter.m_shading_type_flag )
                m_renderer->enableShading();
            m_renderer->attachPointObject( m_front_object );
            m_back_object = m_front_object;
            screen()->objectManager()->change( m_object_id, m_front_object );
            g_orientation_axis->setObject( m_front_object );
            view_flag = true;
            m_command->m_particle_assign_flag = false;
        }
#endif
        if ( m_command->m_parameter.m_time_step == m_command->m_parameter.m_time_step_key_frame )
        {
            m_is_ready = 1;
        }

#if 1 // FEAST modify 20160128
        if ( m_command->m_is_under_communication && view_flag )
        {
            if ( m_command-> m_parameter.m_transfer_type == VisualizationParameter::Abstract )
            {
                m_command->m_progress->setValue( static_cast<float>( m_command->m_parameter.m_time_step ) );
                m_command->m_time_slider->setValue( static_cast<float>( m_command->m_parameter.m_time_step ) );
            }
            else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
            {
                if ( m_command->m_parameter.m_time_step > m_command->m_parameter.m_max_time_step )
                {
                    m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
                }
              //float ratio = float( m_command->m_parameter.m_time_step - m_command->m_parameter.m_min_time_step ) / float( m_command->m_parameter.m_max_time_step - m_command->m_parameter.m_min_time_step ) * 100;
                int min_time_step = static_cast<int>( m_command->m_min_time_slider->value() );
                int max_time_step = static_cast<int>( m_command->m_max_time_slider->value() );
                float ratio = float( m_command->m_parameter.m_time_step - min_time_step + 1.0 ) / float( max_time_step - min_time_step + 1.0 ) * 100;
                m_command->m_progress->setValue( static_cast<float>( ratio ) );
                m_command->m_time_slider->setValue( static_cast<float>( m_command->m_parameter.m_time_step ) );
            }
        }
#endif // FEAST modify 20160128 end


#if 1 // FEAST modify 20160128
        if ( m_command->m_is_under_animation && view_flag )
        {
            if ( g_curStep < 0 && m_command->m_parameter.m_client_server_mode == 1 )
            {
                m_command->m_is_under_animation     = false;
                m_command->m_is_under_communication = false;
            }
            if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
            {
#ifndef CPUMODE
                if ( command->param.m_repeat_level == command->param.m_detailed_repeat_level )
                {
#endif
                    g_curStep = m_command->m_parameter.m_time_step;
                    m_command->m_parameter.m_time_step++;
#ifndef CPUMODE
                }
#endif
            }
            else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Abstract )
            {
#ifndef CPUMODE
                if ( command->param.m_repeat_level == command->param.m_abstract_repeat_level )
                {
#endif
                    g_curStep = m_command->m_parameter.m_time_step;
                    m_command->m_parameter.m_time_step++;
#ifndef CPUMODE
                }
#endif
            }
        }
        if ( m_command->m_parameter.m_time_step > m_command->m_max_time_slider->value() ) //99) {
        {
            if ( m_command->m_min_time_slider->value() >= m_command->m_parameter.m_min_time_step )
            {
                m_command->m_parameter.m_time_step = m_command->m_min_time_slider->value();
            }
            else
            {
                m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
            }
        }
        // ユーザの、粒子の表示・非表示操作によりカレントtimeが急にminTimeより小さくなった場合の対処
        if ( m_command->m_parameter.m_time_step < m_command->m_min_time_slider->value() ) //99) {
        {
            if ( m_command->m_min_time_slider->value() >= m_command->m_parameter.m_min_time_step )
            {
                m_command->m_parameter.m_time_step = m_command->m_min_time_slider->value();
            }
            else
            {
                m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
            }
        }

#endif // FEAST modify 20160128 end

    }
    else if ( comstatus == ComThread::Running )
    {
        const ParticleServer::Status srvstat = m_command->m_server->getStatus();
        if ( srvstat == ParticleServer::Sending )   UpdateSending();
        if ( srvstat == ParticleServer::Recieving ) UpdateRecieving();
        UpdateGPUMemory();
    }

    if ( m_is_key_frame_animation )
    {
        m_ninterpolation = ValueInterpolation;
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
                if ( m_front_object && m_is_ready )
                {
                    if ( m_interpolation_counter < m_ninterpolation )
                    {
                        kvs::Xform Xform_new = InterpolateXform( t, m_ninterpolation, m_xforms->at( i ), m_xforms->at( i + 1 ) );
                        m_front_object->setXform( Xform_new );
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
                if ( m_front_object && m_is_ready )
                {
                    if ( m_interpolation_counter < m_ninterpolation )
                    {
                        kvs::Xform Xform_new = InterpolateXform( t, m_ninterpolation, m_xforms->at( i ), m_xforms->at( i + 1 ) );
                        m_front_object->setXform( Xform_new );
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
    }

#ifdef CPUMODE
    screen()->redraw();
#endif
    m_command->reDraw();

    if ( m_is_key_frame_animation && m_command->m_previous_key_frame != m_command->m_step_key_frame )
    {
        ScreenShotKeyFrame( m_command->m_screen, m_command->m_step_key_frame );
        m_command->m_previous_key_frame = m_command->m_step_key_frame;
    }

    if ( IsSignal() )
    {
        if ( m_command->m_parameter.m_client_server_mode == 1 )
        {
            m_command->closeServer();
        }
        exit( 1 );
    }

}

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

    kvs::Quaternion<float> q_0 = RtoQ( rotation_0 );
    kvs::Quaternion<float> q_1 = RtoQ( rotation_1 );
    kvs::Quaternion<float> q =
        kvs::Quaternion<float>::sphericalLinearInterpolation( q_0, q_1, t, true, true );
    kvs::Matrix33f rotation = q.toMatrix();

    kvs::Vector3f translation = translation_1 * t + translation_0 * ( 1 - t );

    kvs::Vector3f scaling = scaling_1 * t + scaling_0 * ( 1 - t );

    kvs::Xform xform( translation, scaling, rotation );
    return xform;
}

static float Sign( const float x )
{
    return ( x >= 0.0f ) ? +1.0f : -1.0f;
}

static float Norm( const float a, const float b, const float c, const float d )
{
    return sqrt( a * a + b * b + c * c + d * d );
}
//High precision conversion of Rotation Matrix to Quaternion
static kvs::Quaternion<float> RtoQ( const kvs::Matrix33f& R )
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

    return kvs::Quaternion<float>( q1, q2, q3, q0 );
}
