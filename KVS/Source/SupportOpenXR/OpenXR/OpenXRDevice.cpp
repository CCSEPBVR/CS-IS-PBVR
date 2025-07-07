/*****************************************************************************/
/**
 *  @file   OpenXRDevice.cpp
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#include "OpenXRDevice.h"

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new OpenXRDevice class.
 */
/*===========================================================================*/
OpenXRDevice::OpenXRDevice()
{
    kvsMessageDebug( "OpenXRDevice::OpenXRDevice()" );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the OpenXRDevice class.
 */
/*===========================================================================*/
OpenXRDevice::~OpenXRDevice()
{
    kvsMessageDebug( "OpenXRDevice::~OpenXRDevice()" );

    if ( m_is_session_running )
    {
        m_is_session_running = false;
        xrRequestExitSession( m_session );
        xrEndSession( m_session );
    }

    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        if ( m_swapchains[i] != XR_NULL_HANDLE ) { xrDestroySwapchain( m_swapchains[i] ); }
    }

    for ( kvs::UInt32 i = 0; i < SpaceMax; ++i )
    {
        if ( m_space[i] != XR_NULL_HANDLE ) { xrDestroySpace( m_space[i] ); }
    }

    if ( m_action_set != XR_NULL_HANDLE ) { xrDestroyActionSet( m_action_set );}
    if ( m_session != XR_NULL_HANDLE ) { xrDestroySession( m_session ); }
    if ( m_instance != XR_NULL_HANDLE ) { xrDestroyInstance( m_instance ); }
}

/*===========================================================================*/
/**
 *  @brief Set tha aplication name displayed in OpenXR
 *  @param name [in] app name
 */
/*===========================================================================*/
void OpenXRDevice::setAppName( const std::string& name )
{
   if ( name.length() > 0 )
   {
       m_app_name = name.c_str();
   }
}

/*===========================================================================*/
/**
 *  @brief  Initilize function
 */
