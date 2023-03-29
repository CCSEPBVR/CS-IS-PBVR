#include "call.h"
#include "version.h"
#include <string>
#include <iostream>
#include <kvs/Breakpoint>
#include <kvs/ColorStream>

#include <OVR_CAPI.h>


namespace kvs
{

namespace oculus
{

namespace jaea
{

namespace internal
{

bool HasError(
    const char* file,
    const int line,
    const char* func,
    const char* command )
{
#if KVS_OVR_VERSION_GREATER_OR_EQUAL( 0, 6, 0 )
    ovrErrorInfo error;
    ovr_GetLastErrorInfo( &error );
    if ( OVR_SUCCESS( error.Result ) ) { return false; }
    std::string message( error.ErrorString );
#else
    const char* error = ovrHmd_GetLastError( NULL );
    if ( error == NULL ) { return false; }
    std::string message;
    while ( *error ) message += *error++;
#endif

    // Output message tag.
    std::cerr << kvs::ColorStream::Bold
              << kvs::ColorStream::Red
              << "KVS OVR ERROR"
              << kvs::ColorStream::Reset
              << ": ";

    // Output message with an error string.
    std::cerr << kvs::ColorStream::Underline( kvs::ColorStream::Bold( message ) ) << std::endl;
    std::cerr << "    " << "Func: " << func << std::endl;
    std::cerr << "    " << "File: " << file << ":" << line << std::endl;
    std::cerr << "    " << "OVR Command: " << command << std::endl;

    return true;
}

} // end of namespace internal

} // end of namespace jaea

} // end of namespace oculus

} // end of namespace kvs
