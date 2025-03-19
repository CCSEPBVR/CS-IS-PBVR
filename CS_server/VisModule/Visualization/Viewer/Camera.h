/****************************************************************************/
/**
 *  @file Camera.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Camera.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__CAMERA_H_INCLUDE
#define VIS_MODULE__CAMERA_H_INCLUDE

#include <vismodule/XformControl>
#include <vismodule/ClassName>
#include <vismodule/Vector2>
#include <vismodule/Vector3>
#include <vismodule/Matrix33>
#include <vismodule/ColorImage>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Camera class
 */
/*==========================================================================*/
class Camera : public XformControl
{
    visModuleClassName( vismodule::Camera );

public:

    enum ProjectionType
    {
        Perspective = 0,
        Orthogonal = 1,
        Frustum
    };

protected:

    ProjectionType m_projection_type; ///< projection type
    vismodule::Vector3f m_init_position; ///< initial position in the world coordinate system
    vismodule::Vector3f m_init_up_vector; ///< initial up vector in the world coordinate system
    vismodule::Vector3f m_init_look_at; ///< initial look-at point in the world coordinate system
    vismodule::Vector3f m_position; ///< position in the world coordinate system
    vismodule::Vector3f m_up_vector; ///< up vector in the world coordinate system
    vismodule::Vector3f m_look_at; ///< look-at point in the world coordinate system
    float m_field_of_view; ///< field of view [deg]
    float m_front; ///< front plane position
    float m_back; ///< back plane position
    float m_left; ///< left plane position
    float m_right; ///< right plane position
    float m_bottom; ///< bottom plane position
    float m_top; ///< top plane position
    size_t m_window_width; ///< window width
    size_t m_window_height; ///< window height

public:

    Camera( const bool collision = true );

    virtual ~Camera( void );

public:

    void setProjectionType( const ProjectionType projection_type );

    void setPosition( const vismodule::Vector3f& position );

    void setUpVector( const vismodule::Vector3f& up_vector );

    void setLookAt( const vismodule::Vector3f& look_at );

    void setFieldOfView( const float fov );

    void setBack( const float back );

    void setFront( const float front );

    void setLeft( const float left );

    void setRight( const float right );

    void setBottom( const float bottom );

    void setTop( const float top );

    void setWindowSize( const size_t width, const size_t height );

public:

    const bool isPerspective( void ) const;

    const ProjectionType projectionType( void ) const;

    const vismodule::Vector3f& position( void ) const;

    const vismodule::Vector3f& upVector( void ) const;

    const vismodule::Vector3f& lookAt( void ) const;

    const vismodule::Vector2f lookAtInDevice( void ) const;

    const float fieldOfView( void ) const;

    const float back( void ) const;

    const float front( void ) const;

    const float left( void ) const;

    const float right( void ) const;

    const float bottom( void ) const;

    const float top( void ) const;

    const size_t windowWidth( void ) const;

    const size_t windowHeight( void ) const;

public:

    virtual void initialize( void );

    virtual void update( void );

    virtual vismodule::ColorImage snapshot( void );

public:

    const vismodule::Matrix44f projectionMatrix( void ) const;

    const vismodule::Matrix44f modelViewMatrix( void ) const;

    const vismodule::Matrix44f projectionModelViewMatrix( void ) const;

    void getProjectionMatrix( float (*projection)[16] ) const;

    void getModelViewMatrix( float (*modelview)[16] ) const;

    void getProjectionModelViewMatrix( float (*projection_modelview)[16] ) const;

    void getProjectionModelViewMatrix(
        float projection[16],
        float modelview[16],
        float (*projection_modelview)[16] ) const;

    void getCombinedMatrix( float (*combined)[16] ) const;

    void getCombinedMatrix(
        float projection[16],
        float modelview[16],
        float (*combined)[16] ) const;

    const vismodule::Vector2f projectObjectToWindow(
        float  p_obj_x,
        float  p_obj_y,
        float  p_obj_z,
        float* depth = NULL ) const;

    const vismodule::Vector2f projectObjectToWindow(
        const vismodule::Vector3f& p_obj,
        float*               depth = NULL ) const;

public:

    const vismodule::Vector3f projectWindowToObject(
        const vismodule::Vector2f& p_win,
        float                depth = 0.0 ) const;

    const vismodule::Vector3f projectWindowToCamera(
        const vismodule::Vector2f& p_win,
        float                depth = 0.0 ) const;

    const vismodule::Vector3f projectWindowToWorld(
        const vismodule::Vector2f& p_win,
        float                depth = 0.0 ) const;

    const vismodule::Vector3f projectObjectToCamera( const vismodule::Vector3f& p_obj ) const;

    const vismodule::Vector3f projectCameraToObject( const vismodule::Vector3f& p_cam ) const;

    const vismodule::Vector3f projectWorldToCamera( const vismodule::Vector3f& p_wld ) const;

    const vismodule::Vector3f projectCameraToWorld( const vismodule::Vector3f& p_cam ) const;

    const vismodule::Vector3f projectWorldToObject( const vismodule::Vector3f& p_wld ) const;

    const vismodule::Vector3f projectObjectToWorld( const vismodule::Vector3f& p_obj ) const;

public:

    void resetXform( void );

    void rotate( const vismodule::Matrix33f& rotation );

    void translate( const vismodule::Vector3f& translation );

    void scale( const vismodule::Vector3f& scaling );

private:

    void update_up_at_from( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CAMERA_H_INCLUDE
