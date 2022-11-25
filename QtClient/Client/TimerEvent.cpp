//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <QOpenGLContext>
#include <QGlue/renderarea.h>
#include "TimerEvent.h"
#include "Command.h"
//#include "sighandler.h"
#include "KeyFrameAnimation.h"
#include "ParticleMerger.h"
#include "ParticleServer.h"
//#include "PBRProxy.h"

#include "Panels/animationcontrols.h"
#include "Panels/particlepanel.h"
#include "Panels/coordinatepanel.h"
#include "Panels/timecontrolpanel.h"
#include "Panels/systemstatuspanel.h"

#include <QMutex>
//KVS2.7.0
//ADD BY)T.Osaki 2020.06.02
#include <kvs/Quaternion>
#include <kvs/ObjectManager>
extern QMutex paint_mutex;

static kvs::Xform InterpolateXform( const int interp_step, const int num_frame, const kvs::Xform& start, const kvs::Xform& end );
static float Sign( const float x );
static float Norm( const float a, const float b, const float c, const float d );
//KVS2.7.0
//MOD BY)T.Osaki 2020.06.04
//static kvs::Quaternion<float> RtoQ( const kvs::Matrix33f& R );
static kvs::Quaternion RtoQ( const kvs::Matrix33f& R );

using namespace kvs::visclient;

TimerEvent::TimerEvent( Command* command, ComThread* comthread ):
    m_command( command ),
    m_comthread( comthread )
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

void TimerEvent::update( kvs::TimeEvent* event )
{
    qInfo ("  ## TimerEvent::update starts ##");
    paint_mutex.lock();

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

        if ( m_command->m_is_under_animation && m_command->m_particle_assign_flag )
        {
           kvs::PointObject* m_front_object = new PointObject();
           int sp_level=1;
           if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
            {
               //全タイムステップの配列確保廃止に伴う変更
//                *m_front_object = *( m_command->m_detailed_particles[m_command->m_parameter.m_time_step] );
               *m_front_object = *( m_command->m_detailed_particles[m_command->m_detailed_particles.size()-1] );
//                sp_level= m_command->m_parameter.m_detailed_subpixel_level;
               sp_level = m_command->m_detailed_particles[m_command->m_detailed_particles.size()-1]->size();
            }
            else
            {
                assert( false );
            }
            if ( m_command->m_parameter.m_shading_type_flag )
                m_screen->enableRendererShading();

            kvs::Xform before_Xform;
            if(     m_front_object->numberOfVertices() != 0 &&
                    m_front_object->numberOfColors() != 0 &&
                    m_front_object->numberOfNormals() != 0){
                before_Xform = kvs::Xform(m_screen->scene()->objectManager()->xform());
            }

            m_screen->attachPointObject(m_front_object,sp_level);
            delete m_front_object;
            m_screen->scene()->objectManager()->translate(before_Xform.translation());
            m_screen->scene()->objectManager()->scale(before_Xform.scaling());
            m_screen->scene()->objectManager()->rotate(before_Xform.rotation());
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
#ifdef CS_MODE
                m_command->m_timectrl_panel->setProgress(m_command->m_parameter.m_time_step);
                m_command->m_timectrl_panel->setStepValue(m_command->m_parameter.m_time_step);
#endif
            }
            else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Detailed )
            {
                if ( m_command->m_parameter.m_time_step > m_command->m_parameter.m_max_time_step )
                {
                    m_command->m_parameter.m_time_step = m_command->m_parameter.m_min_time_step;
                }
              //float ratio = float(m_command->m_parameter.m_time_step -m_command->m_parameter.m_min_time_step ) / float(m_command->m_parameter.m_max_time_step -m_command->m_parameter.m_min_time_step ) * 100;
                int min_time_step = static_cast<int>( m_command->m_timectrl_panel->minValue() );
                int max_time_step = static_cast<int>( m_command->m_timectrl_panel->maxValue() );
                float ratio = float( m_command->m_parameter.m_time_step - min_time_step + 1.0 ) / float( max_time_step - min_time_step + 1.0 ) * 100;
#ifdef CS_MODE
                // Unlocking the paint_mutex here , as m_timectrl_panel->setProgress triggers an update event.
                // And if the timecntrl_panel is docked in the same window as the render area, this
                // will lead to a chain of events where finally Screen::paintGL() is called. As Screen::paintGL tries
                // to get the paint_mutex.lock this will cause a hang if we don't unlock here first.
                //
                // In fact - maybe paint_mutex is not needed here any more at all.

                paint_mutex.unlock();
                m_command->m_timectrl_panel->setProgress(ratio);
                m_command->m_timectrl_panel->setStepValue(m_command->m_parameter.m_time_step);
                paint_mutex.lock();
#endif
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
                    #ifdef CS_MODE
                    m_command->m_parameter.m_time_step++;
					#endif
            }
            else if ( m_command->m_parameter.m_transfer_type == VisualizationParameter::Abstract )
            {
                    TimecontrolPanel::g_curStep = m_command->m_parameter.m_time_step;
                    #ifdef CS_MODE
                    m_command->m_parameter.m_time_step++;
					#endif
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
			#ifdef CS_MODE
                m_command->m_parameter.m_min_time_step = m_command->m_timectrl_panel->minValue();
            #else
                m_command->m_parameter.m_min_time_step = m_command->m_timectrl_panel->minValue();
			#endif
            }
            else
            {
			#ifdef CS_MODE
                m_command->m_parameter.m_min_time_step = m_command->m_parameter.m_min_time_step;
			#else
               m_command->m_parameter.m_min_time_step =m_command->m_parameter.m_min_time_step;
			#endif
            }
        }

