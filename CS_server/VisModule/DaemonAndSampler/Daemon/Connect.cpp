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
                   initial_step_master(param, clntMes, servMes, mvpl, nan_error,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts,  transfunc_creator, timer_count );

                } // end of change PFI
                //else
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
                {

                    generate_particle_master(param, clntMes, servMes, mvpl, nan_error,
#ifndef CPU_VER
                           jc, 
#endif                           
                           jd, pts, transfunc_creator, timer_count, clntMes.m_initialize_parameter );

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
