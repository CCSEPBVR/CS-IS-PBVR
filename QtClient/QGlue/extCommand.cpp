//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <kvs/OpenGL>
#include <QOpenGLContext>

#include "extCommand.h"

#include <Client/v3defines.h>
#include "Client/ExtendedParticleVolumeRenderer.h"

#include <Panels/renderoptionpanel.h>
#include <Panels/transformpanel.h>
#include <Panels/systemstatuspanel.h>

#include <QGlue/renderarea.h>
#include "objnameutil.h"

#include <kvs/RotationMatrix33>

#include <kvs/FileFormatBase>
#include <ThreeDS.h>
#include <FBX.h>
#include <TexturedPolygonObject.h>
#include <TexturedPolygonImporter.h>
#include <StochasticTexturedPolygonRenderer.h>

//OSAKI
#include <kvs/Stl>
#include <kvs/PolygonImporter>
//OSAKI

const kvs::Vec3 ExtCommand::referenceMinCoord = kvs::Vec3(-179.542, -109.5, 0.0);
const kvs::Vec3 ExtCommand::referenceMaxCoord = kvs::Vec3(388.83, 109.5, 388.83);


int g_i_w_v2foot = PBVR_SIZE;
int g_i_h_v2foot = PBVR_SIZE;


float lookat_upvectory_level = PBVR_LOOKAT_UPVECTORY_LEVEL;
float lookat_upvectorz_level = PBVR_LOOKAT_UPVECTORZ_LEVEL;


//Shared with renderarea
int  repeatlevela = PBVR_REPEAT_LEVEL_ABSTRACT;
int  repeatleveld = PBVR_REPEAT_LEVEL_DETAILED;
extern int  subpixellevela;// = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
extern int  subpixelleveld;//  = PBVR_SUB_PIXEL_LEVEL_DETAILED;

int     samplingtypelevel = 1;

int detailed_particle_transfer_typelevel = 1;
#define DEFAULT_VOLUMEDATA ""
//static int numvol, numstep = 0;
#ifdef _TIMER_
#define TIMER_MAX_COUNT 9
static int timer_count = 0;
#endif

// APPEND START By)K.Yodo 2020/12/16
static bool cgmodel_registered = true;
// APPEND END By)K.Yodo 2020/12/16
static bool polygon_model_registered = true;

extern char pfi_path_server[256];
extern char filter_parameter_filename[256];

float wk_resolution_width_level = PBVR_RESOLUTION_WIDTH_LEVEL;
float wk_resolution_height_level = PBVR_RESOLUTION_HEIGHT_LEVEL;

int wk_subpixellevela = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
int wk_subpixelleveld = PBVR_SUB_PIXEL_LEVEL_DETAILED;
int wk_repeatlevela = PBVR_REPEAT_LEVEL_ABSTRACT;
int wk_repeatleveld = PBVR_REPEAT_LEVEL_DETAILED;
int wk_plimitlevel = PBVR_PARTICLE_LIMIT;
float wk_pdensitylevel = PBVR_PARTICLE_DENSITY;
float wk_particle_data_size_limit = PBVR_PARTICLE_DATA_SIZE_LIMIT;
float wk_rotation_x_axis_level = PBVR_ROTATION_X_AXIS_LEVEL;
float wk_rotation_y_axis_level = PBVR_ROTATION_Y_AXIS_LEVEL;
float wk_rotation_z_axis_level = PBVR_ROTATION_Z_AXIS_LEVEL;
float wk_translate_x_level = PBVR_TRANSLATE_X_LEVEL;
float wk_translate_y_level = PBVR_TRANSLATE_Y_LEVEL;
float wk_translate_z_level = PBVR_TRANSLATE_Z_LEVEL;
float wk_scaling_level = PBVR_SCALING_LEVEL;
float wk_lookat_upvectory_level = PBVR_LOOKAT_UPVECTORY_LEVEL;
float wk_lookat_upvectorz_level = PBVR_LOOKAT_UPVECTORZ_LEVEL;
char g_volumedata[256]; // converted to char + bool , Martin
bool g_volumedata_active;


ExtCommand::ExtCommand(int argc, char *argv[]):
    argument(argc,argv),
    kvs::visclient::Command(&m_server),
    comthread(this)

{
    //    g_command = (Command*) this;
        this->parseArguments();
        this->initCommand();
}
/*===========================================================================*/
/**
 * @brief ExtCommand::parseArguments
 */
/*===========================================================================*/
void ExtCommand::parseArguments()
{
    if ( argument.m_repeat_level_detailed_flag )
    {
        repeatleveld = argument.m_repeat_level_detailed;
    }
    if ( argument.m_subpixel_level_detailed_flag )
    {
        subpixelleveld = argument.m_subpixel_level_detailed;
    }
#ifdef CS_MODE
    if ( argument.m_particle_limit_flag )
    {
        RenderoptionPanel::plimitlevel = argument.m_particle_limit;
    }
    if ( argument.m_particle_density_flag )
    {
        RenderoptionPanel::pdensitylevel = argument.m_particle_density;
    }
#endif
    // APPEND START BY)M.Tanaka 2015.03.11
    if ( argument.m_sampling_type_f )
    {
        if ( strcmp( argument.m_sampling_type.c_str(), "u" ) == 0 )
        {
            //printf("***** samplingtype:u\n");
            samplingtypelevel = kvs::visclient::VisualizationParameter::UniformSampling;
        }
        else if ( strcmp( argument.m_sampling_type.c_str(), "r" ) == 0 )
        {
            //printf("***** samplingtype:r\n");
            samplingtypelevel = kvs::visclient::VisualizationParameter::RejectionSampling;
        }
        else if ( strcmp( argument.m_sampling_type.c_str(), "m" ) == 0 )
        {
            //printf("***** samplingtype:m\n");
            samplingtypelevel = kvs::visclient::VisualizationParameter::MetropolisSampling;
        }
    }
    if ( argument.m_detailed_particle_transfer_type_flag )
    {
        if ( strcmp( argument.m_detailed_particle_transfer_type.c_str(), "all" ) == 0 )
        {
            //printf("***** transfertype:all\n");
            detailed_particle_transfer_typelevel = kvs::visclient::VisualizationParameter::Summalized;
        }
        else if ( strcmp( argument.m_detailed_particle_transfer_type.c_str(), "div" ) == 0 )
        {
            //printf("***** transfertype:div\n");
            detailed_particle_transfer_typelevel = kvs::visclient::VisualizationParameter::Divided;
        }
    }
    printf( "\n\n\n***** 0 shading:%s\n", RenderArea::shadinglevel );
    if ( argument.m_shading_type_flag )
    {
        strcpy( RenderArea::shadinglevel, argument.m_shading_type.c_str() );
        printf( "***** 1 shading:%s\n", RenderArea::shadinglevel );
    }
}
/*===========================================================================*/
/**
 * @brief ExtCommand::inSituInit
 */
