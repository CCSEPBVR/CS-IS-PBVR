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
            ReceiveInitialStepSignal( buf, volume_data_file_path, transfer_function_file_path );
            mvpl.loadVolumeDataFile( volume_data_file_path );
            SetDefaultParticleParameterCS( transfer_function_file_path, mvpl, particle_property );
            InitialStepCS( volume_data_file_path, mvpl.m_total_start_steps, particle_property, mvpl );

            // 成分数3以上の時Glyphのデフォルトパラメータを設定する
            bool is_glyph_enabled = mvpl.m_total_number_ingredients >= 3;
            glyph_property.m_glyph_flag = is_glyph_enabled;
            SetDefaultGlyphParameterCS( glyph_property );

            // POLのデフォルトパラメータを設定する
            SetDefaultPOLParameterCS( pol_property );
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

