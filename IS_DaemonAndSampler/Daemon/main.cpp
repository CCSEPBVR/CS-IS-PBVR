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

void setDefalutTransferFunction(jpv::ParticleTransferServerMessage* servMes, const int tf_number )
{
    servMes->m_transfer_function.clear();
    servMes->m_transfer_function.resize(tf_number);
    int TF_resolution = 256;
    servMes->m_color_transfer_function_synthesis = "C1"; 
    servMes->m_opacity_transfer_function_synthesis = "O1"; 


    // set defalut opacity & color  parameter
    std::vector<float> o_table ={0, 0.00392157, 0.00784314, 0.0117647, 0.0156863, 0.0196078, 0.0235294, 0.027451, 0.0313726, 0.0352941, 0.0392157, 0.0431373, 0.0470588, 0.0509804, 0.054902, 0.0588235, 0.0627451, 0.0666667, 0.0705882, 0.0745098, 0.0784314, 0.0823529, 0.0862745, 0.0901961, 0.0941177, 0.0980392, 0.101961, 0.105882, 0.109804, 0.113725, 0.117647, 0.121569, 0.12549, 0.129412, 0.133333, 0.137255, 0.141176, 0.145098, 0.14902, 0.152941, 0.156863, 0.160784, 0.164706, 0.168627, 0.172549, 0.176471, 0.180392, 0.184314, 0.188235, 0.192157, 0.196078, 0.2, 0.203922, 0.207843, 0.211765, 0.215686, 0.219608, 0.223529, 0.227451, 0.231373, 0.235294, 0.239216, 0.243137, 0.247059, 0.25098, 0.254902, 0.258824, 0.262745, 0.266667, 0.270588, 0.27451, 0.278431, 0.282353, 0.286275, 0.290196, 0.294118, 0.298039, 0.301961, 0.305882, 0.309804, 0.313726, 0.317647, 0.321569, 0.32549, 0.329412, 0.333333, 0.337255, 0.341176, 0.345098, 0.34902, 0.352941, 0.356863, 0.360784, 0.364706, 0.368627, 0.372549, 0.376471, 0.380392, 0.384314, 0.388235, 0.392157, 0.396078, 0.4, 0.403922, 0.407843, 0.411765, 0.415686, 0.419608, 0.423529, 0.427451, 0.431373, 0.435294, 0.439216, 0.443137, 0.447059, 0.45098, 0.454902, 0.458824, 0.462745, 0.466667, 0.470588, 0.47451, 0.478431, 0.482353, 0.486275, 0.490196, 0.494118, 0.498039, 0.501961, 0.505882, 0.509804, 0.513726, 0.517647, 0.521569, 0.52549, 0.529412, 0.533333, 0.537255, 0.541176, 0.545098, 0.54902, 0.552941, 0.556863, 0.560784, 0.564706, 0.568627, 0.572549, 0.576471, 0.580392, 0.584314, 0.588235, 0.592157, 0.596078, 0.6, 0.603922, 0.607843, 0.611765, 0.615686, 0.619608, 0.623529, 0.627451, 0.631373, 0.635294, 0.639216, 0.643137, 0.647059, 0.65098, 0.654902, 0.658824, 0.662745, 0.666667, 0.670588, 0.67451, 0.678431, 0.682353, 0.686275, 0.690196, 0.694118, 0.698039, 0.701961, 0.705882, 0.709804, 0.713726, 0.717647, 0.721569, 0.72549, 0.729412, 0.733333, 0.737255, 0.741176, 0.745098, 0.74902, 0.752941, 0.756863, 0.760784, 0.764706, 0.768628, 0.772549, 0.776471, 0.780392, 0.784314, 0.788235, 0.792157, 0.796079, 0.8, 0.803922, 0.807843, 0.811765, 0.815686, 0.819608, 0.823529, 0.827451, 0.831373, 0.835294, 0.839216, 0.843137, 0.847059, 0.85098, 0.854902, 0.858824, 0.862745, 0.866667, 0.870588, 0.87451, 0.878431, 0.882353, 0.886275, 0.890196, 0.894118, 0.898039, 0.901961, 0.905882, 0.909804, 0.913726, 0.917647, 0.921569, 0.92549, 0.929412, 0.933333, 0.937255, 0.941177, 0.945098, 0.94902, 0.952941, 0.956863, 0.960784, 0.964706, 0.968628, 0.972549, 0.976471, 0.980392, 0.984314, 0.988235, 0.992157, 0.996078, 1};

    std::vector<kvs::UInt8> c_table = {59,75,192,60,77,194,61,79,195,63,80,197,64,82,198,65,84,200,66,86,201,67,87,203,69,89,204,70,91,206,71,93,207,72,94,209,74,96,210,75,98,212,76,99,213,77,101,214,79,103,216,80,105,217,81,106,218,82,108,220,84,110,221,85,111,222,86,113,223,88,115,224,89,116,226,90,118,227,91,120,228,93,121,229,94,123,230,95,125,231,97,126,232,98,128,233,99,129,234,101,131,235,102,133,236,103,134,237,105,136,238,106,137,239,107,139,240,109,140,241,110,142,242,111,144,242,113,145,243,114,147,244,115,148,245,117,150,245,118,151,246,120,152,247,121,154,247,122,155,248,124,157,249,125,158,249,126,160,250,128,161,250,129,162,251,131,164,251,132,165,252,133,167,252,135,168,253,136,169,253,137,171,253,139,172,254,140,173,254,142,174,254,143,176,254,144,177,255,146,178,255,147,179,255,149,181,255,150,182,255,151,183,255,153,184,255,154,185,255,155,186,255,157,187,255,158,189,255,159,190,255,161,191,255,162,192,255,164,193,255,165,194,255,166,195,255,168,196,255,169,197,254,170,198,254,172,199,254,173,200,254,174,200,253,175,201,253,177,202,253,178,203,252,179,204,252,181,205,251,182,205,251,183,206,250,184,207,250,186,208,249,187,208,249,188,209,248,189,210,248,191,211,247,192,211,246,193,212,246,194,212,245,196,213,244,197,213,243,198,214,243,199,215,242,200,215,241,201,216,240,202,216,239,204,216,238,205,217,237,206,217,237,207,218,236,208,218,235,209,218,234,210,219,233,211,219,232,212,219,230,213,219,229,214,220,228,215,220,227,216,220,226,217,220,225,218,220,224,219,221,222,220,221,221,221,220,220,222,220,219,223,219,217,224,219,216,226,218,214,227,218,213,227,217,211,228,217,210,229,216,208,230,216,207,231,215,205,232,214,204,233,214,202,234,213,201,234,212,199,235,211,198,236,211,196,237,210,195,237,209,193,238,208,192,239,207,190,239,206,189,240,206,187,240,205,186,241,204,184,242,203,182,242,202,181,242,201,179,243,200,178,243,199,176,244,198,175,244,197,173,244,196,171,245,194,170,245,193,168,245,192,167,246,191,165,246,190,163,246,189,162,246,188,160,246,186,159,247,185,157,247,184,156,247,183,154,247,181,152,247,180,151,247,179,149,247,177,148,247,176,146,247,175,144,247,173,143,247,172,141,247,170,140,247,169,138,246,168,136,246,166,135,246,165,133,246,163,132,246,162,130,245,160,129,245,159,127,245,157,125,244,156,124,244,154,122,244,152,121,243,151,119,243,149,118,242,148,116,242,146,115,241,144,113,241,143,112,240,141,110,240,139,109,239,137,107,238,136,106,238,134,104,237,132,103,237,131,101,236,129,100,235,127,98,234,125,97,234,123,95,233,121,94,232,120,92,231,118,91,230,116,89,230,114,88,229,112,87,228,110,85,227,108,84,226,106,82,225,104,81,224,102,80,223,100,78,222,98,77,221,96,76,220,94,74,219,92,73,218,90,72,217,88,70,216,86,69,214,84,68,213,82,66,212,80,65,211,77,64,210,75,62,209,73,61,207,71,60,206,68,59,205,66,57,203,64,56,202,61,55,201,59,54,199,56,53,198,54,51,197,51,50,195,48,49,194,46,48,192,43,47,191,40,46,190,37,45,188,33,43,187,30,42,185,26,41,184,21,40,182,16,39,181,10,38,179,3,37};


    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream cc, qq, tt;
        cc << "C" << i + 1;
        qq << "q" << i + 1;
        tt << "t" << i + 1;  
        servMes->m_transfer_function[i].m_name          = tt.str();
        servMes->m_transfer_function[i].m_color_variable       = qq.str();
        servMes->m_transfer_function[i].m_color_variable_min   = servMes->m_variable_range.min( tt.str() + "_var_c" );
        servMes->m_transfer_function[i].m_color_variable_max   = servMes->m_variable_range.max( tt.str() + "_var_c" ); 
        servMes->m_transfer_function[i].m_opacity_variable     = qq.str();
        servMes->m_transfer_function[i].m_opacity_variable_min = servMes->m_variable_range.min( tt.str() + "_var_o" );
        servMes->m_transfer_function[i].m_opacity_variable_max = servMes->m_variable_range.max( tt.str() + "_var_o" ); 
        servMes->m_transfer_function[i].m_resolution           = TF_resolution;
        servMes->m_transfer_function[i].m_equation_red         = ""; 
        servMes->m_transfer_function[i].m_equation_green       = ""; 
        servMes->m_transfer_function[i].m_equation_blue        = ""; 
        servMes->m_transfer_function[i].m_equation_opacity     = "";
        kvs::ValueArray<kvs::UInt8> cc_table(c_table);
        kvs::ValueArray<float> oo_table(o_table);
        kvs::ColorMap color_map( cc_table    , servMes->m_variable_range.min( tt.str() + "_var_c" ), servMes->m_variable_range.max( tt.str() + "_var_c" )  );
        kvs::OpacityMap opacity_map( oo_table, servMes->m_variable_range.min( tt.str() + "_var_o" ), servMes->m_variable_range.max( tt.str() + "_var_o" )  );
        servMes->m_transfer_function[i].setColorMap( color_map );
        servMes->m_transfer_function[i].setOpacityMap( opacity_map );

        servMes->m_transfer_function[i].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
    }

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
            std::string statePath = visParamDir + "state.txt";
            std::string historyPath = visParamDir + "history";
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
                //clntMes.show();
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
                servMes.m_last_step = fil.total_endSteps;
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

