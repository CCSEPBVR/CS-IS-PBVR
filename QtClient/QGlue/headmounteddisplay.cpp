#include "headmounteddisplay.h"
#include "oculus.h"
#include "ovr.h"
#include "call.h"
#include <kvs/Message>
#include <kvs/Platform>
#include <kvs/IgnoreUnusedVariable>
#include <vector>
#include "oculustexturebuffer.h"
#include "widgethandler.h"

#include <QtGlobal>

#include <iostream>

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

namespace kvs
{
namespace oculus
{
namespace jaea
{

HeadMountedDisplay::HeadMountedDisplay():
    m_handler( 0 ),
    m_descriptor( 0 ),
    hudTexture(nullptr),
    m_widget_handler(nullptr),
    m_mirror_buffer_type(DistortedBothEyeImage)
{
#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::HeadMountedDisplay() called." << std::endl;
#endif // DEBUG_SCREEN

    // 20201116 yodo@meshman.jp added
    // In OculusKVS, kvs::oculus::jaea::Initialize() called from Application::Application().
    static bool flag = true;
    if ( flag )
    {
        flag = false;
        if ( !kvs::oculus::jaea::Initialize() )
        {
            kvsMessageError( "Cannot initialize LibOVR." );
            std::exit( EXIT_FAILURE );
        }
    }

#if defined(MIRROR_IMAGE_BOTH)
   m_mirror_buffer_type = BothEyeImage;
#elif defined(MIRROR_IMAGE_LEFT_ONLY)
   m_mirror_buffer_type = LeftEyeImage;
#elif defined(MIRROR_IMAGE_RIGHT_ONLY)
   m_mirror_buffer_type = RightEyeImage;
#else
   m_mirror_buffer_type = DistortedBothEyeImage;
#endif
}

void HeadMountedDisplay::setWidgetHandler(WidgetHandler *wh)
{
#ifdef DEBUG_HUD
    std::cerr << "HMD::setWidgetHandler() : wh=" << wh << std::endl;
#endif // DEBUG_HUD

    if (m_widget_handler == wh)
    {
        // this handler is already set! -> do nothing
        return;
    }

    if (m_widget_handler != nullptr)
    {
        // TODO : invalidate old widget handler
    }

    m_widget_handler = wh;

    // too early to call!
    // texture cannot be created before QtOpenGL initialization finished.
    //initialize_hud_texture();
}

kvs::UInt32 HeadMountedDisplay::availableHmdCaps() const
{
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    return m_descriptor->AvailableHmdCaps;

#else // 0.5.0 - 0.6.0
    return m_descriptor->HmdCaps;
#endif
}

kvs::UInt32 HeadMountedDisplay::availableTrackingCaps() const
{
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    return m_descriptor->AvailableTrackingCaps;

#else // 0.5.0 - 0.6.0
    return m_descriptor->TrackingCaps;
#endif
}

kvs::UInt32 HeadMountedDisplay::defaultHmdCaps() const
{
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    return m_descriptor->DefaultHmdCaps;

#else // 0.5.0 - 0.6.0
    return m_descriptor->HmdCaps;
#endif
}

kvs::UInt32 HeadMountedDisplay::defaultTrackingCaps() const
{
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    return m_descriptor->DefaultTrackingCaps;

#else // 0.5.0 - 0.6.0
    return m_descriptor->TrackingCaps;
#endif
}

bool HeadMountedDisplay::create( const int index )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::create() called." << std::endl;
#endif // DEBUG_SCREEN


#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    ovrGraphicsLuid luid;
    ovrResult result;
    KVS_OVR_CALL( result = ovr_Create( &m_handler, &luid ) );
    if ( OVR_FAILURE( result ) ) {
        kvsMessageError( "ovr_Create() failed." );

        return false;
    }

    m_descriptor = new ovrHmdDesc;
    KVS_OVR_CALL( *m_descriptor = ovr_GetHmdDesc( m_handler ) );

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    ovrResult result;
    KVS_OVR_CALL( result = ovrHmd_Create( index, &m_handler ) );
    if ( OVR_FAILURE( result ) )
    {
        KVS_OVR_CALL( ovrHmd_CreateDebug( ovrHmd_DK1, &m_handler ) );
        if ( !m_handler ) { return false; }
    }

    m_descriptor = m_handler;

#else // 0.5.0
    KVS_OVR_CALL( m_handler = ovrHmd_Create( index ) );
    if ( !m_handler )
    {
        KVS_OVR_CALL( m_handler = ovrHmd_CreateDebug( ovrHmd_DK1 ) );
        if ( !m_handler ) { return false; }
    }

    m_descriptor = m_handler;

#endif
    return true;
}

void HeadMountedDisplay::destroy()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::destroy() called." << std::endl;
#endif // DEBUG_SCREEN


    if (hudTexture != nullptr)
    {
        delete hudTexture;
    }
    if (m_widget_handler != nullptr)
    {
        m_widget_handler->destroy();
        m_widget_handler = nullptr;
    }

    if ( m_handler )
    {
        KVS_GL_CALL( glDeleteFramebuffers( 1, &m_mirror_fbo ) );
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
        if ( m_mirror_tex ) {
            KVS_OVR_CALL( ovr_DestroyMirrorTexture( m_handler, m_mirror_tex ) );
        }
#else
        if ( m_mirror_tex ) {
            KVS_OVR_CALL( ovrHmd_DestroyMirrorTexture( m_handler, m_mirror_tex ) );
        }
#endif

        if (m_eyeRenderTexture[0] != nullptr) {
            delete m_eyeRenderTexture[0];
        }
        if (m_eyeRenderTexture[1] != nullptr) {
            delete m_eyeRenderTexture[1];
        }

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
        KVS_OVR_CALL( ovr_Destroy( m_handler ) );
        if ( m_descriptor ) { delete m_descriptor; }

#else // 0.5.0 - 0.6.0
        KVS_OVR_CALL( ovrHmd_Destroy( m_handler ) );
#endif
        m_handler = 0;
        m_descriptor = 0;
    }
}

void HeadMountedDisplay::print( std::ostream& os, const kvs::Indent& indent ) const
{
    os << indent << "Product name: " << this->productName() << std::endl;
    os << indent << "Manufacture: " << this->manufacturer() << std::endl;
    os << indent << "Vender ID: " << this->vendorId() << std::endl;
    os << indent << "Product ID: " << this->productId() << std::endl;
    os << indent << "Resolution: " << kvs::oculus::jaea::ToVec2i( this->resolution() ) << std::endl;
}

bool HeadMountedDisplay::configureTracking( const kvs::UInt32 supported_caps, const kvs::UInt32 required_caps )
{
#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::configTracking() called." << std::endl;
#endif // DEBUG_SCREEN


#if ( OVR_PRODUCT_VERSION >= 1 ) // Oculus SDK 1.x.x
    kvs::IgnoreUnusedVariable( supported_caps );
    kvs::IgnoreUnusedVariable( required_caps );
    ovrResult result;
    KVS_OVR_CALL( result = ovr_SetTrackingOriginType( m_handler, ovrTrackingOrigin_EyeLevel) );
    //KVS_OVR_CALL( result = ovr_SetTrackingOriginType( m_handler, ovrTrackingOrigin_FloorLevel ) );
    return OVR_SUCCESS( result );

#else // Oculus SDK 0.x.x
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 8, 0 )
    // Usage of ovr_ConfigureTracking is no longer needed unless you want to disable tracking features.
    // By default, ovr_Create enables the full tracking capabilities supported by any given device.
    kvs::IgnoreUnusedVariable( supported_caps );
    kvs::IgnoreUnusedVariable( required_caps );
    return true;

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    ovrResult result;
    KVS_OVR_CALL( result = ovr_ConfigureTracking( m_handler, supported_caps, required_caps ) );
    return OVR_SUCCESS( result );

#else // 0.5.0 - 0.6.0
    ovrBool result;
    KVS_OVR_CALL( result = ovrHmd_ConfigureTracking( m_handler, supported_caps, required_caps ) );
    return result == ovrTrue;
#endif
#endif
}

