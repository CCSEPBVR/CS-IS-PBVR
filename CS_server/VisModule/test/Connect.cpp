/*****************************************************************************/
/**
 *  main.cpp
 *  CPU Server Program without CUDA, without VIS_MODULE, without OpenGL.
 */
/*****************************************************************************/

#include <vismodule/Connect>
//#include "Connect.h"
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

#include <vismodule/GlyphObjectGenerator>
#include <vismodule/GlyphObjectCreator>

//plot over line
#include <vismodule/POLObjectGenerator>

//#include <vismodule/Calculate>
//#include <vismodule/PointObjectCreator>
//#include <vismodule/SignalHandler>
#include <vismodule/InitialStep>
#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/GeneratePOL>
#include <signal.h>



using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;

bool SigServer = false;
bool useAllNodes = true;
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
//int main( int argc, char** argv )
void  CS_Connect( int argc, char** argv )
{
//#ifndef CPU_VER
//    MPI_Init( &argc, &argv );
//#endif
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
                   initial_step_worker(param, clntMes, mvpl, nan_error, point_creator_lst, 
#ifndef CPU_VER
                           jc, 
#endif
                           jd, useAllNodes, transfunc_creator, timer_count );
               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
               {
//                    generate_particle_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                   generate_particle_worker(param, clntMes, mvpl, nan_error, point_creator_lst, 
#ifndef CPU_VER
                           jc, 
#endif
                           jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter  );

               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
               {
//                    generate_glyph_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                   generate_glyph_worker(param, clntMes, mvpl, nan_error, point_creator_lst, 
#ifndef CPU_VER
                           jc, 
#endif
                           jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter  );

               } // end of generate_glyph
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
               {
//                    generate_plot_over_line_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                   generate_plot_over_line_worker(param, clntMes, mvpl, nan_error, point_creator_lst, 
#ifndef CPU_VER
                           jc, 
#endif
                           jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter  );

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
                   initial_step_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, useAllNodes, transfunc_creator, timer_count );

                } // end of change PFI
                //else
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
                {
//                    generate_particle_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

                    generate_particle_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

                } // end of initParam == 1 generate_particle 
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
                {
//                    generate_glyph_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                    generate_glyph_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

                } // end of initParam = 3 // generateglyph
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
                {
//                   generate_plot_over_line_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                   generate_plot_over_line_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

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
//#ifndef CPU_VER
//    MPI_Finalize();
//#endif
    return ;
}

//IS
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>
#include <vismodule/ParameterFileReader>
#include <vismodule/SetDefaultTransferFunction>
#include <DaemonAndSampler/Daemon/Timer.h>