/*===========================================================================*/
void ExtCommand::inSituInit()
{
#ifdef IS_MODE
    char buff_volumedata[100] = "/work/filter/binary08/binary";

    if ( argument.server_subvol_dir != "" )
    {
        param.inputdir = argument.server_subvol_dir;
    }
    else
    {
        param.inputdir = buff_volumedata;
    }
    // APPEND START BY)M.Tanaka 2015.03.03
    strcpy( pfi_path_server, param.inputdir.c_str() );
    // APPEND END   BY)M.Tanaka 2015.03.03

    isImportTransferFunctionParam = true;
    getServerParticleInfomation( param, &result );
    isImportTransferFunctionParam = false;

    if ( argument.particle_limit_f )
    {
        RenderoptionPanel::plimitlevel = argument.particle_limit;
    }
    if ( argument.particle_density_f )
    {
        RenderoptionPanel::pdensitylevel = argument.particle_density;
    }
    if ( argument.viewer_f )
    {
        sscanf( argument.viewer.c_str(), "%fx%f",
                &RenderoptionPanel::resolution_width_level,
                &RenderoptionPanel::resolution_height_level );
    }
    if ( argument.particle_data_size_limit_f )
    {

        particle_data_size_limit_input = argument.particle_data_size_limit;
    }

    param.particle_limit = RenderoptionPanel::plimitlevel;
    param.particle_density = RenderoptionPanel::pdensitylevel;
    param.particle_data_size_limit = RenderoptionPanel::data_size_limit;
#endif
}
/*===========================================================================*/
/**
 * @brief ExtCommand::clientServerInit
 */
/*===========================================================================*/
void ExtCommand::clientServerInit()
{
#ifdef CS_MODE
    if (!this->m_parameter.m_input_directory.empty()) {
        // APPEND START BY)M.Tanaka 2015.03.03
        strcpy( pfi_path_server, this->m_parameter.m_input_directory.c_str() );
        // APPEND END   BY)M.Tanaka 2015.03.03
        //        g_volumedata->set_text(this->m_parameter.m_input_directory.c_str() );
        //        this->setWindowTitle(this->m_parameter.m_input_directory.c_str() );
        this->m_parameter.m_hasfin = false;
    }
    else {
        pfi_path_server[0] = 0x00;
    }


    // add by @hira at 2016/12/01


    this->m_parameter.m_filter_parameter_filename = argument.m_filter_parameter_filename;
    if (!this->m_parameter.m_filter_parameter_filename.empty()) {
        strcpy( filter_parameter_filename, this->m_parameter.m_filter_parameter_filename.c_str() );
        //        g_volumedata->set_text( this->m_parameter.m_filter_parameter_filename.c_str() );
        //        this->setWindowTitle( this->m_parameter.m_filter_parameter_filename.c_str() );
        this->m_parameter.m_hasfin = true;
    }
    else {
        filter_parameter_filename[0] = 0x00;
    }
#endif
}
/*===========================================================================*/
/**
 * @brief ExtCommand::initCommand
 */
/*===========================================================================*/
void ExtCommand::initCommand()
{
    qInfo("ExtCommand::initCommand()");
    // APPEND END   fp)m.tanaka 2013.09.01
#ifdef IS_MODE
//    inSituInit();
#else
    this->m_parameter.m_particle_limit = RenderoptionPanel::plimitlevel;
    this->m_parameter.m_particle_density = RenderoptionPanel::pdensitylevel ;
#endif
    this->m_parameter.m_detailed_repeat_level = repeatleveld;
    this->m_parameter.m_detailed_subpixel_level = subpixelleveld;
    // APPEND START BY)M.Tanaka 2015.03.11
    this->m_parameter.m_sampling_type =
            static_cast<kvs::visclient::VisualizationParameter::SamplingType>( samplingtypelevel );
    this->m_parameter.m_detailed_transfer_type =
            static_cast<kvs::visclient::VisualizationParameter::DetailedParticleTransferType>( detailed_particle_transfer_typelevel );
    // APPEND END   BY)M.Tanaka 2015.03.11

    sscanf( argument.m_viewer.c_str(), "%fx%f",
            &RenderoptionPanel::resolution_width_level,
            &RenderoptionPanel::resolution_height_level );
    // APPEND END   BY)M.Tanaka 2015.03.03

    this->m_parameter.m_abstract_repeat_level = argument.m_repeat_level_abstract;
    this->m_parameter.m_abstract_subpixel_level = argument.m_subpixel_level_abstract;
    this->m_parameter.m_transfer_function = argument.m_transfer_function;

    this->m_parameter.m_hostname = argument.m_hostname;
    this->m_parameter.m_port = argument.m_port;
    this->m_parameter.m_shading_type_flag = argument.m_shading_type_flag;

    if ( argument.m_server_subvolume_directory != "" )
    {
        this->m_parameter.m_input_directory = argument.m_server_subvolume_directory;
    }
    else
    {
        this->m_parameter.m_input_directory = DEFAULT_VOLUMEDATA;
    }
#ifdef CS_MODE
    clientServerInit();
#else
    inSituInit();
#endif
    if ( argument.m_header != "" )
    {
        this->m_parameter.m_particle_directory = argument.m_header;
        this->m_parameter.m_client_server_mode = 0;
    }

    // pin引数を反映してUIを作成
    for ( int i = 0; i < 10; i++ )
    {
        if ( argument.m_pin_option_directory[i].empty() != true )
        {
            this->m_parameter.m_particle_merge_param.m_particles[i + 1].m_file_path.append( argument.m_pin_option_directory[i] );
        }
    }

    // APPEND START By)K.yodo 2020.12.16
    if (argument.m_cgmodel_flag) {
        this->m_parameter.m_cgmodel_filename = argument.m_cgmodel;
        cgmodel_registered = false;
    }
    // APPEND END By)K.yodo 2020.12.16

    // APPEND START By)K.yodo 2021.1.22
    this->m_parameter.m_lefty_mode = argument.m_lefty_flag;

    //std::cout << "ExtCommand::initCommand() : m_lefty_mode=" << this->m_parameter.m_lefty_mode << std::endl;

    // APPEND END By)K.yodo 2020.12.16

    //OSAKI
    if(argument.m_polygon_model_flag) {
        this->m_parameter.m_polygon_model_filename = argument.m_polygon_model;
        polygon_model_registered = false;
    }
    //OSAKI


}
/*===========================================================================*/
/**
 * @brief ExtCommand::CallBackApply
 * @param i
 */