bool HeadMountedDisplay::configureRendering()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::configureRendering() called." << std::endl;
#endif // DEBUG_SCREEN

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    KVS_OVR_CALL( m_render_descs[0] = ovr_GetRenderDesc( m_handler, ovrEye_Left, this->defaultEyeFov(0) ) );
    KVS_OVR_CALL( m_render_descs[1] = ovr_GetRenderDesc( m_handler, ovrEye_Right, this->defaultEyeFov(1) ) );

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    KVS_OVR_CALL( m_render_descs[0] = ovrHmd_GetRenderDesc( m_handler, ovrEye_Left, this->defaultEyeFov(0) ) );
    KVS_OVR_CALL( m_render_descs[1] = ovrHmd_GetRenderDesc( m_handler, ovrEye_Right, this->defaultEyeFov(1) ) );

#else // 0.5.0
    ovrGLConfig config;
    config.OGL.Header.API = ovrRenderAPI_OpenGL;
    config.OGL.Header.BackBufferSize = OVR::Sizei( this->resolution().w, this->resolution().h );
    config.OGL.Header.Multisample = 1;
#if defined( KVS_PLATFORM_WINDOWS )
    HDC dc = wglGetCurrentDC();
    HWND wnd = WindowFromDC( dc );
    if ( !( this->hmdCaps() & ovrHmdCap_ExtendDesktop ) )
    {
        KVS_OVR_CALL( ovrHmd_AttachToWindow( m_handler, wnd, NULL, NULL ) );
    }
    config.OGL.Window = wnd;
    config.OGL.DC = dc;
#endif
    ovrFovPort fov[2] = { this->defaultEyeFov(0), this->defaultEyeFov(1) };
    kvs::UInt32 caps = ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;
    ovrBool result = ovrTrue;
    KVS_OVR_CALL( result = ovrHmd_ConfigureRendering( m_handler, &config.Config, caps, fov, m_render_descs ) );
    if ( result == ovrFalse ) { return false; }

    KVS_OVR_CALL( ovrHmd_SetEnabledCaps( m_handler, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction ) );
    KVS_OVR_CALL( ovrHmd_DismissHSWDisplay( m_handler ) );
#endif

    this->update_viewport();

#ifdef DEBUG_HUD
    std::cerr << "HeadMountedDisplay::configureRendering() : initialize textures" << std::endl;
#endif // DEBUG_HUD

    return this->initialize_render_texture() && this->initialize_mirror_texture()
    && this->initialize_hud_texture();
}

void HeadMountedDisplay::beginFrame( const kvs::Int64 frame_index )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::beginFrame() called." << std::endl;
#endif // DEBUG_SCREEN

#if ( KVS_OVR_MAJOR_VERSION >= 1 ) // Oculus SDK 1.x.x
    ovrResult result;
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 19, 0 )
    KVS_OVR_CALL( result = ovr_WaitToBeginFrame( m_handler, frame_index ) );
    KVS_OVR_CALL( result = ovr_BeginFrame( m_handler, frame_index ) );
    KVS_ASSERT( OVR_SUCCESS( result ) );
#endif

    // // Get the crrent color texture ID.
    // const ovrTextureSwapChain& color_textures = m_layer_data.ColorTexture[0];
    // int current_index = 0;
    // KVS_OVR_CALL( result = ovr_GetTextureSwapChainCurrentIndex( m_handler, color_textures, &current_index ) );
    // KVS_ASSERT( OVR_SUCCESS( result ) );

    // // Bind the frame buffer object.
    // m_framebuffer.bind();
    // GLuint tex_id = 0;
    // KVS_OVR_CALL( result = ovr_GetTextureSwapChainBufferGL( m_handler, color_textures, current_index, &tex_id ) );
    // KVS_ASSERT( OVR_SUCCESS( result ) );
    // KVS_GL_CALL( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 ) );

#else // Oculus SDK 0.x.x
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    // ovrHmd_BeginFrame was removed and ovrHmd_EndFrame was replaced with ovrHmd_SubmitFrame.
    // A list of layer pointers is passed into ovrHmd_SubmitFrame.
    kvs::IgnoreUnusedVariable( frame_index );

    // Get the crrent color texture ID.
    ovrSwapTextureSet* color_textures = m_layer_data.ColorTexture[0];
    const int texture_count = color_textures->TextureCount;
    const int prev_index = color_textures->CurrentIndex;
    const int current_index = ( prev_index + 1 ) % texture_count;
    color_textures->CurrentIndex = current_index;

    // Bind the frame buffer object.
    m_framebuffer.bind();
    const GLuint tex_id = reinterpret_cast<ovrGLTexture *>( &color_textures->Textures[ current_index ] )->OGL.TexId;
    KVS_GL_CALL( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 ) );

#else // 0.5.0
    ovrFrameTiming result;
    KVS_OVR_CALL( result = ovrHmd_BeginFrame( m_handler, frame_index ) );

    // Bind the frame buffer object.
    m_framebuffer.bind();
#endif
#endif

    this->update_eye_poses( frame_index );

    m_posTimewarpProjectionDesc = {};
}

void HeadMountedDisplay::endFrame( const kvs::Int64 frame_index )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::endFrame() called." << std::endl;
#endif // DEBUG_SCREEN

#if KVS_OVR_VERSION_LESS_OR_EQUAL( 0, 5, 0 ) // Oculus SDK 0.5.0
    kvs::IgnoreUnusedVariable( frame_index );
    m_framebuffer.unbind();

    const ovrTexture* color_texture = &m_color_textures[0].Texture;
    KVS_OVR_CALL( ovrHmd_EndFrame( m_handler, m_eye_poses, color_texture ) );

#else // 0.6.0 - 1.x.x
    // m_framebuffer.unbind();
    // KVS_OVR_CALL( ovr_CommitTextureSwapChain( m_handler, m_layer_data.ColorTexture[0] ) );

    // Set view-scale descriptor.
    ovrViewScaleDesc view_scale_desc;
    view_scale_desc.HmdSpaceToWorldScaleInMeters = 1.0f;
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 17, 0 )
    view_scale_desc.HmdToEyePose[0] = m_render_descs[0].HmdToEyePose;
    view_scale_desc.HmdToEyePose[1] = m_render_descs[1].HmdToEyePose;
