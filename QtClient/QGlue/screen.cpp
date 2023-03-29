//#include <GL/glew.h>
//#include <kvs/OpenGL>
#include "screen.h"
#include <kvs/FrameBufferObject>
#include <kvs/Camera>
#include <kvs/ObjectBase>
#include <QElapsedTimer>
#include <QMutex>

// from OculusKVS (kvs::oculus::jaea::Screen)
//#include <kvs/Scene>
#include <kvs/Background>
//#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/StochasticRenderingCompositor>
#include <kvs/IDManager>
#include <kvs/RendererBase>
#include <kvs/PaintEvent>
#include <kvs/FrameEvent>

#include <OVR_CAPI.h>

#include "pbvrwidgethandler.h"
#include "TouchController.h"
#include "Scene.h"
#include "PBVRObjectManager.h"
#include "objnameutil.h"
#include "ovr.h"

#ifdef ENABLE_TIME_MEASURE
#include "TimeMeasureUtility.h"
#endif // ENABLE_TIME_MEASURE

namespace {

inline kvs::Vec3 ToVec3(const OVR::Vector3f& v) {
    return kvs::Vec3(v.x, v.y, v.z);
}

inline OVR::Vector3f ToVec3(const kvs::Vec3& v) {
    return OVR::Vector3f(v.x(), v.y(), v.z());
}

inline kvs::Mat4 ToMat4(const OVR::Matrix4f& m) {
    return kvs::Mat4(m.M[0][0], m.M[0][1], m.M[0][2], m.M[0][3], m.M[1][0], m.M[1][1], m.M[1][2], m.M[1][3], m.M[2][0], m.M[2][1], m.M[2][2],
            m.M[2][3], m.M[3][0], m.M[3][1], m.M[3][2], m.M[3][3]);
}

inline OVR::Matrix4f ToMat4(const kvs::Mat3& m) {
    return OVR::Matrix4f(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);
}

inline OVR::Matrix4f ToMat4(const kvs::Mat4& m) {
    return OVR::Matrix4f(m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1], m[2][2], m[2][3], m[3][0], m[3][1],
            m[3][2], m[3][3]);
}
} // end of namespace <anon>

extern QMutex paint_mutex;

//const kvs::Vec3 Screen::KVS_CAMERA_INITIAL_POSITION = kvs::Vec3(0.0f,0.0f,6.0f);
const kvs::Vec3 Screen::KVS_CAMERA_INITIAL_POSITION = kvs::Vec3(0.0f,0.0f,12.0f);

/**
 * @brief Screen::Screen
 * @param parent_surface, parent Qt Widget
 */
Screen::Screen( QWidget* parent_surface) :
    m_widget_handler(nullptr),
    m_compositor(nullptr)
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG Screen::Screen() called." << std::endl;
#endif // DEBUG_SCREEN

    Q_UNUSED(parent_surface);
    //m_scene = new kvs::Scene(this);
    m_scene = new kvs::oculus::jaea::Scene(this);
    m_scene->camera()->setPosition(KVS_CAMERA_INITIAL_POSITION);

    Screen::setFocusPolicy(Qt::StrongFocus );
    ScreenBase::create();
    m_scene->camera()->setWindowSize(340,340);
    m_scene->mouse()->attachCamera( m_scene->camera());

#ifdef ENABLE_ORIENTATION_CORRECTION
    m_v_inverse_initialized = V_INVERSE_IS_NOT_INITIALIZED; // not initialized
#endif // ENABLE_ORIENTATION_CORRECTION

    {
        kvs::oculus::jaea::PBVRObjectManager* objectManager = m_scene->objectManager();
        kvs::Vec3 minExtCrd = objectManager->minExternalCoord();
        kvs::Vec3 maxExtCrd = objectManager->maxExternalCoord();
        kvs::Vec3 minObjCrd = objectManager->minObjectCoord();
        kvs::Vec3 maxObjCrd = objectManager->maxObjectCoord();
        kvs::Mat4 m = objectManager->modelingMatrix();

//        std::cerr << "Screen::Screen() : initial status of object manager:" << std::endl;
//        std::cerr << "minExtCrd = " << minExtCrd << std::endl;
//        std::cerr << "maxExtCrd = " << maxExtCrd << std::endl;
//        std::cerr << "minObjCrd = " << minObjCrd << std::endl;
//        std::cerr << "maxObjCrd = " << maxObjCrd << std::endl;
//        std::cerr << "m=" << std::endl;
//        std::cerr << m << std::endl;
    }

    update();

    static bool flag = true;
    if (flag) {
        flag = false;
        if (!m_hmd.create()) {
            kvsMessageError("Failed HMD create");
        }

        // from OculusKVS (kvs::oculus::jaea::Screen)
        m_touch_controller = new kvs::oculus::jaea::TouchController(this->m_hmd, this->m_scene);

#ifdef DEBUG_SCREEN
        std::cout << "Screen::Screen() : create TouchController()" << std::endl;
#endif //DEBUG_SCREEN

        //this->addEvent(m_touch_controller);
        //m_widget_handler = nullptr;
        //m_mirror_buffer_type = DistortedBothEyeImage;

#ifdef DEBUG_SCREEN
        std::cout << "Screen::Screen() : add TouchController as frame event handler." << std::endl;
#endif // DEBUG_SCREEN

        // add widget handler
        this->m_widget_handler = PBVRWidgetHandlerInstance();

#ifdef DEBUG_SCREEN
        std::cout << "Screen::Screen() : create WidgetHandler" << std::endl;
#endif // DEBUG_SCREEN

        m_hmd.setWidgetHandler(this->m_widget_handler);
        m_touch_controller->setWidgetHandler (this->m_widget_handler);

#ifdef DEBUG_SCREEN
        std::cout << "Screen::Screen() : set widget handler to hmd and touch controller." << std::endl;
#endif // DEBUG_SCREEN

        const int desktop_width = 340;
        const int desktop_height = 340;

        const int hmd_width = m_hmd.resolution().w;
        const int hmd_height = m_hmd.resolution().h;

        const float ratio = static_cast<float>(desktop_width) / hmd_width;
        const int screen_width = (ratio >= 1.0f) ? hmd_width : desktop_width;
        const int screen_height = (ratio >= 1.0f) ? hmd_height : static_cast<int>(hmd_height * ratio);
        setSize(screen_width, screen_height);
    }

}
/**
 * @brief Screen::paintGL, paint GL method - automatically called as part of Qt Event chain.
 *                         Do not call directly.
 *
 *                         Attains paint_mutex lock, checks opengl status and paints the scene.
 *
 *                         After painting the scene, the virtual method onPaintGL is called to allow
 *                         any subclass to paint additional items on top of scene.
 *
 *                         Finally glGetError is called until any and all errors are cleared.
 */