void  IS_Connect( int argc, char** argv )
{

//#ifndef CPU_VER
//    MPI::Init( argc, argv );
//#endif
    VIS_MODULE_TIMER_INIT();
    VIS_MODULE_TIMER_STA( 1 );

    vismodule::Timer timer( vismodule::Timer::Start );

    Argument param( argc, argv );
//    FilterInfoLst mvpl;
    MultiVolumePropertyList mvpl;
    //2018 kawamura comment out
    //TransferFunctionSynthesizerCreator transfunc_creator;
    vismodule::Camera camera;
    int retval = 0;
    int mpi_rank = 0;

    //std::vector<PointObjectCreator> point_creator_lst;
    //PointObjectCreator point_creator;

    //pbvr::PointObject* object = NULL;
    std::string output, outdir;
    std::string pout = "PARTICLE_OUTDIR";
    std::string prfx = "PARTICLE_SERVER_PREFIX";


//#ifndef CPU_VER
//    int rank = MPI::COMM_WORLD.Get_rank();
//#else
    int rank = 0;
//#endif

    if ( param.m_batch == true )
    {
        //デーモンツールでは param.m_batch == false
    } // end of batch mode

    else  //=================== client-server mode ===================
    {

        char* buf;
        int bsz = 0;
        JobDispatcher jd;
//#ifndef CPU_VER
//        JobCollector  jc( &jd );
//#endif
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
//#ifndef CPU_VER
//                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
//#endif
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
//jupiter start
//              param.input_data_base = clntMes.m_input_directory;
                param.m_input_data_base = "dummy";

                std::cout<<"main.cpp:L221"<<std::endl;
                //clntMes.show();
//jupiter end
                VariableRange range;

                strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                servMes.m_transfer_function_count = 0;
                servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                servMes.m_time_step = mvpl.m_total_start_steps;
                servMes.m_start_step = mvpl.m_total_start_steps;
                servMes.m_last_step = mvpl.m_total_last_step;
                servMes.m_number_step = mvpl.m_total_last_step;
                //servMes.m_number_volume_divide = mvpl.m_total_numSubVolumes;
                //servMes.m_time_step = mvpl.m_total_staSteps;
                //servMes.m_start_step = mvpl.m_total_staSteps;
                //servMes.m_last_step = mvpl.m_total_endSteps;
                //servMes.m_number_step = mvpl.m_total_numSteps;
//jupiter start
#if 0
                servMes.minObjectCoord[0] = mvpl.total_minObjectCoord[0];
                servMes.minObjectCoord[1] = mvpl.total_minObjectCoord[1];
                servMes.minObjectCoord[2] = mvpl.total_minObjectCoord[2];
                servMes.maxObjectCoord[0] = mvpl.total_maxObjectCoord[0];
                servMes.maxObjectCoord[1] = mvpl.total_maxObjectCoord[1];
                servMes.maxObjectCoord[2] = mvpl.total_maxObjectCoord[2];
#else

#if 0
                //clntMesに含まれる-vinオプションで指定されたパスを使用
                //デーモン(pbvr_server)をサンプラと同じディレクトリに配置
                //相対パスでOK  ./jupiter_particle_out/t
                jupiter_prefix = clntMes.m_input_directory;
                // 存在領域設定ファイルがあるか確認をする
                std::string filename( jupiter_prefix );
#else
                // 20181226 start
                // 環境変数の値を直接使用する
                std::string filename( particlePath );
                // 20181226 end
#endif
                filename.append( "_pfi_coords_minmax.txt" );
                vismodule::File f( filename.c_str()  );
                if ( f.isExisted() )
                {
                    // ファイルがある
                    FILE* fp = NULL;
                    fp = fopen( filename.c_str(), "r" );
                    fscanf( fp, "%f %f %f %f %f %f",
                            &servMes.m_min_object_coord[0],
                            &servMes.m_min_object_coord[1],
                            &servMes.m_min_object_coord[2],
                            &servMes.m_max_object_coord[0],
                            &servMes.m_max_object_coord[1],
                            &servMes.m_max_object_coord[2]);
                    if ( fp != NULL ) fclose( fp );
                }
                else
                {
                    servMes.m_min_object_coord[0]=0.f;
                    servMes.m_min_object_coord[1]=0.f;
                    servMes.m_min_object_coord[2]=0.f;
                    servMes.m_max_object_coord[0]=0.1;
                    servMes.m_max_object_coord[1]=0.1;
                    servMes.m_max_object_coord[2]=0.1;
                }

#endif

//jupiter end

//// read minmax & histrogram 

                ParticleMonitor pm( particlePath, glyphFilePath, plotOverLineFilePath, statePath.c_str(), historyPath.c_str() );
                pm.check();

                servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                if( pm.stepExisted() )
                {
                    pm.setTimeStep_particle(pm.particleStatusFile().getLatestTimeStep());
                    //pm.setTimeStep(0);
                }
                else
                {
                    pm.setTimeStep_particle(0);
                }
                pm.readParticleHistoryFile();
                range = pm.particleHistoryFile().variableRange();

                const int tf_number = pm.particleHistoryFile().colorHistogramArray().size();
                servMes.m_transfer_function_count =       tf_number;//TF_COUNT
                servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
                servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];

                servMes.m_color_bins.resize(   tf_number );
                servMes.m_opacity_bins.resize( tf_number );

                for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                {
                    servMes.m_color_nbins[tf] = DEFAULT_NBINS;
                    servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
                    servMes.m_color_bins[tf]   =  new vismodule::UInt64[ servMes.m_color_nbins[tf] ];
                    servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ servMes.m_opacity_nbins[tf] ];
                    for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                    {
                        servMes.m_color_bins[tf][res] = 0;
                    }
                    for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                    {
                        servMes.m_opacity_bins[tf][res] = 0;
                    }
                }

                servMes.m_transfer_function_count = pm.particleHistoryFile().colorHistogramArray().size();
                for ( int tf = 0; tf < pm.particleHistoryFile().colorHistogramArray().size() && tf < servMes.m_transfer_function_count; tf++ )
                {
                    servMes.m_color_nbins[tf] = pm.particleHistoryFile().colorHistogramArray()[ tf ].size();
                    for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                    {
                        servMes.m_color_bins[tf][res] = pm.particleHistoryFile().colorHistogramArray()[ tf ][res];
                    }
                }

                for ( int tf = 0; tf < pm.particleHistoryFile().opacityHistogramArray().size() && tf < servMes.m_transfer_function_count; tf++ )
                {
                    servMes.m_opacity_nbins[tf] = pm.particleHistoryFile().opacityHistogramArray()[ tf ].size();
                    for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                    {
                        servMes.m_opacity_bins[tf][res] = pm.particleHistoryFile().opacityHistogramArray()[ tf ][ res ];
                    }
                }

                servMes.m_min_value = mvpl.m_total_min_value;
                servMes.m_max_value = mvpl.m_total_max_value;
                servMes.m_number_nodes = mvpl.m_total_number_nodes;
                servMes.m_number_elements = mvpl.m_total_number_elements;
                servMes.m_number_ingredients = pm.particleHistoryFile().nVariables();
                servMes.m_server_side_variable_range = range;
                servMes.m_flag_send_bins = 1;
                servMes.m_number_glyph = 0;
                servMes.m_particle_limit = pm.particleHistoryFile().ParticleLimit();
                servMes.m_particle_density = pm.particleHistoryFile().ParticleDensity();

                // 20181226 start  環境変数で指定したパスおよび名前でファイル参照を行う
                //初期化 : jupiter_old.tfを読む
                ParameterFileReader ppr;
                //ppr.readParameterFile("jupiter_old.tf");
                ppr.readParameterFile( tfFilePath_old.c_str() );
                NameListFile nm = ppr.getNameListFile();
                //最初の送信(daemon->client)
                //jupiter_old.tfの内容をクライアントに送信
                std::ifstream file(tfFilePath_old.c_str());
                if(file)
                {
                    ppr.outputMessage( &servMes );
                }
                else
                { 
                    setDefalutTransferFunction(&servMes, pm.particleHistoryFile().colorHistogramArray().size() );
                }
                file.close();

                std::cout<<"main.cpp:571"<<std::endl;
                servMes.show();

