#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <QOpenGLContext>

#include <kvs/Message>
#include "QGlue/renderarea.h"
#include "Panels/animationcontrols.h"
#include <QGlue/timer.h>
std::string ValueKeyFrameFile="./xform.dat";

static std::vector<kvs::Xform> m_xforms;
static std::vector<int> m_time_steps;

static size_t WriteFloat( FILE* fp, const float val )
{
    return fwrite( &val, sizeof( float ), 1, fp );
}

static size_t ReadFloat( FILE* fp, float* val )
{
    return fread( val, sizeof( float ), 1, fp );
}

static size_t WriteInt( FILE* fp, const int val )
{
    return fwrite( &val, sizeof( int ), 1, fp );
}

static size_t ReadInt( FILE* fp, int* val )
{
    return fread( val, sizeof( int ), 1, fp );
}

// Initialize Xform
void KeyFrameAnimationInit()
{
    m_xforms.clear();
    m_time_steps.clear();
    //    ValueNumKeyFrames = m_xforms.size();
    // RedrawAnimationControl();
}


// Add Xform
int KeyFrameAnimationAdd(kvs::Xform xform)
{
    qInfo(" $$$$ KeyFrameAnimationAdd()");
    RenderArea::g_timer_event->disableKeyFrameAnimation();

    m_xforms.push_back( xform );
    m_time_steps.push_back( RenderArea::g_timer_event->getTimeStep() );
    qInfo("   $ has object, num frames: %d", m_xforms.size());

    int ValueNumKeyFrames = m_xforms.size();
    AnimationControls::RedrawAnimationControl(ValueNumKeyFrames);
    return m_xforms.size();
}

// Animation start
void KeyFrameAnimationStart()
{
    RenderArea::g_timer_event->enableKeyFrameAnimation();
    extCommand->m_glut_timer->start();
}

// Animation stop
void KeyFrameAnimationStop()
{
    RenderArea::g_timer_event->disableKeyFrameAnimation();
    extCommand->m_glut_timer->stop();
}

// Animation switch start/stop
void KeyFrameAnimationToggle()
{
    static bool animation_active=false;
    animation_active=!animation_active;
    if (animation_active){
        KeyFrameAnimationStart();
    }
    else{
        KeyFrameAnimationStop();
    }
}

// delete all Xform
void KeyFrameAnimationDelete()
{
    m_xforms.clear();
    m_time_steps.clear();
    int ValueNumKeyFrames = m_xforms.size();
    AnimationControls::RedrawAnimationControl(ValueNumKeyFrames);
}

// delete last Xform
int KeyFrameAnimationErase()
{
    if ( m_xforms.size() > 1 )
    {

        m_xforms.pop_back();
        m_time_steps.pop_back();
        qInfo("   $ KeyFrameAnimationErase, num frames: %d", m_xforms.size());
    }

    int ValueNumKeyFrames = m_xforms.size();
    AnimationControls::RedrawAnimationControl(ValueNumKeyFrames);
    return m_xforms.size();
}

// write Xform data;
void KeyFrameAnimationWrite()
{
    std::cout<<"Opening xform file for write :"<<ValueKeyFrameFile<<std::endl;

    FILE* fp = fopen( ValueKeyFrameFile.c_str(), "w" );
    if ( fp )
    {
        for ( size_t i = 0; i < m_xforms.size(); i++ )
        {
            WriteInt( fp, m_time_steps[i] );
            for ( int j = 0; j < 3; j++ )
            {
                WriteFloat( fp, m_xforms[i].rotation()[j].x() );
                WriteFloat( fp, m_xforms[i].rotation()[j].y() );
                WriteFloat( fp, m_xforms[i].rotation()[j].z() );
            }
            WriteFloat( fp, m_xforms[i].translation().x() );
            WriteFloat( fp, m_xforms[i].translation().y() );
            WriteFloat( fp, m_xforms[i].translation().z() );
            WriteFloat( fp, m_xforms[i].scaling().x() );
            WriteFloat( fp, m_xforms[i].scaling().y() );
            WriteFloat( fp, m_xforms[i].scaling().z() );
        }
        fclose( fp );
    }
    else
    {
        fprintf( stderr, " KeyFrameAnimation: Can't open file %s.\n", ValueKeyFrameFile.c_str() );
    }
}

// read Xform data
int KeyFrameAnimationRead()
{
    std::cout<<"Opening xform file for read :"<<ValueKeyFrameFile<<std::endl;

    RenderArea::g_timer_event->disableKeyFrameAnimation();
    FILE* fp = fopen( ValueKeyFrameFile.c_str(), "r" );
    if ( fp )
    {
        m_xforms.clear();
        while ( 1 )
        {
            int   ival;
            float fval[3];
            if ( ReadInt( fp, &ival ) )
            {
                m_time_steps.push_back( ival );
                ReadFloat( fp, &fval[0] );
                ReadFloat( fp, &fval[1] );
                ReadFloat( fp, &fval[2] );
                kvs::Vector3<float> vec0 = kvs::Vector3<float>( fval );
                ReadFloat( fp, &fval[0] );
                ReadFloat( fp, &fval[1] );
                ReadFloat( fp, &fval[2] );
                kvs::Vector3<float> vec1 = kvs::Vector3<float>( fval );
                ReadFloat( fp, &fval[0] );
                ReadFloat( fp, &fval[1] );
                ReadFloat( fp, &fval[2] );
                kvs::Vector3<float> vec2 = kvs::Vector3<float>( fval );
                kvs::Matrix33f rotation = kvs::Matrix33f( vec0, vec1, vec2 );
                ReadFloat( fp, &fval[0] );
                ReadFloat( fp, &fval[1] );
                ReadFloat( fp, &fval[2] );
                kvs::Vector3<float> translation = kvs::Vector3<float>( fval );
                ReadFloat( fp, &fval[0] );
                ReadFloat( fp, &fval[1] );
                ReadFloat( fp, &fval[2] );
                kvs::Vector3<float> scaling = kvs::Vector3<float>( fval );
                kvs::Xform xform = kvs::Xform( translation, scaling, rotation );

                m_xforms.push_back( xform );

            }
            else
            {
                fclose( fp );
                break;
            }
        }
        int ValueNumKeyFrames = m_xforms.size();
        AnimationControls::RedrawAnimationControl(ValueNumKeyFrames);
        // Animation start
        RenderArea::g_timer_event->enableKeyFrameAnimation();
    }
    else
    {
        fprintf( stderr, " KeyFrameAnimation: Can't open file %s.\n", ValueKeyFrameFile.c_str() );
        return -1;
    }

    return 0;
}

// get vector of Xforms
std::vector<kvs::Xform>* KeyFrameAnimationGetXforms()
{
    return &m_xforms;
}

// get vector of timestep
std::vector<int>* KeyFrameAnimationGetTimeSteps()
{
    return &m_time_steps;
}

