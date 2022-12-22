/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__FILESYSTEM_H_INCLUDE
#define CVT__FILESYSTEM_H_INCLUDE

#pragma GCC warning                                                                                \
    "This header is for gcc < 8. This must be replaced with <filesystem> in the future"

#if __has_include( <filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std
{
namespace filesystem = std::experimental::filesystem;
} // namespace std
#endif

#endif // CVT__FILESYSTEM_H_INCLUDE