#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 3, 0 )
    view_scale_desc.HmdToEyeOffset[0] = m_render_descs[0].HmdToEyeOffset;
    view_scale_desc.HmdToEyeOffset[1] = m_render_descs[1].HmdToEyeOffset;
#else
    view_scale_desc.HmdToEyeViewOffset[0] = m_render_descs[0].HmdToEyeViewOffset;
    view_scale_desc.HmdToEyeViewOffset[1] = m_render_descs[1].HmdToEyeViewOffset;
#endif

    std::vector < ovrLayerHeader* > layers;

    ovrLayerEyeFovDepth ld = { };
    ld.Header.Type = ovrLayerType_EyeFovDepth;
    ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft; // Because OpenGL.

    for (size_t eye = 0; eye < 2; eye++) {
        ld.ColorTexture[eye] = eyeRenderTexture(eye)->ColorTextureChain;
        ld.DepthTexture[eye] = eyeRenderTexture(eye)->DepthTextureChain;
        ld.Viewport[eye].Pos.x = 0;
        ld.Viewport[eye].Pos.y = 0;
        ld.Viewport[eye].Size.w = eyeRenderTexture(eye)->getTexureWidth();
        ld.Viewport[eye].Size.h = eyeRenderTexture(eye)->getTexureHeight();
        ld.Fov[eye] = defaultEyeFov(eye);
        ld.RenderPose[eye] = m_eye_poses[eye];
        ld.SensorSampleTime = m_sensorSampleTime;

        OVR::Matrix4f proj = ovrMatrix4f_Projection(defaultEyeFov(eye), OCULUS_NEAR, OCULUS_FAR, ovrProjection_None);
        m_posTimewarpProjectionDesc = ovrTimewarpProjectionDesc_FromProjection(proj, ovrProjection_None);

        // std::cerr
        // << "#### " <<defaultEyeFov(eye).LeftTan
        // << ", " <<defaultEyeFov(eye).RightTan
        // << ", " <<defaultEyeFov(eye).DownTan
        // << ", " <<defaultEyeFov(eye).UpTan
        //         << std::endl;
    }
    ld.ProjectionDesc = m_posTimewarpProjectionDesc;
    layers.push_back(&ld.Header);

    // // Set layer eye fov.
    // ovrLayerEyeFov layer_eye_fov;
    // layer_eye_fov.Header.Type = ovrLayerType_EyeFov;
    // layer_eye_fov.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
    // layer_eye_fov.ColorTexture[0] = m_layer_data.ColorTexture[0];
    // layer_eye_fov.ColorTexture[1] = m_layer_data.ColorTexture[1];
    // layer_eye_fov.Viewport[0] = m_viewports[0];
    // layer_eye_fov.Viewport[1] = m_viewports[1];
    // layer_eye_fov.Fov[0] = this->defaultEyeFov(0);
    // layer_eye_fov.Fov[1] = this->defaultEyeFov(1);
    // layer_eye_fov.RenderPose[0] = m_eye_poses[0];
    // layer_eye_fov.RenderPose[1] = m_eye_poses[1];

    //ovrLayerHeader* layers = &layer_eye_fov.Header;
    //layers.push_back(&layer_eye_fov.Header);

    ovrLayerQuad quad;
    if (m_widget_handler != nullptr && hudTexture != nullptr)
    {
        quad.Header.Type = ovrLayerType_Quad;
        quad.Header.Flags = ovrLayerFlag_HeadLocked;
        quad.ColorTexture = hudTexture->ColorTextureChain;
        quad.Viewport.Pos.x = 0;
        quad.Viewport.Pos.y = 0;
        quad.Viewport.Size.w = hudTexture->texWidth;
        quad.Viewport.Size.h = hudTexture->texHeight;

        quad.QuadPoseCenter.Position.x = 0;
        quad.QuadPoseCenter.Position.y = 0;
        quad.QuadPoseCenter.Position.z = -1;
        quad.QuadPoseCenter.Orientation.x = 1; 
        quad.QuadPoseCenter.Orientation.y = 0; 
        quad.QuadPoseCenter.Orientation.z = 0; 
        quad.QuadPoseCenter.Orientation.w = 0;
        quad.QuadSize.x = 1.0f;
        quad.QuadSize.y = 1.0f;

        layers.push_back(&quad.Header);
    }
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 19, 0 )
    KVS_OVR_CALL( ovr_EndFrame( m_handler, frame_index, &view_scale_desc, layers.data(), layers.size() ) );
#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    KVS_OVR_CALL( ovr_SubmitFrame( m_handler, frame_index, &view_scale_desc, layers.data(), layers.size() ) );
#else
    KVS_OVR_CALL( ovrHmd_SubmitFrame( m_handler, frame_index, &view_scale_desc, layers.data(), layers.size() ) );
#endif
#endif
}

double HeadMountedDisplay::frameTiming( const kvs::Int64 frame_index ) const
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::frameTiming() called." << std::endl;
#endif // DEBUG_SCREEN

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 8, 0 )
    double result;
    KVS_OVR_CALL( result = ovr_GetPredictedDisplayTime( m_handler, frame_index ) );
    return result;

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    ovrFrameTiming timing;
    KVS_OVR_CALL( timing = ovr_GetFrameTiming( m_handler, frame_index ) );
    return timing.DisplayMidpointSeconds;

#else // 0.5.0
    kvs::IgnoreUnusedVariable( frame_index );
    return kvs::oculus::TimeInSecond();
#endif
}


void HeadMountedDisplay::renderToMirror()
{
   int e;

#ifdef DEBUG_SCREEN
   std::cout << "MSG HeadMountedDisplay::renderToMrror() called." << std::endl;
#endif  // DEBUG_SCREEN

    this->bind_mirror_buffer();

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::renderToMirror GL HAS ERROR after call bind_mirror_buffer : %d", e);
    }
#endif // DEBUG_SCREEN


    const bool flip = true;
    const kvs::Vec4i mirror_viewport( 0, 0, this->resolution().w, this->resolution().h );
    this->blit_mirror_buffer( kvs::Vec4( mirror_viewport ), flip );

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::renderToMirror GL HAS ERROR after call blit_mirror_buffer : %d", e);
    }
#endif // DEBUG_SCREEN
    this->unbind_mirror_buffer();

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::renderToMirror GL HAS ERROR after call unbind_mirror_buffer : %d", e);
    }
#endif // DEBUG_SCREEN

}

