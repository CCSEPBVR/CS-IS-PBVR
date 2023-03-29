/*****************************************************************************/
/**
 *  @file   Scene.h
 *  @author Insight, Inc.
 */
/*****************************************************************************/
#pragma once

#include <kvs/Mouse>
//#include <kvs/ScreenBase>
//#include <kvs/Scene>
#include <kvs/ObjectBase>
#include <kvs/Light>
#include <kvs/Xform>
#include "screen.h"
#include <vector>
#include <set>
#include <iostream>
#include <cmath>

namespace kvs
{
class Camera;
class Light;
class Mouse;
class Background;
class RendererManager;
class IDManager;
class ObjectBase;
class RendererBase;
class CubicImage;

namespace oculus
{

namespace jaea
{

class PBVRObjectManager;

class NormalizedDeviceCoordinate;
class WorldCoordinate;
class ObjectCoordinate;

// copy from kvs::CameraCoordinate
/*===========================================================================*/
/**
 *  @brief  Camera coordinate class.
 */
/*===========================================================================*/
class CameraCoordinate
{
private:

    kvs::Vec3 m_position; ///< position in camera coordinates
    const kvs::Camera* m_camera; ///< camera defines camera coordinates

public:

    CameraCoordinate( const kvs::Vec3& position, const kvs::Camera* camera );

    const kvs::Vec3& position() const { return m_position; }
    const NormalizedDeviceCoordinate toNormalizedDeviceCoordinate() const;
    const WorldCoordinate toWorldCoordinate() const;
};

// copy from kvs::WorldCoordinate
/*===========================================================================*/
/**
 *  @brief  World coordinate class.
 */
/*===========================================================================*/
class WorldCoordinate
{
private:

    kvs::Vec3 m_position; ///< position in world coordinates

public:

    WorldCoordinate( const kvs::Vec3& position );

    const kvs::Vec3& position() const { return m_position; }
    const CameraCoordinate toCameraCoordinate( const kvs::Camera* camera ) const;
    const ObjectCoordinate toObjectCoordinate( const kvs::ObjectBase* object ) const;
};

// copy from kvs::ObjectCoordinate
/*===========================================================================*/
/**
 *  @brief  Object coordinate class.
 */
/*===========================================================================*/
class ObjectCoordinate
{
private:

    kvs::Vector3f m_position; ///< position in object coordinates
    const kvs::ObjectBase* m_object; ///< object defines object coordinates

public:

    ObjectCoordinate( const kvs::Vec3& position, const kvs::ObjectBase* object );

    const kvs::Vec3& position() const { return m_position; }
    const WorldCoordinate toWorldCoordinate() const;
};



// copy from kvs::WorldCoordinate
class NormalizedDeviceCoordinate;

/*===========================================================================*/
/**
 *  @brief  Window coordinate class.
 */
/*===========================================================================*/
class WindowCoordinate
{
private:

    kvs::Vec3 m_position; ///< position in window coordinates
    int m_x; ///< x coordinate value of left corner of the viewport
    int m_y; ///< y coordinate value of left corner of the viewport
    size_t m_width; ///< width of the viewport
    size_t m_height; ///< height of the viewport
    static float m_front; ///< depth value of the front clipping plane
    static float m_back; ///< depth value of the back clipping plane

public:

    static void SetDepthRange( const float front, const float back );
    static float CalculateDepth( const float nd_depth );
    static float InvertDepth( const float depth );

public:

    WindowCoordinate( const kvs::Vec3& position, const int x, const int y, const size_t width, const size_t height );
    WindowCoordinate( const kvs::Vec3& position, const kvs::Vec4i& viewport );

    const kvs::Vec3& position() const { return m_position; }
    const NormalizedDeviceCoordinate toNormalizedDeviceCoordinate() const;
};

// copy from kvs::NomralizedDeviceCoordinate
/*===========================================================================*/
/**
 *  @brief  Normalized device coordinate class.
 */
/*===========================================================================*/
class NormalizedDeviceCoordinate
{
private:

    kvs::Vec3 m_position; ///< position in normalized device coordinates

public:

    NormalizedDeviceCoordinate( const kvs::Vec3& position );

    const kvs::Vec3& position() const { return m_position; }
    const WindowCoordinate toWindowCoordinate( const int x, const int y, const size_t width, const size_t height ) const;
    const WindowCoordinate toWindowCoordinate( const kvs::Vec4i& viewport ) const;
    const CameraCoordinate toCameraCoordinate( const kvs::Camera* camera ) const;
};



/*===========================================================================*/
/**
 *  @brief  Default scene class.
 */
/*===========================================================================*/
class Scene //: public kvs::Scene
{
    // copy from kvs::Scene
public:
    enum ControlTarget
    {
        TargetObject = 0, ///< controlling object
        TargetCamera, ///< controlling camera
        TargetLight, ///< controlling light
        NumberOfTargets ///< number of control targets
    };

private:
    kvs::ScreenBase* m_screen; ///< screen
    kvs::Camera* m_camera; ///< camera
    kvs::Light* m_light; ///< light
    kvs::Mouse* m_mouse; ///< mouse
    kvs::Background* m_background; ///< background
    kvs::oculus::jaea::PBVRObjectManager* m_object_manager; ///< object manager
    kvs::RendererManager* m_renderer_manager; ///< renderer manager
    kvs::IDManager* m_id_manager; ///< ID manager ( object_id, renderer_id )
    ControlTarget m_target; ///< control target
    bool m_enable_object_operation;  ///< flag for object operation
    bool m_enable_collision_detection; ///< flag for collision detection

public:
    Scene( kvs::ScreenBase* screen );
    virtual ~Scene();

    const std::pair<int,int> registerObject( kvs::ObjectBase* object, kvs::RendererBase* renderer = 0 );
    void removeObject( int object_id, bool delete_object = true, bool delete_renderer = true );
    void removeObject( std::string object_name, bool delete_object = true, bool delete_renderer = true );
    void replaceObject( int object_id, kvs::ObjectBase* object, bool delete_object = true );
    void replaceObject( std::string object_name, kvs::ObjectBase* object, bool delete_object = true );
    void replaceRenderer( int renderer_id, kvs::RendererBase* renderer, bool delete_renderer = true );
    void replaceRenderer( std::string renderer_name, kvs::RendererBase* renderer, bool delete_renderer = true );

    int numberOfObjects() const;
    bool hasObjects() const;
    bool hasObject( int id ) const;
    bool hasObject( std::string name ) const;
    kvs::ObjectBase* object( int id = -1 );
    kvs::ObjectBase* object( std::string name );
    kvs::RendererBase* renderer( int id = -1 );
    kvs::RendererBase* renderer( std::string name );

    void reset();
    bool isActiveMove( int x, int y );
    void updateControllingObject();
    void updateCenterOfRotation();
    void updateXform();
    void updateXform( kvs::oculus::jaea::PBVRObjectManager* manager );
    void updateXform( kvs::Camera* camera );
    void updateXform( kvs::Light* light );

    void updateGLModelingMatrix( const kvs::ObjectBase* object ) const;
    void updateGLModelingMatrix() const;
    void updateGLViewingMatrix() const;
    void updateGLProjectionMatrix() const;
    void updateGLLightParameters() const;

    kvs::CubicImage cubemap();

    void setEnabledCollisionDetection( bool enable );
    void enableCollisionDetection() { this->setEnabledCollisionDetection( true ); }
    void disableCollisionDetection() { this->setEnabledCollisionDetection( false ); }
    bool isEnabledCollisionDetection() const { return m_enable_collision_detection; }

    void setEnabledObjectOperation( bool enable ) { m_enable_object_operation = enable; }
    void enableObjectOperation() { this->setEnabledObjectOperation( true ); }
    void disableObjectOperation() { this->setEnabledObjectOperation( false ); }
    bool isEnabledObjectOperation() const { return m_enable_object_operation; }

