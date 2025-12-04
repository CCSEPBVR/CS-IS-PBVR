/*****************************************************************************/
/**
 *  main.cpp
 *  CPU Server Program without CUDA, without VIS_MODULE, without OpenGL.
 */
/*****************************************************************************/

#include <vismodule/Connect>
#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/CS_PointObjectGenerator>

#include <vismodule/Argument>

#include <vismodule/AVSField>
#include <vismodule/Timer>
#include <vismodule/KVSMLObjectPointWriter>
#include <vismodule/JobDispatcher>
#ifndef CPU_VER
#include <vismodule/JobCollector>
#endif
#include <vismodule/MultiVolumeProperty>
#include <vismodule/TransferFunctionProperty>
#ifndef CPU_VER
#include "mpi.h"
#endif

#include <cassert>
#include <signal.h> /* 140319 for client stop by Ctrl+c */
#include <sys/stat.h>
#if (defined(VIS_MODULE_PLATFORM_LINUX) || defined(VIS_MODULE_PLATFORM_MACOSX))
#include <execinfo.h>
#endif
#include <vismodule/File>

#include <vismodule/ExtendedTransferFunction>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "VariableRange.h"

#include <vismodule/timer_simple>

#include <vismodule/Compiler>
#ifdef VIS_MODULE_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

#include <vismodule/FileChecker>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/CellByCellParticleGenerator>

#include <vismodule/GlyphSeedGenerator>

//plot over line
#include <vismodule/PlotOverLineGenerator>

#include <vismodule/InitialStep>
#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/GeneratePOL>
#include <signal.h>

//IS
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>
#include <vismodule/ParameterFileReader>
#include <vismodule/SetDefaultTransferFunction>
#include <DaemonAndSampler/Daemon/Timer.h>

//using FuncParser::Variable;
//using FuncParser::Variables;
//using FuncParser::Function;
//using FuncParser::FunctionParser;

bool SigServer = false;
void SignalHandler( const int sig )
{
    SigServer = true;
}

/**
 * ???C??????:
 * @param argc
 * @param argv
 * @return
 */
void  Connect( int argc, char** argv )
{
    jpv::ServerMode server_mode;

    if ( argc < 2 )
    {
        std::cout << "Server Mode is CS" << std::endl;
        server_mode = jpv::ServerMode::CS;
    }
    else
    {
        if ( strcmp(argv[1], "CS") == 0 )
        {
            std::cout << "Server Mode is CS" << std::endl;
            server_mode = jpv::ServerMode::CS;
        }
        else if ( strcmp(argv[1], "IS") == 0 )
        {
            std::cout << "Server Mode is IS" << std::endl;
            server_mode = jpv::ServerMode::IS;
        }
    }

    std::string tfFilePath;
    std::string tfFilePath_old;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old;
    
    // Set paramter file path when in IS mode
    if ( server_mode == jpv::ServerMode::IS )
    {
        SetParameterFilePath(
            tfFilePath,
            tfFilePath_old,
            glyphParameterPath,
            glyphParameterPath_old,
            plotOverLineParameterPath,
            plotOverLineParameterPath_old
        );
    }

    Argument param( argc, argv );
    MultiVolumePropertyList mvpl;
    TransferFunctionSynthesizerCreator transfunc_creator;

    bool nan_error = false; // Add for NaN 2016.01.14

    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    // =================== 新規作成 ここから ===================
    JobDispatcher jd;
#ifndef CPU_VER
    JobCollector  jc( &jd );
#endif

    assert( jpv::ParticleTransferUtils::isLittleEndian() );

    int ptss;
    jpv::ParticleTransferServer pts;

    jpv::ParticleTransferServerMessage servMes;
    jpv::ParticleTransferClientMessage clntMes;
    
    char* buf;
    int bsz = 0;
    
    servMes.m_camera = new vismodule::Camera();
    clntMes.m_camera = new vismodule::Camera();

    if ( rank == 0 )
    {
        ptss = pts.initializeServer( param.m_port );
    }

    while ( true )
    {
        if ( rank == 0 )
        {
            if ( ptss == -1 )
            {
                std::cout << "ERROR:server initialize failed" << std::endl;
                break;
            }

            pts.acceptServer(); // wait for client connection

            if ( !pts.good() )
            {
                std::cout << "ERROR:server accept faild" << std::endl;
                break;
            }

            ptss = pts.recvMessage( &clntMes );

            if ( ptss == -1 )
            {
                std::cout << "ERROR:receive client message failed" << std::endl;
                break;
            }

            std::cout << "Rank " << rank << ": Recv Client Message" << std::endl;
            clntMes.show();

            if ( server_mode == jpv::ServerMode::CS )
            {
#ifdef _WIN32 
                std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(), '/', '\\');
#elif defined(_WIN64)
                std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(), '/', '\\');
#endif
#ifdef __APPLE__
                std::string target = "¥";
                std::string replacement = "/";

                size_t pos = 0;
                while ((pos = clntMes.m_input_directory.find(target, pos)) != std::string::npos) {
                    clntMes.m_input_directory.replace(pos, target.length(), replacement);
                    pos += replacement.length();
                }
#elif defined(__linux__)
                std::string target = "\\";
                std::string replacement = "/";

                size_t pos = 0;
                while ((pos = clntMes.m_input_directory.find(target, pos)) != std::string::npos) {
                    clntMes.m_input_directory.replace(pos, target.length(), replacement);
                    pos += replacement.length();
                }
#endif

                if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step )
                {
                    bool result = CheckFileFormat( clntMes, pts, param );

                    if( !result )
                    {
                        std::cout << "ERROR:file format check failed" << std::endl;
                        break;
                    }
                }

                if ( 
                    clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step      ||
                    clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_particle ||
                    clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_glyph    ||
                    clntMes.m_initialize_parameter == jpv::InitializeParameter::plot_over_line
                )
                {
                    // send cltMes to all worker process >>
                    bsz = clntMes.byteSize();
#ifndef CPU_VER
                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                    char* buf;
                    buf = new char[bsz];
                    clntMes.pack( buf );
#ifndef CPU_VER
                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                    delete[] buf;
                    // send cltMes to all worker process <<
                }
            } // server_mode == jpv::ServerMode::CS
        } // rank == 0
        else // rank > 0
        {
            if ( server_mode == jpv::ServerMode::CS )
            {
                // recv cltMes from process 0 start
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                if ( bsz < 0 )
                {
                    std::cout << "Rank " << rank << ": Worker is terminated" << std::endl;
                    break;
                }
                buf = new char[bsz];
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                clntMes.unpack( buf );
                delete[] buf;
                // recv cltMes from process 0 end

                std::cout << "Rank " << rank << ": Recv Client Message" << std::endl;
            } // server_mode == jpv::ServerMode::CS
            else // server_mode == jpv::ServerMode::IS
            {
                std::cout << "WARN:Daemon is single process" << std::endl;
                break;
            }
        }

        if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
        {
            std::cout << "ERROR : jpv::InitializeParameter::connection_reset" << std::endl;
            break;
        } // connection reset
        else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
        {
            std::cout << "ERROR : jpv::InitializeParameter::end" << std::endl;
            break;
        } // end
        else if ( 
            clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step ||
            clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_particle
        )
        {
            if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step )
            {
                std::cout << "==================== Initial Step Start ====================" << std::endl;
            }
            else
            {
                std::cout << "==================== Generate Particle Start ====================" << std::endl;
            }

            jpv::InitializeParameter init_param = clntMes.m_initialize_parameter;

            if ( server_mode == jpv::ServerMode::CS )
            {
                bool result = SetParticleParameterCS( clntMes, pts, param, mvpl );

                if( !result )
                {
                    std::cout << "ERROR:particle parameter setting failed" << std::endl;
                    break;
                }
            }
            else // server_mode == jpv::ServerMode::IS
            {
                param.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
                bool result = SetParticleParameterIS( clntMes, tfFilePath, tfFilePath_old, param, mvpl );

                if( !result )
                {
                    std::cout << "ERROR:particle parameter setting failed" << std::endl;
                    break;
                }
            }
            