#if 0
void HeadMountedDisplay::renderToMirrorLeftEyeImage()
{
    // TODO : re-implement me!
    // const bool distorted = false;
    // this->bind_mirror_buffer( distorted );

    // const int mirror_x = m_layer_data.Viewport[0].Pos.x;
    // const int mirror_y = m_layer_data.Viewport[0].Pos.y;
    // const int mirror_width = m_layer_data.Viewport[0].Size.w;
    // const int mirror_height = m_layer_data.Viewport[0].Size.h;
    // const kvs::Vec4i mirror_viewport( mirror_x, mirror_y, mirror_width, mirror_height );
    // const bool flip = false;
    // this->blit_mirror_buffer( kvs::Vec4( mirror_viewport ), flip );

    // this->unbind_mirror_buffer();
}

void HeadMountedDisplay::renderToMirrorRightEyeImage()
{
    // TODO : re-implement me!
    // const bool distorted = false;
    // this->bind_mirror_buffer( distorted );

    // const int mirror_x = m_layer_data.Viewport[1].Pos.x;
    // const int mirror_y = m_layer_data.Viewport[1].Pos.y;
    // const int mirror_width = m_layer_data.Viewport[1].Size.w;
    // const int mirror_height = m_layer_data.Viewport[1].Size.h;
    // const kvs::Vec4i mirror_viewport( mirror_x, mirror_y, mirror_width, mirror_height );

    // const bool flip = false;
    // this->blit_mirror_buffer( kvs::Vec4( mirror_viewport ), flip );

    // this->unbind_mirror_buffer();
}

void HeadMountedDisplay::renderToMirrorBothEyeImage()
{
    // TODO : re-implement me!
    // const bool distorted = false;
    // this->bind_mirror_buffer( distorted );

    // const int mirror_x = m_layer_data.Viewport[0].Pos.x;
    // const int mirror_y = m_layer_data.Viewport[0].Pos.y;
    // const int mirror_width = m_layer_data.Viewport[0].Size.w + m_layer_data.Viewport[1].Size.w;
    // const int mirror_height = m_layer_data.Viewport[0].Size.h;
    // const kvs::Vec4i mirror_viewport( mirror_x, mirror_y, mirror_width, mirror_height );
    // const bool flip = false;
    // this->blit_mirror_buffer( kvs::Vec4( mirror_viewport ), flip );

    // this->unbind_mirror_buffer();
}

void HeadMountedDisplay::renderToMirrorDistortedBothEyeImage()
{
   int e;

#ifdef DEBUG_SCREEN
   std::cout << "MSG HeadMountedDisplay::renderToMrrorDistortedBothEyeImage() called." << std::endl;
#endif  // DEBUG_SCREEN

    const bool distorted = true;
    this->bind_mirror_buffer( distorted );

    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::renderToMirrorDistortedBothEyeImage GL HAS ERROR after call bind_mirror_buffer : %d", e);
    }


    const bool flip = true;
    const kvs::Vec4i mirror_viewport( 0, 0, this->resolution().w, this->resolution().h );
    this->blit_mirror_buffer( kvs::Vec4( mirror_viewport ), flip );

    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::renderToMirrorDistortedBothEyeImage GL HAS ERROR after call blit_mirror_buffer : %d", e);
    }


    this->unbind_mirror_buffer();

    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::renderToMirrorDistortedBothEyeImage GL HAS ERROR after call unbind_mirror_buffer : %d", e);
    }

}
#endif

void HeadMountedDisplay::resetTracking()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::resetTracking() called." << std::endl;
#endif // DEBUG_SCREEN

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 3, 0 )
    KVS_OVR_CALL( ovr_RecenterTrackingOrigin( m_handler ) );

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    KVS_OVR_CALL( ovr_RecenterPose( m_handler ) );

#else // 0.5.0 - 0.6.0
    KVS_OVR_CALL( ovrHmd_RecenterPose( m_handler ) );
#endif
}

ovrTrackingState HeadMountedDisplay::trackingState( const double absolute_time, const bool latency ) const
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::trackingState() called." << std::endl;
#endif // DEBUG_SCREEN

    ovrTrackingState result;
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 8, 0 )
    const ovrBool latency_marker = ( latency ) ? ovrTrue : ovrFalse;
    KVS_OVR_CALL( result = ovr_GetTrackingState( m_handler, absolute_time, latency_marker ) );

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    kvs::IgnoreUnusedVariable( latency );
    KVS_OVR_CALL( result = ovr_GetTrackingState( m_handler, absolute_time ) );

#else // 0.5.0 - 0.6.0
    kvs::IgnoreUnusedVariable( latency );
    KVS_OVR_CALL( result = ovrHmd_GetTrackingState( m_handler, absolute_time ) );
#endif
    return result;
}

ovrInputState HeadMountedDisplay::inputState( const ovrControllerType type ) const
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::inputState() called." << std::endl;
#endif // DEBUG_SCREEN

    ovrInputState result;
    KVS_OVR_CALL( ovr_GetInputState( m_handler, type, &result ) );
    return result;
}

void HeadMountedDisplay::update_viewport()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::update_viewport() called." << std::endl;
#endif // DEBUG_SCREEN

    // Left eye info.
    const ovrEyeType eye0 = ovrEye_Left;
    const ovrFovPort fov0 = this->defaultEyeFov(0);
    const ovrSizei tex0 = this->fov_texture_size( eye0, fov0, 1.0f );

    // Right eye info.
    const ovrEyeType eye1 = ovrEye_Right;
    const ovrFovPort fov1 = this->defaultEyeFov(1);
    const ovrSizei tex1 = this->fov_texture_size( eye1, fov1, 1.0f );

    // // Rendering buffer size.
    // m_buffer_size.w = tex0.w + tex1.w;
    // m_buffer_size.h = kvs::Math::Max( tex0.h, tex1.h );

    // Viewport of each eye.
    m_viewports[0].Pos = OVR::Vector2i( 0, 0 );
    m_viewports[0].Size = OVR::Sizei( tex0.w, tex0.h );
    m_viewports[1].Pos = OVR::Vector2i( 0, 0 );
    m_viewports[1].Size =  OVR::Sizei( tex1.w, tex1.h );
    // m_viewports[0].Pos = OVR::Vector2i( 0, 0 );
    // m_viewports[0].Size = OVR::Sizei( m_buffer_size.w / 2, m_buffer_size.h );
    // m_viewports[1].Pos = OVR::Vector2i( ( m_buffer_size.w + 1 ) / 2, 0 );
    // m_viewports[1].Size = m_viewports[0].Size;

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    // m_layer_data.Viewport[0] = m_viewports[0];
    // m_layer_data.Viewport[1] = m_viewports[1];

#else // 0.5.0
    m_color_textures[0].OGL.Header.API = ovrRenderAPI_OpenGL;
    m_color_textures[0].OGL.Header.TextureSize = m_buffer_size;
    m_color_textures[0].OGL.Header.RenderViewport = m_viewports[0];
    m_color_textures[1] = m_color_textures[0];
    m_color_textures[1].OGL.Header.RenderViewport = m_viewports[1];
#endif
}

// TODO : call me!
static ovrGraphicsLuid GetDefaultAdapterLuid() {
    ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
    IDXGIFactory* factory = nullptr;

    if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
        IDXGIAdapter* adapter = nullptr;

        if (SUCCEEDED(factory->EnumAdapters(0, &adapter))) {
            DXGI_ADAPTER_DESC desc;

            adapter->GetDesc(&desc);
            memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
            adapter->Release();
        }

        factory->Release();
    }