//                servMes.m_server_status =0;
                servMes.m_message_size = servMes.byteSize();
                pts.sendMessage( servMes );
                delete servMes.m_camera;
                delete clntMes.m_camera;
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
                } // end of change PFI
                else if( clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_particle )
                {
                    timer_count++;
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_STA( 461 );
                    }

//                    // send cltMes to all worker process >>
//                    bsz = clntMes.byteSize();
//#ifndef CPU_VER
//                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
//#endif
//                    buf = new char[bsz];
//                    clntMes.pack( buf );
//#ifndef CPU_VER
//                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
//#endif
//                    delete[] buf;
//                    // send cltMes to all worker process <<

                    std::cout << "initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                    if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
                    {

                        std::cout << "sampling method = " << clntMes.m_sampling_method << std::endl;
                        std::cout << "subpixel level = " << clntMes.m_subpixel_level << std::endl;
                        std::cout << "repeat level = " << clntMes.m_repeat_level << std::endl;
                    }
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
                        servMes.m_number_glyph = 0;
                        servMes.m_flag_send_bins = 1;

                        servMes.m_message_size = servMes.byteSize();

                        std::cout<<"main.cpp:L509"<<std::endl;
                        clntMes.show();

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

                        std::cout<<"main.cpp:L526"<<std::endl;
                        clntMes.show();

                        pts.sendMessage( servMes );
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        TimerInitialize();
                        TimerStart( 10 );
                        strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
                        servMes.m_message_size = servMes.byteSize();
                      //servMes.m_time_step = clntMes.m_step;
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        servMes.m_number_glyph = 0;
                        param.m_sampling_method = clntMes.m_sampling_method;
                        param.m_component_Id = clntMes.m_rendering_id;
                        param.m_crop.setEnable( clntMes.m_enable_crop_region );
                        param.m_crop.set( clntMes.m_crop_region );
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

                        if ( clntMes.m_node_type == 'a' )
                        {
                            useAllNodes = true;
                        }
                        else if ( clntMes.m_node_type == 's' )
                        {
                            useAllNodes = false;
                        }
                        else
                        {
                            assert( false );
                        }

                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                        
                        mvpl.m_total_number_subvolumes=1;
                        if ( param.m_crop.isEnabled() )
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size,
                                           param.m_crop.getMinCoord(),
                                           param.m_crop.getMaxCoord() );
                            servMes.m_number_volume_divide = jd.getCountVolumes();
                        }
                        else
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size );
                            servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                        }

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_STA( 470 );
                        }

                        param.m_sampling_step = CalculateSamplingStep( mvpl );
//                      param.subpixel_level = CalculateSubpixelLevel( param, fil, *clntMes.m_camera );

                        VariableRange vr;

                        pm.check();
                        servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                        servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                        if( pm.stepExisted() )
                        {
                            //if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step && pm.getTimeStep() > -1 )
                            if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step )
                            {
                                servMes.m_time_step = clntMes.m_step;
                            }
                            else
                            {
                                servMes.m_time_step = pm.particleStatusFile().getLatestTimeStep();
                                clntMes.m_step = servMes.m_time_step;
                            }
                        }
                        else
                        {
                            std::cout << " no step !!!!!!!!!!!" << std::endl;
                            clntMes.m_step = -1;
                        }

                        std::cout<<"main.cpp:L614"<<std::endl;
                        //clntMes.show();

                        pts.sendMessage( servMes );
                        std::cout<< __LINE__ << __FUNCTION__ <<std::endl;


                        timer.start();

                        servMes.m_transfer_function_count = clntMes.m_transfer_function.size();//TF_COUNT
                        servMes.m_color_nbins = new vismodule::UInt64[clntMes.m_transfer_function.size()];
                        servMes.m_opacity_nbins = new vismodule::UInt64[clntMes.m_transfer_function.size()];

                        servMes.m_color_bins.resize( clntMes.m_transfer_function.size() );
                        servMes.m_opacity_bins.resize( clntMes.m_transfer_function.size() );

                        c_bins_size = 0;
                        o_bins_size = 0;

                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            servMes.m_color_nbins[tf] = DEFAULT_NBINS;
                            servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
                            servMes.m_color_bins[tf] =  new vismodule::UInt64[ servMes.m_color_nbins[tf] ];
                            servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ servMes.m_opacity_nbins[tf] ];
                            c_bins_size += servMes.m_color_nbins[tf];
                            o_bins_size += servMes.m_opacity_nbins[tf];
                            for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                            {
                                servMes.m_color_bins[tf][res] = 0;
                            }
                            for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                            {
                                servMes.m_opacity_bins[tf][res] = 0;
                            }
                        }

                        std::cout<< __LINE__ << __FUNCTION__ <<std::endl;
                        tmp_c_bins = new vismodule::UInt64[c_bins_size];
                        tmp_o_bins = new vismodule::UInt64[o_bins_size];

                        for ( int tf = 0; tf < c_bins_size; tf++ )
                        {
                            tmp_c_bins[tf] = 0;
                        }

                        for ( int tf = 0; tf < o_bins_size; tf++ )
                        {
                            tmp_o_bins[tf] = 0;
                        }


                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 471 );
                            }