void Screen::paintGL()
{
#ifdef DEBUG_SCREEN
    std::cout << "MSG Screen::paintGL() called." << std::endl;
#endif // DEBUG_SCREEN

    if (QThread::currentThread() != this->thread()) {
        qWarning("Screen::paintGL was not called from main thread, skipping frame");
        return;
    }
    if (!Screen::m_gl_initialized || m_hold_paintGL){
        qWarning("Screen either not initialized or on hold, skipping this frame");
        return;
    }
    int e;
    // Try to get paint_mutex lock.
    // This is probably not needed any more
    if (!paint_mutex.try_lock()){
        qWarning("Screen could not get paint_mutex lock, skipping this frame");
        update(); // Schedule new update
        return;
    }
    // Check and clear GL errors
#ifdef DEBUG_SCREEN
    while ((e = glGetError()) != 0){
        qCritical("Screen::paintGL GL HAS ERROR BEFORE %d",e);
    }
#endif // DEBUG_SCREEN
    // This allows KVS to work with QOpenGLWidgets
    // Requires future release of KVS which includes commit
    // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
    kvs::FrameBufferObject::m_unbind_id=QOpenGLWidget::defaultFramebufferObject();

#ifdef DEBUG_SCREEN
    std::cout << "paintGL : m_unbind_id=" << kvs::FrameBufferObject::m_unbind_id << std::endl;
#endif // DEBUG_SCREEN

    QElapsedTimer timer;

    // from OculusKVS (kvs::oculus::jaea::Screen)
    static kvs::Int64 frame_index = 0;
    static ovrVector3f initial_hp;
    static OVR::Quatf initial_ho;
    static ovrVector3f initial_ep[2];
    static OVR::Quatf initial_eo[2];
    static float ehLength[2];
    static ovrVector3f initial_focus[2];

    ovrSessionStatus sessionStatus;
    ovr_GetSessionStatus(m_hmd.handler(), &sessionStatus);

    {
        ovrErrorInfo error;
        ovr_GetLastErrorInfo( &error );
        if ( !OVR_SUCCESS( error.Result ) ) {
            std::string message( error.ErrorString );
            std::cout << "ERROR/Oculus : after ovr_GetSessionStatus() : " << message << std::endl;
        }
    }

    if (sessionStatus.ShouldQuit) {
        // Because the application is requested to quit, should not request retry
        //retryCreate = false;
        //break;
        // TODO : implement shutdown event.
    }
    if (sessionStatus.ShouldRecenter)
    {
        ovr_RecenterTrackingOrigin(m_hmd.handler());
    }

    if (sessionStatus.IsVisible) {
#ifdef ENABLE_TIME_MEASURE
        SET_INDEX_FOR_TIMER(frame_index);

        MAKE_AND_START_TIMER(A);
        MAKE_AND_START_TIMER(A_0);
        MAKE_AND_START_TIMER(A_0_1);

#endif // ENABLE_TIME_MEASURE

        const double frame_timing = m_hmd.frameTiming(frame_index);
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_1, "(A_0_1)   m_hmd.frameTiming(frame_index)");
        MAKE_AND_START_TIMER(A_0_2);
#endif // ENABLE_TIME_MEASURE
        const ovrTrackingState& ts = m_hmd.trackingState(frame_timing);
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_2, "(A_0_2)   m_hmd.trackingState(frame_timing)");
        MAKE_AND_START_TIMER(A_0_3);
#endif // ENABLE_TIME_MEASURE

        {
            ovrErrorInfo error;
            ovr_GetLastErrorInfo( &error );
            if ( !OVR_SUCCESS( error.Result ) ) {
                std::string message( error.ErrorString );
                std::cout << "ERROR/Oculus : after m_hmd.trackingState() : " << message << std::endl;
            }
        }
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_3, "(A_0_3)   ovr_GetLastErrorInfo()");
        MAKE_AND_START_TIMER(A_0_4);
#endif // ENABLE_TIME_MEASURE

        const kvs::Vec4 current_viewport = kvs::OpenGL::Viewport();
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_4, "(A_0_4)   kvs::OpenGL::Viewport()");
        MAKE_AND_START_TIMER(A_0_5);
#endif // ENABLE_TIME_MEASURE

        // Check and clear GL errors
#ifdef DEBUG_SCREEN
        while ((e = glGetError()) != 0){
            qCritical("Screen::paintGL GL HAS ERROR after kvs::OpenGL::Viewport() : %d",e);
        }
#endif // DEBUG_SCREEN

#ifdef USE_KVS_EVENTHANDLER
        {
            // call frame event listeners
            kvs::FrameEvent event;
            BaseClass::eventHandler()->notify( &event );
        }
#else //USE_KVS_EVENTHANDLER
        this->m_touch_controller->frameEvent();
#endif // USE_KVS_EVENTHANDLER
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_5, "(A_0_5)   m_touch_controller->frameEvent()");
        MAKE_AND_START_TIMER(A_0_6);
#endif // ENABLE_TIME_MEASURE

        glPushAttrib( GL_ALL_ATTRIB_BITS );
        kvs::OpenGL::WithPushedMatrix p(GL_MODELVIEW);
        p.loadIdentity();
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_6, "(A_0_6)   p.loadIdentity()");
        MAKE_AND_START_TIMER(A_0_7);
#endif // ENABLE_TIME_MEASURE

        m_hmd.beginFrame(frame_index);
        timer.start();
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_7, "(A_0_7)   m_hmd.beginFrame(frame_index)");
        MAKE_AND_START_TIMER(A_0_8);
#endif // ENABLE_TIME_MEASURE

        scene()->updateGLLightParameters();
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_8, "(A_0_8)   scene()->updateGLLightParameters()");
        MAKE_AND_START_TIMER(A_0_9);
#endif // ENABLE_TIME_MEASURE

        // Check and clear GL errors
#ifdef DEBUG_SCREEN
        while ((e = glGetError()) != 0){
            qCritical("Screen::paintGL GL HAS ERROR after scene()->updateGLLightParameters() : %d",e);
        }
#endif // DEBUG_SCREEN

        // 20210129 yodo append to force to locate KVS camera at fixed position.
        scene()->camera()->setPosition(KVS_CAMERA_INITIAL_POSITION);
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_9, "(A_0_9)   scene()->camera()->setPosition()");
        MAKE_AND_START_TIMER(A_0_10);
#endif // ENABLE_TIME_MEASURE

        // preserve camera status
        const kvs::Vec3 camera_position = scene()->camera()->position();
        const kvs::Vec3 camera_lookat = scene()->camera()->lookAt();
        const kvs::Vec3 camera_upvector = scene()->camera()->upVector();
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_10, "(A_0_10)   ");
        MAKE_AND_START_TIMER(A_0_11);
#endif // ENABLE_TIME_MEASURE
        const OVR::Vector3f position0 = ::ToVec3(camera_position);
        const OVR::Vector3f lookat0 = ::ToVec3(camera_lookat);
        const OVR::Vector3f upvector0 = ::ToVec3(camera_upvector);
        const OVR::Matrix4f rotation0 = ::ToMat4(scene()->camera()->xform().rotation());
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_11, "(A_0_11)   ");
        MAKE_AND_START_TIMER(A_0_12);