/*===========================================================================*/
bool OpenXRDevice::initialize()
{
    if ( m_is_initialized )
    {
        return true;
    }

    std::vector<const char*> extensions = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };
    XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
    createInfo.next = nullptr;
    createInfo.enabledExtensionCount = static_cast<kvs::UInt32>( extensions.size() );
    createInfo.enabledExtensionNames = extensions.data();
    strcpy( createInfo.applicationInfo.applicationName, m_app_name.c_str() );
    createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    XrResult ret = xrCreateInstance( &createInfo, &m_instance );
    if ( XR_FAILED( ret ) || m_instance == XR_NULL_HANDLE )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to create XR instance. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    XrSystemGetInfo systemInfo{ XR_TYPE_SYSTEM_GET_INFO };
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    ret = xrGetSystem( m_instance, &systemInfo, &m_system_id );
    if ( XR_FAILED( ret ) && m_instance != XR_NULL_HANDLE && m_system_id != XR_NULL_SYSTEM_ID )
    {
        kvsMessageError("OpenXRDevice::initialize() Failed to xrGetSystem. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    kvs::UInt32 blendModeCount;
    ret = xrEnumerateEnvironmentBlendModes( 
        m_instance,
        m_system_id,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        0,
        &blendModeCount,
        nullptr );
    std::vector<XrEnvironmentBlendMode> blendModes( blendModeCount );
    ret = xrEnumerateEnvironmentBlendModes(
        m_instance,
        m_system_id,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        blendModeCount,
        &blendModeCount,
        blendModes.data() );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Faild to xrEnumerateEnviromentBlendModes. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }
    for ( kvs::UInt32 i = 0; i < blendModeCount; ++i )
    {
        if ( blendModes[i] == XR_ENVIRONMENT_BLEND_MODE_OPAQUE )		m_blend_mode = blendModes[i]; break;
        if ( blendModes[i] == XR_ENVIRONMENT_BLEND_MODE_ADDITIVE )		m_blend_mode = blendModes[i]; break;
        if ( blendModes[i] == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND )	m_blend_mode = blendModes[i]; break;
    }

    PFN_xrGetOpenGLGraphicsRequirementsKHR GetOpenGLGraphicsRequirementsKHR = nullptr;
    ret = xrGetInstanceProcAddr( 
        m_instance,
        "xrGetOpenGLGraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>( &GetOpenGLGraphicsRequirementsKHR ) );
    if ( XR_FAILED( ret ) || GetOpenGLGraphicsRequirementsKHR == nullptr )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrGetInstanceProcAddr. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    XrGraphicsRequirementsOpenGLKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
    ret = GetOpenGLGraphicsRequirementsKHR( m_instance, m_system_id, &graphicsRequirements );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to GetOpenGLGraphicsRequirementsKHR. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    GLint major = 0;
    GLint minor = 0;
    kvs::OpenGL::GetIntegerv( GL_MAJOR_VERSION, &major );
    kvs::OpenGL::GetIntegerv( GL_MINOR_VERSION, &minor );

    const XrVersion desiredApiVersion = XR_MAKE_VERSION( major, minor, 0 );
    if ( graphicsRequirements.minApiVersionSupported > desiredApiVersion )
    {
        kvsMessageError( "OpenXRDevice::initialize() Runtime does not support desired GraphicsAPI." );
        m_is_restart_req = true;
        return false;
    }

    XrSessionCreateInfo sessionInfo{ XR_TYPE_SESSION_CREATE_INFO };
    GraphicsBindingOpenGLWin32KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
    graphicsBinding.hdc = wglGetCurrentDC();
    graphicsBinding.hglrc = wglGetCurrentContext();
    sessionInfo.next = reinterpret_cast<const XrBaseInStructure*>( &graphicsBinding );
    sessionInfo.systemId = m_system_id;
    ret = xrCreateSession( m_instance, &sessionInfo, &m_session );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrCreateSession. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    kvs::UInt32 spaceCount;
    ret = xrEnumerateReferenceSpaces( m_session, 0, &spaceCount, nullptr );
    std::vector<XrReferenceSpaceType> spaces( spaceCount );
    ret = xrEnumerateReferenceSpaces( m_session, spaceCount, &spaceCount, spaces.data() );
    kvsMessageDebug( "OpenXRDevice::initialize() Available reference spaces: %d", spaceCount );
    for ( kvs::UInt32 i = 0; i < spaceCount; ++i )
    {
        kvsMessageDebug( "OpenXRDevice::initialize() reference space name = %s", xrSpaceTypeString( spaces[i] ).c_str() );
    }

    for ( kvs::UInt32 i = 0; i < SpaceMax; ++i )
    {
        if ( i == SpaceLeftController || i == SpaceRightController ) { continue; }

        XrReferenceSpaceType referenceSpaceType;
        if ( i == SpaceDefault )	referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
        if ( i == SpaceHead )		referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;

        XrReferenceSpaceCreateInfo refSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
        XrPosef pose{}; pose.orientation.w = 1;
        refSpaceCreateInfo.poseInReferenceSpace = pose;
        refSpaceCreateInfo.referenceSpaceType = referenceSpaceType;
        ret = xrCreateReferenceSpace( m_session, &refSpaceCreateInfo, &m_space[i] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError("OpenXRDevice::initialize() Failed to xrCreateReferenceSpace. xrResult:%s", xrResultString( ret ).c_str() );
            m_is_restart_req = true;
            return false;
        }
    }

    XrActionSetCreateInfo actionSetInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
    strcpy_s( actionSetInfo.actionSetName, "gameplay" );
    strcpy_s( actionSetInfo.localizedActionSetName, "Gameplay" );
    actionSetInfo.priority = 0;
    ret = xrCreateActionSet( m_instance, &actionSetInfo, &m_action_set );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrCreateActionSet. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    ret = xrStringToPath( m_instance, "/user/hand/left", &m_hand_subaction_path[kvs::Side::Left] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    ret = xrStringToPath( m_instance, "/user/hand/right", &m_hand_subaction_path[kvs::Side::Right] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = kvs::Side::Max;
        actionInfo.subactionPaths = &m_hand_subaction_path[0];
        strcpy_s( actionInfo.actionName, "hand_pose" );
        strcpy_s( actionInfo.localizedActionName, "Hand Pose" );
        actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionPose] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = kvs::Side::Max;
        actionInfo.subactionPaths = &m_hand_subaction_path[0];
        strcpy_s( actionInfo.actionName, "tirgger_value" );
        strcpy_s( actionInfo.localizedActionName, "Trigger Value" );
        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionTrigger] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = kvs::Side::Max;
        actionInfo.subactionPaths = &m_hand_subaction_path[0];
        strcpy_s( actionInfo.actionName, "axis_x" );
        strcpy_s( actionInfo.localizedActionName, "Axis X" );
        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionAxis_X] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = kvs::Side::Max;
        actionInfo.subactionPaths = &m_hand_subaction_path[0];
        strcpy_s( actionInfo.actionName, "axis_y" );
        strcpy_s( actionInfo.localizedActionName, "Axis Y" );
        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionAxis_Y] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = kvs::Side::Max;
        actionInfo.subactionPaths = &m_hand_subaction_path[0];
        strcpy_s( actionInfo.actionName, "click_menu" );
        strcpy_s( actionInfo.localizedActionName, "Click Menu" );
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionClick_Menu] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = 1;
        actionInfo.subactionPaths = &m_hand_subaction_path[kvs::Side::Right];
        strcpy_s( actionInfo.actionName, "click_a" );
        strcpy_s( actionInfo.localizedActionName, "Click A" );
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionClick_A] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = 1;
        actionInfo.subactionPaths = &m_hand_subaction_path[kvs::Side::Right];
        strcpy_s( actionInfo.actionName, "click_b" );
        strcpy_s( actionInfo.localizedActionName, "Click B" );
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionClick_B] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = 1;
        actionInfo.subactionPaths = &m_hand_subaction_path[kvs::Side::Left];
        strcpy_s( actionInfo.actionName, "click_x" );
        strcpy_s( actionInfo.localizedActionName, "Click X" );
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionClick_X] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    {
        XrActionCreateInfo actionInfo{ XR_TYPE_ACTION_CREATE_INFO };
        actionInfo.countSubactionPaths = 1;
        actionInfo.subactionPaths = &m_hand_subaction_path[kvs::Side::Left];
        strcpy_s( actionInfo.actionName, "click_y" );
        strcpy_s( actionInfo.localizedActionName, "Click Y" );
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        ret = xrCreateAction( m_action_set, &actionInfo, &m_action[ControllerActionClick_Y] ); KVS_ASSERT( XR_SUCCEEDED( ret ) );
    }

    for ( kvs::UInt32 i = 0; i < HmdTypeMax; ++i )
    {
        XrPath interactionProfilePath;
        if ( i == HmdTypeMetaQuest )	ret = xrStringToPath( m_instance, "/interaction_profiles/oculus/touch_controller", &interactionProfilePath );
        if ( i == HmdTypeVivePro )		ret = xrStringToPath( m_instance, "/interaction_profiles/htc/vive_controller", &interactionProfilePath );
        if ( i == HmdTypeViveFocus3 )	ret = xrStringToPath( m_instance, "/interaction_profiles/htc/vive_focus3_controller", &interactionProfilePath );
        if ( i == HmdTypePico4 )		ret = xrStringToPath( m_instance, "/interaction_profiles/bytedance/pico4_controller", &interactionProfilePath );
        
        std::vector<XrActionSuggestedBinding> bindings;

        XrPath posePath[kvs::Side::Max];
        ret = xrStringToPath( m_instance, "/user/hand/left/input/grip/pose", &posePath[kvs::Side::Left] );		KVS_ASSERT( XR_SUCCEEDED( ret ) );
        ret = xrStringToPath( m_instance, "/user/hand/right/input/grip/pose", &posePath[kvs::Side::Right] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
        bindings.push_back( { m_action[ControllerActionPose], posePath[kvs::Side::Left] } );
        bindings.push_back( { m_action[ControllerActionPose], posePath[kvs::Side::Right] } );

        XrPath triggerPath[kvs::Side::Max];
        ret = xrStringToPath( m_instance, "/user/hand/left/input/trigger/value", &triggerPath[kvs::Side::Left] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
        ret = xrStringToPath( m_instance, "/user/hand/right/input/trigger/value", &triggerPath[kvs::Side::Right] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
        bindings.push_back( { m_action[ControllerActionTrigger], triggerPath[kvs::Side::Left] } );
        bindings.push_back( { m_action[ControllerActionTrigger], triggerPath[kvs::Side::Right] } );

        if ( i == HmdTypeMetaQuest || i == HmdTypeViveFocus3 || i == HmdTypePico4 )
        {
            XrPath axisXPath[kvs::Side::Max];
            ret = xrStringToPath( m_instance, "/user/hand/left/input/thumbstick/x", &axisXPath[kvs::Side::Left] );		KVS_ASSERT(	XR_SUCCEEDED( ret ) );
            ret = xrStringToPath( m_instance, "/user/hand/right/input/thumbstick/x", &axisXPath[kvs::Side::Right] );	KVS_ASSERT(	XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionAxis_X], axisXPath[kvs::Side::Left] } );
            bindings.push_back( { m_action[ControllerActionAxis_X], axisXPath[kvs::Side::Right] } );

            XrPath axisYPath[kvs::Side::Max];
            ret = xrStringToPath( m_instance, "/user/hand/left/input/thumbstick/y", &axisYPath[kvs::Side::Left] );		KVS_ASSERT( XR_SUCCEEDED( ret ) );
            ret = xrStringToPath( m_instance, "/user/hand/right/input/thumbstick/y", &axisYPath[kvs::Side::Right] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionAxis_Y], axisYPath[kvs::Side::Left] } );
            bindings.push_back( { m_action[ControllerActionAxis_Y], axisYPath[kvs::Side::Right] } );

            XrPath aPath;
            ret = xrStringToPath( m_instance, "/user/hand/right/input/a/click", &aPath );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionClick_A], aPath } );

            XrPath bPath;
            ret = xrStringToPath( m_instance, "/user/hand/right/input/b/click", &bPath );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionClick_B], bPath } );

            XrPath xPath;
            ret = xrStringToPath( m_instance, "/user/hand/left/input/x/click", &xPath );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionClick_X], xPath } );

            XrPath yPath;
            ret = xrStringToPath( m_instance, "/user/hand/left/input/y/click", &yPath );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionClick_Y], yPath } );
        }
        else if ( i == HmdTypeVivePro )
        {
            XrPath axisXPath[kvs::Side::Max];
            ret = xrStringToPath( m_instance, "/user/hand/left/input/trackpad/x", &axisXPath[kvs::Side::Left] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            ret = xrStringToPath( m_instance, "/user/hand/right/input/trackpad/x", &axisXPath[kvs::Side::Right] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionAxis_X], axisXPath[kvs::Side::Left] } );
            bindings.push_back( { m_action[ControllerActionAxis_X], axisXPath[kvs::Side::Right] } );

            XrPath axisYPath[kvs::Side::Max];
            ret = xrStringToPath( m_instance, "/user/hand/left/input/trackpad/y", &axisYPath[kvs::Side::Left] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            ret = xrStringToPath( m_instance, "/user/hand/right/input/trackpad/y", &axisYPath[kvs::Side::Right] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionAxis_Y], axisYPath[kvs::Side::Left] } );
            bindings.push_back( { m_action[ControllerActionAxis_Y], axisYPath[kvs::Side::Right] } );

            XrPath menuPath[kvs::Side::Max];
            ret = xrStringToPath( m_instance, "/user/hand/left/input/menu/click", &menuPath[kvs::Side::Left] );		KVS_ASSERT( XR_SUCCEEDED( ret ) );
            ret = xrStringToPath( m_instance, "/user/hand/right/input/menu/click", &menuPath[kvs::Side::Right] );	KVS_ASSERT( XR_SUCCEEDED( ret ) );
            bindings.push_back( { m_action[ControllerActionClick_Menu], menuPath[kvs::Side::Left] } );
            bindings.push_back( { m_action[ControllerActionClick_Menu], menuPath[kvs::Side::Right] } );
        }

        XrInteractionProfileSuggestedBinding suggestedBinding{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
        suggestedBinding.interactionProfile = interactionProfilePath;
        suggestedBinding.suggestedBindings = bindings.data();
        suggestedBinding.countSuggestedBindings = (kvs::UInt32)bindings.size();
        ret = xrSuggestInteractionProfileBindings( m_instance, &suggestedBinding );
        if ( XR_FAILED( ret ) )
        {
            std::string hmdType = "";
            if (i == HmdTypeMetaQuest)  { hmdType = "HmdTypeMetaQuest"; }
            if (i == HmdTypeVivePro) { hmdType = "HmdTypeVivePro"; }
            if (i == HmdTypeViveFocus3) { hmdType = "HmdTypeViveFocus3"; }
            if (i == HmdTypePico4) { hmdType = "HmdTypePico4"; }
            kvsMessageWarning( "OpenXRDevice::initialize() Failed to xrSuggestInteractionProfileBindings (%s) . xrResult:%s ", hmdType.c_str(), xrResultString(ret).c_str());
            continue;
        }
    }

    XrActionSpaceCreateInfo actionSpaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
    actionSpaceInfo.action = m_action[ControllerActionPose];
    actionSpaceInfo.poseInActionSpace.orientation.w = 1.0f;
    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        kvs::UInt32 spaceIndex = i == kvs::Side::Left ? SpaceLeftController : SpaceRightController;
        actionSpaceInfo.subactionPath = m_hand_subaction_path[i];
        ret = xrCreateActionSpace( m_session, &actionSpaceInfo, &m_space[spaceIndex] );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::initialize() Failed to xrCreateActionSpace left. xrResult:%s", xrResultString( ret ).c_str() );
            m_is_restart_req = true;
            return false;
        }
    }

    XrSessionActionSetsAttachInfo attachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &m_action_set;
    ret = xrAttachSessionActionSets( m_session, &attachInfo );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrAttachSessionActionSets. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    kvs::UInt32 viewCount;
    ret = xrEnumerateViewConfigurationViews(
        m_instance,
        m_system_id,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        0,
        &viewCount,
        nullptr );
    m_config_views.resize( viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr } );
    ret = xrEnumerateViewConfigurationViews(
        m_instance,
        m_system_id,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        viewCount,
        &viewCount,
        m_config_views.data() );
    if ( XR_FAILED( ret ) || viewCount != kvs::Side::Max )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrEnumerateViewConfigurationViews. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }
    m_views.resize( viewCount );
    for ( kvs::UInt32 i = 0; i < viewCount; ++i )
    {
        m_views[i].type = XR_TYPE_VIEW;
        m_views[i].next = nullptr;
    }

    kvs::UInt32 swapchainFormatCount;
    ret = xrEnumerateSwapchainFormats( m_session, 0, &swapchainFormatCount, nullptr );
    std::vector<kvs::Int64> swapchainFormats( swapchainFormatCount );
    ret = xrEnumerateSwapchainFormats( m_session, swapchainFormatCount, &swapchainFormatCount, swapchainFormats.data() );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrEnumerateSwapchainFormats. xrResult:%s", xrResultString( ret ).c_str() );
        m_is_restart_req = true;
        return false;
    }

    std::vector<kvs::UInt64> supportColorSwapchainFormat;
    supportColorSwapchainFormat.push_back( GL_SRGB8_ALPHA8_EXT );
    kvs::Int64 colorFormat = -1;
    for ( kvs::UInt32 i = 0; i < swapchainFormats.size(); ++i )
    {
        kvsMessageDebug( "OpenXRDevice::initialize() Support color swapchain format = %d", swapchainFormats[i] );
    }
    for ( kvs::UInt32 i = 0; i < supportColorSwapchainFormat.size(); ++i )
    {
        for ( kvs::UInt32 j = 0; j < swapchainFormats.size(); ++j )
        {
            if ( supportColorSwapchainFormat[i] == swapchainFormats[j] )
            {
                colorFormat = swapchainFormats[j];
                break;
            }
        }
        if ( colorFormat != -1 ) { break; }
    }
    if ( colorFormat == -1 )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to set colorSwapChainFormat." );
        m_is_restart_req = true;
        return false;
    }

    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        const XrViewConfigurationView& vp = m_config_views[i];
        m_resolution[i] = kvs::Vec2i( vp.recommendedImageRectWidth, vp.recommendedImageRectHeight );

        XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
        swapchainCreateInfo.arraySize = 1;
        swapchainCreateInfo.format = colorFormat;
        swapchainCreateInfo.width = vp.recommendedImageRectWidth;
        swapchainCreateInfo.height = vp.recommendedImageRectHeight;
        swapchainCreateInfo.mipCount = 1;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.sampleCount = 1;
        swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

        XrSwapchain swapchain;
        ret = xrCreateSwapchain( m_session, &swapchainCreateInfo, &swapchain );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::initialize() Failed to xrCreateSwapchain. xrResult:%s", xrResultString( ret ).c_str() );
            m_is_restart_req = true;
            return false;
        }
        m_swapchains[i] = swapchain;

        kvs::UInt32 imageCount;
        ret = xrEnumerateSwapchainImages( swapchain, 0, &imageCount, nullptr );
        for ( kvs::UInt32 j = 0; j < imageCount; ++j )
        {
            XrSwapchainImageOpenGLKHR swapchainImage;
            swapchainImage.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
            swapchainImage.next = nullptr;
            if ( i == kvs::Side::Left )
            {
                m_swapchain_left_images.push_back( std::move( swapchainImage ) );
            }
            else
            {
                m_swapchain_right_images.push_back( std::move( swapchainImage ) );
            }
        }
        if ( i == kvs::Side::Left )
        {
            ret = xrEnumerateSwapchainImages( m_swapchains[i], imageCount, &imageCount, (XrSwapchainImageBaseHeader*)m_swapchain_left_images.data() );
        }
        else
        {
            ret = xrEnumerateSwapchainImages( m_swapchains[i], imageCount, &imageCount, (XrSwapchainImageBaseHeader*)m_swapchain_right_images.data() );
        }
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::initialize() Failed to xrEnumerateSwapchainImages. xrResult:%s", xrResultString( ret ).c_str() );
            m_is_restart_req = true;
            return false;
        }
    }

    XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
    beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    ret = xrBeginSession( m_session, &beginInfo );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::initialize() Failed to xrBeginSession. xrResult:%s", xrResultString( ret ).c_str() );
        return false;
    }
    m_is_session_running = true;

    m_is_initialized = update();
    return m_is_initialized;
}

