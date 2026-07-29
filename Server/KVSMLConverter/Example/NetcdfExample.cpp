/*
 * Created by Japan Atomic Energy Agency
 *
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legal code along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <vtkGlobFileNames.h>
#include <vtkNew.h>
#include <vtkStringArray.h>

#include "Exporter/StructuredVolumeObjectExporter.h"
#include "Exporter/UnstructuredVolumeObjectExporter.h"
#include "FileFormat/NetCDF/Netcdf.h"
#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Filesystem.h"
#include "Importer/VtkImporter.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"
#include "kvs/StructuredVolumeObject"
#include "kvs/UnstructuredVolumeObject"
#include "kvs/VolumeObjectBase"

struct TimestampedNetcdfFile
{
    std::string time_key;
    std::string path;
    std::string format_name;
    cvt::NetcdfGridType grid_type = cvt::NetcdfGridType::Unknown;
};

std::unique_ptr<kvs::VolumeObjectBase> ImportNetcdfVolume( cvt::Netcdf& input )
{
    if ( auto* format = dynamic_cast<cvt::VtkXmlImageData*>( input.format().get() ) )
    {
        auto importer = std::make_unique<cvt::VtkImporter<cvt::VtkXmlImageData>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }
    if ( auto* format = dynamic_cast<cvt::VtkXmlRectilinearGrid*>( input.format().get() ) )
    {
        auto importer =
            std::make_unique<cvt::VtkImporter<cvt::VtkXmlRectilinearGrid>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }
    if ( auto* format = dynamic_cast<cvt::VtkXmlStructuredGrid*>( input.format().get() ) )
    {
        auto importer =
            std::make_unique<cvt::VtkImporter<cvt::VtkXmlStructuredGrid>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }
    if ( auto* format = dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( input.format().get() ) )
    {
        auto importer =
            std::make_unique<cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid>>( format );
        if ( importer->isFailure() )
        {
            return nullptr;
        }
        return importer;
    }

    std::cerr << "Unsupported VTK grid type returned by the " << input.formatName()
              << " NetCDF adapter." << std::endl;
    return nullptr;
}

bool WriteNetcdfVolume( const std::string& directory, const std::string& local_base,
                        const std::string& source, int time_step, int sub_volume_id,
                        int sub_volume_count, kvs::VolumeObjectBase* volume,
                        cvt::UnstructuredPfi& pfi )
{
    volume->print( std::cout, kvs::Indent( 4 ) );
    std::cout << "  Writing to " << directory << " ..." << std::endl;

    if ( auto* unstructured = dynamic_cast<kvs::UnstructuredVolumeObject*>( volume ) )
    {
        std::cout << "  cell type: " << unstructured->cellType() << std::endl;
        cvt::UnstructuredVolumeObjectExporter exporter( unstructured );
        exporter.setWritingDataTypeToExternalBinary();
        if ( !exporter.write( directory, local_base, time_step, sub_volume_id,
                              sub_volume_count, false ) )
        {
            std::cerr << "Failed to write NetCDF time step " << time_step
                      << ": " << source << std::endl;
            return false;
        }
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        return true;
    }

    if ( auto* structured = dynamic_cast<kvs::StructuredVolumeObject*>( volume ) )
    {
        structured->updateMinMaxCoords();
        structured->setMinMaxExternalCoords( structured->minObjectCoord(),
                                             structured->maxObjectCoord() );
        cvt::StructuredVolumeObjectExporter exporter( structured );
        exporter.setWritingDataTypeToExternalBinary();
        if ( !exporter.write( directory, local_base, time_step, sub_volume_id,
                              sub_volume_count, false ) )
        {
            std::cerr << "Failed to write NetCDF time step " << time_step
                      << ": " << source << std::endl;
            return false;
        }
        pfi.registerObject( &exporter, time_step, sub_volume_id );
        return true;
    }

    std::cerr << "The NetCDF adapter did not produce a KVS volume object: "
              << source << std::endl;
    return false;
}

void Netcdf2Kvsml( const std::string& directory, const std::string& base,
                   const std::string& src )
{
    std::cout << "Reading " << src << " ..." << std::endl;
    cvt::Netcdf input( src );
    if ( input.isFailure() )
    {
        std::cerr << "Failed to read the NetCDF file: " << src << std::endl;
        return;
    }

    auto volume = ImportNetcdfVolume( input );
    if ( !volume )
    {
        std::cerr << "Failed to import the " << input.formatName()
                  << " NetCDF file: " << src << std::endl;
        return;
    }

    constexpr int time_step = 0;
    constexpr int last_time_step = 0;
    constexpr int sub_volume_id = 1;
    constexpr int sub_volume_count = 1;
    std::string local_base = base;
    if ( auto* unstructured =
             dynamic_cast<kvs::UnstructuredVolumeObject*>( volume.get() ) )
    {
        local_base += "_" + std::to_string( unstructured->cellType() );
    }

    cvt::UnstructuredPfi pfi( volume->veclen(), last_time_step, sub_volume_count );
    if ( !WriteNetcdfVolume( directory, local_base, src, time_step, sub_volume_id,
                             sub_volume_count, volume.get(), pfi ) )
    {
        return;
    }
    if ( !pfi.write( directory, local_base ) )
    {
        std::cerr << "Failed to write the NetCDF PFI file." << std::endl;
        return;
    }

    cvt::Pfl pfl;
    pfl.registerPfi( directory, local_base );
    if ( !pfl.write( directory, base ) )
    {
        std::cerr << "Failed to write the NetCDF PFL file." << std::endl;
    }
}

bool ListNetcdfTimeSeriesFiles( const std::string& pattern,
                                std::vector<TimestampedNetcdfFile>& timestamped_files )
{
    vtkNew<vtkGlobFileNames> glob;
    glob->RecurseOff();
    glob->AddFileNames( pattern.c_str() );

    auto filenames = glob->GetFileNames();
    if ( filenames->GetNumberOfValues() == 0 )
    {
        std::cerr << "No NetCDF files matched: " << pattern << std::endl;
        return false;
    }

    timestamped_files.reserve( filenames->GetNumberOfValues() );
    std::string expected_format;
    cvt::NetcdfGridType expected_grid_type = cvt::NetcdfGridType::Unknown;
    for ( int i = 0; i < filenames->GetNumberOfValues(); ++i )
    {
        const std::string path = filenames->GetValue( i );
        cvt::NetcdfFileInfo info;
        if ( !cvt::Netcdf::Probe( path, info, true ) )
        {
            return false;
        }
        if ( i == 0 )
        {
            expected_format = info.format_name;
            expected_grid_type = info.grid_type;
        }
        else if ( info.format_name != expected_format ||
                  info.grid_type != expected_grid_type )
        {
            std::cerr << "NetCDF time series mixes formats or VTK grid types: expected "
                      << expected_format << ", but " << path << " was detected as "
                      << info.format_name << std::endl;
            return false;
        }

        timestamped_files.push_back(
            { info.time_key, path, info.format_name, info.grid_type } );
    }

    std::sort(
        timestamped_files.begin(), timestamped_files.end(),
        []( const auto& lhs, const auto& rhs ) {
            return lhs.time_key != rhs.time_key ? lhs.time_key < rhs.time_key
                                                 : lhs.path < rhs.path;
        } );

    for ( std::size_t i = 1; i < timestamped_files.size(); ++i )
    {
        if ( timestamped_files[i - 1].time_key == timestamped_files[i].time_key )
        {
            std::cerr << "Duplicate NetCDF time key "
                      << timestamped_files[i].time_key << ": "
                      << timestamped_files[i - 1].path << " and "
                      << timestamped_files[i].path << std::endl;
            return false;
        }
    }

    return true;
}

std::string NetcdfTimeSeriesBase( const cvt::filesystem::path& pattern )
{
    std::string base = pattern.filename().stem().string();
    const auto wildcard = base.find( '*' );
    if ( wildcard != std::string::npos )
    {
        base.erase( wildcard );
    }
    while ( !base.empty() &&
            ( base.back() == '_' || base.back() == '-' || base.back() == '.' ) )
    {
        base.pop_back();
    }
    return base;
}

void SeriesNetcdf2Kvsml( const std::string& directory, const std::string& base,
                         const std::string& src )
{
    if ( base.empty() )
    {
        std::cerr << "Could not derive an output prefix from the NetCDF wildcard."
                  << std::endl;
        return;
    }

    std::vector<TimestampedNetcdfFile> timestamped_files;
    if ( !ListNetcdfTimeSeriesFiles( src, timestamped_files ) )
    {
        return;
    }

    const int last_time_step = static_cast<int>( timestamped_files.size() ) - 1;
    constexpr int sub_volume_id = 1;
    constexpr int sub_volume_count = 1;
    int expected_cell_type = -1;
    int expected_grid_type = -1;
    int expected_veclen = -1;
    kvs::VolumeObjectBase::VolumeType expected_volume_type =
        kvs::VolumeObjectBase::UnknownVolumeType;
    std::string local_base;
    std::unique_ptr<cvt::UnstructuredPfi> pfi;

    for ( std::size_t i = 0; i < timestamped_files.size(); ++i )
    {
        const int time_step = static_cast<int>( i );
        const auto& file = timestamped_files[i];

        std::cout << "Reading " << file.path << " as time step " << time_step
                  << " (" << file.time_key << ") ..." << std::endl;
        cvt::Netcdf input( file.path );
        if ( input.isFailure() || input.formatName() != file.format_name ||
             input.gridType() != file.grid_type )
        {
            std::cerr << "Failed to read the preflighted NetCDF time step "
                      << time_step << ": " << file.path << std::endl;
            return;
        }

        auto volume = ImportNetcdfVolume( input );
        if ( !volume )
        {
            std::cerr << "Failed to import NetCDF time step " << time_step
                      << ": " << file.path << std::endl;
            return;
        }

        int cell_type = -1;
        int grid_type = -1;
        if ( auto* unstructured =
                 dynamic_cast<kvs::UnstructuredVolumeObject*>( volume.get() ) )
        {
            cell_type = static_cast<int>( unstructured->cellType() );
        }
        else if ( auto* structured =
                      dynamic_cast<kvs::StructuredVolumeObject*>( volume.get() ) )
        {
            grid_type = static_cast<int>( structured->gridType() );
        }

        if ( time_step == 0 )
        {
            expected_volume_type = volume->volumeType();
            expected_cell_type = cell_type;
            expected_grid_type = grid_type;
            expected_veclen = static_cast<int>( volume->veclen() );
            local_base =
                expected_volume_type == kvs::VolumeObjectBase::Unstructured
                    ? base + "_" + std::to_string( expected_cell_type )
                    : base;
            pfi = std::make_unique<cvt::UnstructuredPfi>(
                expected_veclen, last_time_step, sub_volume_count );
        }
        else if ( volume->volumeType() != expected_volume_type ||
                  cell_type != expected_cell_type ||
                  grid_type != expected_grid_type ||
                  static_cast<int>( volume->veclen() ) != expected_veclen )
        {
            std::cerr << "NetCDF time-series structure differs at " << file.path
                      << ": expected volume type " << expected_volume_type
                      << ", cell type " << expected_cell_type << ", grid type "
                      << expected_grid_type << ", and veclen " << expected_veclen
                      << ", but got volume type " << volume->volumeType()
                      << ", cell type " << cell_type << ", grid type " << grid_type
                      << ", and veclen " << volume->veclen() << std::endl;
            return;
        }

        if ( !WriteNetcdfVolume( directory, local_base, file.path, time_step,
                                 sub_volume_id, sub_volume_count, volume.get(), *pfi ) )
        {
            return;
        }
    }

    if ( !pfi->write( directory, local_base ) )
    {
        std::cerr << "Failed to write the NetCDF time-series PFI file." << std::endl;
        return;
    }

    cvt::Pfl pfl;
    pfl.registerPfi( directory, local_base );
    if ( !pfl.write( directory, base ) )
    {
        std::cerr << "Failed to write the NetCDF time-series PFL file." << std::endl;
    }
}
