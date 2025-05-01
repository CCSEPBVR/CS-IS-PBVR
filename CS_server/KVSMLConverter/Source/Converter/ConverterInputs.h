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
#ifndef CVT__CONVERTER_INPUTS_H_INCLUDE
#define CVT__CONVERTER_INPUTS_H_INCLUDE

#include <iostream>
#include <list>
#include <optional>
#include <string>

#include "Converter/ConverterTaskInput.h"
#include "Filesystem.h"

namespace cvt
{
/**
 * Print a task input list.
 *
 * \param[in] l A task input list
 */
inline void PrintInputFileList( std::list<cvt::ConverterTaskInput>& l )
{
    for ( auto& e : l )
    {
        std::cout << e << std::endl;
    }
}
} // namespace cvt

namespace cvt
{

/**
 * Check if the file option is valid.
 *
 * \param[in] arg An element of argv.
 * \return A XML file or std::nullopt.
 */
std::optional<cvt::filesystem::path> CheckConfigFile( const char* arg ) noexcept;
/**
 * Parse a XML file and list target files to convert.
 *
 * \param[in] xml_file_path A path to a XML file.
 * \return A list of data which is passed to a converter task.
 */
std::list<cvt::ConverterTaskInput> ListInputFilesFromConfigFile( const char* xml_file_path );
} // namespace cvt
#endif // CVT__CONVERTER_INPUTS_H_INCLUDE