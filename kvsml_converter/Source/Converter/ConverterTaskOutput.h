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
#ifndef CVT__CONVERTER_CONVERTER_TASK_OUTPUT_H_INCLUDE
#define CVT__CONVERTER_CONVERTER_TASK_OUTPUT_H_INCLUDE

#include <limits>
#include <string>
#include <unordered_map>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/KVSMLUnstructuredVolumeObject"
#include "kvs/Vector3"

namespace cvt
{

/**
 * A class of data which is returned from a converter task.
 */
class ConverterTaskOutput
{
public:
    /**
     * Construct the empty converter returns.
     * This constructor should use for only temporary generations like a list element.
     *
     * \param[in] target_index A target index.
     * \param[in] time_step A time step ID.
     */
    ConverterTaskOutput( int target_index, int time_step ):
        target_index( target_index ), time_step( time_step )
    {
    }
    /**
     * Construct the empty converter returns.
     *
     * \param[in] target_index A target index.
     * \param[in] time_step A time step ID.
     * \param[in] last_time_step The last time step ID.
     * \param[in] has_mesh_deformation A flag that the time series has a mesh deformation.
     * \param[in] sub_volume_counts A table of a volume file prefix and sub volume counts.
     * \param[in] ghost_cell_count A count of ghost cells.
     */
    ConverterTaskOutput( int target_index, int time_step, int last_time_step,
                         int has_mesh_deformation,
                         std::unordered_map<std::string, int> sub_volume_counts,
                         int ghost_cell_count ):
        target_index( target_index ),
        time_step( time_step ),
        last_time_step( last_time_step ),
        has_mesh_deformation( has_mesh_deformation ),
        sub_volume_counts( sub_volume_counts ),
        ghost_cell_count( ghost_cell_count )
    {
        for ( auto& e : sub_volume_counts )
        {
            auto k = e.first;
            auto count = e.second;

            node_counts[k].resize( count );
            cell_counts[k].resize( count );
            min_object_coords[k].resize( count );
            max_object_coords[k].resize( count );
        }

        min_external_coords = kvs::Vec3::Constant( std::numeric_limits<float>::max() );
        max_external_coords = kvs::Vec3::Constant( std::numeric_limits<float>::lowest() );
    }

public:
    /**
     * Check if this was generated for temporary purpose.
     *
     * \return `true` if this was generated for a temporary purpose, otherwise `false`.
     */
    bool isEmpty() { return static_cast<const ConverterTaskOutput*>( this )->isEmpty(); }
    /**
     * Check if this was generated for temporary purpose.
     *
     * \return `true` if this was generated for a temporary purpose, otherwise `false`.
     */
    bool isEmpty() const { return sub_volume_counts.size() == 0; }

public:
    /**
     * Register a KVS object to this.
     *
     * \param[in] object A KVS object.
     * \param[in] local_base A prefix of output files.
     * \param[in] sub_volume_id A sub volume ID.
     */
    void registerObject( kvs::KVSMLUnstructuredVolumeObject* object, std::string local_base,
                         int sub_volume_id );
    /**
     * Register a KVS object to this.
     *
     * \param[in] object A KVS object.
     * \param[in] local_base A prefix of output files.
     * \param[in] sub_volume_id A sub volume ID.
     */
    void registerObject( kvs::KVSMLStructuredVolumeObject* object, std::string local_base,
                         int sub_volume_id );
    /**
     * Serialize this.
     *
     * \return A serialized string.
     */
    std::string serialize() const;
    /**
     * Serialize this.
     *
     * \return A serialized string.
     */
    std::string serialize() { return static_cast<const ConverterTaskOutput*>( this )->serialize(); }
    /**
     * Deserialize this.
     *
     * \param[in] expression A serialized string.
     */
    void deserialize( const std::string& expression );

public:
    /**
     * A target index.
     */
    int target_index;
    /**
     * A time step ID.
     */
    int time_step;
    /**
     * A last time step ID.
     */
    int last_time_step;
    /**
     * A flag that the time series has a mesh deformation.
     */
    int has_mesh_deformation;
    /**
     * A grid type string.
     */
    std::string grid_type;
    // <local_base>_<time_step>_<sub_volume_ids>
    /**
     * A table of a volume file prefix and a type of cells.
     */
    std::unordered_map<std::string, int> type_of_cells;
    /**
     * A table of a volume file prefix and sub volume counts.
     */
    std::unordered_map<std::string, int> sub_volume_counts;
    /**
     * A table of a volume file prefix and node counts.
     */
    std::unordered_map<std::string, std::vector<int>> node_counts;
    /**
     * A table of a volume file prefix and cell counts.
     */
    std::unordered_map<std::string, std::vector<int>> cell_counts;
    /**
     * The min external coords.
     */
    kvs::Vec3 min_external_coords;
    /**
     * The max external coords.
     */
    kvs::Vec3 max_external_coords;
    /**
     * A table of a volume file prefix and min object coords.
     */
    std::unordered_map<std::string, std::vector<kvs::Vec3>> min_object_coords;
    /**
     * A table of a volume file prefix and max object coords.
     */
    std::unordered_map<std::string, std::vector<kvs::Vec3>> max_object_coords;
    /**
     * A table of a volume file prefix and value min/max.
     */
    std::unordered_map<std::string, std::tuple<std::vector<float>, std::vector<float>>>
        value_minmax;
    /**
     * A structured grid resolution.
     * This value is set only from a structured grid.
     */
    kvs::Vec3ui resolution;
    /**
     * A ghost cell count
     */
    int ghost_cell_count;
};
} // namespace cvt

#endif // CVT__CONVERTER_CONVERTER_TASK_OUTPUT_H_INCLUDE