/*===========================================================================*/
/**
 *  @brief Update function
 */
/*===========================================================================*/
bool OpenXRDevice::update()
{
    XrEventDataBuffer eventDataBuffer;
    eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;
    eventDataBuffer.next = NULL;
    XrResult ret = xrPollEvent( m_instance, &eventDataBuffer );
    while ( ret == XR_SUCCESS )
    {
        switch ( eventDataBuffer.type )
        {
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
        {
            const XrEventDataInstanceLossPending& pending =
                *reinterpret_cast<const XrEventDataInstanceLossPending*>( &eventDataBuffer );
            m_is_restart_req = true;

            kvsMessageError( "OpenXRDevice::update() XrEventDataInstanceLossPending by %lld", pending.lossTime);
            return false;
            break;
        }
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
        {
            XrEventDataSessionStateChanged sessionState = 
                *reinterpret_cast<const XrEventDataSessionStateChanged*>( &eventDataBuffer );

            const XrSessionState oldState = m_session_state;
            m_session_state = sessionState.state;
            kvsMessageDebug( "OpenXRDevice::update() XrEventDataSessionStateChanged: state %s->%s time=%lld",
                xrSessionStateString( oldState ).c_str(),
                xrSessionStateString( m_session_state ).c_str(),
                sessionState.time );

            if ( sessionState.session != XR_NULL_HANDLE && sessionState.session != m_session )
            {
                kvsMessageError( "OpenXRDevice::update()XrEventDataSessionStateChanged for unknown session." );
                return false;
            }

            switch ( m_session_state )
            {
            case XR_SESSION_STATE_IDLE: break;
            case XR_SESSION_STATE_READY:
            {
                if ( !m_is_session_running )
                {
                    KVS_ASSERT( m_session != XR_NULL_HANDLE );
                    XrSessionBeginInfo beginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
                    beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    ret = xrBeginSession( m_session, &beginInfo );
                    if ( XR_FAILED( ret ) )
                    {
                        kvsMessageError( "OpenXRDevice::update() Faild to xrBeginSession. xrResult:%s", xrResultString( ret ).c_str() );
                        return false;
                    }
                    m_is_session_running = true;
                }
                m_is_rendering_req = true;
                break;
            }
            case XR_SESSION_STATE_VISIBLE: { m_is_rendering_req = true; break; }
            case XR_SESSION_STATE_STOPPING:
            {
                if ( m_is_session_running )
                {
                    m_is_session_running = false;
                    m_is_rendering_req = false;
                    ret = xrEndSession( m_session );
                    if ( XR_FAILED( ret ) )
                    {
                        kvsMessageError( "OpenXRDevice::update() Faild to xrEndSession. xrResult:%s", xrResultString( ret ).c_str() );
                        return false;
                    }
                }
                break;
            }
            case XR_SESSION_STATE_LOSS_PENDING:
            {
                if (m_is_session_running)
                {
                    m_is_session_running = false;
                    m_is_rendering_req = false;
                    ret = xrRequestExitSession( m_session );
                    if ( XR_FAILED( ret ) )
                    {
                        kvsMessageError( "OpenXRDevice::update() Faild to xrRequestExitSession. xrResult:%s", xrResultString( ret ).c_str() );
                        return false;
                    }
                    ret = xrEndSession( m_session );
                    if ( XR_FAILED( ret ) )
                    {
                        kvsMessageError( "OpenXRDevice::update() Faild to xrEndSession. xrResult:%s", xrResultString( ret ).c_str() );
                        return false;
                    }
                }
                break;
            }
            case XR_SESSION_STATE_EXITING:
            {
                m_is_rendering_req = false;
                m_is_restart_req = true;
                break;
            }
            default: break;
            }
            break;
        }
        default: break;
        }
        eventDataBuffer.type = XR_TYPE_EVENT_DATA_BUFFER;
        ret = xrPollEvent( m_instance, &eventDataBuffer );
    }
    return true;
}

