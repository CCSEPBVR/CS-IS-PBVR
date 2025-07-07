/*****************************************************************************/
/**
 *  @file   OpenXRDevice.h
 *  @author Keisuke Tajiri
 */
 /*****************************************************************************/
#pragma once
#include <SupportOpenXR/OpenXR.h>
#include <kvs/OpenGL>
#include <kvs/Controller>
#include <vector>
#include <map>
#include <utility>

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  OpenXRDevice class.
 */
/*===========================================================================*/
class OpenXRDevice
{
public:
	typedef std::map<std::string, std::vector<std::string>> XrLayerExtensionsMap;

	enum HmdType
	{
		HmdTypeMetaQuest,
		HmdTypeVivePro,
		HmdTypeViveFocus3,
		HmdTypePico4,
		HmdTypeMax,
	};

	enum SpaceType
	{
		SpaceDefault,
		SpaceHead,
		SpaceLeftController,
		SpaceRightController,
		SpaceMax,
	};

	enum ControllerActionType
	{
		ControllerActionPose,
		ControllerActionTrigger,
		ControllerActionAxis_X,
		ControllerActionAxis_Y,
		ControllerActionClick_Menu,
		ControllerActionClick_A,
		ControllerActionClick_B,
		ControllerActionClick_X,
		ControllerActionClick_Y,
		ControllerActionMax,
	};

	struct GraphicsBindingOpenGLWin32KHR
	{
		XrStructureType type;
		const void* next;
		HDC hdc;
		HGLRC hglrc;
	};

private:
	std::string m_app_name = "KVS OpenXR Application";
	XrInstance m_instance = XR_NULL_HANDLE;
	kvs::UInt64 m_system_id = 0;
	XrSession m_session = XR_NULL_HANDLE;
	XrSpace m_space[SpaceMax] = { XR_NULL_HANDLE };
	std::vector<XrViewConfigurationView> m_config_views;
	XrSwapchain m_swapchains[kvs::Side::Max] = { XR_NULL_HANDLE };
	std::vector<XrView> m_views;
	XrEnvironmentBlendMode m_blend_mode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	std::vector<XrSwapchainImageOpenGLKHR> m_swapchain_left_images;
	std::vector<XrSwapchainImageOpenGLKHR> m_swapchain_right_images;
	XrSessionState m_session_state = XR_SESSION_STATE_UNKNOWN;
	XrFrameState m_frame_state = {};
	XrActionSet m_action_set = XR_NULL_HANDLE;
	XrAction m_action[ControllerActionMax] = { XR_NULL_HANDLE };
	XrPath m_hand_subaction_path[kvs::Side::Max] = { XrPath()};
	
	bool m_is_initialized = false;
	bool m_is_session_running = false;
	bool m_is_rendering_req = false;
	bool m_is_restart_req = false;
	kvs::Vec2i m_resolution[kvs::Side::Max] = { kvs::Vec2i::Zero() };
	kvs::Mat4 m_projection_matrix[kvs::Side::Max] = { kvs::Mat4::Identity() };
	kvs::Xform m_camera_xform[kvs::Side::Max] = { kvs::Xform() };
	kvs::Xform m_head_xform = kvs::Xform();
	kvs::Controller::ControllerStatus m_controller_status = kvs::Controller::ControllerStatus();
	kvs::UInt32 m_require_image[kvs::Side::Max] = { 0 };

public:
	OpenXRDevice();
	virtual ~OpenXRDevice();

public:
	void setAppName( const std::string& name );

	bool initialized() const { return m_is_initialized; }
	bool initialize();
	bool update();
	bool beginFrame();
	const kvs::UInt32 requireImage( kvs::UInt32 side ) const { return m_require_image[side]; }
	bool endFrame();

	bool isRunning() const { return m_is_session_running; }
	bool isRenderingReq() const { return m_is_rendering_req; }
	bool isRestartReq() const { return m_is_restart_req; }

	const kvs::Vec2i& resolution( kvs::UInt32 side ) const { return m_resolution[side]; }
	const kvs::Mat4& projectionMatrix( kvs::UInt32 side ) const { return m_projection_matrix[side]; }
	const kvs::Xform& cameraXform( kvs::UInt32 side ) const { return m_camera_xform[side]; }
	const kvs::Xform& headXform() const { return m_head_xform; }
	const kvs::Controller::ControllerStatus& controllerStatus() const { return m_controller_status; }

private:
	kvs::Mat4 toProjectionMatrix( const XrFovf& fov );
	kvs::Xform toXform( const XrPosef& pose );
	std::string xrResultString( XrResult result );
	std::string xrViewConfigTypeString( XrViewConfigurationType type );
	std::string xrSpaceTypeString( XrReferenceSpaceType type );
	std::string xrSessionStateString( XrSessionState state );
};
//------------------------------------------------------------------------------

} // end of namespace openxr

} // end of namespace kvs
