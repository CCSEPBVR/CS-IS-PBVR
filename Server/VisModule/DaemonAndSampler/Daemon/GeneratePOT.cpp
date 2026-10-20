#include "GeneratePOT.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vismodule/GenerateParticle>
#include <vismodule/PlotOverTimeGenerator>
#include <vismodule/ImporterBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#ifndef CPU_VER
#include <mpi.h>
#endif

#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>

namespace
{

std::string EnvValueOrUnsetIS( const char* name )
{
    const char* value = std::getenv( name );
    return value ? std::string( value ) : std::string( "(unset)" );
}

void PrintMissingParameterFileWarning(
    const std::string& parameter_name,
    const std::string& file_name,
    const std::string& default_parameter_message
)
{
    std::cout << "================================================================" << std::endl;
    std::cout << "[WARN] " << parameter_name << " does not exist." << std::endl;
    std::cout << "[WARN] File: " << file_name << std::endl;
    std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnsetIS( "VIS_PARAM_DIR" ) << std::endl;
    std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnsetIS( "PARTICLE_DIR" ) << std::endl;
    std::cout << "[INFO] " << default_parameter_message << std::endl;
    std::cout << "================================================================" << std::endl;
}

} // namespace

void SetDefaultPOTParameter( PlotOverTimeProperty& pot_property )
{
    pot_property.m_plot_flag = false;
    pot_property.m_target_point[0] = 0;
    pot_property.m_target_point[1] = 0;
    pot_property.m_target_point[2] = 0;
}

void SetDefaultPOTParameterIS( PlotOverTimeProperty& pot_property )
{
    const char *envBuf = NULL;
    std::string visParamDir;
    std::string plotOverTimeParameterPath_old;
    ParameterFileReader ppr;

    envBuf = std::getenv( "VIS_PARAM_DIR" );

    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }

    plotOverTimeParameterPath_old =  visParamDir;
    plotOverTimeParameterPath_old += "parameter_old.pot";

    std::ifstream plotOverTimeParameterFileOld( plotOverTimeParameterPath_old );
    if ( !plotOverTimeParameterFileOld.good() )
    {
        PrintMissingParameterFileWarning(
            "Plot over time parameter file",
            "parameter_old.pot",
            "Set default plot over time parameters."
        );
        SetDefaultPOTParameter( pot_property );
        return;
    }

    ppr.readPlotOverTimeParameterFile( plotOverTimeParameterPath_old.c_str() );
    ppr.setPlotOverTimeParameter( pot_property );
}

bool GeneratePOTIS(
    const int time_step,
    const PlotOverTimeProperty& pot_property,
    std::unique_ptr<vismodule::KVSMLObjectPlotOverTime>& kvsml_object_pot
)
{
    ParticleMonitor pm;
    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_pot( time_step );
    }
    else
    {
        // pm.setTimeStep_pot(0);
        std::cerr << __FILE__ << "," << __func__ << "," << __LINE__ << "ERROR: Time step is not exist." << std::endl;
        return false;
    }
    
    // get plot over line
    bool result = false;
    result = pm.readPlotOverTimeFile();
    if ( !result ) return false; // データファイルが存在しないタイムステップはスキップ
    pm.getPlotOverTime( kvsml_object_pot.get() );

    return true;
}

