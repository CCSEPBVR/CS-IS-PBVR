/****************************************************************************/
/**
 *  @file Trackball.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Trackball.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TRACKBALL_H_INCLUDE
#define VIS_MODULE__TRACKBALL_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Vector2>
#include <vismodule/Vector3>
#include <vismodule/Quaternion>


namespace vismodule
{

class Camera;

/*==========================================================================*/
/**
 *  Trackball class
 */
/*==========================================================================*/
class Trackball
{
    visModuleClassName( vismodule::Trackball );

public:

    enum ScalingType
    {
        ScalingXYZ = 0,
        ScalingX,
        ScalingY,
        ScalingZ,
        ScalingXY,
        ScalingYZ,
        ScalingZX,
        ScalingNot,
        ScalingTypeSize
    };

protected:

    float                  m_size;            ///< trackball size
    float                  m_depth;           ///< how near from center
    vismodule::Vector2f          m_rotation_center; ///< center of rotation in the devise coordinate system
    vismodule::Vector3f          m_scaling;         ///< current scaling value
    vismodule::Vector3f          m_translation;     ///< current translation vector
    vismodule::Quaternion<float> m_rotation;        ///< current rotation quaternion
    int                    m_window_width;    ///< window width
    int                    m_window_height;   ///< window height
    vismodule::Camera*           m_ref_camera;      ///< pointer to camera (reference only)

public:

    Trackball( void );

    virtual ~Trackball( void );

public:

    void attachCamera( vismodule::Camera* camera );

    void resetRotationCenter( void );

    void setTrackballSize( const float size );

    void setDepthValue( const float depth );

    void setRotationCenter( const vismodule::Vector2f& center );

    void setScaling( const vismodule::Vector3f& scaling );

    void setTranslation( const vismodule::Vector3f& translation );

    void setRotation( const vismodule::Quaternion<float>& rotation );

    void setWindowSize( const int w, const int h );

    const float size( void ) const;

    const float depthValue( void ) const;

    const vismodule::Vector2f& rotationCenter( void ) const;

    const vismodule::Vector3f& scaling( void ) const;

    const vismodule::Vector3f& translation( void ) const;

    const vismodule::Quaternion<float>& rotation( void ) const;

    const int windowWidth( void ) const;

    const int windowHeight( void ) const;

public:

    void scale( const vismodule::Vector2i& start, const vismodule::Vector2i& end, ScalingType type = ScalingXYZ );

    void rotate( const vismodule::Vector2i& start, const vismodule::Vector2i& end );

    void translate( const vismodule::Vector2i& start, const vismodule::Vector2i& end );

protected:

    void reset( void );

protected:

    const float depth_on_sphere( const vismodule::Vector2f& dir ) const;

    const vismodule::Vector2f get_norm_position( const vismodule::Vector2i& pos ) const;

public:

    static void x_scaling( const vismodule::Vector2f& start, const vismodule::Vector2f& end, Trackball* track );
    static void y_scaling( const vismodule::Vector2f& start, const vismodule::Vector2f& end, Trackball* track );
    static void z_scaling( const vismodule::Vector2f& start, const vismodule::Vector2f& end, Trackball* track );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TRACKBALL_H_INCLUDE
