/*****************************************************************************/
/**
 *  main.cpp
 *  CPU Server Program without CUDA, without VIS_MODULE, without OpenGL.
 */
/*****************************************************************************/

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/PointObjectGenerator>

#include <vismodule/Argument>

#include <vismodule/AVSField>
#include <vismodule/Timer>
#include <vismodule/KVSMLObjectPointWriter>
//#include "KVSMLObjectPointMPIWriter.h"
#include <vismodule/JobDispatcher>
//#ifndef CPU_VER
#include <vismodule/JobCollector>
//#endif
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

//#include <vismodule/GlyphObjectGenerator>
//#include <vismodule/GlyphObjectCreator>

//plot over line
//#include <vismodule/POLObjectGenerator>

//#include <vismodule/Calculate>
//#include <vismodule/PointObjectCreator>
//#include <vismodule/InitialStep>
//#include <vismodule/GenerateParticle>
//#include <vismodule/GenerateGlyph>
//#include <vismodule/GeneratePOL>
#include <vismodule/Connect>
//#include <test/Connect.h>

//using FuncParser::Variable;
//using FuncParser::Variables;
//using FuncParser::Function;
//using FuncParser::FunctionParser;


int main( int argc, char** argv )
{
#ifndef CPU_VER
    MPI_Init( &argc, &argv );
#endif
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
//    std::vector<PointObjectCreator> point_creator_lst;
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
            jpv::ParticleTransferServerMessage servMes;
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new vismodule::Camera();
            servMes.m_camera = new vismodule::Camera();
#if 0
if(rank == 0)
{

            // CSかISかを判別するための通信
            jpv::ParticleTransferServer pts;
            ptss = pts.initializeServer( param.m_port );


            // クライアント接続待ち
            pts.acceptServer();
            static int timer_count = 0;

            ptss = pts.recvMessage( &clntMes );
            //debug add by shimomura 2023/1/18
            //clntMes.show();

            //受け取ったメッセージを各MPIプロセスに共有
            int bsz = 0;
            bsz = clntMes.byteSize();
            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
            buf = new char[bsz];
            clntMes.pack( buf );
            MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
            delete[] buf;
            // send cltMes to all worker process <<

}
else //rank == 0以外の処理
{
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                if ( bsz < 0 )
                {
                    loop = false;
                    std::cerr << "invalid message !!!!! "
                    return 0; // terminate server
                }
                buf = new char[bsz];
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                clntMes.unpack( buf );
                delete[] buf;
}
#endif
//debug 
//clntMes.m_server_mode = jpv::ServerMode::CS; 
clntMes.m_server_mode = jpv::ServerMode::IS; 

if( clntMes.m_server_mode == jpv::ServerMode::IS )
{
    if (rank ==0) IS_Connect(argc, argv);
    else std::cout << "warning !!! daemon does not work in MPI !!!  only rank 0 process is working!!!" << std::endl;
}
else if( clntMes.m_server_mode == jpv::ServerMode::CS )
{
    CS_Connect(argc, argv);
}
else
{
    std::cerr << "invalid sellect !!" << std::endl;
    return 0;
}



#ifndef CPU_VER
    MPI_Finalize();
#endif
    return retval;
}

#if 0
/**
 * ???C??????:
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char** argv )
{
#ifndef CPU_VER
    MPI_Init( &argc, &argv );
#endif
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
	int mpi_size = 1;
#endif

    if ( param.m_batch == true )
    {
    } // end of batch mode

    else  //=================== client-server mode ===================
    {

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
                   initial_step_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count );
               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
               {
                    generate_particle_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
               {
                    generate_glyph_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
               } // end of generate_glyph
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
               {
                    generate_plot_over_line_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
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
//                    std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(),"¥"[0], '/');
//                    std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(),'\\', '/');
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
                   initial_step_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count );
                } // end of change PFI
                //else
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
                {
                    generate_particle_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                } // end of initParam == 1 generate_particle 
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
                {
                    generate_glyph_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                } // end of initParam = 3 // generateglyph
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
                {
                    generate_plot_over_line_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
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
    }		// client-server mode
    if ( param.m_batch == true )
    {
        VIS_MODULE_TIMER_END( 1 );
        VIS_MODULE_TIMER_FIN();
    }
#ifndef CPU_VER
    MPI_Finalize();
#endif
    return retval;
}
#endif