//jupiter start
//                          TimerInitialize();
                            TimerStart( 1 );
                            ParameterFileWriter ppw;
                            ParameterFileReader ppr;

                            //clntMes.show();
                            // 20181226 start　環境変数で指定したパスおよび名前でファイル参照を行う
                            ppw.inputMessage( clntMes );
                            //ppr.readParameterFile("jupiter_old.tf");
                            ppr.readParameterFile( tfFilePath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();
                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( tfFilePath.c_str() );
                            }
                            // 20181226 end

                            vismodule::PointObject* originalObject = new vismodule::PointObject;
//                          ParticleMonitor pm( "/Users/admin/Work_noda/2015_PBVR/03_testrun/v1.08_test/pout/case","" );
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
                            //std::string filename( jupiter_prefix );
                            std::string filename( particlePath );
                            // 20181226 end
                            filename.append( "_pfi_coords_minmax.txt" );
                            vismodule::File f( filename.c_str()  );

                            if ( f.isExisted() )
                            {
                                // ファイルがある
                                FILE* fp = NULL;
                                fp = fopen( filename.c_str(), "r" );
                                fscanf( fp, "%f %f %f %f %f %f",
                                        &servMes.m_min_object_coord[0],
                                        &servMes.m_min_object_coord[1],
                                        &servMes.m_min_object_coord[2],
                                        &servMes.m_max_object_coord[0],
                                        &servMes.m_max_object_coord[1],
                                        &servMes.m_max_object_coord[2]);
                                if ( fp != NULL ) fclose( fp );
                            }
                            else
                            {
                                servMes.m_min_object_coord[0]=0.f;
                                servMes.m_min_object_coord[1]=0.f;
                                servMes.m_min_object_coord[2]=0.f;
                                servMes.m_max_object_coord[0]=0.1;
                                servMes.m_max_object_coord[1]=0.1;
                                servMes.m_max_object_coord[2]=0.1;
                            }

                            TimerStop( 2 );
                            if( pm.setTimeStep_particle( clntMes.m_step ) ) servMes.m_flag_send_bins = 0;
                            else                                 servMes.m_flag_send_bins = 1;
