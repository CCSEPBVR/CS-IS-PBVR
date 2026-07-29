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
#include "FileFormat/NetCDF/Netcdf.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "Filesystem.h"
#include "kvs/Message"
#include "kvs/Type"
#include <vtkCallbackCommand.h>
#include <vtkCellType.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkErrorCode.h>
#include <vtkFloatArray.h>
#include <vtkNetCDFCFReader.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

namespace cvt
{
namespace detail
{

struct NetcdfDiagnostics
{
    std::string phase;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

struct NetcdfDimensions
{
    vtkIdType x = 0;
    vtkIdType y = 0;
    vtkIdType z = 0;
};

void OnNetcdfVtkMessage( vtkObject*, unsigned long event_id, void* client_data, void* call_data )
{
    auto* diagnostics = static_cast<NetcdfDiagnostics*>( client_data );
    const char* message = static_cast<const char*>( call_data );
    std::ostringstream text;
    text << diagnostics->phase << ": " << ( message ? message : "(no message)" );

    if ( event_id == vtkCommand::ErrorEvent )
    {
        diagnostics->errors.push_back( text.str() );
    }
    else
    {
        diagnostics->warnings.push_back( text.str() );
    }
}

void ObserveNetcdfReader( vtkNetCDFCFReader* reader, vtkCallbackCommand* callback,
                          NetcdfDiagnostics& diagnostics )
{
    callback->SetClientData( &diagnostics );
    callback->SetCallback( OnNetcdfVtkMessage );
    reader->AddObserver( vtkCommand::ErrorEvent, callback );
    reader->AddObserver( vtkCommand::WarningEvent, callback );
}

bool SelectNetcdfVariables( vtkNetCDFCFReader* reader, const std::string& dimensions,
                            const std::vector<std::string>& variable_names,
                            const std::string& phase, NetcdfDiagnostics& diagnostics )
{
    diagnostics.phase = phase + " metadata";
    if ( reader->UpdateMetaData() == 0 )
    {
        diagnostics.errors.push_back( diagnostics.phase + ": failed to read NetCDF metadata" );
        return false;
    }

    vtkStringArray* variable_dimensions = reader->GetVariableDimensions();
    bool valid = true;
    for ( const auto& variable_name : variable_names )
    {
        bool found = false;
        for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
        {
            if ( variable_name == reader->GetVariableArrayName( i ) )
            {
                found = true;
                const std::string actual_dimensions = variable_dimensions->GetValue( i );
                if ( actual_dimensions != dimensions )
                {
                    diagnostics.errors.push_back(
                        phase + ": variable " + variable_name + " has dimensions " +
                        actual_dimensions + "; expected " + dimensions );
                    valid = false;
                }
                break;
            }
        }
        if ( !found )
        {
            diagnostics.errors.push_back( phase + ": required variable " + variable_name +
                                          " was not found" );
            valid = false;
        }
    }
    if ( !valid )
    {
        return false;
    }

    for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
    {
        reader->SetVariableArrayStatus( reader->GetVariableArrayName( i ), 0 );
    }
    for ( const auto& variable_name : variable_names )
    {
        reader->SetVariableArrayStatus( variable_name.c_str(), 1 );
    }
    return true;
}

vtkDataArray* GetRequiredNetcdfArray( vtkDataSet* data_set, const char* name,
                                      const std::string& phase,
                                      NetcdfDiagnostics& diagnostics )
{
    if ( !data_set )
    {
        diagnostics.errors.push_back( phase + ": the reader returned no data set" );
        return nullptr;
    }

    auto* array = data_set->GetPointData()->GetArray( name );
    if ( !array )
    {
        diagnostics.errors.push_back( phase + ": point-data array " + name + " was not loaded" );
    }
    return array;
}

bool CheckNetcdfReaderError( vtkNetCDFCFReader* reader, const std::string& phase,
                             NetcdfDiagnostics& diagnostics )
{
    const unsigned long error_code = reader->GetErrorCode();
    if ( error_code == vtkErrorCode::NoError )
    {
        return true;
    }

    std::ostringstream message;
    message << phase << ": VTK error code " << error_code << " ("
            << vtkErrorCode::GetStringFromErrorCode( error_code ) << ")";
    diagnostics.errors.push_back( message.str() );
    return false;
}

void ThrowNetcdfDiagnostics( const NetcdfDiagnostics& diagnostics )
{
    for ( const auto& warning : diagnostics.warnings )
    {
        kvsMessageWarning( warning.c_str() );
    }

    if ( !diagnostics.errors.empty() )
    {
        std::ostringstream message;
        message << "Failed to read NetCDF";
        for ( const auto& error : diagnostics.errors )
        {
            message << "\n  " << error;
        }
        throw std::runtime_error( message.str() );
    }
}

class GearnNetcdfFormatAdapter : public cvt::NetcdfFormatAdapter
{
public:
    const char* name() const override { return "GEARN"; }
    cvt::NetcdfGridType gridType() const override
    {
        return cvt::NetcdfGridType::UnstructuredGrid;
    }