/*===========================================================================*/
void ExtCommand::CallBackApply( const int i )
{
    if ( wk_subpixellevela != subpixellevela )
    {
        this->m_parameter.m_abstract_subpixel_level = subpixellevela;
        wk_subpixellevela = subpixellevela;
        std::cout << "*** new param.m_abstract_subpixel_level = " << subpixellevela << std::endl;
    }
    if ( wk_subpixelleveld != subpixelleveld )
    {
        this->m_parameter.m_detailed_subpixel_level = subpixelleveld;
        wk_subpixelleveld = subpixelleveld;
        std::cout << "*** new param.m_detailed_subpixel_level = " << subpixelleveld << std::endl;
    }

    if ( wk_repeatlevela != repeatlevela )
    {
        this->m_parameter.m_abstract_repeat_level = repeatlevela;
        wk_repeatlevela = repeatlevela;
        std::cout << "*** new param.m_abstract_repeat_level = " << repeatlevela << std::endl;
    }
    if ( wk_repeatleveld != repeatleveld )
    {
        this->m_parameter.m_detailed_repeat_level = repeatleveld;
        wk_repeatleveld = repeatleveld;
        std::cout << "*** new param.m_detailed_repeat_level = " << repeatleveld << std::endl;
    }

    if ( wk_plimitlevel != RenderoptionPanel::plimitlevel )
    {
        this->m_parameter.m_particle_limit = RenderoptionPanel::plimitlevel;
        wk_plimitlevel = RenderoptionPanel::plimitlevel;
        std::cout << "*** new param.m_particle_limit = " << RenderoptionPanel::plimitlevel << std::endl;
    }

    if ( wk_pdensitylevel != RenderoptionPanel::pdensitylevel )
    {
        this->m_parameter.m_particle_density = RenderoptionPanel::pdensitylevel;
        wk_pdensitylevel = RenderoptionPanel::pdensitylevel;
        std::cout << "*** new param.m_particle_density = " << RenderoptionPanel::pdensitylevel << std::endl;
    }

    if (wk_particle_data_size_limit != RenderoptionPanel::data_size_limit){
        this->m_parameter.particle_data_size_limit= RenderoptionPanel::data_size_limit;
        wk_particle_data_size_limit = RenderoptionPanel::data_size_limit;
        std::cout << "*** new param.particle_data_size_limit = " << RenderoptionPanel::data_size_limit << std::endl;
    }
    kvs::Vector3f v3;
    if ( ( wk_translate_x_level != TransformPanel::param.translateX )
         || ( wk_translate_y_level != TransformPanel::param.translateY )
         || ( wk_translate_z_level != TransformPanel::param.translateZ ) )
    {
        v3.set( TransformPanel::param.translateX,
                TransformPanel::param.translateY,
                TransformPanel::param.translateZ );
        wk_translate_x_level = TransformPanel::param.translateX;
        wk_translate_y_level = TransformPanel::param.translateY;
        wk_translate_z_level = TransformPanel::param.translateZ;
        std::cout << "*** new param.lookut_center_level = " <<
                     TransformPanel::param.translateX << ", " <<
                     TransformPanel::param.translateY << ", " <<
                     TransformPanel::param.translateZ << std::endl;
    }
    if ( ( wk_rotation_x_axis_level != TransformPanel::param.rotationAxisX )
         || ( wk_rotation_y_axis_level != TransformPanel::param.rotationAxisY )
         || ( wk_rotation_z_axis_level != TransformPanel::param.rotationAxisZ ) )
    {
        v3.set( TransformPanel::param.rotationAxisX,
                TransformPanel::param.rotationAxisY,
                TransformPanel::param.rotationAxisZ );
        wk_rotation_x_axis_level = TransformPanel::param.rotationAxisX;
        wk_rotation_y_axis_level = TransformPanel::param.rotationAxisY;
        wk_rotation_z_axis_level = TransformPanel::param.rotationAxisZ;
        std::cout << "*** new param.lookut_eye_level = " <<
                     TransformPanel::param.rotationAxisX << ", " <<
                     TransformPanel::param.rotationAxisY << ", " <<
                     TransformPanel::param.rotationAxisZ << std::endl;
    }
    if ( ( wk_scaling_level != TransformPanel::param.scalingLevel )
         || ( wk_lookat_upvectory_level != lookat_upvectory_level )
         || ( wk_lookat_upvectorz_level != lookat_upvectorz_level ) )
    {
        v3.set( TransformPanel::param.scalingLevel, lookat_upvectory_level, lookat_upvectorz_level );
        wk_scaling_level = TransformPanel::param.scalingLevel;
        wk_lookat_upvectory_level = lookat_upvectory_level;
        wk_lookat_upvectorz_level = lookat_upvectorz_level;
        std::cout << "*** new param.lookut_upvector_level = " <<
                     TransformPanel::param.scalingLevel << ", " <<
                     lookat_upvectory_level << ", " <<
                     lookat_upvectorz_level << std::endl;
    }
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.06
    //m_screen->kvs::ScreenBase::reset();
    //m_screen->reset(); //camera()->resetXform();
//    m_screen->Scene::reset();
    /*
    m_screen->objectManager()->translate( kvs::Vector3f(
                                            TransformPanel::param.translateX,
                                            TransformPanel::param.translateY,
                                            TransformPanel::param.translateZ) );
    m_screen->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
                                         TransformPanel::param.rotationAxisZ,
                                         TransformPanel::param.rotationAxisY,
                                         TransformPanel::param.rotationAxisX) );
    float s = TransformPanel::param.scalingLevel;
    m_screen->objectManager()->scale( kvs::Vector3f( s, s, s ) );
    */
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    m_screen->scene()->reset();
    m_screen->scene()->objectManager()->translate( kvs::Vector3f(
                                            TransformPanel::param.translateX,
                                            TransformPanel::param.translateY,
                                            TransformPanel::param.translateZ) );
    m_screen->scene()->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
                                         TransformPanel::param.rotationAxisZ,
                                         TransformPanel::param.rotationAxisY,
                                         TransformPanel::param.rotationAxisX) );
    float s = TransformPanel::param.scalingLevel;
    m_screen->scene()->objectManager()->scale( kvs::Vector3f( s, s, s ) );

    // APPEND START FP)K.YAJIMA & M.Tanaka 2015.03.03
    if ( wk_resolution_width_level != RenderoptionPanel::resolution_width_level )
    {
        this->m_parameter.m_resolution_width = RenderoptionPanel::resolution_width_level;
        wk_resolution_width_level = RenderoptionPanel::resolution_width_level;
        std::cout << "*** new param.m_resolution_width = " << RenderoptionPanel::resolution_width_level << std::endl;
    }
    if ( wk_resolution_height_level != RenderoptionPanel::resolution_height_level )
    {
        this->m_parameter.m_resolution_height = RenderoptionPanel::resolution_height_level;
        wk_resolution_height_level = RenderoptionPanel::resolution_height_level;
        std::cout << "*** new param.m_resolution_height = " << RenderoptionPanel::resolution_height_level << std::endl;
    }
    int i_w = ( int )( ( long )RenderoptionPanel::resolution_width_level );
    int i_h = ( int )( ( long )RenderoptionPanel::resolution_height_level );


    // APPEND START FP)K.YAJIMA 2015.03.17
    if ( g_i_w_v2foot != i_w || g_i_h_v2foot != i_h )
    {
        // 幅、高さのどちらかが現解像度と異なった場合
        // hide(),show()必須

        //        m_screen->hide();

        printf( "動的に変化させる\n" );
        printf( "g_i_w_v2foot = %d , g_i_h_v2foot = %d\n", g_i_w_v2foot, g_i_h_v2foot );
        printf( "i_w = %d , i_h = %d\n", i_w, i_h );
        //動的に変化させる
        m_screen->setGeometry(QRect( 0, 0, i_w, i_h ));

        printf( "DISABLED LABEL POSITION IN FOOTER\n" );
        int yl0 = i_h - PBVR_I_H_OFFSET;

    }

    g_i_w_v2foot = i_w;
    g_i_h_v2foot = i_h;



    /* change pfi file */
