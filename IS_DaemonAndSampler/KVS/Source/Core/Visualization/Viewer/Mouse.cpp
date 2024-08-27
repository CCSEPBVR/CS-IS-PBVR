/****************************************************************************/
/**
 *  @file Mouse.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Mouse.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Mouse.h"


namespace kvs
{

int   Mouse::SpinTime       = 10;
float Mouse::WheelUpValue   = 1.05f;
float Mouse::WheelDownValue = 0.95f;

/*==========================================================================*/
/**
 *  Constructor.
 *  @param auto_flg [in] auto mode flag
 */
/*==========================================================================*/
Mouse::Mouse( AutoMode auto_flg ) :
    m_mode( Mouse::Rotation ),
    m_scaling_type( ScalingXYZ ),
    m_old( Vector2i(0,0) ),
    m_new( Vector2i(0,0) ),
    m_start( Vector2i(0,0) ),
    m_stop( Vector2i(0,0) ),
    m_timer(),
    m_is_auto( false ),
    m_is_slow( false ),
    m_is_use_auto( auto_flg == Mouse::AutoOn )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Mouse::~Mouse( void )
{
}

/*==========================================================================*/
/**
 *  @brief Reset parameters.
 */
/*==========================================================================*/
void Mouse::reset( void )
{
    kvs::Trackball::reset();
    m_mode         = kvs::Mouse::Rotation;
    m_scaling_type = kvs::Trackball::ScalingXYZ;
    m_old          = kvs::Vector2i(0,0);
    m_new          = kvs::Vector2i(0,0);
    m_start        = kvs::Vector2i(0,0);
    m_stop         = kvs::Vector2i(0,0);
    m_is_auto      = false;
    m_is_slow      = false;
}

/*==========================================================================*/
/**
 *  Press event function.
 *  @param x [in] x coordinate value of the mouse pointer position
 *  @param y [in] y coordinate value of the mouse pointer position
 */
/*==========================================================================*/
void Mouse::press( const int x, const int y )
{
    m_old   = kvs::Vector2i( x, y );
    m_new   = kvs::Vector2i( x, y );
    m_start = kvs::Vector2i( x, y );

    m_timer.start();
    m_is_auto = false;
    m_is_slow = false;
}

/*==========================================================================*/
/**
 *  Move event function.
 *  @param x [in] x coordinate value of the mouse pointer position
 *  @param y [in] y coordinate value of the mouse pointer position
 */
/*==========================================================================*/
void Mouse::move( const int x, const int y )
{
    m_new = kvs::Vector2i( x, y );

    switch( m_mode )
    {
    case Mouse::Scaling:
        scale( m_old, m_new, m_scaling_type );
        break;
    case Mouse::Rotation:
        rotate( m_old, m_new );
        break;
    case Mouse::Translation:
        translate( m_old, m_new );
        break;
    default:
        break;
    }

    // Update the mouse cursor position.
    m_old = m_new;
}

/*==========================================================================*/
/**
 *  Wheel event function.
 *  @param value [in] scaling ratio
 */
/*==========================================================================*/
void Mouse::wheel( const float value )
{
    kvs::Vector3f scale( 1.0 );

    switch( scalingType() )
    {
    case ScalingXYZ: scale     *= value; break;
    case ScalingX:   scale.x() *= value; break;
    case ScalingY:   scale.y() *= value; break;
    case ScalingZ:   scale.z() *= value; break;
    case ScalingXY:  scale.x() *= value; scale.y() *= value; break;
    case ScalingYZ:  scale.y() *= value; scale.z() *= value; break;
    case ScalingZX:  scale.z() *= value; scale.x() *= value; break;
    default: break;
    }

    BaseClass::setScaling( scale );

    m_old = m_new;
}

/*==========================================================================*/
/**
 *  Release event function.
 *  @param x [in] x coordinate value of the mouse pointer position
 *  @param y [in] y coordinate value of the mouse pointer position
 */
/*==========================================================================*/
void Mouse::release( const int x, const int y )
{
    m_timer.stop();
    m_stop = kvs::Vector2i( x, y );

    const double threshould_time    = 500.0;
    const double tolerance_auto_len = 2.0;
    const double tolerance_slow_len = 80.0;

    const double length = ( m_start - m_stop ).length();

    if( m_timer.msec() < threshould_time )
    {
        if( length > tolerance_auto_len )
        {
            m_is_auto = true;
            m_is_slow = length < tolerance_slow_len;
        }
    }
}

