/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__CVT_MODE_H_INCLUDE
#define CVT__CVT_MODE_H_INCLUDE

namespace cvt
{

/**
 * A uniform gird mode to import/export a structured grid.
 */
constexpr int UNIFORM_GRID_MODE = 0;
/**
 * A curvilinear gird mode to import/export a structured grid.
 */
constexpr int CURVILINEAR_GRID_MODE = 1;
} // namespace cvt
#endif // CVT__CVT_MODE_H_INCLUDE