#endif // ENABLE_TIME_MEASURE

        // 20201228
        kvs::Vec3 camera_focus = -camera_position;
        const OVR::Vector3f focus0 = ::ToVec3(camera_focus);
#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_12, "(A_0_12)   ");
        MAKE_AND_START_TIMER(A_0_13);
#endif // ENABLE_TIME_MEASURE

        const size_t neyes = ovrEye_Count;

#ifdef DEBUG_SCREEN
        std::cout << "Screen::paintGL() : neyes=" << neyes << std::endl;
#endif // DEBUG_SCREEN

        OVR::Vector3f eyeCameraPP[2] = {OVR::Vector3f(-1.f, -1.f, -1.f), OVR::Vector3f(-1.f, -1.f, -1.f)};
        OVR::Vector3f eyeCameraT[2] = {OVR::Vector3f(-1.f, -1.f, -1.f), OVR::Vector3f(-1.f, -1.f, -1.f)};
        OVR::Vector3f eyeCameraEP[2] = {OVR::Vector3f(-1.f, -1.f, -1.f), OVR::Vector3f(-1.f, -1.f, -1.f)};
        OVR::Matrix4f eyeCameraR[2];

        float eyePosition[2][3] = {{-1.f,-1.f,-1.f},{-1.f,-1.f,-1.f}};
        float eyeOrientation[2][4] = {{-1.f,-1.f,-1.f,-1.f},{-1.f,-1.f,-1.f,-1.f}};
        float fovUpTan[2] = {-1.f,-1.f};
        float fovDownTan[2] = {-1.f,-1.f};
        float fovLeftTan[2] = {-1.f,-1.f};
        float fovRightTan[2] = {-1.f,-1.f};
        kvs::Vec3 eyeCameraPosition[2] = {kvs::Vec3(-1.f, -1.f, -1.f), kvs::Vec3(-1.f, -1.f, -1.f)};
        kvs::Vec3 eyeCameraLookAt[2] = {kvs::Vec3(-1.f, -1.f, -1.f), kvs::Vec3(-1.f, -1.f, -1.f)};
        kvs::Vec3 eyeCameraUpVector[2] = {kvs::Vec3(-1.f, -1.f, -1.f), kvs::Vec3(-1.f, -1.f, -1.f)};

        kvs::Mat4 kvsViewingMatrix[2] = {
            kvs::Mat4(),
            kvs::Mat4()
        };

        OVR::Matrix4f ovrViewingMatrix[2] = {
            OVR::Matrix4f(),
            OVR::Matrix4f()
        };

        const ovrQuatf headOrientation = ts.HeadPose.ThePose.Orientation;
        const OVR::Vector3f headPosition = ts.HeadPose.ThePose.Position;

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_0_13, "(A_0_13)   ");
        STOP_AND_RECORD_TIMER_WITH_ID(A_0, "(A_0)   ");
#endif // ENABLE_TIME_MEASURE

        for (size_t eye = 0; eye < neyes; eye++) {
#ifdef ENABLE_TIME_MEASURE
            SET_EYE_FOR_TIMER(eye);
            MAKE_AND_START_TIMER(A_1or2);
            MAKE_AND_START_TIMER(A_1or2_1);
            MAKE_AND_START_TIMER(A_1or2_1_1);
#endif // ENABLE_TIME_MEASURE

        // bind eye render target
            m_hmd.eyeRenderTexture(eye)->SetAndClearRenderSurface();
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_1, eye==0 ? "(A_1_1_1)   SetAndClearRenderSurface()" : "(A_2_1_1)   SetAndClearRenderSurface()");
            MAKE_AND_START_TIMER(A_1or2_1_2);
#endif // ENABLE_TIME_MEASURE

            const ovrEyeRenderDesc& render_desc = m_hmd.renderDesc(eye);
            const ovrPosef& eye_pose = m_hmd.eyePose(eye);
            const ovrRecti& viewport = m_hmd.viewport(eye);
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_2, eye==0 ? "(A_1_1_2)   m_hmd.viewport(eye)" : "(A_2_1_2)   m_hmd.viewport(eye)");
            MAKE_AND_START_TIMER(A_1or2_1_3);
#endif // ENABLE_TIME_MEASURE

            {
                ovrErrorInfo error;
                ovr_GetLastErrorInfo( &error );
                if ( !OVR_SUCCESS( error.Result ) ) {
                    std::string message( error.ErrorString );
                    std::cout << "ERROR/Oculus : after m_hmd.renderDesc/eyePose/viewport() : " << message << std::endl;
                }
            }
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_3, eye==0 ? "(A_1_1_3)   ovr_GetLastErrorInfo()" : "(A_2_1_3)   ovr_GetLastErrorInfo()");
            MAKE_AND_START_TIMER(A_1or2_1_4);
#endif // ENABLE_TIME_MEASURE

            eyePosition[eye][0] = eye_pose.Position.x;
            eyePosition[eye][1] = eye_pose.Position.y;
            eyePosition[eye][2] = eye_pose.Position.z;
            eyeOrientation[eye][0] = eye_pose.Orientation.x;
            eyeOrientation[eye][1] = eye_pose.Orientation.y;
            eyeOrientation[eye][2] = eye_pose.Orientation.z;
            eyeOrientation[eye][3] = eye_pose.Orientation.w;
            fovUpTan[eye] = render_desc.Fov.UpTan;
            fovDownTan[eye] = render_desc.Fov.DownTan;
            fovLeftTan[eye] = render_desc.Fov.LeftTan;
            fovRightTan[eye] = render_desc.Fov.RightTan;
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_4, eye==0 ? "(A_1_1_4)   " : "(A_2_1_4)   ");
            MAKE_AND_START_TIMER(A_1or2_1_5);
#endif // ENABLE_TIME_MEASURE

            // Set viewport.
            const ovrVector2i pos = viewport.Pos;
            const ovrSizei size = viewport.Size;
            kvs::OpenGL::SetViewport(pos.x, pos.y, size.w, size.h);
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_5, eye==0 ? "(A_1_1_5)   kvs::OpenGL::SetViewport()" : "(A_2_1_5)   kvs::OpenGL::SetViewport()");
            MAKE_AND_START_TIMER(A_1or2_1_6);
#endif // ENABLE_TIME_MEASURE

#ifdef DEBUG_SCREEN
            std::cout << "paintGL() : viewport[" << eye << "] pos(" << pos.x << ", " << pos.y << "), size(" << size.w << ", " << size.h << ")" << std::endl;

            // Check and clear GL errors
            while ((e = glGetError()) != 0){
                qCritical("Screen::paintGL GL HAS ERROR after kvs::OpenGL::SetViewport(eye=%d) : %d", eye, e);
            }
#endif // DEBUG_SCREEN

