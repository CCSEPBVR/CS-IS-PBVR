#include "ovr.h"
#include "call.h"
#include "version.h"
#include <kvs/Assert>
#include <kvs/Macro>


namespace kvs
{

namespace oculus
{
namespace jaea
{

bool Initialize()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG kvs::oculus::jaea::Initialize() called." << std::endl;
#endif // DEBUG_SCREEN



#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    ovrResult result = ovrSuccess;
    //    ovrInitParams params = { ovrInit_RequestVersion | ovrInit_FocusAware | ovrInit_MixedRendering, OVR_MINOR_VERSION, NULL, 0, 0 };
    ovrInitParams params;
    params.Flags = ovrInit_RequestVersion | ovrInit_FocusAware | ovrInit_MixedRendering;
    params.RequestedMinorVersion = OVR_MINOR_VERSION;
    params.LogCallback = NULL;
    params.UserData = 0;
    params.ConnectionTimeoutMS = 0;
    //KVS_OVR_CALL( result = ovr_Initialize( &params ) );
    result = ovr_Initialize( &params );

    std::cout << "Oculus initialized using SDK Version " << kvs::oculus::jaea::Version() << std::endl;

    //KVS_OVR_CALL( result = ovr_Initialize( NULL ) );
    return OVR_SUCCESS( result );
#else
    ovrBool result = ovrTrue;
    KVS_OVR_CALL( result = ovr_Initialize( NULL ) );
    return result == ovrTrue;
#endif
}

void Shutdown()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG kvs::oculus::jaea::Shutdown() called." << std::endl;
#endif // DEBUG_SCREEN


    KVS_OVR_CALL( ovr_Shutdown() );
}

int Detect()
{

#ifdef DEBUG_SCREEN
    std::cout << "MSG kvs::oculus::jaea::Detect() called." << std::endl;
#endif // DEBUG_SCREEN

#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 7, 0 )
    ovrHmdDesc desc = {};
    KVS_OVR_CALL( desc = ovr_GetHmdDesc( NULL ) );
    return desc.Type == ovrHmd_None ? 0 : 1;
#else
    int result = 0;
    KVS_OVR_CALL( result = ovrHmd_Detect() );
    return result;
#endif
}

std::string VersionString()
{
    std::string result;
    KVS_OVR_CALL( result = ovr_GetVersionString() );
    return result;
}

double TimeInSecond()
{
    double result = 0.0;
    KVS_OVR_CALL( result = ovr_GetTimeInSeconds() );
    return result;
}

int TraceMessage( int level, const char* message )
{
    int result = 0;
    KVS_OVR_CALL( result = ovr_TraceMessage( level, message ) );
    return result;
}

} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
