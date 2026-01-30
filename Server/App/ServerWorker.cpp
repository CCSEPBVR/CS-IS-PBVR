#include "ServerWorker.h"

ServerWorker::ServerWorker()
{
    Run();
}

ServerWorker::Run()
{
    TaskSignal task_signal = TaskSignal::WAITING;
    MultiVolumePropertyList mvpl;
    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;

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
    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

        vismodule::Serializer::read( 0, &task_signal );

        switch ( task_signal )
        {
        case TaskSignal::EXIT:
            break;
        case TaskSignal::WAITING:
            std::cout << "WARN: WAITING signal is received" << std::endl;
            break;
        case TaskSignal::INITIAL_STEP:
            ReceiveInitialStepSignal( buf, volume_data_file_path, transfer_function_file_path );
            SetDefaultParticleParameterCS( volume_data_file_path, transfer_function_file_path, particle_property, mvpl );
            InitialStepCS( volume_data_file_path, mvpl.m_total_start_steps, particle_property, mvpl );

            // 成分数3以上の時Glyphのデフォルトパラメータを設定する
            bool is_glyph_enabled = m_multi_volume_property_list->m_total_number_ingredients >= 3;
            glyph_property->m_glyph_flag = is_glyph_enabled;
            SetDefaultGlyphParameterCS( glyph_property );

            // POLのデフォルトパラメータを設定する
            SetDefaultPOLParameterCS( pol_property );
            break;
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
            particle_property.unpack( buf );
            particle_property.UpdateTransferFunctionSynthesizer();
            break;
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

void SendInitialStepSignal( const std::string& volume_data_file_path, const std::string& transfer_function_file_path )
{
    TaskSignal task_signal = TaskSignal::INITIAL_STEP;

    int32_t size = 0;
    size += sizeof( TaskSignal ); // task_signal
    size += sizeof( int64_t ); // volume_data_file_path.size()
    size += sizeof( char ) * ( volume_data_file_path.size() + 1 );
    size += sizeof( int64_t ); // transfer_function_file_path.size()
    size += sizeof( char ) * ( transfer_function_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    size_t index = 0;
    buf = new char[size];
    
    index += vismodule::Serializer::write( buf + index, task_signal );
    index += vismodule::Serializer::write( buf + index, volume_data_file_path.size() );
    index += vismodule::Serializer::writeArray( buf + index, volume_data_file_path.c_str(), volume_data_file_path.size() + 1 );
    index += vismodule::Serializer::write( buf + index, transfer_function_file_path.size() );
    index += vismodule::Serializer::writeArray( buf + index, transfer_function_file_path.c_str(), transfer_function_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete[] buf;
}

void SendGenerateParticleSignal( const std::string& volume_data_file_path, const int time_step )
{
    TaskSignal task_signal = TaskSignal::GENERATE_PARTICLE;
    SendGenerateObjectSignal( task_signal, volume_data_file_path, time_step );
}

void SendGenerateGlyphSignal( const std::string& volume_data_file_path, const int time_step )
{
    TaskSignal task_signal = TaskSignal::GENERATE_GLYPH;
    SendGenerateObjectSignal( task_signal, volume_data_file_path, time_step );
}

void SendGeneratePlorOverLineSignal( const std::string& volume_data_file_path, const int time_step )
{
    TaskSignal task_signal = TaskSignal::GENERATE_PLOT_OVER_LINE;
    SendGenerateObjectSignal( task_signal, volume_data_file_path, time_step );
}

void SendGenerateObjectSignal( const TaskSignal task_signal, const std::string& volume_data_file_path, const int time_step )
{
    int32_t size = 0;
    int time_step = 0;
    size += sizeof( task_signal ); // task_signal
    size += sizeof( int ); // time_step
    size += sizeof( int64_t ); // volume_data_file_path.size()
    size += sizeof( char ) * ( volume_data_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    size_t index = 0;
    buf = new char[size];
    
    index += vismodule::Serializer::write( buf + index, task_signal );
    index += vismodule::Serializer::write( buf + index, time_step );
    index += vismodule::Serializer::write( buf + index, volume_data_file_path.size() );
    index += vismodule::Serializer::writeArray( buf + index, volume_data_file_path.c_str(), volume_data_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete buf[]
}

void SendParticlePropertySignal( const ParticleProperty& particle_property )
{
    char* buf;
    int size = particle_property.byteSize();

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    buf = new char[size];
    particle_property.pack( buf );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
    delete[] buf;
}

void SendGlyphPropertySignal( const GlyphProperty& glyph_property )
{
    char* buf;
    int size = glyph_property.byteSize();

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    buf = new char[size];
    glyph_property.pack( buf );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete[] buf;
}

void SendPlotOverLinePropertySignal( const PlotOverLineProperty& pol_property )
{
    char* buf;
    int size = pol_property.byteSize();

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    buf = new char[size];
    pol_property.pack( buf );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete[] buf;
}

void ReceiveInitialStepSignal( const char* buf, std::string& volume_data_file_path, std::string& transfer_function_file_path )
{
    size_t index = 0;
    int64_t tmp_char_size;
    char* tmp_char = NULL;
    TaskSignal tmp_task_signal;

    index += vismodule::Serializer::read( buf + index, &tmp_task_signal );

    index += vismodule::Serializer::read( buf + index, &tmp_char_size );
    tmp_char = new char[tmp_char_size + 1];
    index += vismodule::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
    volume_data_file_path = std::string( tmp_char );
    delete[] tmp_char;

    index += vismodule::Serializer::read( buf + index, &tmp_char_size );
    tmp_char = new char[tmp_char_size + 1];
    index += vismodule::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
    transfer_function_file_path = std::string( tmp_char );
    delete[] tmp_char;
}

void ReceiveGenerateObjectSignal( const char* buf, std::string& volume_data_file_path, int& time_step )
{
    size_t index = 0;
    int64_t tmp_char_size;
    char* tmp_char = NULL;
    TaskSignal task_signal;

    index += vismodule::Serializer::read( buf + index, &task_signal );
    index += vismodule::Serializer::read( buf + index, &time_step );
    index += vismodule::Serializer::read( buf + index, &tmp_char_size );
    tmp_char = new char[tmp_char_size + 1];
    index += vismodule::Serializer::readArray( buf + index, tmp_char, tmp_char_size + 1 );
    volume_data_file_path = std::string( tmp_char );
    delete[] tmp_char;
}