#ifdef IS_MODE
    if (false){
#else
    if (this->m_parameter.m_hasfin) {
#endif
        if ( strlen( g_volumedata ) > 0 &&
             strcmp( filter_parameter_filename, g_volumedata ) != 0 )
        {
            sprintf( filter_parameter_filename, "%s", g_volumedata );
            for ( int i = strlen( filter_parameter_filename ); i >= 0; i-- )
            {
                if ( isspace( filter_parameter_filename[i] ) )
                {
                    filter_parameter_filename[i] = '\0';
                }
                else
                {
                    break;
                }
            }
        }
    }
    else {
        if ( strlen( g_volumedata ) > 0 &&
             strcmp( pfi_path_server, g_volumedata ) != 0 )
        {
            sprintf( pfi_path_server, "%s", g_volumedata );
            for ( int i = strlen( pfi_path_server ); i >= 0; i-- )
            {
                if ( isspace( pfi_path_server[i] ) )
                {
                    pfi_path_server[i] = '\0';
                }
                else
                {
                    break;
                }
            }
            int s_len = strlen( pfi_path_server );
            if ( pfi_path_server[s_len - 4] == '.' &&
                 pfi_path_server[s_len - 3] == 'p' &&
                 pfi_path_server[s_len - 2] == 'f' &&
                 pfi_path_server[s_len - 1] == 'i' )
            {
                pfi_path_server[s_len - 4] = '\0';
            }
            /* printf(" [debug] Set parameter pressed. pfi_path_server = %s\n", pfi_path_server); */
        }
    }

    // necessary server param extension
#ifdef NOTOPEN
    if ( strcmp( wk_volumedata, volumedata ) != 0 )
    {
        strcpy( this->param.xxxxx, volumedata );
        wk_volumedata = volumedata;
        std::cout << "*** new param.xxxxx = " << volumedata << std::endl;
    }
    if ( wk_cxmaxlevel != cxmaxlevel )
    {
        this->param.xxxxx = cxmaxlevel;
        wk_cxmaxlevel = cxmaxlevel;
        std::cout << "*** new param.xxxxx = " << cxmaxlevel << std::endl;
    }
    if ( wk_cxminlevel != cxminlevel )
    {
        this->param.xxxxx = cxminlevel;
        wk_cxminlevel = cxminlevel;
        std::cout << "*** new param.xxxxx = " << cxminlevel << std::endl;
    }
    if ( wk_cymaxlevel != cymaxlevel )
    {
        this->param.xxxxx = cymaxlevel;
        wk_cymaxlevel = cymaxlevel;
        std::cout << "*** new param.xxxxx = " << cymaxlevel << std::endl;
    }
    if ( wk_cyminlevel != cyminlevel )
    {
        this->param.xxxxx = cyminlevel;
        wk_cyminlevel = cyminlevel;
        std::cout << "*** new param.xxxxx = " << cyminlevel << std::endl;
    }
    if ( wk_czmaxlevel != czmaxlevel )
    {
        this->param.xxxxx = czmaxlevel;
        wk_czmaxlevel = czmaxlevel;
        std::cout << "*** new param.xxxxx = " << czmaxlevel << std::endl;
    }
    if ( wk_czminlevel != czminlevel )
    {
        this->param.xxxxx = czminlevel;
        wk_czminlevel = czminlevel;
        std::cout << "*** new param.xxxxx = " << czminlevel << std::endl;
    }

    if ( wk_translate_x_level != translate_x_level )
    {
        this->param.xxxxx = translate_x_level;
        wk_translate_x_level = translate_x_level;
        std::cout << "*** new param.xxxxx = " << translate_x_level << std::endl;
    }
    // APPEND START FP)K.YAJIMA 2015.02.27
    if ( wk_resolution_width_level != resolution_width_level )
    {
        this->param.xxxxx = resolution_width_level;
        wk_resolution_width_level = resolution_width_level;
        std::cout << "*** new param.xxxxx = " << resolution_width_level << std::endl;
    }
    if ( wk_resolution_height_level != resolution_height_level )
    {
        this->param.xxxxx = resolution_height_level;
        wk_resolution_height_level = resolution_height_level;
        std::cout << "*** new param.xxxxx = " << resolution_height_level << std::endl;
    }
    // APPEND END FP)K.YAJIMA 2015.02.27
#endif

    this->m_parameter.m_no_repsampling = SystemstatusPanel::m_no_repsampling;
    this->m_glut_timer->setInterval( SystemstatusPanel::interval_msec );

    // APPEND START By) K.Yodo 2020/12/16
    //if (!cgmodel_registered) {
    std::cout << "ExtCommand::CallBackApply() : register cg model : " << m_parameter.m_cgmodel_filename << std::endl;
    if (this->m_parameter.m_cgmodel_filename != "") {
        this->registerCGModel();
        cgmodel_registered = true;
    }
    // APPEND END By) K.Yodo 2020/12/16

    // APPEND START By) K.Yodo 2021/1/22
    kvs::oculus::jaea::pbvr::PBVRWidgetHandler *widget_handler = PBVRWidgetHandlerInstance();
    widget_handler->setLeftyMode (this->m_parameter.m_lefty_mode);

    std::cout << "ExtCommand::CallBackApply() : PBVRWidgetHandler->leftyMode()=" << widget_handler->leftyMode() << std::endl;

    // APPEND END By) K.Yodo 2021/1/22

//    // APPEND START By) K.Yodo 2021/2/4
//#ifdef ENABLE_BENCHMARK_POLYGON
//    registerBenchmarkModel ();
//#endif //  ENABLE_BENCHMARK_POLYGON
//    // APPEND END By) K.Yodo 2021/2/4


    return;

}

