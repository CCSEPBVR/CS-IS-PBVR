/*
 * Copyright (c) 2023 Japan Atomic Energy Agency
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
#ifndef CVT__MPI_TAG_H_INCLUDE
#define CVT__MPI_TAG_H_INCLUDE

namespace cvt
{
/**
 * \private
 */
const int MPI_SERIALIZED_INPUT_LENGTH_TAG = 0;
/**
 * \private
 */
const int MPI_SERIALIZED_INPUT_TAG = 1;
/**
 * \private
 */
const int MPI_SERIALIZED_OUTPUT_LENGTH_TAG = 2;
/**
 * \private
 */
const int MPI_SERIALIZED_OUTPUT_TAG = 3;
} // namespace cvt

#endif