/****************************************************************************/
/**
 *  @file Mouse.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Mouse.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MOUSE_H_INCLUDE
#define VIS_MODULE__MOUSE_H_INCLUDE

#include <vismodule/Trackball>
#include <vismodule/OpenGL>
#include <vismodule/ClassName>
#include <vismodule/Timer>
#include <vismodule/Vector2>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Mouse class
 */
/*==========================================================================*/
class Mouse : public vismodule::Trackball
{
    typedef vismodule::Trackball BaseClass;

    visModuleClassName( vismodule::Mouse );

public:

    enum TransMode
    {
        Rotation    = 0, ///< rotation mode
        Translation = 1, ///< translation mode
        Scaling     = 2  ///< scaling mode
    };

    enum AutoMode
    {
        AutoOff = 0, ///< auto rotation mode off
        AutoOn  = 1  ///< auto rotation mode on
    };

    static int   SpinTime;       ///< spin time
    static float WheelUpValue;   ///< wheel up value
    static float WheelDownValue; ///< wheel down value

protected:

    TransMode     m_mode;         ///< transform mode
    ScalingType   m_scaling_type; ///< scaling type
    vismodule::Vector2i m_old;          ///< old mouse position on the window coordinate (org: upper-left)
    vismodule::Vector2i m_new;          ///< new mouse position on the window coordinate (org: upper-left)
    vismodule::Vector2i m_start;        ///< position at start of rotation
    vismodule::Vector2i m_stop;         ///< position at stop of rotation
    vismodule::Timer    m_timer;        ///< auto check timer
    bool          m_is_auto;      ///< auto flag (true: if auto mode)
    bool          m_is_slow;      ///< slow flag (true: if slow mode)
    bool          m_is_use_auto;  ///< auto use flag (true: if user use auto mode )

public:

    Mouse( AutoMode auto_flg = AutoOn );

    virtual ~Mouse( void );

public:

    void reset( void );

    void press( const int x, const int y );

    void move( const int x, const int y );

    void wheel( const float value );

    void release( const int x, const int y );

    bool idle( void );

public:

    void setMode( const TransMode mode );

    void setScalingType( const ScalingType type );

    void setScalingType( const bool x, const bool y, const bool z );

    const TransMode mode( void ) const;

    const ScalingType scalingType( void ) const;

public:

    void setUseAuto( const bool flg );

    bool isUseAuto( void );

    bool isAuto( void );

    bool isSlow( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__MOUSE_H_INCLUDE