#endif

    return luid;
}
static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs) {
    return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}

bool HeadMountedDisplay::initialize_render_texture()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::initialize_render_texture() called." << std::endl;
#endif // DEBUG_SCREEN


    // Make eye render buffers
    for (int eye = 0; eye < 2; ++eye) {
        ovrSizei idealTextureSize = ovr_GetFovTextureSize(m_handler, ovrEyeType(eye), defaultEyeFov(eye), 1);

        std::cout << "RenderTexture for eye[" << eye << "] : (" << idealTextureSize.w << ", " << idealTextureSize.h << ")" << std::endl;

        m_eyeRenderTexture[eye] = new OculusTextureBuffer(m_handler, idealTextureSize.w, idealTextureSize.h, 1);
        //m_eyeRenderTexture[eye]->BackgroundColor(1.0f * eye, 0.0f, 1.0f, 1.0f);
        if (!m_eyeRenderTexture[eye]->ColorTextureChain || !m_eyeRenderTexture[eye]->DepthTextureChain) {
            //if (retryCreate)goto Done;
            kvsMessageError("Failed to create texture.");
        }
    }

    {
        ovrErrorInfo errInfo;
        for (int ierr = 0;;ierr++) {
            ovr_GetLastErrorInfo(&errInfo);
            if (OVR_SUCCESS(errInfo.Result)) {
                break;
            }
            std::fprintf (stdout, "HeadMountedDisplay::initialize_render_texture(): get OVR error [%d] %s\n", ierr, errInfo.ErrorString);
        }
    }

// #if ( KVS_OVR_MAJOR_VERSION >= 1 ) // Oculus SDK 1.x.x
//     const ovrTextureSwapChainDesc chain = {
//         ovrTexture_2D, // Type
//         OVR_FORMAT_R8G8B8A8_UNORM_SRGB, // Format
//         1, // ArraySize
//         m_buffer_size.w, // Width
//         m_buffer_size.h, // Height
//         1, // MipLevels
//         1, // SampleCount
//         ovrFalse, // StaticImage
//         0, // MiscFlags
//         0 // BindFlags
//     };

//     // Create color textures
//     ovrResult result;
//     ovrTextureSwapChain color_textures;
//     KVS_OVR_CALL( result = ovr_CreateTextureSwapChainGL( m_handler, &chain, &color_textures ) );
//     if ( OVR_FAILURE( result ) )
//     {
//         kvsMessageError("Cannot create color textures.");
//         return false;
//     }

//     int texture_count = 0;
//     KVS_OVR_CALL( result = ovr_GetTextureSwapChainLength( m_handler, color_textures, &texture_count ) );
//     if ( OVR_FAILURE( result ) || !texture_count )
//     {
//         kvsMessageError("Cannot get number of color textures.");
//         return false;
//     }

//     for ( int i = 0; i < texture_count; i++ )
//     {
//         GLuint tex_id = 0;
//         KVS_OVR_CALL( result = ovr_GetTextureSwapChainBufferGL( m_handler, color_textures, i, &tex_id ) );
//         KVS_ASSERT( OVR_SUCCESS( result ) );        //KVS_ASSERT( OVR_FAILURE( result ) );
//         KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, tex_id ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );
//         KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, 0 ) );
//     }

//     m_layer_data.ColorTexture[0] = color_textures;
//     m_layer_data.ColorTexture[1] = color_textures;

//     // Create a depth buffer.
//     m_depth_buffer.setInternalFormat( GL_DEPTH_COMPONENT24 );
//     m_depth_buffer.create( m_buffer_size.w, m_buffer_size.h );

//     // Create a frame buffer object.
//     m_framebuffer.create();
//     m_framebuffer.bind();
//     m_framebuffer.attachDepthRenderBuffer( m_depth_buffer );
//     m_framebuffer.unbind();

// #else // Oculus SDK 0.x.x
// #if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
//     // Create color textures
//     ovrResult result;
//     ovrSwapTextureSet* color_textures;
// #if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
//     KVS_OVR_CALL( result = ovr_CreateSwapTextureSetGL(
//                       m_handler,
//                       GL_SRGB8_ALPHA8,
//                       m_buffer_size.w, m_buffer_size.h,
//                       &color_textures ) );
// #else
//     KVS_OVR_CALL( result = ovrHmd_CreateSwapTextureSetGL(
//                       m_handler,
//                       GL_RGBA,
//                       m_buffer_size.w, m_buffer_size.h,
//                       &color_textures ) );
// #endif
//     if ( OVR_FAILURE( result ) )
//     {
//         kvsMessageError("Cannot create color textures.");
//         return false;
//     }

//     const int texture_count = color_textures->TextureCount;
//     for ( int i = 0; i < texture_count; i++ )
//     {
//         ovrGLTexture* tex = (ovrGLTexture*)&color_textures->Textures[i];
//         KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, tex->OGL.TexId ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
//         KVS_GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );
//         KVS_GL_CALL( glBindTexture( GL_TEXTURE_2D, 0 ) );
//     }

//     m_layer_data.ColorTexture[0] = color_textures;
//     m_layer_data.ColorTexture[1] = color_textures;

//     // Create a depth buffer.
//     m_depth_buffer.setInternalFormat( GL_DEPTH_COMPONENT24 );
//     m_depth_buffer.create( m_buffer_size.w, m_buffer_size.h );

//     // Create a frame buffer object.
//     m_framebuffer.create();
//     m_framebuffer.bind();
//     m_framebuffer.attachDepthRenderBuffer( m_depth_buffer );
//     m_framebuffer.unbind();

// #else
//     // Create a color buffer.
//     m_color_buffer.setMagFilter( GL_LINEAR );
//     m_color_buffer.setMinFilter( GL_LINEAR );
//     m_color_buffer.setWrapS( GL_CLAMP_TO_EDGE );
//     m_color_buffer.setWrapT( GL_CLAMP_TO_EDGE );
//     m_color_buffer.setPixelFormat( GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE );
//     m_color_buffer.create( m_buffer_size.w, m_buffer_size.h );

//     m_color_textures[0].OGL.TexId = m_color_buffer.id();
//     m_color_textures[1].OGL.TexId = m_color_buffer.id();

//     // Create a depth buffer.
//     m_depth_buffer.setInternalFormat( GL_DEPTH_COMPONENT24 );
//     m_depth_buffer.create( m_buffer_size.w, m_buffer_size.h );

//     // Create a frame buffer object.
//     m_framebuffer.create();
//     m_framebuffer.bind();
//     m_framebuffer.attachColorTexture( m_color_buffer );
//     m_framebuffer.attachDepthRenderBuffer( m_depth_buffer );
//     m_framebuffer.unbind();

// #endif
// #endif

    return true;
}

