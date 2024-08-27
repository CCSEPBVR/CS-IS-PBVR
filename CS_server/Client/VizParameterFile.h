#ifndef PBVR__VIZ_PARAMETER_FILE_H_INCLUDE
#define PBVR__VIZ_PARAMETER_FILE_H_INCLUDE

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fstream>
#include "Types.h"
#include "GL/glui.h"
#include <kvs/RGBColor>

extern GLUI* glui_3vd;

extern int subpixellevela;
extern int subpixelleveld;
extern int repeatlevela;
extern int repeatleveld;
extern int plimitlevel;
extern int samplingtypelevel;
extern float rotation_x_axis_level;
extern float rotation_y_axis_level;
extern float rotation_z_axis_level;
extern float translate_x_level;
extern float translate_y_level;
extern float translate_z_level;
extern float scaling_level;
extern float lookat_upvectory_level;
extern float lookat_upvectorz_level;
extern float resolution_width_level;
extern float resolution_height_level;
extern char pfi_path_server[256];
extern char filter_parameter_filename[256];		// add by @hira at 2016/12/01

extern int   obj_3d;
extern float tbcxminlevel;
extern float tbcxmaxlevel;
extern float tbcyminlevel;
extern float tbcymaxlevel;
extern float tbczminlevel;
extern float tbczmaxlevel;

extern float tbsradius;
extern float tbscenterx;
extern float tbscentery;
extern float tbscenterz;

extern float tbpradius;
extern float tbpheight;
extern float tbpcenterx;
extern float tbpcentery;
extern float tbpcenterz;


extern GLUI_EditText* subpixeld;
extern GLUI_EditText* repeatd;
extern GLUI_EditText* plimit;
extern GLUI_EditText* rotation_x_axis;
extern GLUI_EditText* rotation_y_axis;
extern GLUI_EditText* rotation_z_axis;
extern GLUI_EditText* translate_x;
extern GLUI_EditText* translate_y;
extern GLUI_EditText* translate_z;
extern GLUI_EditText* scaling;
extern GLUI_EditText* lookat_upvectory;
extern GLUI_EditText* lookat_upvectorz;
extern GLUI_EditText* resolution_width;
extern GLUI_EditText* resolution_height;

extern GLUI_EditText* crop_cxmin2;
extern GLUI_EditText* crop_cxmax2;
extern GLUI_EditText* crop_cymin2;
extern GLUI_EditText* crop_cymax2;
extern GLUI_EditText* crop_czmin2;
extern GLUI_EditText* crop_czmax2;

extern GLUI_EditText* sphere_radius;
extern GLUI_EditText* sphere_centerx;
extern GLUI_EditText* sphere_centery;
extern GLUI_EditText* sphere_centerz;

extern GLUI_EditText* pillar_radius;
extern GLUI_EditText* pillar_height;
extern GLUI_EditText* pillar_centerx;
extern GLUI_EditText* pillar_centery;
extern GLUI_EditText* pillar_centerz;

extern void CallBackCropMinLevelX( const int num );
extern void CallBackCropMinLevelY( const int num );
extern void CallBackCropMinLevelZ( const int num );
extern void CallBackCropMaxLevelX( const int num );
extern void CallBackCropMaxLevelY( const int num );
extern void CallBackCropMaxLevelZ( const int num );
extern void CallBackSphereRadius( const int num );
extern void CallBackSphereCenterX( const int num );
extern void CallBackSphereCenterY( const int num );
extern void CallBackSphereCenterZ( const int num );
extern void CallBackPillarRadius( const int num );
extern void CallBackPillarHeight( const int num );
extern void CallBackPillarCenterX( const int num );
extern void CallBackPillarCenterY( const int num );
extern void CallBackPillarCenterZ( const int num );

// APPEND START FP)M.TANAKA 2015.03.16
extern float g_ro[3][3];
extern float g_tr[3];
extern float g_sc[3];
// APPEND END   FP)M.TANAKA 2015.03.16

extern std::string param_x_synthesis;
extern std::string param_y_synthesis;
extern std::string param_z_synthesis;
extern std::string param_fonttype;
extern kvs::RGBColor crgb;

class VizParameterFile
{

public:
//	VizParameterFile();
    static int ReadParamFile( const char* fname, const int readflag = 1 );
    static int WriteParamFile( const char* fname );
    static int UpdateWin3v( const char* fname );
};

#endif // PBVR__VIZ_PARAMETER_FILE_H_INCLUDE