#endif // FEAST modify 20160128 end
#ifdef IS_MODE
        if( m_command->lastStepCheckBoxState )
        {
           m_command->m_parameter.m_time_step =m_command->m_parameter.m_max_time_step;
//            m_command->m_timectrl_panel->setStepValue(m_command->m_parameter.timeStep);
        }
#endif
    }
    else if ( comstatus == ComThread::Running )
    {
//        const ParticleServer::Status srvstat = m_command->m_server->getStatus();
//        if ( srvstat == ParticleServer::Sending )   UpdateSending();
//        if ( srvstat == ParticleServer::Recieving ) UpdateRecieving();
//        UpdateGPUMemory();
        SystemstatusPanel::updateSystemStatus(-1);
    }

    if ( m_is_key_frame_animation )
    {
        m_ninterpolation = AnimationControls::getInterpolationValue();
        if ( m_interpolation_counter > m_ninterpolation )
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

            if ( m_command->m_parameter.m_client_server_mode == 1 || m_command->m_parameter.m_client_server_mode == 0)
            {
                if ( m_command->m_parameter.m_time_step != m_command->m_parameter.m_time_step_key_frame )
                {
                    m_is_ready = 0;
                }
                if ( m_is_ready )
                {
                    if ( m_interpolation_counter <= m_ninterpolation )
                    {
                        kvs::Xform Xform_new = InterpolateXform( t, m_ninterpolation, m_xforms->at( i ), m_xforms->at( i + 1 ) );
                        //2020,11,27 sceneが持つObjectManagerのtranslate,scale,rotateに補完された値が入っているXform_newを渡す。
//                        m_front_object->setXform( Xform_new );
                        m_screen->scene()->reset();
                        m_screen->scene()->objectManager()->translate(Xform_new.translation());
                        m_screen->scene()->objectManager()->scale(Xform_new.scaling());
                        m_screen->scene()->objectManager()->rotate(Xform_new.rotation());

//                        m_screen->setPointObjectXform(Xform_new);
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
                        //2020,11,27 sceneが持つObjectManagerのtranslate,scale,rotateに補完された値が入っているXform_newを渡す。
//                        m_front_object->setXform( Xform_new );
                        m_screen->scene()->reset();
                        m_screen->scene()->objectManager()->translate(Xform_new.translation());
                        m_screen->scene()->objectManager()->scale(Xform_new.scaling());
                        m_screen->scene()->objectManager()->rotate(Xform_new.rotation());
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
//            this->disableKeyFrameAnimation();
            KeyFrameAnimationStop();

        }
    }

    m_screen->update();
    paint_mutex.unlock();
    if ( m_is_key_frame_animation && m_command->m_previous_key_frame != m_command->m_step_key_frame )
    {
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        RenderArea::ScreenShotKeyFrame( m_command->m_screen->scene(), m_command->m_step_key_frame );
        m_command->m_previous_key_frame = m_command->m_step_key_frame;
    }

//    if ( IsSignal() ) // Letting Qt handle this
//    {
//        if (m_command->m_parameter.m_client_server_mode == 1 )
//        {
//            m_command->closeServer();
//        }
//        exit( 1 );
//    }
//    paint_mutex.unlock();
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

    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    //kvs::Quaternion<float> q_0 = RtoQ( rotation_0 );
    //kvs::Quaternion<float> q_1 = RtoQ( rotation_1 );
    //kvs::Quaternion<float> q =
    //      kvs::Quaternion<float>::sphericalLinearInterpolation( q_0, q_1, t, true, true );

    kvs::Quaternion q_0 = RtoQ( rotation_0 );
    kvs::Quaternion q_1 = RtoQ( rotation_1 );
    kvs::Quaternion q =
            kvs::Quaternion::SphericalLinearInterpolation( q_0, q_1, t, true, true );

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
//KVS2.7.0
//MOD BY)T.Osaki 2020.06.04
//static kvs::Quaternion<float> RtoQ( const kvs::Matrix33f& R )
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