bool HeadMountedDisplay::initialize_hud_texture()
{

#ifdef DEBUG_HUD
    std::cout << "MSG HeadMountedDisplay::initialize_hud_texture() called." << std::endl;
#endif // DEBUG_HUD

    if (m_widget_handler == nullptr)
    {
        return true;
    }

    hudTexture = new OculusTextureBuffer(m_handler, m_widget_handler->getTextureWidth(), m_widget_handler->getTextureHeight(), 1);
    if (!hudTexture->ColorTextureChain)
    {
        std::cout << "HeadMountedDisplay::initialize_hud_texture() : failed to create color texture chain!!" << std::endl;
        return false;
    }
    if (!hudTexture->DepthTextureChain)
    {
        std::cout << "HeadMountedDisplay::initialize_hud_texture() : failed to create depth texture chain!!" << std::endl;
        return false;
    }
    kvs::RGBAColor bg = m_widget_handler->getBackgroundColor();
    float r = ((float)(bg.r()))/256.0f;
    float g = ((float)(bg.g()))/256.0f;
    float b = ((float)(bg.b()))/256.0f;
    float a = bg.a();
    hudTexture->BackgroundColor(r, g, b, a);

#ifdef DEBUG_HUD
    std::cout << "MSG HeadMountedDisplay::initialize_hud_texture() will call m_widget_handler->initialize()" << std::endl;
#endif // DEBUG_HUD

    bool ret = m_widget_handler->initialize();

#ifdef DEBUG_HUD
    std::cout << "MSG HeadMountedDisplay::initialize_hud_texture() called m_widget_handler->initialize() with return value " << ret << std::endl;
#endif // DEBUG_HUD

    return ret;
}

bool HeadMountedDisplay::initialize_mirror_texture()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::initialize_mirror_texture() called." << std::endl;
#endif // DEBUG_SCREEN

    int e;
    ovrResult result;
    const int width = this->resolution().w;
    const int height = this->resolution().h;

#if ( KVS_OVR_MAJOR_VERSION >= 1 ) // Oculus SDK 1.x.x
    // // Mirror texture descriptor.
    // const ovrMirrorTextureDesc mirror = {
    //     OVR_FORMAT_R8G8B8A8_UNORM_SRGB, // Format
    //     width, // Width
    //     height, // Height
    //     0
    // };

    // // Create a mirror texture.
    // KVS_OVR_CALL( result = ovr_CreateMirrorTextureGL( m_handler, &mirror, &m_mirror_tex ) );
    // if ( OVR_FAILURE( result ) )
    // {
    //     kvsMessageError( "Cannot create mirror texture." );
    //     return false;
    // }

    // // Create a mirror frame buffer object.
    // KVS_GL_CALL( glGenFramebuffers( 1, &m_mirror_fbo ) );

#ifdef DEBUG_SCREEN
    std::cout << "HeadMountedDisplay::initialize_mirror_texture() initialize texture with (" << width << ", " << height << ")" << std::endl;
#endif // DEBUG_SCREEN

    ovrMirrorOptions mirror_options[SizeOfEyeImage];
    mirror_options[LeftEyeImage]          = ovrMirrorOption_LeftEyeOnly;
    mirror_options[RightEyeImage]         = ovrMirrorOption_RightEyeOnly;
    mirror_options[BothEyeImage]          = ovrMirrorOption_Default;
    mirror_options[DistortedBothEyeImage] = ovrMirrorOption_PostDistortion;

    KVS_GL_CALL( glGenFramebuffers(1, &m_mirror_fbo) );
    {
        ovrMirrorTextureDesc desc;
        memset(&desc, 0, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
        desc.MirrorOptions = mirror_options[m_mirror_buffer_type];
        //desc.MirrorOptions = ovrMirrorOption_PostDistortion;

        // Create mirror texture and an FBO used to copy mirror texture to back buffer
        KVS_OVR_CALL(result = ovr_CreateMirrorTextureWithOptionsGL(m_handler, &desc, &m_mirror_tex));
        // Configure the mirror read buffer
        GLuint texId;
        KVS_OVR_CALL( ovr_GetMirrorTextureBufferGL(m_handler, m_mirror_tex, &texId) );

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call ovr_GetMirrorTextureBufferGL : %d", e);
        }
#endif // DEBUG_SCREEN

        // 20201118 yodo@meshman.jp add
        // save & restore framebuffer id

        m_mirror_fbo_old = kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING );

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call kvs::OpenGL::Integer(GL_FRAMEBUFFER_BINDING) : %d", e);
        }
#endif // DEBUG_SCREEN

    #ifdef DEBUG_SCREEN
        std::cout << "HeadMountedDisplay::initialize_mirror_texture() fbo old=" << m_mirror_fbo_old << ", new=" << m_mirror_fbo << std::endl;
    #endif // DEBUG_SCREEN


#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call glGenFramebuffers : %d", e);
        }
#endif // DEBUG_SCREEN

        KVS_GL_CALL( glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirror_fbo) );

#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call glBindFramebuffer(m_mirror_fbo) : %d", e);
        }
#endif // DEBUG_SCREEN
        KVS_GL_CALL( glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0) );
#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call glFramebufferTexture2D : %d", e);
        }
#endif // DEBUG_SCREEN
        //KVS_GL_CALL( glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_mirror_fbo_old) );
        KVS_GL_CALL( glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0) );
#ifdef DEBUG_SCREEN
        // Check and clear GL errors
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call glFramebufferRenderBuffer : %d", e);
        }
#endif // DEBUG_SCREEN
        KVS_GL_CALL( glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirror_fbo_old) );

        // Check and clear GL errors
#ifdef DEBUG_SCREEN
        while ((e = glGetError()) != 0){
            qCritical("HeadMountedDisplay::initialize_mirror_buffer GL HAS ERROR after call glBindFramebuffer(m_mirror_fbo_old) : %d", e);
        }
#endif // DEBUG_SCREEN
    }
    // Turn off vsync to let the compositor do its magic
    //wglSwapIntervalEXT(0);

    // FloorLevel will give tracking poses where the floor height is 0
    KVS_OVR_CALL( ovr_SetTrackingOriginType(m_handler, ovrTrackingOrigin_FloorLevel) );

#else // Oculus SDK 0.x.x

    // Create a mirror texture.
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    KVS_OVR_CALL( result = ovr_CreateMirrorTextureGL( m_handler, GL_SRGB8_ALPHA8, width, height, reinterpret_cast<ovrTexture **>(&m_mirror_tex) ) );
#else
    KVS_OVR_CALL( result = ovrHmd_CreateMirrorTextureGL( m_handler, GL_SRGB8_ALPHA8, width, height, reinterpret_cast<ovrTexture **>(&m_mirror_tex) ) );
#endif
    if ( OVR_FAILURE( result ) )
    {
        kvsMessageError( "Cannot create mirror texture." );
        return false;
    }

    // Create a mirror frame buffer object.
    KVS_GL_CALL( glGenFramebuffers( 1, &m_mirror_fbo ) );
    KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ) );
    KVS_GL_CALL( glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mirror_txt->OGL.TexId, 0 ) );
    KVS_GL_CALL( glFramebufferRenderbuffer( GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 ) );
    KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 ) );
