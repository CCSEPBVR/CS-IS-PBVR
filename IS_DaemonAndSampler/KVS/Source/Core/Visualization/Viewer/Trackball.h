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
#ifndef KVS__TRACKBALL_H_INCLUDE
#define KVS__TRACKBALL_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Quaternion>


namespace kvs
{

class Camera;

/*==========================================================================*/
/**
 *  Trackball class
 */
/*==========================================================================*/
class Trackball
{
    kvsClassName( kvs::Trackball );

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
    kvs::Vector2f          m_rotation_center; ///< center of rotation in the devise coordinate system
    kvs::Vector3f          m_scaling;         ///< current scaling value
    kvs::Vector3f          m_translation;     ///< current translation vector
    kvs::Quaternion<float> m_rotation;        ///< current rotation quaternion
    int                    m_window_width;    ///< window width
    int                    m_window_height;   ///< window height
    kvs::Camera*           m_ref_camera;      ///< pointer to camera (reference only)

public:

    Trackball( void );

    virtual ~Trackball( void );

public:

    void attachCamera( kvs::Camera* camera );

    void resetRotationCenter( void );

    void setTrackballSize( const float size );

    void setDepthValue( const float depth );

    void setRotationCenter( const kvs::Vector2f& center );

    void setScaling( const kvs::Vector3f& scaling );

    void setTranslation( const kvs::Vector3f& translation );

    void setRotation( const kvs::Quaternion<float>& rotation );

    void setWindowSize( const int w, const int h );

    const float size( void ) const;

    const float depthValue( void ) const;

    const kvs::Vector2f& rotationCenter( void ) const;

    const kvs::Vector3f& scaling( void ) const;

    const kvs::Vector3f& translation( void ) const;

    const kvs::Quaternion<float>& rotation( void ) const;

    const int windowWidth( void ) const;

    const int windowHeight( void ) const;

public:

    void scale( const kvs::Vector2i& start, const kvs::Vector2i& end, ScalingType type = ScalingXYZ );

    void rotate( const kvs::Vector2i& start, const kvs::Vector2i& end );

    void translate( const kvs::Vector2i& start, const kvs::Vector2i& end );

protected:

    void reset( void );

protected:

    const float depth_on_sphere( const kvs::Vector2f& dir ) const;

    const kvs::Vector2f get_norm_position( const kvs::Vector2i& pos ) const;

public:

    static void x_scaling( const kvs::Vector2f& start, const kvs::Vector2f& end, Trackball* track );
    static void y_scaling( const kvs::Vector2f& start, const kvs::Vector2f& end, Trackball* track );
    static void z_scaling( const kvs::Vector2f& start, const kvs::Vector2f& end, Trackball* track );
};

} // end of namespace kvs

#endif // KVS__TRACKBALL_H_INCLUDE
