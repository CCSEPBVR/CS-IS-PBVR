#ifndef PBVR__V2_HEAD_H_INCLUDE
#define PBVR__V2_HEAD_H_INCLUDE

#include <fstream>

#include "GL/glui.h"
#define PBVR_GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define PBVR_GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

#define PBVR_SUB_PIXEL_LEVEL_ABSTRACT  2
#define PBVR_SUB_PIXEL_LEVEL_DETAILED  2
#define PBVR_REPEAT_LEVEL_ABSTRACT  1
//#define PBVR_REPEAT_LEVEL_DETAILED 16
#define PBVR_REPEAT_LEVEL_DETAILED 1 /* change 2014.07.21 */
#define PBVR_PARTICLE_LIMIT 10000000 /* change 1 to 10 2015.03.26 */
//#define PBVR_PARTICLE_LIMIT 1000 /* change 1 to 10 2015.03.26 */
#define PBVR_PARTICLE_DENSITY 1.0f

#define PBVR_ROTATION_X_AXIS_LEVEL 0.0F
#define PBVR_ROTATION_Y_AXIS_LEVEL 0.0F
#define PBVR_ROTATION_Z_AXIS_LEVEL 0.0F
#define PBVR_TRANSLATE_X_LEVEL 0.0F
#define PBVR_TRANSLATE_Y_LEVEL 0.0F
#define PBVR_TRANSLATE_Z_LEVEL 0.0F
#define PBVR_SCALING_LEVEL 1.0F
#define PBVR_LOOKAT_UPVECTORY_LEVEL 0.0F
#define PBVR_LOOKAT_UPVECTORZ_LEVEL 0.0F
// APPEND START FP)K.YAJIMA 2015.02.27
#define PBVR_RESOLUTION_WIDTH_LEVEL 620.0F
#define PBVR_RESOLUTION_HEIGHT_LEVEL 620.0F
// APPEND END FP)K.YAJIMA 2015.02.27
#define PBVR_RESOLUTION_HEIGHT_DEVICE 900

#define PBVR_CX_MAX_LEVEL 0.0F
#define PBVR_CX_MIN_LEVEL 0.0F
#define PBVR_CY_MAX_LEVEL 0.0F
#define PBVR_CY_MIN_LEVEL 0.0F
#define PBVR_CZ_MAX_LEVEL 0.0F
#define PBVR_CZ_MIN_LEVEL 0.0F

#include "CropParameter.h"
#define PBVR_CROP_CXMIN 1.0
#define PBVR_CROP_CXMAX 1.0
#define PBVR_CROP_CYMIN 1.0
#define PBVR_CROP_CYMAX 1.0
#define PBVR_CROP_CZMIN 1.0
#define PBVR_CROP_CZMAX 1.0

#define PBVR_DATAFILE "data"  // /tmp/data_20140311101010.dat
#define PBVR_LIMIT_DATA_FILE "limit" // /tmp/limit.dat
#define PBVR_DOCK_H 40  // for right or left Dock of Mac and Linux
//#define DOCK_H 132 // for bottom Dock of Mac
#define PBVR_I_H_OFFSET 55

void CallBackTransferFunctionEditorPanelShow( const int i );
void CallBackParticlePanelShow( const int i );
void CallBackAnimationPanelShow( const int i );
void CallBackLagendPanelShow( const int i );
void CallBackCoordinatePanelShow( const int i );
void CallBackViewerPanelShow( const int i );

void CallBackParticleLimitLevel( const int i );
void CallBackParticleDensityLevel( const int i );
void CallBackLookAtEyeX( const int i );
void CallBackLookAtEyeY( const int i );
void CallBackLookAtEyeZ( const int i );
void CallBackLookAtCenterX( const int i );
void CallBackLookAtCenterY( const int i );
void CallBackLookAtCenterZ( const int i );
void CallBackLookAtUpvectorX( const int i );
void CallBackLookAtUpvectorY( const int i );
void CallBackLookAtUpvectorZ( const int i );
// APPEND START FP)K.YAJIMA 2015.03.02
void CallBackResolutionWidth( const int i );
void CallBackResolutionHeight( const int i );
// APPEND END FP)K.YAJIMA 2015.03.02
void CallBackNoRepeatSampling( const int i );
void CallBackApply( const int i );
void CallBackParameterFile( const int i );
// tantantan
void CallBackReadFile( const int i );
// tantantan
void InitializePanel();