#ifndef CPU_VER
            generate_particle( param, mvpl, nan_error, jc, jd, pts, server_mode, init_param );
#else
            generate_particle( param, mvpl, nan_error, jd, pts, server_mode, init_param );
#endif                           

            // CSではSetParticleParameterCS->TransferFunctionSynthesizerCreator::setProtocol()でメモリを確保している
            delete param.m_transfunc_synthesizer;

            if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step )
            {
                std::cout << "==================== Initial Step End ====================" << std::endl;
            }
            else
            {
                std::cout << "==================== Generate Particle End ====================" << std::endl;
            }
        } // generate particle
        else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_glyph )
        {
            std::cout << "==================== Generate Glyph Start ====================" << std::endl;

            if ( server_mode == jpv::ServerMode::CS )
            {
                bool result = SetGlyphParameterCS( clntMes, param, mvpl );

                if( !result )
                {
                    std::cout << "ERROR:glyph parameter setting failed" << std::endl;
                    break;
                }
            }
            else // server_mode == jpv::ServerMode::IS
            {
                bool result = SetGlyphParameterIS( clntMes, glyphParameterPath, glyphParameterPath_old, param );

                if( !result )
                {
                    std::cout << "ERROR:glyph parameter setting failed" << std::endl;
                    break;
                }
            }

#ifndef CPU_VER
            generate_glyph( param, mvpl, nan_error, jc, jd, pts, server_mode );
#else
            generate_glyph( param, mvpl, nan_error, jd, pts, server_mode );
#endif

            std::cout << "==================== Generate Glyph End ====================" << std::endl;
        } // generate glyph
        else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::plot_over_line )
        {
            std::cout << "==================== Plot Over Line Start ====================" << std::endl;

            if ( server_mode == jpv::ServerMode::CS )
            {
                bool result = SetPOLParameterCS( clntMes, param );

                if( !result )
                {
                    std::cout << "ERROR:plot over line parameter setting failed" << std::endl;
                    break;
                }
            }
            else // server_mode == jpv::ServerMode::IS
            {
                bool result = SetPOLParameterIS( clntMes, plotOverLineParameterPath, plotOverLineParameterPath_old, param );

                if( !result )
                {
                    std::cout << "ERROR:plot over line parameter setting failed" << std::endl;
                    break;
                }
            }

#ifndef CPU_VER
            generate_plot_over_line( param, mvpl, nan_error, jc, jd, pts, server_mode );
#else
            generate_plot_over_line( param, mvpl, nan_error, jd, pts, server_mode );
#endif
            std::cout << "==================== Plot Over Line End ====================" << std::endl;
        } // plot over line

        // connection reset
        if ( rank == 0 )
        {
            ptss = pts.recvMessage( &clntMes );

            if ( ptss == -1 )
            {
                std::cout << "ERROR:receive client message failed(connection reset)" << std::endl;
                break;
            }

            if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
            {
                std::cout << "==================== Connection Reset Start ====================" << std::endl;
                strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                servMes.m_camera = param.m_camera;
                servMes.m_server_status = 0;
                servMes.m_time_step = param.m_time_step;
                servMes.m_level_index = param.m_level_index;
                servMes.m_repeat_level = param.m_repeat_level;
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0 ;
                servMes.m_flag_send_bins = 1;
                servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                servMes.m_transfer_function_count = 0;
                servMes.m_start_step = mvpl.m_total_start_steps;
                servMes.m_last_step = mvpl.m_total_last_step;
                servMes.m_number_step = mvpl.m_total_number_steps;
                servMes.m_min_object_coord[0] = mvpl.m_total_min_object_coord[0];
                servMes.m_min_object_coord[1] = mvpl.m_total_min_object_coord[1];
                servMes.m_min_object_coord[2] = mvpl.m_total_min_object_coord[2];
                servMes.m_max_object_coord[0] = mvpl.m_total_max_object_coord[0];
                servMes.m_max_object_coord[1] = mvpl.m_total_max_object_coord[1];
                servMes.m_max_object_coord[2] = mvpl.m_total_max_object_coord[2];
                servMes.m_min_value = mvpl.m_total_min_value;
                servMes.m_max_value = mvpl.m_total_max_value;
                servMes.m_number_nodes = mvpl.m_total_number_nodes;
                servMes.m_number_elements = mvpl.m_total_number_elements;
                servMes.m_element_type = mvpl.m_list[0].m_elem_type;
                servMes.m_file_type = mvpl.m_list[0].m_file_type;
                servMes.m_number_ingredients = mvpl.m_list[0].m_number_ingredients;
                servMes.m_particle_limit = param.m_particle_limit;
                servMes.m_particle_density = param.m_particle_density;
                servMes.m_subpixel_level = param.m_subpixel_level;
                setParamTransferFunctionToServer( &servMes, &param );
                servMes.m_message_size = servMes.byteSize();
                servMes.show();
                pts.sendMessage( servMes );
                pts.disconnect();
                std::cout << "==================== Connection Reset End ====================" << std::endl;
            }
        } // connection reset 
    } // while ( true )

    if ( rank == 0 )
    {
        bsz = -1;
#ifndef CPU_VER
        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
        pts.termServer();
    }

    return;
    // =================== 新規作成 ここまで ===================

#if 0 // 新規作成以降をコメントアウト
    //=================== client-server mode ===================

    char* buf;
    int bsz = 0;

    JobDispatcher jd;
#ifndef CPU_VER
    JobCollector  jc( &jd );