#ifdef USE_KVS_PROJECTION_MATRIX
            // Set projection matrix.
            const float aspect = static_cast<float>(size.w) / size.h;
            const float front = scene()->camera()->front();
            scene()->camera()->setWindowSize(size.w, size.h);
            if (aspect >= 1.0f) {
                scene()->camera()->setTop(render_desc.Fov.UpTan * front);
                scene()->camera()->setBottom(-render_desc.Fov.DownTan * front);
                scene()->camera()->setLeft(-render_desc.Fov.LeftTan * front / aspect);
                scene()->camera()->setRight(render_desc.Fov.RightTan * front / aspect);
            } else {
                scene()->camera()->setTop(render_desc.Fov.UpTan * front * aspect);
                scene()->camera()->setBottom(-render_desc.Fov.DownTan * front * aspect);
                scene()->camera()->setLeft(-render_desc.Fov.LeftTan * front);
                scene()->camera()->setRight(render_desc.Fov.RightTan * front);
            }

            // kvs original but something wrong for Oculus
            scene()->updateGLProjectionMatrix();

#else // USE_KVS_PROJECTION_MATRIX
            // 20210107 K.yodo append
            // same as Oculus SDK sample code
            scene()->camera()->setWindowSize(size.w, size.h);
            ovrViewingMatrix[eye] = ovrMatrix4f_Projection(m_hmd.defaultEyeFov(eye), OCULUS_NEAR, OCULUS_FAR, ovrProjection_None);
            float m[16];
            kvs::OpenGL::SetMatrixMode( GL_PROJECTION );
            kvs::OpenGL::LoadIdentity();
            kvs::Xform p(ToMat4(ovrViewingMatrix[eye]));
            p.toArray( m );
            kvs::OpenGL::MultMatrix( m );
#endif // USE_KVS_PROJECTION_MATRIX
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_6, eye==0 ? "(A_1_1_6)   kvs::OpenGL::MultMatrix()" : "(A_2_1_6)   kvs::OpenGL::MultMatrix()");
            MAKE_AND_START_TIMER(A_1or2_1_7);
#endif // ENABLE_TIME_MEASURE

            kvsViewingMatrix[eye] = scene()->camera()->projectionMatrix();
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_7, eye==0 ? "(A_1_1_7)   projectionMatrix()" : "(A_2_1_7)   projectionMatrix()");
            MAKE_AND_START_TIMER(A_1or2_1_8);
#endif // ENABLE_TIME_MEASURE

#ifdef DEBUG_SCREEN
            // Check and clear GL errors
            while ((e = glGetError()) != 0){
                qCritical("Screen::paintGL GL HAS ERROR after scene()->updateGLProjectionMatrix(eye=%d) : %d", eye, e);
            }
#endif // DEBUG_SCREEN

            // Set viewing matrix.
            if (frame_index == 0) {
                initial_hp = headPosition;
                initial_ho = headOrientation;
                initial_ep[eye] = eye_pose.Position;
                initial_eo[eye] = eye_pose.Orientation;

                float dx = initial_ep[eye].x - initial_hp.x;
                float dy = initial_ep[eye].y - initial_hp.y;
                float dz = initial_ep[eye].z - initial_hp.z;

                ehLength[eye] = std::sqrt(dx*dx + dy*dy + dz*dz);

                initial_focus[eye].x = focus0.x;
                initial_focus[eye].y = focus0.y;
                initial_focus[eye].z = focus0.z;

#ifdef ENABLE_ORIENTATION_CORRECTION
                m_v_inverse[eye] = kvs::Xform(ToMat4(OVR::Matrix4f(eye_pose.Orientation))).inverse();
#endif // ENABLE_ORIENTATION_CORRECTION
            }
#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_8, eye==0 ? "(A_1_1_8)   frame_index==0" : "(A_2_1_8)   frame_index==0");
            MAKE_AND_START_TIMER(A_1or2_1_9);
#endif // ENABLE_TIME_MEASURE

            if (frame_index > 0) {
                float transFactor = 1e0f;
                OVR::Vector3f position;
                position.x = eye_pose.Position.x - initial_ep[eye].x + position0.x;
                position.y = eye_pose.Position.y - initial_ep[eye].y + position0.y;
                position.z = eye_pose.Position.z - initial_ep[eye].z + position0.z;

#ifdef ENABLE_ORIENTATION_CORRECTION
                OVR::Matrix4f R_original = OVR::Matrix4f(eye_pose.Orientation);
                kvs::Xform R_kvs = kvs::Xform(ToMat4(R_original));
                kvs::Xform R_corrected = R_kvs * m_v_inverse[eye];
                float R_array[16]; R_corrected.toArray(R_array);
                kvs::Mat4 R_mat4 = kvs::Mat4(R_array);
                OVR::Matrix4f R = ToMat4(R_mat4);
#else // ENABLE_ORIENTATION_CORRECTION
                OVR::Matrix4f R = OVR::Matrix4f(eye_pose.Orientation);
#endif // ENABLE_ORIENTATION_CORRECTION

                const OVR::Vector3f upvector = R.Transform(upvector0);
                const OVR::Vector3f forward = R.Transform(initial_focus[eye]);
                const OVR::Vector3f lookat = position + forward * transFactor;

                kvs::Vec3 position_v3 = ::ToVec3(position);
                kvs::Vec3 lookat_v3 = ::ToVec3(lookat);
                kvs::Vec3 upvector_v3 = ::ToVec3(upvector);
                scene()->camera()->setPosition(position_v3, lookat_v3, upvector_v3);

                eyeCameraR[eye] = R;
                eyeCameraPosition[eye] = position_v3;
                eyeCameraLookAt[eye] = lookat_v3;
                eyeCameraUpVector[eye] = upvector_v3;
            }
            scene()->updateGLViewingMatrix();

//            {
//                kvs::Mat4 vm = scene()->camera()->viewingMatrix();
//                std::cerr << "frame[" << frame_index << "/" << eye << "[] : viewint matrix(from camera)=" << std::endl;
//                std::cerr << vm[0][0] << " " << vm[0][1] << vm[0][2] << " " << vm[0][3] << std::endl;
//                std::cerr << vm[1][0] << " " << vm[1][1] << vm[0][2] << " " << vm[1][3] << std::endl;
//                std::cerr << vm[2][0] << " " << vm[2][1] << vm[0][2] << " " << vm[2][3] << std::endl;
//                std::cerr << vm[3][0] << " " << vm[3][1] << vm[0][2] << " " << vm[3][3] << std::endl;

//                kvs::Mat4 vm_gl = kvs::OpenGL::ModelViewMatrix();
//                std::cerr << "frame[" << frame_index << "/" << eye << "[] : viewint matrix(from GL)=" << std::endl;
//                std::cerr << vm_gl[0][0] << " " << vm_gl[0][1] << vm_gl[0][2] << " " << vm_gl[0][3] << std::endl;
//                std::cerr << vm_gl[1][0] << " " << vm_gl[1][1] << vm_gl[0][2] << " " << vm_gl[1][3] << std::endl;
//                std::cerr << vm_gl[2][0] << " " << vm_gl[2][1] << vm_gl[0][2] << " " << vm_gl[2][3] << std::endl;
//                std::cerr << vm_gl[3][0] << " " << vm_gl[3][1] << vm_gl[0][2] << " " << vm_gl[3][3] << std::endl;
//            }


