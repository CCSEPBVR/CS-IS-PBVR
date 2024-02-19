#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

// KVS2.7.0
// ADD BY)T.Osaki 2020.06.08
#include <QOpenGLContext>

#include "extCommand.h"
#include "SphereGlyph.h"
#include "ArrowGlyph.h"

#include <Client/v3defines.h>
// #include "Client/PBRProxy.h"

#include <Panels/renderoptionpanel.h>
#include <Panels/systemstatuspanel.h>
#include <Panels/transformpanel.h>

#include <QGlue/renderarea.h>

#include <kvs/RotationMatrix33>

#include <kvs/FileFormatBase>
#include <kvs/PolygonObject>
// OSAKI
#include <kvs/PolygonImporter>
#include <kvs/Stl>
// OSAKI
#include <kvs/StochasticPolygonRenderer>

#ifdef ENABLE_FEATURE_DRAW_TEXTURED_POLYGON
#ifdef ENABLE_ASSIMP
#include <ThreeDS.h>
#endif
#ifdef ENABLE_FBXSDK
#include <FBX.h>
#endif
#include <TexturedPolygonObject.h>
#include <TexturedPolygonImporter.h>
#include "stochastictexturedpolygonrenderer.h"
#endif // ENABLE_FEATURE_DRAW_TEXTURED_POLYGON

#include "objnameutil.h"

#ifdef ENABLE_LIBLAS
#include "QGlue/LASImporter.h"
#endif

#ifdef DISP_MODE_VR
#include "objectscale.h"
#include "pbvrwidgethandler.h"
#endif // DISP_MODE_VR

int g_i_w_v2foot = PBVR_SIZE;
int g_i_h_v2foot = PBVR_SIZE;

float lookat_upvectory_level = PBVR_LOOKAT_UPVECTORY_LEVEL;
float lookat_upvectorz_level = PBVR_LOOKAT_UPVECTORZ_LEVEL;

// Shared with renderarea
int repeatlevela = PBVR_REPEAT_LEVEL_ABSTRACT;
int repeatleveld = PBVR_REPEAT_LEVEL_DETAILED;
extern int subpixellevela; // = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
extern int subpixelleveld; //  = PBVR_SUB_PIXEL_LEVEL_DETAILED;

int samplingtypelevel = 2;

int detailed_particle_transfer_typelevel = 1;
#define DEFAULT_VOLUMEDATA ""
// static int numvol, numstep = 0;
#ifdef _TIMER_
#define TIMER_MAX_COUNT 9
static int timer_count = 0;
#endif

// OSAKI
static bool polygon_model_registered = true;
#ifdef ENABLE_LIBLAS
static bool las_model_registred = true;
#endif
// OSAKI

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

ExtCommand::ExtCommand(int argc, char *argv[])
    : argument(argc, argv), kvs::visclient::Command(&m_server), comthread(this)

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
void ExtCommand::parseArguments() {
  if (argument.m_repeat_level_detailed_flag) {
    repeatleveld = argument.m_repeat_level_detailed;
  }
  if (argument.m_subpixel_level_detailed_flag) {
    subpixelleveld = argument.m_subpixel_level_detailed;
  }
#ifdef COMM_MODE_CS
  if (argument.m_particle_limit_flag) {
    RenderoptionPanel::plimitlevel = argument.m_particle_limit;
  }
  if (argument.m_particle_density_flag) {
    RenderoptionPanel::pdensitylevel = argument.m_particle_density;
  }
#endif
  // APPEND START BY)M.Tanaka 2015.03.11
  if (argument.m_sampling_type_f) {
    if (strcmp(argument.m_sampling_type.c_str(), "u") == 0) {
      // printf("***** samplingtype:u\n");
      samplingtypelevel = kvs::visclient::VisualizationParameter::UniformSampling;
    } else if (strcmp(argument.m_sampling_type.c_str(), "r") == 0) {
      // printf("***** samplingtype:r\n");
      samplingtypelevel = kvs::visclient::VisualizationParameter::RejectionSampling;
    } else if (strcmp(argument.m_sampling_type.c_str(), "m") == 0) {
      // printf("***** samplingtype:m\n");
      samplingtypelevel = kvs::visclient::VisualizationParameter::MetropolisSampling;
    }
  }
  if (argument.m_detailed_particle_transfer_type_flag) {
    if (strcmp(argument.m_detailed_particle_transfer_type.c_str(), "all") == 0) {
      // printf("***** transfertype:all\n");
      detailed_particle_transfer_typelevel = kvs::visclient::VisualizationParameter::Summalized;
    } else if (strcmp(argument.m_detailed_particle_transfer_type.c_str(), "div") == 0) {
      // printf("***** transfertype:div\n");
      detailed_particle_transfer_typelevel = kvs::visclient::VisualizationParameter::Divided;
    }
  }
  printf("\n\n\n***** 0 shading:%s\n", RenderArea::shadinglevel);
  if (argument.m_shading_type_flag) {
    strcpy(RenderArea::shadinglevel, argument.m_shading_type.c_str());
    printf("***** 1 shading:%s\n", RenderArea::shadinglevel);
  }
}
/*===========================================================================*/
/**
 * @brief ExtCommand::inSituInit
 */