/*===========================================================================*/
/**
 *  @brief Begin frame.
 */
/*===========================================================================*/
bool OpenXRDevice::beginFrame()
{
    static const float OPENXR_CONTROLLER_THRESHOLD = 0.4f;

    if ( !m_is_session_running || !m_is_rendering_req )
    {
        return false;
    }

    XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
    m_frame_state = { XR_TYPE_FRAME_STATE };
    XrResult ret = xrWaitFrame( m_session, &frameWaitInfo, &m_frame_state );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrWaitFrame. xrResult:%s", xrResultString( ret ).c_str() );
        return false;
    }
    
    XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
    ret = xrBeginFrame( m_session, &frameBeginInfo );
    if ( XR_FAILED( ret ) )
    {
        kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrBeginFrame. xrResult:%s", xrResultString( ret ).c_str() );
        return false;
    }

    if ( m_frame_state.shouldRender )
    {
        XrViewState viewState{ XR_TYPE_VIEW_STATE };
        XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
        viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        viewLocateInfo.displayTime = m_frame_state.predictedDisplayTime;
        viewLocateInfo.space = m_space[SpaceDefault];

        kvs::UInt32 viewCountOutput = 0;
        ret = xrLocateViews(
            m_session,
            &viewLocateInfo, 
            &viewState, 
            kvs::Side::Max, 
            &viewCountOutput,
            m_views.data() );
        if ( XR_FAILED( ret ) || viewCountOutput != kvs::Side::Max )
        {
            kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrLocateViews. xrResult:%s", xrResultString( ret ).c_str() );
            return false;
        }

        if ( ( viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT ) != 0 &&
             ( viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT ) != 0 )
        {
            for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
            {
                m_projection_matrix[i] = toProjectionMatrix( m_views[i].fov );
                m_camera_xform[i] = toXform( m_views[i].pose );
            }
        }

        XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
        ret = xrLocateSpace(
            m_space[SpaceHead],
            m_space[SpaceDefault],
            m_frame_state.predictedDisplayTime,
            &spaceLocation );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrLocateSpace. xrResult:%s", xrResultString( ret ).c_str() );
            return false;
        }
        if ( ( spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT ) != 0 &&
             ( spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT ) != 0 ) 
        {
            m_head_xform = toXform( spaceLocation.pose );
        }
        else
        {
            m_head_xform = kvs::Xform();
        }

        const XrActiveActionSet activeActionSet{ m_action_set, XR_NULL_PATH };
        XrActionsSyncInfo syncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
        syncInfo.countActiveActionSets = 1;
        syncInfo.activeActionSets = &activeActionSet;
        ret = xrSyncActions( m_session, &syncInfo );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrSyncActions. xrResult:%s", xrResultString( ret ).c_str() );
        }

        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            m_controller_status.is_active[i] = false;
            m_controller_status.axis_value[i] = kvs::Vec2::Zero();

            XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
            getInfo.subactionPath = m_hand_subaction_path[i];

            if ( m_action[ControllerActionPose] != XR_NULL_HANDLE )
            {
                XrActionStatePose poseState{ XR_TYPE_ACTION_STATE_POSE };
                getInfo.action = m_action[ControllerActionPose];
                ret = xrGetActionStatePose( m_session, &getInfo, &poseState );
                if ( XR_SUCCEEDED( ret ) && poseState.isActive)
                {
                    m_controller_status.is_active[i] = poseState.isActive;

                    kvs::UInt32 spaceIndex = i == kvs::Side::Left ? SpaceLeftController : SpaceRightController;
                    XrSpaceLocation handSpaceLocation{ XR_TYPE_SPACE_LOCATION };
                    ret = xrLocateSpace( m_space[spaceIndex],
                        m_space[SpaceDefault],
                        m_frame_state.predictedDisplayTime,
                        &handSpaceLocation );
                    if ( XR_SUCCEEDED( ret ) )
                    {
                        if ( ( handSpaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT ) != 0 &&
                             ( handSpaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT ) != 0)
                        {
                            m_controller_status.last_xform[i] = m_controller_status.xform[i];
                            m_controller_status.xform[i] = toXform( handSpaceLocation.pose );
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
            if ( m_action[ControllerActionTrigger] != XR_NULL_HANDLE )
            {
                XrActionStateFloat floatState{ XR_TYPE_ACTION_STATE_FLOAT };
                getInfo.action = m_action[ControllerActionTrigger];
                ret = xrGetActionStateFloat( m_session, &getInfo, &floatState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    bool last = m_controller_status.button_status[i][kvs::Controller::Trigger].last;
                    bool current = floatState.currentState == 1.0f;
                    m_controller_status.button_status[i][kvs::Controller::Trigger].pressed = !last && current;
                    m_controller_status.button_status[i][kvs::Controller::Trigger].pressing = last && current;
                    m_controller_status.button_status[i][kvs::Controller::Trigger].released = last && !current;
                    m_controller_status.button_status[i][kvs::Controller::Trigger].last = current;
                }
            }
            if ( m_action[ControllerActionAxis_X] != XR_NULL_HANDLE )
            {
                XrActionStateFloat floatState{ XR_TYPE_ACTION_STATE_FLOAT };
                getInfo.action = m_action[ControllerActionAxis_X];
                ret = xrGetActionStateFloat( m_session, &getInfo, &floatState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    if ( kvs::Math::Abs( floatState.currentState ) > OPENXR_CONTROLLER_THRESHOLD )
                    {
                        m_controller_status.axis_value[i][0] = floatState.currentState;
                    }
                }
            }
            if ( m_action[ControllerActionAxis_Y] != XR_NULL_HANDLE )
            {
                XrActionStateFloat floatState{ XR_TYPE_ACTION_STATE_FLOAT };
                getInfo.action = m_action[ControllerActionAxis_Y];
                ret = xrGetActionStateFloat( m_session, &getInfo, &floatState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    if ( kvs::Math::Abs( floatState.currentState ) > OPENXR_CONTROLLER_THRESHOLD )
                    {
                        m_controller_status.axis_value[i][1] = floatState.currentState;
                    }
                }
            }
            if ( m_action[ControllerActionClick_Menu] != XR_NULL_HANDLE )
            {
                XrActionStateBoolean boolState{ XR_TYPE_ACTION_STATE_BOOLEAN };
                getInfo.action = m_action[ControllerActionClick_Menu];
                ret = xrGetActionStateBoolean( m_session, &getInfo, &boolState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    bool last = m_controller_status.button_status[i][kvs::Controller::Menu].last;
                    bool current = boolState.currentState;
                    m_controller_status.button_status[i][kvs::Controller::Menu].pressed = !last && current;
                    m_controller_status.button_status[i][kvs::Controller::Menu].pressing = last && current;
                    m_controller_status.button_status[i][kvs::Controller::Menu].released = last && !current;
                    m_controller_status.button_status[i][kvs::Controller::Menu].last = current;
                }
            }
            if ( m_action[ControllerActionClick_A] != XR_NULL_HANDLE )
            {
                XrActionStateBoolean boolState{ XR_TYPE_ACTION_STATE_BOOLEAN };
                getInfo.action = m_action[ControllerActionClick_A];
                ret = xrGetActionStateBoolean( m_session, &getInfo, &boolState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    bool last = m_controller_status.button_status[i][kvs::Controller::A].last;
                    bool current = boolState.currentState;
                    m_controller_status.button_status[i][kvs::Controller::A].pressed = !last && current;
                    m_controller_status.button_status[i][kvs::Controller::A].pressing = last && current;
                    m_controller_status.button_status[i][kvs::Controller::A].released = last && !current;
                    m_controller_status.button_status[i][kvs::Controller::A].last = current;
                }
            }
            if (m_action[ControllerActionClick_B] != XR_NULL_HANDLE)
            {
                XrActionStateBoolean boolState{ XR_TYPE_ACTION_STATE_BOOLEAN };
                getInfo.action = m_action[ControllerActionClick_B];
                ret = xrGetActionStateBoolean( m_session, &getInfo, &boolState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    bool last = m_controller_status.button_status[i][kvs::Controller::B].last;
                    bool current = boolState.currentState;
                    m_controller_status.button_status[i][kvs::Controller::B].pressed = !last && current;
                    m_controller_status.button_status[i][kvs::Controller::B].pressing = last && current;
                    m_controller_status.button_status[i][kvs::Controller::B].released = last && !current;
                    m_controller_status.button_status[i][kvs::Controller::B].last = current;
                }
            }
            if (m_action[ControllerActionClick_X] != XR_NULL_HANDLE)
            {
                XrActionStateBoolean boolState{ XR_TYPE_ACTION_STATE_BOOLEAN };
                getInfo.action = m_action[ControllerActionClick_X];
                ret = xrGetActionStateBoolean( m_session, &getInfo, &boolState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    bool last = m_controller_status.button_status[i][kvs::Controller::X].last;
                    bool current = boolState.currentState;
                    m_controller_status.button_status[i][kvs::Controller::X].pressed = !last && current;
                    m_controller_status.button_status[i][kvs::Controller::X].pressing = last && current;
                    m_controller_status.button_status[i][kvs::Controller::X].released = last && !current;
                    m_controller_status.button_status[i][kvs::Controller::X].last = current;
                }
            }
            if (m_action[ControllerActionClick_Y] != XR_NULL_HANDLE)
            {
                XrActionStateBoolean boolState{ XR_TYPE_ACTION_STATE_BOOLEAN };
                getInfo.action = m_action[ControllerActionClick_Y];
                ret = xrGetActionStateBoolean( m_session, &getInfo, &boolState );
                if ( XR_SUCCEEDED( ret ) )
                {
                    bool last = m_controller_status.button_status[i][kvs::Controller::Y].last;
                    bool current = boolState.currentState;
                    m_controller_status.button_status[i][kvs::Controller::Y].pressed = !last && current;
                    m_controller_status.button_status[i][kvs::Controller::Y].pressing = last && current;
                    m_controller_status.button_status[i][kvs::Controller::Y].released = last && !current;
                    m_controller_status.button_status[i][kvs::Controller::Y].last = current;
                }
            }
        }

        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
            kvs::UInt32 imageIndex;
            ret = xrAcquireSwapchainImage( m_swapchains[i], &acquireInfo, &imageIndex );
            if ( XR_FAILED( ret ) )
            {
                kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrAcquireSwapchainImage. xrResult:%s", xrResultString( ret ).c_str() );
                return false;
            }

            XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
            waitInfo.timeout = XR_INFINITE_DURATION;
            ret = xrWaitSwapchainImage(m_swapchains[i], &waitInfo);
            if ( XR_FAILED ( ret ) )
            {
                kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrAcquireSwapchainImage. xrResult:%s", xrResultString( ret ).c_str() );
                return false;
            }
            if ( i == kvs::Side::Left )
            {
                XrSwapchainImageOpenGLKHR swapchainImage = m_swapchain_left_images[imageIndex];
                m_require_image[i] = swapchainImage.image;
            }
            else
            {
                XrSwapchainImageOpenGLKHR swapchainImage = m_swapchain_right_images[imageIndex];
                m_require_image[i] = swapchainImage.image;
            }
        }
    }
    else
    {
        XrFrameEndInfo frameEndInfo{};
        frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
        frameEndInfo.next = NULL;
        frameEndInfo.displayTime = m_frame_state.predictedDisplayTime;
        frameEndInfo.environmentBlendMode = m_blend_mode;
        frameEndInfo.layerCount = 0;
        frameEndInfo.layers = nullptr;
        XrResult ret = xrEndFrame( m_session, &frameEndInfo );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::beginFrame() Faild to xrEndFrame. xrResult:%s", xrResultString( ret ).c_str() );
            return false;
        }
        return false;
    }
    return true;
}