using kvs::visclient::CropParameter;
void SetCropParameter( CropParameter* param );
CropParameter* GetCropParameter();

#ifdef WIN32
char buff_paramfile[100] = "param.in";
char buff_capfile[100] = "c:\\temp\\hogehoge.bmp";
char buff_workdir[100] = ".";
#else
char buff_paramfile[100] = "./param.in";
char buff_capfile[100] = "/tmp/hogehoge.bmp";
char buff_workdir[100] = "/tmp";
#endif
char data_filename[100];
char limitdata_filename[100];
//int data_cnt = 0;
bool data_output = false;

int subpixellevela = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
int subpixelleveld = PBVR_SUB_PIXEL_LEVEL_DETAILED;
int repeatlevela = PBVR_REPEAT_LEVEL_ABSTRACT;
int repeatleveld = PBVR_REPEAT_LEVEL_DETAILED;
int plimitlevel = PBVR_PARTICLE_LIMIT;
float pdensitylevel = PBVR_PARTICLE_DENSITY;
// APPEND START FP 2015.02.27
// APPEND START BY)M.Tanaka 2015.03.11
int samplingtypelevel = 1;
int detailed_particle_transfer_typelevel = 1;
char shadinglevel[256] = "";
// APPEND END   BY)M.Tanaka 2015.03.11
char pfi_path_server[256];
// APPEND END   FP 2015.02.27
// add by @hira at 2016/12/01
char filter_parameter_filename[256];

int wk_subpixellevela = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
int wk_subpixelleveld = PBVR_SUB_PIXEL_LEVEL_DETAILED;
int wk_repeatlevela = PBVR_REPEAT_LEVEL_ABSTRACT;
int wk_repeatleveld = PBVR_REPEAT_LEVEL_DETAILED;
int wk_plimitlevel = PBVR_PARTICLE_LIMIT;
float wk_pdensitylevel = PBVR_PARTICLE_DENSITY;


float cxmaxlevel = PBVR_CX_MAX_LEVEL;
float cxminlevel = PBVR_CX_MIN_LEVEL;
float cymaxlevel = PBVR_CY_MAX_LEVEL;
float cyminlevel = PBVR_CY_MIN_LEVEL;
float czmaxlevel = PBVR_CZ_MAX_LEVEL;
float czminlevel = PBVR_CZ_MIN_LEVEL;

float wk_cxmaxlevel = PBVR_CX_MAX_LEVEL;
float wk_cxminlevel = PBVR_CX_MIN_LEVEL;
float wk_cymaxlevel = PBVR_CY_MAX_LEVEL;
float wk_cyminlevel = PBVR_CY_MIN_LEVEL;
float wk_czmaxlevel = PBVR_CZ_MAX_LEVEL;
float wk_czminlevel = PBVR_CZ_MIN_LEVEL;


float rotation_x_axis_level = PBVR_ROTATION_X_AXIS_LEVEL;
float rotation_y_axis_level = PBVR_ROTATION_Y_AXIS_LEVEL;
float rotation_z_axis_level = PBVR_ROTATION_Z_AXIS_LEVEL;
float translate_x_level = PBVR_TRANSLATE_X_LEVEL;
float translate_y_level = PBVR_TRANSLATE_Y_LEVEL;
float translate_z_level = PBVR_TRANSLATE_Z_LEVEL;
float scaling_level = PBVR_SCALING_LEVEL;
float lookat_upvectory_level = PBVR_LOOKAT_UPVECTORY_LEVEL;
float lookat_upvectorz_level = PBVR_LOOKAT_UPVECTORZ_LEVEL;
// APPEND START FP)K.YAJIMA 2015.02.27
float resolution_width_level = PBVR_RESOLUTION_WIDTH_LEVEL;
float resolution_height_level = PBVR_RESOLUTION_HEIGHT_LEVEL;
// APPEND END FP)K.YAJIMA 2015.02.27
int resolution_height_device = PBVR_RESOLUTION_HEIGHT_DEVICE;