// APPEND START By) K.Yodo 2021/2/4
#ifdef ENABLE_BENCHMARK_POLYGON
void ExtCommand::registerBenchmarkModel () {
    // 点の数
    const static size_t N = 16;

    // ポリゴンの数
    const static size_t P = 14;

    static kvs::Real32 CoordArray[N * 3] = {
        10,  0, 20, // 点0の座標  (1.0, 0.0, 2.0)
        20,  0, 20, // 点1の座標  (2.0, 0.0, 2.0)
        15,  8, 17, // 点2の座標  (1.5, 0.8, 1.7)
        15,  0, 11, // 点3の座標  (1.5, 0.0, 1.1)
        30,  0, 20, // 点4の座標  (3.0, 0.0, 2.0)
        40,  0, 20, // 点5の座標  (4.0, 0.0, 2.0)
        35,  8, 17, // 点6の座標  (3.5, 0.8, 1.7)
        35,  0, 11, // 点7の座標  (3.5, 0.0, 1.1)
        20,  0, 30, // 点8の座標  (2.0, 0.0, 3.0)
        30,  0, 30, // 点9の座標  (3.0, 0.0, 3.0)
        25,  8, 27, // 点10の座標 (2.5, 0.8, 2.7)
        25,  0, 21, // 点11の座標 (2.5, 0.0, 2.1)
         0, -1,  0, // 点12の座標 (0.0, -0.1, 0.0)
        50, -1,  0, // 点13の座標 (5.0, -0.1, 0.0)
        50, -1, 50, // 点14の座標 (5.0, -0.1, 5.0)
         0, -1, 50  // 点15の座標 (0.0, -0.1, 5.0)
    };

    // カラー値配列の定義
    static kvs::UInt8 ColorArray[N * 3] = {
        255,   0,   0, // 点0の色  赤(255,   0,   0)
          0, 255,   0, // 点1の色  緑(  0, 255,   0)
          0,   0, 255, // 点2の色  青(  0,   0, 255)
        255, 255,   0, // 点3の色  黄(255, 255,   0)
        255,   0,   0, // 点4の色  赤(255,   0,   0)
          0, 255,   0, // 点5の色  緑(  0, 255,   0)
          0,   0, 255, // 点6の色  青(  0,   0, 255)
        255, 255,   0, // 点7の色  黄(255, 255,   0)
        255,   0,   0, // 点8の色  赤(255,   0,   0)
          0, 255,   0, // 点9の色  緑(  0, 255,   0)
          0,   0, 255, // 点10の色 青(  0,   0, 255)
        255, 255,   0, // 点11の色 黄(255, 255,   0)
        255,   0,   0, // 点12の色 黄(255,   0,   0)
        255,   0,   0, // 点13の色 黄(255,   0,   0)
        255,   0,   0, // 点14の色 黄(255,   0,   0)
        255,   0,   0  // 点15の色 黄(255,   0,   0)
    };

    // 法線ベクトル配列の定義
    static kvs::Real32 NormalArray[N * 3] = {
         0.00f,  0.3f,  0.8f, // 点0に対する法線ベクトル
         0.00f, -0.9f,  0.0f, // 点1に対する法線ベクトル
         0.72f,  0.3f, -0.4f, // 点2に対する法線ベクトル
        -0.72f,  0.3f, -0.4f, // 点3に対する法線ベクトル
         0.00f,  0.3f,  0.8f, // 点4に対する法線ベクトル
         0.00f, -0.9f,  0.0f, // 点5に対する法線ベクトル
         0.72f,  0.3f, -0.4f, // 点6に対する法線ベクトル
        -0.72f,  0.3f, -0.4f, // 点7に対する法線ベクトル
         0.00f,  0.3f,  0.8f, // 点8に対する法線ベクトル
         0.00f, -0.9f,  0.0f, // 点9に対する法線ベクトル
         0.72f,  0.3f, -0.4f, // 点10に対する法線ベクトル
        -0.72f,  0.3f, -0.4f, // 点11に対する法線ベクトル
         0.00f, -1.0f,  0.0f, // 点12に対する法線ベクトル
         0.00f, -1.0f,  0.0f, // 点13に対する法線ベクトル
         0.00f, -1.0f,  0.0f, // 点14に対する法線ベクトル
         0.00f, -1.0f,  0.0f  // 点15に対する法線ベクトル
    };

    // ポリゴンの接続情報配列の定義
    static kvs::UInt32 PolygonConnectionArray[P * 3] = {
         0,  1,  2, // 点0, 1, 2を接続
         0,  3,  1, // 点0, 3, 1を接続
         1,  3,  2, // 点1, 3, 2を接続
         0,  2,  3, // 点0, 2, 3を接続
         4,  5,  6, // 点4, 5, 6を接続
         4,  7,  5, // 点4, 7, 5を接続
         5,  7,  6, // 点5, 7, 6を接続
         4,  6,  7, // 点4, 6, 7を接続
         8,  9, 10, // 点8, 9, 10を接続
         8, 11,  9, // 点8, 11, 9を接続
         9, 11, 10, // 点9, 11, 10を接続
         8, 10, 11, // 点8, 10, 11を接続
        12, 13, 14, // 点12, 13, 14を接続
        12, 14, 15  // 点12, 14, 15を接続
    };

    // テクスチャ座標の配列の定義
    static kvs::Real32 TextureCoordArray[N * 2] = {
        0.2f, 0.3f,  // 点0に対するテクスチャ座標
        0.6f, 0.3f,  // 点1に対するテクスチャ座標
        0.4f, 0.7f,  // 点2に対するテクスチャ座標
        0.8f, 0.7f,  // 点3に対するテクスチャ座標
        0.8f, 0.7f,  // 点4に対するテクスチャ座標
        0.4f, 0.7f,  // 点5に対するテクスチャ座標
        0.6f, 0.3f,  // 点6に対するテクスチャ座標
        0.2f, 0.3f,  // 点7に対するテクスチャ座標
        0.8f, 0.7f,  // 点8に対するテクスチャ座標
        0.4f, 0.7f,  // 点9に対するテクスチャ座標
        0.6f, 0.3f,  // 点10に対するテクスチャ座標
        0.2f, 0.3f,  // 点11に対するテクスチャ座標
        0.0f, 0.0f,  // 点12に対するテクスチャ座標
        1.0f, 0.0f,  // 点13に対するテクスチャ座標
        1.0f, 1.0f,  // 点14に対するテクスチャ座標
        0.0f, 1.0f   // 点15に対するテクスチャ座標
    };

    // テクスチャIDの配列の定義
    static kvs::UInt32 TextureIdArray[N] = {
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3
    };

    // テクスチャ画像のサイズ フィルタリングの差異を確認するには、ある程度細かい画像である必要がある
    const static size_t TextureSize = 2048;
    static unsigned char *TextureArray0 = new unsigned char[TextureSize * TextureSize * 4];
    static unsigned char *TextureArray1 = new unsigned char[TextureSize * TextureSize * 4];
    static unsigned char *TextureArray2 = new unsigned char[TextureSize * TextureSize * 4];
    static unsigned char *TextureArray3 = new unsigned char[TextureSize * TextureSize * 4];

//    void makeTexture()
//    {
        for (size_t j=0; j<TextureSize; j++)
        {
            for (size_t i=0; i<TextureSize; i++)
            {
                unsigned char r0 = 0;
                unsigned char g0 = 0;
                unsigned char b0 = 0;
                unsigned char a0 = (unsigned char)255;
                unsigned char r1 = 0;
                unsigned char g1 = 0;
                unsigned char b1 = 0;
                unsigned char a1 = (unsigned char)255;
                unsigned char r2 = 0;
                unsigned char g2 = 0;
                unsigned char b2 = 0;
                unsigned char a2 = (unsigned char)255;
                unsigned char r3 = 0;
                unsigned char g3 = 0;
                unsigned char b3 = 0;
                unsigned char a3 = (unsigned char)255;

                int boxSize = 3;
                int ii = i / boxSize;
                int jj = j / boxSize;
                if((ii + jj) % 2 == 0){
                    r0 = (unsigned char)255;
                    g0 = (unsigned char)255;
                    b0 = (unsigned char)255;
                    r1 = (unsigned char)255;
                    g1 = (unsigned char)255;
                    b1 = (unsigned char)255;
                    r2 = (unsigned char)255;
                    g2 = (unsigned char)255;
                    b2 = (unsigned char)255;
                } else {
                    r0 = (unsigned char)( (i + j) / 2 % 256 );
                    g0 = (unsigned char)( (i + j) / 3 % 256 );
                    b0 = (unsigned char)( (i + j) / 4 % 256 );
                    r1 = (unsigned char)( (i + j) / 5 % 256 );
                    g1 = (unsigned char)( (i + j) / 4 % 256 );
                    b1 = (unsigned char)( (i + j) / 3 % 256 );
                    r2 = (unsigned char)( (i + j) / 4 % 256 );
                    g2 = (unsigned char)( (i + j) / 5 % 256 );
                    b2 = (unsigned char)( (i + j) / 6 % 256 );
                }
                if(ii  % 2 == 0){
                    r3 = (unsigned char)255;
                    g3 = (unsigned char)255;
                    b3 = (unsigned char)255;
                } else {
                    r3 = (unsigned char)( (i ) / 4 % 256 );
                    g3 = (unsigned char)( (i ) / 5 % 256 );
                    b3 = (unsigned char)( (i ) / 6 % 256 );
                }

                size_t offset = (i + j*TextureSize) * 4;
                TextureArray0[offset + 0] = r0;
                TextureArray0[offset + 1] = g0;
                TextureArray0[offset + 2] = b0;
                TextureArray0[offset + 3] = a0;
                TextureArray1[offset + 0] = r1;
                TextureArray1[offset + 1] = g1;
                TextureArray1[offset + 2] = b1;
                TextureArray1[offset + 3] = a1;
                TextureArray2[offset + 0] = r2;
                TextureArray2[offset + 1] = g2;
                TextureArray2[offset + 2] = b2;
                TextureArray2[offset + 3] = a2;
                TextureArray3[offset + 0] = r3;
                TextureArray3[offset + 1] = g3;
                TextureArray3[offset + 2] = b3;
                TextureArray3[offset + 3] = a3;
            }
        }
//    }


        // KVSの配列クラス（kvs::ValueArray）にセットする。
        kvs::ValueArray<kvs::Real32> coords(CoordArray, N * 3);
        kvs::ValueArray<kvs::UInt8> colors(ColorArray, N * 3);
        kvs::ValueArray<kvs::Real32> normals(NormalArray, N * 3);
        kvs::ValueArray<kvs::UInt32> connections(PolygonConnectionArray, P * 3);
        kvs::ValueArray<kvs::Real32> texture2DCoords(TextureCoordArray, N * 2);
        kvs::ValueArray<kvs::UInt32> textureIds(TextureIdArray, N);

        // ポリゴンオブジェクトを生成する。
        kvs::jaea::TexturedPolygonObject* texturedPolygon = new kvs::jaea::TexturedPolygonObject();
        texturedPolygon->setCoords(coords);
        texturedPolygon->setColors(colors);
        texturedPolygon->setNormals(normals);
        texturedPolygon->setConnections(connections);
        texturedPolygon->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
        texturedPolygon->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
        texturedPolygon->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
        texturedPolygon->setTexture2DCoords(texture2DCoords);
        texturedPolygon->setTextureIds(textureIds);

        {
            size_t bytesPerChannel = 1;
            int width = TextureSize;
            int height = TextureSize;
            int nChannels = 4;
            kvs::ValueArray<kvs::UInt8> pixels0(TextureArray0, width * height * nChannels);
            kvs::ValueArray<kvs::UInt8> pixels1(TextureArray1, width * height * nChannels);
            kvs::ValueArray<kvs::UInt8> pixels2(TextureArray2, width * height * nChannels);
            kvs::ValueArray<kvs::UInt8> pixels3(TextureArray3, width * height * nChannels);

            texturedPolygon->addColorArray(0, pixels0, width, height);
            texturedPolygon->addColorArray(1, pixels1, width, height);
            texturedPolygon->addColorArray(2, pixels2, width, height);
            texturedPolygon->addColorArray(3, pixels3, width, height);
        }



//    // ポリゴンオブジェクトのコネクティビティを複製してポリゴン数を増やす
//    kvs::jaea::TexturedPolygonObject* DuplicatePolygons(kvs::jaea::TexturedPolygonObject* texturedPolygonIn, int nDuplicate )
//    {
//        kvs::ValueArray<kvs::UInt32> connections = texturedPolygon->connections();

        int nDuplicate = 3;

        int nPolygonsOld = connections.size() / 3;
        int nPolygonsNew = nPolygonsOld * nDuplicate;

        kvs::ValueArray<kvs::UInt32> connectionsNew(nPolygonsNew * 3);
        for(int j=0;j<nDuplicate;j++){
            for(int i=0;i<connections.size();i++){
                int p = connections[i];
                connectionsNew[i + j * connections.size()] = p;
            }
        }


        kvs::jaea::TexturedPolygonObject* texturedPolygonOut = new kvs::jaea::TexturedPolygonObject();
        texturedPolygonOut->deepCopy(*texturedPolygon);
        texturedPolygonOut->setConnections(connectionsNew);

        std::cout << "registerBenchmarkModel() : " << std::endl;
        std::cout << "       nvertices=" << (texturedPolygonOut->coords().size() / 3) << std::endl;
        std::cout << "       npolygons=" << nPolygonsNew << std::endl;

        kvs::jaea::StochasticTexturedPolygonRenderer* renderer = new kvs::jaea::StochasticTexturedPolygonRenderer ();
        this->m_screen->scene()->registerObject(texturedPolygonOut, renderer);


//        return texturedPolygonOut;
//    }

//    // ポリゴンオブジェクトを生成する関数
//    kvs::jaea::TexturedPolygonObject* CreateTestData()
//    {
//        makeTexture();

//        // KVSの配列クラス（kvs::ValueArray）にセットする。
//        kvs::ValueArray<kvs::Real32> coords(CoordArray, N * 3);
//        kvs::ValueArray<kvs::UInt8> colors(ColorArray, N * 3);
//        kvs::ValueArray<kvs::Real32> normals(NormalArray, N * 3);
//        kvs::ValueArray<kvs::UInt32> connections(PolygonConnectionArray, P * 3);
//        kvs::ValueArray<kvs::Real32> texture2DCoords(TextureCoordArray, N * 2);
//        kvs::ValueArray<kvs::UInt32> textureIds(TextureIdArray, N);

//        // ポリゴンオブジェクトを生成する。
//        kvs::jaea::TexturedPolygonObject* texturedPolygon = new kvs::jaea::TexturedPolygonObject();
//        texturedPolygon->setCoords(coords);
//        texturedPolygon->setColors(colors);
//        texturedPolygon->setNormals(normals);
//        texturedPolygon->setConnections(connections);
//        texturedPolygon->setPolygonType(kvs::jaea::TexturedPolygonObject::Triangle);
//        texturedPolygon->setColorType(kvs::jaea::TexturedPolygonObject::VertexColor);
//        texturedPolygon->setNormalType(kvs::jaea::TexturedPolygonObject::VertexNormal);
//        texturedPolygon->setTexture2DCoords(texture2DCoords);
//        texturedPolygon->setTextureIds(textureIds);

//        {
//            size_t bytesPerChannel = 1;
//            int width = TextureSize;
//            int height = TextureSize;
//            int nChannels = 4;
//            kvs::ValueArray<kvs::UInt8> pixels0(TextureArray0, width * height * nChannels);
//            kvs::ValueArray<kvs::UInt8> pixels1(TextureArray1, width * height * nChannels);
//            kvs::ValueArray<kvs::UInt8> pixels2(TextureArray2, width * height * nChannels);
//            kvs::ValueArray<kvs::UInt8> pixels3(TextureArray3, width * height * nChannels);

//            texturedPolygon->addColorArray(0, pixels0, width, height);
//            texturedPolygon->addColorArray(1, pixels1, width, height);
//            texturedPolygon->addColorArray(2, pixels2, width, height);
//            texturedPolygon->addColorArray(3, pixels3, width, height);
//        }

//        // 生成したポリゴンオブジェクトのポインタを返す。
//        return texturedPolygon;
//    }

}
#endif // ENABLE_BENCHMARK_POLYGON
// APPEND START By) K.Yodo 2021/2/4