/*===========================================================================*/
/**
 *  @brief End frame
 */
/*===========================================================================*/
bool OpenXRDevice::endFrame()
{
    if ( !m_is_session_running )
    {
        return false;
    }

    if ( m_frame_state.shouldRender )
    {
        XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
        std::vector<XrCompositionLayerProjectionView> layerViews;
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i ) 
        {
            XrCompositionLayerProjectionView layerView{};
            layerView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
            layerView.pose = m_views[i].pose;
            layerView.fov = m_views[i].fov;
            layerView.subImage.swapchain = m_swapchains[i];
            layerView.subImage.imageArrayIndex = 0;
            layerView.subImage.imageRect.offset = { 0 , 0};
            layerView.subImage.imageRect.extent = { m_resolution[i][0], m_resolution[i][1]};
            layerViews.push_back( layerView );

            XrResult ret = xrReleaseSwapchainImage( m_swapchains[i], &releaseInfo );
            if ( XR_FAILED( ret ) )
            {
                kvsMessageError( "OpenXRDevice::endFrame() Faild to xrReleaseSwapchainImage. xrResult:%s", xrResultString( ret ).c_str() );
                return false;
            }
        }

        std::vector<XrCompositionLayerBaseHeader*> layers;
        XrCompositionLayerProjection layer{};
        layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
        layer.space = m_space[SpaceDefault];
        layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        layer.viewCount = static_cast<kvs::UInt32>( layerViews.size() );
        layer.views = layerViews.data();
        layers.push_back( reinterpret_cast<XrCompositionLayerBaseHeader*>( &layer ) );

        XrFrameEndInfo frameEndInfo{};
        frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
        frameEndInfo.next = NULL;
        frameEndInfo.displayTime = m_frame_state.predictedDisplayTime;
        frameEndInfo.environmentBlendMode = m_blend_mode;
        frameEndInfo.layerCount = static_cast<kvs::UInt32>( layers.size() );
        frameEndInfo.layers = layers.data();
        XrResult ret = xrEndFrame( m_session, &frameEndInfo );
        if ( XR_FAILED( ret ) )
        {
            kvsMessageError( "OpenXRDevice::endFrame() Faild to xrEndFrame. xrResult:%s", xrResultString( ret ).c_str() );
            return false;
        }
    }
    return true;
}