#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1_9, eye==0 ? "(A_1_1_9)   updateGLViewingMatrix()" : "(A_2_1_9)   updateGLViewingMatrix()");
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_1, eye==0 ? "(A_1_1)   " : "(A_2_1)   ");
            MAKE_AND_START_TIMER(A_1or2_2);
#endif // ENABLE_TIME_MEASURE


            // Setup the modeling matrix and render the objects.
            if (m_enable_default_paint_event) {

#ifdef DEBUG_COMPOSITOR
            std::cerr << "Screen::paintGL() : use defaultPaintEvent()" << std::endl;
#endif // DEBUG_COMPOSITOR


                this->defaultPaintEvent();

#ifdef DEBUG_SCREEN
                // Check and clear GL errors
                while ((e = glGetError()) != 0){
                    qCritical("Screen::paintGL GL HAS ERROR after call defaultPaintEvent(eye=%d) : %d", eye, e);
                }
#endif // DEBUG_SCREEN
            } else {

#ifdef DEBUG_COMPOSITOR
            std::cerr << "Screen::paintGL() : use compositor" << std::endl;
#endif // DEBUG_COMPOSITOR


            // m_compositor->setRepetitionLevel(10);
//                kvs::PaintEvent event;
//                BaseClass::eventHandler()->notify( &event );
                m_compositor->setEyeIndex(eye);
                m_compositor->update();

#ifdef DEBUG_SCREEN
                // Check and clear GL errors
                while ((e = glGetError()) != 0){
                    qCritical("Screen::paintGL GL HAS ERROR after call StochasticRenderingCompositor::update(eye=%d) : %d", eye, e);
                }
#endif // DEBUG_SCREEN

            }

#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_2, eye==0 ? "(A_1_2)   " : "(A_2_2)   ");
            MAKE_AND_START_TIMER(A_1or2_3);
#endif // ENABLE_TIME_MEASURE

            // unbind eye render target
            m_hmd.eyeRenderTexture(eye)->UnsetRenderSurface();
            m_hmd.eyeRenderTexture(eye)->Commit();

#ifdef DEBUG_SCREEN
            // Check and clear GL errors
            while ((e = glGetError()) != 0){
                qCritical("Screen::paintGL GL HAS ERROR after call eyeRenderTexture(%d)->UnsetRenderSurface & Commit() : %d", eye, e);
            }
#endif // DEBUG_SCREEN

#ifdef ENABLE_TIME_MEASURE
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2_3, eye==0 ? "(A_1_3)   " : "(A_2_3)   ");
            STOP_AND_RECORD_TIMER_WITH_ID(A_1or2, eye==0 ? "(A_1)   " : "(A_2)   ");
#endif // ENABLE_TIME_MEASURE

        } // for(eye)

#ifdef ENABLE_TIME_MEASURE
        MAKE_AND_START_TIMER(A_3);
#endif // ENABLE_TIME_MEASURE


        // recover camera position
        scene()->camera()->setPosition(camera_position, camera_lookat, camera_upvector);