#endif                   

    if ( rank > 0 )
    {
        //--------------------- WORKER --------------------
        jpv::ParticleTransferClientMessage clntMes;
        clntMes.m_camera = new vismodule::Camera();

        bool loop = true;

        while ( loop )
        {
            static int timer_count = 0;

            // recv cltMes from process 0 start
#ifndef CPU_VER
            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
            if ( bsz < 0 )
            {
                loop = false;
                std::cout << "Rank " << rank << ": Failed to Recv Client Message" << std::endl;
                break; // terminate server
            }
            buf = new char[bsz];
#ifndef CPU_VER
            MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
            clntMes.unpack( buf );
            delete[] buf;
            // recv cltMes from process 0 end

            // clntMes.show();
            std::cout << "Rank " << rank << ": Recv Client Message" << std::endl;

            if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
            {
            }
            else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
            {
            }
            else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step )
            {
                param.m_input_data_base = clntMes.m_input_directory;
                param.m_sampling_method = 'h';
                param.m_particle_limit = clntMes.m_particle_limit;
                param.m_particle_density = clntMes.m_particle_density;
                param.m_camera = clntMes.m_camera;
                param.m_x_synthesis = clntMes.m_x_synthesis;
                param.m_y_synthesis = clntMes.m_y_synthesis;
                param.m_z_synthesis = clntMes.m_z_synthesis;
                mvpl.searchFile(param);

                if( !clntMes.m_import_flag ) 
                {
                    std::cout << "defalt parameter " << std::endl;
                    VariableRange range = Calculate_minmax( param, mvpl );
                    transfunc_creator.setInitialProtocol( mvpl.m_total_number_ingredients, range );
                }
                else
                {
                    std::cout << "user define parameter " << std::endl;
                    transfunc_creator.setProtocol( clntMes );
                }

                param.m_transfunc_synthesizer = transfunc_creator.create();
                param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                {
                    param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                }

                initial_step_worker( param, mvpl, nan_error,
#ifndef CPU_VER
                    jc,
#endif
                    jd, transfunc_creator, timer_count );
            }
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
            {
                if ( clntMes.m_time_parameter == 0 )
                {
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                }
                else
                {
                    param.m_input_data_base = clntMes.m_input_directory;
                    param.m_time_step = clntMes.m_step;
                    param.m_level_index = clntMes.m_level_index;
                    param.m_repeat_level = clntMes.m_repeat_level;
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;
                    param.m_camera = clntMes.m_camera;
                    param.m_x_synthesis = clntMes.m_x_synthesis;
                    param.m_y_synthesis = clntMes.m_y_synthesis;
                    param.m_z_synthesis = clntMes.m_z_synthesis;

                    transfunc_creator.setProtocol( clntMes );
                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>( transfunc_creator.transfunc()[i] );
                    }

                    generate_particle_worker( param, mvpl, nan_error,
#ifndef CPU_VER
                        jc,
#endif
                        jd, transfunc_creator, timer_count );
               }
            }
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
            {
                generate_glyph_worker(param, clntMes, mvpl, nan_error, 
#ifndef CPU_VER
                    jc, 
#endif
                    jd,  transfunc_creator, timer_count, clntMes.m_initialize_parameter  );

            } // end of generate_glyph
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
            {
                generate_plot_over_line_worker(param, clntMes, mvpl, nan_error, 
#ifndef CPU_VER
                    jc, 
#endif
                    jd,  transfunc_creator, timer_count, clntMes.m_initialize_parameter  );

            } // end of plot_over_line
        } // end of while
    }
    else                    // rank == 0
    {
        //--------------------- MASTER --------------------
        int ptss;

        assert( jpv::ParticleTransferUtils::isLittleEndian() );

        jpv::ParticleTransferServer pts;
        ptss = pts.initializeServer( param.m_port );

        jpv::ParticleTransferServerMessage servMes;
        jpv::ParticleTransferClientMessage clntMes;
        clntMes.m_camera = new vismodule::Camera();
        servMes.m_camera = new vismodule::Camera();

        servMes.m_server_status = 0;
        // クライアント接続待ち
        pts.acceptServer();

        while ( ( ptss != -1 ) && ( pts.good() ) )
        {
            static int timer_count = 0;

            ptss = pts.recvMessage( &clntMes );
            //debug add by shimomura 2023/1/18
            clntMes.show();

#ifdef _WIN32 
            std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(), '/', '\\');
#elif defined(_WIN64)
            std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(), '/', '\\');
#endif
#ifdef __APPLE__
            std::string target = "¥";
            std::string replacement = "/";

            size_t pos = 0;
            while ((pos = clntMes.m_input_directory.find(target, pos)) != std::string::npos) {
                clntMes.m_input_directory.replace(pos, target.length(), replacement);
                pos += replacement.length();
            }
#elif defined(__linux__)
            std::string target = "\\";
            std::string replacement = "/";

            size_t pos = 0;
            while ((pos = clntMes.m_input_directory.find(target, pos)) != std::string::npos) {
                clntMes.m_input_directory.replace(pos, target.length(), replacement);
                pos += replacement.length();
            }