//                            else                                 servMes.m_flag_send_bins = 0;
                            if( servMes.m_flag_send_bins == 0)
                            {
                                pm.readParticleHistoryFile();
                                pm.readParticleFile();
                                pm.getParticle( originalObject );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            servMes.m_subpixel_level = pm.getSubpixelLevel();
                            vr = pm.particleHistoryFile().variableRange();
                           
                            //　粒子生成時に変数の数も取得するよう変更, paerticle_limit,particle_densityを取得するよう変更 2025/05/30
                            //servMes.m_number_elements = fil.total_numElements;
                            servMes.m_number_ingredients = pm.particleHistoryFile().nVariables();
                            servMes.m_particle_limit = pm.particleHistoryFile().ParticleLimit();
                            servMes.m_particle_density = pm.particleHistoryFile().ParticleDensity();

                            TimerStart( 3 );
                            for ( int tf = 0; tf < pm.particleHistoryFile().colorHistogramArray().size() && tf < servMes.m_transfer_function_count; tf++ )
                            {
                                servMes.m_color_nbins[tf] = pm.particleHistoryFile().colorHistogramArray()[ tf ].size();
                                for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                                {
                                    servMes.m_color_bins[tf][res] = pm.particleHistoryFile().colorHistogramArray()[ tf ][res];
                                }
                            }

                            for ( int tf = 0; tf < pm.particleHistoryFile().opacityHistogramArray().size() && tf < servMes.m_transfer_function_count; tf++ )
                            {
                                servMes.m_opacity_nbins[tf] = pm.particleHistoryFile().opacityHistogramArray()[ tf ].size();
                                for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                                {
                                    servMes.m_opacity_bins[tf][res] = pm.particleHistoryFile().opacityHistogramArray()[ tf ][ res ];
                                }
                            }
                            TimerStop( 3 );
//jupiter end
                            servMes.m_number_particle = originalObject->coords().size() / 3;
                            if ( servMes.m_number_particle > 0 )
                            {
                                servMes.m_positions = std::make_unique<float[]>(3 * servMes.m_number_particle);
                                servMes.m_normals = std::make_unique<float[]>(3 * servMes.m_number_particle);
                                servMes.m_colors = std::make_unique<unsigned char[]>(3 * servMes.m_number_particle);
                            }
                            else
                            {
                                servMes.m_positions = NULL;
                                servMes.m_normals   = NULL;
                                servMes.m_colors    = NULL;
                            }
                        std::cout<< __LINE__ << __FUNCTION__ <<std::endl;
                            for ( int i = 0; i < servMes.m_number_particle; ++i )
                            {
                                servMes.m_positions[3 * i + 0] = originalObject->coords()[3 * i + 0];
                                servMes.m_positions[3 * i + 1] = originalObject->coords()[3 * i + 1];
                                servMes.m_positions[3 * i + 2] = originalObject->coords()[3 * i + 2];
                                servMes.m_normals[3 * i + 0] = originalObject->normals()[3 * i + 0];
                                servMes.m_normals[3 * i + 1] = originalObject->normals()[3 * i + 1];
                                servMes.m_normals[3 * i + 2] = originalObject->normals()[3 * i + 2];
                                servMes.m_colors[3 * i + 0] = originalObject->colors()[3 * i + 0];
                                servMes.m_colors[3 * i + 1] = originalObject->colors()[3 * i + 1];
                                servMes.m_colors[3 * i + 2] = originalObject->colors()[3 * i + 2];
                            }
                            servMes.m_server_side_variable_range = vr;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 471 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 472 );
                            }
                            servMes.m_message_size = servMes.byteSize();
                            TimerStart( 4 );

                            std::cout<<"main.cpp:L779"<<std::endl;

                            servMes.show();

                            pts.sendMessage( servMes );
                            TimerStop( 4 );
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 472 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 473 );
                            }
//                            delete[] servMes.m_positions;
//                            delete[] servMes.m_normals;
//                            delete[] servMes.m_colors;
                            delete originalObject;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 473 );
                            }
//jupiter
                            TimerStop( 1 );
//                          TimerFinish( servMes.m_time_step );
//jupiter
                        } // end of while(DispatchNext)
//#ifndef CPU_VER
//                        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
//                        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
//
//                        int c_count = 0;
//                        int o_count = 0;
//                        for ( int tf = 0; tf < servMes.m_transfer_function_count ; tf++ )
//                        {
//                            for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
//                            {
//                                servMes.m_color_bins[tf][res] = tmp_c_bins[c_count];
//                                c_count++;
//                            }
//
//                            for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
//                            {
//                                servMes.m_opacity_bins[tf][res] = tmp_o_bins[o_count];
//                                o_count++;
//                            }
//                        }
//#endif
                        servMes.m_flag_send_bins = 1;
//                      servMes.m_subpixel_level = param.subpixel_level;
                        servMes.m_message_size = servMes.byteSize();
                        //servMes.m_number_particle = 2;
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        TimerStart( 11 );
                        pts.sendMessage( servMes );
                        TimerStop( 11 );

                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            delete[] servMes.m_color_bins[tf];
                            delete[] servMes.m_opacity_bins[tf];
                        }
                        delete[] servMes.m_color_nbins;
                        delete[] servMes.m_opacity_nbins;
                        servMes.m_transfer_function_count = 0;
                        servMes.m_flag_send_bins = 1;
                        delete[] tmp_c_bins;
                        delete[] tmp_o_bins;

                        timer.stop();
                        std::cout << "Particle File: " << timer.sec() << " [sec/step]" << std::endl;

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_END( 470 );
                        }
                        TimerStop( 10 );
                        TimerFinish( servMes.m_time_step );
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

                        timer_count++;
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_STA( 461 );
                    }

                    // send cltMes to all worker process >>