#ifdef DEBUG_SCREEN
        kvs::ObjectManager *om = scene()->objectManager();
        kvs::Vec3 omExtMinCoord = om->minExternalCoord();
        kvs::Vec3 omExtMaxCoord = om->maxExternalCoord();
        kvs::Vec3 omObjMinCoord = om->minObjectCoord();
        kvs::Vec3 omObjMaxCoord = om->maxObjectCoord();
        const kvs::Mat4 om_xform = om->xform().toMatrix();

        std::fprintf (stdout, "frame %lld : \n", frame_index);
        std::fprintf (stdout, "--------------------\n");
        std::fprintf (stdout, "objMan : nObjects = %d\n", om->numberOfObjects());
        std::fprintf (stdout, "         obj min(% e, % e, % e) - max(% e, % e, % e)\n",  omObjMinCoord.x(), omObjMinCoord.y(), omObjMinCoord.z(),  omObjMaxCoord.x(), omObjMaxCoord.y(), omObjMaxCoord.z());
        std::fprintf (stdout, "         ext min(% e, % e, % e) - max(% e, % e, % e)\n",  omExtMinCoord.x(), omExtMinCoord.y(), omExtMinCoord.z(),  omExtMaxCoord.x(), omExtMaxCoord.y(), omExtMaxCoord.z());
        std::fprintf (stdout, "          xform |% e  % e  % e  %e|\n", om_xform[0][0], om_xform[0][1], om_xform[0][2], om_xform[0][3]);
        std::fprintf (stdout, "                |% e  % e  % e  %e|\n", om_xform[1][0], om_xform[1][1], om_xform[1][2], om_xform[1][3]);
        std::fprintf (stdout, "                |% e  % e  % e  %e|\n", om_xform[2][0], om_xform[2][1], om_xform[2][2], om_xform[2][3]);
        std::fprintf (stdout, "                |% e  % e  % e  %e|\n", om_xform[3][0], om_xform[3][1], om_xform[3][2], om_xform[3][3]);
        std::fprintf (stdout, "--------------------\n");
        for (int iobj = 0; iobj < om->numberOfObjects(); iobj++) {
            kvs::ObjectBase *obj = om->object(iobj);
            if (obj == nullptr) {
                std::fprintf (stdout, "object[%d/%d] is null\n", iobj, om->numberOfObjects());
                std::fprintf (stdout, "--------------------\n");
            } else {
                kvs::Vec3 extMinCoord = obj->minExternalCoord();
                kvs::Vec3 extMaxCoord = obj->maxExternalCoord();
                kvs::Vec3 objMinCoord = obj->minObjectCoord();
                kvs::Vec3 objMaxCoord = obj->maxObjectCoord();
                const kvs::Mat4 xform = obj->xform().toMatrix();
                std::fprintf (stdout, "object[%d/%d] : name=%s, %s\n", iobj, om->numberOfObjects(), obj->name().c_str(), (obj->isShown() ? "show" : "hide"));
                std::fprintf (stdout, "         obj min(% e, % e, % e) - max(% e, % e, % e)\n",  objMinCoord.x(), objMinCoord.y(), objMinCoord.z(),  objMaxCoord.x(), objMaxCoord.y(), objMaxCoord.z());
                std::fprintf (stdout, "         ext min(% e, % e, % e) - max(% e, % e, % e)\n",  extMinCoord.x(), extMinCoord.y(), extMinCoord.z(),  extMaxCoord.x(), extMaxCoord.y(), extMaxCoord.z());
                std::fprintf (stdout, "          xform |% e  % e  % e  %e|\n", xform[0][0], xform[0][1], xform[0][2], xform[0][3]);
                std::fprintf (stdout, "                |% e  % e  % e  %e|\n", xform[1][0], xform[1][1], xform[1][2], xform[1][3]);
                std::fprintf (stdout, "                |% e  % e  % e  %e|\n", xform[2][0], xform[2][1], xform[2][2], xform[2][3]);
                std::fprintf (stdout, "                |% e  % e  % e  %e|\n", xform[3][0], xform[3][1], xform[3][2], xform[3][3]);
                std::fprintf (stdout, "--------------------\n");
            }
        }
        std::fprintf (stdout, "initial h(%e, %e, %e) l(%e, %e, %e) r(%e, %e, %e)\n",
                         initial_hp.x,
                         initial_hp.y,
                         initial_hp.z,
                         initial_ep[0].x,
                         initial_ep[0].y,
                         initial_ep[0].z,
                        initial_ep[1].x,
                        initial_ep[1].y,
                        initial_ep[1].z);
        std::fprintf (stdout, "--------------------\n");
        std::fprintf (stdout, "  original camera:\n");
        std::fprintf (stdout, "     p(% e, % e, % e)\n", camera_position.x(), camera_position.y(), camera_position.z());
        std::fprintf (stdout, "     l(% e, % e, % e)\n", camera_lookat.x(), camera_lookat.y(), camera_lookat.z());
        std::fprintf (stdout, "     u(% e, % e, % e)\n", camera_upvector.x(), camera_upvector.y(), camera_upvector.z());
        std::fprintf (stdout, "     r|% e  % e  % e  % e|\n", rotation0(0,0), rotation0(0,1), rotation0(0,2), rotation0(0,3));
        std::fprintf (stdout, "      |% e  % e  % e  % e|\n", rotation0(1,0), rotation0(1,1), rotation0(1,2), rotation0(1,3));
        std::fprintf (stdout, "      |% e  % e  % e  % e|\n", rotation0(2,0), rotation0(2,1), rotation0(2,2), rotation0(2,3));
        std::fprintf (stdout, "      |% e  % e  % e  % e|\n", rotation0(3,0), rotation0(3,1), rotation0(3,2), rotation0(3,3));
        std::fprintf (stdout, "--------------------\n");
        std::fprintf (stdout, "  head p(% e, % e, % e), o(% e  % e  % e  % e)\n",
                      headPosition.x, headPosition.y, headPosition.z,
                      headOrientation.x, headOrientation.y, headOrientation.z, headOrientation.w);
        std::fprintf (stdout, "--------------------\n");
        for (int eye = 0; eye < 2; eye++) {
            std::fprintf(stdout, "eye[%d] p(% e, % e, % e) o(% e, % e, % e, % e) fov(%e, %e, %e, %e)\n",
                    eye,
                    eyePosition[eye][0],
                    eyePosition[eye][1],
                    eyePosition[eye][2],
                    eyeOrientation[eye][0],
                    eyeOrientation[eye][1],
                    eyeOrientation[eye][2],
                    eyeOrientation[eye][3],
                    fovUpTan[eye],
                    fovDownTan[eye],
                    fovLeftTan[eye],
                    fovRightTan[eye]);
        }
        std::fprintf(stdout, "r-l    p(% e, % e, % e)\n",
                    eyePosition[1][0] - eyePosition[0][0],
                    eyePosition[1][1] - eyePosition[0][1],
                    eyePosition[1][2] - eyePosition[0][2]);
        std::fprintf (stdout, "--------------------\n");
        for (int eye = 0; eye < 2; eye++) {
            std::fprintf(stdout, "eye[%d] pp(%e, %e, %e) t(%e, %e, %e) ep(%e, %e, %e)\n",
                        eye,
                        eyeCameraPP[eye].x,
                        eyeCameraPP[eye].y,
                        eyeCameraPP[eye].z,
                         eyeCameraT[eye].x,
                         eyeCameraT[eye].y,
                         eyeCameraT[eye].z,
                         eyeCameraEP[eye].x,
                         eyeCameraEP[eye].y,
                         eyeCameraEP[eye].z);
        }
        std::fprintf (stdout, "      R : left                               right\n"   );
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", eyeCameraR[0](0,0), eyeCameraR[0](0,1), eyeCameraR[0](0,2), eyeCameraR[0](0,3), eyeCameraR[1](0,0), eyeCameraR[1](0,1), eyeCameraR[1](0,2), eyeCameraR[1](0,3));
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", eyeCameraR[0](1,0), eyeCameraR[0](1,1), eyeCameraR[0](1,2), eyeCameraR[0](1,3), eyeCameraR[1](1,0), eyeCameraR[1](1,1), eyeCameraR[1](1,2), eyeCameraR[1](1,3));
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", eyeCameraR[0](2,0), eyeCameraR[0](2,1), eyeCameraR[0](2,2), eyeCameraR[0](2,3), eyeCameraR[1](2,0), eyeCameraR[1](2,1), eyeCameraR[1](2,2), eyeCameraR[1](2,3));
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", eyeCameraR[0](3,0), eyeCameraR[0](3,1), eyeCameraR[0](3,2), eyeCameraR[0](3,3), eyeCameraR[1](3,0), eyeCameraR[1](3,1), eyeCameraR[1](3,2), eyeCameraR[1](3,3));
        std::fprintf (stdout, "--------------------\n");

        for (int eye = 0; eye < 2; eye++) {
            std::fprintf(stdout, "cam[%d] p(%e, %e, %e) l(%e, %e, %e) u(%e, %e, %e)\n",
                    eye,
                    eyeCameraPosition[eye].x(),
                    eyeCameraPosition[eye].y(),
                    eyeCameraPosition[eye].z(),
                    eyeCameraLookAt[eye].x(),
                    eyeCameraLookAt[eye].y(),
                    eyeCameraLookAt[eye].z(),
                    eyeCameraUpVector[eye].x(),
                    eyeCameraUpVector[eye].y(),
                    eyeCameraUpVector[eye].z());
        }
        std::fprintf(stdout, "r-l    p(%e, %e, %e) l(%e, %e, %e) u(%e, %e, %e)\n",
                eyeCameraPosition[1].x() - eyeCameraPosition[0].x(),
                eyeCameraPosition[1].y() - eyeCameraPosition[0].y(),
                eyeCameraPosition[1].z() - eyeCameraPosition[0].z(),
                eyeCameraLookAt[1].x()   - eyeCameraLookAt[0].x(),
                eyeCameraLookAt[1].y()   - eyeCameraLookAt[0].y(),
                eyeCameraLookAt[1].z()   - eyeCameraLookAt[0].z(),
                eyeCameraUpVector[1].x() - eyeCameraUpVector[0].x(),
                eyeCameraUpVector[1].y() - eyeCameraUpVector[0].y(),
                eyeCameraUpVector[1].z() - eyeCameraUpVector[0].z());
        std::fprintf(stdout, "cam_orig p(%e, %e, %e) l(%e, %e, %e) u(%e, %e, %e), pointer=%p\n",
                     camera_position.x(),
                     camera_position.y(),
                     camera_position.z(),
                     camera_lookat.x(),
                     camera_lookat.y(),
                     camera_lookat.z(),
                     camera_upvector.x(),
                     camera_upvector.y(),
                     camera_upvector.z(),
                     scene()->camera());
        std::fprintf (stdout, "--------------------\n");

        std::fprintf (stdout, "kvs viewing matrix   left <- -> right\n"   );
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", kvsViewingMatrix[0][0][0], kvsViewingMatrix[0][0][1], kvsViewingMatrix[0][0][2], kvsViewingMatrix[0][0][3], kvsViewingMatrix[1][0][0], kvsViewingMatrix[1][0][1], kvsViewingMatrix[1][0][2], kvsViewingMatrix[1][0][3]);
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", kvsViewingMatrix[0][1][0], kvsViewingMatrix[0][1][1], kvsViewingMatrix[0][1][2], kvsViewingMatrix[0][1][3], kvsViewingMatrix[1][1][0], kvsViewingMatrix[1][1][1], kvsViewingMatrix[1][1][2], kvsViewingMatrix[1][1][3]);
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", kvsViewingMatrix[0][2][0], kvsViewingMatrix[0][2][1], kvsViewingMatrix[0][2][2], kvsViewingMatrix[0][2][3], kvsViewingMatrix[1][2][0], kvsViewingMatrix[1][2][1], kvsViewingMatrix[1][2][2], kvsViewingMatrix[1][2][3]);
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", kvsViewingMatrix[0][3][0], kvsViewingMatrix[0][3][1], kvsViewingMatrix[0][3][2], kvsViewingMatrix[0][3][3], kvsViewingMatrix[1][3][0], kvsViewingMatrix[1][3][1], kvsViewingMatrix[1][3][2], kvsViewingMatrix[1][3][3]);
        std::fprintf (stdout, "--------------------\n");

        std::fprintf (stdout, "OVR viewing matrix   left <- -> right\n"   );
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", ovrViewingMatrix[0](0,0), ovrViewingMatrix[0](0,1), ovrViewingMatrix[0](0,2), ovrViewingMatrix[0](0,3), ovrViewingMatrix[1](0,0), ovrViewingMatrix[1](0,1), ovrViewingMatrix[1](0,2), ovrViewingMatrix[1](0,3));
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", ovrViewingMatrix[0](1,0), ovrViewingMatrix[0](1,1), ovrViewingMatrix[0](1,2), ovrViewingMatrix[0](1,3), ovrViewingMatrix[1](1,0), ovrViewingMatrix[1](1,1), ovrViewingMatrix[1](1,2), ovrViewingMatrix[1](1,3));
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", ovrViewingMatrix[0](2,0), ovrViewingMatrix[0](2,1), ovrViewingMatrix[0](2,2), ovrViewingMatrix[0](2,3), ovrViewingMatrix[1](2,0), ovrViewingMatrix[1](2,1), ovrViewingMatrix[1](2,2), ovrViewingMatrix[1](2,3));
        std::fprintf (stdout, "          |% e  % e  % e  % e|  |% e  % e  % e  %e|\n", ovrViewingMatrix[0](3,0), ovrViewingMatrix[0](3,1), ovrViewingMatrix[0](3,2), ovrViewingMatrix[0](3,3), ovrViewingMatrix[1](3,0), ovrViewingMatrix[1](3,1), ovrViewingMatrix[1](3,2), ovrViewingMatrix[1](3,3));
        std::fprintf (stdout, "--------------------\n");


