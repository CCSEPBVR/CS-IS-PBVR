/*****************************************************************************/
/**
 *  main.cpp
 *  CPU Server Program without CUDA, without KVS, without OpenGL.
 */
/*****************************************************************************/

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include "PointObject.h"
#include <kvs/CommandLine>
#include <kvs/Camera>
#include "TransferFunction.h"
#include <kvs/Matrix33>
#include <kvs/RotationMatrix33>

#include "common.h"

#include "Argument.h"

#include <kvs/AVSField>
#include <kvs/Timer>
#include "KVSMLObjectPointWriter.h"
//#include "KVSMLObjectPointMPIWriter.h"
#include "JobDispatcher.h"
#ifndef CPU_VER
#include "JobCollector.h"
#endif
#include "FilterInfo.h"
#include "ParamInfo.h"
#ifndef CPU_VER
#include "mpi.h"
#endif
#ifdef KMATH
#include <kmath_random.h>
#endif

#include <cassert>
#include <signal.h> /* 140319 for client stop by Ctrl+c */
#include <sys/stat.h>
#include <kvs/File>

#include "VariableRange.h"

#include "timer_simple.h"

#include <kvs/Compiler>
#ifdef KVS_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

#include "ParticleMonitor.h"
#include "ParameterFileWriter.h" 
#include "ParameterFileReader.h" 
#include "Timer.h" 

bool useAllNodes = true;
#ifdef KMATH
KMATH_Random km_random;
#endif

inline const size_t GetRevisedSubpixelLevel(
    const size_t subpixel_level,
    const size_t repetition_level )
{
    return ( static_cast<size_t>( subpixel_level * std::sqrt( ( double )repetition_level ) + 0.5f ) );
}

/* 140319 for client stop by Ctrl+c */
bool SigServer = false;
void sig_handler( int sig )
{
    SigServer = true;
}
/* 140319 for client stop by Ctrl+c */

bool isDirectory( std::string directory_path )
{
#if defined ( WIN32 )
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA( directory_path.c_str(), &find_data );
    if ( hFind == INVALID_HANDLE_VALUE )
    {
        return ( false );
    }
    return ( ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 );
#else
    struct stat filestat;
    if ( stat( directory_path.c_str(), &filestat ) )
    {
        return ( false );
    }
    return ( filestat.st_mode & S_IFDIR );
#endif
}

inline float CalculateSamplingStep( const FilterInfoLst& fil )
{
    float max_coord_length = kvs::Math::Max<float>( fil.total_maxObjectCoord.x() - fil.total_minObjectCoord.x(),
                                                    fil.total_maxObjectCoord.y() - fil.total_minObjectCoord.y(),
                                                    fil.total_maxObjectCoord.z() - fil.total_minObjectCoord.z() );
    return 0.1 * max_coord_length;
}