float wk_rotation_x_axis_level = PBVR_ROTATION_X_AXIS_LEVEL;
float wk_rotation_y_axis_level = PBVR_ROTATION_Y_AXIS_LEVEL;
float wk_rotation_z_axis_level = PBVR_ROTATION_Z_AXIS_LEVEL;
float wk_translate_x_level = PBVR_TRANSLATE_X_LEVEL;
float wk_translate_y_level = PBVR_TRANSLATE_Y_LEVEL;
float wk_translate_z_level = PBVR_TRANSLATE_Z_LEVEL;
float wk_scaling_level = PBVR_SCALING_LEVEL;
float wk_lookat_upvectory_level = PBVR_LOOKAT_UPVECTORY_LEVEL;
float wk_lookat_upvectorz_level = PBVR_LOOKAT_UPVECTORZ_LEVEL;
// APPEND START FP)K.YAJIMA 2015.02.27
float wk_resolution_width_level = PBVR_RESOLUTION_WIDTH_LEVEL;
float wk_resolution_height_level = PBVR_RESOLUTION_HEIGHT_LEVEL;
// APPEND END FP)K.YAJIMA 2015.02.27

GLUI_StaticText* lbl_sending;
GLUI_StaticText* lbl_recieving;
GLUI_StaticText* lbl_smemory;
GLUI_StaticText* lbl_gmemory;
kvs::visclient::Command* g_command;
kvs::glut::Screen* g_screen;
GLUI_EditText* subpixela;
GLUI_EditText* subpixeld;
GLUI_EditText* repeata;
GLUI_EditText* repeatd;
GLUI_EditText* plimit;
GLUI_EditText* pdensity;
GLUI_EditText* crop_cxmin;
GLUI_EditText* crop_cxmax;
GLUI_EditText* crop_cymin;
GLUI_EditText* crop_cymax;
GLUI_EditText* crop_czmin;
GLUI_EditText* crop_czmax;
// tantantan
GLUI_EditText* rotation_x_axis;
GLUI_EditText* rotation_y_axis;
GLUI_EditText* rotation_z_axis;
GLUI_EditText* translate_x;
GLUI_EditText* translate_y;
GLUI_EditText* translate_z;
GLUI_EditText* scaling;
GLUI_EditText* lookat_upvectory;
GLUI_EditText* lookat_upvectorz;
GLUI_EditText* resolution_width;
GLUI_EditText* resolution_height;
// tantantan
GLUI_EditText* g_volumedata;    /* for PFI file */
GLUI_Checkbox* m_no_repsampling;
GLUI_EditText* interval_msec;

GLUI_StaticText* lbl_numParticles;
GLUI_StaticText* lbl_numNodes;
GLUI_StaticText* lbl_numElements;
GLUI_StaticText* lbl_elemType;
GLUI_StaticText* lbl_fileType;
GLUI_StaticText* lbl_numIngredients;
GLUI_StaticText* lbl_numStep;
GLUI_StaticText* lbl_numVolDiv;
GLUI_StaticText* lbl_objectCoord0;
GLUI_StaticText* lbl_objectCoord1;
GLUI_StaticText* lbl_objectCoord2;
//kvs::glut::Screen hc;

// APPEND START FP)M.TANAKA 2015.03.16
float g_ro[3][3];
float g_tr[3];
float g_sc[3];
// APPEND END   FP)M.TANAKA 2015.03.16
size_t c_smemory;

std::string param_x_synthesis;
std::string param_y_synthesis;
std::string param_z_synthesis;
std::string param_fonttype;
kvs::RGBColor crgb( 200, 200, 210 );     // Background Color Data
int start_store_step = -1;

#endif //PBVR__V2_HEAD_H_INCLUDE