/*===========================================================================*/
/**
 *  @brief Return projection matrix.
 *  @param fov [in] XrFovf
 *  @return projection matrix.
 */
/*===========================================================================*/
kvs::Mat4 OpenXRDevice::toProjectionMatrix( const XrFovf& fov )
{
    static const float OPENXR_NEAR = 0.1f;
    static const float OPENXR_FAR = 1000.f;

    float right = std::tanf( fov.angleRight );
    float left = std::tanf( fov.angleLeft );
    float top = std::tanf( fov.angleUp ) ;
    float bottom = std::tanf( fov.angleDown ) ;

    kvs::Vec4 d0 = kvs::Vec4::Zero();
    kvs::Vec4 d1 = kvs::Vec4::Zero();
    kvs::Vec4 d2 = kvs::Vec4::Zero();
    kvs::Vec4 d3 = kvs::Vec4::Zero();

    d0[0] = 2.0f / ( right - left );
    d0[2] = ( right + left ) / ( right - left );
    
    d1[1] = 2.0f / ( top - bottom );
    d1[2] = ( top + bottom ) / ( top - bottom );

    d2[2] = -( OPENXR_FAR + OPENXR_NEAR ) / ( OPENXR_FAR - OPENXR_NEAR );
    d2[3] = -( 2.0f * OPENXR_FAR * OPENXR_NEAR ) / ( OPENXR_FAR - OPENXR_NEAR );

    d3[2] = -1.0f;

    return kvs::Mat4( d0, d1, d2, d3);
}

/*===========================================================================*/
/**
 *  @brief Return xform.
 *  @param pose [in] XrPosef 
 *  @return xform.
 */
/*===========================================================================*/
kvs::Xform OpenXRDevice::toXform( const XrPosef& pose)
{
    kvs::Vec3 p = kvs::Vec3( pose.position.x, pose.position.y, pose.position.z );
    kvs::Vec3 s = kvs::Vec3::Ones();
    kvs::Mat3 r = kvs::Mat3(
        pose.orientation.x,
        pose.orientation.y,
        pose.orientation.z,
        pose.orientation.w );
    return kvs::Xform( p, s, r);
}


/*===========================================================================*/
/**
 *  @brief Return XrResult values to corresponding strings.
 *  @param result [in] XrResult
 *  @return XrResult values to corresponding strings.
 */
