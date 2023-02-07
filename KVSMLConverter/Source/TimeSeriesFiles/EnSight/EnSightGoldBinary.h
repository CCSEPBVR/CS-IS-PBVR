/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CVT__ENSIGHT_GOLD_BINARY_H_INCLUDE
#define CVT__ENSIGHT_GOLD_BINARY_H_INCLUDE

#include "EnSightGold.h"

namespace cvt
{
/*
 * An interface of a binary EnSight Gold file.
 *
 * This class is to access each time step data in an EnSight Gold file.
 */
using EnSightGoldBinary = cvt::EnSightGold<vtkEnSightGoldBinaryReader>;
} // namespace cvt

#endif // CVT__ENSIGHT_GOLD_BINARY_H_INCLUDE