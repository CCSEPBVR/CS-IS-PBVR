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
void  CS_Connect( int argc, char** argv )
{
    VIS_MODULE_TIMER_INIT();
    VIS_MODULE_TIMER_STA( 1 );
    Argument param( argc, argv );
    MultiVolumePropertyList mvpl;
    TransferFunctionSynthesizerCreator transfunc_creator;

//    vismodule::Timer timer( vismodule::Timer::Start );
    vismodule::Camera camera;
    //Timer_CS test;

    //2023/06/01 shimomura 
    
    int retval = 0;
    int mpi_rank = 0;
    std::vector<PointObjectCreator> point_creator_lst;
    vismodule::PointObject* object = NULL;
    std::string output, outdir;
    std::string pout = "PARTICLE_OUTDIR";
    std::string prfx = "PARTICLE_SERVER_PREFIX";

    bool nan_error = false; // Add for NaN 2016.01.14

#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    int rank = 0;
	int mpi_size = 1;
#endif

    //=================== client-server mode ===================

        char* buf;
        int bsz = 0;
        JobDispatcher jd;
#ifndef CPU_VER
        JobCollector  jc( &jd );
#endif
       int st, vl, wid = 0;

        int c_bins_size = 0;
        int o_bins_size = 0;
        vismodule::UInt64* tmp_c_bins;
        vismodule::UInt64* tmp_o_bins;
                        
        //add by shimomura 2023/06/14
        float*  tmp_max;
        float*  tmp_min;

        if ( rank > 0 )
        {
            //--------------------- WORKER --------------------
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new vismodule::Camera();

            bool loop = true;

            while ( loop )
            {
                static int timer_count = 0;

                // recv cltMes from process 0 >>
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                if ( bsz < 0 )
                {
                    loop = false;
                    break; // terminate server
                }
                buf = new char[bsz];
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                clntMes.unpack( buf );
                delete[] buf;
                std::cout << "Rank " << rank << ": Recv Client Message" << std::endl;
                std::cout << "clntMes.m_initialize_parameter = " << (int)clntMes.m_initialize_parameter << std::endl;
                // recv cltMes from process 0 <<
               if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
               {
               }
               else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
               {
               }
               else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step )
               {
                   initial_step_worker(param, clntMes, mvpl, nan_error,  
#ifndef CPU_VER
                           jc, 
#endif
                           jd,  transfunc_creator, timer_count );
               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
               {
                   generate_particle_worker(param, clntMes, mvpl, nan_error, 
#ifndef CPU_VER
                           jc, 
#endif
                           jd, transfunc_creator, timer_count, clntMes.m_initialize_parameter  );

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
//            timer.start();
//            VIS_MODULE_TIMER_STA( 10 );
//            VIS_MODULE_TIMER_END( 10 );
//            timer.stop();
//            std::cout << "first reading time[ms]:" << timer.msec() << std::endl;

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
//              signal( SIGSEGV, SignalHandler );
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
                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                    pts.disconnect();

                    pts.acceptServer();
                }
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::end )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    // ADD by FEAST 2015.12.24
                    //servMes.m_server_status = 0;
                    // ADD END 2015.12.24
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;

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
                    param.m_is_tf_file_imported = clntMes.m_import_flag;

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

                    mvpl.searchFile(param);

                    if ( mvpl.m_list.size() <= 0 )
                    {
                        if ( rank == 0 )
                        {
                            std::cerr << "Error: pfifile doesn't exist" << std::endl;
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

                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;
                    param.m_camera = clntMes.m_camera;
                    param.m_x_synthesis = clntMes.m_x_synthesis;
                    param.m_y_synthesis = clntMes.m_y_synthesis;
                    param.m_z_synthesis = clntMes.m_z_synthesis;

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
                        // transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                    }                    

                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

                    initial_step_master( param, mvpl, nan_error,
#ifndef CPU_VER
                        jc,
#endif
                        jd, pts, transfunc_creator, timer_count );

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
                        transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                        param.m_transfunc_synthesizer = transfunc_creator.create();
                        param.m_transfunc_array.resize(transfunc_creator.transfunc().size());

                        for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
                        {
                            param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                        }

                        generate_particle_master( param, mvpl, nan_error,
#ifndef CPU_VER
                            jc,
#endif                           
                            jd, pts, transfunc_creator, timer_count );
                    } // end of timeParam == 2
                    else
                    {
                        break;
                    }
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 461 );
                    }
                    if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 1 );
                        VIS_MODULE_TIMER_FIN();
                    }
                } // end of initParam == 1 generate_particle 
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
                {
                    generate_glyph_master(param, clntMes, servMes, mvpl, nan_error,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

                } // end of initParam = 3 // generateglyph
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
                {
                   generate_plot_over_line_master(param, clntMes, servMes, mvpl, nan_error,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

                } // end of initParam = 5 plot_over_line

            } // end of while (pts.good)

            delete clntMes.m_camera;
            delete servMes.m_camera;
            bsz = -1;
#ifndef CPU_VER
            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

            pts.termServer();
        }		// rank == 0
//#ifndef CPU_VER
//    MPI_Finalize();
//#endif
    return ;
}