    kvs::ScreenBase* screen() { return m_screen; }
    kvs::Camera* camera() { return m_camera; }
    kvs::Light* light() { return m_light; }
    kvs::Mouse* mouse() { return m_mouse; }
    kvs::Background* background() { return m_background; }
    kvs::oculus::jaea::PBVRObjectManager* objectManager() { return m_object_manager; }
    kvs::RendererManager* rendererManager() { return m_renderer_manager; }
    kvs::IDManager* IDManager() { return m_id_manager; }
    ControlTarget& controlTarget() { return m_target; }

    const kvs::Camera* camera() const { return m_camera; }
    const kvs::Light* light() const { return m_light; }
    const kvs::Mouse* mouse() const { return m_mouse; }
    const kvs::Background* background() const { return m_background; }
    const kvs::oculus::jaea::PBVRObjectManager* objectManager() const { return m_object_manager; }
    const kvs::RendererManager* rendererManager() const { return m_renderer_manager; }
    const kvs::IDManager* IDManager() const { return m_id_manager; }
    const ControlTarget& controlTarget() const { return m_target; }

    void initializeFunction();
    void paintFunction();
    void resizeFunction( int width, int height );
    void mouseReleaseFunction( int x, int y );
    void mousePressFunction( int x, int y, kvs::Mouse::OperationMode mode );
    void mouseMoveFunction( int x, int y );
    void wheelFunction( int value );

private:
    kvs::Vec2 position_in_device( const kvs::ObjectBase* object ) const;
    kvs::Vec2 position_in_device() const;
    bool detect_collision( const kvs::Vec2& p_win );
    bool detect_collision( const kvs::ObjectBase* object, const kvs::Vec2& p_win );


    // ----------------------------------------------------------------------------------
    // kvs::oculus::jaea::Scene original
private:
    kvs::Real32 m_scaling_ratio_accum;

public:
//    Scene( kvs::ScreenBase* screen );

#ifdef USE_OLD_TRANSLATION
    void threedcontrollerPressFunction (kvs::Vec3 eyePos, kvs::Vec3 eyeDir, kvs::Vec3 left_p, kvs::Vec3 right_p, kvs::Vec3 m_prev_left_p, kvs::Vec3 m_prev_right_p, kvs::Vec3 m_start_left_p, kvs::Vec3 m_start_right_p);
    void threedcontrollerReleaseFunction (kvs::Vec3 left_p, kvs::Vec3 right_p, kvs::Vec3 m_prev_left_p, kvs::Vec3 m_prev_right_p, kvs::Vec3 m_start_left_p, kvs::Vec3 m_start_right_p);
#else // USE_OLD_TRANSLATION
    void threedcontrollerPressFunction (kvs::Vec3 hmdPos, kvs::Vec3 initialHmdPos, kvs::Vec3 hmdDir, kvs::Vec3 left_p, kvs::Vec3 right_p, kvs::Vec3 prev_left_p, kvs::Vec3 prev_right_p);
    void threedcontrollerReleaseFunction (kvs::Vec3 hmdPos, kvs::Vec3 initialHmdPos, kvs::Vec3 hmdDir, kvs::Vec3 left_p, kvs::Vec3 right_p, kvs::Vec3 prev_left_p, kvs::Vec3 prev_right_p);
#endif // USE_OLD_TRANSLATION

//    kvs::Real32 getScalingRatioAccum() {return m_scaling_ratio_accum;};
    kvs::Real32 getScalingRatioAccum();

    void resetObjects();

private:
#ifdef USE_OLD_TRANSLATION
    float calcTranslationFactor(kvs::Vec3 cameraPos, kvs::Vec3 cameraDir, float prev_x, float prev_y, float prev_z);
//    float calcRotationFactor();
#else // USE_OLD_TRANSLATION
    float calcTranslationFactor(kvs::Vec3 hmdPos, kvs::Vec3 hmdDir, kvs::Vec3 hand_prev, kvs::Vec3 cameraPos);
#endif // USE_OLD_TRANSLATION

};


} // jaea

} // end of namespace oculus

} // end of namespace kvs

typedef kvs::oculus::jaea::Scene::ControlTarget ControlTarget;
