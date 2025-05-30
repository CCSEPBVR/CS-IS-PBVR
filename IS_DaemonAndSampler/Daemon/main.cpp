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

//#include <kvs/GlyphObject>
#include "KVSMLObjectGlyph.h"

#include "ParticleMonitor.h"
#include "ParameterFileWriter.h" 
#include "ParameterFileReader.h" 
#include "Timer.h" 

bool useAllNodes = true;

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

    std::vector<kvs::UInt8> c_table = {5,48,97,6,50,100,7,52,102,8,54,105,9,56,108,10,58,111,11,60,114,12,62,116,14,64,119,15,66,122,16,68,125,17,70,128,18,72,131,19,74,134,20,76,136,21,78,139,22,80,142,23,83,145,24,85,148,25,87,151,27,89,154,28,91,157,29,93,160,30,95,163,31,98,166,32,100,169,33,102,172,35,104,173,37,105,174,38,107,175,40,109,176,41,111,177,43,113,178,45,114,178,46,116,179,47,118,180,49,120,181,50,121,182,51,123,183,53,125,184,54,127,185,55,129,186,57,130,187,58,132,188,59,134,189,60,136,189,61,138,190,63,140,191,64,141,192,65,143,193,66,145,194,67,147,195,71,149,196,74,151,197,78,153,198,81,154,199,85,156,200,88,158,201,91,160,202,95,162,203,98,164,204,101,166,205,104,168,206,107,170,207,110,172,209,113,174,210,116,175,211,118,177,212,121,179,213,124,181,214,127,183,215,130,185,216,132,187,217,135,189,218,138,191,219,141,193,220,143,195,221,146,197,222,149,198,223,151,200,223,154,201,224,157,202,225,159,203,226,162,205,226,164,206,227,167,207,228,169,208,228,172,210,229,174,211,230,177,212,231,179,214,231,182,215,232,184,216,233,187,217,234,189,219,234,192,220,235,194,221,236,197,223,236,199,224,237,202,225,238,204,226,239,207,228,239,209,229,240,210,230,240,212,230,241,213,231,241,215,232,241,216,232,241,218,233,242,219,234,242,221,235,242,222,235,242,224,236,243,225,237,243,227,237,243,228,238,244,230,239,244,231,239,244,233,240,244,234,241,245,235,241,245,237,242,245,238,243,245,240,244,246,241,244,246,243,245,246,244,246,246,246,246,247,247,247,247,247,246,245,248,245,243,248,244,241,248,243,240,249,242,238,249,241,236,249,239,234,250,238,232,250,237,230,250,236,228,250,235,227,251,234,225,251,233,223,251,232,221,251,231,219,251,230,217,252,229,215,252,228,214,252,227,212,252,225,210,252,224,208,252,223,206,253,222,204,253,221,203,253,220,201,253,219,199,253,217,196,253,215,193,252,212,191,252,210,188,252,208,185,252,206,182,252,204,179,251,202,177,251,200,174,251,197,171,250,195,168,250,193,165,250,191,163,249,189,160,249,187,157,248,184,154,248,182,152,248,180,149,247,178,146,247,176,143,246,174,141,246,171,138,245,169,135,245,167,133,244,165,130,243,162,128,242,160,126,241,157,124,240,155,122,239,152,119,238,149,117,237,147,115,235,144,113,234,142,111,233,139,109,232,136,107,231,134,105,230,131,103,229,128,101,227,126,99,226,123,97,225,120,95,224,118,93,223,115,91,221,112,89,220,110,87,219,107,85,218,104,83,217,102,81,215,99,79,214,96,77,213,94,76,211,91,74,210,89,73,208,86,71,207,84,70,206,82,68,204,79,67,203,77,66,201,74,64,200,72,63,198,69,62,197,66,60,196,64,59,194,61,57,193,58,56,191,55,55,190,53,53,188,50,52,187,46,51,185,43,49,184,40,48,182,36,47,181,33,46,179,29,44,178,24,43,175,23,43,172,22,42,169,21,42,166,20,41,162,19,41,159,18,40,156,17,40,153,15,39,150,14,39,147,13,38,144,12,38,141,11,37,138,10,37,135,9,36,132,8,36,129,7,35,126,6,35,123,5,34,120,4,34,117,3,33,115,2,33,112,2,33,109,1,32,106,1,32,103,0,31};


    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream cc, qq, tt;
        cc << "C" << i + 1;
        qq << "q" << i + 1;
        tt << "t" << i + 1;  
        servMes->m_transfer_function[i].m_name          = tt.str();
        servMes->m_transfer_function[i].m_color_variable       = qq.str();
        servMes->m_transfer_function[i].m_color_variable_min   = servMes->m_server_side_variable_range.min( tt.str() + "_var_c" );
        servMes->m_transfer_function[i].m_color_variable_max   = servMes->m_server_side_variable_range.max( tt.str() + "_var_c" ); 
        servMes->m_transfer_function[i].m_opacity_variable     = qq.str();
        servMes->m_transfer_function[i].m_opacity_variable_min = servMes->m_server_side_variable_range.min( tt.str() + "_var_o" );
        servMes->m_transfer_function[i].m_opacity_variable_max = servMes->m_server_side_variable_range.max( tt.str() + "_var_o" ); 
        servMes->m_transfer_function[i].m_resolution           = TF_resolution;
        servMes->m_transfer_function[i].m_equation_red         = ""; 
        servMes->m_transfer_function[i].m_equation_green       = ""; 
        servMes->m_transfer_function[i].m_equation_blue        = ""; 
        servMes->m_transfer_function[i].m_equation_opacity     = "";
        kvs::ValueArray<kvs::UInt8> cc_table(c_table);
        kvs::ValueArray<float> oo_table(o_table);
        kvs::ColorMap color_map( cc_table    , servMes->m_server_side_variable_range.min( tt.str() + "_var_c" ), servMes->m_server_side_variable_range.max( tt.str() + "_var_c" )  );
        kvs::OpacityMap opacity_map( oo_table, servMes->m_server_side_variable_range.min( tt.str() + "_var_o" ), servMes->m_server_side_variable_range.max( tt.str() + "_var_o" )  );
        //kvs::ColorMap color_map( TF_resolution*3, m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max  );
        //kvs::OpacityMap opacity_map( TF_resolution, m_transfunc[i].m_color_variable_min, m_transfunc[i].m_color_variable_max  );
        //for (int n =0 ; n < TF_resolution*3 ; n++ ) color_map.table.at(n) = 1; // dummy data
        //for (int n =0 ; n < TF_resolution ; n++ ) opacity_map.table.at(n) = 1; // dummy data
        servMes->m_transfer_function[i].setColorMap( color_map );
        servMes->m_transfer_function[i].setOpacityMap( opacity_map );

        //servMes->m_transfer_function[i].m_selection = NamedTransferFunctionParameter::SelectExtendTransferFunction;
        servMes->m_transfer_function[i].m_selection = NamedTransferFunctionParameter::SelectTransferFunction;