//                    bsz = clntMes.byteSize();
//#ifndef CPU_VER
//                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
//#endif
//                    buf = new char[bsz];
//                    clntMes.pack( buf );
//#ifndef CPU_VER
//                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
//#endif
//                    delete[] buf;
                    // send cltMes to all worker process <<

                    std::cout << "initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                    if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
                    {

                        std::cout << "sampling method = " << clntMes.m_sampling_method << std::endl;
                        std::cout << "subpixel level = " << clntMes.m_subpixel_level << std::endl;
                        std::cout << "repeat level = " << clntMes.m_repeat_level << std::endl;
                    }
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
                        servMes.m_number_glyph = 0;
                        servMes.m_flag_send_bins = 1;

                        servMes.m_message_size = servMes.byteSize();

                        std::cout<<"main.cpp:L509"<<std::endl;
                        clntMes.show();

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

                        std::cout<<"main.cpp:L526"<<std::endl;
                        clntMes.show();

                        pts.sendMessage( servMes );
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        TimerInitialize();
                        TimerStart( 10 );
                        strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        param.m_sampling_method = clntMes.m_sampling_method;
                        param.m_component_Id = clntMes.m_rendering_id;
                        param.m_crop.setEnable( clntMes.m_enable_crop_region );
                        param.m_crop.set( clntMes.m_crop_region );
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

                        if ( clntMes.m_node_type == 'a' )
                        {
                            useAllNodes = true;
                        }
                        else if ( clntMes.m_node_type == 's' )
                        {
                            useAllNodes = false;
                        }
                        else
                        {
                            assert( false );
                        }

                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                       
                        pm.findGlyphFile();
                        mvpl.m_total_number_subvolumes=1;
                        if ( param.m_crop.isEnabled() )
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size,
                                           param.m_crop.getMinCoord(),
                                           param.m_crop.getMaxCoord() );
                            servMes.m_number_volume_divide = jd.getCountVolumes();
                        }
                        else
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size );
                            servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                        }

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_STA( 470 );
                        }

                        param.m_sampling_step = CalculateSamplingStep( mvpl );

                        VariableRange vr;

                        pm.check();
                        servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                        servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                        if( pm.stepExisted() )
                        {
                            //if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step && pm.getTimeStep() > -1 )
                            if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step )
                            {
                                servMes.m_time_step = clntMes.m_step;
                            }
                            else
                            {
                                servMes.m_time_step = pm.particleStatusFile().getLatestTimeStep();
                                clntMes.m_step = servMes.m_time_step;
                            }
                        }
                        else
                        {
                            std::cout << " no step !!!!!!!!!!!" << std::endl;
                            clntMes.m_step = -1;
                        }

                        std::cout<<"main.cpp:L1319"<<std::endl;
                        //clntMes.show();

                        pts.sendMessage( servMes );

                        timer.start();
               
                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 471 );
                            }

//jupiter start
//                          TimerInitialize();
                            TimerStart( 1 );
                            ParameterFileWriter ppw;
                            ParameterFileReader ppr;

                            //clntMes.show();
                            // 20181226 start　環境変数で指定したパスおよび名前でファイル参照を行う
                            ppw.inputGlyphParameterMessage( clntMes );
                            ppr.readParameterFile( glyphParameterPath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();

                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( glyphParameterPath.c_str() );
                                // 20181226 end
                            }
                            vismodule::PointObject* originalObject = new vismodule::PointObject;
                            vismodule::KVSMLObjectGlyph* originalGlyph = new vismodule::KVSMLObjectGlyph;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