POTSample GeneratePOTCS(
    const std::string& file_path, const int time_step,
    const PlotOverTimeProperty& pot_property, MultiVolumePropertyList& mvpl )
{
    int rank = 0, size = 1;
    PlotOverTimeProperty observation = pot_property;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
#endif
    const int nvalues = mvpl.m_total_number_ingredients;
    const int no_volume = std::numeric_limits<int>::max();
    int selected_volume = no_volume;
    int failed = ( nvalues <= 0 || mvpl.m_total_number_subvolumes <= 0 ) ? 1 : 0;
    for ( float coordinate : observation.m_target_point )
        if ( !std::isfinite( coordinate ) ) failed = 1;
    POTSample result;
    result.values.assign( std::max( 0, nvalues ), std::numeric_limits<float>::quiet_NaN() );

    // Stable global subvolume order, independent of MPI completion order.
    // All ranks (including rank zero) participate in reading/interpolation.
    for ( int vl = rank; vl < mvpl.m_total_number_subvolumes; vl += size )
    {
        vismodule::VolumeObjectBase* raw_volume = nullptr;
        try
        {
            int local_volume = 0;
            const int file_index = mvpl.getFileIndex( vl, &local_volume );
            if ( file_index < 0 ) throw std::runtime_error( "Invalid POT subvolume" );
            auto& mvp = mvpl.m_list[file_index];
            // These formats have no implemented reader in GeneratePOLCS either.
            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 )
                throw std::runtime_error( "POT does not support gathered volumes" );
            std::string path = file_path;
            mvp.setFilePath( path, time_step, local_volume );
            if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
            {
#ifdef EXTEND_FILE_FORMAT
                generate_volume( path, mvp, time_step, local_volume, raw_volume, true );
#else
                throw std::runtime_error( "POT requires EXTEND_FILE_FORMAT for this volume" );
#endif
            }
            else
            {
                generate_volume( path, mvp, raw_volume, true );
            }
            std::unique_ptr<vismodule::VolumeObjectBase> volume( raw_volume );
            raw_volume = nullptr;
            if ( !volume ) throw std::runtime_error( "Failed to read POT volume" );
            const auto* importer = dynamic_cast<const vismodule::ImporterBase*>( volume.get() );
            if ( importer && importer->isFailure() )
                throw std::runtime_error( "Failed to import POT volume" );
            if ( volume->veclen() != static_cast<size_t>( nvalues ) || volume->nnodes() == 0 ||
                 volume->values().size() != volume->nnodes() * volume->veclen() )
                throw std::runtime_error( "Invalid POT volume values" );

            std::unique_ptr<std::unique_ptr<Type[]>[]> values;
            int nvariables = 0, ncoords = 0;
            std::vector<float> sample( nvalues );
            PlotOverTimeGenerator generator;
            bool valid = false;
            if ( volume->volumeType() == vismodule::VolumeObjectBase::Unstructured )
            {
                const auto* unstructured = static_cast<const vismodule::UnstructuredVolumeObject*>( volume.get() );
                const auto type = unstructured->cellType();
                int nodes_per_cell = 0;
                switch ( type )
                {
                case vismodule::VolumeObjectBase::Tetrahedra: nodes_per_cell = 4; break;
                case vismodule::VolumeObjectBase::Hexahedra: nodes_per_cell = 8; break;
                case vismodule::VolumeObjectBase::Pyramid: nodes_per_cell = 5; break;
                case vismodule::VolumeObjectBase::Prism: nodes_per_cell = 6; break;
                default: throw std::runtime_error( "Unsupported POT cell type" );
                }
                if ( unstructured->coords().size() != volume->nnodes() * 3 ||
                     unstructured->connections().size() != unstructured->ncells() * nodes_per_cell )
                    throw std::runtime_error( "Invalid POT mesh arrays" );
                for ( size_t i = 0; i < unstructured->coords().size(); ++i )
                    if ( !std::isfinite( unstructured->coords()[i] ) )
                        throw std::runtime_error( "Invalid POT mesh coordinate" );
                for ( size_t i = 0; i < unstructured->connections().size(); ++i )
                    if ( unstructured->connections()[i] >= volume->nnodes() )
                        throw std::runtime_error( "Invalid POT node index" );
                domain_parameters_unstruct dom;
                std::unique_ptr<float[]> coordinates;
                std::unique_ptr<unsigned int[]> connections;
                int ncells = 0;
                vismodule::VolumeObjectBase::CellType celltype;
                store_volume_in_variables_array_unstruct( volume.get(), dom, values, nvariables,
                    coordinates, ncoords, connections, ncells, celltype );
                std::vector<Type*> pointers( nvariables );
                for ( int i = 0; i < nvariables; ++i ) pointers[i] = values[i].get();
                valid = generator.GeneratePOTUnstruct( observation, pointers.data(), nvariables,
                    coordinates.get(), ncoords, connections.get(), ncells, celltype, sample );
            }
            else if ( volume->volumeType() == vismodule::VolumeObjectBase::Structured )
            {
                domain_parameters_struct dom;
                std::array<int, 3> resolution;
                store_volume_in_variables_array_struct( volume.get(), dom, resolution, values, nvariables, ncoords );
                if ( resolution[0] < 2 || resolution[1] < 2 || resolution[2] < 2 ||
                     static_cast<size_t>(resolution[0]) * resolution[1] * resolution[2] != volume->nnodes() )
                    throw std::runtime_error( "Invalid POT structured resolution" );
                std::vector<Type*> pointers( nvariables );
                for ( int i = 0; i < nvariables; ++i ) pointers[i] = values[i].get();
                valid = generator.GeneratePOTStruct( observation, dom, pointers.data(), nvariables, sample );
            }
            else throw std::runtime_error( "Unsupported POT volume type" );

            if ( valid )
            {
                if ( !std::all_of( sample.begin(), sample.end(), []( float v ) { return std::isfinite( v ); } ) )
                    throw std::runtime_error( "Non-finite POT interpolation" );
                if ( vl < selected_volume )
                {
                    selected_volume = vl;
                    result.values = std::move( sample );
                }
            }
        }
        catch ( const std::exception& e )
        {
            delete raw_volume;
            failed = 1;
            std::cerr << "POT step " << time_step << ", subvolume " << vl << ": " << e.what() << std::endl;
        }
    }
#ifndef CPU_VER
    int global_failed = 0, global_volume = no_volume;
    MPI_Allreduce( &failed, &global_failed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD );
    MPI_Allreduce( &selected_volume, &global_volume, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD );
    failed = global_failed;
    selected_volume = global_volume;
    if ( !failed && selected_volume != no_volume )
        MPI_Bcast( result.values.data(), nvalues, MPI_FLOAT, selected_volume % size, MPI_COMM_WORLD );
#endif
    result.success = !failed;
    result.mask = result.success && selected_volume != no_volume;
    if ( !result.mask )
        std::fill( result.values.begin(), result.values.end(), std::numeric_limits<float>::quiet_NaN() );
    return result;
}