#endif

            std::cout << "input_directory = " << clntMes.m_input_directory << std::endl;
                    
            if ( ptss == -1 ) break;
            /* 140319 for client stop by Ctrl+c */
            signal( SIGABRT, SignalHandler );
            signal( SIGTERM, SignalHandler );
            signal( SIGINT, SignalHandler ); /* SIGINT is invalid here, because mpiexec uses it. */
            // signal( SIGSEGV, SignalHandler );
            if ( clntMes.m_step > mvpl.m_total_last_step )
            {
                clntMes.m_step = mvpl.m_total_last_step;
            }
            else if ( clntMes.m_step < mvpl.m_total_start_steps )
            {
                clntMes.m_step = mvpl.m_total_start_steps;
            }

            if ( SigServer )
            {
                clntMes.m_initialize_parameter = jpv::InitializeParameter::end; 
                std::cout << "*** SigServer" << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
            }
            else
            {
                /* 140319 for client stop by Ctrl+c */
                if ( clntMes.m_initialize_parameter != jpv::InitializeParameter::initial_step )
                {
                    clntMes.m_input_directory = param.m_input_data_base;
                }
            }

            std::cout << "Recieve message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
            if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
            {

                strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0 ;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;
                servMes.m_message_size = servMes.byteSize();
                pts.sendMessage( servMes );
                pts.disconnect();
                pts.acceptServer();
            }
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::end )
            {

                strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0 ;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;
                servMes.m_message_size = servMes.byteSize();
                pts.sendMessage( servMes );
                break;
            }
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::initial_step ) // change PFI file.
            {
                bool open_flag = true; // ファイルを開けるかどうか
                bool ExtendFileFormat_flag = true; // データ形式拡張に対応しているか
                bool pfi_flag = true; // .pfl, .pfiファイルを選択しているか
                
                param.m_input_data_base = clntMes.m_input_directory;

                std::ifstream fin( param.m_input_data_base, std::ios::in);
                /*
                if (!fin.is_open()) 
                {
                    std::cout << "ファイルを開けませんでした: " << param.m_input_data_base << std::endl;
                    open_flag = false;
                }
                */

#ifndef EXTEND_FILE_FORMAT
                ExtendFileFormat_flag = false;
                pfi_flag = false;
                size_t found_pfl = param.m_input_data_base.find(".pfl");
                size_t found_pfi = param.m_input_data_base.find(".pfi");
                if (found_pfl != std::string::npos) pfi_flag = true;
                if (found_pfi != std::string::npos) pfi_flag = true;
#endif

                std::cout << "open_flag = "             << open_flag             << ", "
                            << "ExtendFileFormat_flag = " << ExtendFileFormat_flag << ", "
                            << "pfi_flag = "              << pfi_flag              << std::endl;

                strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                // ADD by FEAST 2015.12.24
                //servMes.m_server_status = 0;
                // ADD END 2015.12.24
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0 ;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;
                servMes.m_message_size = servMes.byteSize();

                if ( open_flag == true && ExtendFileFormat_flag == true )
                {
                    servMes.m_file_enable_flag = jpv::FileEnableFlag::Enable_VTK;
                }
                if ( open_flag == true && ExtendFileFormat_flag == false && pfi_flag == true )
                {
                    // VTKには対応していないのでもう1つ状態が必要 Enable_pfi 追加予定
                    servMes.m_file_enable_flag = jpv::FileEnableFlag::Enable_VTK;
                }
                if ( open_flag == true && ExtendFileFormat_flag == false && pfi_flag == false )
                {
                    servMes.m_file_enable_flag = jpv::FileEnableFlag::NotEnable_VTK;
                }
                if (open_flag == false)
                {
                    servMes.m_file_enable_flag = jpv::FileEnableFlag::NoFile;
                }
                pts.sendMessage( servMes );

                if( servMes.m_file_enable_flag == jpv::FileEnableFlag::NotEnable_VTK ||
                    servMes.m_file_enable_flag == jpv::FileEnableFlag::NoFile ) 
                {
                    if ( rank == 0 )
                    {
                        std::cerr << "Error: pfifile doesn't exist" << std::endl;
                    }

                    bsz = -1;
#ifndef CPU_VER
                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

// #ifndef CPU_VER
                    // 開けなくても停止しないよう変更  予定 
                    // MPI_Finalize();
// #endif
                    // return 0;
                    // continue;
                    break;
                }

                // send cltMes to all worker process >>
                bsz = clntMes.byteSize();
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                char* buf;
                buf = new char[bsz];
                clntMes.pack( buf );
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                delete[] buf;
                // send cltMes to all worker process <<

                param.m_time_step = clntMes.m_step; 
                param.m_level_index = clntMes.m_level_index;
                param.m_repeat_level = clntMes.m_repeat_level;
                param.m_sampling_method = 'h';
                param.m_particle_limit = clntMes.m_particle_limit;
                param.m_particle_density = clntMes.m_particle_density;
                param.m_camera = clntMes.m_camera;
                param.m_x_synthesis = clntMes.m_x_synthesis;
                param.m_y_synthesis = clntMes.m_y_synthesis;
                param.m_z_synthesis = clntMes.m_z_synthesis;
                param.m_particle_data_size_limit = clntMes.m_particle_data_size_limit;

                mvpl.searchFile(param);

                if ( mvpl.m_list.size() <= 0 )
                {
                    if ( rank == 0 )
                    {
                        std::cerr << "Error: pfifile doesn't exist(rank:" << rank << ")" << std::endl;
                    }
                    bsz = -1;
#ifndef CPU_VER
                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
                    MPI_Finalize(); // 開けなくても停止しないよう変更  予定 
#endif
                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    // ADD by FEAST 2015.12.24
                    //servMes.m_server_status = 0;
                    // ADD END 2015.12.24
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1; // histogram
                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                            
                    break;
                }

                if( !clntMes.m_import_flag ) 
                {
                    std::cout << "defalt parameter " << std::endl;
                    VariableRange range = Calculate_minmax( param, mvpl );
                    transfunc_creator.setInitialProtocol( mvpl.m_total_number_ingredients, range );
                }
                else
                {
                    std::cout << "user define parameter " << std::endl;
                    transfunc_creator.setProtocol( clntMes );
                }                    

                param.m_transfunc_synthesizer = transfunc_creator.create();
                param.m_transfunc_array.resize(transfunc_creator.transfunc().size());

                for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                {
                    param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                }

                param.m_sampling_step = CalculateSamplingStep( mvpl );
                param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *param.m_camera );
                if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                initial_step_master( param, mvpl, nan_error,
#ifndef CPU_VER
                    jc,
#endif
                    jd, pts, timer_count );

            } // end of change PFI
            //else
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
            {
                timer_count++;
                if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                {
                    VIS_MODULE_TIMER_STA( 461 );
                }

                // send cltMes to all worker process >>
                bsz = clntMes.byteSize();
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                buf = new char[bsz];
                clntMes.pack( buf );
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                delete[] buf;
                // send cltMes to all worker process <<

                std::cout << "Recieve message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                std::cout << "sampling method = " << clntMes.m_sampling_method << std::endl;
                std::cout << "subpixel level = " << clntMes.m_subpixel_level << std::endl;
                std::cout << "repeat level = " << clntMes.m_repeat_level << std::endl;
                std::cout << "timeParam = " << clntMes.m_time_parameter << std::endl;

                if ( clntMes.m_time_parameter == 0 )
                {
                    std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                    std::cout << "beginTime = " << clntMes.m_begin_time << std::endl;
                    std::cout << "endTime = " << clntMes.m_last_time << std::endl;
                    std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    std::cout << "step = " << clntMes.m_step << std::endl;
                }
                std::cout << "transParam = " << clntMes.m_trans_parameter << std::endl;
                if ( clntMes.m_trans_parameter == 1 )
                {
                    std::cout << "levelIndex = " << clntMes.m_level_index << std::endl;
                }
                if ( clntMes.m_time_parameter == 0 )
                {
                    strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                    servMes.m_time_step = clntMes.m_step;
                    servMes.m_repeat_level = clntMes.m_repeat_level;
                    servMes.m_level_index = clntMes.m_level_index;
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                }
                else if ( clntMes.m_time_parameter == 1 )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                    servMes.m_time_step = clntMes.m_step;
                    servMes.m_repeat_level = clntMes.m_repeat_level;
                    servMes.m_level_index = clntMes.m_level_index;
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    param.m_time_step = clntMes.m_step;
                    param.m_level_index = clntMes.m_level_index;
                    param.m_repeat_level = clntMes.m_repeat_level;
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;
                    param.m_camera = clntMes.m_camera;
                    param.m_x_synthesis = clntMes.m_x_synthesis;
                    param.m_y_synthesis = clntMes.m_y_synthesis;
                    param.m_z_synthesis = clntMes.m_z_synthesis;

                    transfunc_creator.setProtocol( clntMes );
                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

                    generate_particle( param, mvpl, nan_error,
#ifndef CPU_VER
                        jc,
#endif                           
                        jd, pts );
                } // end of timeParam == 2
                else
                {
                    break;
                }
            } // end of initParam == 1 generate_particle 
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
            {
                // send cltMes to all worker process >>
                bsz = clntMes.byteSize();
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                buf = new char[bsz];
                clntMes.pack( buf );
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                delete[] buf;
                // send cltMes to all worker process <<

                std::cout << "Recieve message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                std::cout << "timeParam = " << clntMes.m_time_parameter << std::endl;

                if ( clntMes.m_time_parameter == 0 )
                {
                    std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                    std::cout << "beginTime = " << clntMes.m_begin_time << std::endl;
                    std::cout << "endTime = " << clntMes.m_last_time << std::endl;
                    std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    std::cout << "step = " << clntMes.m_step << std::endl;
                }
                std::cout << "transParam = " << clntMes.m_trans_parameter << std::endl;
                if ( clntMes.m_trans_parameter == 1 )
                {
                    std::cout << "levelIndex = " << clntMes.m_level_index << std::endl;
                }
                if ( clntMes.m_time_parameter == 0 )
                {
                    strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                    servMes.m_time_step = clntMes.m_step;
                    servMes.m_repeat_level = clntMes.m_repeat_level;
                    servMes.m_level_index = clntMes.m_level_index;
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                }
                else if ( clntMes.m_time_parameter == 1 )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                    servMes.m_time_step = clntMes.m_step;
                    servMes.m_repeat_level = clntMes.m_repeat_level;
                    servMes.m_level_index = clntMes.m_level_index;
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    param.m_time_step = clntMes.m_step;
                    param.m_level_index = clntMes.m_level_index;
                    param.m_repeat_level = clntMes.m_repeat_level;
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;
                    param.m_camera = clntMes.m_camera;
                    
                    Calculate_minmax_glyph( param, mvpl, clntMes );
                    param.m_transfunc_array.resize( transfunc_creator.transfunc().size() );
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

                    generate_glyph(param, mvpl, nan_error,
#ifndef CPU_VER
                        jc,
#endif                           
                        jd, pts );
                } // end of timeParam == 2
                else
                {
                    break;
                }
            } // end of initParam = 3 // generateglyph
            else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
            {
                // send cltMes to all worker process >>
                bsz = clntMes.byteSize();
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                buf = new char[bsz];
                clntMes.pack( buf );
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                delete[] buf;
                // send cltMes to all worker process <<

                std::cout << "Recieve message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                std::cout << "timeParam = " << clntMes.m_time_parameter << std::endl;

                if ( clntMes.m_time_parameter == 0 )
                {
                    std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                    std::cout << "beginTime = " << clntMes.m_begin_time << std::endl;
                    std::cout << "endTime = " << clntMes.m_last_time << std::endl;
                    std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    std::cout << "step = " << clntMes.m_step << std::endl;
                }
                std::cout << "transParam = " << clntMes.m_trans_parameter << std::endl;
                if ( clntMes.m_trans_parameter == 1 )
                {
                    std::cout << "levelIndex = " << clntMes.m_level_index << std::endl;
                }
                if ( clntMes.m_time_parameter == 0 )
                {
                    strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                    servMes.m_time_step = clntMes.m_step;
                    servMes.m_repeat_level = clntMes.m_repeat_level;
                    servMes.m_level_index = clntMes.m_level_index;
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                    strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                    servMes.m_time_step = clntMes.m_step;
                    servMes.m_repeat_level = clntMes.m_repeat_level;
                    servMes.m_level_index = clntMes.m_level_index;
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    param.m_time_step = clntMes.m_step;
                    param.m_level_index = clntMes.m_level_index;
                    param.m_repeat_level = clntMes.m_repeat_level;
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;
                    param.m_camera = clntMes.m_camera;
                    param.m_sampling_size = clntMes.m_sampling_size;

                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    for( int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

                    generate_plot_over_line_master( param, mvpl, nan_error,
#ifndef CPU_VER
                        jc, 
#endif                           
                        jd, pts );
                } // end of timeParam == 2
                else
                {
                    break;
                }
            } // end of initParam = 5 plot_over_line
        } // end of while (pts.good)

        delete clntMes.m_camera;
        delete servMes.m_camera;
        bsz = -1;
