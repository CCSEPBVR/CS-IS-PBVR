#include "TaskSignalTransferProtocol.h"

#ifndef CPU_VER
#include "mpi.h"
#endif

void SendInitialStepSignal( const std::string& volume_data_file_path, const std::string& transfer_function_file_path )
{
    TaskSignal task_signal = TaskSignal::INITIAL_STEP;

    int32_t size = 0;
    size += sizeof( int ); // task_signal
    size += sizeof( int64_t ); // volume_data_file_path.size()
    size += sizeof( char ) * ( volume_data_file_path.size() + 1 );
    size += sizeof( int64_t ); // transfer_function_file_path.size()
    size += sizeof( char ) * ( transfer_function_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    std::size_t index = 0;
    char* buf;
    buf = new char[size];
    
    index += vismodule::Serializer::write( buf + index, task_signal );
    // index += vismodule::Serializer::write( buf + index, task_signal_num );
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
    size += sizeof( task_signal ); // task_signal
    size += sizeof( int ); // time_step
    size += sizeof( int64_t ); // volume_data_file_path.size()
    size += sizeof( char ) * ( volume_data_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    std::size_t index = 0;
    char* buf;
    buf = new char[size];
    
    index += vismodule::Serializer::write( buf + index, task_signal );
    index += vismodule::Serializer::write( buf + index, time_step );
    index += vismodule::Serializer::write( buf + index, volume_data_file_path.size() );
    index += vismodule::Serializer::writeArray( buf + index, volume_data_file_path.c_str(), volume_data_file_path.size() + 1 );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete[] buf;
}

void SendParticlePropertySignal( const ParticleProperty& particle_property )
{
    int size = particle_property.byteSize();

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    char* buf;
    buf = new char[size];
    particle_property.pack( buf );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
    delete[] buf;
}

void SendGlyphPropertySignal( const GlyphProperty& glyph_property )
{
    int size = glyph_property.byteSize();

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    char* buf;
    buf = new char[size];
    glyph_property.pack( buf );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete[] buf;
}

void SendPlotOverLinePropertySignal( const PlotOverLineProperty& pol_property )
{
    int size = pol_property.byteSize();

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    char* buf;
    buf = new char[size];
    pol_property.pack( buf );

#ifndef CPU_VER
    MPI_Bcast( buf, size, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif

    delete[] buf;
}

void ReceiveInitialStepSignal( const char* buf, std::string& volume_data_file_path, std::string& transfer_function_file_path )
{
    std::size_t index = 0;
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
    std::size_t index = 0;
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
