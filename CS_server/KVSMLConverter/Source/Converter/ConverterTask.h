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
#ifndef CVT__CONVERTER_TASK_H_INCLUDE
#define CVT__CONVERTER_TASK_H_INCLUDE

#include <optional>

#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"

namespace cvt
{

/**
 * Convert files.
 *
 * \param[in] input A convert configuration.
 * \return A convert result if success. Otherwise std::nullopt.
 */
std::optional<cvt::ConverterTaskOutput> Convert( cvt::ConverterTaskInput input );
} // namespace cvt

#endif // CVT__CONVERTER_TASK_H_INCLUDE