    bool matches( const cvt::NetcdfMetadata& metadata ) const override
    {
        return metadata.hasVariable( "XDIS", "(NCX)" ) &&
               metadata.hasVariable( "YDIS", "(NCY)" ) &&
               metadata.hasVariable( "ZZT", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "U", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "V", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "W", "(NCZ, NCY, NCX)" ) &&
               metadata.hasVariable( "CONC_Cs137", "(NCZ, NCY, NCX)" );
    }

    std::shared_ptr<kvs::FileFormatBase> read( const std::string& filename ) const override
    {
        NetcdfDiagnostics diagnostics;
        vtkNew<vtkNetCDFCFReader> x_reader;
        vtkNew<vtkNetCDFCFReader> y_reader;
        vtkNew<vtkNetCDFCFReader> data_reader;
        vtkNew<vtkCallbackCommand> callback;

        ObserveNetcdfReader( x_reader, callback, diagnostics );
        ObserveNetcdfReader( y_reader, callback, diagnostics );
        ObserveNetcdfReader( data_reader, callback, diagnostics );

        x_reader->SetFileName( filename.c_str() );
        y_reader->SetFileName( filename.c_str() );
        data_reader->SetFileName( filename.c_str() );
        data_reader->SetOutputTypeToUnstructured();

        if ( !SelectNetcdfVariables( x_reader, "(NCX)", { "XDIS" }, "XDIS reader",
                                     diagnostics ) ||
             !SelectNetcdfVariables( y_reader, "(NCY)", { "YDIS" }, "YDIS reader",
                                     diagnostics ) ||
             !SelectNetcdfVariables(
                 data_reader, "(NCZ, NCY, NCX)", { "ZZT", "U", "V", "W", "CONC_Cs137" },
                 "3D data reader", diagnostics ) )
        {
            ThrowNetcdfDiagnostics( diagnostics );
        }

        diagnostics.phase = "XDIS reader Update";
        x_reader->Update();
        diagnostics.phase = "YDIS reader Update";
        y_reader->Update();
        diagnostics.phase = "3D data reader Update";
        data_reader->Update();

        CheckNetcdfReaderError( x_reader, "XDIS reader", diagnostics );
        CheckNetcdfReaderError( y_reader, "YDIS reader", diagnostics );
        CheckNetcdfReaderError( data_reader, "3D data reader", diagnostics );

        auto* x_data_set = vtkDataSet::SafeDownCast( x_reader->GetOutputDataObject( 0 ) );
        auto* y_data_set = vtkDataSet::SafeDownCast( y_reader->GetOutputDataObject( 0 ) );
        auto* data_grid =
            vtkUnstructuredGrid::SafeDownCast( data_reader->GetOutputDataObject( 0 ) );
        auto* xdis = GetRequiredNetcdfArray( x_data_set, "XDIS", "XDIS reader", diagnostics );
        auto* ydis = GetRequiredNetcdfArray( y_data_set, "YDIS", "YDIS reader", diagnostics );
        auto* zzt = GetRequiredNetcdfArray( data_grid, "ZZT", "3D data reader", diagnostics );
        auto* u = GetRequiredNetcdfArray( data_grid, "U", "3D data reader", diagnostics );
        auto* v = GetRequiredNetcdfArray( data_grid, "V", "3D data reader", diagnostics );
        auto* w = GetRequiredNetcdfArray( data_grid, "W", "3D data reader", diagnostics );
        auto* cs137 =
            GetRequiredNetcdfArray( data_grid, "CONC_Cs137", "3D data reader", diagnostics );
        ThrowNetcdfDiagnostics( diagnostics );

        const vtkIdType x_size = xdis->GetNumberOfTuples();
        const vtkIdType y_size = ydis->GetNumberOfTuples();
        const vtkIdType number_of_points = data_grid->GetNumberOfPoints();
        if ( x_size < 2 || y_size < 2 || number_of_points <= 0 ||
             x_size > std::numeric_limits<vtkIdType>::max() / y_size ||
             number_of_points % ( x_size * y_size ) != 0 )
        {
            throw std::runtime_error(
                "GEARN NetCDF coordinate assembly: NCX, NCY, and 3D point counts are "
                "inconsistent" );
        }

        NetcdfDimensions dimensions;
        dimensions.x = x_size;
        dimensions.y = y_size;
        dimensions.z = number_of_points / ( x_size * y_size );
        if ( dimensions.z < 2 )
        {
            throw std::runtime_error( "GEARN NetCDF coordinate assembly: NCZ must be at least 2" );
        }

        const vtkIdType number_of_cells =
            ( dimensions.x - 1 ) * ( dimensions.y - 1 ) * ( dimensions.z - 1 );
        if ( data_grid->GetNumberOfCells() != number_of_cells ||
             zzt->GetNumberOfTuples() != number_of_points ||
             u->GetNumberOfTuples() != number_of_points ||
             v->GetNumberOfTuples() != number_of_points ||
             w->GetNumberOfTuples() != number_of_points ||
             cs137->GetNumberOfTuples() != number_of_points )
        {
            throw std::runtime_error(
                "GEARN NetCDF coordinate assembly: point-data or hexahedral-cell counts are "
                "inconsistent" );
        }

        vtkDataArray* value_arrays[] = { u, v, w, cs137 };
        const char* value_names[] = { "U", "V", "W", "CONC_Cs137" };
        for ( std::size_t i = 0; i < 4; ++i )
        {
            if ( value_arrays[i]->GetNumberOfComponents() != 1 )
            {
                throw std::runtime_error( std::string( "GEARN NetCDF point-data array " ) +
                                          value_names[i] + " must have one component" );
            }
        }
        for ( vtkIdType i = 0; i < number_of_cells; ++i )
        {
            if ( data_grid->GetCellType( i ) != VTK_HEXAHEDRON )
            {
                throw std::runtime_error(
                    "GEARN NetCDF grid contains a non-hexahedral VTK cell" );
            }
        }
        if ( static_cast<unsigned long long>( number_of_points - 1 ) >
             std::numeric_limits<kvs::UInt32>::max() )
        {
            throw std::runtime_error(
                "GEARN NetCDF node count exceeds the KVS UInt32 connection limit" );
        }

        vtkNew<vtkFloatArray> point_array;
        point_array->SetNumberOfComponents( 3 );
        point_array->SetNumberOfTuples( number_of_points );
        float* point_values = point_array->GetPointer( 0 );

#pragma omp parallel for
        for ( vtkIdType k = 0; k < dimensions.z; ++k )
        {
            for ( vtkIdType j = 0; j < dimensions.y; ++j )
            {
                for ( vtkIdType i = 0; i < dimensions.x; ++i )
                {
                    const vtkIdType id =
                        i + j * dimensions.x + k * dimensions.x * dimensions.y;
                    point_values[id * 3] = static_cast<float>( xdis->GetComponent( i, 0 ) );
                    point_values[id * 3 + 1] =
                        static_cast<float>( ydis->GetComponent( j, 0 ) );
                    point_values[id * 3 + 2] =
                        static_cast<float>( zzt->GetComponent( id, 0 ) );
                }
            }
        }

        vtkNew<vtkPoints> points;
        points->SetData( point_array );
        vtkSmartPointer<vtkUnstructuredGrid> normalized =
            vtkSmartPointer<vtkUnstructuredGrid>::New();
        normalized->ShallowCopy( data_grid );
        normalized->SetPoints( points );
        normalized->GetPointData()->Initialize();
        normalized->GetPointData()->AddArray( u );
        normalized->GetPointData()->AddArray( v );
        normalized->GetPointData()->AddArray( w );
        normalized->GetPointData()->AddArray( cs137 );

        return std::make_shared<cvt::VtkXmlUnstructuredGrid>( normalized.GetPointer() );
    }