/*==========================================================================*/
/**
 *  Idle event function.
 */
/*==========================================================================*/
bool Mouse::idle( void )
{
    if( !( m_is_use_auto && m_is_auto ) ) return( false );

    if( m_is_slow )
    {
        switch( m_mode )
        {
        case Mouse::Scaling:
        {
            const float big_scale   = 1.0003f;
            const float small_scale = 0.9997f;
            switch( scalingType() )
            {

            case ScalingXYZ:
            {
                if( BaseClass::m_scaling.x() < 1.0f )
                {
                    BaseClass::m_scaling *= big_scale;
                    if( BaseClass::m_scaling.x() > 1.0f )
                    {
                        BaseClass::m_scaling = kvs::Vector3f( 1.0f );
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.x() > 1.0f )
                {
                    BaseClass::m_scaling *= small_scale;
                    if( BaseClass::m_scaling.x() < 1.0f )
                    {
                        BaseClass::m_scaling = kvs::Vector3f( 1.0f );
                        m_is_slow = false;
                    }
                }
                break;
            }

            case ScalingX:
            {
                if( BaseClass::m_scaling.x() < 1.0f )
                {
                    BaseClass::m_scaling.x() *= big_scale;
                    if( BaseClass::m_scaling.x() > 1.0f )
                    {
                        BaseClass::m_scaling.x() = 1.0f;
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.x() > 1.0f )
                {
                    BaseClass::m_scaling.x() *= small_scale;
                    if( BaseClass::m_scaling.x() < 1.0f )
                    {
                        BaseClass::m_scaling.x() = 1.0f;
                        m_is_slow = false;
                    }
                }
                break;
            }

            case ScalingY:
            {
                if( BaseClass::m_scaling.y() < 1.0f )
                {
                    BaseClass::m_scaling.y() *= big_scale;
                    if( BaseClass::m_scaling.y() > 1.0f )
                    {
                        BaseClass::m_scaling.y() = 1.0f;
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.y() > 1.0f )
                {
                    BaseClass::m_scaling.y() *= small_scale;
                    if( BaseClass::m_scaling.y() < 1.0f )
                    {
                        BaseClass::m_scaling.y() = 1.0f;
                        m_is_slow = false;
                    }
                }
                break;
            }

            case ScalingZ:
            {
                if( BaseClass::m_scaling.z() < 1.0f )
                {
                    BaseClass::m_scaling.z() *= big_scale;
                    if( BaseClass::m_scaling.z() > 1.0f )
                    {
                        BaseClass::m_scaling.z() = 1.0f;
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.z() > 1.0f )
                {
                    BaseClass::m_scaling.z() *= small_scale;
                    if( BaseClass::m_scaling.z() < 1.0f )
                    {
                        BaseClass::m_scaling.z() = 1.0f;
                        m_is_slow = false;
                    }
                }
                break;
            }

            case ScalingXY:
            {
                if( BaseClass::m_scaling.x() < 1.0f )
                {
                    BaseClass::m_scaling.x() *= big_scale;
                    BaseClass::m_scaling.y() *= big_scale;
                    if( BaseClass::m_scaling.x() > 1.0f )
                    {
                        BaseClass::m_scaling.x() = 1.0f;
                        BaseClass::m_scaling.y() = 1.0f;
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.x() > 1.0f )
                {
                    BaseClass::m_scaling.x() *= small_scale;
                    BaseClass::m_scaling.y() *= small_scale;
                    if( BaseClass::m_scaling.x() < 1.0f )
                    {
                        BaseClass::m_scaling.x() = 1.0f;
                        BaseClass::m_scaling.y() = 1.0f;
                        m_is_slow = false;
                    }
                }
                break;
            }

            case ScalingYZ:
            {
                if( BaseClass::m_scaling.y() < 1.0f )
                {
                    BaseClass::m_scaling.y() *= big_scale;
                    BaseClass::m_scaling.z() *= big_scale;
                    if( BaseClass::m_scaling.y() > 1.0f )
                    {
                        BaseClass::m_scaling.y() = 1.0f;
                        BaseClass::m_scaling.z() = 1.0f;
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.y() > 1.0f )
                {
                    BaseClass::m_scaling.y() *= small_scale;
                    BaseClass::m_scaling.z() *= small_scale;
                    if( BaseClass::m_scaling.y() < 1.0f )
                    {
                        BaseClass::m_scaling.y() = 1.0f;
                        BaseClass::m_scaling.z() = 1.0f;
                        m_is_slow = false;
                    }
                }
                break;
            }

            case ScalingZX:
            {
                if( BaseClass::m_scaling.z() < 1.0f )
                {
                    BaseClass::m_scaling.z() *= big_scale;
                    BaseClass::m_scaling.x() *= big_scale;
                    if( BaseClass::m_scaling.z() > 1.0f )
                    {
                        BaseClass::m_scaling.z() = 1.0f;
                        BaseClass::m_scaling.x() = 1.0f;
                        m_is_slow = false;
                    }
                }
                else if( BaseClass::m_scaling.z() > 1.0f )
                {
                    BaseClass::m_scaling.z() *= small_scale;
                    BaseClass::m_scaling.x() *= small_scale;
                    if( BaseClass::m_scaling.z() < 1.0f )
                    {
                        BaseClass::m_scaling.z() = 1.0f;
                        BaseClass::m_scaling.x() = 1.0f;
                        m_is_slow = false;
                    }
                }
                break;
            }

            default: break;
            }
            break;
        } // Mouse::Scaling

        case Mouse::Rotation:
        {
            BaseClass::m_rotation.x() *= 0.9f;
            BaseClass::m_rotation.y() *= 0.9f;
            BaseClass::m_rotation.z() *= 0.9f;
            break;
        }

        case Mouse::Translation:
        {
            BaseClass::m_translation *= 0.9f;
            if( BaseClass::m_translation.length() < 0.001 )
            {
                BaseClass::m_translation = kvs::Vector3f( 0.0f );
                m_is_slow = false;
            };
            break;
        }
        default:
            break;
        }
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Set the translation mode.
 *  @param mode [in] translation mode
 */
/*==========================================================================*/
void Mouse::setMode( const Mouse::TransMode mode )
{
    m_mode = mode;
}

/*==========================================================================*/
/**
 *  Set the scaling type.
 *  @param type [in] scaling type
 */
/*==========================================================================*/
void Mouse::setScalingType( const Trackball::ScalingType type )
{
    m_scaling_type = type;
}

/*==========================================================================*/
/**
 *  Set the scaling type.
 *  @param x [in] x axis flag
 *  @param y [in] y axis flag
 *  @param z [in] z axis flag
 */
/*==========================================================================*/
void Mouse::setScalingType( const bool x, const bool y, const bool z )
{
    m_scaling_type = x ?
        ( y ? ( z ? ScalingXYZ : ScalingXY ) : ( z ? ScalingZX : ScalingX   ) ) :
        ( y ? ( z ? ScalingYZ  : ScalingY  ) : ( z ? ScalingZ  : ScalingNot ) );
}

/*==========================================================================*/
/**
 *  Get the translation mode.
 *  @return translation mode
 */
/*==========================================================================*/
const Mouse::TransMode Mouse::mode( void ) const
{
    return( m_mode );
}

/*==========================================================================*/
/**
 *  Get the scaling type.
 *  @return scaling type
 */
/*==========================================================================*/
const Trackball::ScalingType Mouse::scalingType( void ) const
{
    return( m_scaling_type );
}

/*==========================================================================*/
/**
 *  Set use-auto-rotation mode.
 *  @param flg [in] auto-rotation mode flag
 */
/*==========================================================================*/
void Mouse::setUseAuto( const bool flg )
{
    m_is_use_auto = flg;
}

/*==========================================================================*/
/**
 *  Test whether the auto-rotation mode is enable or not.
 *  @return true, if the auto-rotation mode is enable.
 */
/*==========================================================================*/
bool Mouse::isUseAuto( void )
{
    return( m_is_use_auto );
}

/*==========================================================================*/
/**
 *  Test whether the current mode is auto-rotation mode or not.
 *  @return true, if the current mode is auto-rotation mode.
 */
/*==========================================================================*/
bool Mouse::isAuto( void )
{
    return( m_is_auto );
}

/*==========================================================================*/
/**
 *  Test whether the current mode is slowdown-rotation mode or not.
 *  @return true, if the current mode is slowdown-rotation mode.
 */
/*==========================================================================*/
bool Mouse::isSlow( void )
{
    return( m_is_slow );
}

} // end of namespace kvs
