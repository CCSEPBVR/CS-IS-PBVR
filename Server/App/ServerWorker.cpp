#include "ServerWorker.h"

#include <memory>

#ifndef CPU_VER
#include "mpi.h"
#endif

#include "TaskSignal.h"
#include "TaskSignalTransferProtocol.h"

#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/Calculate>
#include <vismodule/InitialStep>
#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/GeneratePOL>

#include <kvs/PointObject>

void ServerWorker::Run()
{
    TaskSignal task_signal = TaskSignal::WAITING;
    MultiVolumePropertyList mvpl;
    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;

    particle_property.m_camera = new vismodule::Camera();
    particle_property.m_camera->setWindowSize(620, 620); // FIXME:クライアント側から送信されるようになったら削除
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();

    while ( task_signal != TaskSignal::EXIT )
    {
        int size = 0;
        int time_step = 0;
        char *buf;
        std::string volume_data_file_path;
        std::string transfer_function_file_path;
        std::string cam_connectivity_file_path;
        std::vector<std::string> slac_mode_file_paths;
        std::vector<std::string> primary_step_file_paths;
        std::unique_ptr<kvs::PointObject> pointObject;
        std::unique_ptr<kvs::PolygonObject> polygonObject;
        std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> kvsml_object_pol;

#ifndef CPU_VER
        MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

        if ( size <= 0 )
        {
            std::cout << "ERROR: Failed to receive message from Master" << std::endl;
            task_signal = TaskSignal::EXIT;
        }

        buf = new char[size];

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

        vismodule::Serializer::read( buf, &task_signal );

        switch ( task_signal )
        {
        case TaskSignal::EXIT:
            break;
        case TaskSignal::WAITING:
            std::cout << "WARN: WAITING signal is received" << std::endl;
            break;
        case TaskSignal::INITIAL_STEP:
        {
            ReceiveInitialStepSignal( buf, volume_data_file_path, transfer_function_file_path,
                                      cam_connectivity_file_path, slac_mode_file_paths,
                                      primary_step_file_paths );
            MultiVolumePropertyList candidate;
            int local_load_success = 1;
            try
            {
                candidate.loadVolumeDataFile( volume_data_file_path,
                                              cam_connectivity_file_path,
                                              slac_mode_file_paths );
                if ( candidate.m_list.empty() )
                    throw std::runtime_error( "resolved dataset produced no volume" );
                auto& property = candidate.m_list.front();
                if ( property.m_time_step_file_paths.size() != primary_step_file_paths.size() )
                    throw std::runtime_error( "resolved PBVR step mapping differs on worker" );
                property.m_time_step_file_paths = primary_step_file_paths;
#ifdef EXTEND_FILE_FORMAT
                if ( !property.m_netcdf_dataset.steps.empty() &&
                     property.m_netcdf_dataset.steps.size() != primary_step_file_paths.size() )
                    throw std::runtime_error( "resolved NetCDF step count differs on worker" );
                for ( std::size_t i = 0; i < property.m_netcdf_dataset.steps.size(); ++i )
                    property.m_netcdf_dataset.steps[i].primary_path =
                        primary_step_file_paths[i];
#endif
            }
            catch ( const std::exception& error )
            {
                local_load_success = 0;
                std::cerr << "ERROR: Worker dataset load failed: " << error.what()
                          << std::endl;
            }
#ifndef CPU_VER
            int all_ranks_loaded = 0;
            MPI_Allreduce( &local_load_success, &all_ranks_loaded, 1, MPI_INT, MPI_MIN,
                           MPI_COMM_WORLD );
            if ( !all_ranks_loaded ) break;
#else
            if ( !local_load_success ) break;
#endif

            int local_success = 1;
            ParticleProperty candidate_particle_property = particle_property;
            GlyphProperty candidate_glyph_property = glyph_property;
            PlotOverLineProperty candidate_pol_property = pol_property;
            try
            {
                SetDefaultParticleParameterCS(
                    transfer_function_file_path, candidate, candidate_particle_property );
                InitialStepCS( volume_data_file_path, candidate.m_total_start_steps,
                               candidate_particle_property, candidate );

                bool is_glyph_enabled = candidate.m_total_number_ingredients >= 3;
                candidate_glyph_property.m_glyph_flag = is_glyph_enabled;
                SetDefaultGlyphParameterCS( candidate_glyph_property );
                SetDefaultPOLParameterCS( candidate_pol_property );
            }
            catch ( const std::exception& error )
            {
                local_success = 0;
                std::cerr << "ERROR: Worker initialization failed: " << error.what()
                          << std::endl;
            }
#ifndef CPU_VER
            int all_ranks_success = 0;
            MPI_Allreduce( &local_success, &all_ranks_success, 1, MPI_INT, MPI_MIN,
                           MPI_COMM_WORLD );
            if ( !all_ranks_success ) break;
#else
            if ( !local_success ) break;
#endif
            std::swap( mvpl, candidate );
            particle_property = candidate_particle_property;
            glyph_property = candidate_glyph_property;
            pol_property = candidate_pol_property;
            break;
        }
        case TaskSignal::GENERATE_PARTICLE:
            pointObject = std::make_unique<kvs::PointObject>();
            ReceiveGenerateObjectSignal( buf, volume_data_file_path, time_step );
            GenerateParticleCS( volume_data_file_path, time_step, particle_property, mvpl, pointObject );
            break;
        case TaskSignal::GENERATE_GLYPH:
            ReceiveGenerateObjectSignal( buf, volume_data_file_path, time_step );
            Calculate_minmax_glyph( time_step, glyph_property, mvpl );
            polygonObject = GenerateGlyphCS( volume_data_file_path, time_step, glyph_property, mvpl );
            break;
        case TaskSignal::GENERATE_PLOT_OVER_LINE:
            ReceiveGenerateObjectSignal( buf, volume_data_file_path, time_step );
            kvsml_object_pol = GeneratePOLCS( volume_data_file_path, time_step, pol_property, mvpl );
            break;
        case TaskSignal::UPDATE_PARTICLE_PROPERTY:
        {
            particle_property.unpack( buf );
            particle_property.UpdateTransferFunctionSynthesizer();

            // 粒子パラメータの再計算
            particle_property.m_sampling_step  = CalculateSamplingStep( mvpl ) / particle_property.m_extra_opacity_factor;
            particle_property.m_subpixel_level = CalculateSubpixelLevel( particle_property, mvpl, *(particle_property.m_camera) );
            break;
        }
        case TaskSignal::UPDATE_GLYPH_PROPERTY:
            glyph_property.unpack( buf );
            break;
        case TaskSignal::UPDATE_PLOT_OVER_LINE_PROPERTY:
            pol_property.unpack( buf );
            break;
        default:
            std::cout << "ERROR: Unknown task signal" << std::endl;
            task_signal = TaskSignal::EXIT;
        }

        delete[] buf;
    }
}