// APPEND START By) K.Yodo 2020/12/16
void ExtCommand::registerCGModel () {
    const std::string filePath = this->m_parameter.m_cgmodel_filename;
    const std::string extension = filePath.substr(filePath.length()-4, 4);

    kvs::FileFormatBase* io = nullptr;

    // select importer
    if(extension == ".3ds"){
        io = new kvs::jaea::ThreeDS();
    }else if(extension == ".fbx"){
        io = new kvs::jaea::FBX();
    }else{
        std::cerr << "Unknown file format:" << filePath << std::endl;
        return;
    }

    io->read(filePath);

    kvs::jaea::TexturedPolygonImporter* importer = nullptr;
    if(io->isSuccess()){
        std::cout << "loading " << io->filename() << " succeed." << std::endl;
        importer = new kvs::jaea::TexturedPolygonImporter();
        importer->exec(io);
        importer->setName (OBJ_NAME_CGMODEL);

        //bug調査
        importer->updateMinMaxCoords();

//#ifdef ENABLE_CGMODEL_EXT_COORD_MODE
#ifdef ENABLE_EXTCRD_SCALING_FOR_DEMO
        //CGモデルを、粒子モデル同レベルのサイズ、位置で表示するための座標値を計算する

        //粒子モデルのデータ
//        const kvs::Vec3 particleMin = kvs::Vector3f(-179.542, -109.5, 0.0);
//        const kvs::Vec3 particleMax = kvs::Vector3f(388.83, 109.5, 388.83);
        //        const kvs::Vec3 particleBoxLength = particleMax - particleMin;
        //        const kvs::Vec3 particleCenter = (particleMax + particleMin) / 2.0f;
        const kvs::Vec3 particleBoxLength = referenceMaxCoord - referenceMinCoord;
        const kvs::Vec3 particleCenter = (referenceMaxCoord + referenceMinCoord) / 2.0f;

        //CGモデルのデータ
        const kvs::Vec3 modelMin = importer->minObjectCoord();
        const kvs::Vec3 modelMax = importer->maxObjectCoord();
        const kvs::Vec3 modelBoxLength = modelMax - modelMin;
        const kvs::Vec3 modelCenter = (modelMax + modelMin) / 2.0f;

        float scale = particleBoxLength.length() / modelBoxLength.length();

        const kvs::Vec3 coordMin = (modelMin - modelCenter) * scale + particleCenter;
        const kvs::Vec3 coordMax = (modelMax - modelCenter) * scale + particleCenter;
        importer->setMinMaxExternalCoords(coordMin, coordMax);
#endif
        std::cout << "ntriangles=" << (importer->numberOfConnections()) << std::endl;
        std::cout << "Object###" << importer->minObjectCoord() << "," << importer->maxObjectCoord() << std::endl;
        std::cout << "External###" << importer->minExternalCoord() << "," << importer->maxExternalCoord() << std::endl;

#ifdef ENABLE_BENCHMARK_POLYGON
        //int nDuplicate = 15;
        int nDuplicate = 2;
        kvs::ValueArray<kvs::UInt32> connections = importer->connections();
        int nPolygonsOld = connections.size() / 3;
        int nPolygonsNew = nPolygonsOld * nDuplicate;

        kvs::ValueArray<kvs::UInt32> connectionsNew(nPolygonsNew * 3);
        for(int j=0;j<nDuplicate;j++){
            for(int i=0;i<connections.size();i++){
                int p = connections[i];
                connectionsNew[i + j * connections.size()] = p;
            }
        }

        kvs::jaea::TexturedPolygonObject* texturedPolygonOut = new kvs::jaea::TexturedPolygonObject();
        texturedPolygonOut->deepCopy(*importer);
        texturedPolygonOut->setConnections(connectionsNew);

        std::cout << "registerCGModel() : benchmark mode (nDuplicate=" << nDuplicate << ")" << std::endl;
        std::cout << "       nvertices=" << (texturedPolygonOut->coords().size() / 3) << std::endl;
        std::cout << "       npolygons=" << nPolygonsNew << std::endl;

        kvs::jaea::StochasticTexturedPolygonRenderer* renderer = new kvs::jaea::StochasticTexturedPolygonRenderer ();
        this->m_screen->scene()->registerObject(texturedPolygonOut, renderer);

#else // ENABLE_BENCHMARK_POLYGON
        kvs::jaea::StochasticTexturedPolygonRenderer* renderer = new kvs::jaea::StochasticTexturedPolygonRenderer ();
        this->m_screen->scene()->registerObject(importer, renderer);
#endif // ENABLE_BENCHMARK_POLYGON

    }else{
        std::cerr << "ERROR : loading " << io->filename() << " failed." << std::endl;
    }

    delete io;
}
// APPEND END By) K.Yodo 2020/12/16

