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
#ifndef CVT__CONVERTER_CONVERTER_TASK_INPUT_H_INCLUDE
#define CVT__CONVERTER_CONVERTER_TASK_INPUT_H_INCLUDE

#include <iostream>
#include <string>
#include <vector>

namespace cvt
{

/**
 * A class of data which is passed to a converter task.
 *
 * This stores information of a distributed files in one time step.
 */
class ConverterTaskInput
{
public:
    /**
     * A target index.
     */
    int target_index;
    /**
     * File paths to convert.
     */
    std::vector<std::string> source_file_paths;
    /**
     * Q file paths for plot3d.
     */
    std::vector<std::string> q;
    /**
     * F file paths for plot3d.
     */
    std::vector<std::string> f;
    /**
     * A grid type of a source file.
     */
    std::string source_grid_type;
    /**
     * A directory path to output.
     */
    std::string destination_directory;
    /**
     * A prefix of output files.
     */
    std::string destination_prefix;
    /**
     * A time step of this.
     */
    int time_step;
    /**
     * The last time step.
     */
    int last_time_step;
    /**
     * An ID of this sub volume.
     */
    int sub_volume_id;
    /**
     * A count of sub volumes including this.
     */
    int sub_volume_count;
    /**
     * A flag to output a convert profile.
     */
    int output_profile;
    /**
     * A flag that coords files are shared.
     */
    int has_mesh_deformation;
    /**
     * A flag that the files are binary.
     */
    int is_binary;

public:
    ConverterTaskInput(): output_profile( true ) {}

public:
    /**
     * Serialize this.
     *
     * \return A XML string.
     */
    std::string serialize() { return static_cast<const ConverterTaskInput&>( *this ).serialize(); }
    /**
     * Serialize this.
     *
     * \return A XML string.
     */
    std::string serialize() const;
    /**
     * Deserialize this.
     * \param[in] expression A XML string.
     */
    void deserialize( const std::string& expression );
};
} // namespace cvt

namespace cvt
{

/**
 * Print a task input to stream.
 *
 * \param[inout] stream A stream.
 * \param[in] e A task input.
 * \return `stream`.
 */
template <typename Stream>
inline Stream& operator<<( Stream& stream, const cvt::ConverterTaskInput& e )
{
    return ( stream << e.serialize() );
}
} // namespace cvt
#endif // CVT__CONVERTER_CONVERTER_TASK_INPUT_H_INCLUDE