void  IS_Connect( int argc, char** argv )
{

    VIS_MODULE_TIMER_INIT();
    VIS_MODULE_TIMER_STA( 1 );

    vismodule::Timer timer( vismodule::Timer::Start );

    Argument param( argc, argv );
//    FilterInfoLst mvpl;
    MultiVolumePropertyList mvpl;
    //2018 kawamura comment out
    vismodule::Camera camera;
    int retval = 0;
    int mpi_rank = 0;
    
    std::string output, outdir;
    std::string pout = "PARTICLE_OUTDIR";
    std::string prfx = "PARTICLE_SERVER_PREFIX";


    int rank = 0;

     //=================== client-server mode ===================

    char* buf;
    int bsz = 0;
    JobDispatcher jd;
    int st, vl, wid = 0;

    int c_bins_size = 0;
    int o_bins_size = 0;
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;

    if ( rank > 0 )
    {
        //デーモンツールはシングルプロセス rank == 0
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
        plotOverLineParameterPath = visParamDir;
        glyphParameterPath_old = visParamDir;
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
        //ソケット通信手順(1)~(3)
        ptss = pts.initializeServer( param.m_port );//デフォルトparam.port:60000


        timer.stop();
        std::cout << "Initialize: " << timer.sec() << " [sec/step]" << std::endl;


        if ( ptss == -1 )
        {
            bsz = -1;
        }
        else //start init process 初期化処理開始
        {
            //ソケット通信手順(4)
            pts.acceptServer();

            pts.good();

            jpv::ParticleTransferServerMessage servMes;
            pts.good();
            jpv::ParticleTransferClientMessage clntMes;

            pts.good();

            servMes.m_camera = new vismodule::Camera();
            clntMes.m_camera = new vismodule::Camera();
            //ソケット通信手順(5)
            //受信したデータをclntMesが読み取る
            int ptss = 0;
            ptss = pts.recvMessage( &clntMes );
            if( ptss == -1 ) std::cout<<"main.cpp:L214. pts.recvMessage has ERROR."<<std::endl;
            //最初の受信(client->daemon)
            //受信内容clntMesにはデフォルト伝達関数が含まれるが
            //jupiter_old.tfを反映するため無視する
            int timerTmp = 0; 
            initial_step_IS(param, clntMes, servMes, mvpl, 
                    particlePath, glyphFilePath, plotOverLineFilePath, statePath, historyPath, tfFilePath_old,
                    jd, pts,  timerTmp);

        }// end of init process 初期化終了

        jpv::ParticleTransferServerMessage servMes;
        jpv::ParticleTransferClientMessage clntMes;
        clntMes.m_camera = new vismodule::Camera();
        servMes.m_camera = new vismodule::Camera();
        servMes.m_server_status =0;
        // 20181226 start
        // stateおよびhistory用に、環境変数から指定されたパスをもとにファイルパスを作成
        ParticleMonitor pm( particlePath, glyphFilePath, plotOverLineFilePath, statePath.c_str(), historyPath.c_str() );
        // 20181226 end

        //ソケットが存在すればgood
        while ( ( ptss != -1 ) && ( pts.good() ) )
        {
            static int timer_count = 0;

            //ソケット通信手順(5)
            //２回めの受信(client->daemon)
            //受信したデータをclntMesが読み取る
            ptss = pts.recvMessage( &clntMes );
            std::cout<<"main.cpp:L388"<<std::endl;
            clntMes.show();
            std::cout<<"ptss="<<ptss<<std::endl;

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
                //ほぼ空のソケットを送信する
                strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;

                servMes.m_message_size = servMes.byteSize();

                //2回目の送信(daemon->client)
                //servMesの中身はheaderのみ。
                //ほぼ空の情報を送信する。
                std::cout<<"main.cpp:L422"<<std::endl;
                clntMes.show();

                pts.sendMessage( servMes );
                pts.disconnect();

                //ソケット通信手順(4)
                pts.acceptServer();
                //whileループの頭に戻る
            }
            else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
            {
                //終了する
                strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_number_glyph = 0;
                servMes.m_flag_send_bins = 1;
                servMes.m_transfer_function_count = 0;

                servMes.m_message_size = servMes.byteSize();

                std::cout<<"main.cpp:L439"<<std::endl;

                pts.sendMessage( servMes );
                break;
                //whileループを抜けてpts.terminateを実行
            }
            else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step ) // change PFI file.
            {
                initial_step_IS(param, clntMes, servMes, mvpl, 
                        particlePath, glyphFilePath, plotOverLineFilePath, statePath, historyPath, tfFilePath_old,
                        jd, pts,  timer_count);
            } // end of change PFI
            else if( clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_particle )
            {

                generate_particle_IS(param, clntMes, servMes, mvpl, 
                        jd, pts, pm, timer,
                        particlePath, tfFilePath,tfFilePath_old, 
                        timer_count, clntMes.m_initialize_parameter );
            } // end of initParam =1
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::export_TFfile )
            {
                std::ifstream file(tfFilePath.c_str());
                ParameterFileWriter ppw;
                ppw.inputMessage( clntMes );
                ppw.writeParameterFile( tfFilePath.c_str() );
                file.close();
            }
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_glyph )
            {
                generate_glyph_IS(param, clntMes, servMes, mvpl, 
                        jd, pts, pm, timer,
                        particlePath, glyphParameterPath, glyphParameterPath_old, 
                         timer_count, clntMes.m_initialize_parameter );

            }  // end loop of generate_glyph
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::send_glyph_flag_false )
            {
                ParameterFileWriter ppw;
                ppw.inputGlyphParameterMessage( clntMes );
                ppw.writeParameterFile( glyphParameterPath.c_str() );

            }
            else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::plot_over_line )
            {
                generate_plot_over_line_IS(param, clntMes, servMes, mvpl, 
                        jd, pts, pm, timer,
                        particlePath, plotOverLineParameterPath, plotOverLineParameterPath_old, 
                         timer_count, clntMes.m_initialize_parameter );
            }  // end loop of plot over line
        } // end of while (pts.good)

        delete servMes.m_camera;
        delete clntMes.m_camera;

        pts.termServer();
    }
return ;
}