void ExtCommand::registerPolygonModel(std::string str, int currentIndex, double opacity, kvs::RGBColor color, bool isSTL){
    std::cout << "[DEBUG_REG]" << str << std::endl;
    const std::string filePath = str;
    const std::string extension = filePath.substr(filePath.length()-4, 4);

    kvs::FileFormatBase* io = nullptr;

    // select importer
//    if(extension == ".stl"){
//        io = new kvs::Stl;
//    }else if( extension == ".kvsml"){
//    }else{
//        std::cerr << "Unknown file format:" << filePath << std::endl;
//        return;
//    }

    kvs::KVSMLPolygonObject* kvsml;
    if(isSTL == true)
    {
    }else
    {
        kvsml = new kvs::KVSMLPolygonObject(filePath);
    }

//        kvs::KVSMLPolygonObject* kvsml = new kvs::KVSMLPolygonObject("/home/osaki/Sandbox/STL/src/YAY.kvsml");


    kvs::PolygonObject* m_polygon_object_empty = new kvs::PolygonObject(); //空オブジェクト
//    kvs::PolygonObject* m_polygon_imp_object = new kvs::PolygonImporter(filePath);
    kvs::PolygonObject* m_polygon_imp_object;
    if(isSTL ==true)
    {
        std::cout << __LINE__ << std::endl;
        m_polygon_imp_object = new kvs::PolygonImporter(filePath);
    }else
    {
        std::cout << __LINE__ << std::endl;
        m_polygon_imp_object = new kvs::PolygonImporter(kvsml);
    }

    std::string ObjNameEmpty = "POLYGON_OBJ_EMPTY" + std::to_string(currentIndex - 5);
    std::string ObjNameImp = "POLYGON_OBJ_IMP" + std::to_string(currentIndex - 5);

    m_polygon_object_empty->setXform(m_screen->scene()->objectManager()->xform());
    m_polygon_object_empty->setMinMaxObjectCoords(m_polygon_imp_object->minObjectCoord(),m_polygon_imp_object->maxObjectCoord());
    m_polygon_object_empty->setMinMaxExternalCoords(m_polygon_imp_object->minExternalCoord(),m_polygon_imp_object->maxExternalCoord());

    m_polygon_object_empty->setName(ObjNameEmpty);
    m_polygon_imp_object->setName(ObjNameImp);
    m_polygon_imp_object->setOpacity(opacity*255);
    std::cout << isSTL << std::endl;
    if(isSTL == true)
    {
        m_polygon_imp_object->setColor(color);
    }else
    {
        std::cout << __LINE__ << std::endl;

    }
    kvs::StochasticPolygonRenderer* renderer = new kvs::StochasticPolygonRenderer();
    if(m_screen->scene()->hasObject(ObjNameEmpty) == false && m_screen->scene()->hasObject(ObjNameImp) == false)
    {
        m_screen->scene()->registerObject(m_polygon_object_empty,renderer);
        m_screen->scene()->replaceObject(ObjNameEmpty,m_polygon_imp_object,false);
    }

    if(m_screen->scene()->hasObject(ObjNameImp) == true)
    {
        m_screen->scene()->replaceObject(ObjNameImp,m_polygon_imp_object,false);
    }

    if(m_screen->scene()->hasObject(ObjNameEmpty) == true)
    {
        m_screen->scene()->replaceObject(ObjNameEmpty,m_polygon_imp_object,false);
    }

}