//                            std::string filename( glyphFilePath );
                            std::string filename( particlePath );
                            // 20181226 end
                            filename.append( "_pfi_coords_minmax.txt" );
                            vismodule::File f( filename.c_str()  );

                            if ( f.isExisted() )
                            {
                                // ファイルがある
                                FILE* fp = NULL;
                                fp = fopen( filename.c_str(), "r" );
                                fscanf( fp, "%f %f %f %f %f %f",
                                        &servMes.m_min_object_coord[0],
                                        &servMes.m_min_object_coord[1],
                                        &servMes.m_min_object_coord[2],
                                        &servMes.m_max_object_coord[0],
                                        &servMes.m_max_object_coord[1],
                                        &servMes.m_max_object_coord[2]);
                                if ( fp != NULL ) fclose( fp );
                            }
                            else
                            {
                                servMes.m_min_object_coord[0]=0.f;
                                servMes.m_min_object_coord[1]=0.f;
                                servMes.m_min_object_coord[2]=0.f;
                                servMes.m_max_object_coord[0]=0.1;
                                servMes.m_max_object_coord[1]=0.1;
                                servMes.m_max_object_coord[2]=0.1;
                            }



                            TimerStop( 2 );
                            if( pm.setTimeStep_glyph( clntMes.m_step ) || pm.stepExisted() ) servMes.m_flag_send_bins = 2;
                            else                                 servMes.m_flag_send_bins = 1;
                            if( servMes.m_flag_send_bins == 2)
                            {
                                pm.readGlyphFile();
                                pm.getGlyph( originalGlyph );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            servMes.m_subpixel_level = pm.getSubpixelLevel();
                            
                            servMes.m_number_glyph = originalGlyph->coords().size() / 3;
                            if ( servMes.m_number_glyph > 0 )
                            {
                                servMes.m_glyph_coords = std::make_unique<float[]>(3 * servMes.m_number_glyph);
                                servMes.m_glyph_vectors = std::make_unique<float[]>(3 * servMes.m_number_glyph);
                                servMes.m_glyph_colors = std::make_unique<unsigned char[]>(3 * servMes.m_number_glyph);
                                servMes.m_glyph_sizes = std::make_unique<float[]>(servMes.m_number_glyph);
                            }
                            else
                            {
                                servMes.m_glyph_coords  = NULL;
                                servMes.m_glyph_vectors = NULL;
                                servMes.m_glyph_colors  = NULL;
                                servMes.m_glyph_sizes   = NULL;
                            }
                            for ( int i = 0; i < servMes.m_number_glyph; ++i )
                            {
                                servMes.m_glyph_coords[3 * i + 0] = originalGlyph->coords()[3 * i + 0];
                                servMes.m_glyph_coords[3 * i + 1] = originalGlyph->coords()[3 * i + 1];
                                servMes.m_glyph_coords[3 * i + 2] = originalGlyph->coords()[3 * i + 2];
                                servMes.m_glyph_vectors[3 * i + 0] = originalGlyph->directions()[3 * i + 0];
                                servMes.m_glyph_vectors[3 * i + 1] = originalGlyph->directions()[3 * i + 1];
                                servMes.m_glyph_vectors[3 * i + 2] = originalGlyph->directions()[3 * i + 2];
                                servMes.m_glyph_colors[3 * i + 0] = originalGlyph->colors()[3 * i + 0];
                                servMes.m_glyph_colors[3 * i + 1] = originalGlyph->colors()[3 * i + 1];
                                servMes.m_glyph_colors[3 * i + 2] = originalGlyph->colors()[3 * i + 2];
                                servMes.m_glyph_sizes[i ] = originalGlyph->sizes()[ i ];
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 471 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 472 );
                            }
                            servMes.m_message_size = servMes.byteSize();
                            TimerStart( 4 );

                            std::cout<<"main.cpp:L1497"<<std::endl;

                            servMes.show();
                            pts.sendMessage( servMes );
                            TimerStop( 4 );
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 472 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 473 );
                            }
                            delete originalObject;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 473 );
                            }
                            TimerStop( 1 );
                        } // end of while(DispatchNext)
                        servMes.m_flag_send_bins = 1;
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        TimerStart( 11 );
                        pts.sendMessage( servMes );
                        TimerStop( 11 );
                        timer.stop();
                        std::cout << "Particle File: " << timer.sec() << " [sec/step]" << std::endl;

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_END( 470 );
                        }
                        TimerStop( 10 );
                        TimerFinish( servMes.m_time_step );

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

                }  // end loop of generate_glyph
                else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::send_glyph_flag_false )
                {
                        ParameterFileWriter ppw;
                        ppw.inputGlyphParameterMessage( clntMes );
                        ppw.writeParameterFile( glyphParameterPath.c_str() );
 
                }
                else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::plot_over_line )
                {

                        timer_count++;
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                       VIS_MODULE_TIMER_STA( 461 );
                    }

//                    // send cltMes to all worker process >>
//                    bsz = clntMes.byteSize();
//#ifndef CPU_VER
//                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
//#endif
//                    buf = new char[bsz];
//                    clntMes.pack( buf );
//#ifndef CPU_VER
//                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
//#endif
//                    delete[] buf;
//                    // send cltMes to all worker process <<

                    std::cout << "initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                    if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
                    {

                        std::cout << "sampling method = " << clntMes.m_sampling_method << std::endl;
                        std::cout << "subpixel level = " << clntMes.m_subpixel_level << std::endl;
                        std::cout << "repeat level = " << clntMes.m_repeat_level << std::endl;
                    }
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
                        servMes.m_number_glyph = 0;
                        servMes.m_flag_send_bins = 1;

                        servMes.m_message_size = servMes.byteSize();

                        std::cout<<"main.cpp:L509"<<std::endl;
                        clntMes.show();

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

                        std::cout<<"main.cpp:L526"<<std::endl;
                        clntMes.show();

                        pts.sendMessage( servMes );
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        TimerInitialize();
                        TimerStart( 10 );
                        strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        param.m_sampling_method = clntMes.m_sampling_method;
                        param.m_component_Id = clntMes.m_rendering_id;
                        param.m_crop.setEnable( clntMes.m_enable_crop_region );
                        param.m_crop.set( clntMes.m_crop_region );
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

                        if ( clntMes.m_node_type == 'a' )
                        {
                            useAllNodes = true;
                        }
                        else if ( clntMes.m_node_type == 's' )
                        {
                            useAllNodes = false;
                        }
                        else
                        {
                            assert( false );
                        }

                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                       
                        mvpl.m_total_number_subvolumes=1;
                        if ( param.m_crop.isEnabled() )
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size,
                                           param.m_crop.getMinCoord(),
                                           param.m_crop.getMaxCoord() );
                            servMes.m_number_volume_divide = jd.getCountVolumes();
                        }
                        else
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size );
                            servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                        }

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_STA( 470 );
                        }

                        param.m_sampling_step = CalculateSamplingStep( mvpl );

                        VariableRange vr;

                        pm.check();
                        servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                        servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                        if( pm.stepExisted() )
                        {
                            if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step )
                            {
                                servMes.m_time_step = clntMes.m_step;
                            }
                            else
                            {
                                servMes.m_time_step = pm.particleStatusFile().getLatestTimeStep();
                                clntMes.m_step = servMes.m_time_step;
                            }
                        }
                        else
                        {
                            std::cout << " no step !!!!!!!!!!!" << std::endl;
                            clntMes.m_step = -1;
                        }

                        std::cout<<"main.cpp:L1319"<<std::endl;
                        //clntMes.show();

                        pts.sendMessage( servMes );


                        timer.start();
                        std::cout << __LINE__ << __FUNCTION__ << std::endl;

                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 471 );
                            }

