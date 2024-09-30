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
#ifndef CVT__CONVERTER_XML_TAG_H_INCLUDE
#define CVT__CONVERTER_XML_TAG_H_INCLUDE

#include <algorithm>
#include <cstring>
#include <string>

namespace cvt
{
/**
 * \private
 */
namespace xml
{
constexpr const char* VERSION_TAG = "version";

constexpr const char* TARGET_TAG = "target";
constexpr const char* SUB_TARGET_TAG = "subTarget";
constexpr const char* INPUT_TAG = "input";
constexpr const char* MESH_DEFORMATION_FLAG_TAG = "meshDeformation";
constexpr const char* GHOST_CELL_COUNT_TAG = "numberOfGhostCells";
constexpr const char* BINARY_FILE_FLAG_TAG = "binary";
constexpr const char* SINGLE_TAG = "single";
constexpr const char* DISTRIBUTED_TAG = "distributed";
constexpr const char* EXTENSION_TAG = "extension";
constexpr const char* WILDCARD_TAG = "wildcard";
constexpr const char* OUTPUT_TAG = "output";
constexpr const char* DIRECTORY_PATH_TAG = "directoryPath";
constexpr const char* PREFIX_TAG = "prefix";
constexpr const char* GRID_TYPE_NAME = "gridType";

constexpr const char* TARGET_INDEX_TAG = "index";
constexpr const char* TIME_STEP_ID_TAG = "stepId";
constexpr const char* LAST_TIME_STEP_ID_TAG = "lastStepId";
constexpr const char* SUB_VOLUME_ID_TAG = "subVolumeId";
constexpr const char* SUB_VOLUME_COUNT_TAG = "numberOfSubVolumes";

constexpr const char* PFI_TAG = "pfi";
constexpr const char* LOCAL_PREFIX_NAME = "localPrefix";
constexpr const char* NODE_COUNT_TAG = "numberOfNodes";
constexpr const char* CELL_COUNT_TAG = "numberOfCells";
constexpr const char* TYPE_OF_CELL_TAG = "typeOfCells";
constexpr const char* MIN_EXTERNAL_COORDS_TAG = "minExternalCoords";
constexpr const char* MAX_EXTERNAL_COORDS_TAG = "maxExternalCoords";
constexpr const char* MIN_OBJECT_COORDS_TAG = "minObjectCoords";
constexpr const char* MAX_OBJECT_COORDS_TAG = "maxObjectCoords";
constexpr const char* VECLEN_TAG = "veclen";
constexpr const char* MIN_VALUES_TAG = "minValues";
constexpr const char* MAX_VALUES_TAG = "maxValues";

constexpr const char* VALUE_DEFINITION_TYPE = "valueDefinitionType";

constexpr const char* CONFIG_TYPE = "config";
constexpr const char* CONVERT_PROFILE_OUTPUT_TYPE = "outputConvertProfile";
} // namespace xml
} // namespace cvt

namespace cvt
{
/**
 * Remove spaces, tabs and new lines from a string.
 *
 * \see https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
 * \param[in] s A string.
 * \return A trimmed string.
 */
inline std::string& Trim( std::string& s )
{
    s.erase( s.begin(), std::find_if( s.begin(), s.end(),
                                      []( unsigned char ch ) { return !std::isspace( ch ); } ) );
    s.erase(
        std::find_if( s.rbegin(), s.rend(), []( unsigned char ch ) { return !std::isspace( ch ); } )
            .base(),
        s.end() );

    return s;
}

/**
 * \see
 * https://stackoverflow.com/questions/54512286/how-to-join-an-int-array-to-string-in-c-separated-by-a-dot
 */
template <typename Iterator>
inline std::string Join( Iterator begin, Iterator end, char separator = ',' )
{
    std::ostringstream o;
    if ( begin != end )
    {
        o << *begin++;
        for ( ; begin != end; ++begin )
            o << separator << *begin;
    }
    return o.str();
}

/**
 * Check if the tag is an expected tag.
 *
 * \param[in] tag A tag string.
 * \param[in] name An expected tag name.
 * \return `true` if the tag is same as the expected name, otherwise, `false`.
 */
inline bool IsTheXmlTag( const char* tag, const char* name )
{
    return std::strncmp( name, tag, std::strlen( name ) ) == 0;
}
} // namespace cvt
#endif // CVT__CONVERTER_XML_TAG_H_INCLUDE
