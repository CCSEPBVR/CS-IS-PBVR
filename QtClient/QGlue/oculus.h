#pragma once
#ifndef OCULUS_H
#define OCULUS_H

#include <Extras/OVR_Math.h>
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <OVR_Version.h>
#include <kvs/OpenGL>
#include <string>

inline const std::string Description() {
  const std::string description("Oculus Rift SDK");
  return description;
}

inline const std::string Version() {
  const std::string version(OVR_VERSION_STRING);
  return version;
}

#endif // OCULUS_H
