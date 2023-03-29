#pragma once
#ifndef VERSION_H
#define VERSION_H
#include <OVR_Version.h>

#if OVR_PRODUCT_VERSION
#define KVS_OVR_MAJOR_VERSION OVR_MAJOR_VERSION
#define KVS_OVR_MINOR_VERSION OVR_MINOR_VERSION
#define KVS_OVR_PATCH_VERSION OVR_PATCH_VERSION
#else
#define KVS_OVR_MAJOR_VERSION 0
#define KVS_OVR_MINOR_VERSION OVR_MAJOR_VERSION
#define KVS_OVR_PATCH_VERSION OVR_MINOR_VERSION
#endif

#define KVS_OVR_MAKE_VERSION( major, minor, patch )  \
    ( major * 10000 +                                \
      minor * 100 +                                  \
      patch )

#define KVS_OVR_VERSION                              \
    KVS_OVR_MAKE_VERSION( KVS_OVR_MAJOR_VERSION,     \
                          KVS_OVR_MINOR_VERSION,     \
                          KVS_OVR_PATCH_VERSION )

#define KVS_OVR_VERSION_LESS_THAN( major, minor, patch )                \
    ( KVS_OVR_VERSION < KVS_OVR_MAKE_VERSION( major, minor, patch ) )

#define KVS_OVR_VERSION_LESS_OR_EQUAL( major, minor, patch )            \
    ( KVS_OVR_VERSION <= KVS_OVR_MAKE_VERSION( major, minor, patch ) )

#define KVS_OVR_VERSION_GREATER_THAN( major, minor, patch )             \
    ( KVS_OVR_VERSION > KVS_OVR_MAKE_VERSION( major, minor, patch ) )

#define KVS_OVR_VERSION_GREATER_OR_EQUAL( major, minor, patch )        \
    ( KVS_OVR_VERSION >= KVS_OVR_MAKE_VERSION( major, minor, patch ) )

#define KVS_OVR_VERSION_EQUAL( major, minor, patch )                    \
    ( KVS_OVR_VERSION == KVS_OVR_MAKE_VERSION( major, minor, patch ) )


#endif // VERSION_H