#endif // DEBUG_SCREEN

        if (m_widget_handler != nullptr) {
 #ifdef DEBUG_HUD
            std::cerr << "Screen::paintEvent(): call m_hmd.updateHUD()" << std::endl;
 #endif // DEBUG_HUD
            m_hmd.updateHUD();
        }

        m_fps = 1.0 / ((double)timer.elapsed()/1000.0);
        m_hmd.endFrame(frame_index);

        //std::cout << "# x26" << std::endl;
        // Render to the frame buffer.
        kvs::OpenGL::SetViewport(current_viewport);

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("Screen::paintGL GL HAS ERROR before m_hmd.renderToMirror***Image() : %d", e);
        }
#endif // DEBUG_SCREEN

        m_hmd.renderToMirror ();

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("Screen::paintGL GL HAS ERROR after m_hmd.renderToMirror***Image() : %d", e);
        }
#endif // DEBUG_SCREEN

        // Redraw.
        //kvs::OpenGL::Finish();
        kvs::OpenGL::Flush();

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("Screen::paintGL GL HAS ERROR after kvs::OpenGL::flush() : %d", e);
        }
#endif // DEBUG_SCREEN

        redraw();

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("Screen::paintGL GL HAS ERROR after redraw()) : %d", e);
        }
#endif // DEBUG_SCREEN

        glPopAttrib();

        frame_index++;

#ifdef ENABLE_TIME_MEASURE
        STOP_AND_RECORD_TIMER_WITH_ID(A_3, "(A_3)   ");
        STOP_AND_RECORD_TIMER_WITH_ID(A, "(A)   ");
        SHOW_TIMER_RESULT();
#endif // ENABLE_TIME_MEASURE

    } // if (sessionStatus.IsVisible)

    {
        // debug
        ovrErrorInfo info;
        ovr_GetLastErrorInfo(&info);

        if (!OVR_SUCCESS(info.Result)) {
            qCritical("Screen::paintGL GOT OVR ERROR %s", info.ErrorString);
        }
    }

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("Screen::paintGL GL GOT GL ERROR %d",e);
    }
#endif // DEBUG_SCREEN

    paint_mutex.unlock();
}

//    p.loadIdentity();
//    {
//        timer.start();
//        m_scene->paintFunction();
//        m_fps = 1.0 / ((double)timer.elapsed()/1000.0);
//    }
//    glPopAttrib();
//    onPaintGL();

//    // Check and clear GL errors
//    while (e = glGetError()){
//        qCritical("Screen::paintGL GL GOT GL ERROR %d",e);
//    }
//    paint_mutex.unlock();
//}


//void Screen::paintGL()
//{
//    if (QThread::currentThread() != this->thread()) {
//        qWarning("Screen::paintGL was not called from main thread, skipping frame");
//        return;
//    }
//    if (!Screen::m_gl_initialized || m_hold_paintGL){
//        qWarning("Screen either not initialized or on hold, skipping this frame");
//        return;
//    }
//    int e;
//    // Try to get paint_mutex lock.
//    // This is probably not needed any more
//    if (!paint_mutex.try_lock()){
//        qWarning("Screen could not get paint_mutex lock, skipping this frame");
//        update(); // Schedule new update
//        return;
//    }
//    // Check and clear GL errors
//    while (e = glGetError()){
//        qCritical("Screen::paintGL GL HAS ERROR BEFORE %d",e);
//    }
//    // This allows KVS to work with QOpenGLWidgets
//    // Requires future release of KVS which includes commit
//    // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
//    kvs::FrameBufferObject::m_unbind_id=QOpenGLWidget::defaultFramebufferObject();
//    QElapsedTimer timer;