int main( int argc, char** argv )
{
#ifndef CPU_VER
    MPI::Init( argc, argv );
#endif
    TIMER_INIT();
    TIMER_STA( 1 );

    kvs::Timer timer( kvs::Timer::Start );


#ifdef KMATH
#ifndef CPU_VER
    km_random.init( MPI_COMM_WORLD );
#else
    km_random.init();
#endif
    km_random.seed( 1 );
#endif
    Argument param( argc, argv );
    FilterInfoLst fil;
    //2018 kawamura comment out
    //TransferFunctionSynthesizerCreator transfunc_creator;

    kvs::Camera camera;
    int retval = 0;
    int mpi_rank = 0;

    //std::vector<PointObjectCreator> point_creator_lst;
    //PointObjectCreator point_creator;

    //pbvr::PointObject* object = NULL;
    std::string output, outdir;
    std::string pout = "PARTICLE_OUTDIR";
    std::string prfx = "PARTICLE_SERVER_PREFIX";


#ifndef CPU_VER
    int rank = MPI::COMM_WORLD.Get_rank();
#else
    int rank = 0;
#endif

    if ( param.batch == true )
    {
        //デーモンツールでは param.batch == false
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
        kvs::UInt64* tmp_c_bins;
        kvs::UInt64* tmp_o_bins;

        if ( rank > 0 )
        {
            //デーモンツールはシングルプロセス rank == 0
        } // end of if( rank > 0 )
        else                    // rank == 0
        {
            //--------------------- MASTER --------------------
            int ptss;

#if 0
//jupiter start
            std::string jupiter_prefix;
//jupiter end
#else
            // 20181226 start
            // 環境変数からパスを指定する
            // 粒子データ：$PARTICLE_DIR　粒子データの接頭辞はInSituLib内で固定のため、t_で固定する
            // 可視化パラメータ：$VIS_PARAM_DIR
            // tfファイル：$TF_NAME（未設定の場合、default.tf）
            std::string particlePath;
            std::string visParamDir;
            std::string tfFilePath;
            std::string tfFilePath_old;

            const char *envBuf = NULL;
            envBuf = std::getenv( "PARTICLE_DIR" );
            if (envBuf == NULL) {
                particlePath = "./t";
            }
            else {
                particlePath = envBuf;
                if (particlePath[particlePath.size() - 1] != '/') {
                    particlePath += "/t";
                }
                else {
                    particlePath += "t";
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
            if (envBuf == NULL) {
                tfFilePath += "default.tf";
                tfFilePath_old += "default_old.tf";
            }
            else {
                tfFilePath += envBuf;
                tfFilePath += ".tf";
                tfFilePath_old += envBuf;
                tfFilePath_old += "_old.tf";
            }
            // 20181226 end
#endif


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
            ptss = pts.initializeServer( param.port );//デフォルトparam.port:60000


            timer.stop();
            std::cout << "Initialize: " << timer.sec() << " [sec/step]" << std::endl;


            if ( ptss == -1 )
            {
                bsz = -1;
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
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

                servMes.m_camera = new kvs::Camera();
                clntMes.m_camera = new kvs::Camera();
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
                param.input_data_base = "dummy";

                std::cout<<"main.cpp:L221"<<std::endl;
                clntMes.show();
//jupiter end

/* 2018 kawamura comment out
                std::string pflfile, pfifile;
                if ( param.input_data_base.substr( param.input_data_base.size() - 3 ) == "pfl" )
                {
                    pflfile = param.input_data_base;
                    kvs::File pfl( pflfile );
                    if ( pfl.isExisted() )
                    {
                        fil.LoadPFL( pflfile );
                    }
                }
                else
                {
                    pfifile = param.input_data_base + ".pfi";
                    kvs::File pfi( pfifile );
                    pflfile = param.input_data_base + ".pfl";
                    kvs::File pfl( pflfile );
                    if ( pfl.isExisted() )
                    {
                        fil.LoadPFL( pflfile );
                    }
                    else if ( pfi.isExisted() )
                    {
                        fil.LoadPFL( pfifile );
                    }
                }

                if ( fil.m_list.size() > 0 )
                {
                    point_creator_lst.clear();
                    for ( int idx = 0; idx < fil.m_list.size(); idx++ )
                    {
                        PointObjectCreator point_creator;
                        if ( param.GT5D == true ) point_creator.setGT5D();
                        point_creator.setFilterInfo( fil.m_list[idx] );
                        point_creator_lst.push_back( point_creator );
                    }

                    transfunc_creator.setFilterInfo( fil.m_list[0] );

                    std::cout << " start step = "         << fil.total_staSteps
                              << " end step = "           << fil.total_endSteps
                              << " time step = "          << fil.total_numSteps
                              << " subvolume division = " << fil.total_numSubVolumes
                              << std::endl;

                }
*/
//              else
//              {
//                  std::cerr << "Error: pfifile (" << pfifile << ") nor pflfile ("
//                            << pflfile << ") doesn't exist" << std::endl;
//                  bsz = -1;
//#ifndef CPU_VER
//                  MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
//#endif
//#ifdef KMATH
//                  km_random.finalize();
//#endif
//#ifndef CPU_VER
//                  MPI::Finalize();
//#endif
//                  return 0;
//              }

/* 2018 kawamura comment out
                transfunc_creator.setProtocol( clntMes );
                TransferFunctionSynthesizer* tfs = transfunc_creator.create();
              //VariableRange range = RangeEstimater::Estimation( 0, fil.m_list[0], *tfs );
                delete tfs;
*/
                VariableRange range;

                strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                servMes.m_transfer_function_count = 0;
                servMes.m_number_volume_divide = fil.total_numSubVolumes;
                servMes.m_time_step = fil.total_staSteps;
                servMes.m_start_step = fil.total_staSteps;
                servMes.m_end_step = fil.total_endSteps;
                servMes.m_number_step = fil.total_numSteps;
//jupiter start
#if 0
                servMes.minObjectCoord[0] = fil.total_minObjectCoord[0];
                servMes.minObjectCoord[1] = fil.total_minObjectCoord[1];
                servMes.minObjectCoord[2] = fil.total_minObjectCoord[2];
                servMes.maxObjectCoord[0] = fil.total_maxObjectCoord[0];
                servMes.maxObjectCoord[1] = fil.total_maxObjectCoord[1];
                servMes.maxObjectCoord[2] = fil.total_maxObjectCoord[2];
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
                kvs::File f( filename.c_str()  );
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
#endif
//jupiter end
                servMes.m_min_value = fil.total_minValue;
                servMes.m_max_value = fil.total_maxValue;
                servMes.m_number_nodes = fil.total_numNodes;
                servMes.m_number_elements = fil.total_numElements;
              //servMes.elemType = fil.m_list[0].elemType;
              //servMes.fileType = fil.m_list[0].fileType;
              //servMes.numIngredients = fil.m_list[0].numIngredients;
                servMes.m_variable_range = range;
                servMes.m_flag_send_bins = 1;

                // 20181226 start  環境変数で指定したパスおよび名前でファイル参照を行う
                //初期化 : jupiter_old.tfを読む
                ParameterFileReader ppr;
                //ppr.readParameterFile("jupiter_old.tf");
                ppr.readParameterFile( tfFilePath_old.c_str() );
                NameListFile nm = ppr.getNameListFile();
                //nm.setFileName("jupiter.tf");
                nm.setFileName( tfFilePath.c_str() );
                nm.write();
                std::ofstream ofs;
                //ofs.open( "jupiter.tf" , std::ios::out | std::ios::app  );
                ofs.open( tfFilePath.c_str() , std::ios::out | std::ios::app  );
                ofs << "END_PARAMETER_FILE=SUCCESS" << std::endl;
                ofs.close();
                // 20181226 end
                
<<<<<<< HEAD

                ParameterFileWriter ppw;
                ppw.inputMessage( clntMes );
                std::ifstream file(tfFilePath.c_str());
                if(!file)
                {
                    ppw.writeParameterFile( tfFilePath.c_str() );
                }
=======
                ParameterFileWriter ppw;
                ppw.inputMessage( clntMes );
                ppw.writeParameterFile( tfFilePath.c_str() );
                
>>>>>>> upstream/feature/CSPBVR_Initprotcol_rev
                //最初の送信(daemon->client)
                //jupiter_old.tfの内容をクライアントに送信
                ppr.outputMessage( &servMes );
                servMes.m_message_size = servMes.byteSize();

                std::cout<<"main.cpp:L366"<<std::endl;
                clntMes.show();

                pts.sendMessage( servMes );
                delete servMes.m_camera;
                delete clntMes.m_camera;
            }// end of init process 初期化終了

            jpv::ParticleTransferServerMessage servMes;
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new kvs::Camera();
            servMes.m_camera = new kvs::Camera();
            // 20181226 start
            // stateおよびhistory用に、環境変数から指定されたパスをもとにファイルパスを作成
            std::string statePath = visParamDir + "state.txt";
            std::string historyPath = visParamDir + "history";
            //ParticleMonitor pm( jupiter_prefix,"state.txt", "history" );
            ParticleMonitor pm( particlePath, statePath.c_str(), historyPath.c_str() );
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
                signal( SIGABRT, sig_handler );
                signal( SIGTERM, sig_handler );
                signal( SIGINT, sig_handler ); /* SIGINT is invalid here, because mpiexec uses it. */

                if ( SigServer )
                {
                    clntMes.m_initialize_parameter = -2;
                    std::cout << "*** SigServer" << clntMes.m_initialize_parameter << std::endl;
                }
                else
                {
                    /* 140319 for client stop by Ctrl+c */
                    if ( clntMes.m_initialize_parameter != -3 )
                    {
                        clntMes.m_input_directory = param.input_data_base;
                    }
                }

                std::cout << "Receive message initParam = " << clntMes.m_initialize_parameter << std::endl;
                //initParam -1:空ソケットの送信, -2:daemonを終了, それ以外:粒子データの送信
                if ( clntMes.m_initialize_parameter == -1 )
                {
                    //ほぼ空のソケットを送信する
                    strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
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
                else if ( clntMes.m_initialize_parameter == -2 )
                {
                    //終了する
                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
                    servMes.m_flag_send_bins = 1;
                    servMes.m_transfer_function_count = 0;

                    servMes.m_message_size = servMes.byteSize();

                    std::cout<<"main.cpp:L439"<<std::endl;
                    clntMes.show();

                    pts.sendMessage( servMes );
                    break;
                    //whileループを抜けてpts.terminateを実行
                }
                else if ( clntMes.m_initialize_parameter == -3 ) // change PFI file.
                {
                } // end of change PFI
                else
                {
                    timer_count++;
                    if ( timer_count <= TIMER_COUNT_NUM )
                    {
                        TIMER_STA( 461 );
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

                    std::cout << "initParam = " << clntMes.m_initialize_parameter << std::endl;
                    if ( clntMes.m_initialize_parameter == 1 )
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
                        std::cout << "endTime = " << clntMes.m_end_time << std::endl;
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
                        param.sampling_method = clntMes.m_sampling_method;
                        param.component_Id = clntMes.m_rendering_id;
                        param.crop.set_enable( clntMes.m_enable_crop_region );
                        param.crop.set( clntMes.m_crop_region );
                        param.particle_limit = clntMes.m_particle_limit;
                        param.particle_density = clntMes.m_particle_density;


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

                        if ( !param.hasOption( "L" ) ) param.latency_threshold = -1.0;
                        
                        fil.total_numSubVolumes=1;
                        if ( param.crop.isenabled() )
                        {
                            jd.Initialize( clntMes.m_step, clntMes.m_step, fil.total_numSubVolumes,
                                           fil.total_minSubVolumeCoord,
                                           fil.total_maxSubVolumeCoord,
                                           param.latency_threshold, param.jid_pack_size,
                                           param.crop.get_min_coord(),
                                           param.crop.get_max_coord() );
                            servMes.m_number_volume_divide = jd.GetCountVolumes();
                        }
                        else
                        {
                            jd.Initialize( clntMes.m_step, clntMes.m_step, fil.total_numSubVolumes,
                                           fil.total_minSubVolumeCoord,
                                           fil.total_maxSubVolumeCoord,
                                           param.latency_threshold, param.jid_pack_size );
                            servMes.m_number_volume_divide = fil.total_numSubVolumes;
                        }

                        if ( timer_count <= TIMER_COUNT_NUM )
                        {
                            TIMER_STA( 470 );
                        }

                        param.sampling_step = CalculateSamplingStep( fil );
//                      param.subpixel_level = CalculateSubpixelLevel( param, fil, *clntMes.m_camera );

                        VariableRange vr;

                        pm.check();
                        servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                        servMes.m_end_step = pm.particleStatusFile().getLatestTimeStep();
                        if( pm.stepExisted() )
                        {
                            if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_end_step && pm.getTimeStep() > -1 )
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
                            clntMes.m_step = -1;
                        }

                        std::cout<<"main.cpp:L614"<<std::endl;
                        servMes.show();

                        pts.sendMessage( servMes );


                        timer.start();

                        servMes.m_transfer_function_count = clntMes.m_transfer_function.size();//TF_COUNT
                        servMes.m_color_nbins = new kvs::UInt64[clntMes.m_transfer_function.size()];
                        servMes.m_opacity_nbins = new kvs::UInt64[clntMes.m_transfer_function.size()];

                        servMes.m_color_bins.resize( clntMes.m_transfer_function.size() );
                        servMes.m_opacity_bins.resize( clntMes.m_transfer_function.size() );

                        c_bins_size = 0;
                        o_bins_size = 0;

                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            servMes.m_color_nbins[tf] = DEFAULT_NBINS;
                            servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
                            servMes.m_color_bins[tf] =  new kvs::UInt64[ servMes.m_color_nbins[tf] ];
                            servMes.m_opacity_bins[tf] =  new kvs::UInt64[ servMes.m_opacity_nbins[tf] ];
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

                        tmp_c_bins = new kvs::UInt64[c_bins_size];
                        tmp_o_bins = new kvs::UInt64[o_bins_size];

                        for ( int tf = 0; tf < c_bins_size; tf++ )
                        {
                            tmp_c_bins[tf] = 0;
                        }

                        for ( int tf = 0; tf < o_bins_size; tf++ )
                        {
                            tmp_o_bins[tf] = 0;
                        }


                        while ( jd.DispatchNext( wid, &st, &vl ) )
                        {
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_STA( 471 );
                            }

//jupiter start
//                          TimerInitialize();
                            TimerStart( 1 );
                            ParameterFileWriter ppw;
                            ParameterFileReader ppr;

                            clntMes.show();
                            // 20181226 start　環境変数で指定したパスおよび名前でファイル参照を行う
                            ppw.inputMessage( clntMes );
                            //ppr.readParameterFile("jupiter_old.tf");
                            ppr.readParameterFile( tfFilePath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();
                            if( nm1 != nm2 )
                            {
                                //ppw.writeParameterFile("jupiter.tf");
                                ppw.writeParameterFile( tfFilePath.c_str() );
                            }
                            // 20181226 end

                            pbvr::PointObject* originalObject = new pbvr::PointObject;
//                          ParticleMonitor pm( "/Users/admin/Work_noda/2015_PBVR/03_testrun/v1.08_test/pout/case","" );
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
                            //std::string filename( jupiter_prefix );
                            std::string filename( particlePath );
                            // 20181226 end
                            filename.append( "_pfi_coords_minmax.txt" );
                            kvs::File f( filename.c_str()  );

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

                            TimerStop( 2 );
                            if( pm.setTimeStep( clntMes.m_step ) ) servMes.m_flag_send_bins = 0;
                            else                                 servMes.m_flag_send_bins = 1;
                            if( servMes.m_flag_send_bins == 0)
                            {
                                pm.readParticleHistoryFile();
                                pm.readParticleFile();
                                pm.getParticle( originalObject );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            servMes.m_subpixel_level = pm.getSubpixelLevel();
                            vr = pm.particleHistoryFile().variableRange();

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
                            //pbvr::PointObject* object = originalObject;

                            //if ( originalObject != object ) delete originalObject;
                            servMes.m_number_particle = originalObject->coords().size() / 3;
                            if ( servMes.m_number_particle > 0 )
                            {
//                                servMes.m_positions = new float[3 * servMes.m_number_particle];
//                                servMes.m_normals = new float[3 * servMes.m_number_particle];
//                                servMes.m_colors = new unsigned char[3 * servMes.m_number_particle];
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
                            servMes.m_variable_range = vr;
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_END( 471 );
                            }
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_STA( 472 );
                            }
                            servMes.m_message_size = servMes.byteSize();
                            TimerStart( 4 );

                            std::cout<<"main.cpp:L779"<<std::endl;
                            clntMes.show();

                            pts.sendMessage( servMes );
                            TimerStop( 4 );
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_END( 472 );
                            }
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_STA( 473 );
                            }
//                            delete[] servMes.m_positions;
//                            delete[] servMes.m_normals;
//                            delete[] servMes.m_colors;
                            delete originalObject;
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_END( 473 );
                            }
//jupiter
                            TimerStop( 1 );
//                          TimerFinish( servMes.m_time_step );
//jupiter
                        } // end of while(DispatchNext)
#ifndef CPU_VER
                        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
                        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );

                        int c_count = 0;
                        int o_count = 0;
                        for ( int tf = 0; tf < servMes.m_transfer_function_count ; tf++ )
                        {
                            for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                            {
                                servMes.m_color_bins[tf][res] = tmp_c_bins[c_count];
                                c_count++;
                            }

                            for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                            {
                                servMes.m_opacity_bins[tf][res] = tmp_o_bins[o_count];
                                o_count++;
                            }
                        }
#endif
                        servMes.m_flag_send_bins = 1;
//                      servMes.m_subpixel_level = param.subpixel_level;
                        servMes.m_message_size = servMes.byteSize();
                        //servMes.m_number_particle = 2;
                        servMes.m_number_particle = 0;
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



                        if ( timer_count <= TIMER_COUNT_NUM )
                        {
                            TIMER_END( 470 );
                        }
                        TimerStop( 10 );
                        TimerFinish( servMes.m_time_step );
                    } // end of timeParam == 2
                    else
                    {
                        break;
                    }
                    if ( timer_count <= TIMER_COUNT_NUM )
                    {
                        TIMER_END( 461 );
                    }
                    if ( timer_count == TIMER_COUNT_NUM )
                    {
                        TIMER_END( 1 );
                        TIMER_FIN();
                    }
                } // end of initParam >= 0
            } // end of while (pts.good)

            delete servMes.m_camera;
            delete clntMes.m_camera;

            bsz = -1;
#ifndef CPU_VER
            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

            pts.termServer();
        }
    }
#ifdef KMATH
    km_random.finalize();
#endif
    if ( param.batch == true )
    {
        TIMER_END( 1 );
        TIMER_FIN();
    }
#ifndef CPU_VER
    MPI::Finalize();
#endif
    return retval;
}
