#pragma once
#ifndef OVR_H
#define OVR_H

#include "oculus.h"
#include <kvs/Matrix44>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <string>

const float OCULUS_FAR = 2000.0f;
const float OCULUS_NEAR = 0.1f;

// UNIT=METER
// const float ZED_FAR = 15.0f;
// const float ZED_NEAR = 0.2f;

// UNIT=MILLIMETER
const float ZED_FAR = 15.0e3f;
const float ZED_NEAR = 0.2e3f;

bool Initialize();
void Shutdown();
int Detect();
std::string VersionString();
double TimeInSecond();
int TraceMessage(int level, const char *message);

inline kvs::Vec2i ToVec2i(const ovrVector2i &v) { return kvs::Vec2i(v.x, v.y); }

inline kvs::Vec2i ToVec2i(const ovrSizei &s) { return kvs::Vec2i(s.w, s.h); }

inline kvs::Vec2 ToVec2(const ovrVector2f &v) { return kvs::Vec2(v.x, v.y); }

inline kvs::Vec3 ToVec3(const ovrVector3f &v) { return kvs::Vec3(v.x, v.y, v.z); }

inline kvs::Mat4 ToMat4(const ovrMatrix4f &m) {
  const kvs::Vec4 m0(m.M[0]);
  const kvs::Vec4 m1(m.M[1]);
  const kvs::Vec4 m2(m.M[2]);
  const kvs::Vec4 m3(m.M[3]);
  return kvs::Mat4(m0, m1, m2, m3);
}

#endif // OVR_H