/*===========================================================================*/
void ExtCommand::inSituInit() {
#ifdef COMM_MODE_IS
  char buff_volumedata[100] = "/work/filter/binary08/binary";

  if (argument.m_server_subvolume_directory != "") {
    m_parameter.m_input_directory = argument.m_server_subvolume_directory;
  } else {
    m_parameter.m_input_directory = buff_volumedata;
  }
  // APPEND START BY)M.Tanaka 2015.03.03
  strcpy(pfi_path_server, m_parameter.m_input_directory.c_str());
  // APPEND END   BY)M.Tanaka 2015.03.03

  m_is_import_transfer_function_parameter = true;
  getServerParticleInfomation(&m_parameter, &m_result);
  m_is_import_transfer_function_parameter = false;

  if (argument.m_particle_limit_flag) {
    RenderoptionPanel::plimitlevel = argument.m_particle_limit;
  }
  if (argument.m_particle_density_flag) {
    RenderoptionPanel::pdensitylevel = argument.m_particle_density;
  }
  if (argument.m_viewer_flag) {
    sscanf(argument.m_viewer.c_str(), "%fx%f", &RenderoptionPanel::resolution_width_level, &RenderoptionPanel::resolution_height_level);
  }
  if (argument.particle_data_size_limit_f) {

    particle_data_size_limit_input = argument.particle_data_size_limit;
  }

  m_parameter.m_particle_limit = RenderoptionPanel::plimitlevel;
  m_parameter.m_particle_density = RenderoptionPanel::pdensitylevel;
  m_parameter.particle_data_size_limit = RenderoptionPanel::data_size_limit;
#endif
}
/*===========================================================================*/
/**
 * @brief ExtCommand::clientServerInit
 */
/*===========================================================================*/
void ExtCommand::clientServerInit() {
#ifdef COMM_MODE_CS
  if (!this->m_parameter.m_input_directory.empty()) {
    // APPEND START BY)M.Tanaka 2015.03.03
    strcpy(pfi_path_server, this->m_parameter.m_input_directory.c_str());
    // APPEND END   BY)M.Tanaka 2015.03.03
    //        g_volumedata->set_text(this->m_parameter.m_input_directory.c_str() );
    //        this->setWindowTitle(this->m_parameter.m_input_directory.c_str() );
    this->m_parameter.m_hasfin = false;
  } else {
    pfi_path_server[0] = 0x00;
  }

  // add by @hira at 2016/12/01

  this->m_parameter.m_filter_parameter_filename = argument.m_filter_parameter_filename;
  if (!this->m_parameter.m_filter_parameter_filename.empty()) {
    strcpy(filter_parameter_filename, this->m_parameter.m_filter_parameter_filename.c_str());
    //        g_volumedata->set_text( this->m_parameter.m_filter_parameter_filename.c_str() );
    //        this->setWindowTitle( this->m_parameter.m_filter_parameter_filename.c_str() );
    this->m_parameter.m_hasfin = true;
  } else {
    filter_parameter_filename[0] = 0x00;
  }
#endif
}
/*===========================================================================*/
/**
 * @brief ExtCommand::initCommand
 */
/*===========================================================================*/
void ExtCommand::initCommand() {
  qInfo("ExtCommand::initCommand()");
  // APPEND END   fp)m.tanaka 2013.09.01
#ifdef COMM_MODE_IS
//    inSituInit();
#else
  this->m_parameter.m_particle_limit = RenderoptionPanel::plimitlevel;
  this->m_parameter.m_particle_density = RenderoptionPanel::pdensitylevel;
#endif
  this->m_parameter.m_detailed_repeat_level = repeatleveld;
  this->m_parameter.m_detailed_subpixel_level = subpixelleveld;
  // APPEND START BY)M.Tanaka 2015.03.11
  this->m_parameter.m_sampling_type = static_cast<kvs::visclient::VisualizationParameter::SamplingType>(samplingtypelevel);
  this->m_parameter.m_detailed_transfer_type =
      static_cast<kvs::visclient::VisualizationParameter::DetailedParticleTransferType>(detailed_particle_transfer_typelevel);
  // APPEND END   BY)M.Tanaka 2015.03.11

  sscanf(argument.m_viewer.c_str(), "%fx%f", &RenderoptionPanel::resolution_width_level, &RenderoptionPanel::resolution_height_level);
  // APPEND END   BY)M.Tanaka 2015.03.03

  this->m_parameter.m_abstract_repeat_level = argument.m_repeat_level_abstract;
  this->m_parameter.m_abstract_subpixel_level = argument.m_subpixel_level_abstract;
  this->m_parameter.m_transfer_function = argument.m_transfer_function;

  this->m_parameter.m_hostname = argument.m_hostname;
  this->m_parameter.m_port = argument.m_port;
  this->m_parameter.m_shading_type_flag = argument.m_shading_type_flag;

  if (argument.m_server_subvolume_directory != "") {
    this->m_parameter.m_input_directory = argument.m_server_subvolume_directory;
  } else {
    this->m_parameter.m_input_directory = DEFAULT_VOLUMEDATA;
  }
#ifdef COMM_MODE_CS
  clientServerInit();
#else
  inSituInit();
#endif
  if (argument.m_header == false) {
    this->m_parameter.m_particle_directory = argument.m_header;
    this->m_parameter.m_client_server_mode = 0;
  }

  // pin引数を反映してUIを作成
  for (int i = 0; i < 10; i++) {
    if (argument.m_pin_option_directory[i].empty() != true) {
      this->m_parameter.m_particle_merge_param.m_particles[i + 1].m_file_path.append(argument.m_pin_option_directory[i]);
    }
  }

  // OSAKI
  if (argument.m_polygon_model_flag) {
    this->m_parameter.m_polygon_model_filename = argument.m_polygon_model;
    polygon_model_registered = false;
  }

#ifdef ENABLE_LIBLAS
  if(argument.m_las_model_flag){
      this->m_parameter.m_las_model_filename = argument.m_las_model;
      las_model_registred = false;
  }
#endif
  // OSAKI

  if( argument.m_host_flag )
  {
      this->m_parameter.m_host_flag = argument.m_host_flag;
  }

#ifdef DISP_MODE_VR
  this->m_parameter.m_lefty_mode = argument.m_lefty_mode;
#endif // DISP_MODE_VR
}
/*===========================================================================*/
/**
 * @brief ExtCommand::CallBackApply
 * @param i
 */