#ifndef CPU_VER
        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
        pts.termServer();
    } // rank == 0
//#ifndef CPU_VER
//    MPI_Finalize();
//#endif
    return;
#endif // 新規作成以降をコメントアウト
}

#if 0
void  IS_Connect( int argc, char** argv )
{

    VIS_MODULE_TIMER_INIT();
    VIS_MODULE_TIMER_STA( 1 );

    vismodule::Timer timer( vismodule::Timer::Start );

    Argument param( argc, argv );
    MultiVolumePropertyList mvpl;

    int rank = 0;
	int mpi_size = 1;

     //=================== client-server mode ===================

    char* buf;
    int bsz = 0;
    JobDispatcher jd;

    if ( rank > 0 )
    {
        // デーモンツールはシングルプロセス rank == 0
    } // end of if( rank > 0 )
    else                    // rank == 0
    {
        //--------------------- MASTER --------------------
        int ptss;

        // 20181226 start
        // 環境変数からパスを指定する
        // 粒子データ：$PARTICLE_DIR　粒子データの接頭辞はInSituLib内で固定のため、t_で固定する
        // 可視化パラメータ：$VIS_PARAM_DIR
        // tfファイル：$TF_NAME（未設定の場合、default.tf）
        std::string particlePath;
        std::string visParamDir;
        std::string tfFilePath;
        std::string tfFilePath_old;
        std::string glyphFilePath;
        std::string glyphParameterPath;
        std::string glyphParameterPath_old;
        std::string plotOverLineFilePath;
        std::string plotOverLineParameterPath;
        std::string plotOverLineParameterPath_old;

        const char *envBuf = NULL;
        envBuf = std::getenv( "PARTICLE_DIR" );
        if (envBuf == NULL) {
            particlePath = "./t";
            glyphFilePath = "./g";
            plotOverLineFilePath = "./p";
        }
        else {
            particlePath = envBuf;
            glyphFilePath = envBuf;
            plotOverLineFilePath = envBuf;
            if (particlePath[particlePath.size() - 1] != '/') {
                particlePath += "/t";
                glyphFilePath += "/g";
                plotOverLineFilePath += "/p";
            }
            else {
                particlePath += "t";
                glyphFilePath += "g";
                plotOverLineFilePath += "p";
            }
        }
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

        envBuf = std::getenv( "TF_NAME" );
        tfFilePath = visParamDir;
        tfFilePath_old = visParamDir;
        glyphParameterPath = visParamDir;
        glyphParameterPath_old = visParamDir;
        plotOverLineParameterPath = visParamDir;
        plotOverLineParameterPath_old = visParamDir;
        if (envBuf == NULL) {
            tfFilePath += "default.tf";
            tfFilePath_old += "default_old.tf";
            glyphParameterPath += "parameter.gly";
            plotOverLineParameterPath += "parameter.pol";
            glyphParameterPath_old += "parameter_old.gly";
            plotOverLineParameterPath_old += "parameter_old.pol";
        }
        else {
            tfFilePath += envBuf;
            tfFilePath += ".tf";
            tfFilePath_old += envBuf;
            tfFilePath_old += "_old.tf";
            glyphParameterPath += "parameter.gly";
            plotOverLineParameterPath += "parameter.pol";
            glyphParameterPath_old += "parameter_old.gly";
            plotOverLineParameterPath_old += "parameter_old.pol";
        }
        // 20181226 end
        std::string statePath = visParamDir + "state.txt";
        std::string historyPath = visParamDir + "history";

        assert( jpv::ParticleTransferUtils::isLittleEndian() );

        //ソケット通信手順
        //(1)ソケット生成 socket()
        //(2)ソケット登録 bind()
        //(3)通信準備    listen()
        //(4)接続待機    accept()
        //(5)受信/送信   recv()/send()
        //(6)ソケット切断 close()
        jpv::ParticleTransferServer pts;
        ptss = pts.initializeServer( param.m_port );

        jpv::ParticleTransferServerMessage servMes;
        jpv::ParticleTransferClientMessage clntMes;
        clntMes.m_camera = new vismodule::Camera();
        servMes.m_camera = new vismodule::Camera();

        timer.stop();
        std::cout << "Initialize: " << timer.sec() << " [sec/step]" << std::endl;

        servMes.m_server_status = 0;
        pts.acceptServer(); // クライアント接続待ち

        // Using environment variables, the constructor of the ParticleMonitor class
        // set particle file, glyph file, plot over line file, status file, history file,
        // and the min/max coordinates of the object.
        ParticleMonitor pm;

        //ソケットが存在すればgood
        while ( ( ptss != -1 ) && ( pts.good() ) )
        {
            static int timer_count = 0;

            //ソケット通信手順(5)
            //２回めの受信(client->daemon)
            //受信したデータをclntMesが読み取る
            ptss = pts.recvMessage( &clntMes );
            clntMes.show();

            if ( ptss == -1 ) break;
            /* 140319 for client stop by Ctrl+c */
            signal( SIGABRT, SignalHandler );
            signal( SIGTERM, SignalHandler );
            signal( SIGINT, SignalHandler ); /* SIGINT is invalid here, because mpiexec uses it. */

            if ( SigServer )
            {
                clntMes.m_initialize_parameter = jpv::InitializeParameter::end ;
                std::cout << "*** SigServer" << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
            }
            else
            {
                /* 140319 for client stop by Ctrl+c */
                if ( clntMes.m_initialize_parameter != jpv::InitializeParameter::initial_step )
                {
                    clntMes.m_input_directory = param.m_input_data_base;
                }
            }

            std::cout << "Receive message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
            //initParam -1:空ソケットの送信, -2:daemonを終了, それ以外:粒子データの送信
            if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
            {
std::cout << "jpv::InitializeParameter::connection_reset" << std::endl;
                //ほぼ空のソケットを送信する
                strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;
                servMes.m_message_size = servMes.byteSize();
                servMes.show();
                pts.sendMessage( servMes );
                pts.disconnect();
                pts.acceptServer(); // whileループの頭に戻る
            }
            else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
            {
std::cout << "jpv::InitializeParameter::end" << std::endl;
                strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;
                servMes.m_message_size = servMes.byteSize();
                servMes.show();
                pts.sendMessage( servMes );
                break; // whileループを抜けて終了
            }
            else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step ) // change PFI file.
            {
                std::cout << "==================== Initial Step Start ====================" << std::endl;
                pm.check();

                // store client message in param
                param.m_time_step = clntMes.m_step; 
                param.m_level_index = clntMes.m_level_index;
                param.m_repeat_level = clntMes.m_repeat_level;
                param.m_sampling_method = 'h';
                param.m_camera = clntMes.m_camera;
                param.m_x_synthesis = clntMes.m_x_synthesis;
                param.m_y_synthesis = clntMes.m_y_synthesis;
                param.m_z_synthesis = clntMes.m_z_synthesis;
                param.m_particle_data_size_limit = clntMes.m_particle_data_size_limit;
                if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                // 20181226 start  環境変数で指定したパスおよび名前でファイル参照を行う
                std::ifstream file(tfFilePath_old.c_str());
                if(file)
                {
                    ParameterFileReader ppr;
                    ppr.readParameterFile( tfFilePath_old.c_str() );
                    ppr.setParameter( &param );
                    ppr.setTransferFunction( &param );
                }
                else
                {
                    int tf_number;
                    VariableRange vr;
                    ParticleMonitor tmp_pm;

                    tmp_pm.check();

                    if( tmp_pm.stepExisted() )
                    {
                        tmp_pm.setTimeStep_particle( pm.particleStatusFile().getLatestTimeStep() );
                    }
                    else
                    {
                        tmp_pm.setTimeStep_particle(0);
                    }
                    tmp_pm.readParticleHistoryFile();

                    tf_number = pm.particleHistoryFile().colorHistogramArray().size();
                    vr = pm.particleHistoryFile().variableRange();
                    setDefalutTransferFunction( &param, vr, tf_number );
                }
                file.close();

                initial_step_IS( servMes, param, mvpl, jd, pts, 1 );
                std::cout << "==================== Initial Step end ====================" << std::endl;
            } // end of change PFI
            else if( clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_particle )
            {
                std::cout << "==================== Generate Particle Start ====================" << std::endl;
                std::cout << "jpv::InitializeParameter::generate_particle" << std::endl;

                // store client message in param
                param.m_time_step = clntMes.m_step; 
                param.m_level_index = clntMes.m_level_index;
                param.m_repeat_level = clntMes.m_repeat_level;
                param.m_sampling_method = clntMes.m_sampling_method;
                param.m_camera = clntMes.m_camera;
                param.m_x_synthesis = clntMes.m_x_synthesis;
                param.m_y_synthesis = clntMes.m_y_synthesis;
                param.m_z_synthesis = clntMes.m_z_synthesis;
                param.m_particle_data_size_limit = clntMes.m_particle_data_size_limit;
                if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                // update the transfer function file using the client message
                // updated transfer function file is loaded by InSitu
                // Daemon loads the particle file generated by InSitu
                ParameterFileWriter ppw;
                ParameterFileReader ppr;
                ppw.inputMessage( clntMes );
                ppr.readParameterFile( tfFilePath_old.c_str() );
                NameListFile nm1 = ppr.getNameListFile();
                NameListFile nm2 = ppw.getNameListFile();
                if( nm1 != nm2 )
                {
                    ppw.writeParameterFile( tfFilePath.c_str() );
                }

                if ( clntMes.m_time_parameter == 0 )
                {
                    std::cout << "ERROR:clntMes.m_time_parameter == 0 is an unexpected parameter." << std::endl;
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                    std::cout << "ERROR:clntMes.m_time_parameter == 1 is an unexpected parameter." << std::endl;
                }
                else if ( clntMes.m_time_parameter == 2 )
                {
                    generate_particle_IS(servMes, param, mvpl, jd, pts );
                } // clntMes.m_time_parameter == 2
                else
                {
                    return;
                }
                std::cout << "==================== Generate Particle End ====================" << std::endl;
            } // end of initParam =1
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::export_TFfile )
            {
std::cout << "jpv::InitializeParameter::export_TFfile" << std::endl;
                std::ifstream file(tfFilePath.c_str());
                ParameterFileWriter ppw;
                ppw.inputMessage( clntMes );
                ppw.writeParameterFile( tfFilePath.c_str() );
                file.close();
            }
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_glyph )
            {
std::cout << "jpv::InitializeParameter::generate_glyph" << std::endl;
                pm.check();
                generate_glyph_IS(param, clntMes, servMes, mvpl, 
                        jd, pts, pm, timer,
                        particlePath, glyphParameterPath, glyphParameterPath_old, 
                         timer_count, clntMes.m_initialize_parameter );

            }  // end loop of generate_glyph
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::send_glyph_flag_false )
            {
std::cout << "jpv::InitializeParameter::send_glyph_flag_false" << std::endl;
                ParameterFileWriter ppw;
                ppw.inputGlyphParameterMessage( clntMes );
                ppw.writeParameterFile( glyphParameterPath.c_str() );

            }
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::plot_over_line )
            {
std::cout << "jpv::InitializeParameter::plot_over_line" << std::endl;
                pm.check();
                generate_plot_over_line_IS(param, clntMes, servMes, mvpl, 
                        jd, pts, pm, timer,
                        particlePath, plotOverLineParameterPath, plotOverLineParameterPath_old, 
                         timer_count, clntMes.m_initialize_parameter );
            }  // end loop of plot over line

            if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_END( 1 );
                VIS_MODULE_TIMER_FIN();
            }
        } // end of while (pts.good)

        delete servMes.m_camera;
        delete clntMes.m_camera;

        pts.termServer();

    }

    return;
}