/*===========================================================================*/
std::string OpenXRDevice::xrResultString( XrResult result )
{
    switch ( result )
    {
    case XR_SUCCESS:													return "XR_SUCCESS";
    case XR_TIMEOUT_EXPIRED:											return "XR_TIMEOUT_EXPIRED";
    case XR_SESSION_LOSS_PENDING:										return "XR_SESSION_LOSS_PENDING";
    case XR_EVENT_UNAVAILABLE:											return "XR_EVENT_UNAVAILABLE";
    case XR_SPACE_BOUNDS_UNAVAILABLE:									return "XR_SPACE_BOUNDS_UNAVAILABLE";
    case XR_SESSION_NOT_FOCUSED:										return "XR_SESSION_NOT_FOCUSED";
    case XR_FRAME_DISCARDED:											return "XR_FRAME_DISCARDED";
    case XR_ERROR_VALIDATION_FAILURE:									return "XR_ERROR_VALIDATION_FAILURE";
    case XR_ERROR_RUNTIME_FAILURE:										return "XR_ERROR_RUNTIME_FAILURE";
    case XR_ERROR_OUT_OF_MEMORY:										return "XR_ERROR_OUT_OF_MEMORY";
    case XR_ERROR_API_VERSION_UNSUPPORTED:								return "XR_ERROR_API_VERSION_UNSUPPORTED";
    case XR_ERROR_INITIALIZATION_FAILED:								return "XR_ERROR_INITIALIZATION_FAILED";
    case XR_ERROR_FUNCTION_UNSUPPORTED:									return "XR_ERROR_FUNCTION_UNSUPPORTED";
    case XR_ERROR_FEATURE_UNSUPPORTED:									return "XR_ERROR_FEATURE_UNSUPPORTED";
    case XR_ERROR_EXTENSION_NOT_PRESENT:								return "XR_ERROR_EXTENSION_NOT_PRESENT";
    case XR_ERROR_LIMIT_REACHED:										return "XR_ERROR_LIMIT_REACHED";
    case XR_ERROR_SIZE_INSUFFICIENT:									return "XR_ERROR_SIZE_INSUFFICIENT";
    case XR_ERROR_HANDLE_INVALID:										return "XR_ERROR_HANDLE_INVALID";
    case XR_ERROR_INSTANCE_LOST:										return "XR_ERROR_INSTANCE_LOST";
    case XR_ERROR_SESSION_RUNNING:										return "XR_ERROR_SESSION_RUNNING";
    case XR_ERROR_SESSION_NOT_RUNNING:									return "XR_ERROR_SESSION_NOT_RUNNING";
    case XR_ERROR_SESSION_LOST:											return "XR_ERROR_SESSION_LOST";
    case XR_ERROR_SYSTEM_INVALID:										return "XR_ERROR_SYSTEM_INVALID";
    case XR_ERROR_PATH_INVALID:											return "XR_ERROR_PATH_INVALID";
    case XR_ERROR_PATH_COUNT_EXCEEDED:									return "XR_ERROR_PATH_COUNT_EXCEEDED";
    case XR_ERROR_PATH_FORMAT_INVALID:									return "XR_ERROR_PATH_FORMAT_INVALID";
    case XR_ERROR_PATH_UNSUPPORTED:										return "XR_ERROR_PATH_UNSUPPORTED";
    case XR_ERROR_LAYER_INVALID:										return "XR_ERROR_LAYER_INVALID";
    case XR_ERROR_LAYER_LIMIT_EXCEEDED:									return "XR_ERROR_LAYER_LIMIT_EXCEEDED";
    case XR_ERROR_SWAPCHAIN_RECT_INVALID:								return "XR_ERROR_SWAPCHAIN_RECT_INVALID";
    case XR_ERROR_SWAPCHAIN_FORMAT_UNSUPPORTED:							return "XR_ERROR_SWAPCHAIN_FORMAT_UNSUPPORTED";
    case XR_ERROR_ACTION_TYPE_MISMATCH:									return "XR_ERROR_ACTION_TYPE_MISMATCH";
    case XR_ERROR_SESSION_NOT_READY:									return "XR_ERROR_SESSION_NOT_READY";
    case XR_ERROR_SESSION_NOT_STOPPING:									return "XR_ERROR_SESSION_NOT_STOPPING";
    case XR_ERROR_TIME_INVALID:											return "XR_ERROR_TIME_INVALID";
    case XR_ERROR_REFERENCE_SPACE_UNSUPPORTED:							return "XR_ERROR_REFERENCE_SPACE_UNSUPPORTED";
    case XR_ERROR_FILE_ACCESS_ERROR:									return "XR_ERROR_FILE_ACCESS_ERROR";
    case XR_ERROR_FILE_CONTENTS_INVALID:								return "XR_ERROR_FILE_CONTENTS_INVALID";
    case XR_ERROR_FORM_FACTOR_UNSUPPORTED:								return "XR_ERROR_FORM_FACTOR_UNSUPPORTED";
    case XR_ERROR_FORM_FACTOR_UNAVAILABLE:								return "XR_ERROR_FORM_FACTOR_UNAVAILABLE";
    case XR_ERROR_API_LAYER_NOT_PRESENT:								return "XR_ERROR_API_LAYER_NOT_PRESENT";
    case XR_ERROR_CALL_ORDER_INVALID:									return "XR_ERROR_CALL_ORDER_INVALID";
    case XR_ERROR_GRAPHICS_DEVICE_INVALID:								return "XR_ERROR_GRAPHICS_DEVICE_INVALID";
    case XR_ERROR_POSE_INVALID:											return "XR_ERROR_POSE_INVALID";
    case XR_ERROR_INDEX_OUT_OF_RANGE:									return "XR_ERROR_INDEX_OUT_OF_RANGE";
    case XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED:					return "XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED";
    case XR_ERROR_ENVIRONMENT_BLEND_MODE_UNSUPPORTED:					return "XR_ERROR_ENVIRONMENT_BLEND_MODE_UNSUPPORTED";
    case XR_ERROR_NAME_DUPLICATED:										return "XR_ERROR_NAME_DUPLICATED";
    case XR_ERROR_NAME_INVALID:											return "XR_ERROR_NAME_INVALID";
    case XR_ERROR_ACTIONSET_NOT_ATTACHED:								return "XR_ERROR_ACTIONSET_NOT_ATTACHED";
    case XR_ERROR_ACTIONSETS_ALREADY_ATTACHED:							return "XR_ERROR_ACTIONSETS_ALREADY_ATTACHED";
    case XR_ERROR_LOCALIZED_NAME_DUPLICATED:							return "XR_ERROR_LOCALIZED_NAME_DUPLICATED";
    case XR_ERROR_LOCALIZED_NAME_INVALID:								return "XR_ERROR_LOCALIZED_NAME_INVALID";
    case XR_ERROR_GRAPHICS_REQUIREMENTS_CALL_MISSING:					return "XR_ERROR_GRAPHICS_REQUIREMENTS_CALL_MISSING";
    case XR_ERROR_RUNTIME_UNAVAILABLE:									return "XR_ERROR_RUNTIME_UNAVAILABLE";
    case XR_ERROR_ANDROID_THREAD_SETTINGS_ID_INVALID_KHR:				return "XR_ERROR_ANDROID_THREAD_SETTINGS_ID_INVALID_KHR";
    case XR_ERROR_ANDROID_THREAD_SETTINGS_FAILURE_KHR:					return "XR_ERROR_ANDROID_THREAD_SETTINGS_FAILURE_KHR";
    case XR_ERROR_CREATE_SPATIAL_ANCHOR_FAILED_MSFT:					return "XR_ERROR_CREATE_SPATIAL_ANCHOR_FAILED_MSFT";
    case XR_ERROR_SECONDARY_VIEW_CONFIGURATION_TYPE_NOT_ENABLED_MSFT:	return "XR_ERROR_SECONDARY_VIEW_CONFIGURATION_TYPE_NOT_ENABLED_MSFT";
    case XR_ERROR_CONTROLLER_MODEL_KEY_INVALID_MSFT:					return "XR_ERROR_CONTROLLER_MODEL_KEY_INVALID_MSFT";
    case XR_ERROR_REPROJECTION_MODE_UNSUPPORTED_MSFT:					return "XR_ERROR_REPROJECTION_MODE_UNSUPPORTED_MSFT";
    case XR_ERROR_COMPUTE_NEW_SCENE_NOT_COMPLETED_MSFT:					return "XR_ERROR_COMPUTE_NEW_SCENE_NOT_COMPLETED_MSFT";
    case XR_ERROR_SCENE_COMPONENT_ID_INVALID_MSFT:						return "XR_ERROR_SCENE_COMPONENT_ID_INVALID_MSFT";
    case XR_ERROR_SCENE_COMPONENT_TYPE_MISMATCH_MSFT:					return "XR_ERROR_SCENE_COMPONENT_TYPE_MISMATCH_MSFT";
    case XR_ERROR_SCENE_MESH_BUFFER_ID_INVALID_MSFT:					return "XR_ERROR_SCENE_MESH_BUFFER_ID_INVALID_MSFT";
    case XR_ERROR_SCENE_COMPUTE_FEATURE_INCOMPATIBLE_MSFT:				return "XR_ERROR_SCENE_COMPUTE_FEATURE_INCOMPATIBLE_MSFT";
    case XR_ERROR_SCENE_COMPUTE_CONSISTENCY_MISMATCH_MSFT:				return "XR_ERROR_SCENE_COMPUTE_CONSISTENCY_MISMATCH_MSFT";
    case XR_ERROR_DISPLAY_REFRESH_RATE_UNSUPPORTED_FB:					return "XR_ERROR_DISPLAY_REFRESH_RATE_UNSUPPORTED_FB";
    case XR_ERROR_COLOR_SPACE_UNSUPPORTED_FB:							return "XR_ERROR_COLOR_SPACE_UNSUPPORTED_FB";
    case XR_ERROR_SPACE_COMPONENT_NOT_SUPPORTED_FB:						return "XR_ERROR_SPACE_COMPONENT_NOT_SUPPORTED_FB";
    case XR_ERROR_SPACE_COMPONENT_NOT_ENABLED_FB:						return "XR_ERROR_SPACE_COMPONENT_NOT_ENABLED_FB";
    case XR_ERROR_SPACE_COMPONENT_STATUS_PENDING_FB:					return "XR_ERROR_SPACE_COMPONENT_STATUS_PENDING_FB";
    case XR_ERROR_SPACE_COMPONENT_STATUS_ALREADY_SET_FB:				return "XR_ERROR_SPACE_COMPONENT_STATUS_ALREADY_SET_FB";
    case XR_ERROR_UNEXPECTED_STATE_PASSTHROUGH_FB:						return "XR_ERROR_UNEXPECTED_STATE_PASSTHROUGH_FB";
    case XR_ERROR_FEATURE_ALREADY_CREATED_PASSTHROUGH_FB:				return "XR_ERROR_FEATURE_ALREADY_CREATED_PASSTHROUGH_FB";
    case XR_ERROR_FEATURE_REQUIRED_PASSTHROUGH_FB:						return "XR_ERROR_FEATURE_REQUIRED_PASSTHROUGH_FB";
    case XR_ERROR_NOT_PERMITTED_PASSTHROUGH_FB:							return "XR_ERROR_NOT_PERMITTED_PASSTHROUGH_FB";
    case XR_ERROR_INSUFFICIENT_RESOURCES_PASSTHROUGH_FB:				return "XR_ERROR_INSUFFICIENT_RESOURCES_PASSTHROUGH_FB";
    case XR_ERROR_UNKNOWN_PASSTHROUGH_FB:								return "XR_ERROR_UNKNOWN_PASSTHROUGH_FB";
    case XR_ERROR_RENDER_MODEL_KEY_INVALID_FB:							return "XR_ERROR_RENDER_MODEL_KEY_INVALID_FB";
    case XR_RENDER_MODEL_UNAVAILABLE_FB:								return "XR_RENDER_MODEL_UNAVAILABLE_FB";
    case XR_ERROR_MARKER_NOT_TRACKED_VARJO:								return "XR_ERROR_MARKER_NOT_TRACKED_VARJO";
    case XR_ERROR_MARKER_ID_INVALID_VARJO:								return "XR_ERROR_MARKER_ID_INVALID_VARJO";
    case XR_ERROR_SPATIAL_ANCHOR_NAME_NOT_FOUND_MSFT:					return "XR_ERROR_SPATIAL_ANCHOR_NAME_NOT_FOUND_MSFT";
    case XR_ERROR_SPATIAL_ANCHOR_NAME_INVALID_MSFT:						return "XR_ERROR_SPATIAL_ANCHOR_NAME_INVALID_MSFT";
    case XR_ERROR_SPACE_MAPPING_INSUFFICIENT_FB:						return "XR_ERROR_SPACE_MAPPING_INSUFFICIENT_FB";
    case XR_ERROR_SPACE_LOCALIZATION_FAILED_FB:							return "XR_ERROR_SPACE_LOCALIZATION_FAILED_FB";
    case XR_ERROR_SPACE_NETWORK_TIMEOUT_FB:								return "XR_ERROR_SPACE_NETWORK_TIMEOUT_FB";
    case XR_ERROR_SPACE_NETWORK_REQUEST_FAILED_FB:						return "XR_ERROR_SPACE_NETWORK_REQUEST_FAILED_FB";
    case XR_ERROR_SPACE_CLOUD_STORAGE_DISABLED_FB:						return "XR_ERROR_SPACE_CLOUD_STORAGE_DISABLED_FB";
    case XR_ERROR_PASSTHROUGH_COLOR_LUT_BUFFER_SIZE_MISMATCH_META:		return "XR_ERROR_PASSTHROUGH_COLOR_LUT_BUFFER_SIZE_MISMATCH_META";
    case XR_ERROR_HINT_ALREADY_SET_QCOM:								return "XR_ERROR_HINT_ALREADY_SET_QCOM";
    case XR_ERROR_SPACE_NOT_LOCATABLE_EXT:								return "XR_ERROR_SPACE_NOT_LOCATABLE_EXT";
    case XR_ERROR_PLANE_DETECTION_PERMISSION_DENIED_EXT:				return "XR_ERROR_PLANE_DETECTION_PERMISSION_DENIED_EXT";
    case XR_RESULT_MAX_ENUM:											return "XR_RESULT_MAX_ENUM";
    default:															return "";
    }
}