//// read minmax & histrogram 
                ParticleMonitor pm( particlePath, statePath.c_str(), historyPath.c_str() );
                pm.check();
                if( pm.stepExisted() )
                {
                    //std::cout << "pm.particleStatusFile().getLatestTimeStep() = " << pm.particleStatusFile().getLatestTimeStep() <<std::endl;
                    pm.setTimeStep(pm.particleStatusFile().getLatestTimeStep());
                    //pm.setTimeStep(0);
                }
                else
                {
                    pm.setTimeStep(0);
                }
                pm.readParticleHistoryFile();
                range = pm.particleHistoryFile().variableRange();
                const int tf_number = pm.particleHistoryFile().colorHistogramArray().size();
                servMes.m_transfer_function_count =       tf_number;//TF_COUNT
                servMes.m_color_nbins   = new kvs::UInt64[tf_number];
                servMes.m_opacity_nbins = new kvs::UInt64[tf_number];
                servMes.m_color_bins.resize(   tf_number );
                servMes.m_opacity_bins.resize( tf_number );
                for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                {
                    servMes.m_color_nbins[tf] = DEFAULT_NBINS;
                    servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
                    servMes.m_color_bins[tf]   =  new kvs::UInt64[ servMes.m_color_nbins[tf] ];
                    servMes.m_opacity_bins[tf] =  new kvs::UInt64[ servMes.m_opacity_nbins[tf] ];
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
//                nm.setFileName( tfFilePath.c_str() );
//                nm.write();
//                std::ofstream ofs;
//                //ofs.open( "jupiter.tf" , std::ios::out | std::ios::app  );
//                ofs.open( tfFilePath.c_str() , std::ios::out | std::ios::app  );
//                ofs << "END_PARAMETER_FILE=SUCCESS" << std::endl;
//                ofs.close();
//                // 20181226 end
//                
//                ParameterFileWriter ppw;
//                ppw.inputMessage( clntMes );
//                std::ifstream file(tfFilePath.c_str());
//                if(!file)
//                {
//                    ppw.writeParameterFile( tfFilePath.c_str() );
//                }
//                file.close();
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

                servMes.m_message_size = servMes.byteSize();

                std::cout<<"main.cpp:L366"<<std::endl;
                servMes.show();

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
//            std::string statePath = visParamDir + "state.txt";
//            std::string historyPath = visParamDir + "history";
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
//                clntMes.show();
                std::cout<<"ptss="<<ptss<<std::endl;

                if ( ptss == -1 ) break;
                /* 140319 for client stop by Ctrl+c */
                signal( SIGABRT, sig_handler );
                signal( SIGTERM, sig_handler );
                signal( SIGINT, sig_handler ); /* SIGINT is invalid here, because mpiexec uses it. */

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
                        clntMes.m_input_directory = param.input_data_base;
                    }
                }

                std::cout << "Receive message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                //initParam -1:空ソケットの送信, -2:daemonを終了, それ以外:粒子データの送信
                if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::empty )
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
                else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
                {
                    //終了する
                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
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

                    std::cout << "initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                    if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::empty )
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

                            //clntMes.show();
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

                            servMes.show();
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
                } // end of initParam =1
                else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::export_TFfile )
                {
                    std::ifstream file(tfFilePath.c_str());
                    ParameterFileWriter ppw;
                    ppw.inputMessage( clntMes );
                    ppw.writeParameterFile( tfFilePath.c_str() );
                    file.close();
                }
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