#endif

void SetParameterFilePath(
    std::string& tfFilePath,
    std::string& tfFilePath_old,
    std::string& glyphParameterPath,
    std::string& glyphParameterPath_old,
    std::string& plotOverLineParameterPath,
    std::string& plotOverLineParameterPath_old
)
{
        const char *envBuf = NULL;
        std::string visParamDir;

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

        tfFilePath                    = visParamDir;
        tfFilePath_old                = visParamDir;
        glyphParameterPath            = visParamDir;
        glyphParameterPath_old        = visParamDir;
        plotOverLineParameterPath     = visParamDir;
        plotOverLineParameterPath_old = visParamDir;

        envBuf = std::getenv( "TF_NAME" );

        if (envBuf == NULL) {
            tfFilePath                    += "default.tf";
            tfFilePath_old                += "default_old.tf";
            glyphParameterPath            += "parameter.gly";
            plotOverLineParameterPath     += "parameter.pol";
            glyphParameterPath_old        += "parameter_old.gly";
            plotOverLineParameterPath_old += "parameter_old.pol";
        }
        else {
            tfFilePath                    += envBuf;
            tfFilePath                    += ".tf";
            tfFilePath_old                += envBuf;
            tfFilePath_old                += "_old.tf";
            glyphParameterPath            += "parameter.gly";
            plotOverLineParameterPath     += "parameter.pol";
            glyphParameterPath_old        += "parameter_old.gly";
            plotOverLineParameterPath_old += "parameter_old.pol";
        } 
}