//    glPushAttrib( GL_ALL_ATTRIB_BITS );
//    kvs::OpenGL::WithPushedMatrix p( GL_MODELVIEW );
//    p.loadIdentity();
//    {
//        timer.start();
//        m_scene->paintFunction();
//        m_fps = 1.0 / ((double)timer.elapsed()/1000.0);
//    }
//    glPopAttrib();
//    onPaintGL();

//    // Check and clear GL errors
//    while (e = glGetError()){
//        qCritical("Screen::paintGL GL GOT GL ERROR %d",e);
//    }
//    paint_mutex.unlock();
//}

/**
 * @brief Screen::resizeGL automatically called as part of Qt Event chain.
 *                         Do not call directly. Handles scene and  camera resizing based on devicePixelRatio.
 *
 *                         After resizing scene and camera, virtual onResizeGL is called to allow subclass to
 *                         take additional action if needed
 * @param width in pixels
 * @param height in pixels
 */
void Screen::resizeGL(int width, int height)
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG Screen::resizeGL() called." << std::endl;
#endif // DEBUG_SCREEN

    std::cout<< "Screen::resizeGL"<<std::endl;
    // This allows KVS to work with QOpenGLWidgets
    // Requires future release of KVS which includes commit
    // https://github.com/naohisas/KVS/commit/4e3a36473cc0b7cc1418e49b949e5f8d62d8c489
    kvs::FrameBufferObject::m_unbind_id=QOpenGLWidget::defaultFramebufferObject();
    const int dpr = QOpenGLWidget::devicePixelRatio();
    size_t dpr_width = static_cast<size_t>( width / dpr + 0.5 );
    size_t dpr_height = static_cast<size_t>( height / dpr + 0.5 );
    m_scene->resizeFunction( dpr_width, dpr_height );
    m_scene->camera()->setWindowSize(dpr_width,dpr_height);

    // HMD
    const int desktop_width = width;
    const int desktop_height = height;

    const int hmd_width = m_hmd.resolution().w;
    const int hmd_height = m_hmd.resolution().h;

    const float ratio = static_cast<float>(desktop_width) / hmd_width;
    const int screen_width = (ratio >= 1.0f) ? hmd_width : desktop_width;
    const int screen_height = (ratio >= 1.0f) ? hmd_height : static_cast<int>(hmd_height * ratio);
    setSize(screen_width, screen_height);

    onResizeGL(width,height);
}

/**
 * @brief Screen::initializeGL automatically called as part of Qt Event chain.
 *                             Do not call directly. Handles initialization of OpenGL context (initializeOpenGLFunctions)
 *
 *                             After context has been initialized, virtual onInitializeGL is called to allow
 *                             a subclass to take additional actions
 */
void Screen::initializeGL()
{
#ifdef DEBUG_SCREEN
    std::cout<< "MSG Screen::initializeGL called"<<std::endl;
#endif // DEBUG_SCREEN
    if (! isValid())
    {
        qWarning("Screen::initalizeGL while surface still not valid ");
        return;
    }
    initializeOpenGLFunctions();
    onInitializeGL();

    // from OculusKVS (kvs::oculus::jaea::Application)
#if defined( KVS_ENABLE_GLEW )
    GLenum result = glewInit();
    if ( result != GLEW_OK )
    {
        const GLubyte* message = glewGetErrorString( result );
        qFatal("GLEW initialization failed. ");
    }
#endif // defined( KVS_ENABLE_GLEW )

    // from OculusKVS (kvs::oculus::jaea::Screen)
#ifdef DEBUG_HUD
    std::cerr << "Screen::initializeEvent() : start" << std::endl;
#endif //DEBUG_HUD

    // Configure tracking.
    const kvs::UInt32 trac_caps = ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position;
    if (!m_hmd.configureTracking(trac_caps, 0)) {
        kvsMessageError("Failed tracking configuration");
    }

    // Configure rendering.
    if (!m_hmd.configureRendering()) {
        kvsMessageError("Failed rendering configuration.");
    }

#ifdef DEBUG_HUD
    std::cerr << "Screen::initializeEvent() : m_hmd.configureRendering() finished." << std::endl;
#endif //DEBUG_HUD

    //kvs::glut::Screen::initializeEvent();

#ifdef DEBUG_HUD
    //std::cerr << "Screen::initializeEvent() : kvs::glut::Screen::initializeEvent() finished." << std::endl;
#endif //DEBUG_HUD

    // 20210111 added by K.Yodo.
    //BaseClass::eventHandler()->attach( m_touch_controller );
    m_touch_controller->initializeEvent();

//#if 0
    // set compositor
    m_compositor = new PBVRStochasticRenderingCompositor( m_scene );
    m_compositor->setRepetitionLevel( 5 );
    //m_compositor->enableLODControl();
    m_compositor->disableLODControl();
    BaseClass::eventHandler()->attach( m_compositor );
    m_compositor->initializeEvent();
    this->m_enable_default_paint_event = false;
//#endif

    if (m_widget_handler != nullptr) {
        // ImGui initialize functions must be called after initialize glut.
        m_hmd.setWidgetHandler(m_widget_handler);
    }


#ifdef DEBUG_SCREEN
    std::cerr << "main(): create compositor" << std::endl;
#endif


    m_gl_initialized=true;


    // 20210111

}

Screen::~Screen()
{
    delete (m_scene);
    // from OculusKVS (kvs::oculus::jaea::Screen)
    m_hmd.destroy();
}

// ------------------------------------------------------------
// from OculusKVS (kvs::oculus::jaea::Screen)

void Screen::setWidgetHandler(kvs::oculus::jaea::WidgetHandler *handler) {
#ifdef DEBUG_HUD
    std::cerr << "Screen::setWidgethandler() : handler=" << handler << std::endl;
#endif // DEBUG_HUD

    m_widget_handler = handler;
    //m_hmd.setWidgetHandler(handler);
}

void Screen::defaultPaintEvent() {
    kvs::oculus::jaea::Scene *s = scene();
    kvs::oculus::jaea::PBVRObjectManager *o = s->objectManager();

    //    if (scene()->objectManager()->hasObject()) {
    if (o->hasObject()) {
        const int size = scene()->IDManager()->size();
        for (int index = 0; index < size; index++) {
            kvs::IDManager::IDPair id = scene()->IDManager()->id(index);
            kvs::ObjectBase* object = scene()->objectManager()->object(id.first);
            kvs::RendererBase* renderer = scene()->rendererManager()->renderer(id.second);
            if (object->isShown()) {
                kvs::OpenGL::PushMatrix();
                if (!kvs::oculus::jaea::pbvr::ObjectNameUtil::isToIgnoreModelingMatrix(object->name())) {
                    scene()->updateGLModelingMatrix(object);
                }
                renderer->exec(object, scene()->camera(), scene()->light());
                kvs::OpenGL::PopMatrix();
            }
        }
    } else {
        scene()->updateGLModelingMatrix();
    }
}