void ExtCommand::registerEmptyPolygonModel(std::string str, int currentIndex, double opacity, kvs::RGBColor color){
    const std::string filePath = str;
    const std::string extension = filePath.substr(filePath.length()-4, 4);

    kvs::FileFormatBase* io = nullptr;

    // select importer
//    if(extension == ".stl"){
//        io = new kvs::Stl;
//    }else if( extension == ".kvsml"){
//    }else{
//        std::cerr << "Unknown file format:" << filePath << std::endl;
//        return;
//    }

    kvs::PolygonObject* m_polygon_object_empty = new kvs::PolygonObject(); //空オブジェクト
    kvs::PolygonObject* m_polygon_imp_object = new kvs::PolygonImporter(filePath);

    std::string ObjNameEmpty = "POLYGON_OBJ_EMPTY" + std::to_string(currentIndex - 5);
    std::string ObjNameImp = "POLYGON_OBJ_IMP" + std::to_string(currentIndex - 5);

    m_polygon_object_empty->setXform(m_screen->scene()->objectManager()->xform());
    m_polygon_object_empty->setMinMaxObjectCoords(m_polygon_imp_object->minObjectCoord(),m_polygon_imp_object->maxObjectCoord());
    m_polygon_object_empty->setMinMaxExternalCoords(m_polygon_imp_object->minExternalCoord(),m_polygon_imp_object->maxExternalCoord());

    m_polygon_object_empty->setName(ObjNameEmpty);
    m_polygon_imp_object->setName(ObjNameImp);
    m_polygon_imp_object->setOpacity(opacity*255);
    m_polygon_imp_object->setColor(color);

    kvs::StochasticPolygonRenderer* renderer = new kvs::StochasticPolygonRenderer();
    if(m_screen->scene()->hasObject(ObjNameEmpty) == false && m_screen->scene()->hasObject(ObjNameImp) == false)
    {
        m_screen->scene()->registerObject(m_polygon_object_empty,renderer);
        m_screen->scene()->replaceObject(ObjNameEmpty,m_polygon_object_empty,false);
    }

    if(m_screen->scene()->hasObject(ObjNameEmpty) == true)
    {
        m_screen->scene()->replaceObject(ObjNameEmpty,m_polygon_object_empty,false);
    }

    if(m_screen->scene()->hasObject(ObjNameImp) == true)
    {
        m_screen->scene()->replaceObject(ObjNameImp,m_polygon_object_empty,false);
    }

}
//OSAKI

void ExtCommand::deletePolygonModel (int currentIndex) {
    std::string ObjName = "POLYGON_MODEL" + std::to_string(currentIndex - 6);
    std::string ObjNameEmpty = "POLYGON_OBJ_EMPTY" + std::to_string(currentIndex - 5);
    std::string ObjNameImp = "POLYGON_OBJ_IMP" + std::to_string(currentIndex - 5);
    if(this->m_screen->scene()->hasObject(ObjName) == true)
    {
        this->m_screen->scene()->removeObject(ObjName,false,false);
    }
    if(this->m_screen->scene()->hasObject(ObjNameEmpty) == true)
    {
        this->m_screen->scene()->removeObject(ObjNameEmpty,false,false);
    }
    if(this->m_screen->scene()->hasObject(ObjNameImp) == true)
    {
        this->m_screen->scene()->removeObject(ObjNameImp,false,false);
    }
}