//        servMes->m_transfer_function.push_back( tf );
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
                        servMes.m_number_glyph = 0;
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
                servMes.m_number_ingredients = pm.particleHistoryFile().nVariables();
                servMes.m_server_side_variable_range = range;
                servMes.m_flag_send_bins = 1;
                servMes.m_number_glyph = 0;

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

                servMes.m_server_status =0;
                servMes.m_message_size = servMes.byteSize();
                pts.sendMessage( servMes );
                delete servMes.m_camera;
                delete clntMes.m_camera;
            }// end of init process 初期化終了

            jpv::ParticleTransferServerMessage servMes;
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new kvs::Camera();
            servMes.m_camera = new kvs::Camera();
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

                            std::cout << "m_particle_density =" << servMes.m_particle_density << std::endl;

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
                else if (clntMes.m_initialize_parameter == jpv::InitializeParameter::generate_glyph )
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
                       
                        pm.findGlyphFile();
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
                            ppw.inputGlyphParameterMessage( clntMes );
                            ppr.readParameterFile( glyphParameterPath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();

                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( glyphParameterPath.c_str() );
                                // 20181226 end
                            }
                            pbvr::PointObject* originalObject = new pbvr::PointObject;
                            kvs::KVSMLObjectGlyph* originalGlyph = new kvs::KVSMLObjectGlyph;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
                            std::string filename( glyphFilePath );
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

                            std::cout<<"main.cpp:L1497"<<std::endl;

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
                            delete originalObject;
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_END( 473 );
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
                            ppw.inputPlotOverLineParameterMessage( clntMes );
                            ppr.readParameterFile( plotOverLineParameterPath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();

                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( plotOverLineParameterPath.c_str() );
                            }


                            kvs::KVSMLObjectPlotOverLine* originalObject = new kvs::KVSMLObjectPlotOverLine;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
                            std::string filename( plotOverLineFilePath );
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

                            std::cout<<"main.cpp:L1497"<<std::endl;

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
                            delete originalObject;
                            if ( timer_count <= TIMER_COUNT_NUM )
                            {
                                TIMER_END( 473 );
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

                }  // end loop of plot over line

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