/*===========================================================================*/
/**
 *  @brief Return XrViewConfigurationType values to corresponding strings.
 *  @param result [in] XrViewConfigurationType
 *  @return XrViewConfigurationType values to corresponding strings.
 */
/*===========================================================================*/
std::string OpenXRDevice::xrViewConfigTypeString( XrViewConfigurationType type )
{
    switch ( type )
    {
    case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO:								return "XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO";
    case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO:								return "XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO";
    case XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO:							return "XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO";
    case XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT:	return "XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT";
    case XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM:									return "XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM";
    default:																	return "";
    }
}

/*===========================================================================*/
/**
 *  @brief Return XrReferenceSpaceType values to corresponding strings.
 *  @param result [in] XrReferenceSpaceType
 *  @return XrReferenceSpaceType values to corresponding strings.
 */
/*===========================================================================*/
std::string OpenXRDevice::xrSpaceTypeString( XrReferenceSpaceType type )
{
    switch ( type )
    {
    case XR_REFERENCE_SPACE_TYPE_VIEW:					return "XR_REFERENCE_SPACE_TYPE_VIEW";
    case XR_REFERENCE_SPACE_TYPE_LOCAL:					return "XR_REFERENCE_SPACE_TYPE_LOCAL";
    case XR_REFERENCE_SPACE_TYPE_STAGE:					return "XR_REFERENCE_SPACE_TYPE_STAGE";
    case XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT:		return "XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT";
    case XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO:	return "XR_REFERENCE_SPACE_TYPE_COMBINED_EYE_VARJO";
    case XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT:		return "XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT";
    case XR_REFERENCE_SPACE_TYPE_MAX_ENUM:				return "XR_REFERENCE_SPACE_TYPE_MAX_ENUM";
    default:											return "";
    }
}

/*===========================================================================*/
/**
 *  @brief Return XrSessionState values to corresponding strings.
 *  @param result [in] XrSessionState
 *  @return XrSessionState values to corresponding strings.
 */
/*===========================================================================*/
std::string OpenXRDevice::xrSessionStateString( XrSessionState state )
{
    switch ( state )
    {
    case XR_SESSION_STATE_UNKNOWN:		return "XR_SESSION_STATE_UNKNOWN";
    case XR_SESSION_STATE_IDLE:			return "XR_SESSION_STATE_IDLE";
    case XR_SESSION_STATE_READY:		return "XR_SESSION_STATE_READY";
    case XR_SESSION_STATE_SYNCHRONIZED:	return "XR_SESSION_STATE_SYNCHRONIZED";
    case XR_SESSION_STATE_VISIBLE:		return "XR_SESSION_STATE_VISIBLE";
    case XR_SESSION_STATE_FOCUSED:		return "XR_SESSION_STATE_FOCUSED";
    case XR_SESSION_STATE_STOPPING:		return "XR_SESSION_STATE_STOPPING";
    case XR_SESSION_STATE_LOSS_PENDING:	return "XR_SESSION_STATE_LOSS_PENDING";
    case XR_SESSION_STATE_EXITING:		return "XR_SESSION_STATE_EXITING";
    case XR_SESSION_STATE_MAX_ENUM:		return "XR_SESSION_STATE_MAX_ENUM";
    default:							return "";
    }
}

} // end of namespace openxr

} // end of namespace kvs
