#ifndef OBJECT_INFO_UTILS_H
#define OBJECT_INFO_UTILS_H

#include "../../Shared/ObjectInfoExtractor.h"

namespace ServerObjectUtils
{

inline void DestroyObject( ObjectInfoExtractor::ObjectInfo& info ) noexcept
{
    delete info.object;
    info.object = nullptr;
}

} // namespace ServerObjectUtils

#endif // OBJECT_INFO_UTILS_H