    bool supportsTimeSeries() const override { return true; }

    bool extractTimeKey( const std::string& filename, const cvt::NetcdfMetadata&,
                         std::string& time_key, std::string& error ) const override
    {
        const cvt::filesystem::path input_path( filename );
        if ( input_path.extension() != ".nc" )
        {
            error = "GEARN NetCDF filename must have the .nc extension: " + filename;
            return false;
        }

        const std::string stem = input_path.filename().stem().string();
        const auto separator = stem.find_last_of( '_' );
        if ( separator == std::string::npos )
        {
            error = "GEARN NetCDF filename must end with _yyyymmddhhmmss.nc: " + filename;
            return false;
        }

        time_key = stem.substr( separator + 1 );
        if ( time_key.size() != 14 ||
             !std::all_of( time_key.begin(), time_key.end(),
                           []( unsigned char c ) { return std::isdigit( c ) != 0; } ) )
        {
            error = "GEARN NetCDF filename must end with _yyyymmddhhmmss.nc: " + filename;
            return false;
        }
        return true;
    }
};

const std::vector<std::shared_ptr<cvt::NetcdfFormatAdapter>>& RegisteredNetcdfAdapters()
{
    static const std::vector<std::shared_ptr<cvt::NetcdfFormatAdapter>> adapters = {
        std::make_shared<GearnNetcdfFormatAdapter>()
    };
    return adapters;
}

bool MatchesNetcdfGridType( const std::shared_ptr<kvs::FileFormatBase>& format,
                            cvt::NetcdfGridType grid_type )
{
    switch ( grid_type )
    {
    case cvt::NetcdfGridType::ImageData:
        return dynamic_cast<cvt::VtkXmlImageData*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::RectilinearGrid:
        return dynamic_cast<cvt::VtkXmlRectilinearGrid*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::StructuredGrid:
        return dynamic_cast<cvt::VtkXmlStructuredGrid*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::UnstructuredGrid:
        return dynamic_cast<cvt::VtkXmlUnstructuredGrid*>( format.get() ) != nullptr;
    case cvt::NetcdfGridType::Unknown:
    default:
        return false;
    }
}
} // namespace detail
} // namespace cvt

bool cvt::NetcdfMetadata::hasVariable( const std::string& name,
                                       const std::string& dimensions ) const
{
    const auto found = m_variable_dimensions.find( name );
    return found != m_variable_dimensions.end() && found->second == dimensions;
}

bool cvt::Netcdf::ReadMetadata( const std::string& filename, cvt::NetcdfMetadata& metadata )
{
    try
    {
        cvt::detail::NetcdfDiagnostics diagnostics;
        diagnostics.phase = "NetCDF format detection";
        vtkNew<vtkNetCDFCFReader> reader;
        vtkNew<vtkCallbackCommand> callback;
        cvt::detail::ObserveNetcdfReader( reader, callback, diagnostics );
        reader->SetFileName( filename.c_str() );
        if ( reader->UpdateMetaData() == 0 )
        {
            diagnostics.errors.push_back(
                diagnostics.phase + ": failed to read NetCDF metadata from " + filename );
        }
        cvt::detail::CheckNetcdfReaderError( reader, diagnostics.phase, diagnostics );
        cvt::detail::ThrowNetcdfDiagnostics( diagnostics );

        vtkStringArray* dimensions = reader->GetVariableDimensions();
        metadata.m_variable_dimensions.clear();
        for ( int i = 0; i < reader->GetNumberOfVariableArrays(); ++i )
        {
            metadata.m_variable_dimensions.emplace( reader->GetVariableArrayName( i ),
                                                    dimensions->GetValue( i ) );
        }
        return true;
    }
    catch ( const std::exception& e )
    {
        kvsMessageError( e.what() );
        return false;
    }
}

const cvt::NetcdfFormatAdapter* cvt::Netcdf::SelectAdapter(
    const cvt::NetcdfMetadata& metadata )
{
    std::vector<const cvt::NetcdfFormatAdapter*> matches;
    for ( const auto& adapter : cvt::detail::RegisteredNetcdfAdapters() )
    {
        if ( adapter->matches( metadata ) )
        {
            matches.push_back( adapter.get() );
        }
    }

    if ( matches.empty() )
    {
        std::ostringstream message;
        message << "Unsupported NetCDF data format; variables:";
        for ( const auto& variable : metadata.variableDimensions() )
        {
            message << " " << variable.first << variable.second;
        }
        kvsMessageError( message.str().c_str() );
        return nullptr;
    }
    if ( matches.size() > 1 )
    {
        std::ostringstream message;
        message << "Ambiguous NetCDF data format; matched adapters:";
        for ( const auto* adapter : matches )
        {
            message << " " << adapter->name();
        }
        kvsMessageError( message.str().c_str() );
        return nullptr;
    }
    return matches.front();
}

cvt::Netcdf::Netcdf( const std::string& filename ) { this->read( filename ); }

bool cvt::Netcdf::read( const std::string& filename )
{
    this->setFilename( filename );
    this->setSuccess( false );
    m_format.reset();
    m_format_name.clear();
    m_grid_type = cvt::NetcdfGridType::Unknown;

    cvt::NetcdfMetadata metadata;
    if ( !ReadMetadata( filename, metadata ) )
    {
        return false;
    }

    const auto* adapter = SelectAdapter( metadata );
    if ( !adapter )
    {
        return false;
    }

    try
    {
        m_format = adapter->read( filename );
        if ( !m_format ||
             !cvt::detail::MatchesNetcdfGridType( m_format, adapter->gridType() ) )
        {
            kvsMessageError( ( std::string( adapter->name() ) +
                               " NetCDF adapter returned an invalid VTK grid type for " +
                               filename )
                                 .c_str() );
            m_format.reset();
            return false;
        }
        m_format_name = adapter->name();
        m_grid_type = adapter->gridType();
        this->setSuccess( true );
        return true;
    }
    catch ( const std::exception& e )
    {
        kvsMessageError( ( std::string( adapter->name() ) + " NetCDF: " + e.what() ).c_str() );
        return false;
    }
    catch ( ... )
    {
        kvsMessageError(
            ( std::string( "Unknown error while reading " ) + adapter->name() + " NetCDF" )
                .c_str() );
        return false;
    }
}

bool cvt::Netcdf::write( const std::string& filename )
{
    this->setFilename( filename );
    this->setSuccess( false );
    kvsMessageError( "Writing NetCDF has not been implemented" );
    return false;
}

bool cvt::Netcdf::Probe( const std::string& filename, cvt::NetcdfFileInfo& info,
                         bool require_time_key )
{
    cvt::NetcdfMetadata metadata;
    if ( !ReadMetadata( filename, metadata ) )
    {
        return false;
    }

    const auto* adapter = SelectAdapter( metadata );
    if ( !adapter )
    {
        return false;
    }

    info.format_name = adapter->name();
    info.grid_type = adapter->gridType();
    info.supports_time_series = adapter->supportsTimeSeries();
    info.time_key.clear();

    if ( require_time_key )
    {
        if ( !info.supports_time_series )
        {
            kvsMessageError(
                ( info.format_name + " NetCDF does not support wildcard time series" ).c_str() );
            return false;
        }
        std::string error;
        if ( !adapter->extractTimeKey( filename, metadata, info.time_key, error ) )
        {
            kvsMessageError( error.c_str() );
            return false;
        }
    }
    return true;
}
