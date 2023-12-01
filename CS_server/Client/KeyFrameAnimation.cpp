#include <kvs/Message>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/glut/LegendBar>
#include <kvs/Date>
#include <kvs/Time>
#include <kvs/CommandLine>
#include <kvs/ParticleVolumeRenderer>
#include <kvs/KVSMLObjectPoint>
#include <kvs/PointExporter>
#include <kvs/PointImporter>
#include <stdio.h>
#include "TimerEvent.h"

extern kvs::visclient::TimerEvent* g_timer_event;
extern char ValueKeyFrameFile[256];
extern int ValueNumKeyFrames;
extern void RedrawAnimationControl();

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
    ValueNumKeyFrames = m_xforms.size();
    // RedrawAnimationControl();
}


// Add Xform
int KeyFrameAnimationAdd()
{
    g_timer_event->disableKeyFrameAnimation();
    kvs::PointObject* object = g_timer_event->getPointObject();
    if ( object )
    {
        kvs::Xform xform = object->xform();
        m_xforms.push_back( xform );
        m_time_steps.push_back( g_timer_event->getTimeStep() );
    }

    ValueNumKeyFrames = m_xforms.size();
    RedrawAnimationControl();
    return m_xforms.size();
}

// Animation start
void KeyFrameAnimationStart()
{
    g_timer_event->enableKeyFrameAnimation();
}

// Animation stop
void KeyFrameAnimationStop()
{
    g_timer_event->disableKeyFrameAnimation();
}

// Animation switch start/stop
void KeyFrameAnimationToggle()
{
    g_timer_event->toggleKeyFrameAnimation();
}

// delete all Xform
void KeyFrameAnimationDelete()
{
    m_xforms.clear();
    m_time_steps.clear();
    ValueNumKeyFrames = m_xforms.size();
    RedrawAnimationControl();
}

// delete last Xform
int KeyFrameAnimationErase()
{
    if ( m_xforms.size() > 1 )
    {
        m_xforms.pop_back();
        m_time_steps.pop_back();
    }

    ValueNumKeyFrames = m_xforms.size();
    RedrawAnimationControl();
    return m_xforms.size();
}

// write Xform data;
void KeyFrameAnimationWrite()
{
    FILE* fp = fopen( ValueKeyFrameFile, "w" );
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
        fprintf( stderr, " KeyFrameAnimation: Can't open file %s.\n", ValueKeyFrameFile );
    }
}

// read Xform data
int KeyFrameAnimationRead()
{
    g_timer_event->disableKeyFrameAnimation();
    FILE* fp = fopen( ValueKeyFrameFile, "r" );
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
        ValueNumKeyFrames = m_xforms.size();
        RedrawAnimationControl();
        // Animation start
        g_timer_event->enableKeyFrameAnimation();
    }
    else
    {
        fprintf( stderr, " KeyFrameAnimation: Can't open file %s.\n", ValueKeyFrameFile );
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