bool CheckFileFormat(
    jpv::ParticleTransferClientMessage& clntMes,
    jpv::ParticleTransferServer pts,
    Argument& param
)
{
    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    jpv::ParticleTransferServerMessage servMes;
    bool open_flag = true; // ファイルを開けるかどうか
    bool ExtendFileFormat_flag = true; // データ形式拡張に対応しているか
    bool pfi_flag = true; // .pfl, .pfiファイルを選択しているか

    param.m_input_data_base = clntMes.m_input_directory;

#ifndef EXTEND_FILE_FORMAT
    ExtendFileFormat_flag = false;
    pfi_flag = false;
    size_t found_pfl = param.m_input_data_base.find(".pfl");
    size_t found_pfi = param.m_input_data_base.find(".pfi");
    if (found_pfl != std::string::npos) pfi_flag = true;
    if (found_pfi != std::string::npos) pfi_flag = true;
#endif

    std::cout << "open_flag = "             << open_flag             << ", "
              << "ExtendFileFormat_flag = " << ExtendFileFormat_flag << ", "
              << "pfi_flag = "              << pfi_flag              << std::endl;

    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
    servMes.m_server_status = 0;
    servMes.m_number_particle = 0;
    servMes.m_number_glyph = 0 ;
    servMes.m_flag_send_bins = 1;
    servMes.m_transfer_function_count = 0;
    servMes.m_camera = new vismodule::Camera();
    servMes.m_message_size = servMes.byteSize();

    if ( open_flag == true && ExtendFileFormat_flag == true )
    {
        servMes.m_file_enable_flag = jpv::FileEnableFlag::Enable_VTK;
    }
    if ( open_flag == true && ExtendFileFormat_flag == false && pfi_flag == true )
    {
        // VTKには対応していないのでもう1つ状態が必要 Enable_pfi 追加予定
        servMes.m_file_enable_flag = jpv::FileEnableFlag::Enable_VTK;
    }
    if ( open_flag == true && ExtendFileFormat_flag == false && pfi_flag == false )
    {
        servMes.m_file_enable_flag = jpv::FileEnableFlag::NotEnable_VTK;
    }
    if (open_flag == false)
    {
        servMes.m_file_enable_flag = jpv::FileEnableFlag::NoFile;
    }
    pts.sendMessage( servMes );

    if( servMes.m_file_enable_flag == jpv::FileEnableFlag::NotEnable_VTK ||
        servMes.m_file_enable_flag == jpv::FileEnableFlag::NoFile ) 
    {
        if ( rank == 0 )
        {
            std::cerr << "Error: pfifile doesn't exist" << std::endl;
        }
        return false;
    }

    return true;
}

bool SetParticleParameterIS(
    jpv::ParticleTransferClientMessage& clntMes,
    std::string tfFilePath,
    std::string tfFilePath_old,
    Argument& param,
    MultiVolumePropertyList& mvpl
)
{
    MultiVolumeProperty mvp;

    std::cout << "clntMes.m_time_parameter = " << clntMes.m_time_parameter << std::endl;

    param.m_camera = clntMes.m_camera;

    /*
    param.m_time_step                = clntMes.m_step; 
    param.m_level_index              = clntMes.m_level_index;
    param.m_repeat_level             = clntMes.m_repeat_level;
    param.m_sampling_method          = 'h';
    param.m_x_synthesis              = clntMes.m_x_synthesis;
    param.m_y_synthesis              = clntMes.m_y_synthesis;
    param.m_z_synthesis              = clntMes.m_z_synthesis;
    param.m_particle_data_size_limit = clntMes.m_particle_data_size_limit;
    */

    // Using environment variables, the constructor of the ParticleMonitor class
    // set particle file, glyph file, plot over line file, status file, history file,
    // and the min/max coordinates of the object.
    ParticleMonitor pm;
    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_particle( pm.particleStatusFile().getLatestTimeStep() );
    }
    else
    {
        pm.setTimeStep_particle(0);
        std::cout << "WARN:particle status file does not exist" << std::endl;
    }
    pm.readParticleHistoryFile();                

    // store particle monitor in mvpl
    mvpl.m_total_start_steps       = pm.particleStatusFile().getStartTimeStep();
    mvpl.m_total_last_step         = pm.particleStatusFile().getLatestTimeStep();
    mvpl.m_total_number_steps      = mvpl.m_total_last_step - mvpl.m_total_start_steps + 1;
    mvp.m_file_type                = 0;
    mvp.m_elem_type                = 0;
    mvp.m_number_ingredients       = pm.particleHistoryFile().nVariables();
    mvpl.m_list.push_back(mvp);
    mvpl.m_total_number_elements   = 0;
    mvpl.m_total_number_nodes      = 0;
    mvpl.m_total_number_subvolumes = 1;
    mvpl.m_total_min_object_coord  = pm.getMinObjectCoords();
    mvpl.m_total_max_object_coord  = pm.getMaxObjectCoords();
    mvpl.m_total_min_value         = 0;
    mvpl.m_total_max_value         = 0;

    // store particle monitor in param
    // sampling step is not used in IS mode
    param.m_subpixel_level   = pm.getSubpixelLevel();
    // particle limit and particle density will be overwritten later
    // when transfer function file is readed(ParameterFileReader)
    param.m_particle_limit   = pm.particleHistoryFile().ParticleLimit();
    param.m_particle_density = pm.particleHistoryFile().ParticleDensity();
    param.m_server_side_variable_range = pm.particleHistoryFile().variableRange();

    VariableRange vr        = pm.particleHistoryFile().variableRange();
    int tf_number           = pm.particleHistoryFile().colorHistogramArray().size();

    ParameterFileReader ppr;
    ppr.readParticleParameterFile( tfFilePath_old.c_str() );
    ppr.setParticleParameter( param );

    if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_particle )
    {
        // update the transfer function file using the client message
        // updated transfer function file is loaded by InSitu
        // Daemon loads the particle file generated by InSitu
        ParameterFileWriter ppw;
        ppw.inputParticleParameterMessage( clntMes );
        NameListFile nm1 = ppr.getNameListFile();
        NameListFile nm2 = ppw.getNameListFile();
        if( nm1 != nm2 )
        {
            ppw.writeParameterFile( tfFilePath.c_str() );
        }
    }