#endif

    {
        ovrErrorInfo errInfo;
        for (int ierr = 0;; ierr++) {
            ovr_GetLastErrorInfo(&errInfo);
            if (OVR_SUCCESS(errInfo.Result)) {
                break;
            }
            std::fprintf (stdout, "HeadMountedDisplay::initialize_mirror_texture() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
        }
    }

#ifdef DEBUG_SCREEN
    std::cout << "HeadMountedDisplay::initialize_mirror_texture() finished." << std::endl;
#endif // DEBUG_SCREEN

    return true;
}

void HeadMountedDisplay::updateHUD()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::updateHUD() called." << std::endl;
#endif // DEBUG_SCREEN


#ifdef DEBUG_HUD
std::cerr << "HMD::updateHUD() : start" << std::endl;
#endif // DEBUG_HUD

    if(m_widget_handler == nullptr)
    {
        return;
    }
    
    hudTexture->SetAndClearRenderSurface();

#ifdef DEBUG_HUD
std::cerr << "HMD::updateHUD() : hudTexture->SetAndClearRenderSurface() finished." << std::endl;
#endif // DEBUG_HUD

    m_widget_handler->update();

#ifdef DEBUG_HUD
std::cerr << "HMD::updateHUD() : m_widget_handler->update() finished." << std::endl;
#endif // DEBUG_HUD

    hudTexture->UnsetRenderSurface();
    hudTexture->Commit();

#ifdef DEBUG_HUD
std::cerr << "HMD::updateHUD() : hudTexture^>UnsetRenderSurface() & Commit() finished." << std::endl;
#endif // DEBUG_HUD
}

void HeadMountedDisplay::update_eye_poses( const kvs::Int64 frame_index )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::update_eye_poses() called." << std::endl;
#endif // DEBUG_SCREEN


#if ( KVS_OVR_MAJOR_VERSION >= 1 ) // Oculus SDK 1.x.x
    const double timing = this->frameTiming( frame_index );
    const ovrTrackingState state = this->trackingState( timing );

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 17, 0 )
    const ovrPosef poses[2] = {
        m_render_descs[0].HmdToEyePose,
        m_render_descs[1].HmdToEyePose
    };
    KVS_OVR_CALL( ovr_CalcEyePoses( state.HeadPose.ThePose, poses, m_eye_poses ) );

#elif KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 3, 0 )
    const ovrVector3f offsets[2] = {
        m_render_descs[0].HmdToEyeOffset,
        m_render_descs[1].HmdToEyeOffset
    };
    KVS_OVR_CALL( ovr_CalcEyePoses( state.HeadPose.ThePose, offsets, m_eye_poses ) );

#else
    const ovrVector3f offsets[2] = {
        m_render_descs[0].HmdToEyeViewOffset,
        m_render_descs[1].HmdToEyeViewOffset
    };
    KVS_OVR_CALL( ovr_CalcEyePoses( state.HeadPose.ThePose, offsets, m_eye_poses ) );
#endif

#else // Oculus SDK 0.x.x
    const ovrVector3f offsets[2] = {
        m_render_descs[0].HmdToEyeViewOffset,
        m_render_descs[1].HmdToEyeViewOffset
    };

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    const double timing = this->frameTiming( frame_index );
    const ovrTrackingState state = this->trackingState( timing );
    KVS_OVR_CALL( ovr_CalcEyePoses( state.HeadPose.ThePose, offsets, m_eye_poses ) );

#else // 0.5.0
    KVS_OVR_CALL( ovrHmd_GetEyePoses( m_handler, frame_index, offsets, m_eye_poses, NULL ) );
#endif

#endif

    {
        ovrErrorInfo errInfo;
        for (int ierr = 0;; ierr++) {
            ovr_GetLastErrorInfo(&errInfo);
            if (OVR_SUCCESS(errInfo.Result)) {
                break;
            }
            std::fprintf (stdout, "HeadMountedDisplay::update_eye_pose() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
        }
    }


}

ovrSizei HeadMountedDisplay::fov_texture_size( const ovrEyeType eye, const ovrFovPort fov, const float pixels_per_display_pixel )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::fov_texture_size() called." << std::endl;
#endif // DEBUG_SCREEN

    ovrSizei result;
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    KVS_OVR_CALL( result = ovr_GetFovTextureSize( m_handler, eye, fov, pixels_per_display_pixel ) );

#else // 0.5.0 - 0.6.0
    KVS_OVR_CALL( result = ovrHmd_GetFovTextureSize( m_handler, eye, fov, pixels_per_display_pixel ) );
#endif
    return result;
}

#if 0
void HeadMountedDisplay::bind_mirror_buffer( const bool distorted )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::bind_mirror_buffer() called." << std::endl;
#endif // DEBUG_SCREEN

    int e;

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::bind_mirror_buffer() current buffer = " << kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING ) << std::endl;
#endif // DEBUG_SCREEN

    // QOpenGLWidget does not support stereo hardware support.
    // @see https://forum.qt.io/topic/54208/qopenglwidget-stereo-rendering-using-gldrawbuffer/4
    // @see https://bugreports.qt.io/browse/QTBUG-64587
    //kvs::OpenGL::SetDrawBuffer( GL_FRONT );
    //kvs::OpenGL::SetDrawBuffer( GL_BACK );

    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call kvs::OpenGL::SetDrawBuffer(GL_FRONT) : %d", e);
    }



    kvs::OpenGL::SetClearColor( kvs::RGBColor::Black() );

    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call kvs::OpenGL::SetClearColor(black) : %d", e);
    }

    kvs::OpenGL::Clear( GL_COLOR_BUFFER_BIT );

    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call kvs::OpenGL::Clear(GL_COLOR_BUFFER_BIT) : %d", e);
    }



    if ( distorted )
    {
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 0, 0 )
        // Get the crrent color texture ID.
        GLuint tex_id = 0;
        KVS_OVR_CALL( ovr_GetMirrorTextureBufferGL( m_handler, m_mirror_tex, &tex_id ) );

        // Bind the frame buffer object.
        m_mirror_fbo_old = kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING );
        KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ) );
        KVS_GL_CALL( glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 ) );

#else
//        KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ) );
//        KVS_GL_CALL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
#endif
    }
    else
    {
//#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 0, 0 )
//         // Get the crrent color texture ID.
//         int current_index = 0;
//         //const ovrTextureSwapChain& color_textures = m_layer_data.ColorTexture[0];
//         const ovrTextureSwapChain& color_textures = m_mirror_tex.ColorTexture[0];
//         KVS_OVR_CALL( ovr_GetTextureSwapChainCurrentIndex( m_handler, color_textures, &current_index ) );

//         GLuint tex_id = 0;
//         KVS_OVR_CALL( ovr_GetTextureSwapChainBufferGL( m_handler, color_textures, current_index, &tex_id ) );

//         // Bind the frame buffer object.
//         m_mirror_fbo_old = kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING );
//         KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ));
//         //KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_framebuffer.id() ) );
//         KVS_GL_CALL( glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 ) );
//#else
// //        KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ) );
// //        KVS_GL_CALL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
//#endif
    }

    {
        ovrErrorInfo errInfo;
        for (int ierr = 0;; ierr++) {
            ovr_GetLastErrorInfo(&errInfo);
            if (OVR_SUCCESS(errInfo.Result)) {
                break;
            }
            std::fprintf (stdout, "HeadMountedDisplay::bind_mirror_buffer() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
        }
    }


#ifdef DEBUG_SCREEN
    std::cout << "HeadMountedDisplay::bind_mirror_texture() finished." << std::endl;
#endif // DEBUG_SCREEN

}
#endif

