/*
 * Copyright (c) 2026 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CVT_NETCDF_H_INCLUDE
#define CVT_NETCDF_H_INCLUDE

#include <map>
#include <memory>
#include <string>

#include "kvs/FileFormatBase"

namespace cvt
{

enum class NetcdfGridType
{
    Unknown,
    ImageData,
    RectilinearGrid,
    StructuredGrid,
    UnstructuredGrid
};

class NetcdfMetadata
{
public:
    bool hasVariable( const std::string& name, const std::string& dimensions ) const;
    const std::map<std::string, std::string>& variableDimensions() const
    {
        return m_variable_dimensions;
    }

private:
    std::map<std::string, std::string> m_variable_dimensions;

    friend class Netcdf;
};

struct NetcdfFileInfo
{
    std::string format_name;
    NetcdfGridType grid_type = NetcdfGridType::Unknown;
    bool supports_time_series = false;
    std::string time_key;
};

class NetcdfFormatAdapter
{
public:
    virtual ~NetcdfFormatAdapter() = default;

    virtual const char* name() const = 0;
    virtual NetcdfGridType gridType() const = 0;
    virtual bool matches( const NetcdfMetadata& metadata ) const = 0;
    virtual std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const = 0;
    virtual bool supportsTimeSeries() const { return false; }
    virtual bool extractTimeKey( const std::string&, const NetcdfMetadata&, std::string&,
                                 std::string& error ) const
    {
        error = std::string( name() ) + " NetCDF does not support wildcard time series";
        return false;
    }
};

class Netcdf : public kvs::FileFormatBase
{
public:
    Netcdf() = default;
    explicit Netcdf( const std::string& filename );

public:
    bool read( const std::string& filename ) override;
    bool write( const std::string& filename ) override;

    const std::shared_ptr<kvs::FileFormatBase>& format() const { return m_format; }
    const std::string& formatName() const { return m_format_name; }
    NetcdfGridType gridType() const { return m_grid_type; }

    static bool Probe( const std::string& filename, NetcdfFileInfo& info,
                       bool require_time_key = false );

private:
    static bool ReadMetadata( const std::string& filename, NetcdfMetadata& metadata );
    static const NetcdfFormatAdapter* SelectAdapter( const NetcdfMetadata& metadata );

private:
    std::shared_ptr<kvs::FileFormatBase> m_format;
    std::string m_format_name;
    NetcdfGridType m_grid_type = NetcdfGridType::Unknown;
};
} // namespace cvt

#endif // CVT_NETCDF_H_INCLUDE