std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;    

    return true;
}

bool SetGlyphParameterCS(
    jpv::ParticleTransferClientMessage& clntMes,
    Argument& param,
    MultiVolumePropertyList& mvpl
)
{
    std::cout << "clntMes.m_time_parameter = " << clntMes.m_time_parameter << std::endl;

    if( clntMes.m_time_parameter != 2 )
    {
        std::cout << "ERROR:glyph clntMes.m_time_parameter != 2" << std::endl;
        return false;
    }

    Calculate_minmax_glyph( param, mvpl, clntMes );

    param.m_stride                     = clntMes.m_stride;
    param.m_seed                       = clntMes.m_seed;
    param.m_number_of_sampling_point   = clntMes.m_number_of_sampling_point;
    param.m_glyph_color_min            = clntMes.m_glyph_color_min;
    param.m_glyph_color_max            = clntMes.m_glyph_color_max;
    param.m_glyph_size_min             = clntMes.m_glyph_size_min;
    param.m_glyph_size_max             = clntMes.m_glyph_size_max;
    param.m_glyph_color_map_table      = clntMes.m_glyph_color_map_table;
    param.m_color_map                  = clntMes.m_color_map;
    param.m_glyph_flag                 = clntMes.m_glyph_flag;
    param.m_direction_variable[0]      = clntMes.m_direction_variable[0];
    param.m_direction_variable[1]      = clntMes.m_direction_variable[1];
    param.m_direction_variable[2]      = clntMes.m_direction_variable[2];
    param.m_size_sampling_method       = clntMes.m_size_sampling_method;
    param.m_size_variable              = clntMes.m_size_variable;
    param.m_distribution_mode          = clntMes.m_distribution_mode;
    param.m_color_data_sampling_method = clntMes.m_color_data_sampling_method;
    param.m_color_data_variable        = clntMes.m_color_data_variable;

    return true;
}

bool SetGlyphParameterIS(
    jpv::ParticleTransferClientMessage& clntMes,
    std::string glyphParameterPath,
    std::string glyphParameterPath_old,
    Argument& param
)
{
    ParameterFileWriter ppw;
    ParameterFileReader ppr;
    
    ppw.inputGlyphParameterMessage( clntMes );
    ppr.readGlyphParameterFile( glyphParameterPath_old.c_str() );
    ppr.setGlyphParameter( param );

    NameListFile nm1 = ppr.getNameListFile();
    NameListFile nm2 = ppw.getNameListFile();

    if( nm1 != nm2 )
    {
        ppw.writeParameterFile( glyphParameterPath.c_str() );
    }

    return true;
}

bool SetPOLParameterCS(
    jpv::ParticleTransferClientMessage& clntMes,
    Argument& param
)
{
    std::cout << "clntMes.m_time_parameter = " << clntMes.m_time_parameter << std::endl;

    if( clntMes.m_time_parameter != 2 )
    {
        std::cout << "ERROR:plot over line clntMes.m_time_parameter != 2" << std::endl;
        return false;
    }

    param.m_plot_variable  = clntMes.m_plot_variable;
    param.m_start_point[0] = clntMes.m_start_point[0];
    param.m_start_point[1] = clntMes.m_start_point[1];
    param.m_start_point[2] = clntMes.m_start_point[2];
    param.m_end_point[0]   = clntMes.m_end_point[0];
    param.m_end_point[1]   = clntMes.m_end_point[1];
    param.m_end_point[2]   = clntMes.m_end_point[2];
    param.m_sampling_size  = clntMes.m_sampling_size;

    return true;
}

bool SetPOLParameterIS(
    jpv::ParticleTransferClientMessage& clntMes,    
    std::string plotOverLineParameterPath,
    std::string plotOverLineParameterPath_old,
    Argument& param
)
{
    ParameterFileWriter ppw;
    ParameterFileReader ppr;

    // 20181226 start　環境変数で指定したパスおよび名前でファイル参照を行う
    ppw.inputPlotOverLineParameterMessage( clntMes );
    ppr.readPlotOverLineParameterFile( plotOverLineParameterPath_old.c_str() );
    ppr.setPlotOverLineParameter( param );

    NameListFile nm1 = ppr.getNameListFile();
    NameListFile nm2 = ppw.getNameListFile();

    if( nm1 != nm2 )
    {
        ppw.writeParameterFile( plotOverLineParameterPath.c_str() );
    }

    return true;
}

void SetServerMessageParameter(
    Argument& param,
    MultiVolumePropertyList& mvpl,
    jpv::ParticleTransferServerMessage& servMes
)
{
    strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
    servMes.m_camera = param.m_camera;
    servMes.m_server_status = 0;
    servMes.m_time_step = param.m_time_step;
    servMes.m_level_index = param.m_level_index;
    servMes.m_repeat_level = param.m_repeat_level;
    servMes.m_number_particle = 0;
    servMes.m_number_glyph = 0;
    servMes.m_flag_send_bins = 1;
    servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
    servMes.m_transfer_function_count = 0;
    servMes.m_start_step = mvpl.m_total_start_steps;
    servMes.m_last_step = mvpl.m_total_last_step;
    servMes.m_number_step = mvpl.m_total_number_steps;
    servMes.m_min_object_coord[0] = mvpl.m_total_min_object_coord[0];
    servMes.m_min_object_coord[1] = mvpl.m_total_min_object_coord[1];
    servMes.m_min_object_coord[2] = mvpl.m_total_min_object_coord[2];
    servMes.m_max_object_coord[0] = mvpl.m_total_max_object_coord[0];
    servMes.m_max_object_coord[1] = mvpl.m_total_max_object_coord[1];
    servMes.m_max_object_coord[2] = mvpl.m_total_max_object_coord[2];
    servMes.m_min_value = mvpl.m_total_min_value;
    servMes.m_max_value = mvpl.m_total_max_value;
    servMes.m_number_nodes = mvpl.m_total_number_nodes;
    servMes.m_number_elements = mvpl.m_total_number_elements;
    servMes.m_element_type = mvpl.m_list[0].m_elem_type;
    servMes.m_file_type = mvpl.m_list[0].m_file_type;
    servMes.m_number_ingredients = mvpl.m_list[0].m_number_ingredients;
    servMes.m_opacity_transfer_function_synthesis = "O1";
    servMes.m_color_transfer_function_synthesis = "C1";
    servMes.m_particle_limit = param.m_particle_limit;
    servMes.m_particle_density = param.m_particle_density;
    servMes.m_subpixel_level = param.m_subpixel_level;
    servMes.m_server_side_variable_range = param.m_server_side_variable_range;
}