/*===========================================================================*/
void ExtCommand::CallBackApply(const int i) {
  if (wk_subpixellevela != subpixellevela) {
    this->m_parameter.m_abstract_subpixel_level = subpixellevela;
    wk_subpixellevela = subpixellevela;
    std::cout << "*** new param.m_abstract_subpixel_level = " << subpixellevela << std::endl;
  }
  if (wk_subpixelleveld != subpixelleveld) {
    this->m_parameter.m_detailed_subpixel_level = subpixelleveld;
    wk_subpixelleveld = subpixelleveld;
    std::cout << "*** new param.m_detailed_subpixel_level = " << subpixelleveld << std::endl;
  }

  if (wk_repeatlevela != repeatlevela) {
    this->m_parameter.m_abstract_repeat_level = repeatlevela;
    wk_repeatlevela = repeatlevela;
    std::cout << "*** new param.m_abstract_repeat_level = " << repeatlevela << std::endl;
  }
  if (wk_repeatleveld != repeatleveld) {
    this->m_parameter.m_detailed_repeat_level = repeatleveld;
    wk_repeatleveld = repeatleveld;
    std::cout << "*** new param.m_detailed_repeat_level = " << repeatleveld << std::endl;
  }

  if (wk_plimitlevel != RenderoptionPanel::plimitlevel) {
    this->m_parameter.m_particle_limit = RenderoptionPanel::plimitlevel;
    wk_plimitlevel = RenderoptionPanel::plimitlevel;
    std::cout << "*** new param.m_particle_limit = " << RenderoptionPanel::plimitlevel << std::endl;
  }

  if (wk_pdensitylevel != RenderoptionPanel::pdensitylevel) {
    this->m_parameter.m_particle_density = RenderoptionPanel::pdensitylevel;
    wk_pdensitylevel = RenderoptionPanel::pdensitylevel;
    std::cout << "*** new param.m_particle_density = " << RenderoptionPanel::pdensitylevel << std::endl;
  }

  if (wk_particle_data_size_limit != RenderoptionPanel::data_size_limit) {
    this->m_parameter.particle_data_size_limit = RenderoptionPanel::data_size_limit;
    wk_particle_data_size_limit = RenderoptionPanel::data_size_limit;
    std::cout << "*** new param.particle_data_size_limit = " << RenderoptionPanel::data_size_limit << std::endl;
  }
  kvs::Vector3f v3;
  if ((wk_translate_x_level != TransformPanel::param.translateX) || (wk_translate_y_level != TransformPanel::param.translateY) ||
      (wk_translate_z_level != TransformPanel::param.translateZ)) {
    v3.set(TransformPanel::param.translateX, TransformPanel::param.translateY, TransformPanel::param.translateZ);
    wk_translate_x_level = TransformPanel::param.translateX;
    wk_translate_y_level = TransformPanel::param.translateY;
    wk_translate_z_level = TransformPanel::param.translateZ;
    std::cout << "*** new param.lookut_center_level = " << TransformPanel::param.translateX << ", " << TransformPanel::param.translateY << ", "
              << TransformPanel::param.translateZ << std::endl;
  }
  if ((wk_rotation_x_axis_level != TransformPanel::param.rotationAxisX) || (wk_rotation_y_axis_level != TransformPanel::param.rotationAxisY) ||
      (wk_rotation_z_axis_level != TransformPanel::param.rotationAxisZ)) {
    v3.set(TransformPanel::param.rotationAxisX, TransformPanel::param.rotationAxisY, TransformPanel::param.rotationAxisZ);
    wk_rotation_x_axis_level = TransformPanel::param.rotationAxisX;
    wk_rotation_y_axis_level = TransformPanel::param.rotationAxisY;
    wk_rotation_z_axis_level = TransformPanel::param.rotationAxisZ;
    std::cout << "*** new param.lookut_eye_level = " << TransformPanel::param.rotationAxisX << ", " << TransformPanel::param.rotationAxisY << ", "
              << TransformPanel::param.rotationAxisZ << std::endl;
  }
  if ((wk_scaling_level != TransformPanel::param.scalingLevel) || (wk_lookat_upvectory_level != lookat_upvectory_level) ||
      (wk_lookat_upvectorz_level != lookat_upvectorz_level)) {
    v3.set(TransformPanel::param.scalingLevel, lookat_upvectory_level, lookat_upvectorz_level);
    wk_scaling_level = TransformPanel::param.scalingLevel;
    wk_lookat_upvectory_level = lookat_upvectory_level;
    wk_lookat_upvectorz_level = lookat_upvectorz_level;
    std::cout << "*** new param.lookut_upvector_level = " << TransformPanel::param.scalingLevel << ", " << lookat_upvectory_level << ", "
              << lookat_upvectorz_level << std::endl;
  }
  // KVS2.7.0
  // MOD BY)T.Osaki 2020.06.06
  // m_screen->kvs::ScreenBase::reset();
  // m_screen->reset(); //camera()->resetXform();
  //    m_screen->Scene::reset();
//  m_screen->objectManager()->translate( kvs::Vector3f(
//                                          TransformPanel::param.translateX,
//                                          TransformPanel::param.translateY,
//                                          TransformPanel::param.translateZ) );
//  m_screen->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
//                                       TransformPanel::param.rotationAxisZ,
//                                       TransformPanel::param.rotationAxisY,
//                                       TransformPanel::param.rotationAxisX) );
//  float s = TransformPanel::param.scalingLevel;
//  m_screen->objectManager()->scale( kvs::Vector3f( s, s, s ) );

  // KVS2.7.0
  // MOD BY)T.Osaki 2020.07.20
  kvs::Vector3f tf_t = kvs::Vector3f(TransformPanel::param.translateX, TransformPanel::param.translateY, TransformPanel::param.translateZ);
  kvs::Matrix33<float> tf_r = kvs::RPYRotationMatrix33<float>(TransformPanel::param.rotationAxisZ, TransformPanel::param.rotationAxisY, TransformPanel::param.rotationAxisX);
  float tf_s = TransformPanel::param.scalingLevel;

#ifdef DISP_MODE_VR
  tf_t *= OBJECT_SCALING_FACTOR;
  tf_s *= OBJECT_SCALING_FACTOR;
#endif // DISP_MODE_VR
//  std::cerr << "DEBUG: disable OBJECT_SCALING_FACTOR in ExtCommand::CallBackApply()" << std::endl;

//  {
//    float xform_after[16];
//    m_screen->scene()->objectManager()->xform().toArray(xform_after);
//    std::cerr << "ExtCommand::CallBackApply()" << std::endl;
//    std::cerr << "before apply" << std::endl;
//    std::cerr << " | " << xform_after[0*4+0] << "  " << xform_after[0*4+1] << "  " << xform_after[0*4+2] << "  " << xform_after[0*4+3] << std::endl;
//    std::cerr << " | " << xform_after[1*4+0] << "  " << xform_after[1*4+1] << "  " << xform_after[1*4+2] << "  " << xform_after[1*4+3] << std::endl;
//    std::cerr << " | " << xform_after[2*4+0] << "  " << xform_after[2*4+1] << "  " << xform_after[2*4+2] << "  " << xform_after[2*4+3] << std::endl;
//    std::cerr << " | " << xform_after[3*4+0] << "  " << xform_after[3*4+1] << "  " << xform_after[3*4+2] << "  " << xform_after[3*4+3] << std::endl;
//  }

  m_screen->scene()->reset();

//  {
//    float xform_after[16];
//    m_screen->scene()->objectManager()->xform().toArray(xform_after);
//    std::cerr << "ExtCommand::CallBackApply()" << std::endl;
//    std::cerr << "reset" << std::endl;
//    std::cerr << " | " << xform_after[0*4+0] << "  " << xform_after[0*4+1] << "  " << xform_after[0*4+2] << "  " << xform_after[0*4+3] << std::endl;
//    std::cerr << " | " << xform_after[1*4+0] << "  " << xform_after[1*4+1] << "  " << xform_after[1*4+2] << "  " << xform_after[1*4+3] << std::endl;
//    std::cerr << " | " << xform_after[2*4+0] << "  " << xform_after[2*4+1] << "  " << xform_after[2*4+2] << "  " << xform_after[2*4+3] << std::endl;
//    std::cerr << " | " << xform_after[3*4+0] << "  " << xform_after[3*4+1] << "  " << xform_after[3*4+2] << "  " << xform_after[3*4+3] << std::endl;
//  }

  m_screen->scene()->objectManager()->translate(tf_t);
  m_screen->scene()->objectManager()->rotate(tf_r);
  m_screen->scene()->objectManager()->scale(kvs::Vector3f(tf_s, tf_s, tf_s));

//  {
//    float xform_after[16];
//    m_screen->scene()->objectManager()->xform().toArray(xform_after);
//    std::cerr << "ExtCommand::CallBackApply()" << std::endl;
//    std::cerr << "after apply" << std::endl;
//    std::cerr << " | " << xform_after[0*4+0] << "  " << xform_after[0*4+1] << "  " << xform_after[0*4+2] << "  " << xform_after[0*4+3] << std::endl;
//    std::cerr << " | " << xform_after[1*4+0] << "  " << xform_after[1*4+1] << "  " << xform_after[1*4+2] << "  " << xform_after[1*4+3] << std::endl;
//    std::cerr << " | " << xform_after[2*4+0] << "  " << xform_after[2*4+1] << "  " << xform_after[2*4+2] << "  " << xform_after[2*4+3] << std::endl;
//    std::cerr << " | " << xform_after[3*4+0] << "  " << xform_after[3*4+1] << "  " << xform_after[3*4+2] << "  " << xform_after[3*4+3] << std::endl;
//  }

  // APPEND START FP)K.YAJIMA & M.Tanaka 2015.03.03
  if (wk_resolution_width_level != RenderoptionPanel::resolution_width_level) {
    this->m_parameter.m_resolution_width = RenderoptionPanel::resolution_width_level;
    wk_resolution_width_level = RenderoptionPanel::resolution_width_level;
    std::cout << "*** new param.m_resolution_width = " << RenderoptionPanel::resolution_width_level << std::endl;
  }
  if (wk_resolution_height_level != RenderoptionPanel::resolution_height_level) {
    this->m_parameter.m_resolution_height = RenderoptionPanel::resolution_height_level;
    wk_resolution_height_level = RenderoptionPanel::resolution_height_level;
    std::cout << "*** new param.m_resolution_height = " << RenderoptionPanel::resolution_height_level << std::endl;
  }
  int i_w = (int)((long)RenderoptionPanel::resolution_width_level);
  int i_h = (int)((long)RenderoptionPanel::resolution_height_level);

  // APPEND START FP)K.YAJIMA 2015.03.17
  if (g_i_w_v2foot != i_w || g_i_h_v2foot != i_h) {
    // 幅、高さのどちらかが現解像度と異なった場合
    // hide(),show()必須

    //        m_screen->hide();

    printf("動的に変化させる\n");
    printf("g_i_w_v2foot = %d , g_i_h_v2foot = %d\n", g_i_w_v2foot, g_i_h_v2foot);
    printf("i_w = %d , i_h = %d\n", i_w, i_h);
    // 動的に変化させる
    m_screen->setGeometry(QRect(0, 0, i_w, i_h));

    printf("DISABLED LABEL POSITION IN FOOTER\n");
    int yl0 = i_h - PBVR_I_H_OFFSET;
  }

  g_i_w_v2foot = i_w;
  g_i_h_v2foot = i_h;

  /* change pfi file */
#ifdef COMM_MODE_IS
  if (false) {
#else
  if (this->m_parameter.m_hasfin) {
#endif
    if (strlen(g_volumedata) > 0 && strcmp(filter_parameter_filename, g_volumedata) != 0) {
      sprintf(filter_parameter_filename, "%s", g_volumedata);
      for (int i = strlen(filter_parameter_filename); i >= 0; i--) {
        if (isspace(filter_parameter_filename[i])) {
          filter_parameter_filename[i] = '\0';
        } else {
          break;
        }
      }
    }
  } else {
    if (strlen(g_volumedata) > 0 && strcmp(pfi_path_server, g_volumedata) != 0) {
      sprintf(pfi_path_server, "%s", g_volumedata);
      for (int i = strlen(pfi_path_server); i >= 0; i--) {
        if (isspace(pfi_path_server[i])) {
          pfi_path_server[i] = '\0';
        } else {
          break;
        }
      }
      int s_len = strlen(pfi_path_server);
      if (pfi_path_server[s_len - 4] == '.' && pfi_path_server[s_len - 3] == 'p' && pfi_path_server[s_len - 2] == 'f' &&
          pfi_path_server[s_len - 1] == 'i') {
        pfi_path_server[s_len - 4] = '\0';
      }
      /* printf(" [debug] Set parameter pressed. pfi_path_server = %s\n", pfi_path_server); */
    }
  }

  // necessary server param extension
#ifdef NOTOPEN
  if (strcmp(wk_volumedata, volumedata) != 0) {
    strcpy(this->param.xxxxx, volumedata);
    wk_volumedata = volumedata;
    std::cout << "*** new param.xxxxx = " << volumedata << std::endl;
  }
  if (wk_cxmaxlevel != cxmaxlevel) {
    this->param.xxxxx = cxmaxlevel;
    wk_cxmaxlevel = cxmaxlevel;
    std::cout << "*** new param.xxxxx = " << cxmaxlevel << std::endl;
  }
  if (wk_cxminlevel != cxminlevel) {
    this->param.xxxxx = cxminlevel;
    wk_cxminlevel = cxminlevel;
    std::cout << "*** new param.xxxxx = " << cxminlevel << std::endl;
  }
  if (wk_cymaxlevel != cymaxlevel) {
    this->param.xxxxx = cymaxlevel;
    wk_cymaxlevel = cymaxlevel;
    std::cout << "*** new param.xxxxx = " << cymaxlevel << std::endl;
  }
  if (wk_cyminlevel != cyminlevel) {
    this->param.xxxxx = cyminlevel;
    wk_cyminlevel = cyminlevel;
    std::cout << "*** new param.xxxxx = " << cyminlevel << std::endl;
  }
  if (wk_czmaxlevel != czmaxlevel) {
    this->param.xxxxx = czmaxlevel;
    wk_czmaxlevel = czmaxlevel;
    std::cout << "*** new param.xxxxx = " << czmaxlevel << std::endl;
  }
  if (wk_czminlevel != czminlevel) {
    this->param.xxxxx = czminlevel;
    wk_czminlevel = czminlevel;
    std::cout << "*** new param.xxxxx = " << czminlevel << std::endl;
  }

  if (wk_translate_x_level != translate_x_level) {
    this->param.xxxxx = translate_x_level;
    wk_translate_x_level = translate_x_level;
    std::cout << "*** new param.xxxxx = " << translate_x_level << std::endl;
  }
  // APPEND START FP)K.YAJIMA 2015.02.27
  if (wk_resolution_width_level != resolution_width_level) {
    this->param.xxxxx = resolution_width_level;
    wk_resolution_width_level = resolution_width_level;
    std::cout << "*** new param.xxxxx = " << resolution_width_level << std::endl;
  }
  if (wk_resolution_height_level != resolution_height_level) {
    this->param.xxxxx = resolution_height_level;
    wk_resolution_height_level = resolution_height_level;
    std::cout << "*** new param.xxxxx = " << resolution_height_level << std::endl;
  }
  // APPEND END FP)K.YAJIMA 2015.02.27
#endif

  this->m_parameter.m_no_repsampling = SystemstatusPanel::m_no_repsampling;
  this->m_glut_timer->setInterval(SystemstatusPanel::interval_msec);

  // OSAKI
  std::cout << "ExtCommand::CallBackApply() : register polygon model : " << m_parameter.m_polygon_model_filename << std::endl;
  if (this->m_parameter.m_polygon_model_filename != "") {
    this->registerPolygonModel();
    polygon_model_registered = true;
  }

#ifdef ENABLE_LIBLAS
  if(this->m_parameter.m_las_model_filename != "") {
      this->registerLASModel();
      las_model_registred = true;
      las_registerd = true;
  }
#endif

  // OSAKI

#ifdef DISP_MODE_VR
  PBVRWidgetHandler *widget_handler = PBVRWidgetHandlerInstance();
  widget_handler->setLeftyMode (this->m_parameter.m_lefty_mode);
#endif // DISP_MODE_VR

  return;
}

template <typename OBJECT, typename RENDERER>
inline static void registerPolygon(ExtCommand* cmd, OBJECT* importer, RENDERER* renderer)
{
#ifdef ENABLE_EXTCRD_SCALING_FOR_DEMO
    //CGモデルを、粒子モデル同レベルのサイズ、位置で表示するための座標値を計算する

    //粒子モデルのデータ
    const kvs::Vec3 particleMin = kvs::Vector3f(4.06067, 0, -8.67);
    const kvs::Vec3 particleMax = kvs::Vector3f(10,6.94658, 0.725);
    //        const kvs::Vec3 particleMin = kvs::Vector3f(-179.542, -109.5, 0.0);
    //        const kvs::Vec3 particleMax = kvs::Vector3f(388.83, 109.5, 388.83);
    const kvs::Vec3 particleBoxLength = particleMax - particleMin;
    const kvs::Vec3 particleCenter = (particleMax + particleMin) / 2.0f;

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
    std::cout << "Object###" << importer->minObjectCoord() << "," << importer->maxObjectCoord() << std::endl;
    std::cout << "External###" << importer->minExternalCoord() << "," << importer->maxExternalCoord() << std::endl;

    OBJECT* polygon = importer;

    #ifdef ENABLE_BENCHMARK_POLYGON
    int nDuplicate = 15;
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

    polygon = new OBJECT;
    polygon->deepCopy(*importer);
    polygon->setConnections(connectionsNew);

    std::cout << "registerPolygonModel(kvs::PolygonImoprter) : benchmark mode (nDuplicate=" << nDuplicate << ")" << std::endl;
    std::cout << "       nvertices=" << (polygon->coords().size() / 3) << std::endl;
    std::cout << "       npolygons=" << nPolygonsNew << std::endl;

    #endif // ENABLE_BENCHMARK_POLYGON

    cmd->m_screen->scene()->registerObject(polygon, renderer);
}

// OSAKI
bool ExtCommand::registerPolygonModel () {
    const std::string filePath = this->m_parameter.m_polygon_model_filename;
    const std::string extension = filePath.substr(filePath.length()-4, 4);

    kvs::FileFormatBase* io = nullptr;
    bool polygon_is_textured = false;

    // select importer
    if(extension == ".stl"){
        io = new kvs::Stl;
    }
#ifdef ENABLE_ASSIMP
    else if(extension == ".3ds"){
        io = new kvs::jaea::ThreeDS();
        polygon_is_textured = true;
    }
#endif
#ifdef ENABLE_FBXSDK
    else if(extension == ".fbx"){
        io = new kvs::jaea::FBX();
        polygon_is_textured = true;
    }
#endif
    else {
        std::cerr << "Unknown file format:" << filePath << std::endl;
        return false;
    }

    io->read(filePath);

    bool ret;
    if((ret = io->isSuccess())){
        std::cout << "loading " << io->filename() << " succeed." << std::endl;

        if (polygon_is_textured)
        {
            kvs::jaea::TexturedPolygonImporter* importer = new kvs::jaea::TexturedPolygonImporter();
            importer->exec(io);
            importer->setName (OBJ_NAME_CGMODEL);
            importer->updateMinMaxCoords();

            registerPolygon<kvs::jaea::TexturedPolygonObject, StochasticTexturedPolygonRenderer>(this, importer, new StochasticTexturedPolygonRenderer());
        }
        else
        {
            kvs::PolygonImporter* importer = new kvs::PolygonImporter();
            importer->exec(io);
            importer->setOpacity(100);
            importer->setName (OBJ_NAME_POLYGONMODEL);
            importer->updateMinMaxCoords();

            registerPolygon<kvs::PolygonObject, kvs::StochasticPolygonRenderer>(this, importer, new kvs::StochasticPolygonRenderer());
        }
    }else{
        std::cerr << "ERROR : loading " << io->filename() << " failed." << std::endl;
    }

    delete io;

    return ret;
}
// OSAKI

#ifdef ENABLE_LIBLAS
void ExtCommand::registerLASModel()
{
    const std::string filePath = this->m_parameter.m_las_model_filename;
    const std::string extension = filePath.substr(filePath.length()-4,4);

    kvs::FileFormatBase* io = nullptr;
    if(extension == ".las")
    {

    }
//    LASImporter* importer = new LASImporter(filePath);
    kvs::PointObject* las_point = new LASImporter(filePath);
    las_point->setName("LAS_MODEL");
    kvs::glsl::ParticleBasedRenderer* las_renderer = new kvs::glsl::ParticleBasedRenderer();
    las_renderer->setShader( kvs::Shader::Lambert( 1,0 ) );
    this->m_screen->scene()->registerObject( las_point, las_renderer );


}
#endif

// OSAKI
#ifdef GPU_MODE
void ExtCommand::registerPolygonModel(std::string str, int currentIndex, double opacity, kvs::RGBColor color, bool isSTL) {
  std::cout << "[DEBUG_REG]" << str << std::endl;
  const std::string filePath = str;
  const std::string extension = filePath.substr(filePath.length() - 4, 4);

  kvs::FileFormatBase *io = nullptr;

  // select importer
  //    if(extension == ".stl"){
  //        io = new kvs::Stl;
  //    }else if( extension == ".kvsml"){
  //    }else{
  //        std::cerr << "Unknown file format:" << filePath << std::endl;
  //        return;
  //    }

  kvs::KVSMLPolygonObject *kvsml;
  kvs::PolygonObject *m_polygon_object_empty = new kvs::PolygonObject(); // 空オブジェクト
  kvs::PolygonObject *m_polygon_imp_object;
  if (isSTL == true) {
    std::cout << __LINE__ << std::endl;
    m_polygon_imp_object = new kvs::PolygonImporter(filePath);
  } else {
    kvsml = new kvs::KVSMLPolygonObject(filePath);
    std::cout << __LINE__ << std::endl;
    m_polygon_imp_object = new kvs::PolygonImporter(kvsml);
  }

  std::string ObjNameEmpty = "POLYGON_OBJ_EMPTY" + std::to_string(currentIndex - 5);
  std::string ObjNameImp = "POLYGON_OBJ_IMP" + std::to_string(currentIndex - 5);

  m_polygon_object_empty->setXform(m_screen->scene()->objectManager()->xform());
  m_polygon_object_empty->setMinMaxObjectCoords(m_polygon_imp_object->minObjectCoord(), m_polygon_imp_object->maxObjectCoord());
  m_polygon_object_empty->setMinMaxExternalCoords(m_polygon_imp_object->minExternalCoord(), m_polygon_imp_object->maxExternalCoord());

  m_polygon_object_empty->setName(ObjNameEmpty);
  m_polygon_imp_object->setName(ObjNameImp);
  m_polygon_imp_object->setOpacity(opacity * 255);
  std::cout << isSTL << std::endl;
  if (isSTL == true) {
    m_polygon_imp_object->setColor(color);
  } else {
    std::cout << __LINE__ << std::endl;
  }
  kvs::StochasticPolygonRenderer *renderer = new kvs::StochasticPolygonRenderer();
  if (m_screen->scene()->hasObject(ObjNameEmpty) == false && m_screen->scene()->hasObject(ObjNameImp) == false) {
    m_screen->scene()->registerObject(m_polygon_object_empty, renderer);
    m_screen->scene()->replaceObject(ObjNameEmpty, m_polygon_imp_object, false);
  }

  if (m_screen->scene()->hasObject(ObjNameImp) == true) {
    m_screen->scene()->replaceObject(ObjNameImp, m_polygon_imp_object, false);
  }

  if (m_screen->scene()->hasObject(ObjNameEmpty) == true) {
    m_screen->scene()->replaceObject(ObjNameEmpty, m_polygon_imp_object, false);
  }
}
#endif

#ifdef GPU_MODE
void ExtCommand::registerEmptyPolygonModel(std::string str, int currentIndex, double opacity, kvs::RGBColor color) {
  const std::string filePath = str;
  const std::string extension = filePath.substr(filePath.length() - 4, 4);

  kvs::FileFormatBase *io = nullptr;

  // select importer
  //    if(extension == ".stl"){
  //        io = new kvs::Stl;
  //    }else if( extension == ".kvsml"){
  //    }else{
  //        std::cerr << "Unknown file format:" << filePath << std::endl;
  //        return;
  //    }

  kvs::PolygonObject *m_polygon_object_empty = new kvs::PolygonObject(); // 空オブジェクト
  kvs::PolygonObject *m_polygon_imp_object = new kvs::PolygonImporter(filePath);

  std::string ObjNameEmpty = "POLYGON_OBJ_EMPTY" + std::to_string(currentIndex - 5);
  std::string ObjNameImp = "POLYGON_OBJ_IMP" + std::to_string(currentIndex - 5);

  m_polygon_object_empty->setXform(m_screen->scene()->objectManager()->xform());
  m_polygon_object_empty->setMinMaxObjectCoords(m_polygon_imp_object->minObjectCoord(), m_polygon_imp_object->maxObjectCoord());
  m_polygon_object_empty->setMinMaxExternalCoords(m_polygon_imp_object->minExternalCoord(), m_polygon_imp_object->maxExternalCoord());

  m_polygon_object_empty->setName(ObjNameEmpty);
  m_polygon_imp_object->setName(ObjNameImp);
  m_polygon_imp_object->setOpacity(opacity * 255);
  m_polygon_imp_object->setColor(color);

  kvs::StochasticPolygonRenderer *renderer = new kvs::StochasticPolygonRenderer();
  if (m_screen->scene()->hasObject(ObjNameEmpty) == false && m_screen->scene()->hasObject(ObjNameImp) == false) {
    m_screen->scene()->registerObject(m_polygon_object_empty, renderer);
    m_screen->scene()->replaceObject(ObjNameEmpty, m_polygon_object_empty, false);
  }

  if (m_screen->scene()->hasObject(ObjNameEmpty) == true) {
    m_screen->scene()->replaceObject(ObjNameEmpty, m_polygon_object_empty, false);
  }

  if (m_screen->scene()->hasObject(ObjNameImp) == true) {
    m_screen->scene()->replaceObject(ObjNameImp, m_polygon_object_empty, false);
  }
}
#endif
// OSAKI

#ifdef GPU_MODE
void ExtCommand::deletePolygonModel(int currentIndex) {
  std::string ObjName = "POLYGON_MODEL" + std::to_string(currentIndex - 6);
  std::string ObjNameEmpty = "POLYGON_OBJ_EMPTY" + std::to_string(currentIndex - 5);
  std::string ObjNameImp = "POLYGON_OBJ_IMP" + std::to_string(currentIndex - 5);
  if (this->m_screen->scene()->hasObject(ObjName) == true) {
    this->m_screen->scene()->removeObject(ObjName, false, false);
  }
  if (this->m_screen->scene()->hasObject(ObjNameEmpty) == true) {
    this->m_screen->scene()->removeObject(ObjNameEmpty, false, false);
  }
  if (this->m_screen->scene()->hasObject(ObjNameImp) == true) {
    this->m_screen->scene()->removeObject(ObjNameImp, false, false);
  }
}
#endif

void ExtCommand::registerGlyph( int port )
{
  std::string ObjName = "GLYPH";
  if( m_screen->scene()->hasObject( ObjName ) != true )
  {
    int n_glyphs = 1;
    kvs::ValueArray<kvs::Real32> pos;
    pos.allocate(3*n_glyphs);
//    57.9533,60.5863,104.576
//    pos.at(0) = 57.9533;
//    pos.at(1) = 60.5863;
//    pos.at(2) = 104.576;
    pos.at(0) = 0;
    pos.at(1) = 0;
    pos.at(2) = 0;
    kvs::PolygonObject* glyph_polygon = new kvs::SphereGlyph( n_glyphs, pos );
    pos.release();
    glyph_polygon->setName( ObjName );
    glyph_polygon->setColor( kvs::RGBColor( 0, 255, 0 ) );
    glyph_polygon->setOpacity( 255 );
    kvs::StochasticPolygonRenderer *renderer = new kvs::StochasticPolygonRenderer();
    m_screen->scene()->registerObject( glyph_polygon, renderer );
    std::cout << __LINE__ << std::endl;
    std::cout << glyph_polygon->minObjectCoord() << std::endl;
    std::cout << glyph_polygon->maxObjectCoord() << std::endl;
    std::cout << glyph_polygon->minExternalCoord() << std::endl;
    std::cout << glyph_polygon->maxExternalCoord() << std::endl;
  }
//  else if( m_screen->scene()->hasObject( ObjName ) != true && port != 60000 )
//  {
//    //デーモン側から送られてくる座標を取得してリプレイスオブジェクトしてください。
//    int n_glyphs = 1;
//    kvs::ValueArray<kvs::Real32> pos;
//    pos.allocate(3*n_glyphs);

////    pos.at(0) = 0;//
////    pos.at(1) = 0;//
////    pos.at(2) = 0;//

//    pos.at(0) = this->m_result.arrow_component[0];//
//    pos.at(1) = this->m_result.arrow_component[1];//
//    pos.at(2) = this->m_result.arrow_component[2];//

//    kvs::PolygonObject* glyph_polygon = new kvs::SphereGlyph( n_glyphs, pos );
//    pos.release();

//    m_screen->scene()->replaceObject( ObjName, glyph_polygon );
//  }
}

void ExtCommand::updateGuestGlyph()
{
  std::string ObjName = "GLYPH";
  if( m_screen->scene()->hasObject( ObjName ) )
  {
    //デーモン側から送られてくる座標を取得してリプレイスオブジェクトしてください。
    int n_glyphs = 1;
    kvs::ValueArray<kvs::Real32> pos;
    pos.allocate(3*n_glyphs);

//        pos.at(0) = 28.0069;//
//        pos.at(1) = 27.5624;//
//        pos.at(2) = 71.0342;//

    pos.at(0) = this->m_result.arrow_component[0];//
    pos.at(1) = this->m_result.arrow_component[1];//
    pos.at(2) = this->m_result.arrow_component[2];//

    kvs::PolygonObject* glyph_polygon = new kvs::SphereGlyph( n_glyphs, pos );
    glyph_polygon->setColor( kvs::RGBColor( 0, 255, 0 ) );
    pos.release();

    m_screen->scene()->replaceObject( ObjName, glyph_polygon, false );
  }
}