void HeadMountedDisplay::bind_mirror_buffer()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::bind_mirror_buffer() called." << std::endl;
#endif // DEBUG_SCREEN

    int e;

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::bind_mirror_buffer() current buffer = " << kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING ) << std::endl;
#endif // DEBUG_SCREEN

    // QOpenGLWidget does not support stereo hardware support.
    // @see https://forum.qt.io/topic/54208/qopenglwidget-stereo-rendering-using-gldrawbuffer/4
    // @see https://bugreports.qt.io/browse/QTBUG-64587
    //kvs::OpenGL::SetDrawBuffer( GL_FRONT );
    //kvs::OpenGL::SetDrawBuffer( GL_BACK );

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call kvs::OpenGL::SetDrawBuffer(GL_FRONT) : %d", e);
    }
#endif // DEBUG_SCREEN

    kvs::OpenGL::SetClearColor( kvs::RGBColor::Black() );

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call kvs::OpenGL::SetClearColor(black) : %d", e);
    }
#endif // DEBUG_SCREEN

    kvs::OpenGL::Clear( GL_COLOR_BUFFER_BIT );

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call kvs::OpenGL::Clear(GL_COLOR_BUFFER_BIT) : %d", e);
    }
#endif // DEBUG_SCREEN

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 1, 0, 0 )
    // Get the crrent color texture ID.
    GLuint tex_id = 0;
    KVS_OVR_CALL( ovr_GetMirrorTextureBufferGL( m_handler, m_mirror_tex, &tex_id ) );

    // Bind the frame buffer object.
    m_mirror_fbo_old = kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING );
    KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ) );
    KVS_GL_CALL( glFramebufferTexture2D( GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0 ) );

#else
//        KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo ) );
//        KVS_GL_CALL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
#endif
    {
        ovrErrorInfo errInfo;
        for (int ierr = 0;; ierr++) {
            ovr_GetLastErrorInfo(&errInfo);
            if (OVR_SUCCESS(errInfo.Result)) {
                break;
            }
            std::fprintf (stdout, "HeadMountedDisplay::bind_mirror_buffer() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
        }
    }


#ifdef DEBUG_SCREEN
    std::cout << "HeadMountedDisplay::bind_mirror_texture() finished." << std::endl;
#endif // DEBUG_SCREEN

}

void HeadMountedDisplay::unbind_mirror_buffer()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::unbind_mirror_buffer() called." << std::endl;
#endif // DEBUG_SCREEN

    int e;

#ifdef DEBUG_SCREEN
    std::cout << "unbind_mirror_buffer() : switch FBO from " << kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING )  << " to " << m_mirror_fbo_old << std::endl;
#endif // DEBUG_SCREEN

    // 20201118 yodo@meshman.jp changed to use m_mirror_fbo_old
    KVS_GL_CALL( glBindFramebuffer( GL_READ_FRAMEBUFFER, m_mirror_fbo_old));

#ifdef DEBUG_SCREEN
    // Check and clear GL errors
    while ((e = glGetError()) != 0){
        qCritical("HeadMountedDisplay::bind_mirror_buffer GL HAS ERROR after call glBindFramebuffer : %d", e);
    }
#endif // DEBUG_SCREEN

    {
        ovrErrorInfo errInfo;
        for (int ierr = 0;; ierr++) {
            ovr_GetLastErrorInfo(&errInfo);
            if (OVR_SUCCESS(errInfo.Result)) {
                break;
            }
            std::fprintf (stdout, "HeadMountedDisplay::unbind_mirror_buffer() has OVR error [%d] %s\n", ierr, errInfo.ErrorString);
        }
    }


#ifdef DEBUG_SCREEN
    std::cout << "HeadMountedDisplay::unbind_mirror_texture() finished." << std::endl;
#endif // DEBUG_SCREEN

}

void HeadMountedDisplay::blit_mirror_buffer( const kvs::Vec4 mirror_viewport, const bool flip )
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG HeadMountedDisplay::blit_mirror_buffer() called." << std::endl;

    std::cout << "blit_mirror_buffer() : start : current FBO is " << kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING ) << std::endl;
#endif // DEBUG_SCREEN

    const kvs::Vec4 screen_viewport = kvs::OpenGL::Viewport();
    const int screen_width = static_cast<int>( screen_viewport[2] );
    const int screen_height = static_cast<int>( screen_viewport[3] );

    const int mirror_x = static_cast<int>( mirror_viewport[0] );
    const int mirror_y = static_cast<int>( mirror_viewport[1] );
    const int mirror_width = static_cast<int>( mirror_viewport[2] );
    const int mirror_height = static_cast<int>( mirror_viewport[3] );

    const float mirror_ratio = static_cast<float>( mirror_width ) / mirror_height;
    const float screen_ratio = static_cast<float>( screen_width ) / screen_height;

    const int src_x0 = mirror_x;
    const int src_y0 = flip ? mirror_height : mirror_y;
    const int src_x1 = src_x0 + mirror_width;
    const int src_y1 = flip ? mirror_y: src_y0 + mirror_height;

    if ( screen_ratio > mirror_ratio )
    {
        const int width = static_cast<int>( screen_height * mirror_ratio );
        const int height = screen_height;
        const int dst_x0 = static_cast<int>( ( screen_width - width ) * 0.5f );
        const int dst_y0 = 0;
        const int dst_x1 = dst_x0 + width;
        const int dst_y1 = screen_height;
        KVS_GL_CALL( glBlitFramebuffer( src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0, dst_x1, dst_y1, GL_COLOR_BUFFER_BIT, GL_LINEAR ) );
    }
    else
    {
        const int width = screen_width;
        const int height = static_cast<int>( screen_width / mirror_ratio );
        const int dst_x0 = 0;
        const int dst_y0 = static_cast<int>( ( screen_height - height ) * 0.5f );
        const int dst_x1 = screen_width;
        const int dst_y1 = dst_y0 + screen_height;
        KVS_GL_CALL( glBlitFramebuffer( src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0, dst_x1, dst_y1, GL_COLOR_BUFFER_BIT, GL_LINEAR ) );
    }

#ifdef DEBUG_SCREEN
    std::cout << "blit_mirror_buffer() : finish : current FBO is " << kvs::OpenGL::Integer( GL_FRAMEBUFFER_BINDING ) << std::endl;
#endif // DEBUG_SCREEN


}

//void HeadMountedDisplay::CreateFrameBuffer() {
//    hudTexture->Create();
//}



} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