//jupiter start
//                          TimerInitialize();
                            TimerStart( 1 );
                            ParameterFileWriter ppw;
                            ParameterFileReader ppr;

                            //clntMes.show();
                            // 20181226 start　環境変数で指定したパスおよび名前でファイル参照を行う
                            ppw.inputPlotOverLineParameterMessage( clntMes );
                            ppr.readParameterFile( plotOverLineParameterPath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();

                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( plotOverLineParameterPath.c_str() );
                            }


                            vismodule::KVSMLObjectPlotOverLine* originalObject = new vismodule::KVSMLObjectPlotOverLine;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
//                            std::string filename( plotOverLineFilePath );
                            std::string filename( particlePath );
                            // 20181226 end
                            filename.append( "_pfi_coords_minmax.txt" );
                            vismodule::File f( filename.c_str()  );

                            if ( f.isExisted() )
                            {
                                // ファイルがある
                                FILE* fp = NULL;
                                fp = fopen( filename.c_str(), "r" );
                                fscanf( fp, "%f %f %f %f %f %f",
                                        &servMes.m_min_object_coord[0],
                                        &servMes.m_min_object_coord[1],
                                        &servMes.m_min_object_coord[2],
                                        &servMes.m_max_object_coord[0],
                                        &servMes.m_max_object_coord[1],
                                        &servMes.m_max_object_coord[2]);
                                if ( fp != NULL ) fclose( fp );
                            }
                            else
                            {
                                servMes.m_min_object_coord[0]=0.f;
                                servMes.m_min_object_coord[1]=0.f;
                                servMes.m_min_object_coord[2]=0.f;
                                servMes.m_max_object_coord[0]=0.1;
                                servMes.m_max_object_coord[1]=0.1;
                                servMes.m_max_object_coord[2]=0.1;
                            }

                            TimerStop( 2 );
                            if( pm.setTimeStep_pol( clntMes.m_step ) || pm.stepExisted() ) servMes.m_flag_send_bins = 3; //plot over line
                            else                                 servMes.m_flag_send_bins = 1;
                            if( servMes.m_flag_send_bins == 3)
                            {
                                pm.readPlotOverLineFile();
                                pm.getPlotOverLine( originalObject );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            servMes.m_subpixel_level = pm.getSubpixelLevel();
                            
                            int resolution = originalObject->x_axis().size();
                            servMes.m_resolution = resolution;
                            servMes.m_xAxis.clear();
                            servMes.m_line_values.clear();
                            servMes.m_mask.clear();
                            for ( int i = 0; i < resolution; ++i )
                            {
                                servMes.m_xAxis.push_back(  originalObject->x_axis()[i]);
                                servMes.m_line_values.push_back(  originalObject->values_on_line()[i]);
                                if(originalObject->mask()[i]) servMes.m_mask.push_back( 1 );
                                else servMes.m_mask.push_back( 0 );
                            }

                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 471 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 472 );
                            }
                            servMes.m_message_size = servMes.byteSize();
                            TimerStart( 4 );

                            std::cout<<"main.cpp:L1497"<<std::endl;

                            servMes.show();
                            pts.sendMessage( servMes );
                            TimerStop( 4 );
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 472 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                               VIS_MODULE_TIMER_STA( 473 );
                            }
                            delete originalObject;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 473 );
                            }
                            TimerStop( 1 );
                        } // end of while(DispatchNext)
                        servMes.m_flag_send_bins = 1;
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        TimerStart( 11 );
                        pts.sendMessage( servMes );
                        TimerStop( 11 );
                        timer.stop();
                        std::cout << "Particle File: " << timer.sec() << " [sec/step]" << std::endl;

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_END( 470 );
                        }
                        TimerStop( 10 );
                        TimerFinish( servMes.m_time_step );
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
                    

                }  // end loop of plot over line

            } // end of while (pts.good)

            delete servMes.m_camera;
            delete clntMes.m_camera;

//            bsz = -1;
//#ifndef CPU_VER
//            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
//#endif

            pts.termServer();
        }
    }
    if ( param.m_batch == true )
    {
        VIS_MODULE_TIMER_END( 1 );
        VIS_MODULE_TIMER_FIN();
    }
//#ifndef CPU_VER
//    MPI::Finalize();
//#endif
//    return retval;
//#endif
return ;
}
