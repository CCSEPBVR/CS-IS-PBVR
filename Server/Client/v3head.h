#ifndef PBVR__V3_HEAD_H_INCLUDE
#define PBVR__V3_HEAD_H_INCLUDE

#ifdef NOTOPEN
#include <stdio.h>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "GL/glui.h"

#else
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/PolygonObject>
#endif

#include "CropParameter.h"

// APPEND START FP)M.Tanaka 2015.03.02
#include "FileDialog.h"
GLUI_EditText* g_paramfile;
// APPEND END   FP)M.Tanaka 2015.03.02

//テスト用
//---------------------------------

void	Display();
int		iwin_main;

//---------------------------------

void	CalculateCropCoord();
void	CalculateSphereCoord();
void	CalculatePillarCoord();
void	CalculatePillarXZCoord();
void	CalculatePillarYZCoord();

void	DrawCrop( const int type );
void	DrawSphere( const int type );
void	DrawPillar( const int type );

// APPEND START FP)K.YAJIMA 2015.02.26
GLUI_FileBrowser* fb;
// APPEND END FP)K.YAJIMA 2015.02.26

//描画するオブジェクトの種類

#define	PBVR_CROP		1
#define	PBVR_SPHERE		2
#define	PBVR_PILLAR		3
#define	PBVR_PILLARXY	3
#define	PBVR_PILLARYZ	4
#define	PBVR_PILLARXZ	5

//視点
#define	PBVR_NO_SEL	0			//未選択
#define	PBVR_X_VIEW	1			//X軸視点
#define	PBVR_Y_VIEW	2			//Y軸視点
#define	PBVR_Z_VIEW	3			//Z軸視点

//ボタンの状態
#define	PBVR_ON	1
#define	PBVR_OFF	0

//フラグの状態
#define	PBVR_FALSE	0
#define	PBVR_TRUE	1

//メインウィンドウ・サイズ(テスト用）
#define	PBVR_MAINPBVR_WINDOW_W	300
#define	PBVR_MAINPBVR_WINDOW_H	300

//デフォルトウィンドウ・サイズ
#define	PBVR_WINDOW_W		660
#define	PBVR_WINDOW_H		660

//ビューポート・サイズ(ウィンドウ・サイズの 1 / 2)
#define	PBVR_VIEWPORT_WIDTH	330
#define	PBVR_VIEWPORT_HEIGHT	330

//法線の長さ
#define	PBVR_VEC_LEN			0.8

//法線の表示領域のサイズ
#define	PBVR_VECTOR_VIEW_SIZE	80

//π
#define	PBVR_PI			3.1415

//拡大/縮小の割合
#define	PBVR_REDSCALE		0.9F		//縮小スケール
#define	PBVR_MAGSCALE		1.1F		//拡大スケール
#define	PBVR_SCALE_MAX		2.0F		//拡大最大倍率
#define	PBVR_SCALE_MIN		0.01F		//拡大最小倍率

#define	PBVR_COORDINATES_MAX	1.0F		//座標の最大値
#define	PBVR_COORDINATES_MIN	-1.0F		//	〃	最小値

#define	PBVR_RADIUS_MAX	1.0F		//球体・円柱の半径最大値
#define	PBVR_RADIUS_MIN	0.0F	//	0.01F		//		〃		  最小値
#define	PBVR_HEIGHTMAX	2.0F	//円柱の高さ最大値
#define	PBVR_HEIGHTMIN	0.0F	//		0.01F		//		〃	最小値

//SIDE数
//#define	SIDES		8
#define	PBVR_SIDES		16

//PBVRウィンドウ用倍率
#define	PBVR_SCALE	3.0F
//#define	PBVR_SCALE	5.0F

//3VDウィンドウ用倍率
#define	PBVR_WIN3VDSCALE	0.5F

//CROPのデフォルト値

#define	PBVR_CROP_X_MIN	-0.5F
#define	PBVR_CROP_X_MAX	0.5F
#define	PBVR_CROP_Y_MIN	-0.5F
#define	PBVR_CROP_Y_MAX	0.5F
#define	PBVR_CROP_Z_MIN	-0.5F
#define	PBVR_CROP_Z_MAX	0.5F
/*
#define PBVR_CROP_X_MIN        -0.714F
#define PBVR_CROP_X_MAX         0.714F
#define PBVR_CROP_Y_MIN        -0.357F
#define PBVR_CROP_Y_MAX         0.357F
#define PBVR_CROP_Z_MIN        -1.0F
#define PBVR_CROP_Z_MAX         1.0F
*/
//PBVR_SPHEREのデフォルト値
#define	PBVR_SRADIUS	1.0F //0.5F

//PILLARのデフォルト値
#define	PBVR_PRADIUS	1.0F //0.5F
#define	PBVR_PHEIGHT	1.0F //0.8F

//変形、移動処理のモード
#define PBVR_BIT(num)		((unsigned int)1 << (num))
#define PBVR_MODIFIMODE		PBVR_BIT(0)				//変形モードON
#define PBVR_RIGHTSIDE		PBVR_BIT(1)				//右サイド
#define PBVR_LEFTSIDE		PBVR_BIT(2)				//左サイド
#define PBVR_TOP_SIDE		PBVR_BIT(3)				//上サイド
#define PBVR_BOTTOM_SIDE	PBVR_BIT(4)				//下サイド
#define PBVR_SURFSIDE		PBVR_BIT(5)				//前サイド
#define PBVR_DEPTH_SIDE		PBVR_BIT(6)				//奥サイド

#define PBVR_SOLID			0					//オブジェクトの塗りつぶし指定
#define PBVR_WIRE			1					//ワイヤーフレーム
#define PBVR_ALPHA			0.3F				//アルファ値

// cropping parameter
CropParameter* m_parameter_crop;

void SetCropParameter( CropParameter* param )
{
    m_parameter_crop = param;
}

CropParameter* GetCropParameter()
{
    return m_parameter_crop;
}

float rotate[16] =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

//PBVR最大／最小座標値(サーバー（Command::generateAbstractParticles）より)
extern	float	PVBRmaxcoords[3];
extern	float	PVBRmincoords[3];

int	obj_sel				= PBVR_CROP - 1;			//表示するオブジェクトタイプの選択
int	obj_3d				= PBVR_CROP;

int	iwin_3vd;								//３面図用ウィンドウのID
int	clicking			= PBVR_OFF;				//マウスの左ボタン押下状態
int	win_sel				= PBVR_NO_SEL;			//ビューポート選択状態
int	modifier			= PBVR_OFF;				//特殊キー押下状態
unsigned int ModifyMode = PBVR_OFF;				//変形モード
unsigned int CropTrans	= PBVR_OFF;				//CROP座標変換モード
int	WindowWidth			= PBVR_WINDOW_W;			//現在のウィンドウ・サイズ(幅）
int	WindowHeight		= PBVR_WINDOW_H;			//現在のウィンドウ・サイズ(高さ）
float	mapscale;							//現在の座標系の倍率
float	PBVRscale		= PBVR_SCALE;		//PBVRウィンドウの座標系の倍率
#ifdef NOTOPEN
int	crop_flg			= PBVR_TRUE;				//領域選択有効フラグ
#else
int	crop_flg			= PBVR_FALSE;			//領域選択有効フラグ
#endif
double	PBVRCoordScale;						//PVBR座標変換スケール
double	PBVRxmin, PBVRxmax, PBVRymin, PBVRymax, PBVRzmin, PBVRzmax;							//PBVR最大／最小値
double	PBVRxminChk, PBVRxmaxChk, PBVRyminChk, PBVRymaxChk, PBVRzminChk, PBVRzmaxChk;		//PBVR最大／最小値チェック用（立方体空間）
double	PBVRsides;																			//PBVR領域の最大の辺
float	Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;													//3vdウインドウ内で取りうる最大値／最小値
float	CropXmin, CropXmax, CropYmin, CropYmax, CropZmin, CropZmax;							//Cropの座標の最大値／最小値（-1.0〜1.0）
float	SphereXmin, SphereXmax, SphereYmin, SphereYmax, SphereZmin, SphereZmax;				//Sphereの座標の最大値／最小値（-1.0〜1.0）
float	PillarXYXmin, PillarXYXmax, PillarXYYmin, PillarXYYmax, PillarXYZmin, PillarXYZmax;	//PillarXYの座標の最大値／最小値（-1.0〜1.0）
float	PillarYZXmin, PillarYZXmax, PillarYZYmin, PillarYZYmax, PillarYZZmin, PillarYZZmax;	//PillarYZの座標の最大値／最小値（-1.0〜1.0）
float	PillarXZXmin, PillarXZXmax, PillarXZYmin, PillarXZYmax, PillarXZZmin, PillarXZZmax;	//PillarXZの座標の最大値／最小値（-1.0〜1.0）


//３面図表示（本体組み込み時には、これをコールする）

void	Win3vd();
// APPEND START FP)K.YAJIMA 2015.03.11

void	FILEpanelCreate();
// APPEND END FP)K.YAJIMA 2015.03.11

void	Select3vd( const int num );

void	GLDrawNormal( const double length, const int dirx, const int diry,
                      const int dirz, const int viewport_x, const int viewport_y );
void	DisplayString( const float x, const float  y, const float z, const char* str );
void	DrawFrame( const double size );

void	Initialize3vd( const int winid );
void	End3vd( const int winid );

double	DegreeToRadian( const double degree );
int		CalculateNormal( const float* p1, const float* p2, const float* p3, float* n );

void	PBVRScaling();
void	CalculateCoordinateScaling( const double xmin, const double xmax,
                                    const double ymin, const double ymax,
                                    const double zmin, const double zmax,
                                    double* sides, double* CScale );

void	Calculate3vdCoord( const float max, const float min, const float p, float* point );
void	CalculatePBVRCoord( const float max, const float min, const float p, float* point );
void	InitializePBVRParameter();
void	Set3vdMax( const float max, const float min, float* areamax, float* areamin );
void	SetPBVRMax( const double max, const double min, double* areamax, double* areamin );
void	SetCropMaxMin( const float x, const float y, const float z );
void	SetSphereMaxMin( const float x, const float y, const float z, const float rad );
void	SetPillarXYMaxMin( const float x, const float y, const float z, const float rad, const float heigttop, const float heigtbase );
void	SetPillarYZMaxMin( const float x, const float y, const float z, const float rad, const float heigttop, const float heigtbase );
void	SetPillarXZMaxMin( const float x, const float y, const float z, const float rad, const float heigttop, const float heigtbase );

//各種コールバック

void	Display3vd();

void	CallBackGLUI( const int num );
void	GUIReset( const int num );
void	GUIApply( const int num );
void	Resize( const int w, const int h );

void	MouseFunc( const int button, const int state, const int x, const int y );
void	MouseCropX( const int button, const int state, const int x, const int y );
void	MouseCropY( const int button, const int state, const int x, const int y );
void	MouseCropZ( const int button, const int state, const int x, const int y );
void	MouseSphereX( const int button, const int state, const int x, const int y );
void	MouseSphereY( const int button, const int state, const int x, const int y );
void	MouseSphereZ( const int button, const int state, const int x, const int y );
void	MousePillarXY_X( const int button, const int state, const int x, const int y );
void	MousePillarXY_Y( const int button, const int state, const int x, const int y );
void	MousePillarXY_Z( const int button, const int state, const int x, const int y );
void	MousePillarXZ_X( const int button, const int state, const int x, const int y );
void	MousePillarXZ_Y( const int button, const int state, const int x, const int y );
void	MousePillarXZ_Z( const int button, const int state, const int x, const int y );
void	MousePillarYZ_X( const int button, const int state, const int x, const int y );
void	MousePillarYZ_Y( const int button, const int state, const int x, const int y );
void	MousePillarYZ_Z( const int button, const int state, const int x, const int y );

void	MouseMotion( const int x, const int y );
void	MotionCropX( const int x, const int y );
void	MotionCropY( const int x, const int y );
void	MotionCropZ( const int x, const int y );
void	MotionSphereX( const int x, const int y );
void	MotionSphereY( const int x, const int y );
void	MotionSphereZ( const int x, const int y );
void	MotionPillarXY_X( const int x, const int y );
void	MotionPillarXY_Y( const int x, const int y );
void	MotionPillarXY_Z( const int x, const int y );
void	MotionPillarXZ_X( const int x, const int y );
void	MotionPillarXZ_Y( const int x, const int y );
void	MotionPillarXZ_Z( const int x, const int y );
void	MotionPillarYZ_X( const int x, const int y );
void	MotionPillarYZ_Y( const int x, const int y );
void	MotionPillarYZ_Z( const int x, const int y );

void	SpecialFunc( const int key, const int x, const int y );
void	SpecialFuncCropX( const int key, const int x, const int y );
void	SpecialFuncCropY( const int key, const int x, const int y );
void	SpecialFuncCropZ( const int key, const int x, const int y );
void	SpecialFuncSphereX( const int key, const int x, const int y );
void	SpecialFuncSphereY( const int key, const int x, const int y );
void	SpecialFuncSphereZ( const int key, const int x, const int y );
void	SpecialFuncPillarXY_X( const int key, const int x, const int y );
void	SpecialFuncPillarXY_Y( const int key, const int x, const int y );
void	SpecialFuncPillarXY_Z( const int key, const int x, const int y );
void	SpecialFuncPillarXZ_X( const int key, const int x, const int y );
void	SpecialFuncPillarXZ_Y( const int key, const int x, const int y );
void	SpecialFuncPillarXZ_Z( const int key, const int x, const int y );
void	SpecialFuncPillarYZ_X( const int key, const int x, const int y );
void	SpecialFuncPillarYZ_Y( const int key, const int x, const int y );
void	SpecialFuncPillarYZ_Z( const int key, const int x, const int y );

//編集可能テキストボックス用コールバック

void	CallBackCropMinLevelX( const int num );
void	CallBackCropMinLevelY( const int num );
void	CallBackCropMinLevelZ( const int num );
void	CallBackCropMaxLevelX( const int num );
void	CallBackCropMaxLevelY( const int num );
void	CallBackCropMaxLevelZ( const int num );
void	CallBackSphereRadius( const int num );
void	CallBackSphereCenterX( const int num );
void	CallBackSphereCenterY( const int num );
void	CallBackSphereCenterZ( const int num );
void	CallBackPillarRadius( const int num );
void	CallBackPillarHeight( const int num );
void	CallBackPillarCenterX( const int num );
void	CallBackPillarCenterY( const int num );
void	CallBackPillarCenterZ( const int num );

void	CallBackCrop( const int num );

// APPEND START FP)K.YAJIMA 2015.02.20
void	CallBackFile( const int num );
void	CallBackFileDialog( const int num );
void	CallBackParticle( const int num );
void	CallBackDirectoryDialog( const int num );

bool	FileCallback_Cancel = false;
// APPEND END FP)K.YAJIMA 2015.02.20
// APPEND START FP)M.Tanaka 2015.03.05
void    CallBackFileClose( const int num );
GLUI* g_glui_filepanel;
// APPEND END   FP)M.Tanaka 2015.03.05

//GUIパネル用

int SubWinID; 						//サブウィンドウのID

GLUI* glui_3vd;
GLUI_Panel*      obj_panel;
GLUI_RadioGroup* obj_sel_b;

float cxminlevel2 = PBVR_CROP_X_MIN;						//CROP座標計算用
float cxmaxlevel2 = PBVR_CROP_X_MAX;
float cyminlevel2 = PBVR_CROP_Y_MIN;
float cymaxlevel2 = PBVR_CROP_Y_MAX;
float czminlevel2 = PBVR_CROP_Z_MIN;
float czmaxlevel2 = PBVR_CROP_Z_MAX;

float tbcxminlevel = PBVR_CROP_X_MIN;						//CROPテキストボックス入力用
float tbcxmaxlevel = PBVR_CROP_X_MAX;
float tbcyminlevel = PBVR_CROP_Y_MIN;
float tbcymaxlevel = PBVR_CROP_Y_MAX;
float tbczminlevel = PBVR_CROP_Z_MIN;
float tbczmaxlevel = PBVR_CROP_Z_MAX;

float region_crop_x_min_level = PBVR_CROP_X_MIN;						//CROP選択領域受け渡し用
float region_crop_x_max_level = PBVR_CROP_X_MAX;
float region_crop_y_min_level = PBVR_CROP_Y_MIN;
float region_crop_y_max_level = PBVR_CROP_Y_MAX;
float region_crop_z_min_level = PBVR_CROP_Z_MIN;
float region_crop_z_max_level = PBVR_CROP_Z_MAX;

float save_cxmin	= PBVR_CROP_X_MIN;
float save_cxmax	= PBVR_CROP_X_MAX;
float save_cymin	= PBVR_CROP_Y_MIN;
float save_cymax	= PBVR_CROP_Y_MAX;
float save_czmin	= PBVR_CROP_Z_MIN;
float save_czmax	= PBVR_CROP_Z_MAX;

float sradius = PBVR_SRADIUS;							//PBVR_SPHERE座標計算用
float scenterx = 0.0F;
float scentery = 0.0F;
float scenterz = 0.0F;

float tbsradius = PBVR_SRADIUS;							//PBVR_SPHEREテキストボックス入力用
float tbscenterx = 0.0F;
float tbscentery = 0.0F;
float tbscenterz = 0.0F;

float region_sphere_radius = PBVR_SRADIUS;							//PBVR_SPHERE選択領域受け渡し用
float region_sphere_center_x = 0.0F;
float region_sphere_center_y = 0.0F;
float region_sphere_center_z = 0.0F;

float save_sradius = PBVR_SRADIUS;						//PBVR_SPHERE変形処理時のサイズ保存用

float pradius = PBVR_PRADIUS;							//PILLAR座標計算用
float pheight = PBVR_PHEIGHT;
float pcenterx = 0.0F;
float pcentery = 0.0F;
float pcenterz = -0.5F; //0.0F;

float tbpradius;									//PILLARテキストボックス入力用
float tbpheight;
float tbpcenterx;
float tbpcentery;
float tbpcenterz;

float region_pillar_radius;									//PILLAR選択領域受け渡し用
float region_pillar_height;
float region_pillar_center_x;
float region_pillar_center_y;
float region_pillar_center_z;

float save_pradius = PBVR_PRADIUS;						//PILLAR変形処理時のサイズ保存用
float save_pheight = PBVR_PHEIGHT;

float save_pYZradius = PBVR_PRADIUS;
float save_pYZheight = PBVR_PHEIGHT;

float save_pXZradius = PBVR_PRADIUS;
float save_pXZheight = PBVR_PHEIGHT;

GLUI_EditText* crop_cxmin2;
GLUI_EditText* crop_cxmax2;
GLUI_EditText* crop_cymin2;
GLUI_EditText* crop_cymax2;
GLUI_EditText* crop_czmin2;
GLUI_EditText* crop_czmax2;

GLUI_EditText* sphere_radius;
GLUI_EditText* sphere_centerx;
GLUI_EditText* sphere_centery;
GLUI_EditText* sphere_centerz;

GLUI_EditText* pillar_radius;
GLUI_EditText* pillar_height;
GLUI_EditText* pillar_centerx;
GLUI_EditText* pillar_centery;
GLUI_EditText* pillar_centerz;

GLUI_Panel* obj_panelcr_col;

// APPEND START FP)M.Tanaka 2015.03.02
GLUI_Button* g_btn_vdd;
// APPEND END   FP)M.Tanaka 2015.03.02

//パラメータセット用クラス

class	CropCubeParameter
{
public:
    float m_parameter_cx_min;
    float m_parameter_cx_max;
    float m_parameter_cy_min;
    float m_parameter_cy_max;
    float m_parameter_cz_min;
    float m_parameter_cz_max;

    CropCubeParameter();
    void	CropParaSet();
};

class	CropSphereParameter
{
public:
    float m_parameter_sphere_radius;
    float m_parameter_sphere_center_x;
    float m_parameter_sphere_center_y;
    float m_parameter_sphere_center_z;

    CropSphereParameter();
    void	SphereParaSet();
};

class	CropPillerParameter
{
public:
    float m_parameter_pillar_radius;
    float m_parameter_pillar_height;
    float m_parameter_pillar_center_x;
    float m_parameter_pillar_center_y;
    float m_parameter_pillar_center_z;

    CropPillerParameter();
    void	PillarParaSet();
};

class	CropDisplayParameter	:	public CropCubeParameter,
    public CropSphereParameter,
    public CropPillerParameter
{
public:
    int	m_select_area;							//選択領域の形状（ CROP or PBVR_SPHERE or PILLAR )
    int	m_apply;								//APPLYボタン押下フラグ（本体側でOFFにする）
    CropDisplayParameter();
};

CropCubeParameter::CropCubeParameter()
{
    m_parameter_cx_min  = cxminlevel2;
    m_parameter_cx_max  = cxmaxlevel2;
    m_parameter_cy_min  = cyminlevel2;
    m_parameter_cy_max  = cymaxlevel2;
    m_parameter_cz_min  = czminlevel2;
    m_parameter_cz_max  = czmaxlevel2;
}

CropSphereParameter::CropSphereParameter()
{
    m_parameter_sphere_radius  = sradius;
    m_parameter_sphere_center_x = scenterx;
    m_parameter_sphere_center_y = scentery;
    m_parameter_sphere_center_z = scenterz;
}

CropPillerParameter::CropPillerParameter()
{
    m_parameter_pillar_radius  = pradius;
    m_parameter_pillar_height  = pheight;
    m_parameter_pillar_center_x = pcenterx;
    m_parameter_pillar_center_y = pcentery;
    m_parameter_pillar_center_z = pcenterz;
}

CropDisplayParameter::CropDisplayParameter()
{
    m_select_area = PBVR_CROP;
    m_apply = PBVR_OFF;
}

//オブジェクト操作用クラス
class	Motion
{
private:
    int	m_save_x;												//マウスボタンUP時の移動量のX座標
    int	m_save_y;												//		〃			        Y座標
    int	m_save_z;												//		〃			        Z座標

public:
    int	m_previous_x;												//マウスボタン押下時のX座標
    int	m_previous_y;												//		〃			  Y座標
    int	m_previous_z;												//		〃			  Z座標
    double	m_scale_x;										//X軸方向の拡大/縮小倍率
    double	m_scale_y;										//Y軸方向		〃
    double	m_scale_z;										//Z軸方向		〃
    int	m_move_x;												//オブジェクトのの移動量のX座標
    int	m_move_y;												//		〃			  	  Y座標
    int	m_move_z;												//		〃			 	  Z座標

public:
    Motion();
    void	motionInitialize();
    void	setRetentionPosition( const int x, const int y, const int z );
    void	setPosition(  const int x, const int y, const int z );
    void	scaling( const float x, const float y, const float z );
    void	setScale( const float x, const float y, const float z );
    void	savePosition( const int x, const int y, const int z );
    void	objectControl();
};

//領域選択用オブジェクトのクラス
//直方体
class	Crop : public	Motion
{
public:
    Crop();
    void	initializeCrop( const int flag = 0 );
    void	drawCrop( const int angle, const int dirx, const int diry, const int dirz, const int type );
    void	scalingCrop( const float scalex, const float scaley, const float scalez );
    void	setCropTextData();
    void	setCropTextBox();

    float	m_side_x;											//X軸方向の一辺の長さ（m_side_length * scale_x)
    float	m_side_y;											//Y    〃
    float	m_side_z;											//Z    〃

private:
    int		m_sides;											//SIDE数
    double	m_side_length;										//一辺の長さ(初期値）
};

//球体
class	Sphere : public	Motion
{
private:
    int		m_sides;											//SIDE数

public:
    double	m_radius;											//半径（0〜1.0）
    float	m_center_x;										//-1.0〜1.0
    float	m_center_y;										//	〃
    float	m_center_z;										//	〃
    float	m_save_center_x;									//マウスボタンUP時の中心座標
    float	m_save_center_y;									//	〃
    float	m_save_center_z;									//	〃
    double	m_save_radius;									//マウスボタンUP時の半径

public:
    Sphere();
    void	initializeSphere( const int flag = 0 );
    void	drawSphere( const int angle , const int dirx, const int diry, const int dirz, const int type );
    void	scalingSphere( const float scalex, const float scaley, const float scalez );
    void	setSphereTextData();
    void	setSphereTextBox();
};

//円柱
class	Pillar : public	Motion
{
private:
    int		m_sides; 										//SIDE数

    float	m_save_tbpradius;									//PILLARテキストボックス入力値保存用
    float	m_save_tbpheight;
    float	m_save_tbpcenterx;
    float	m_save_tbpcentery;
    float	m_save_tbpcenterz;

public:
    double	m_radius;											//半径（0〜1.0）
    double	m_height;											//高さ（0〜2.0）
    float	m_center_x;										//中心座標（-1.0〜1.0）
    float	m_center_y;										//	〃
    float	m_center_z;										//	〃
    float	m_save_center_x;									//マウスボタンUP時の中心座標
    float	m_save_center_y;									//	〃
    float	m_save_center_z;									//	〃
    double	m_save_height;  									//マウスボタンUP時の高さ
    double	m_save_radius;  									//マウスボタンUP時の半径

public:
    Pillar();
    void	initializePillar( const int flag = 0 );
    void	drawPillar( const int angle , const int dirx, const int diry, const int dirz, const int type );
    void	scalingPillar( const float scalex, const float scaley, const float scalez );
    void	setPillarTextData();
    void	setPillarTextBox ();
    void	rem_loadPillarTextBox ();
    void	rem_loadPillar();
    void	checkPillarTextBox();
};

Motion::Motion()
{
    m_previous_x = 0;
    m_previous_y = 0;
    m_previous_z = 0;
    m_move_x = 0;
    m_move_y = 0;
    m_move_z = 0;
    m_save_x = 0;
    m_save_y = 0;
    m_save_z = 0;
    m_scale_x = 1.0;
    m_scale_y = 1.0;
    m_scale_z = 1.0;
}

Sphere::Sphere()
{
    m_radius = PBVR_SRADIUS;
    m_sides = PBVR_SIDES;
    m_center_x = 0.0F;
    m_center_y = 0.0F;
    m_center_z = 0.0F;
    m_save_center_x = 0.0F;
    m_save_center_y = 0.0F;
    m_save_center_z = 0.0F;
    m_save_radius = PBVR_PRADIUS;
}

Crop::Crop()
{
    m_side_length = 1.0;
    m_sides = PBVR_SIDES;
    m_side_x = 1.0;
    m_side_y = 1.0;
    m_side_z = 1.0;
}

Pillar::Pillar()
{
    m_radius = PBVR_PRADIUS;
    m_height = PBVR_PHEIGHT;
    m_sides = PBVR_SIDES;
    m_center_x = 0.0F;
    m_center_y = 0.0F;
    m_center_z = -0.5F; //0.0F;
    m_save_center_x = 0.0F;
    m_save_center_y = 0.0F;
    m_save_center_z = -0.5F; //0.0F;
    m_save_radius = PBVR_PRADIUS;
    m_save_height = PBVR_PHEIGHT;

    m_save_tbpradius  = tbpradius;
    m_save_tbpheight  = tbpheight;
    m_save_tbpcenterx = tbpcenterx;
    m_save_tbpcentery = tbpcentery;
    m_save_tbpcenterz = tbpcenterz;
}


//表示するオブジェクトの生成
Crop	crop;
Sphere	sphere;
Pillar	pillar, pillarYZ, pillarXZ;

//パラメータ用オブジェクトの生成
CropDisplayParameter	param3vd;

//kvs オブジェクト描画用配列

//CROP
#ifdef NOTOPEN
GLfloat	verticesCrop[8 * 3];
float	NormalArrayCrop[6 * 3];
int		ConnectionArrayCrop[6 * 4] =
{
#else
kvs::Real32	verticesCrop[8 * 3];
kvs::Real32	NormalArrayCrop[6 * 3];
kvs::UInt32	ConnectionArrayCrop[6 * 4] =
{
#endif
    4, 0, 1, 5 ,											//左
    5, 1, 2, 6 ,											//前
    6, 2, 3, 7 ,											//右
    7, 3, 0, 4 ,											//後
    4, 5, 6, 7 ,											//上
    1, 0, 3, 2 											//下
};

//PBVR_SPHERE
#ifdef NOTOPEN
GLfloat	verticesSphere[( 2 + PBVR_SIDES * ( PBVR_SIDES - 1 ) ) * 3];
float	NormalArraySphere[( PBVR_SIDES * 2 + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ) * 3];
int		ConnectionArraySphere[( PBVR_SIDES * 2 + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ) * 3];
#else
kvs::Real32	verticesSphere[( 2 + PBVR_SIDES * ( PBVR_SIDES - 1 ) ) * 3];
kvs::Real32	NormalArraySphere[( PBVR_SIDES * 2 + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ) * 3];
kvs::UInt32	ConnectionArraySphere[( PBVR_SIDES * 2 + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ) * 3];
#endif

//PILLAR
#ifdef NOTOPEN
GLfloat	verticesPillar [( PBVR_SIDES * 2 + 2 ) * 3];
float	NormalArrayPillar[( PBVR_SIDES * 4 ) * 3];
int		ConnectionArrayPillar [( PBVR_SIDES * 4 ) * 3];
#else
kvs::Real32		verticesPillar [( PBVR_SIDES * 2 + 2 ) * 3];
kvs::Real32		NormalArrayPillar[( PBVR_SIDES * 4 ) * 3];
kvs::UInt32		ConnectionArrayPillar[( PBVR_SIDES * 4 ) * 3];
#endif

//編集可能テキストボックス用コールバック（直方体用）
void	CallBackCropMinLevelX( const int num )
{
    //最大／最小値のチェック
    if ( tbcxminlevel < PBVRxminChk )
        tbcxminlevel = PBVRxminChk;

    if ( tbcxminlevel >= tbcxmaxlevel )
        tbcxminlevel = tbcxmaxlevel - PBVR_SCALE_MIN;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRxmax, PBVRxmin, tbcxminlevel, &cxminlevel2 );

    crop.m_side_x = cxmaxlevel2 - cxminlevel2;
    crop.m_move_x = ( cxmaxlevel2 + cxminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    crop.savePosition( crop.m_move_x, 0, 0 );
    save_cxmin = cxminlevel2;

    crop_cxmin2->set_float_val( tbcxminlevel );
    param3vd.CropParaSet();
}

void	CallBackCropMaxLevelX( const int num )
{
    //最大／最小値のチェック
    if ( tbcxmaxlevel > PBVRxmaxChk )
        tbcxmaxlevel = PBVRxmaxChk;

    if ( tbcxmaxlevel <= tbcxminlevel )
        tbcxmaxlevel = tbcxminlevel + PBVR_SCALE_MIN;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRxmax, PBVRxmin, tbcxmaxlevel, &cxmaxlevel2 );

    crop.m_side_x = cxmaxlevel2 - cxminlevel2;
    crop.m_move_x = ( cxmaxlevel2 + cxminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    crop.savePosition( crop.m_move_x, 0, 0 );
    save_cxmax = cxmaxlevel2;

    crop_cxmax2->set_float_val( tbcxmaxlevel );
    param3vd.CropParaSet();
}

void	CallBackCropMinLevelY( const int num )
{
    //最大／最小値のチェック
    if ( tbcyminlevel < PBVRyminChk )
        tbcyminlevel = PBVRyminChk;

    if ( tbcyminlevel >= tbcymaxlevel )
        tbcyminlevel = tbcymaxlevel - PBVR_SCALE_MIN;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRymax, PBVRymin, tbcyminlevel, &cyminlevel2 );

    crop.m_side_y = cymaxlevel2 - cyminlevel2;
    crop.m_move_y = ( cymaxlevel2 + cyminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    crop.savePosition( 0, crop.m_move_y, 0 );
    save_cymin = cyminlevel2;

    crop_cymin2->set_float_val( tbcyminlevel );
    param3vd.CropParaSet();
}

void	CallBackCropMaxLevelY( const int num )
{
    //最大／最小値のチェック
    if ( tbcymaxlevel > PBVRymaxChk )
        tbcymaxlevel = PBVRymaxChk;

    if ( tbcymaxlevel <= tbcyminlevel )
        tbcymaxlevel = tbcyminlevel + PBVR_SCALE_MIN;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRymax, PBVRymin, tbcymaxlevel, &cymaxlevel2 );

    crop.m_side_y = cymaxlevel2 - cyminlevel2;
    crop.m_move_y = ( cymaxlevel2 + cyminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    crop.savePosition( 0, crop.m_move_y, 0 );
    save_cymax = cymaxlevel2;

    crop_cymax2->set_float_val( tbcymaxlevel );
    param3vd.CropParaSet();
}

void	CallBackCropMinLevelZ( const int num )
{
    //最大／最小値のチェック
    if ( tbczminlevel < PBVRzminChk )
        tbczminlevel = PBVRzminChk;

    if ( tbczminlevel >= tbczmaxlevel )
        tbczminlevel = tbczmaxlevel - PBVR_SCALE_MIN;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRzmax, PBVRzmin, tbczminlevel, &czminlevel2 );

    crop.m_side_z = czmaxlevel2 - czminlevel2;
    crop.m_move_z = ( czmaxlevel2 + czminlevel2 ) * (  PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    crop.savePosition( 0, 0, crop.m_move_z );
    save_czmin = czminlevel2;

    crop_czmin2->set_float_val( tbczminlevel );
    param3vd.CropParaSet();
}

void	CallBackCropMaxLevelZ( const int num )
{
    //最大／最小値のチェック
    if ( tbczmaxlevel > PBVRzmaxChk )
        tbczmaxlevel = PBVRzmaxChk;

    if ( tbczmaxlevel <= tbczminlevel )
        tbczmaxlevel = tbczminlevel + PBVR_SCALE_MIN;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRzmax, PBVRzmin, tbczmaxlevel, &czmaxlevel2 );

    crop.m_side_z = czmaxlevel2 - czminlevel2;
    crop.m_move_z = ( czmaxlevel2 + czminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    crop.savePosition( 0, 0, crop.m_move_z );
    save_czmax = czmaxlevel2;

    crop_czmax2->set_float_val( tbczmaxlevel );
    param3vd.CropParaSet();
}

//編集可能テキストボックス用コールバック（球体用）
void	CallBackSphereRadius( const int num )
{
    sradius = tbsradius * PBVRCoordScale;
#if 0
    //最大／最小値のチェック
    if ( sradius < PBVR_RADIUS_MIN )
    {
        sradius = PBVR_RADIUS_MIN;
        tbsradius = sradius / PBVRCoordScale;
    }

    else if ( sradius > PBVR_RADIUS_MAX )
    {
        sradius = PBVR_RADIUS_MAX;
        tbsradius = sradius / PBVRCoordScale;
    }
#endif
    sphere.m_radius = sradius;
    sphere.setScale( 1.0F , 1.0F, 1.0F );

    sphere_radius->set_float_val( tbsradius );
    param3vd.SphereParaSet();
}

void	CallBackSphereCenterX( const int num )
{
    //最大／最小値のチェック
    if ( tbscenterx < PBVRxminChk )
        tbscenterx = PBVRxminChk;

    if ( tbscenterx > PBVRxmaxChk )
        tbscenterx = PBVRxmaxChk;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRxmax, PBVRxmin, tbscenterx, &scenterx );

    sphere.m_center_x = scenterx;
    sphere.m_move_x = sphere.m_center_x * ( PBVR_VIEWPORT_WIDTH / 2 );
    sphere.savePosition( sphere.m_move_x, 0, 0 );

    sphere_centerx->set_float_val( tbscenterx );
    param3vd.SphereParaSet();
}

void	CallBackSphereCenterY( const int num )
{
    //最大／最小値のチェック
    if ( tbscentery < PBVRyminChk )
        tbscentery = PBVRyminChk;

    if ( tbscentery > PBVRymaxChk )
        tbscentery = PBVRymaxChk;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRymax, PBVRymin, tbscentery, &scentery );

    sphere.m_center_y = scentery;
    sphere.m_move_y = sphere.m_center_y * ( PBVR_VIEWPORT_HEIGHT / 2 );
    sphere.savePosition( 0, sphere.m_move_y, 0 );

    sphere_centery->set_float_val( tbscentery );
    param3vd.SphereParaSet();
}

void	CallBackSphereCenterZ( const int num )
{
    //最大／最小値のチェック
    if ( tbscenterz < PBVRzminChk )
        tbscenterz = PBVRzminChk;

    if ( tbscenterz > PBVRzmaxChk )
        tbscenterz = PBVRzmaxChk;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRzmax, PBVRzmin, tbscenterz, &scenterz );

    sphere.m_center_z = scenterz;
    sphere.m_move_z = sphere.m_center_z * ( PBVR_VIEWPORT_WIDTH / 2 );
    sphere.savePosition( 0, 0, sphere.m_move_z );

    sphere_centerz->set_float_val( tbscenterz );
    param3vd.SphereParaSet();
}

//編集可能テキストボックス用コールバック（円柱用）
void	CallBackPillarRadius( const int num )
{
    pradius = tbpradius * PBVRCoordScale;
#if 0
    //最大／最小値のチェック
    if ( pradius < PBVR_RADIUS_MIN )
    {
        pradius = PBVR_RADIUS_MIN;
        tbpradius = pradius / PBVRCoordScale;
    }

    else if ( pradius > PBVR_RADIUS_MAX )
    {
        pradius = PBVR_RADIUS_MAX;
        tbpradius = pradius / PBVRCoordScale;
    }
#endif
    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:
        pillar.m_radius = pradius;
        pillar.setScale( 1.0F , 1.0F, 1.0F );
        break;

    case	PBVR_PILLARYZ:
        pillarYZ.m_radius = pradius;
        pillarYZ.setScale( 1.0F , 1.0F, 1.0F );
        break;

    case	PBVR_PILLARXZ:
        pillarXZ.m_radius = pradius;
        pillarXZ.setScale( 1.0F , 1.0F, 1.0F );
        break;

    default:
        break;
    }

    pillar_radius->set_float_val( tbpradius );
    param3vd.PillarParaSet();
}

void	CallBackPillarHeight( const int num )
{
    pheight = tbpheight * PBVRCoordScale;

    //最大／最小値のチェック
    if ( pheight < PBVR_HEIGHTMIN )
    {
        pheight = PBVR_HEIGHTMIN;
        tbpheight = pheight / PBVRCoordScale;
    }

    else if ( pheight > PBVR_HEIGHTMAX )
    {
        pheight = PBVR_HEIGHTMAX;
        tbpheight = pheight / PBVRCoordScale;
    }

    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:
        pillar.m_height = pheight;
        pillar.setScale( 1.0F , 1.0F, 1.0F );
        break;

    case	PBVR_PILLARYZ:
        pillarYZ.m_height = pheight;
        pillarYZ.setScale( 1.0F , 1.0F, 1.0F );
        break;

    case	PBVR_PILLARXZ:
        pillarXZ.m_height = pheight;
        pillarXZ.setScale( 1.0F , 1.0F, 1.0F );
        break;

    default:
        break;
    }

    pillar_height->set_float_val( tbpheight );
    param3vd.PillarParaSet();
}

void	CallBackPillarCenterX( const int num )
{
    //最大／最小値のチェック
    if ( tbpcenterx < PBVRxminChk )
        tbpcenterx = PBVRxminChk;

    if ( tbpcenterx > PBVRxmaxChk )
        tbpcenterx = PBVRxmaxChk;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRxmax, PBVRxmin, tbpcenterx, &pcenterx );

    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:
        pillar.m_center_x = pcenterx;
        pillar.m_move_x = pillar.m_center_x * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillar.savePosition( pillar.m_move_x, 0, 0 );
        break;

    case	PBVR_PILLARYZ:
        pillarYZ.m_center_x = pcenterx;
        pillarYZ.m_move_x = pillarYZ.m_center_x * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillarYZ.savePosition( pillarYZ.m_move_x, 0, 0 );
        break;

    case	PBVR_PILLARXZ:
        pillarXZ.m_center_x = pcenterx;
        pillarXZ.m_move_x = pillarXZ.m_center_x * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillarXZ.savePosition( pillarXZ.m_move_x, 0, 0 );
        break;

    default:
        break;
    }

    pillar_centerx->set_float_val( tbpcenterx );
    param3vd.PillarParaSet();
}

void	CallBackPillarCenterY( const int num )
{
    //最大／最小値のチェック
    if ( tbpcentery < PBVRyminChk )
        tbpcentery = PBVRyminChk;

    if ( tbpcentery > PBVRymaxChk )
        tbpcentery = PBVRymaxChk;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRymax, PBVRymin, tbpcentery, &pcentery );

    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:
        pillar.m_center_y = pcentery;
        pillar.m_move_y = pillar.m_center_y * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillar.savePosition( 0, pillar.m_move_y, 0 );
        break;

    case	PBVR_PILLARYZ:
        pillarYZ.m_center_y = pcentery;
        pillarYZ.m_move_y = pillarYZ.m_center_y * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillarYZ.savePosition( 0, pillarYZ.m_move_y, 0 );
        break;

    case	PBVR_PILLARXZ:
        pillarXZ.m_center_y = pcentery;
        pillarXZ.m_move_y = pillarXZ.m_center_y * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillarXZ.savePosition( 0, pillarXZ.m_move_y, 0 );
        break;

    default:
        break;
    }

    pillar_centery->set_float_val( tbpcentery );
    param3vd.PillarParaSet();
}

void	CallBackPillarCenterZ( const int num )
{
    //最大／最小値のチェック
    if ( tbpcenterz < PBVRzminChk )
        tbpcenterz = PBVRzminChk;

    if ( tbpcenterz > PBVRzmaxChk )
        tbpcenterz = PBVRzmaxChk;

    //テキストボックス入力値を3vdウィンドウの座標に変換
    Calculate3vdCoord( PBVRzmax, PBVRzmin, tbpcenterz, &pcenterz );

    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:
        pillar.m_center_z = pcenterz;
        pillar.m_move_z = pillar.m_center_z * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillar.savePosition( 0, 0, pillar.m_move_z );
        break;

    case	PBVR_PILLARYZ:
        pillarYZ.m_center_z = pcenterz;
        pillarYZ.m_move_z = pillarYZ.m_center_z * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillarYZ.savePosition( 0, 0, pillarYZ.m_move_z );

    case	PBVR_PILLARXZ:
        pillarXZ.m_center_z = pcenterz;
        pillarXZ.m_move_z = pillarXZ.m_center_z * ( PBVR_VIEWPORT_WIDTH / 2 );
        pillarXZ.savePosition( 0, 0, pillarXZ.m_move_z );
        break;

        break;
    default:
        break;
    }

    pillar_centerz->set_float_val( tbpcenterz );
    param3vd.PillarParaSet();
}

//CROP テキストボックスに値をセット
void	Crop::setCropTextBox()
{
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, cxminlevel2, &tbcxminlevel );
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, cxmaxlevel2, &tbcxmaxlevel );
    CalculatePBVRCoord( PBVRymax, PBVRymin, cyminlevel2, &tbcyminlevel );
    CalculatePBVRCoord( PBVRymax, PBVRymin, cymaxlevel2, &tbcymaxlevel );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, czminlevel2, &tbczminlevel );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, czmaxlevel2, &tbczmaxlevel );

    crop_cxmax2->set_float_val( tbcxmaxlevel );
    crop_cxmin2->set_float_val( tbcxminlevel );
    crop_cymax2->set_float_val( tbcymaxlevel );
    crop_cymin2->set_float_val( tbcyminlevel );
    crop_czmax2->set_float_val( tbczmaxlevel );
    crop_czmin2->set_float_val( tbczminlevel );
}

//PBVR_SPHERE テキストボックスに値をセット
void	Sphere::setSphereTextBox()
{
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, scenterx, &tbscenterx );
    CalculatePBVRCoord( PBVRymax, PBVRymin, scentery, &tbscentery );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, scenterz, &tbscenterz );
    tbsradius = sradius / PBVRCoordScale;

    sphere_radius->set_float_val( tbsradius );
    sphere_centerx->set_float_val( tbscenterx );
    sphere_centery->set_float_val( tbscentery );
    sphere_centerz->set_float_val( tbscenterz );
}

//PILLAR テキストボックスに値をセット
void	Pillar::setPillarTextBox()
{
    pradius  = m_radius;
    pheight  = m_height;
    pcenterx = m_center_x;
    pcentery = m_center_y;
    pcenterz = m_center_z;

    CalculatePBVRCoord( PBVRxmax, PBVRxmin, pcenterx, &tbpcenterx );
    CalculatePBVRCoord( PBVRymax, PBVRymin, pcentery, &tbpcentery );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, pcenterz, &tbpcenterz );

    tbpradius = pradius / PBVRCoordScale;
    tbpheight = pheight / PBVRCoordScale;

    pillar_radius->set_float_val( tbpradius );
    pillar_height->set_float_val( tbpheight );
    pillar_centerx->set_float_val( tbpcenterx );
    pillar_centery->set_float_val( tbpcentery );
    pillar_centerz->set_float_val( tbpcenterz );

    m_save_tbpradius = tbpradius;
    m_save_tbpheight = tbpheight;
    m_save_tbpcenterx = tbpcenterx;
    m_save_tbpcentery = tbpcentery;
    m_save_tbpcenterz = tbpcenterz;
}

//PILLAR テキストボックスに値をリロード
void	Pillar::rem_loadPillarTextBox()
{
    pillar_radius->set_float_val( m_save_tbpradius );
    pillar_height->set_float_val( m_save_tbpheight );
    pillar_centerx->set_float_val( m_save_tbpcenterx );
    pillar_centery->set_float_val( m_save_tbpcentery );
    pillar_centerz->set_float_val( m_save_tbpcenterz );
}

//PILLAR テキストボックスセーブ値をリロード
void	Pillar::rem_loadPillar()
{
    m_save_tbpradius = tbpradius;
    m_save_tbpheight = tbpheight;
    m_save_tbpcenterx = tbpcenterx;
    m_save_tbpcentery = tbpcentery;
    m_save_tbpcenterz = tbpcenterz;
}

//CROP テキストボックスに値を計算してセット
void	Crop::setCropTextData()
{
//	printf("before : cxminlevel2=%f cxmaxlevel2=%f\n",cxminlevel2,cxmaxlevel2);
    cxmaxlevel2 = ( crop.m_side_x / ( float )2 ) + ( m_move_x / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );
    cxminlevel2 = cxmaxlevel2 - m_side_x;

    cymaxlevel2 = ( crop.m_side_y / ( float )2 ) + ( m_move_y / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) );
    cyminlevel2 = cymaxlevel2 - m_side_y;

    czmaxlevel2 = ( crop.m_side_z / ( float )2 ) + ( m_move_z / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );
    czminlevel2 = czmaxlevel2 - m_side_z;

    setCropTextBox();
//	printf("After : cxminlevel2=%f cxmaxlevel2=%f\n",cxminlevel2,cxmaxlevel2);
}

//PBVR_SPHERE テキストボックスに値を計算してセット
void	Sphere::setSphereTextData()
{
    m_center_x = ( m_move_x / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );
    m_center_y = ( m_move_y / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) );
    m_center_z = ( m_move_z / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );

    sradius  = m_radius;
    scenterx = m_center_x;
    scentery = m_center_y;
    scenterz = m_center_z;

    setSphereTextBox();
}

//PILLAR テキストボックスに値を計算してセット
void	Pillar::setPillarTextData()
{
    m_center_x = ( m_move_x / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );
    m_center_y = ( m_move_y / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) );
    m_center_z = ( m_move_z / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );

    setPillarTextBox();
}

//guiパネルを表示するサブウィンドウのコールバック
void DisplayControl()
{
    glutSetWindow( SubWinID );
    glClear( GL_COLOR_BUFFER_BIT );

//gui ウィンドウと同じ灰色でウィンドウをクリア
    glClearColor( 0.8f, 0.8f, 0.8f, 1.0f );

    glutSwapBuffers();
    glutPostRedisplay();
}

//サブウィンドウにguiパネルを表示
void	GUIPanel()
{

//サブウィンドウ生成
    SubWinID = glutCreateSubWindow( iwin_3vd, 0, PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );						//左上原点
    glui_3vd = GLUI_Master.create_glui_subwindow( SubWinID, GLUI_SUBWINDOW_BOTTOM );
    glutDisplayFunc( Display3vd );

//   glui_3vd->set_main_gfx_window( SubWinID );
//   glutDisplayFunc(DisplayControl);

//---------------
    GLUI_Panel* obj_panelcr = glui_3vd->add_panel( "CROP", 1 );
    obj_panelcr->set_alignment( GLUI_ALIGN_LEFT );

    crop_cxmin2 = glui_3vd->add_edittext_to_panel( obj_panelcr, "            XMIN:", GLUI_EDITTEXT_FLOAT, &tbcxminlevel, 1, CallBackCropMinLevelX );
    crop_cxmin2->set_alignment( GLUI_ALIGN_RIGHT );
    crop_cxmin2->set_w( 80 );

    crop_cxmax2 = glui_3vd->add_edittext_to_panel( obj_panelcr, "XMAX:", GLUI_EDITTEXT_FLOAT, &tbcxmaxlevel, 1, CallBackCropMaxLevelX );
    crop_cxmax2->set_alignment( GLUI_ALIGN_RIGHT );
    crop_cxmax2->set_w( 80 );

    crop_cymin2 = glui_3vd->add_edittext_to_panel( obj_panelcr, "YMIN:", GLUI_EDITTEXT_FLOAT, &tbcyminlevel, 1, CallBackCropMinLevelY );
    crop_cymin2->set_alignment( GLUI_ALIGN_RIGHT );
    crop_cymin2->set_w( 80 );

    crop_cymax2 = glui_3vd->add_edittext_to_panel( obj_panelcr, "YMAX:", GLUI_EDITTEXT_FLOAT, &tbcymaxlevel, 1, CallBackCropMaxLevelY );
    crop_cymax2->set_alignment( GLUI_ALIGN_RIGHT );
    crop_cymax2->set_w( 80 );

    crop_czmin2 = glui_3vd->add_edittext_to_panel( obj_panelcr, "ZMIN:", GLUI_EDITTEXT_FLOAT, &tbczminlevel, 1, CallBackCropMinLevelZ );
    crop_czmin2->set_alignment( GLUI_ALIGN_RIGHT );
    crop_czmin2->set_w( 80 );

    crop_czmax2 = glui_3vd->add_edittext_to_panel( obj_panelcr, "ZMAX:", GLUI_EDITTEXT_FLOAT, &tbczmaxlevel, 1, CallBackCropMaxLevelZ );
    crop_czmax2->set_alignment( GLUI_ALIGN_RIGHT );
    crop_czmax2->set_w( 80 );

//--------------------

    GLUI_Panel* obj_panelcr_sp = glui_3vd->add_panel( "SPHERE", 1 );
    obj_panelcr_sp->set_alignment( GLUI_ALIGN_RIGHT );

    sphere_radius = glui_3vd->add_edittext_to_panel( obj_panelcr_sp, "         RADIUS:", GLUI_EDITTEXT_FLOAT, &tbsradius, 1, CallBackSphereRadius );
    sphere_radius->set_alignment( GLUI_ALIGN_RIGHT );
    sphere_radius->set_w( 80 );

    sphere_centerx = glui_3vd->add_edittext_to_panel( obj_panelcr_sp, "CENTER X:", GLUI_EDITTEXT_FLOAT, &tbscenterx, 1, CallBackSphereCenterX );
    sphere_centerx->set_alignment( GLUI_ALIGN_RIGHT );
    sphere_centerx->set_w( 80 );

    sphere_centery = glui_3vd->add_edittext_to_panel( obj_panelcr_sp, "CENTER Y:", GLUI_EDITTEXT_FLOAT, &tbscentery, 1, CallBackSphereCenterY );
    sphere_centery->set_alignment( GLUI_ALIGN_RIGHT );
    sphere_centery->set_w( 80 );

    sphere_centerz = glui_3vd->add_edittext_to_panel( obj_panelcr_sp, "CENTER Z:", GLUI_EDITTEXT_FLOAT, &tbscenterz, 1, CallBackSphereCenterZ );
    sphere_centerz->set_alignment( GLUI_ALIGN_RIGHT );
    sphere_centerz->set_w( 80 );

    glui_3vd->add_statictext( "" );

    glui_3vd->add_button( "APPLY", 0, GUIApply );

    glui_3vd->add_column( true );

//--------------------
    obj_panelcr_col = glui_3vd->add_panel( "PILLAR", 1 );
    obj_panelcr_col->set_alignment( GLUI_ALIGN_RIGHT );

    pillar_radius = glui_3vd->add_edittext_to_panel( obj_panelcr_col, "         RADIUS:", GLUI_EDITTEXT_FLOAT, &tbpradius, 1, CallBackPillarRadius );
    pillar_radius->set_alignment( GLUI_ALIGN_RIGHT );
    pillar_radius->set_w( 80 );

    pillar_height = glui_3vd->add_edittext_to_panel( obj_panelcr_col, "HEIGHT:", GLUI_EDITTEXT_FLOAT, &tbpheight, 1, CallBackPillarHeight );
    pillar_height->set_alignment( GLUI_ALIGN_RIGHT );
    pillar_height->set_w( 80 );

    pillar_centerx = glui_3vd->add_edittext_to_panel( obj_panelcr_col, "CENTER X:", GLUI_EDITTEXT_FLOAT, &tbpcenterx, 1, CallBackPillarCenterX );
    pillar_centerx->set_alignment( GLUI_ALIGN_RIGHT );
    pillar_centerx->set_w( 80 );

    pillar_centery = glui_3vd->add_edittext_to_panel( obj_panelcr_col, "CENTER Y:", GLUI_EDITTEXT_FLOAT, &tbpcentery, 1, CallBackPillarCenterY );
    pillar_centery->set_alignment( GLUI_ALIGN_RIGHT );
    pillar_centery->set_w( 80 );

    pillar_centerz = glui_3vd->add_edittext_to_panel( obj_panelcr_col, "CENTER Z:", GLUI_EDITTEXT_FLOAT, &tbpcenterz, 1, CallBackPillarCenterZ );
    pillar_centerz->set_alignment( GLUI_ALIGN_RIGHT );
    pillar_centerz->set_w( 80 );

//--------------------

//	glui_3vd->add_statictext("");

    GLUI_Panel* t_panel = glui_3vd->add_panel( "  Select Object  ", GLUI_PANEL_EMBOSSED );

    obj_sel_b = glui_3vd->add_radiogroup_to_panel( t_panel, &obj_sel, 3, Select3vd );
    glui_3vd->add_radiobutton_to_group( obj_sel_b, "CROP               " );
    glui_3vd->add_radiobutton_to_group( obj_sel_b, "SPHERE" );
    glui_3vd->add_radiobutton_to_group( obj_sel_b, "PILLAR - XY base " );
    glui_3vd->add_radiobutton_to_group( obj_sel_b, "PILLAR - YZ base " );
    glui_3vd->add_radiobutton_to_group( obj_sel_b, "PILLAR - XZ base " );


    glui_3vd->add_statictext( "" );

    glui_3vd->add_button( "RESET", 0, GUIReset );
    glui_3vd->add_statictext( "" );
//	glui_3vd->add_statictext("");

    glui_3vd->add_button( "CLOSE", 0, CallBackGLUI );
}

//オブジェクトをポリゴンに分割して描画
void	CalculateCropCoord()
{

    int	i;

    verticesCrop[0 * 3 + 0]  = param3vd.m_parameter_cx_min;
    verticesCrop[0 * 3 + 1] = param3vd.m_parameter_cy_min;
    verticesCrop[0 * 3 + 2] = param3vd.m_parameter_cz_min;			//上
    verticesCrop[1 * 3 + 0]  = param3vd.m_parameter_cx_min;
    verticesCrop[1 * 3 + 1] = param3vd.m_parameter_cy_min;
    verticesCrop[1 * 3 + 2] = param3vd.m_parameter_cz_max;
    verticesCrop[2 * 3 + 0]  = param3vd.m_parameter_cx_max;
    verticesCrop[2 * 3 + 1] = param3vd.m_parameter_cy_min;
    verticesCrop[2 * 3 + 2] = param3vd.m_parameter_cz_max;
    verticesCrop[3 * 3 + 0]  = param3vd.m_parameter_cx_max;
    verticesCrop[3 * 3 + 1] = param3vd.m_parameter_cy_min;
    verticesCrop[3 * 3 + 2] = param3vd.m_parameter_cz_min;

    verticesCrop[4 * 3 + 0]  = param3vd.m_parameter_cx_min;
    verticesCrop[4 * 3 + 1] = param3vd.m_parameter_cy_max;
    verticesCrop[4 * 3 + 2] = param3vd.m_parameter_cz_min;			//下
    verticesCrop[5 * 3 + 0]  = param3vd.m_parameter_cx_min;
    verticesCrop[5 * 3 + 1] = param3vd.m_parameter_cy_max;
    verticesCrop[5 * 3 + 2] = param3vd.m_parameter_cz_max;
    verticesCrop[6 * 3 + 0]  = param3vd.m_parameter_cx_max;
    verticesCrop[6 * 3 + 1] = param3vd.m_parameter_cy_max;
    verticesCrop[6 * 3 + 2] = param3vd.m_parameter_cz_max;
    verticesCrop[7 * 3 + 0]  = param3vd.m_parameter_cx_max;
    verticesCrop[7 * 3 + 1] = param3vd.m_parameter_cy_max;
    verticesCrop[7 * 3 + 2] = param3vd.m_parameter_cz_min;

    //法線ベクトルの計算
    for ( i = 0; i < 6 ; i++ )
    {
        CalculateNormal( &verticesCrop[ConnectionArrayCrop[i * 4 + 0] * 3], &verticesCrop[ConnectionArrayCrop[i * 4 + 1] * 3],
                         &verticesCrop[ConnectionArrayCrop[i * 4 + 2] * 3], &NormalArrayCrop[i * 3] );
    }
}

void	CalculateSphereCoord()
{

    int	i, sides;

    int div_h = PBVR_SIDES * 2;						// 水平方向分割数
    int div_v = div_h / 2;						// 垂直方向分割数
    float theta = ( float )( PBVR_PI * 2.0 / div_h );
    float sinTheta = ( float )sin( theta );
    float cosTheta = ( float )cos( theta );
    float radius = param3vd.m_parameter_sphere_radius;			// 球の半径

    float accu_v_s = sinTheta;					// 垂直方向 sin
    float accu_v_c = cosTheta;					// 垂直方向 cos
    float ru = radius * accu_v_s;				// 現theta値で水平スライスした時の半径
    float yu = radius * accu_v_c;				// 垂直方向Y軸値

    //頂点座標の計算
    verticesSphere[0] = param3vd.m_parameter_sphere_center_x;							//センター(北極点）
    verticesSphere[1] = param3vd.m_parameter_sphere_center_y + radius;
    verticesSphere[2] = param3vd.m_parameter_sphere_center_z;

    verticesSphere[3] = param3vd.m_parameter_sphere_center_x;							//センター(南極点）
    verticesSphere[4] = param3vd.m_parameter_sphere_center_y - radius;
    verticesSphere[5] = param3vd.m_parameter_sphere_center_z;

    for ( int y = 0; y < ( div_v - 1 ); ++y )
    {

        // 加法定理によって、垂直方向のsin/cos値を更新する
        float tmp = cosTheta * accu_v_c - sinTheta * accu_v_s;
        accu_v_s = sinTheta * accu_v_c + cosTheta * accu_v_s;
        accu_v_c = tmp;

        float rd = radius * accu_v_s;			// 1個後の半径
        float yd = radius * accu_v_c;			// 1個後の垂直方向Y軸値
        float accu_s = sinTheta;				// 水平方向sin
        float accu_c = cosTheta;				// 水平方向cos

        for ( int x = 0, sides = 0; sides < PBVR_SIDES ; ++x, ++sides )
        {

            verticesSphere[( 2 + y * PBVR_SIDES + x ) * 3 + 0] = ru * accu_c + param3vd.m_parameter_sphere_center_x;
            verticesSphere[( 2 + y * PBVR_SIDES + x ) * 3 + 1] = yu + param3vd.m_parameter_sphere_center_y;
            verticesSphere[( 2 + y * PBVR_SIDES + x ) * 3 + 2] = ru * accu_s + param3vd.m_parameter_sphere_center_z;

            // 加法定理によって、水平方向のsin/cos値を更新する
            tmp = cosTheta * accu_c - sinTheta * accu_s;
            accu_s = sinTheta * accu_c + cosTheta * accu_s;
            accu_c = tmp;

            tmp = cosTheta * accu_c - sinTheta * accu_s;
            accu_s = sinTheta * accu_c + cosTheta * accu_s;
            accu_c = tmp;
        }
        ru = rd;
        yu = yd;
    }

    //ConnectionArray の設定

    //上面
    for ( i = 0, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArraySphere[i * 3]   = 0;															//センター
        ConnectionArraySphere[i * 3 + 1] = i + 2;
        ConnectionArraySphere[i * 3 + 2] = i + 3;
    }
    ConnectionArraySphere[PBVR_SIDES * 1 * 3 - 1] = ConnectionArraySphere[1];									//始点に戻る

    //側面
    for ( int y = 0; y < ( div_v - 2 ); ++y )
    {
        for ( i = PBVR_SIDES, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
        {
            ConnectionArraySphere[( PBVR_SIDES * y + i ) * 3]   = PBVR_SIDES * ( y + 1 ) + sides + 2;
            ConnectionArraySphere[( PBVR_SIDES * y + i ) * 3 + 1] = PBVR_SIDES * ( y + 1 ) + sides + 3;
            ConnectionArraySphere[( PBVR_SIDES * y + i ) * 3 + 2] = PBVR_SIDES * ( y ) + sides + 3;
        }
        ConnectionArraySphere[( PBVR_SIDES * y + i - 1 ) * 3 + 1] = PBVR_SIDES * ( y + 1 ) + 2;									//始点に戻る
        ConnectionArraySphere[( PBVR_SIDES * y + i - 1 ) * 3 + 2] = PBVR_SIDES * ( y ) + 2;
    }

    for ( int y = 0; y < ( div_v - 2 ); ++y )
    {
        for ( i = PBVR_SIDES * ( 1 + div_v - 2 ), sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
        {
            ConnectionArraySphere[( PBVR_SIDES * y + i ) * 3]   = PBVR_SIDES * ( y ) + sides + 3;
            ConnectionArraySphere[( PBVR_SIDES * y + i ) * 3 + 1] = PBVR_SIDES * ( y ) + sides + 2;
            ConnectionArraySphere[( PBVR_SIDES * y + i ) * 3 + 2] = PBVR_SIDES * ( y + 1 ) + sides + 2;
        }
        ConnectionArraySphere[( PBVR_SIDES * y + i - 1 ) * 3] = PBVR_SIDES * ( y ) + 2;										//始点に戻る
    }

    //下面
    for ( i = PBVR_SIDES * ( 1 + ( div_v - 2 ) * 2 ), sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArraySphere[i * 3]   = 2 + PBVR_SIDES * ( PBVR_SIDES - 2 ) + sides;
        ConnectionArraySphere[i * 3 + 1] = 1;															//センター
        ConnectionArraySphere[i * 3 + 2] = 2 + PBVR_SIDES * ( PBVR_SIDES - 2 ) + sides + 1;
    }
    ConnectionArraySphere[PBVR_SIDES * ( 2 + ( PBVR_SIDES - 2 ) * 2 ) * 3 - 1] = 2 + PBVR_SIDES * ( PBVR_SIDES - 2 );							//始点に戻る

    //法線ベクトルの計算
    for ( i = 0; i < ( PBVR_SIDES * 2 + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ) ; i++ )
    {
        CalculateNormal( &verticesSphere[ConnectionArraySphere[i * 3 + 0] * 3], &verticesSphere[ConnectionArraySphere[i * 3 + 1] * 3],
                         &verticesSphere[ConnectionArraySphere[i * 3 + 2] * 3], &NormalArraySphere[i * 3] );
    }
}

//  角度°→ ラジアン変換
double DegreeToRadian( const double degree )
{
    return degree / 180.0 * PBVR_PI;
}

void	CalculatePillarCoord()
{

    int	i, sides;
    double degree;
    float x, y;

    degree = 360 / PBVR_SIDES;

    //頂点座標の設定

    //上面
    verticesPillar[0 * 3 + 0] = param3vd.m_parameter_pillar_center_x;									//センター
    verticesPillar[0 * 3 + 1] = param3vd.m_parameter_pillar_center_y;
    verticesPillar[0 * 3 + 2] = param3vd.m_parameter_pillar_height + param3vd.m_parameter_pillar_center_z;

    for ( i = 1, sides = 0 ; sides < PBVR_SIDES + 1 ; ++sides, ++i )
    {
        x = param3vd.m_parameter_pillar_radius * sin( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_x;
        y = param3vd.m_parameter_pillar_radius * cos( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_y;

        verticesPillar[i * 3 + 0] = x;
        verticesPillar[i * 3 + 1] = y;
        verticesPillar[i * 3 + 2] = param3vd.m_parameter_pillar_height + param3vd.m_parameter_pillar_center_z;
    }

    //下面
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 0] = param3vd.m_parameter_pillar_center_x;							//センター
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 1] = param3vd.m_parameter_pillar_center_y;
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 2] = param3vd.m_parameter_pillar_center_z;

    for ( i = PBVR_SIDES + 2, sides = 0 ;  sides < PBVR_SIDES ; ++sides, ++i )
    {
        x = param3vd.m_parameter_pillar_radius * sin( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_x;
        y = param3vd.m_parameter_pillar_radius * cos( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_y;

        verticesPillar[i * 3 + 0] = x;
        verticesPillar[i * 3 + 1] = y;
        verticesPillar[i * 3 + 2] = param3vd.m_parameter_pillar_center_z;
    }

    //ConnectionArray の設定
    //上面
    for ( i = 0, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = 0;											//センター
        ConnectionArrayPillar[i * 3 + 1] = i + 1;
        ConnectionArrayPillar[i * 3 + 2] = i + 2;

    }
    ConnectionArrayPillar[PBVR_SIDES * 1 * 3 - 1] = ConnectionArrayPillar[1];					//始点に戻る

    //下面
    for ( i = PBVR_SIDES, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = PBVR_SIDES + 1;										//センター
        ConnectionArrayPillar[i * 3 + 1] = i + 3;
        ConnectionArrayPillar[i * 3 + 2] = i + 2;

    }
    ConnectionArrayPillar[PBVR_SIDES * 2 * 3 - 2] = PBVR_SIDES * 1 + 2;									//始点に戻る

    //側面
    for ( i = PBVR_SIDES * 2, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = sides + 1;
        ConnectionArrayPillar[i * 3 + 1] = PBVR_SIDES + sides + 2;
        ConnectionArrayPillar[i * 3 + 2] = PBVR_SIDES + sides + 3;
    }
    ConnectionArrayPillar[PBVR_SIDES * 3 * 3 - 1] = ConnectionArrayPillar[PBVR_SIDES * 2 * 3 + 1];		//始点に戻る

    //側面
    for ( i = PBVR_SIDES * 3, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = PBVR_SIDES + sides + 3;
        ConnectionArrayPillar[i * 3 + 1] = sides + 2;
        ConnectionArrayPillar[i * 3 + 2] = sides + 1;
    }
    ConnectionArrayPillar[PBVR_SIDES * 4 * 3 - 3] = PBVR_SIDES + 2;
    ConnectionArrayPillar[PBVR_SIDES * 4 * 3 - 2] = 1;									//始点に戻る

    //法線ベクトルの計算
    for ( i = 0; i < PBVR_SIDES * 4 ; i++ )
    {
        CalculateNormal( &verticesPillar[ConnectionArrayPillar[i * 3 + 0] * 3], &verticesPillar[ConnectionArrayPillar[i * 3 + 1] * 3],
                         &verticesPillar[ConnectionArrayPillar[i * 3 + 2] * 3], &NormalArrayPillar[i * 3] );
    }
}

void	CalculatePillarXZCoord()
{

    int	i, sides;
    double degree;
    float x , z;

    degree = 360 / PBVR_SIDES;

    //頂点座標の設定

    //上面
    verticesPillar[0 * 3 + 0] = param3vd.m_parameter_pillar_center_x;									//センター
    verticesPillar[0 * 3 + 1] = param3vd.m_parameter_pillar_height + param3vd.m_parameter_pillar_center_y;
    verticesPillar[0 * 3 + 2] = param3vd.m_parameter_pillar_center_z;

    for ( i = 1, sides = 0 ; sides < PBVR_SIDES + 1 ; ++sides, ++i )
    {
        x = param3vd.m_parameter_pillar_radius * sin( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_x;
        z = param3vd.m_parameter_pillar_radius * cos( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_z;

        verticesPillar[i * 3 + 0] = x;
        verticesPillar[i * 3 + 1] = param3vd.m_parameter_pillar_height + param3vd.m_parameter_pillar_center_y;
        verticesPillar[i * 3 + 2] = z;
    }

    //下面
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 0] = param3vd.m_parameter_pillar_center_x;							//センター
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 1] = param3vd.m_parameter_pillar_center_y;
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 2] = param3vd.m_parameter_pillar_center_z;

    for ( i = PBVR_SIDES + 2, sides = 0 ;  sides < PBVR_SIDES ; ++sides, ++i )
    {
        x = param3vd.m_parameter_pillar_radius * sin( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_x;
        z = param3vd.m_parameter_pillar_radius * cos( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_z;
        verticesPillar[i * 3 + 0] = x;
        verticesPillar[i * 3 + 1] = param3vd.m_parameter_pillar_center_y;
        verticesPillar[i * 3 + 2] = z;
    }

    //ConnectionArray の設定

    //上面
    for ( i = 0, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = 0;											//センター
        ConnectionArrayPillar[i * 3 + 1] = i + 1;
        ConnectionArrayPillar[i * 3 + 2] = i + 2;

    }
    ConnectionArrayPillar[PBVR_SIDES * 1 * 3 - 1] = ConnectionArrayPillar[1];					//始点に戻る

    //下面
    for ( i = PBVR_SIDES, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = PBVR_SIDES + 1;										//センター
        ConnectionArrayPillar[i * 3 + 1] = i + 3;
        ConnectionArrayPillar[i * 3 + 2] = i + 2;

    }
    ConnectionArrayPillar[PBVR_SIDES * 2 * 3 - 2] = PBVR_SIDES * 1 + 2;									//始点に戻る

    //側面
    for ( i = PBVR_SIDES * 2, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = sides + 1;
        ConnectionArrayPillar[i * 3 + 1] = PBVR_SIDES + sides + 2;
        ConnectionArrayPillar[i * 3 + 2] = PBVR_SIDES + sides + 3;
    }
    ConnectionArrayPillar[PBVR_SIDES * 3 * 3 - 1] = ConnectionArrayPillar[PBVR_SIDES * 2 * 3 + 1];		//始点に戻る

    //側面
    for ( i = PBVR_SIDES * 3, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = PBVR_SIDES + sides + 3;
        ConnectionArrayPillar[i * 3 + 1] = sides + 2;
        ConnectionArrayPillar[i * 3 + 2] = sides + 1;
    }
    ConnectionArrayPillar[PBVR_SIDES * 4 * 3 - 3] = PBVR_SIDES + 2;
    ConnectionArrayPillar[PBVR_SIDES * 4 * 3 - 2] = 1;									//始点に戻る

    //法線ベクトルの計算
    for ( i = 0; i < PBVR_SIDES * 4 ; i++ )
    {
        CalculateNormal( &verticesPillar[ConnectionArrayPillar[i * 3 + 0] * 3], &verticesPillar[ConnectionArrayPillar[i * 3 + 1] * 3],
                         &verticesPillar[ConnectionArrayPillar[i * 3 + 2] * 3], &NormalArrayPillar[i * 3] );
    }
}

void	CalculatePillarYZCoord()
{

    int	i, sides;
    double degree;
    float y, z;

    degree = 360 / PBVR_SIDES;

    //頂点座標の設定

    //上面
    verticesPillar[0 * 3 + 0] = param3vd.m_parameter_pillar_height + param3vd.m_parameter_pillar_center_x;			//センター
    verticesPillar[0 * 3 + 1] = param3vd.m_parameter_pillar_center_y;
    verticesPillar[0 * 3 + 2] = param3vd.m_parameter_pillar_center_z;

    for ( i = 1, sides = 0 ; sides < PBVR_SIDES + 1 ; ++sides, ++i )
    {
        y = param3vd.m_parameter_pillar_radius * sin( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_y;
        z = param3vd.m_parameter_pillar_radius * cos( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_z;

        verticesPillar[i * 3 + 0] = param3vd.m_parameter_pillar_height + param3vd.m_parameter_pillar_center_x;
        verticesPillar[i * 3 + 1] = y;
        verticesPillar[i * 3 + 2] = z;
    }

    //下面
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 0] = param3vd.m_parameter_pillar_center_x;							//センター
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 1] = param3vd.m_parameter_pillar_center_y;
    verticesPillar[( PBVR_SIDES + 1 ) * 3 + 2] = param3vd.m_parameter_pillar_center_z;

    for ( i = PBVR_SIDES + 2, sides = 0 ;  sides < PBVR_SIDES ; ++sides, ++i )
    {
        y = param3vd.m_parameter_pillar_radius * sin( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_y;
        z = param3vd.m_parameter_pillar_radius * cos( DegreeToRadian( degree * sides ) ) + param3vd.m_parameter_pillar_center_z;
        verticesPillar[i * 3 + 0] = param3vd.m_parameter_pillar_center_x;
        verticesPillar[i * 3 + 1] = y;
        verticesPillar[i * 3 + 2] = z;
    }

    //ConnectionArray の設定

    //上面
    for ( i = 0, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = 0;											//センター
        ConnectionArrayPillar[i * 3 + 1] = i + 1;
        ConnectionArrayPillar[i * 3 + 2] = i + 2;

    }
    ConnectionArrayPillar[PBVR_SIDES * 1 * 3 - 1] = ConnectionArrayPillar[1];					//始点に戻る

    //下面
    for ( i = PBVR_SIDES, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = PBVR_SIDES + 1;										//センター
        ConnectionArrayPillar[i * 3 + 1] = i + 3;
        ConnectionArrayPillar[i * 3 + 2] = i + 2;

    }
    ConnectionArrayPillar[PBVR_SIDES * 2 * 3 - 2] = PBVR_SIDES * 1 + 2;									//始点に戻る

    //側面
    for ( i = PBVR_SIDES * 2, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = sides + 1;
        ConnectionArrayPillar[i * 3 + 1] = PBVR_SIDES + sides + 2;
        ConnectionArrayPillar[i * 3 + 2] = PBVR_SIDES + sides + 3;
    }
    ConnectionArrayPillar[PBVR_SIDES * 3 * 3 - 1] = ConnectionArrayPillar[PBVR_SIDES * 2 * 3 + 1];		//始点に戻る

    //側面
    for ( i = PBVR_SIDES * 3, sides = 0 ; sides < PBVR_SIDES ; ++sides, ++i )
    {
        ConnectionArrayPillar[i * 3]   = PBVR_SIDES + sides + 3;
        ConnectionArrayPillar[i * 3 + 1] = sides + 2;
        ConnectionArrayPillar[i * 3 + 2] = sides + 1;
    }
    ConnectionArrayPillar[PBVR_SIDES * 4 * 3 - 3] = PBVR_SIDES + 2;
    ConnectionArrayPillar[PBVR_SIDES * 4 * 3 - 2] = 1;									//始点に戻る

    //法線ベクトルの計算
    for ( i = 0; i < PBVR_SIDES * 4 ; i++ )
    {
        CalculateNormal( &verticesPillar[ConnectionArrayPillar[i * 3 + 0] * 3], &verticesPillar[ConnectionArrayPillar[i * 3 + 1] * 3],
                         &verticesPillar[ConnectionArrayPillar[i * 3 + 2] * 3], &NormalArrayPillar[i * 3] );
    }
}

//法線ベクトルの計算
int CalculateNormal( const float* p1, const float* p2, const float* p3, float* n )
{
    float v1[3];
    float v2[3];
    float cross[3];
    float length;

    int i;

    /* v1 = p1 - p2を求める */
    for ( i = 0; i < 3; i++ )
    {
        v1[i] = p1[i] - p2[i];
    }

    /* v2 = p3 - p2を求める */
    for ( i = 0; i < 3; i++ )
    {
        v2[i] = p3[i] - p2[i];
    }

    /* 外積v2×v1（= cross）を求める */
    for ( i = 0; i < 3; i++ )
    {
        cross[i] = v2[( i + 1 ) % 3] * v1[( i + 2 ) % 3] - v2[( i + 2 ) % 3] * v1[( i + 1 ) % 3];
    }

    /* 外積v2×v1の長さ|v2×v1|（= length）を求める */
    length = sqrtf( cross[0] * cross[0] +
                    cross[1] * cross[1] +
                    cross[2] * cross[2] );

    /* 長さ|v2×v1|が0のときは法線ベクトルは求められない */
    if ( length == 0.0f )
    {
        return 0;
    }

    /* 外積v2×v1を長さ|v2×v1|で割って法線ベクトルnを求める */
    for ( i = 0; i < 3; i++ )
    {
        n[i] = cross[i] / length;
        if ( n[i] == -0.0F )
            n[i] = 0.0F;
    }
    return 1;
}

#ifdef NOTOPEN
//----------------------------------------------------------
//ディスプレイ・ファンクション(テスト用）
void Display()
{
    glutSetWindow( iwin_main );

    glClear( GL_COLOR_BUFFER_BIT );

    glPushMatrix();
    glMultMatrixf( rotate );
    mapscale = 1.0;

    switch ( obj_3d )
    {
    case	PBVR_CROP:
        CalcCropCoord();
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        crop.drawCrop(  0, 0, 0, 0, PBVR_SOLID );
        glColor3f( 1.0, 0.0, 0.0 );
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        crop.drawCrop(  0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0,  PBVR_MAINPBVR_WINDOW_W, 0 );
        break;

    case	PBVR_SPHERE:
        CalcSphereCoord();
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        sphere.drawSphere(  0, 0, 0, 0, PBVR_SOLID );
        glColor3f( 1.0, 0.0, 0.0 );
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        sphere.drawSphere(  0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, PBVR_MAINPBVR_WINDOW_W, 0 );
        break;

    case	PBVR_PILLARXY:
        pillar.checkPillarTextBox();
        CalcPillarCoord();
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        pillar.drawPillar( 0, 0, 0, 0, PBVR_SOLID );
        glColor3f( 1.0, 0.0, 0.0 );
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        glPushMatrix();
        pillar.drawPillar( 0, 0, 0, 0, PBVR_WIRE );
        glPopMatrix();
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, PBVR_MAINPBVR_WINDOW_W, 0 );
        break;

    case	PBVR_PILLARYZ:
        pillarYZ.checkPillarTextBox();
        CalcPillarYZCoord();
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        pillarYZ.drawPillar( 0, 0, 0, 0, PBVR_SOLID );
        glColor3f( 1.0, 0.0, 0.0 );
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        glPushMatrix();
        pillarYZ.drawPillar( 0, 0, 0, 0, PBVR_WIRE );
        glPopMatrix();
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, PBVR_MAINPBVR_WINDOW_W, 0 );
        break;

    case	PBVR_PILLARXZ:
        pillarXZ.checkPillarTextBox();
        CalcPillarXZCoord();
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        pillarXZ.drawPillar( 0, 0, 0, 0, PBVR_SOLID );
        glColor3f( 1.0, 0.0, 0.0 );
        glViewport( 0, 0, PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
        glPushMatrix();
        pillarXZ.drawPillar( 0, 0, 0, 0, PBVR_WIRE );
        glPopMatrix();
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, PBVR_MAINPBVR_WINDOW_W, 0 );
        break;

    default:
        break;
    }

    glPopMatrix();

    glutSwapBuffers();
    glutPostRedisplay();
}

//--------------------------------------------------------------
#endif

//３面図のディスプレイ・ファンクション
void Display3vd()
{

    glutSetWindow( iwin_3vd );
    Initialize3vd( iwin_3vd );
    mapscale = 1.0;

//	PBVRxmax = PVBRmaxcoords[0]; PBVRymax = PVBRmaxcoords[1]; PBVRzmax = PVBRmaxcoords[2];

//printf("============== Display3vd %f, %f, %f, %f, %f, %f\n", PVBRmincoords[0], PVBRmincoords[1], PVBRmincoords[2], PVBRmaxcoords[0], PVBRmaxcoords[1], PVBRmaxcoords[2] );
//printf("============== ,PBVRxmin %f, %f, %f, %f, %f, %f\n",PBVRxmin, PBVRymin, PBVRzmin, PBVRxmax, PBVRymax, PBVRzmax );


    switch ( obj_3d )
    {

    case	PBVR_CROP:
        CalculateCropCoord();
        glViewport( 0, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_X_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        crop.drawCrop(  90, 0, 1, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 1, 0, PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ), PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Y_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        crop.drawCrop(  90, 1, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 1, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ) );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Z_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        crop.drawCrop(  0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - PBVR_VIEWPORT_HEIGHT );

        break;

    case	PBVR_SPHERE:
        CalculateSphereCoord();
        glViewport( 0, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_X_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        sphere.drawSphere( 90, 0, 1, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 1, 0, PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ), PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Y_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        sphere.drawSphere(  90, 1, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 1, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ) );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Z_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        sphere.drawSphere(  0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - PBVR_VIEWPORT_HEIGHT );

        break;

    case	PBVR_PILLARXY:
        pillar.checkPillarTextBox();
        CalculatePillarCoord();

        glViewport( 0, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_X_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillar.drawPillar( 90, 0, 1, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 1, 0, PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ), PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Y_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillar.drawPillar( 90, 1, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 1, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ) );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Z_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillar.drawPillar( 0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - PBVR_VIEWPORT_HEIGHT );

        break;

    case	PBVR_PILLARYZ:
        pillarYZ.checkPillarTextBox();
        CalculatePillarYZCoord();

        glViewport( 0, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_X_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillarYZ.drawPillar( 90, 0, 1, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 1, 0, PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ), PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Y_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillarYZ.drawPillar( 90, 1, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 1, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ) );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Z_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillarYZ.drawPillar( 0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - PBVR_VIEWPORT_HEIGHT );

        break;

    case	PBVR_PILLARXZ:
        pillarXZ.checkPillarTextBox();
        CalculatePillarXZCoord();

        glViewport( 0, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_X_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillarXZ.drawPillar( 90, 0, 1, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 1, 0, PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ), PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Y_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillarXZ.drawPillar( 90, 1, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 1, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - ( PBVR_VIEWPORT_HEIGHT * 2 ) );

        glViewport( PBVR_VIEWPORT_WIDTH, WindowHeight - PBVR_VIEWPORT_HEIGHT, PBVR_VIEWPORT_WIDTH, PBVR_VIEWPORT_HEIGHT );
        DrawFrame( 2.00 );
        DrawFrame( 1.99 );
        if ( win_sel == PBVR_Z_VIEW )
            DrawFrame( 1.98 );
        glColor3f( 1.0, 0.0, 0.0 );
        pillarXZ.drawPillar( 0, 0, 0, 0, PBVR_WIRE );
        GLDrawNormal( PBVR_VEC_LEN, 0, 0, 0, ( PBVR_VIEWPORT_WIDTH * 2 ), WindowHeight - PBVR_VIEWPORT_HEIGHT );

        break;

    default:
        break;
    }

    End3vd( iwin_3vd );
}

//３面図用ウィンドウの初期化
void Initialize3vd( const int winid )
{
    glutSetWindow( winid );
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear( GL_COLOR_BUFFER_BIT );

    glPushMatrix();
}

//３面図用ウィンドウの終了処理
void End3vd( const int winid )
{

    glPopMatrix();
//	glutSetWindow(winid);
    glutSwapBuffers();
    glutPostRedisplay();
}

//RESET ボタン用コールバック（オブジェクトを初期化）
void GUIReset( const int num )
{
    switch ( obj_3d )
    {
    case	PBVR_CROP:
        crop.motionInitialize();
        crop.initializeCrop();
        param3vd.CropParaSet();
        break;
    case	PBVR_SPHERE:
        sphere.motionInitialize();
        sphere.initializeSphere( 1 );
        param3vd.SphereParaSet();
        break;
    case	PBVR_PILLARXY:
        pillar.motionInitialize();
        pillar.initializePillar( 1 );
        pillar.setPillarTextBox();
        param3vd.PillarParaSet();
        break;
    case	PBVR_PILLARYZ:
        pillarYZ.motionInitialize();
        pillarYZ.initializePillar( 1 );
        pillarYZ.setPillarTextBox();
        param3vd.PillarParaSet();
        break;
    case	PBVR_PILLARXZ:
        pillarXZ.motionInitialize();
        pillarXZ.initializePillar( 1 );
        pillarXZ.setPillarTextBox();
        param3vd.PillarParaSet();
        break;
    default:
        break;
    }
}

//パラメータ用オブジェクトに値をセット
void	CropCubeParameter::CropParaSet()
{
    m_parameter_cx_min  = cxminlevel2;
    m_parameter_cx_max  = cxmaxlevel2;
    m_parameter_cy_min  = cyminlevel2;
    m_parameter_cy_max  = cymaxlevel2;
    m_parameter_cz_min  = czminlevel2;
    m_parameter_cz_max  = czmaxlevel2;
}

void	CropSphereParameter::SphereParaSet()
{
    m_parameter_sphere_radius  = sradius;
    m_parameter_sphere_center_x = scenterx;
    m_parameter_sphere_center_y = scentery;
    m_parameter_sphere_center_z = scenterz;
}

void	CropPillerParameter::PillarParaSet()
{
    m_parameter_pillar_radius  = pradius;
    m_parameter_pillar_height  = pheight;
    m_parameter_pillar_center_x = pcenterx;
    m_parameter_pillar_center_y = pcentery;
    m_parameter_pillar_center_z = pcenterz;
}

//APPLYボタン用コールバック
void GUIApply( const int num )
{
    param3vd.m_apply = PBVR_ON;
    param3vd.m_select_area = obj_3d;

    m_parameter_crop->m_crop_type = ( obj_3d == PBVR_CROP ) ?     CropParameter::Crop :
                                    ( obj_3d == PBVR_SPHERE ) ?   CropParameter::Sphere :
                                    ( obj_3d == PBVR_PILLAR ) ?   CropParameter::Pillar :
                                    ( obj_3d == PBVR_PILLARYZ ) ? CropParameter::PillarYZ :
                                    ( obj_3d == PBVR_PILLARXZ ) ? CropParameter::PillarXZ :
                                    CropParameter::NoCrop;

    m_parameter_crop->m_region_crop_x_min_level = tbcxminlevel;
    m_parameter_crop->m_region_crop_x_max_level = tbcxmaxlevel;
    m_parameter_crop->m_region_crop_y_min_level = tbcyminlevel;
    m_parameter_crop->m_region_crop_y_max_level = tbcymaxlevel;
    m_parameter_crop->m_region_crop_z_min_level = tbczminlevel;
    m_parameter_crop->m_region_crop_z_max_level = tbczmaxlevel;
    m_parameter_crop->m_region_sphere_radius    = tbsradius;
    m_parameter_crop->m_region_sphere_center_x   = tbscenterx;
    m_parameter_crop->m_region_sphere_center_y   = tbscentery;
    m_parameter_crop->m_region_sphere_center_z   = tbscenterz;
    m_parameter_crop->m_region_pillar_radius    = tbpradius;
    m_parameter_crop->m_region_pillar_height    = tbpheight;
    m_parameter_crop->m_region_pillar_center_x   = tbpcenterx;
    m_parameter_crop->m_region_pillar_center_y   = tbpcentery;
    m_parameter_crop->m_region_pillar_center_z   = tbpcenterz;
}

//CLOSEボタン用コールバック
void CallBackGLUI( const int num )
{
    glutSetWindow( iwin_3vd );
    glutHideWindow();
    obj_3d = 0;
    param3vd.m_apply = PBVR_OFF;
    m_parameter_crop->m_crop_type = CropParameter::NoCrop;
#ifndef NOTOPEN
    crop_flg = PBVR_FALSE;
#endif
}

//表示するオブジェクトの選択
void Select3vd( const int num )
{

    obj_3d = obj_sel + 1;

    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:
        pradius  = pillar.m_radius;
        pheight  = pillar.m_height;
        pcenterx = pillar.m_center_x;
        pcentery = pillar.m_center_y;
        pcenterz = pillar.m_center_z;
        pillar.setPillarTextBox();
        pillar.rem_loadPillarTextBox();
        param3vd.PillarParaSet();
        break;

    case	PBVR_PILLARYZ:
        pradius  = pillarYZ.m_radius;
        pheight  = pillarYZ.m_height;
        pcenterx = pillarYZ.m_center_x;
        pcentery = pillarYZ.m_center_y;
        pcenterz = pillarYZ.m_center_z;
        pillarYZ.setPillarTextBox();
        pillarYZ.rem_loadPillarTextBox();
        param3vd.PillarParaSet();
        break;

    case	PBVR_PILLARXZ:
        pradius  = pillarXZ.m_radius;
        pheight  = pillarXZ.m_height;
        pcenterx = pillarXZ.m_center_x;
        pcentery = pillarXZ.m_center_y;
        pcenterz = pillarXZ.m_center_z;
        pillarXZ.setPillarTextBox();
        pillarXZ.rem_loadPillarTextBox();
        param3vd.PillarParaSet();
        break;

    default:
        break;
    }
#ifdef __APPLE__
    Display3vd();
#endif
}

// *** 球の描画 ***
void Sphere::drawSphere( const int angle , const int dirx, const int diry, const int dirz, const int type )
{
    glPushMatrix();

    glRotated( angle, dirx, diry, dirz );
    objectControl();

    //ワイヤーフレーム
    if ( type == PBVR_WIRE )
    {
        DrawSphere( PBVR_WIRE );
    }

    //塗りつぶし
    else if ( type == PBVR_SOLID )
    {
        DrawSphere( PBVR_SOLID );
    }

    glPopMatrix();
}

// *** 立方体の描画 ***
void Crop::drawCrop( const int angle, const int dirx, const int diry, const int dirz, const int type )
{
    glPushMatrix();

    glRotated( angle, dirx, diry, dirz );
    objectControl();

    if ( type == PBVR_WIRE )
    {
        DrawCrop( PBVR_WIRE );
    }

    if ( type == PBVR_SOLID )
    {
        DrawCrop( PBVR_SOLID );
    }

    glPopMatrix();
}

// *** 円柱の描画 ***
void Pillar::drawPillar( const int angle , const int dirx, const int diry, const int dirz, const int type )
{
    glPushMatrix();

    //回転・中央に移動
    if ( dirx )
    {
        glRotated( angle, 1, 0, 0 );
    }
    if ( diry )
    {
        glRotated( angle, 0, 1, 0 );
    }
    if ( dirz )
    {
        glRotated( angle, 0, 0, 1 );
    }

    objectControl();

    if ( type == PBVR_WIRE )
    {
        DrawPillar( PBVR_WIRE );
    }

    else if ( type == PBVR_SOLID )
    {
        DrawPillar( PBVR_SOLID );
    }

    glPopMatrix();
}

//法線の描画
void	GLDrawNormal( const double length, const int dirx, const int diry,
                      const int dirz, const int viewport_x, const int viewport_y )
{
    GLUquadricObj* arrows[3];

    glViewport( viewport_x - PBVR_VECTOR_VIEW_SIZE - 20, viewport_y, PBVR_VECTOR_VIEW_SIZE, PBVR_VECTOR_VIEW_SIZE );

    glPushMatrix();

    //回転
    if ( dirx )
    {
        glRotated( 90, 1, 0, 0 );
    }
    if ( diry )
    {
        glRotated( 90, 0, 1, 0 );
    }
    if ( dirz )
    {
        glRotated( 90, 0, 0, 1 );
    }

    // Draw X-axis
    glColor3ub( 255, 0, 0 );
    glBegin( GL_LINES );
    glVertex3d( -length, 0, 0 );
    glVertex3d( length, 0, 0 );
    glEnd();
    glPushMatrix();
    arrows[0] = gluNewQuadric();
    gluQuadricDrawStyle( arrows[0], GLU_FILL );
    glTranslated( length, 0.0f, 0.0f );
    glRotated( 90.0f, 0, 1, 0 );
    gluCylinder( arrows[0], length / 40, 0.0f, length / 10, 8, 8 );

    DisplayString( length / 10, 0.0, 0.0, "X" );

    glPopMatrix();

    // Draw Y-axis
    glColor3ub(  0, 255, 0 );
    glBegin( GL_LINES );
    glVertex3d( 0, -length, 0 );
    glVertex3d( 0, length, 0 );
    glEnd();
    glPushMatrix();
    arrows[1] = gluNewQuadric();
    gluQuadricDrawStyle( arrows[1], GLU_FILL );
    glTranslated( 0.0f, length, 0.0f );
    glRotated( -90.0f, 1, 0, 0 );
    gluCylinder( arrows[1], length / 40, 0.0f, length / 10, 8, 8 );
    DisplayString( 0.0, length / 10, 0.0, "Y" );

    glPopMatrix();

    // Draw Z-axis
    glColor3ub(  0, 0, 255 );
    glBegin( GL_LINES );
    glVertex3d( 0, 0, -length );
    glVertex3d( 0, 0, length );
    glEnd();
    glPushMatrix();
    arrows[2] = gluNewQuadric();
    gluQuadricDrawStyle( arrows[2], GLU_FILL );
    glTranslated( 0.0f, 0.0f, length );
    gluCylinder( arrows[2], length / 40, 0.0f, length / 10, 8, 8 );
    DisplayString( 0.0, 0.0, length / 10, "Z" );
    glPopMatrix();

    glPopMatrix();
}

//文字列の表示
void DisplayString( const float x, const float  y, const float z, const char* str )
{
    glRasterPos3f( x, y, z );
    char* p = ( char* )str;
    while ( *p != '\0' )glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, *p++ );
}

//ビューポートの枠の描画
void	DrawFrame( const double size )
{
    glColor3f( 1.0, 1.0, 1.0 );
    glutWireCube( size );
}

//円柱用テキストボックスの値が変更されていたら、描画用のパラメータを更新
void	Pillar::checkPillarTextBox()
{
    if ( tbpradius != m_save_tbpradius )
    {
        CallBackPillarRadius( 0 );
        m_save_tbpradius = tbpradius;
    }
    if ( tbpheight  != m_save_tbpheight )
    {
        CallBackPillarHeight( 0 );
        m_save_tbpheight = tbpheight;
    }
    if ( tbpcenterx != m_save_tbpcenterx )
    {
        CallBackPillarCenterX( 0 );
        m_save_tbpcenterx = tbpcenterx;
    }
    if ( tbpcentery != m_save_tbpcentery )
    {
        CallBackPillarCenterY( 0 );
        m_save_tbpcentery = tbpcentery;
    }
    if ( tbpcenterz != m_save_tbpcenterz )
    {
        CallBackPillarCenterZ( 0 );
        m_save_tbpcenterz = tbpcenterz;
    }
}

//オブジェクトの操作
void	Motion::objectControl()
{
    double	x, y, z;

    x = m_move_x / ( double )PBVR_VIEWPORT_WIDTH  / m_scale_x;
    y = m_move_y / ( double )PBVR_VIEWPORT_HEIGHT / m_scale_y;
    z = m_move_z / ( double )PBVR_VIEWPORT_WIDTH  / m_scale_z;

//	glScaled( scale_x, scale_y, scale_z );
//	glTranslated( x*mapscale, y*mapscale, z*mapscale );
}

//各オブジェクトの初期化
void	Motion::motionInitialize()
{
    m_previous_x = 0;
    m_previous_y = 0;
    m_previous_z = 0;
    m_move_x = 0;
    m_move_y = 0;
    m_move_z = 0;
    m_save_x = 0;
    m_save_y = 0;
    m_save_z = 0;
    m_scale_x = 1.0F;
    m_scale_y = 1.0F;
    m_scale_z = 1.0F;
}

void	Crop::initializeCrop( const int flag )
{
    printf( "Call Crop::initializeCrop()\n" );
    m_sides = PBVR_SIDES;
    /*
    	cxminlevel2 = PBVR_CROP_X_MIN;
    	cxmaxlevel2 = PBVR_CROP_X_MAX;
    	cyminlevel2 = PBVR_CROP_Y_MIN;
    	cymaxlevel2 = PBVR_CROP_Y_MAX;
    	czminlevel2 = PBVR_CROP_Z_MIN;
    	czmaxlevel2 = PBVR_CROP_Z_MAX;

    	save_cxmin = PBVR_CROP_X_MIN;
    	save_cxmax = PBVR_CROP_X_MAX;
    	save_cymin = PBVR_CROP_Y_MIN;
    	save_cymax = PBVR_CROP_Y_MAX;
    	save_czmin = PBVR_CROP_Z_MIN;
    	save_czmax = PBVR_CROP_Z_MAX;
    */
    //CROP BOXの初期値をグローバルな座標のmin,maxに設定する
    double maxwidth = PBVRxmax - PBVRxmin;
    if ( maxwidth < PBVRymax - PBVRymin ) maxwidth = PBVRymax - PBVRymin;
    if ( maxwidth < PBVRzmax - PBVRzmin ) maxwidth = PBVRzmax - PBVRzmin;
    maxwidth /= PBVR_WIN3VDSCALE;
    cxmaxlevel2 = ( PBVRxmax - PBVRxmin ) / maxwidth;
    cxminlevel2 = -cxmaxlevel2;
    cymaxlevel2 = ( PBVRymax - PBVRymin ) / maxwidth;
    cyminlevel2 = -cymaxlevel2;
    czmaxlevel2 = ( PBVRzmax - PBVRzmin ) / maxwidth;
    czminlevel2 = -czmaxlevel2;

    save_cxmin = cxminlevel2;
    save_cxmax = cxmaxlevel2;
    save_cymin = cyminlevel2;
    save_cymax = cymaxlevel2;
    save_czmin = czminlevel2;
    save_czmax = czmaxlevel2;

    /*
    	m_side_x = 1.0;
    	m_side_y = 1.0;
    	m_side_z = 1.0;
    */
    m_side_x = cxmaxlevel2 - cxminlevel2;
    m_side_y = cymaxlevel2 - cyminlevel2;
    m_side_z = czmaxlevel2 - czminlevel2;
    m_move_x = ( cxmaxlevel2 + cxminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    m_move_y = ( cymaxlevel2 + cyminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    m_move_z = ( czmaxlevel2 + czminlevel2 ) * (  PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    savePosition( m_move_x, m_move_y, m_move_z );

    if ( flag == 0 )
    {
        setCropTextBox();
        crop.motionInitialize();
    }
}

void	Sphere::initializeSphere( const int flag )
{
    m_sides = PBVR_SIDES;
    m_radius = ( PBVRxmax - PBVRxmin ) * ( PBVRxmax - PBVRxmin ) + ( PBVRymax - PBVRymin ) * ( PBVRymax - PBVRymin ) + ( PBVRzmax - PBVRzmin ) * ( PBVRzmax - PBVRzmin );
    m_radius = sqrt ( m_radius ) / 2.0F;
    m_radius = m_radius * PBVRCoordScale;
    Calculate3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin + ( PBVRxmax - PBVRxmin ) / 2.0F, &m_center_x );
    Calculate3vdCoord( PBVRymax, PBVRymin, PBVRymin + ( PBVRymax - PBVRymin ) / 2.0F, &m_center_y );
    Calculate3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin + ( PBVRzmax - PBVRzmin ) / 2.0F, &m_center_z );
    m_save_center_x = m_center_x;
    m_save_center_y = m_center_y;
    m_save_center_z = m_center_z;
    m_save_radius = m_radius;

    sradius = m_radius;
    scenterx = m_center_x;
    scentery = m_center_y;
    scenterz = m_center_z;

    m_move_x = m_center_x * ( PBVR_VIEWPORT_WIDTH / 2 );
    m_move_y = m_center_y * ( PBVR_VIEWPORT_HEIGHT / 2 );
    m_move_z = m_center_z * ( PBVR_VIEWPORT_WIDTH / 2 );
    savePosition( m_move_x, m_move_y, m_move_z );

    if ( flag == 0 )
    {
        setSphereTextBox();
        sphere.motionInitialize();
    }
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, scenterx, &tbscenterx );
    CalculatePBVRCoord( PBVRymax, PBVRymin, scentery, &tbscentery );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, scenterz, &tbscenterz );
    tbsradius = sradius / PBVRCoordScale;
}

void	Pillar::initializePillar( const int flag )
{
    switch ( obj_3d )
    {
    case    PBVR_PILLARXY:
        m_radius = ( PBVRxmax - PBVRxmin ) * ( PBVRxmax - PBVRxmin ) + ( PBVRymax - PBVRymin ) * ( PBVRymax - PBVRymin );
        m_height = ( PBVRzmax - PBVRzmin ) * PBVRCoordScale;
        Calculate3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin + ( PBVRxmax - PBVRxmin ) / 2.0F, &m_center_x );
        Calculate3vdCoord( PBVRymax, PBVRymin, PBVRymin + ( PBVRymax - PBVRymin ) / 2.0F, &m_center_y );
        Calculate3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin, &m_center_z );
        break;
    case    PBVR_PILLARYZ:
        m_radius = ( PBVRymax - PBVRymin ) * ( PBVRymax - PBVRymin ) + ( PBVRzmax - PBVRzmin ) * ( PBVRzmax - PBVRzmin );
        m_height = ( PBVRxmax - PBVRxmin ) * PBVRCoordScale;
        Calculate3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin, &m_center_x );
        Calculate3vdCoord( PBVRymax, PBVRymin, PBVRymin + ( PBVRymax - PBVRymin ) / 2.0F, &m_center_y );
        Calculate3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin + ( PBVRzmax - PBVRzmin ) / 2.0F, &m_center_z );
        break;
    case    PBVR_PILLARXZ:
        m_radius = ( PBVRxmax - PBVRxmin ) * ( PBVRxmax - PBVRxmin ) + ( PBVRzmax - PBVRzmin ) * ( PBVRzmax - PBVRzmin );
        m_height = ( PBVRymax - PBVRymin ) * PBVRCoordScale;
        Calculate3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin + ( PBVRxmax - PBVRxmin ) / 2.0F, &m_center_x );
        Calculate3vdCoord( PBVRymax, PBVRymin, PBVRymin, &m_center_y );
        Calculate3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin + ( PBVRzmax - PBVRzmin ) / 2.0F, &m_center_z );
        break;
    default:
        break;
    }
    m_radius = sqrt ( m_radius ) / 2.0F;
    m_radius = m_radius * PBVRCoordScale;
//printf("=== initializePillar() %f, %f, %f, %f, %f\n", m_center_x, m_center_y, m_center_z, m_radius, height);

    m_save_center_x = m_center_x; //0.0F;
    m_save_center_y = m_center_y; //0.0F;
    m_save_center_z = m_center_z; //-0.5F; //0.0F;
    m_save_radius = m_radius; //PBVR_PRADIUS;
    m_save_height = m_height; //PBVR_PHEIGHT;

    pradius = m_radius; //PBVR_PRADIUS;
    pheight = m_height; //PBVR_PHEIGHT;
    pcenterx = m_center_x; //0.0F;
    pcentery = m_center_y; //0.0F;
    pcenterz = m_center_z; //-0.5F; //0.0F;

    m_move_x = m_center_x * ( PBVR_VIEWPORT_WIDTH / 2 );
    m_move_y = m_center_y * ( PBVR_VIEWPORT_HEIGHT / 2 );
    m_move_z = m_center_z * ( PBVR_VIEWPORT_WIDTH / 2 );
    savePosition( m_move_x, m_move_y, m_move_z );

    if ( flag == 0 )
    {
        setPillarTextBox();
        motionInitialize();
    }

    m_save_tbpradius  = tbpradius;
    m_save_tbpheight  = tbpheight;
    m_save_tbpcenterx = tbpcenterx;
    m_save_tbpcentery = tbpcentery;
    m_save_tbpcenterz = tbpcenterz;
}

//Crop最大座標値のセット
void	SetCropMaxMin( const float x, const float y, const float z )
{
    if ( CropTrans | PBVR_RIGHTSIDE )
    {
        CropXmax = save_cxmax + x / crop.m_scale_x;
    }
    if ( CropTrans | PBVR_LEFTSIDE )
    {
        CropXmin = save_cxmin + x / crop.m_scale_x;
    }
    if ( CropTrans | PBVR_TOP_SIDE )
    {
        CropYmax = save_cymax + y / crop.m_scale_y;
    }
    if ( CropTrans | PBVR_BOTTOM_SIDE )
    {
        CropYmin = save_cymin + y / crop.m_scale_y;
    }
    if ( CropTrans | PBVR_SURFSIDE )
    {
        CropZmax = save_czmax + z / crop.m_scale_z;
    }
    if ( CropTrans | PBVR_DEPTH_SIDE )
    {
        CropZmin = save_czmin + z / crop.m_scale_z;
    }
}

//Sphere最大座標値のセット
void	SetSphereMaxMin( const float x, const float y, const float z, const float rad )
{
    if ( ModifyMode == PBVR_OFF )
    {
        if ( x )
        {
            SphereXmin = sphere.m_save_center_x - sphere.m_radius / sphere.m_scale_x + x / sphere.m_scale_x;
            SphereXmax = sphere.m_save_center_x + sphere.m_radius / sphere.m_scale_x + x / sphere.m_scale_x;
        }
        if ( y )
        {
            SphereYmin = sphere.m_save_center_y - sphere.m_radius / sphere.m_scale_y + y / sphere.m_scale_y;
            SphereYmax = sphere.m_save_center_y + sphere.m_radius / sphere.m_scale_y + y / sphere.m_scale_y;
        }
        if ( z )
        {
            SphereZmin = sphere.m_save_center_z - sphere.m_radius / sphere.m_scale_z + z / sphere.m_scale_z;
            SphereZmax = sphere.m_save_center_z + sphere.m_radius / sphere.m_scale_z + z / sphere.m_scale_z;
        }
    }
    else
    {
        SphereXmin = sphere.m_save_center_x - sphere.m_save_radius / sphere.m_scale_x - rad;
        SphereXmax = sphere.m_save_center_x + sphere.m_save_radius / sphere.m_scale_x + rad;
        SphereYmin = sphere.m_save_center_y - sphere.m_save_radius / sphere.m_scale_y - rad;
        SphereYmax = sphere.m_save_center_y + sphere.m_save_radius / sphere.m_scale_y + rad;
        SphereZmin = sphere.m_save_center_z - sphere.m_save_radius / sphere.m_scale_z - rad;
        SphereZmax = sphere.m_save_center_z + sphere.m_save_radius / sphere.m_scale_z + rad;
    }
}


//Pillar最大座標値のセット
void	SetPillarXYMaxMin( const float x, const float y, const float z, const float rad, const float heigttop, const float heigtbase )
{
    if ( ModifyMode == PBVR_OFF )
    {
        if ( x )
        {
            PillarXYXmin = pillar.m_save_center_x - pillar.m_radius / pillar.m_scale_x + x / pillar.m_scale_x;
            PillarXYXmax = pillar.m_save_center_x + pillar.m_radius / pillar.m_scale_x + x / pillar.m_scale_x;
        }
        if ( y )
        {
            PillarXYYmin = pillar.m_save_center_y - pillar.m_radius / pillar.m_scale_y + y / pillar.m_scale_y;
            PillarXYYmax = pillar.m_save_center_y + pillar.m_radius / pillar.m_scale_y + y / pillar.m_scale_y;
        }
        if ( z )
        {
            PillarXYZmin = pillar.m_save_center_z + z / pillar.m_scale_z;
            PillarXYZmax = pillar.m_save_center_z + pillar.m_height / pillar.m_scale_z + z / pillar.m_scale_z;
        }
    }

    else
    {
        PillarXYXmin = pillar.m_save_center_x - pillar.m_save_radius / pillar.m_scale_x - rad;
        PillarXYXmax = pillar.m_save_center_x + pillar.m_save_radius / pillar.m_scale_x + rad;
        PillarXYYmin = pillar.m_save_center_y - pillar.m_save_radius / pillar.m_scale_y - rad;
        PillarXYYmax = pillar.m_save_center_y + pillar.m_save_radius / pillar.m_scale_y + rad;
        if ( heigttop )
        {
            PillarXYZmin = pillar.m_save_center_z;
            PillarXYZmax = pillar.m_save_center_z + ( pillar.m_save_height / pillar.m_scale_z ) + heigttop;
        }
        else if ( heigtbase )
        {
            PillarXYZmin = pillar.m_save_center_z + heigtbase;
            PillarXYZmax = pillar.m_save_center_z + ( pillar.m_save_height / pillar.m_scale_z );
        }
    }
}

//Pillar最大座標値のセット
void	SetPillarXZMaxMin( const float x, const float y, const float z, const float rad, const float heigttop, const float heigtbase )
{
    if ( ModifyMode == PBVR_OFF )
    {
        if ( x )
        {
            PillarXZXmin = pillarXZ.m_save_center_x - pillarXZ.m_radius / pillarXZ.m_scale_x + x / pillarXZ.m_scale_x;
            PillarXZXmax = pillarXZ.m_save_center_x + pillarXZ.m_radius / pillarXZ.m_scale_x + x / pillarXZ.m_scale_x;
        }
        if ( y )
        {
            PillarXZYmin = pillarXZ.m_save_center_y + z / pillarXZ.m_scale_y;
            PillarXZYmax = pillarXZ.m_save_center_y + pillarXZ.m_height / pillarXZ.m_scale_y + y / pillarXZ.m_scale_y;
        }
        if ( z )
        {
            PillarXZZmin = pillarXZ.m_save_center_z - pillarXZ.m_radius / pillarXZ.m_scale_z + z / pillarXZ.m_scale_z;
            PillarXZZmax = pillarXZ.m_save_center_z + pillarXZ.m_radius / pillarXZ.m_scale_z + z / pillarXZ.m_scale_z;
        }
    }

    else
    {
        PillarXZXmin = pillarXZ.m_save_center_x - pillarXZ.m_save_radius / pillarXZ.m_scale_x - rad;
        PillarXZXmax = pillarXZ.m_save_center_x + pillarXZ.m_save_radius / pillarXZ.m_scale_x + rad;
        PillarXZZmin = pillarXZ.m_save_center_z - pillarXZ.m_save_radius / pillarXZ.m_scale_z - rad;
        PillarXZZmax = pillarXZ.m_save_center_z + pillarXZ.m_save_radius / pillarXZ.m_scale_z + rad;
        if ( heigttop )
        {
            PillarXZYmin = pillarXZ.m_save_center_y;
            PillarXZYmax = pillarXZ.m_save_center_y + ( pillarXZ.m_save_height / pillarXZ.m_scale_y ) + heigttop;
        }
        else if ( heigtbase )
        {
            PillarXZYmin = pillarXZ.m_save_center_y + heigtbase;
            PillarXZYmax = pillarXZ.m_save_center_y + ( pillarXZ.m_save_height / pillarXZ.m_scale_y );
        }
    }
}

//Pillar最大座標値のセット
void	SetPillarYZMaxMin( const float x, const float y, const float z, const float rad, const float heigttop, const float heigtbase )
{
    if ( ModifyMode == PBVR_OFF )
    {
        if ( x )
        {
            PillarYZXmin = pillarYZ.m_save_center_x + z / pillarYZ.m_scale_x;
            PillarYZXmax = pillarYZ.m_save_center_x + pillarYZ.m_height / pillarYZ.m_scale_x + x / pillarYZ.m_scale_x;
        }
        if ( y )
        {
            PillarYZYmin = pillarYZ.m_save_center_y - pillarYZ.m_radius / pillarYZ.m_scale_y + y / pillarYZ.m_scale_y;
            PillarYZYmax = pillarYZ.m_save_center_y + pillarYZ.m_radius / pillarYZ.m_scale_y + y / pillarYZ.m_scale_y;
        }
        if ( z )
        {
            PillarYZZmin = pillarYZ.m_save_center_z - pillarYZ.m_radius / pillarYZ.m_scale_z + z / pillarYZ.m_scale_z;
            PillarYZZmax = pillarYZ.m_save_center_z + pillarYZ.m_radius / pillarYZ.m_scale_z + z / pillarYZ.m_scale_z;
        }
    }

    else
    {
        PillarYZYmin = pillarYZ.m_save_center_y - pillarYZ.m_save_radius / pillarYZ.m_scale_y - rad;
        PillarYZYmax = pillarYZ.m_save_center_y + pillarYZ.m_save_radius / pillarYZ.m_scale_y + rad;
        PillarYZZmin = pillarYZ.m_save_center_z - pillarYZ.m_save_radius / pillarYZ.m_scale_z - rad;
        PillarYZZmax = pillarYZ.m_save_center_z + pillarYZ.m_save_radius / pillarYZ.m_scale_z + rad;
        if ( heigttop )
        {
            PillarYZXmin = pillarYZ.m_save_center_x;
            PillarYZXmax = pillarYZ.m_save_center_x + ( pillarYZ.m_save_height / pillarYZ.m_scale_x ) + heigttop;
        }
        else if ( heigtbase )
        {
            PillarYZXmin = pillarYZ.m_save_center_x + heigtbase;
            PillarYZXmax = pillarYZ.m_save_center_x + ( pillarYZ.m_save_height / pillarYZ.m_scale_x );
        }
    }
}

//拡大/縮小割合のセット
void	Motion::setScale( const float x, const float y, const float z )
{
    if ( x )
        m_scale_x = x;
    if ( y )
        m_scale_y = y;
    if ( z )
        m_scale_z = z;
}

//拡大・縮小率の計算
void	Motion::scaling( const float x, const float y, const float z )
{
    //０で指定された方向の倍率は更新しない
    if ( x )
        m_scale_x *= x;
    if ( y )
        m_scale_y *= y;
    if ( z )
        m_scale_z *= z;
}

//マウスボタンを押したときの座標を保持
void	Motion::setRetentionPosition( const int x, const int y, const int z )
{
    m_previous_x = x;
    m_previous_y = y;
    m_previous_z = z;
}

//マウスボタンを押した時から離すまでの移動量を保持
void	Motion::savePosition( const int x, const int y, const int z )
{
    if ( x )
        m_save_x = x;
    if ( y )
        m_save_y = y ;
    if ( z )
        m_save_z = z ;
}

//移動距離を計算しセット
void	Motion::setPosition( const int x, const int y, const int z )
{
    //０で指定された方向の座標は更新しない
    if ( m_previous_x && x )
    {
        m_move_x = m_save_x + x - m_previous_x;
    }
    if ( m_previous_y && y )
    {
        m_move_y = m_save_y + y - m_previous_y;
    }
    if ( m_previous_z && z )
    {
        m_move_z = m_save_z + z - m_previous_z;
    }
}

//直方体・指定倍率での拡大/縮小処理
void	Crop::scalingCrop( const float scalex, const float scaley, const float scalez )
{

    ModifyMode = PBVR_ON;

    if ( scalex && scaley && scalez )
    {
        CropTrans = ( PBVR_RIGHTSIDE | PBVR_LEFTSIDE | PBVR_TOP_SIDE | PBVR_BOTTOM_SIDE | PBVR_SURFSIDE | PBVR_DEPTH_SIDE );
        SetCropMaxMin( crop.m_side_x * ( 1 - scalex ), crop.m_side_y * ( 1 - scaley ), crop.m_side_z * ( 1 - scalez ) );

        if ( ( ( crop.m_side_x * scalex > PBVR_SCALE_MIN ) && ( crop.m_side_y * scaley > PBVR_SCALE_MIN ) && ( crop.m_side_z * scalez > PBVR_SCALE_MIN ) ) )
        {

            if ( ( ( CropXmin >= Xmin * 2 ) && ( ( CropXmax <= Xmax * 2 ) || ( scalex < 1.0 ) ) )
                    && ( ( CropYmin >= Ymin * 2 ) && ( ( CropYmax <= Ymax * 2 ) || ( scaley < 1.0 ) ) )
                    && ( ( CropZmin >= Zmin * 2 ) && ( ( CropZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) ) )
            {

                crop.m_side_x *=  scalex;
                crop.m_side_y *=  scaley;
                crop.m_side_z *=  scalez;

                setCropTextData();
                param3vd.CropParaSet();
                save_cxmax = cxmaxlevel2;
                save_cxmin = cxminlevel2;
                save_cymax = cymaxlevel2;
                save_cymin = cyminlevel2;
                save_czmax = czmaxlevel2;
                save_czmin = czminlevel2;
            }

            ModifyMode = PBVR_OFF;
            CropTrans = PBVR_OFF;
            return;
        }
    }


    else if ( scalex )
    {
        CropTrans |= ( PBVR_RIGHTSIDE | PBVR_LEFTSIDE );
        SetCropMaxMin( crop.m_side_x * ( 1 - scalex ), 0, 0 );
        if ( ( ( CropXmin >= Xmin * 2 ) && ( ( CropXmax <= Xmax * 2 ) || ( scalex < 1.0 ) ) && ( crop.m_side_x * scalex > PBVR_SCALE_MIN ) ) )
        {

            crop.m_side_x *=  scalex;
            setCropTextData();
            param3vd.CropParaSet();
            save_cxmax = cxmaxlevel2;
            save_cxmin = cxminlevel2;
        }
    }

    else if ( scaley )
    {
        CropTrans |= ( PBVR_TOP_SIDE | PBVR_BOTTOM_SIDE );
        SetCropMaxMin( 0, crop.m_side_y * ( 1 - scaley ), 0 );
        if ( ( ( CropYmin >= Ymin * 2 ) && ( ( CropYmax <= Ymax * 2 ) || ( scaley < 1.0 ) ) && ( crop.m_side_y * scaley > PBVR_SCALE_MIN ) ) )
        {

            crop.m_side_y *=  scaley;
            setCropTextData();
            param3vd.CropParaSet();
            save_cymax = cymaxlevel2;
            save_cymin = cyminlevel2;
        }
    }

    else if ( scalez )
    {
        CropTrans |= ( PBVR_SURFSIDE | PBVR_DEPTH_SIDE );
        SetCropMaxMin( 0, 0, crop.m_side_z * ( 1 - scalez ) );
        if ( ( ( CropZmin >= Zmin * 2 ) && ( ( CropZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) && ( crop.m_side_z * scalez > PBVR_SCALE_MIN ) ) )
        {

            crop.m_side_z *=  scalez;
            setCropTextData();
            param3vd.CropParaSet();
            save_czmax = czmaxlevel2;
            save_czmin = czminlevel2;
        }
    }
    ModifyMode = PBVR_OFF;
    CropTrans = PBVR_OFF;
}

//球・指定倍率での拡大/縮小処理
void	Sphere::scalingSphere( const float scalex, float const scaley, float const scalez )
{

    ModifyMode = PBVR_ON;
    SetSphereMaxMin( 0, 0, 0, sphere.m_radius * ( scalex - 1 ) );

    if ( ( SphereXmin >= Xmin * 2 ) && ( ( SphereXmax <= Xmax * 2 ) || ( scalex < 1.0 ) )
            && ( SphereYmin >= Ymin * 2 ) && ( ( SphereYmax <= Ymax * 2 ) || ( scaley < 1.0 ) )
            && ( SphereZmin >= Zmin * 2 ) && ( ( SphereZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) && ( sphere.m_radius * scalex > PBVR_SCALE_MIN ) )
    {

        sphere.m_radius *= scalex;
        setSphereTextData();
        param3vd.SphereParaSet();
        m_save_radius = m_radius;
    }

    ModifyMode = PBVR_OFF;
}

//円柱・指定倍率での拡大/縮小処理
void	Pillar::scalingPillar( const float scalex, const float scaley, const float scalez )
{

    ModifyMode = PBVR_ON;
    if ( scalex && scaley && scalez )
    {
        switch ( obj_3d )
        {

        case	PBVR_PILLARXY:

            SetPillarXYMaxMin( 0, 0, 0, pillar.m_radius * ( scalex - 1 ), pillar.m_height * ( scalez - 1 ), 0 );

            if ( ( PillarXYXmin >= Xmin * 2 ) && ( ( PillarXYXmax <= Xmax * 2 ) || ( scalex < 1.0 ) )
                    && ( PillarXYYmin >= Ymin * 2 ) && ( ( PillarXYYmax <= Ymax * 2 ) || ( scaley < 1.0 ) )
                    && ( PillarXYZmin >= Zmin * 2 ) && ( ( PillarXYZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) )
            {
                if ( ( pillar.m_radius * scalex > PBVR_SCALE_MIN ) && ( pillar.m_height * scalez > PBVR_SCALE_MIN ) )
                {

                    pillar.m_radius *= scaley;
                    pillar.m_height *= scalez;
                    setPillarTextData();
                    param3vd.PillarParaSet();
                    m_save_radius = m_radius;
                    m_save_height = m_height;
                }
            }
            break;

        case	PBVR_PILLARYZ:
            SetPillarYZMaxMin( 0, 0, 0, pillarYZ.m_radius * ( scaley - 1 ), pillarYZ.m_height * ( scalex - 1 ), 0 );

            if ( ( PillarYZXmin >= Xmin * 2 ) && ( ( PillarYZXmax <= Xmax * 2 ) || ( scalex < 1.0 ) )
                    && ( PillarYZYmin >= Ymin * 2 ) && ( ( PillarYZYmax <= Ymax * 2 ) || ( scaley < 1.0 ) )
                    && ( PillarYZZmin >= Zmin * 2 ) && ( ( PillarYZZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) )
            {
                if ( ( pillarYZ.m_radius * scaley > PBVR_SCALE_MIN ) && ( pillarYZ.m_height * scalex > PBVR_SCALE_MIN ) )
                {

                    pillarYZ.m_radius *= scaley;
                    pillarYZ.m_height *= scalex;
                    setPillarTextData();
                    param3vd.PillarParaSet();
                    m_save_radius = m_radius;
                    m_save_height = m_height;
                }
            }
            break;

        case	PBVR_PILLARXZ:

            SetPillarXZMaxMin( 0, 0, 0, pillarXZ.m_radius * ( scalex - 1 ), pillarXZ.m_height * ( scaley - 1 ), 0 );

            if ( ( PillarXZXmin >= Xmin * 2 ) && ( ( PillarXZXmax <= Xmax * 2 ) || ( scalex < 1.0 ) )
                    && ( PillarXZYmin >= Ymin * 2 ) && ( ( PillarXZYmax <= Ymax * 2 ) || ( scaley < 1.0 ) )
                    && ( PillarXZZmin >= Zmin * 2 ) && ( ( PillarXZZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) )
            {
                if ( ( pillarXZ.m_radius * scalex > PBVR_SCALE_MIN ) && ( pillarXZ.m_height * scaley > PBVR_SCALE_MIN ) )
                {

                    pillarXZ.m_radius *= scaley;
                    pillarXZ.m_height *= scalez;
                    setPillarTextData();
                    param3vd.PillarParaSet();
                    m_save_radius = m_radius;
                    m_save_height = m_height;
                }
            }
            break;

        default:
            break;
        }

        ModifyMode = PBVR_OFF;
        return;
    }

    switch ( obj_3d )
    {

    case	PBVR_PILLARXY:

        if ( scalex )
        {
            SetPillarXYMaxMin( 0, 0, 0, pillar.m_radius * ( scalex - 1 ), 0, 0 );

            if ( ( PillarXYXmin >= Xmin ) && ( ( PillarXYXmax <= Xmax * 2 ) || ( scalex < 1.0 ) ) && ( pillar.m_radius * scalex >= PBVR_SCALE_MIN ) )
            {
                pillar.m_radius *= scalex;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_radius = m_radius;
            }
        }

        if ( scaley )
        {
            SetPillarXYMaxMin( 0, 0, 0, pillar.m_radius * ( scaley - 1 ), 0, 0 );

            if ( ( PillarXYYmin >= Ymin ) && ( ( PillarXYYmax <= Ymax * 2 ) || ( scaley < 1.0 ) ) && ( pillar.m_radius * scaley >= PBVR_SCALE_MIN ) )
            {
                pillar.m_radius *= scaley;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_radius = m_radius;
            }
        }

        if ( scalez )
        {
            SetPillarXYMaxMin( 0, 0, 0, 0, pillar.m_height * ( scalez - 1 ), 0 );
            if ( ( PillarXYZmin >= Zmin ) && ( ( PillarXYZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) && ( pillar.m_height * scalez > PBVR_SCALE_MIN ) )
            {
                pillar.m_height *= scalez;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_height = m_height;
            }
        }
        break;

    case	PBVR_PILLARYZ:

        if ( scalex )
        {
            SetPillarYZMaxMin( 0, 0, 0, pillarYZ.m_height * ( scalex - 1 ), 0, 0 );

            if ( ( PillarYZXmin >= Xmin * 2 ) && ( ( PillarYZXmax <= Xmax * 2 ) || ( scalex < 1.0 ) ) && ( pillarYZ.m_height * scalex >= PBVR_SCALE_MIN ) )
            {
                pillarYZ.m_height *= scalex;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_height = m_height;
            }
        }

        if ( scaley )
        {
            SetPillarYZMaxMin( 0, 0, 0, pillarYZ.m_radius * ( scaley - 1 ), 0, 0 );

            if ( ( PillarYZYmin >= Ymin * 2 ) && ( ( PillarYZYmax <= Ymax * 2 ) || ( scaley < 1.0 ) ) && ( pillarYZ.m_radius * scaley >= PBVR_SCALE_MIN ) )
            {
                pillarYZ.m_radius *= scaley;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_radius = m_radius;
            }
        }

        if ( scalez )
        {
            SetPillarYZMaxMin( 0, 0, 0, 0, pillarYZ.m_radius * ( scalez - 1 ), 0 );
            if ( ( PillarYZZmin >= Zmin * 2 ) && ( ( PillarYZZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) && ( pillarYZ.m_radius * scalez > PBVR_SCALE_MIN ) )
            {
                pillar.m_radius *= scalez;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_radius = m_radius;
            }
        }
        break;

    case	PBVR_PILLARXZ:

        if ( scalex )
        {
            SetPillarXZMaxMin( 0, 0, 0, pillarXZ.m_radius * ( scalex - 1 ), 0, 0 );

            if ( ( PillarXZXmin >= Xmin * 2 ) && ( ( PillarXZXmax <= Xmax * 2 ) || ( scalex < 1.0 ) ) && ( pillarXZ.m_radius * scalex >= PBVR_SCALE_MIN ) )
            {
                pillarXZ.m_radius *= scalex;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_radius = m_radius;
            }
        }

        if ( scaley )
        {
            SetPillarXZMaxMin( 0, 0, 0, pillarXZ.m_height * ( scaley - 1 ), 0, 0 );

            if ( ( PillarXZYmin >= Ymin * 2 ) && ( ( PillarXZYmax <= Ymax * 2 ) || ( scaley < 1.0 ) ) && ( pillarXZ.m_height * scaley >= PBVR_SCALE_MIN ) )
            {
                pillarXZ.m_height *= scaley;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_height = m_height;
            }
        }

        if ( scalez )
        {
            SetPillarXZMaxMin( 0, 0, 0, 0, pillarXZ.m_radius * ( scalez - 1 ), 0 );
            if ( ( PillarXZZmin >= Zmin * 2 ) && ( ( PillarXZZmax <= Zmax * 2 ) || ( scalez < 1.0 ) ) && ( pillarXZ.m_radius * scalez > PBVR_SCALE_MIN ) )
            {
                pillarXZ.m_radius *= scalez;
                setPillarTextData();
                param3vd.PillarParaSet();
                m_save_radius = m_radius;
            }
        }
        break;

    default:
        break;
    }

    ModifyMode = PBVR_OFF;
}

//マウスイベント コールバック
void	MouseFunc( const int button, const int state, const int x, const int y )
{
    //特殊キー押下状態
    modifier = glutGetModifiers();

    switch ( button )
    {

    case ( GLUT_LEFT_BUTTON ):

        //マウス左ボタンDOWN
        if ( state == GLUT_DOWN )
        {

            //コントロールキーが押されていたら、変形モード
            if ( modifier == GLUT_ACTIVE_CTRL )
            {
                ModifyMode |= PBVR_MODIFIMODE;
            }

            //シフトキーが押されていたら、拡大モード
            if ( modifier == GLUT_ACTIVE_SHIFT )
            {
                switch ( obj_3d )
                {
                case	PBVR_CROP:
                    crop.scalingCrop( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
                    break;
                case	PBVR_SPHERE:
                    sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
                    break;
                case	PBVR_PILLARXY:
                    pillar.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
                    break;
                case	PBVR_PILLARYZ:
                    pillarYZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
                    break;
                case	PBVR_PILLARXZ:
                    pillarXZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
                    break;
                default:
                    break;
                }
            }

            //移動モード or 変形モード
            else
            {
                if ( ( 0 <= x ) && ( x < PBVR_VIEWPORT_WIDTH )  && ( 0 <= y ) && ( y < PBVR_VIEWPORT_HEIGHT ) )
                {
                    win_sel = PBVR_X_VIEW;
                    clicking = PBVR_ON;
                    switch ( obj_3d )
                    {
                    case	PBVR_CROP:
                        MouseCropX(  button, state, x, y );
                        break;
                    case	PBVR_SPHERE:
                        MouseSphereX(  button, state, x, y );
                        break;
                    case	PBVR_PILLARXY:
                        MousePillarXY_X(  button, state, x, y );
                        break;
                    case	PBVR_PILLARYZ:
                        MousePillarYZ_X(  button, state, x, y );
                        break;
                    case	PBVR_PILLARXZ:
                        MousePillarXZ_X(  button, state, x, y );
                        break;
                    default:
                        break;
                    }
                }
                else if ( ( PBVR_VIEWPORT_WIDTH <= x ) && ( x <= PBVR_VIEWPORT_WIDTH * 2 ) && ( y > PBVR_VIEWPORT_HEIGHT  ) && ( PBVR_VIEWPORT_HEIGHT * 2 >= y ) )
                {
                    win_sel = PBVR_Y_VIEW;
                    clicking = PBVR_ON;
                    switch ( obj_3d )
                    {
                    case	PBVR_CROP:
                        MouseCropY(  button, state, x, y );
                        break;
                    case	PBVR_SPHERE:
                        MouseSphereY(  button, state, x, y );
                        break;
                    case	PBVR_PILLARXY:
                        MousePillarXY_Y(  button, state, x, y );
                        break;
                    case	PBVR_PILLARYZ:
                        MousePillarYZ_Y(  button, state, x, y );
                        break;
                    case	PBVR_PILLARXZ:
                        MousePillarXZ_Y(  button, state, x, y );
                        break;
                    default:
                        break;
                    }
                }
                else if ( ( PBVR_VIEWPORT_WIDTH <= x ) && ( x <= PBVR_VIEWPORT_WIDTH * 2 )  && ( 0 <= y ) && ( y < PBVR_VIEWPORT_HEIGHT ) )
                {
                    win_sel = PBVR_Z_VIEW;
                    clicking = PBVR_ON;
                    switch ( obj_3d )
                    {
                    case	PBVR_CROP:
                        MouseCropZ(  button, state, x, y );
                        break;
                    case	PBVR_SPHERE:
                        MouseSphereZ(  button, state, x, y );
                        break;
                    case	PBVR_PILLARXY:
                        MousePillarXY_Z(  button, state, x, y );
                        break;
                    case	PBVR_PILLARYZ:
                        MousePillarYZ_Z(  button, state, x, y );
                        break;
                    case	PBVR_PILLARXZ:
                        MousePillarXZ_Z(  button, state, x, y );
                        break;
                    default:
                        break;
                    }
                }
            }
#ifdef __APPLE__
            Display3vd();
#endif
        }

        //マウス左ボタンUP
        else if ( state == GLUT_UP )
        {

            if ( win_sel == PBVR_X_VIEW )
            {
                switch ( obj_3d )
                {
                case	PBVR_CROP:
                    MouseCropX(  button, state, x, y );
                    break;
                case	PBVR_SPHERE:
                    MouseSphereX(  button, state, x, y );
                    break;
                case	PBVR_PILLARXY:
                    MousePillarXY_X(  button, state, x, y );
                    break;
                case	PBVR_PILLARYZ:
                    MousePillarYZ_X(  button, state, x, y );
                    break;
                case	PBVR_PILLARXZ:
                    MousePillarXZ_X(  button, state, x, y );
                    break;
                default:
                    break;
                }
            }
            else if ( win_sel == PBVR_Y_VIEW )
            {
                switch ( obj_3d )
                {
                case	PBVR_CROP:
                    MouseCropY(  button, state, x, y );
                    break;
                case	PBVR_SPHERE:
                    MouseSphereY(  button, state, x, y );
                    break;
                case	PBVR_PILLARXY:
                    MousePillarXY_Y(  button, state, x, y );
                    break;
                case	PBVR_PILLARYZ:
                    MousePillarYZ_Y(  button, state, x, y );
                    break;
                case	PBVR_PILLARXZ:
                    MousePillarXZ_Y(  button, state, x, y );
                    break;
                default:
                    break;
                }
            }
            else if ( win_sel == PBVR_Z_VIEW )
            {
                switch ( obj_3d )
                {
                case	PBVR_CROP:
                    MouseCropZ(  button, state, x, y );
                    break;
                case	PBVR_SPHERE:
                    MouseSphereZ(  button, state, x, y );
                    break;
                case	PBVR_PILLARXY:
                    MousePillarXY_Z(  button, state, x, y );
                    break;
                case	PBVR_PILLARYZ:
                    MousePillarYZ_Z(  button, state, x, y );
                    break;
                case	PBVR_PILLARXZ:
                    MousePillarXZ_Z(  button, state, x, y );
                    break;
                default:
                    break;
                }
            }
            clicking = PBVR_OFF;
        }

        break;

    case ( GLUT_RIGHT_BUTTON ):

        //マウス右ボタンDOWN
        if ( state == GLUT_DOWN )
        {

            //シフトキーが押されていたら、縮小モード
            if ( modifier == GLUT_ACTIVE_SHIFT )
            {
                switch ( obj_3d )
                {
                case	PBVR_CROP:
                    crop.scalingCrop( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
                    break;
                case	PBVR_SPHERE:
                    sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
                    break;
                case	PBVR_PILLARXY:
                    pillar.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
                    break;
                case	PBVR_PILLARYZ:
                    pillarYZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
                    break;
                case	PBVR_PILLARXZ:
                    pillarXZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
                    break;
                default:
                    break;
                }
            }
        }
    default:
        break;
    }
}

//マウスボタン押下処理（直方体・X軸視点）
void	MouseCropX( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {
        crop.setRetentionPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );

        //＋コントロールキー＝直方体の変形
        if ( ModifyMode )
        {

            // 直方体の上半分を掴むと、上辺を変形
            if ( ( ( ( crop.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) ) >=  ( save_cymin + ( save_cymax - save_cymin ) / 2 ) )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 直方体の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 直方体の右半分を掴むと、右辺を変形
            if ( ( ( ( crop.m_previous_z - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) )  ) >=  ( save_czmin + ( save_czmax - save_czmin ) / 2 ) )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 直方体の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    //左ボタンUP  移動量と変形された形を保持
    else if ( state == GLUT_UP )
    {
        crop.savePosition( 0, crop.m_move_y, crop.m_move_z );
        save_cxmax = cxmaxlevel2;
        save_cxmin = cxminlevel2;
        save_cymax = cymaxlevel2;
        save_cymin = cyminlevel2;
        save_czmax = czmaxlevel2;
        save_czmin = czminlevel2;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（球・X軸視点）
void	MouseSphereX( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN ) 		//マウスが押された状態
    {
        sphere.setRetentionPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );

        //＋コントロールキー＝球の変形

        if ( ModifyMode )
        {
            // 球の上半分を掴むと、上辺を変形
            if ( ( ( sphere.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  sphere.m_save_center_y )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }
            // 球の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 球の右半分を掴むと、右辺を変形
            if ( ( ( sphere.m_previous_z - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  sphere.m_save_center_z )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 球の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }
    else if ( state == GLUT_UP )
    {
        sphere.savePosition( 0, sphere.m_move_y, sphere.m_move_z );

        sphere.m_save_center_x = sphere.m_center_x;
        sphere.m_save_center_y = sphere.m_center_y;
        sphere.m_save_center_z = sphere.m_center_z;
        sphere.m_save_radius = sphere.m_radius;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・X軸視点）
void	MousePillarXY_X( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN ) 		//マウスが押された状態
    {
        pillar.setRetentionPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( pillar.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  pillar.m_save_center_y )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( pillar.m_previous_z - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  pillar.m_save_center_z + pillar.m_save_height / 2 )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {
        pillar.savePosition( 0, pillar.m_move_y, pillar.m_move_z );
        pillar.m_save_center_x = pillar.m_center_x;
        pillar.m_save_center_y = pillar.m_center_y;
        pillar.m_save_center_z = pillar.m_center_z;
        pillar.m_save_radius = pillar.m_radius;
        pillar.m_save_height = pillar.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

void	MousePillarXZ_X( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN ) 		//マウスが押された状態
    {
        pillarXZ.setRetentionPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( pillarXZ.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  pillarXZ.m_save_center_y + pillarXZ.m_save_height / 2 )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( pillarXZ.m_previous_z - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  pillarXZ.m_save_center_z )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {
        pillarXZ.savePosition( 0, pillarXZ.m_move_y, pillarXZ.m_move_z );
        pillarXZ.m_save_center_x = pillarXZ.m_center_x;
        pillarXZ.m_save_center_y = pillarXZ.m_center_y;
        pillarXZ.m_save_center_z = pillarXZ.m_center_z;
        pillarXZ.m_save_radius = pillarXZ.m_radius;
        pillarXZ.m_save_height = pillarXZ.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

void	MousePillarYZ_X( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN ) 		//マウスが押された状態
    {
        pillarYZ.setRetentionPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( pillarYZ.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  pillarYZ.m_save_center_y )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( pillarYZ.m_previous_z - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  pillarYZ.m_save_center_z )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillarYZ.savePosition( 0, pillarYZ.m_move_y, pillarYZ.m_move_z );
        pillarYZ.m_save_center_x = pillarYZ.m_center_x;
        pillarYZ.m_save_center_y = pillarYZ.m_center_y;
        pillarYZ.m_save_center_z = pillarYZ.m_center_z;
        pillarYZ.m_save_radius = pillarYZ.m_radius;
        pillarYZ.m_save_height = pillarYZ.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（直方体・Y軸視点）
void	MouseCropY( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        crop.setRetentionPosition( x, 0, y );

        //＋コントロールキー＝直方体の変形
        if ( ModifyMode )
        {

            // 直方体の上半分を掴むと、上辺を変形
            if ( ( ( ( crop.m_previous_z - PBVR_VIEWPORT_HEIGHT / 2 - PBVR_VIEWPORT_HEIGHT ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) ) >=  ( save_czmin + ( save_czmax - save_czmin ) / 2 ) )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 直方体の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 直方体の右半分を掴むと、右辺を変形
            if ( ( ( ( crop.m_previous_x - PBVR_VIEWPORT_WIDTH / 2 - PBVR_VIEWPORT_WIDTH ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) )  ) >=  ( save_cxmin + ( save_cxmax - save_cxmin ) / 2 ) )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 直方体の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    //左ボタンUP  移動量と変形された形を保持
    else if ( state == GLUT_UP )
    {
        crop.savePosition( crop.m_move_x, 0, crop.m_move_z );
        save_cxmax = cxmaxlevel2;
        save_cxmin = cxminlevel2;
        save_cymax = cymaxlevel2;
        save_cymin = cyminlevel2;
        save_czmax = czmaxlevel2;
        save_czmin = czminlevel2;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（球・Y軸視点）
void	MouseSphereY( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        sphere.setRetentionPosition( x, 0, y );

        //＋コントロールキー＝球の変形

        if ( ModifyMode )
        {
            // 球の上半分を掴むと、上辺を変形
            if ( -( ( sphere.m_previous_z - PBVR_VIEWPORT_HEIGHT - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  -sphere.m_save_center_z )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }
            // 球の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 球の右半分を掴むと、右辺を変形
            if ( ( ( sphere.m_previous_x - PBVR_VIEWPORT_WIDTH - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  sphere.m_save_center_x )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 球の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {
        sphere.savePosition( sphere.m_move_x, 0, sphere.m_move_z );
        sphere.m_save_center_x = sphere.m_center_x;
        sphere.m_save_center_y = sphere.m_center_y;
        sphere.m_save_center_z = sphere.m_center_z;
        sphere.m_save_radius = sphere.m_radius;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・Y軸視点）
void	MousePillarXY_Y( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        pillar.setRetentionPosition( x, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( ( pillar.m_previous_z - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) / pillar.m_scale_z ) >=  pillar.m_save_center_z + pillar.m_save_height / 2 )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( ( pillar.m_previous_x - PBVR_VIEWPORT_HEIGHT - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) / pillar.m_scale_x ) >=  pillar.m_save_center_x )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillar.savePosition( pillar.m_move_x, 0, pillar.m_move_z );
        pillar.m_save_center_x = pillar.m_center_x;
        pillar.m_save_center_y = pillar.m_center_y;
        pillar.m_save_center_z = pillar.m_center_z;
        pillar.m_save_radius = pillar.m_radius;
        pillar.m_save_height = pillar.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・Y軸視点）
void	MousePillarXZ_Y( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        pillarXZ.setRetentionPosition( x, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( ( pillarXZ.m_previous_z - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) / pillarXZ.m_scale_z ) >=  pillarXZ.m_save_center_z )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( ( pillarXZ.m_previous_x - PBVR_VIEWPORT_HEIGHT - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) / pillarXZ.m_scale_x ) >=  pillarXZ.m_save_center_x )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillarXZ.savePosition( pillarXZ.m_move_x, 0, pillarXZ.m_move_z );
        pillarXZ.m_save_center_x = pillarXZ.m_center_x;
        pillarXZ.m_save_center_y = pillarXZ.m_center_y;
        pillarXZ.m_save_center_z = pillarXZ.m_center_z;
        pillarXZ.m_save_radius = pillarXZ.m_radius;
        pillarXZ.m_save_height = pillarXZ.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・Y軸視点）
void	MousePillarYZ_Y( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        pillarYZ.setRetentionPosition( x, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( ( pillarYZ.m_previous_z - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) / pillarYZ.m_scale_z ) >=  pillarYZ.m_save_center_z )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( ( pillarYZ.m_previous_x - PBVR_VIEWPORT_HEIGHT - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) / pillarYZ.m_scale_x ) >=  pillarYZ.m_save_center_x + pillarYZ.m_save_height / 2 )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillarYZ.savePosition( pillarYZ.m_move_x, 0, pillarYZ.m_move_z );
        pillarYZ.m_save_center_x = pillarYZ.m_center_x;
        pillarYZ.m_save_center_y = pillarYZ.m_center_y;
        pillarYZ.m_save_center_z = pillarYZ.m_center_z;
        pillarYZ.m_save_radius = pillarYZ.m_radius;
        pillarYZ.m_save_height = pillarYZ.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（直方体・Z軸視点）
void	MouseCropZ( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        crop.setRetentionPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );

        //＋コントロールキー＝直方体の変形
        if ( ModifyMode )
        {

            // 直方体の上半分を掴むと、上辺を変形
            if ( ( ( ( crop.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) ) >=  ( save_cymin + ( save_cymax - save_cymin ) / 2 ) )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 直方体の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 直方体の右半分を掴むと、右辺を変形
            if ( ( ( ( crop.m_previous_x - PBVR_VIEWPORT_WIDTH / 2 - PBVR_VIEWPORT_WIDTH ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) )  ) >=  ( save_cxmin + ( save_cxmax - save_cxmin ) / 2 ) )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 直方体の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    //左ボタンUP  移動量と変形された形を保持
    else if ( state == GLUT_UP )
    {

        crop.savePosition( crop.m_move_x, crop.m_move_y, 0 );
        save_cxmax = cxmaxlevel2;
        save_cxmin = cxminlevel2;
        save_cymax = cymaxlevel2;
        save_cymin = cyminlevel2;
        save_czmax = czmaxlevel2;
        save_czmin = czminlevel2;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（球・Z軸視点）
void	MouseSphereZ( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        sphere.setRetentionPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );

        //＋コントロールキー＝球の変形

        if ( ModifyMode )
        {
            // 球の上半分を掴むと、上辺を変形
            if ( ( ( sphere.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  sphere.m_save_center_y )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }
            // 球の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 球の右半分を掴むと、右辺を変形
            if ( ( ( sphere.m_previous_x - PBVR_VIEWPORT_WIDTH - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  sphere.m_save_center_x )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 球の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        sphere.savePosition( sphere.m_move_x, sphere.m_move_y, 0 );
        sphere.m_save_center_x = sphere.m_center_x;
        sphere.m_save_center_y = sphere.m_center_y;
        sphere.m_save_center_z = sphere.m_center_z;
        sphere.m_save_radius = sphere.m_radius;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・Z軸視点）
void	MousePillarXY_Z( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        pillar.setRetentionPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( pillar.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  pillar.m_save_center_y )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( pillar.m_previous_x - PBVR_VIEWPORT_WIDTH - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  pillar.m_save_center_x )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillar.savePosition( pillar.m_move_x, pillar.m_move_y, 0 );
        pillar.m_save_center_x = pillar.m_center_x;
        pillar.m_save_center_y = pillar.m_center_y;
        pillar.m_save_center_z = pillar.m_center_z;
        pillar.m_save_radius = pillar.m_radius;
        pillar.m_save_height = pillar.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・Z軸視点）
void	MousePillarXZ_Z( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        pillarXZ.setRetentionPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( pillarXZ.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  pillarXZ.m_save_center_y + pillarXZ.m_save_height / 2 )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }
            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( pillarXZ.m_previous_x - PBVR_VIEWPORT_WIDTH - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  pillarXZ.m_save_center_x )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillarXZ.savePosition( pillarXZ.m_move_x, pillarXZ.m_move_y, 0 );
        pillarXZ.m_save_center_x = pillarXZ.m_center_x;
        pillarXZ.m_save_center_y = pillarXZ.m_center_y;
        pillarXZ.m_save_center_z = pillarXZ.m_center_z;
        pillarXZ.m_save_radius = pillarXZ.m_radius;
        pillarXZ.m_save_height = pillarXZ.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウスボタン押下処理（円柱・Z軸視点）
void	MousePillarYZ_Z( const int button, const int state, const int x, const int y )
{

    //マウス左ボタンDOWN
    if ( state == GLUT_DOWN )
    {

        pillarYZ.setRetentionPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );

        //＋コントロールキー＝円柱の変形
        if ( ModifyMode )
        {

            // 円柱の上半分を掴むと、上辺を変形
            if ( ( ( pillarYZ.m_previous_y - PBVR_VIEWPORT_HEIGHT / 2 ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) ) >=  pillarYZ.m_save_center_y )
            {
                ModifyMode |= PBVR_TOP_SIDE;
            }

            // 円柱の下半分を掴むと、下辺を変形
            else
            {
                ModifyMode |= PBVR_BOTTOM_SIDE;
            }

            // 円柱の右半分を掴むと、右辺を変形
            if ( ( ( pillarYZ.m_previous_x - PBVR_VIEWPORT_WIDTH - PBVR_VIEWPORT_WIDTH / 2 ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) ) >=  pillarYZ.m_save_center_x + pillarYZ.m_save_height / 2 )
            {
                ModifyMode |= PBVR_RIGHTSIDE;
            }

            // 円柱の左半分を掴むと、左辺を変形
            else
            {
                ModifyMode |= PBVR_LEFTSIDE;
            }
        }
    }

    else if ( state == GLUT_UP )
    {

        pillarYZ.savePosition( pillarYZ.m_move_x, pillarYZ.m_move_y, 0 );
        pillarYZ.m_save_center_x = pillarYZ.m_center_x;
        pillarYZ.m_save_center_y = pillarYZ.m_center_y;
        pillarYZ.m_save_center_z = pillarYZ.m_center_z;
        pillarYZ.m_save_radius = pillarYZ.m_radius;
        pillarYZ.m_save_height = pillarYZ.m_height;

        if ( ModifyMode )
            ModifyMode = PBVR_OFF;
    }
}

//マウス・モーションイベント  コールバック
void	MouseMotion( const int x, const int y )
{
    //マウスボタンが押されていなければ、リターン
    if ( clicking != PBVR_ON )
        return;

    //X軸視点ビューポートの選択中
    if ( ( 0 <= x ) && ( x < PBVR_VIEWPORT_WIDTH )  && ( 0 <= y ) && ( y < PBVR_VIEWPORT_HEIGHT ) )
    {

        switch ( obj_3d )
        {
        case	PBVR_CROP:
            MotionCropX( x, y );
            break;
        case	PBVR_SPHERE:
            MotionSphereX( x, y );
            break;
        case	PBVR_PILLARXY:
            MotionPillarXY_X( x, y );
            break;
        case	PBVR_PILLARYZ:
            MotionPillarYZ_X( x, y );
            break;
        case	PBVR_PILLARXZ:
            MotionPillarXZ_X( x, y );
            break;
        default:
            break;
        }
    }
    //Y軸視点ビューポートの選択中
    else if ( ( PBVR_VIEWPORT_WIDTH <= x ) && ( x <= PBVR_VIEWPORT_WIDTH * 2 )  && ( y > PBVR_VIEWPORT_HEIGHT  ) && ( PBVR_VIEWPORT_HEIGHT * 2 >= y ) )
    {

        switch ( obj_3d )
        {
        case	PBVR_CROP:
            MotionCropY( x, y );
            break;
        case	PBVR_SPHERE:
            MotionSphereY( x, y );
            break;
        case	PBVR_PILLARXY:
            MotionPillarXY_Y( x, y );
            break;
        case	PBVR_PILLARYZ:
            MotionPillarYZ_Y( x, y );
            break;
        case	PBVR_PILLARXZ:
            MotionPillarXZ_Y( x, y );
            break;
        default:
            break;
        }
    }
    //Z軸視点ビューポートの選択中
    else if ( ( PBVR_VIEWPORT_WIDTH <= x ) && ( x <= PBVR_VIEWPORT_WIDTH * 2 )  && ( 0 <= y ) && ( y < PBVR_VIEWPORT_HEIGHT ) )
    {
        switch ( obj_3d )
        {
        case	PBVR_CROP:
            MotionCropZ( x, y );
            break;
        case	PBVR_SPHERE:
            MotionSphereZ( x, y );
            break;
        case	PBVR_PILLARXY:
            MotionPillarXY_Z( x, y );
            break;
        case	PBVR_PILLARYZ:
            MotionPillarYZ_Z( x, y );
            break;
        case	PBVR_PILLARXZ:
            MotionPillarXZ_Z( x, y );
            break;
        default:
            break;
        }
    }
}

//マウスモーション処理（直方体・X軸視点）
void	MotionCropX( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量

    //Xビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_X_VIEW )
        return;

    wkx = ( ( x  - crop.m_previous_z ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );											//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - crop.m_previous_y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );							//y軸方向のマウスの移動量
    CropTrans = PBVR_OFF;

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        CropTrans = ( PBVR_TOP_SIDE | PBVR_BOTTOM_SIDE | PBVR_SURFSIDE | PBVR_DEPTH_SIDE );
        SetCropMaxMin( 0, wky, wkx );

        if ( ( CropYmax <= Ymax * 2 ) && ( CropYmin >= Ymin * 2 )
                && ( CropZmax <= Zmax * 2 ) && ( CropZmin >= Zmin * 2 ) )
        {

            crop.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );
            crop.setCropTextData();
            param3vd.CropParaSet();
        }
        return;
    }

    //＋コントロールキー ＝ 直方体の変形


    // 直方体の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        CropTrans = PBVR_TOP_SIDE;
        SetCropMaxMin( 0.0F, wky, 0.0F );
        if ( ( CropYmax <= Ymax * 2 ) && ( CropYmax > cyminlevel2 ) )
            cymaxlevel2 = CropYmax;
    }
    // 直方体の下半分を掴むと、下辺を変形
    if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        CropTrans = PBVR_BOTTOM_SIDE;
        SetCropMaxMin( 0.0F, wky, 0.0F );
        if ( ( CropYmin >= Ymin * 2 ) && ( CropYmin < cymaxlevel2 ) )
            cyminlevel2 = CropYmin;
    }

    // 直方体の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        CropTrans = PBVR_SURFSIDE;
        SetCropMaxMin( 0.0F, 0.0F, wkx );
        if ( ( CropZmax <= Zmax * 2 ) && ( CropZmax > czminlevel2 ) )
            czmaxlevel2 = CropZmax;
    }

    // 直方体の左半分を掴むと、左辺を変形
    if ( ModifyMode & PBVR_LEFTSIDE )
    {
        CropTrans = PBVR_DEPTH_SIDE;
        SetCropMaxMin( 0.0F, 0.0F, wkx );
        if ( ( CropZmin >= Zmin * 2 ) && ( CropZmin < czmaxlevel2 ) )
            czminlevel2 = CropZmin;
    }

    //値を計算し、テキストボックスにセット
    crop.m_side_y = cymaxlevel2 - cyminlevel2;
    crop.m_move_y = ( cymaxlevel2 + cyminlevel2 ) * ( PBVR_VIEWPORT_HEIGHT / 2 ) / 2;

    crop.m_side_z = czmaxlevel2 - czminlevel2;
    crop.m_move_z = ( czmaxlevel2 + czminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;

    crop.setCropTextBox();
    param3vd.CropParaSet();
}

//マウスモーション処理（球・X軸視点）
void	MotionSphereX( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod;					//半径の変化量

    //Xビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_X_VIEW )
        return;

    wkx = ( x - sphere.m_previous_z ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );												//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - sphere.m_previous_y  ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );						//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetSphereMaxMin( 0, wky, wkx, 0 );

        if ( ( SphereYmax <= Ymax * 2 ) && ( SphereYmin >= Ymin * 2 )
                && ( SphereZmax <= Zmax * 2 ) && ( SphereZmin >= Zmin * 2 ) )
        {

            sphere.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );
            sphere.setSphereTextData();
            param3vd.SphereParaSet();
        }
    }


    // 球の右上を掴む
    if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / sphere.m_scale_z )  + ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereYmax <= Ymax * 2 ) && ( SphereZmax <= Zmax * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の左上を掴む
    else if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / sphere.m_scale_z )  + ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereYmax <= Ymax * 2 ) && ( SphereZmin >= Zmin * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の右下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / sphere.m_scale_z )  - ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereYmin >= Ymin * 2 ) && ( SphereZmax <= Zmax * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の左下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / sphere.m_scale_z ) - ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmin >= Xmin * 2 ) && ( SphereYmin >= Ymin * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    //値を計算し、テキストボックスにセット
    sphere.setSphereTextData();
    param3vd.SphereParaSet();
}

//マウスモーション処理（円柱・X軸視点）
void	MotionPillarXY_X( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod, heightmod;		//半径、高さの変化量

    //Xビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_X_VIEW )
        return;

    wkx = ( x - pillar.m_previous_z ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );									//x軸方向のマウスの移動量
    wky = ( ( float )PBVR_VIEWPORT_HEIGHT - pillar.m_previous_y - y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );			//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarXYMaxMin( 0, wky, wkx, 0, 0, 0 );

        if ( ( PillarXYYmax <= Ymax * 2 ) && ( PillarXYYmin >= Ymin * 2 )
                && ( PillarXYZmax <= Zmax * 2 ) && ( PillarXYZmin >= Zmin * 2 ) )
        {

            pillar.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );
            pillar.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    radiusmod = wky / pillar.m_scale_y;														//半径の変化量
    heightmod = wkx / pillar.m_scale_z;														//高さの変化量

    // 円柱の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        SetPillarXYMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXYYmax <= Ymax * 2 ) && ( PillarXYYmax > PillarXYYmin ) )
        {
            pillar.m_radius = pillar.m_save_radius + radiusmod;
        }
    }
    // 円柱の下半分を掴むと、下辺を変形
    else if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        SetPillarXYMaxMin( 0, 0, 0, -radiusmod, 0, 0 );
        if ( ( PillarXYYmin >= Ymin * 2 ) && ( PillarXYYmin < PillarXYYmax ) )
        {
            pillar.m_radius = pillar.m_save_radius - radiusmod;
        }
    }

    // 円柱の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        SetPillarXYMaxMin( 0, 0, 0, 0, heightmod, 0 );
        if ( ( PillarXYZmax <= Zmax * 2 ) && ( PillarXYZmax > PillarXYZmin ) )
        {
            pillar.m_height = pillar.m_save_height + heightmod;
        }
    }

    // 円柱の左半分を掴むと、左辺を変形
    else if ( ModifyMode & PBVR_LEFTSIDE )
    {
        SetPillarXYMaxMin( 0, 0, 0, 0, 0, heightmod );
        if ( ( PillarXYZmin >= Zmin * 2 ) && ( PillarXYZmin < PillarXYZmax ) )
        {
            pillar.m_height = pillar.m_save_height - heightmod;
            pillar.m_center_z = pillar.m_save_center_z + heightmod;
            pillar.setPosition( 0, 0, x );
        }
    }

    //値を計算し、テキストボックスにセット
    pillar.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（円柱・X軸視点）
void	MotionPillarXZ_X( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod, heightmod;		//半径、高さの変化量

    //Xビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_X_VIEW )
        return;

    wkx = ( x - pillarXZ.m_previous_z ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );									//x軸方向のマウスの移動量
    wky = ( ( float )PBVR_VIEWPORT_HEIGHT - pillarXZ.m_previous_y - y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );		//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarXZMaxMin( 0, wky, wkx, 0, 0, 0 );

        if ( ( PillarXZYmax <= Ymax * 2 ) && ( PillarXZYmin >= Ymin * 2 )
                && ( PillarXZZmax <= Zmax * 2 ) && ( PillarXZZmin >= Zmin * 2 ) )
        {

            pillarXZ.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );
            pillarXZ.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    radiusmod = wkx / pillarXZ.m_scale_z;														//半径の変化量
    heightmod = wky / pillarXZ.m_scale_y;														//高さの変化量

    // 円柱の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, 0, heightmod, 0 );
        if ( ( PillarXZYmax <= Ymax * 2 ) && ( PillarXZYmax > PillarXZYmin ) )
        {
            pillarXZ.m_height = pillarXZ.m_save_height + heightmod;
        }
    }
    // 円柱の下半分を掴むと、下辺を変形
    else if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, 0, 0, heightmod );
        if ( ( PillarXZYmin >= Ymin * 2 ) && ( PillarXZYmin < PillarXZYmax ) )
        {
            pillarXZ.m_height = pillarXZ.m_save_height - heightmod;
            pillarXZ.m_center_y = pillarXZ.m_save_center_y + heightmod;
            pillarXZ.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, 0 );
        }
    }

    // 円柱の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZZmax <= Zmax * 2 ) && ( PillarXZZmax > PillarXZZmin ) )
        {
            pillarXZ.m_radius = pillarXZ.m_save_radius + radiusmod;
        }
    }

    // 円柱の左半分を掴むと、左辺を変形
    else if ( ModifyMode & PBVR_LEFTSIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, -radiusmod, 0, 0 );
        if ( ( PillarXZZmin >= Zmin * 2 ) && ( PillarXZZmin < PillarXZZmax ) )
        {
            pillarXZ.m_radius = pillarXZ.m_save_radius - radiusmod;
        }
    }

    //値を計算し、テキストボックスにセット
    pillarXZ.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（円柱・X軸視点）
void	MotionPillarYZ_X( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod;					//半径の変化量

    //Xビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_X_VIEW )
        return;

    wkx = ( x - pillarYZ.m_previous_z ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );									//x軸方向のマウスの移動量
    wky = ( ( float )PBVR_VIEWPORT_HEIGHT - pillarYZ.m_previous_y - y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );		//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarYZMaxMin( 0, wky, wkx, 0, 0, 0 );

        if ( ( PillarYZYmax <= Ymax * 2 ) && ( PillarYZYmin >= Ymin * 2 )
                && ( PillarYZZmax <= Zmax * 2 ) && ( PillarYZZmin >= Zmin * 2 ) )
        {

            pillarYZ.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, x );
            pillarYZ.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    // 円柱の右上を掴む
    if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / pillarYZ.m_scale_z )  + ( wky / pillarYZ.m_scale_y );									//半径の変化量
        SetPillarYZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarYZYmax <= Ymax * 2 ) && ( PillarYZZmax <= Zmax * 2 ) )
        {
            if ( ( pillarYZ.m_save_radius * pillarYZ.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarYZ.m_radius = pillarYZ.m_save_radius + radiusmod;
            }
        }
    }
    // 円柱の左上を掴む
    else if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / pillarYZ.m_scale_z )  + ( wky / pillarYZ.m_scale_y );									//半径の変化量
        SetPillarYZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZYmax <= Ymax * 2 ) && ( PillarYZZmin >= Zmin * 2 ) )
        {
            if ( ( pillarYZ.m_save_radius * pillarYZ.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarYZ.m_radius = pillarYZ.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の右下を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / pillarYZ.m_scale_z )  - ( wky / pillarYZ.m_scale_y );									//半径の変化量
        SetPillarYZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarYZZmax <= Zmax * 2 ) && ( PillarYZYmin >= Ymin * 2 ) )
        {
            if ( ( pillarYZ.m_save_radius * pillarYZ.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarYZ.m_radius = pillarYZ.m_save_radius + radiusmod;
            }
        }
    }


    // 円柱の左下を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / pillarYZ.m_scale_z ) - ( wky / pillarYZ.m_scale_y );									//半径の変化量
        SetPillarYZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarYZZmin >= Zmin * 2 ) && ( PillarYZYmin >= Ymin * 2 ) )
        {
            if ( ( pillarYZ.m_save_radius * pillarYZ.m_scale_z + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarYZ.m_radius = pillarYZ.m_save_radius + radiusmod;
            }
        }
    }

    //値を計算し、テキストボックスにセット
    pillarYZ.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（直方体・Y軸視点）
void	MotionCropY( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量

    //マウスの左ボタンが押されていなければ、リターン
    if ( clicking != PBVR_ON )
        return;

    //Yビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Y_VIEW )
        return;

    wkx = ( x - crop.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );										//x軸方向のマウスの移動量
    wky = ( y - crop.m_previous_z ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );										//y軸方向のマウスの移動量
    CropTrans = PBVR_OFF;

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        CropTrans = ( PBVR_RIGHTSIDE | PBVR_LEFTSIDE | PBVR_TOP_SIDE | PBVR_BOTTOM_SIDE );
        SetCropMaxMin( wkx, 0.0F, wky );

        if ( ( CropXmax <= Xmax * 2 ) && ( CropXmin >= Xmin * 2 )
                && ( CropZmax <= Zmax * 2 ) && ( CropZmin >= Zmin * 2 ) )
        {
            crop.setPosition( x, 0, y );
            crop.setCropTextData();
            param3vd.CropParaSet();
        }
        return;
    }

    //＋コントロールキー ＝ 直方体の変形

    // 直方体の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        CropTrans |= PBVR_DEPTH_SIDE;
        SetCropMaxMin( 0.0F,  0.0F, wky );
        if ( ( CropZmax <= Zmax * 2 ) && ( CropZmax > czminlevel2 ) )
            czmaxlevel2 = CropZmax;
    }

    // 直方体の下半分を掴むと、下辺を変形
    if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        CropTrans |= PBVR_SURFSIDE;
        SetCropMaxMin( 0.0F,  0.0F, wky );
        if ( ( CropZmin >= Zmin * 2 ) && ( CropZmin < czmaxlevel2 ) )
            czminlevel2 = CropZmin;
    }

    // 直方体の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        CropTrans |= PBVR_RIGHTSIDE;
        SetCropMaxMin( wkx, 0.0F,  0.0F );
        if ( ( CropXmax <= Xmax * 2 ) && ( CropXmax > cxminlevel2 ) )
            cxmaxlevel2 = CropXmax;
    }

    // 直方体の左半分を掴むと、左辺を変形
    if ( ModifyMode & PBVR_LEFTSIDE )
    {
        CropTrans |= PBVR_LEFTSIDE;
        SetCropMaxMin( wkx, 0.0F,  0.0F );
        if ( ( CropXmin >= Xmin * 2 ) && ( CropXmin < cxmaxlevel2 ) )
            cxminlevel2 = CropXmin;
    }


    //値を計算し、テキストボックスにセット
    crop.m_side_z = czmaxlevel2 - czminlevel2;
    crop.m_move_z = ( czmaxlevel2 + czminlevel2 ) * (  PBVR_VIEWPORT_HEIGHT / 2 ) / 2;

    crop.m_side_x = cxmaxlevel2 - cxminlevel2;
    crop.m_move_x = ( cxmaxlevel2 + cxminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;

    crop.setCropTextBox();
    param3vd.CropParaSet();
}

//マウスモーション処理（球・Y軸視点）
void	MotionSphereY( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod;					//半径の変化量

    //Yビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Y_VIEW )
        return;

    wkx = ( ( x - sphere.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );										//x軸方向のマウスの移動量
    wky = ( ( y - sphere.m_previous_z ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ) );									//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetSphereMaxMin( wkx, 0.0F, wky, 0.0F );

        if ( ( SphereXmax <= Xmax * 2 ) && ( SphereXmin >= Xmin * 2 )
                && ( SphereZmax <= Zmax * 2 ) && ( SphereZmin >= Zmin * 2 ) )
        {

            sphere.setPosition( x, 0, y );
            sphere.setSphereTextData();
            param3vd.SphereParaSet();
            return;
        }
    }

    // 球の右上を掴む
    if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / sphere.m_scale_x ) - ( wky / sphere.m_scale_z );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmax <= Xmax * 2 ) && ( SphereZmin >= Zmin * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の左上を掴む
    else if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / sphere.m_scale_x )  - ( wky / sphere.m_scale_z );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmin >= Xmin * 2 ) && ( SphereZmin >= Zmin * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の右下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / sphere.m_scale_z )  + ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmax <= Xmax * 2 ) && ( SphereZmax <= Zmax * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の左下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / sphere.m_scale_x ) + ( wky / sphere.m_scale_z );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmin >= Xmin * 2 ) && ( SphereZmax <= Zmax * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    //値を計算し、テキストボックスにセット
    sphere.setSphereTextData();
    param3vd.SphereParaSet();
}

//マウスモーション処理（円柱・Y軸視点）
void	MotionPillarXY_Y( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod, heightmod;		//半径、高さの変化量

    //Yビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Y_VIEW )
        return;

    wkx = ( ( x - pillar.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );										//x軸方向のマウスの移動量
    wky = - ( pillar.m_previous_z - ( y - PBVR_VIEWPORT_HEIGHT ) ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );				//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarXYMaxMin( wkx, 0.0F, wky, 0.0F, 0.0F, 0.0F );

        if ( ( PillarXYXmax <= Xmax * 2 ) && ( PillarXYXmin >= Xmin * 2 )
                && ( PillarXYZmax <= Zmax * 2 ) && ( PillarXYZmin >= Zmin * 2 ) )
        {

            pillar.setPosition( x, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );
            pillar.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    radiusmod = wkx / pillar.m_scale_x;														//半径の変化量
    heightmod = wky / pillar.m_scale_z;														//高さの変化量

    // 円柱の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        SetPillarXYMaxMin( wkx, 0, wky, radiusmod, heightmod, 0 );
        if ( ( PillarXYZmin >= Zmin * 2 ) && ( PillarXYZmin < PillarXYZmax ) )
        {
            pillar.m_height = pillar.m_save_height + heightmod;
        }
    }

    // 円柱の下半分を掴むと、下辺を変形
    else if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        SetPillarXYMaxMin( wkx, 0, wky, radiusmod, 0, heightmod );
        if ( ( PillarXYZmax <= Zmax * 2 ) && ( PillarXYZmax > PillarXYZmin ) )
        {
            pillar.m_height = pillar.m_save_height - heightmod;
            pillar.m_center_z = pillar.m_save_center_z + heightmod;
            pillar.setPosition( 0, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );
        }
    }

    // 円柱の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        SetPillarXYMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXYXmax <= Xmax * 2 ) && ( PillarXYXmax > PillarXYXmin ) )
        {
            pillar.m_radius = pillar.m_save_radius + radiusmod;
        }
    }

    // 円柱の左半分を掴むと、左辺を変形
    if ( ModifyMode & PBVR_LEFTSIDE )
    {
        SetPillarXYMaxMin( 0, 0, 0, -radiusmod, 0, 0 );
        if ( ( PillarXYXmin >= Xmin * 2 ) && ( PillarXYXmin < PillarXYXmax ) )
        {
            pillar.m_radius = pillar.m_save_radius - radiusmod;
        }
    }

    //値を計算し、テキストボックスにセット
    pillar.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（円柱・Y軸視点）
void	MotionPillarXZ_Y( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod;					//半径の変化量

    //Yビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Y_VIEW )
        return;

    wkx = ( ( x - pillarXZ.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );									//x軸方向のマウスの移動量
    wky = - ( pillarXZ.m_previous_z - ( y - PBVR_VIEWPORT_HEIGHT ) ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );			//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarXZMaxMin( wkx, 0.0F, wky, 0.0F, 0.0F, 0.0F );

        if ( ( PillarXZXmax <= Xmax * 2 ) && ( PillarXZXmin >= Xmin * 2 )
                && ( PillarXZZmax <= Zmax * 2 ) && ( PillarXZZmin >= Zmin * 2 ) )
        {

            pillarXZ.setPosition( x, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );
            pillarXZ.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    // 円柱の右上を掴む
    if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / pillarXZ.m_scale_x )  + ( wky / pillarXZ.m_scale_z );									//半径の変化量
        SetPillarXZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZXmax < Xmax * 2 ) && ( PillarXZZmax <= Zmax * 2 ) )
        {
            if ( ( pillarXZ.m_save_radius * pillarXZ.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarXZ.m_radius = pillarXZ.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の左上を掴む
    else if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / pillarXZ.m_scale_x )  + ( wky / pillarXZ.m_scale_z );									//半径の変化量
        SetPillarXZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZXmin >= Xmin * 2 ) && ( PillarXZZmax <= Zmax * 2 ) )
        {
            if ( ( pillarXZ.m_save_radius * pillarXZ.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarXZ.m_radius = pillarXZ.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の右下を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / pillarXZ.m_scale_x )  - ( wky / pillarXZ.m_scale_z );									//半径の変化量
        SetPillarXZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZXmax < Xmax * 2 ) && ( PillarXZZmin >= Zmin * 2 ) )
        {
            if ( ( pillarXZ.m_save_radius * pillarXZ.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarXZ.m_radius = pillarXZ.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の左下を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / pillarXZ.m_scale_x ) - ( wky / pillarXZ.m_scale_z );									//半径の変化量
        SetPillarXZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZXmin >= Xmin * 2 ) && ( PillarXZZmin >= Zmin * 2 ) )
        {
            if ( ( pillarXZ.m_save_radius * pillarXZ.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillarXZ.m_radius = pillarXZ.m_save_radius + radiusmod;
            }
        }
    }

    //値を計算し、テキストボックスにセット
    pillarXZ.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（円柱・Y軸視点）
void	MotionPillarYZ_Y( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod, heightmod;		//半径、高さの変化量

    //Yビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Y_VIEW )
        return;

    wkx = ( ( x - pillarYZ.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 ) );										//x軸方向のマウスの移動量
    wky = - ( pillarYZ.m_previous_z - ( y - PBVR_VIEWPORT_HEIGHT ) ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );				//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarYZMaxMin( wkx, 0.0F, wky, 0.0F, 0.0F, 0.0F );

        if ( ( PillarYZXmax <= Xmax * 2 ) && ( PillarYZXmin >= Xmin * 2 )
                && ( PillarYZZmax <= Zmax * 2 ) && ( PillarYZZmin >= Zmin * 2 ) )
        {

            pillarYZ.setPosition( x, 0, -( PBVR_VIEWPORT_HEIGHT - y ) );
            pillarYZ.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    radiusmod = wky / pillarYZ.m_scale_z;														//半径の変化量
    heightmod = wkx / pillarYZ.m_scale_x;														//高さの変化量

    // 円柱の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarYZZmin >= Zmin * 2 ) && ( PillarYZZmin <  PillarYZZmax ) )
        {
            pillarYZ.m_radius = pillarYZ.m_save_radius + radiusmod;
        }
    }

    // 円柱の下半分を掴むと、下辺を変形
    else if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, -radiusmod, 0, 0 );
        if ( ( PillarYZZmax <= Zmax * 2 ) && ( PillarYZZmax >  PillarYZZmin ) )
        {
            pillarYZ.m_radius = pillarYZ.m_save_radius - radiusmod;
        }
    }

    // 円柱の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, 0, heightmod, 0 );
        if ( ( PillarYZXmax <= Xmax * 2 ) && ( PillarYZXmax >  PillarYZXmin ) )
        {
            pillarYZ.m_height = pillarYZ.m_save_height + heightmod;
        }
    }

    // 円柱の左半分を掴むと、左辺を変形
    if ( ModifyMode & PBVR_LEFTSIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, 0, 0, heightmod );
        if ( ( PillarYZXmin >= Xmin * 2 ) && ( PillarYZXmin < PillarYZXmax ) )
        {
            pillarYZ.m_height = pillarYZ.m_save_height - heightmod;
            pillarYZ.m_center_x = pillarYZ.m_save_center_x + heightmod;
            pillarYZ.setPosition( x, 0, 0 );
        }
    }

    //値を計算し、テキストボックスにセット
    pillarYZ.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（直方体・Z軸視点）
void	MotionCropZ( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量

    //Zビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Z_VIEW )
        return;

    wkx = ( x  - crop.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );										//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - crop.m_previous_y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );					//y軸方向のマウスの移動量
    CropTrans = PBVR_OFF;

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        CropTrans = ( PBVR_RIGHTSIDE | PBVR_LEFTSIDE | PBVR_TOP_SIDE | PBVR_BOTTOM_SIDE );
        SetCropMaxMin( wkx, wky, 0.0F );
        if ( ( CropXmax <= Xmax * 2 ) && ( CropXmin >= Xmin * 2 )
                && ( CropYmax <= Ymax * 2 ) && ( CropYmin >= Ymin * 2 ) )
        {

            crop.setPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );
            crop.setCropTextData();
            param3vd.CropParaSet();
        }
        return;
    }

    //＋コントロールキー ＝ 直方体の変形

    // 直方体の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        CropTrans |= PBVR_TOP_SIDE;
        SetCropMaxMin( 0.0F, wky, 0.0F );
        if ( ( CropYmax <= Ymax * 2 ) && ( CropYmax > cyminlevel2 ) )
            cymaxlevel2 = CropYmax;
    }
    // 直方体の下半分を掴むと、下辺を変形
    if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        CropTrans |= PBVR_BOTTOM_SIDE;
        SetCropMaxMin( 0.0F, wky, 0.0F );
        if ( ( CropYmin >= Ymin * 2 ) && ( CropYmin < cymaxlevel2 ) )
            cyminlevel2 = CropYmin;
    }

    // 直方体の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        CropTrans |= PBVR_RIGHTSIDE;
        SetCropMaxMin( wkx, 0.0F, 0.0F );
        if ( ( CropXmax <= Xmax * 2 ) && ( CropXmax > cxminlevel2 ) )
            cxmaxlevel2 = CropXmax;
    }

    // 直方体の左半分を掴むと、左辺を変形
    if ( ModifyMode & PBVR_LEFTSIDE )
    {
        CropTrans |= PBVR_LEFTSIDE;
        SetCropMaxMin( wkx, 0.0F, 0.0F );
        if ( ( CropXmin >= Xmin * 2 ) && ( CropXmin < cxmaxlevel2 ) )
            cxminlevel2 = CropXmin;
    }

    //値を計算し、テキストボックスにセット
    crop.m_side_y = cymaxlevel2 - cyminlevel2;
    crop.m_move_y = ( cymaxlevel2 + cyminlevel2 ) * ( PBVR_VIEWPORT_HEIGHT / 2 ) / 2;

    crop.m_side_x = cxmaxlevel2 - cxminlevel2;
    crop.m_move_x = ( cxmaxlevel2 + cxminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;

    crop.setCropTextBox();
    param3vd.CropParaSet();
}

//マウスモーション処理（球・Z軸視点）
void	MotionSphereZ( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod;					//半径の変化量

    //Zビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Z_VIEW )
        return;

    wkx = ( x - sphere.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );										//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - sphere.m_previous_y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 );				//y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetSphereMaxMin( wkx, wky, 0.0F , 0.0F );

        if ( ( SphereXmax <= Xmax * 2 ) && ( SphereXmin >= Xmin * 2 )
                && ( SphereYmax <= Ymax * 2 ) && ( SphereYmin >= Ymin * 2 ) )
        {

            sphere.setPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0  );
            sphere.setSphereTextData();
            param3vd.SphereParaSet();
        }
    }

    // 球の右上を掴む
    if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / sphere.m_scale_x )  + ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmax <= Xmax * 2 ) && ( SphereYmax <= Ymax * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の左上を掴む
    else if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / sphere.m_scale_x )  + ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmin >= Xmin * 2 ) && ( SphereYmax <= Ymax * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の右下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / sphere.m_scale_x )  - ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmax <= Xmax * 2 ) && ( SphereYmin >= Ymin * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    // 球の左下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / sphere.m_scale_x ) - ( wky / sphere.m_scale_y );									//半径の変化量
        SetSphereMaxMin( 0, 0, 0, radiusmod );
        if ( ( SphereXmin >= Xmin * 2 ) && ( SphereYmin >= Ymin * 2 ) )
        {
            if ( ( sphere.m_save_radius * sphere.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                sphere.m_radius = sphere.m_save_radius + radiusmod;
            }
        }
    }

    //値を計算し、テキストボックスにセット
    sphere.setSphereTextData();
    param3vd.SphereParaSet();
}

//マウスモーション処理（円柱・Z軸視点）
void	MotionPillarXY_Z( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod;					//半径の変化量

    //Zビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Z_VIEW )
        return;

    wkx = ( x - pillar.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );						//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - pillar.m_previous_y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ); //y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarXYMaxMin( wkx, wky, 0.0F, 0.0F, 0.0F, 0.0F );

        if ( ( PillarXYXmax <= Xmax * 2 ) && ( PillarXYXmin >= Xmin * 2 )
                && ( PillarXYYmax <= Ymax * 2 ) && ( PillarXYYmin >= Ymin * 2 ) )
        {

            pillar.setPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );
            pillar.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    // 円柱の右上を掴む
    if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / pillar.m_scale_x )  + ( wky / pillar.m_scale_y );									//半径の変化量
        SetPillarXYMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXYXmax <= Xmax * 2 ) && ( PillarXYYmax <= Ymax * 2 ) )
        {
            if ( ( pillar.m_save_radius * pillar.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillar.m_radius = pillar.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の左上を掴む
    else if ( ( ModifyMode & PBVR_TOP_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / pillar.m_scale_x )  + ( wky / pillar.m_scale_y );									//半径の変化量
        SetPillarXYMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXYXmin >= Xmin * 2 ) && ( PillarXYYmax <= Ymax * 2 ) )
        {
            if ( ( pillar.m_save_radius * pillar.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillar.m_radius = pillar.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の右下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_RIGHTSIDE ) )
    {
        radiusmod = ( wkx / pillar.m_scale_x )  - ( wky / pillar.m_scale_y );									//半径の変化量
        SetPillarXYMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXYXmax <= Xmax * 2 ) && ( PillarXYYmin >= Ymin * 2 ) )
        {
            if ( ( pillar.m_save_radius * pillar.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillar.m_radius = pillar.m_save_radius + radiusmod;
            }
        }
    }

    // 円柱の左下半分を掴む
    else if ( ( ModifyMode & PBVR_BOTTOM_SIDE ) && ( ModifyMode & PBVR_LEFTSIDE ) )
    {
        radiusmod = -( wkx / pillar.m_scale_x ) - ( wky / pillar.m_scale_y );									//半径の変化量
        SetPillarXYMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXYXmin >= Xmin * 2 ) && ( PillarXYYmin >= Ymin * 2 ) )
        {
            if ( ( pillar.m_save_radius * pillar.m_scale_x + radiusmod ) >= PBVR_RADIUS_MIN )
            {
                pillar.m_radius = pillar.m_save_radius + radiusmod;
            }
        }
    }

    //値を計算し、テキストボックスにセット
    pillar.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（円柱・Z軸視点）
void	MotionPillarXZ_Z( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod, heightmod;		//半径、高さの変化量

    //Zビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Z_VIEW )
        return;

    wkx = ( x - pillarXZ.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );							//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - pillarXZ.m_previous_y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ); //y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarXZMaxMin( wkx, wky, 0.0F, 0.0F, 0.0F, 0.0F );

        if ( ( PillarXZXmax <= Xmax * 2 ) && ( PillarXZXmin >= Xmin * 2 )
                && ( PillarXZYmax <= Ymax * 2 ) && ( PillarXZYmin >= Ymin * 2 ) )
        {

            pillarXZ.setPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );
            pillarXZ.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    radiusmod = wkx / pillarXZ.m_scale_z;														//半径の変化量
    heightmod = wky / pillarXZ.m_scale_y;														//高さの変化量

    // 円柱の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, 0, heightmod, 0 );
        if ( ( PillarXZYmax <= Ymax * 2 ) && ( PillarXZYmax > PillarXZYmin ) )
        {
            pillarXZ.m_height = pillarXZ.m_save_height + heightmod;
        }
    }

    // 円柱の下半分を掴むと、下辺を変形
    else if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, 0, 0, heightmod );
        if ( ( PillarXZYmin >= Ymin * 2 ) && ( PillarXZYmin < PillarXZYmax ) )
        {
            pillarXZ.m_height = pillarXZ.m_save_height - heightmod;
            pillarXZ.m_center_y = pillarXZ.m_save_center_y + heightmod;
            pillarXZ.setPosition( 0, PBVR_VIEWPORT_HEIGHT - y, 0 );
        }
    }

    // 円柱の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarXZXmax <= Xmax * 2 ) && ( PillarXZXmax > PillarXZXmin ) )
        {
            pillarXZ.m_radius = pillarXZ.m_save_radius + radiusmod;
        }
    }

    // 円柱の左半分を掴むと、左辺を変形
    else if ( ModifyMode & PBVR_LEFTSIDE )
    {
        SetPillarXZMaxMin( 0, 0, 0, -radiusmod, 0, 0 );
        if ( ( PillarXZXmin >= Xmin * 2 ) && ( PillarXZXmin < PillarXZXmax ) )
        {
            pillarXZ.m_radius = pillarXZ.m_save_radius - radiusmod;
        }
    }

    //値を計算し、テキストボックスにセット
    pillarXZ.setPillarTextBox();
    param3vd.PillarParaSet();
}

//マウスモーション処理（円柱・Z軸視点）
void	MotionPillarYZ_Z( const int x, const int y )
{

    float	wkx, wky;					//x,y 方向の移動量
    float	radiusmod, heightmod;		//半径、高さの変化量

    //Zビューポートの選択中でなければ、なければ、リターン
    if ( win_sel != PBVR_Z_VIEW )
        return;

    wkx = ( x - pillarYZ.m_previous_x ) / ( ( float )PBVR_VIEWPORT_WIDTH / 2 );						//x軸方向のマウスの移動量
    wky = ( ( PBVR_VIEWPORT_HEIGHT - y ) - pillarYZ.m_previous_y ) / ( ( float )PBVR_VIEWPORT_HEIGHT / 2 ); //y軸方向のマウスの移動量

    //移動処理
    if ( ModifyMode == PBVR_OFF )
    {
        SetPillarYZMaxMin( wkx, wky, 0.0F, 0.0F, 0.0F, 0.0F );

        if ( ( PillarYZXmax <= Xmax * 2 ) && ( PillarYZXmin >= Xmin * 2 )
                && ( PillarYZYmax <= Ymax * 2 ) && ( PillarYZYmin >= Ymin * 2 ) )
        {

            pillarYZ.setPosition( x, PBVR_VIEWPORT_HEIGHT - y, 0 );
            pillarYZ.setPillarTextData();
            param3vd.PillarParaSet();
            return;
        }
    }

    radiusmod = wky / pillarXZ.m_scale_y;														//半径の変化量
    heightmod = wkx / pillarXZ.m_scale_x;														//高さの変化量

    // 円柱の上半分を掴むと、上辺を変形
    if ( ModifyMode & PBVR_TOP_SIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, radiusmod, 0, 0 );
        if ( ( PillarYZYmax <= Ymax * 2 ) && ( PillarYZYmax > PillarYZYmin ) )
        {
            pillarYZ.m_radius = pillarYZ.m_save_radius + radiusmod;
        }
    }

    // 円柱の下半分を掴むと、下辺を変形
    else if ( ModifyMode & PBVR_BOTTOM_SIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, -radiusmod, 0, 0 );
        if ( ( PillarYZYmin >= Ymin * 2 ) && ( PillarYZYmin < PillarYZYmax ) )
        {
            pillarYZ.m_radius = pillarYZ.m_save_radius - radiusmod;
        }
    }

    // 円柱の右半分を掴むと、右辺を変形
    if ( ModifyMode & PBVR_RIGHTSIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, 0, heightmod, 0 );
        if ( ( PillarYZXmax <= Xmax * 2 ) && ( PillarYZXmax > PillarYZXmin ) )
        {
            pillarYZ.m_height = pillarYZ.m_save_height + heightmod;
        }
    }

    // 円柱の左半分を掴むと、左辺を変形
    else if ( ModifyMode & PBVR_LEFTSIDE )
    {
        SetPillarYZMaxMin( 0, 0, 0, 0, 0, heightmod );
        if ( ( PillarYZXmin >= Xmin * 2 ) && ( PillarYZXmin < PillarYZXmax ) )
        {
            pillarYZ.m_height = pillarYZ.m_save_height - heightmod;
            pillarYZ.m_center_x = pillarYZ.m_save_center_x + heightmod;
            pillarYZ.setPosition( x, 0, 0 );
        }
    }

    //値を計算し、テキストボックスにセット
    pillarYZ.setPillarTextBox();
    param3vd.PillarParaSet();
}

//特殊キー用コールバック
void SpecialFunc( const int key, const int x, const int y )
{
    //X軸視点ビューポートの選択中
    if ( ( 0 <= x ) && ( x < PBVR_VIEWPORT_WIDTH )  && ( 0 <= y ) && ( y < PBVR_VIEWPORT_HEIGHT ) )
    {
        switch ( obj_3d )
        {
        case	PBVR_CROP:
            SpecialFuncCropX( key, x, y );
            break;
        case	PBVR_SPHERE:
            SpecialFuncSphereX( key, x, y );
            break;
        case	PBVR_PILLARXY:
            SpecialFuncPillarXY_X( key, x, y );
            break;
        case	PBVR_PILLARYZ:
            SpecialFuncPillarYZ_X( key, x, y );
            break;
        case	PBVR_PILLARXZ:
            SpecialFuncPillarXZ_X( key, x, y );
            break;
        default:
            break;
        }
    }

    //Y軸視点ビューポートの選択中
    else if ( ( PBVR_VIEWPORT_WIDTH <= x ) && ( x <= PBVR_VIEWPORT_WIDTH * 2 ) && ( y > PBVR_VIEWPORT_HEIGHT  ) && ( PBVR_VIEWPORT_HEIGHT * 2 >= y ) )
    {
        switch ( obj_3d )
        {
        case	PBVR_CROP:
            SpecialFuncCropY( key, x, y );
            break;
        case	PBVR_SPHERE:
            SpecialFuncSphereY( key, x, y );
            break;
        case	PBVR_PILLARXY:
            SpecialFuncPillarXY_Y( key, x, y );
            break;
        case	PBVR_PILLARYZ:
            SpecialFuncPillarYZ_Y( key, x, y );
            break;
        case	PBVR_PILLARXZ:
            SpecialFuncPillarXZ_Y( key, x, y );
            break;
        default:
            break;
        }
    }

    //Z軸視点ビューポートの選択中
    else if ( ( PBVR_VIEWPORT_WIDTH <= x ) && ( x <= PBVR_VIEWPORT_WIDTH * 2 ) && ( 0 <= y ) && ( y < PBVR_VIEWPORT_HEIGHT ) )
    {
        switch ( obj_3d )
        {
        case	PBVR_CROP:
            SpecialFuncCropZ( key, x, y );
            break;
        case	PBVR_SPHERE:
            SpecialFuncSphereZ( key, x, y );
            break;
        case	PBVR_PILLARXY:
            SpecialFuncPillarXY_Z( key, x, y );
            break;
        case	PBVR_PILLARYZ:
            SpecialFuncPillarYZ_Z( key, x, y );
            break;
        case	PBVR_PILLARXZ:
            SpecialFuncPillarXZ_Z( key, x, y );
            break;
        default:
            break;
        }
    }
}

//特殊キー押下処理（直方体・X軸視点）
void	SpecialFuncCropX( const int key, const int x, const int y )
{

    switch ( key )
    {

    case GLUT_KEY_RIGHT:
        crop.scalingCrop( 0, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        crop.scalingCrop( 0, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        crop.scalingCrop( 0, PBVR_MAGSCALE, 0 );
        break;
    case GLUT_KEY_DOWN:
        crop.scalingCrop( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        crop.scalingCrop( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        crop.scalingCrop( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（直方体・Y軸視点）
void	SpecialFuncCropY( const int key, const int x, const int y )
{

    switch ( key )
    {

    case GLUT_KEY_RIGHT:
        crop.scalingCrop( PBVR_MAGSCALE, 0, 0 );
        break;
    case GLUT_KEY_LEFT:
        crop.scalingCrop( PBVR_REDSCALE, 0, 0 );
        break;
    case GLUT_KEY_UP:
        crop.scalingCrop( 0, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        crop.scalingCrop( 0, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        crop.scalingCrop( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        crop.scalingCrop( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（直方体・Z軸視点）
void	SpecialFuncCropZ( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        crop.scalingCrop( PBVR_MAGSCALE, 0, 0 );
        break;
    case GLUT_KEY_LEFT:
        crop.scalingCrop( PBVR_REDSCALE, 0, 0 );
        break;
    case GLUT_KEY_UP:
        crop.scalingCrop( 0, PBVR_MAGSCALE,  0 );
        break;
    case GLUT_KEY_DOWN:
        crop.scalingCrop( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        crop.scalingCrop( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        crop.scalingCrop( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（球・X軸視点）
void	SpecialFuncSphereX( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（球・Y軸視点）
void	SpecialFuncSphereY( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（球・Z軸視点）
void	SpecialFuncSphereZ( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        sphere.scalingSphere( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        sphere.scalingSphere( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・X軸視点）
void	SpecialFuncPillarXY_X( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillar.scalingPillar( 0, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        pillar.scalingPillar( 0, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        pillar.scalingPillar( 0, PBVR_MAGSCALE, 0 );
        break;
    case GLUT_KEY_DOWN:
        pillar.scalingPillar( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        pillar.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillar.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・Y軸視点）
void	SpecialFuncPillarXY_Y( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillar.scalingPillar( PBVR_MAGSCALE, 0, 0 );
        break;
    case GLUT_KEY_LEFT:
        pillar.scalingPillar( PBVR_REDSCALE, 0, 0 );
        break;
    case GLUT_KEY_UP:
        pillar.scalingPillar( 0, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        pillar.scalingPillar( 0, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        pillar.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillar.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・Z軸視点）
void	SpecialFuncPillarXY_Z( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillar.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, 0 ) ;
        break;
    case GLUT_KEY_LEFT:
        pillar.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_UP:
        pillar.scalingPillar( 0, PBVR_MAGSCALE, 0 );
        break;
    case GLUT_KEY_DOWN:
        pillar.scalingPillar( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        pillar.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillar.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//------------
//特殊キー押下処理（円柱・X軸視点）
void	SpecialFuncPillarXZ_X( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        pillarXZ.scalingPillar( PBVR_REDSCALE, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        pillarXZ.scalingPillar( 0, PBVR_MAGSCALE, 0 );
        break;
    case GLUT_KEY_DOWN:
        pillarXZ.scalingPillar( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillarXZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・Y軸視点）
void	SpecialFuncPillarXZ_Y( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        pillarXZ.scalingPillar( PBVR_REDSCALE, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, 0, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        pillarXZ.scalingPillar( PBVR_REDSCALE, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillarXZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・Z軸視点）
void	SpecialFuncPillarXZ_Z( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, 0, PBVR_MAGSCALE ) ;
        break;
    case GLUT_KEY_LEFT:
        pillarXZ.scalingPillar( PBVR_REDSCALE, 0, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        pillarXZ.scalingPillar( 0, PBVR_MAGSCALE, 0 );
        break;
    case GLUT_KEY_DOWN:
        pillarXZ.scalingPillar( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        pillarXZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillarXZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//-------------
//特殊キー押下処理（円柱・X軸視点）
void	SpecialFuncPillarYZ_X( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillarYZ.scalingPillar( 0, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_LEFT:
        pillarYZ.scalingPillar( 0, PBVR_MAGSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_UP:
        pillarYZ.scalingPillar( 0, PBVR_MAGSCALE, 0 );
        break;
    case GLUT_KEY_DOWN:
        pillarYZ.scalingPillar( 0, PBVR_REDSCALE, 0 );
        break;
    case GLUT_KEY_PAGE_UP:
        pillarYZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillarYZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・Y軸視点）
void	SpecialFuncPillarYZ_Y( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillarYZ.scalingPillar( PBVR_MAGSCALE, 0, 0 );
        break;
    case GLUT_KEY_LEFT:
        pillarYZ.scalingPillar( PBVR_REDSCALE, 0, 0 );
        break;
    case GLUT_KEY_UP:
        pillarYZ.scalingPillar( 0, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        pillarYZ.scalingPillar( 0, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        pillarYZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillarYZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}

//特殊キー押下処理（円柱・Z軸視点）
void	SpecialFuncPillarYZ_Z( const int key, const int x, const int y )
{

    switch ( key )
    {
    case GLUT_KEY_RIGHT:
        pillarYZ.scalingPillar( PBVR_MAGSCALE, 0, 0 ) ;
        break;
    case GLUT_KEY_LEFT:
        pillarYZ.scalingPillar( PBVR_REDSCALE, 0, 0 );
        break;
    case GLUT_KEY_UP:
        pillarYZ.scalingPillar( 0, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_DOWN:
        pillarYZ.scalingPillar( 0, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    case GLUT_KEY_PAGE_UP:
        pillarYZ.scalingPillar( PBVR_MAGSCALE, PBVR_MAGSCALE, PBVR_MAGSCALE );
        break;
    case GLUT_KEY_PAGE_DOWN:
        pillarYZ.scalingPillar( PBVR_REDSCALE, PBVR_REDSCALE, PBVR_REDSCALE );
        break;
    default:
        break;
    }
}
//----------------

//ウィンドウリサイズ用コールバック
void Resize( const int w, const int h )
{
    WindowWidth = w;
    WindowHeight = h;

    Initialize3vd( iwin_3vd );
    End3vd( iwin_3vd );
}

//
void	Win3vd()
{
    std::cout << "void Win3vd()" << std::endl;
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );

    //三面図用ウィンドウの生成
    glutInitWindowSize( PBVR_WINDOW_W, PBVR_WINDOW_H );
    glutInitWindowPosition( 1130, 360 );
    iwin_3vd = glutCreateWindow( "3vd" );

    //コールバック関数を登録
    glutDisplayFunc( Display3vd );
    glutMouseFunc( MouseFunc );
    glutMotionFunc( MouseMotion );
    glutSpecialFunc( SpecialFunc );
    glutReshapeFunc( Resize );

    //guiパネルの生成
    InitializePBVRParameter( );
    std::cout << " ### InitPBVRParameter( ) done " << std::endl;
    GUIPanel();
    std::cout << " ### GUIPanel() done " << std::endl;
    glutSetWindow( iwin_3vd );
    std::cout << " ### glutSetWindow(iwin_3vd) done " << std::endl;

#ifndef NOTOPEN
    glutHideWindow();
#endif
}



#ifdef NOTOPEN
int main( int argc, char* argv[] )
{
//テスト用メインウィンドウ--------------------------
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );

    glutInitWindowSize( PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
    glutInitWindowPosition( 300, 300 );
    iwin_main = glutCreateWindow( "Main Win" );

    glutDisplayFunc( Display );
    glClearColor( 0.0, 0.0, 0.0, 0.0 );

    glutInitWindowPosition( 600, 600 );
    GLUI* gluitest = GLUI_Master.create_glui( "control", 0, 450, 630 );

    GLUI_Rotation* view_rot = gluitest->add_rotation( "Rotation", rotate );

//--------------------------------------------------
    obj_3d = obj_sel + 1;

//	Win3vd();

    glutMainLoop();
    return 0;
}
#endif

#ifndef NOTOPEN
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/OpenGL>
#include <kvs/PaintEventListener>

// tantan
class PaintEvent : public kvs::PaintEventListener
{
public:
    void update();
};

void	PBVRScaling()
{

    mapscale = PBVRscale / PBVR_WIN3VDSCALE;
}

//PBVRペイントイベント・PBVRウィンドウに選択領域を表示
void PaintEvent::update()
{
    //CROPボタンが押されていなかったら、リターン
    if ( crop_flg == PBVR_FALSE )
        return;

    glPushMatrix();

    PBVRScaling();

    switch ( obj_3d )
    {
    case	PBVR_CROP:

        DrawCrop( PBVR_WIRE );
        DrawCrop( PBVR_SOLID );

        break;

    case	PBVR_SPHERE:

        DrawSphere( PBVR_WIRE );
        DrawSphere( PBVR_SOLID );

        break;

    case	PBVR_PILLARXY:

        DrawPillar( PBVR_WIRE );
        DrawPillar( PBVR_SOLID );

        break;

    case	PBVR_PILLARYZ:

        DrawPillar( PBVR_WIRE );
        DrawPillar( PBVR_SOLID );

        break;

    case	PBVR_PILLARXZ:

        DrawPillar( PBVR_WIRE );
        DrawPillar( PBVR_SOLID );

        break;

    default:
        break;
    }

    glFlush();
    glPopMatrix();
}
#endif

void	CallBackCrop( const int num )
{
    glutSetWindow( iwin_3vd );
    glutShowWindow();
    crop_flg = PBVR_TRUE;
    obj_3d = obj_sel + 1;
    param3vd.m_select_area = obj_sel + 1;
#ifdef __APPLE__
    Display3vd();
#endif
}

// APPEND START FP)K.YAJIMA 2015.03.11
void	CallBackFile( const int num )
{
    if ( FileCallback_Cancel ) return;
    FileCallback_Cancel = true;
    g_glui_filepanel->show();
}
// APPEND END FP)K.YAJIMA 2015.03.11

// APPEND START FP)K.YAJIMA 2015.02.20
// MODIFY START FP)K.YAJIMA 2015.03.11
//void	FileCallback(int num)
void	FILEpanelCreate()
// MODIFY END FP)K.YAJIMA 2015.03.11
{

// MODIFY START FP)K.YAJIMA 2015.03.11
    //if(FileCallback_Cancel) return;
    //FileCallback_Cancel = true;
    //GLUI *glui = GLUI_Master.create_glui("FILE Panel");

    g_glui_filepanel = GLUI_Master.create_glui( "FILE Panel" );
// MODIFY END FP)K.YAJIMA 2015.03.11

// DELETE START FP)K.YAJIMA 2015.03.02
//TO DO:いずれ元に戻す
//	// VOLUME DATA FILE (SERVER)
//	GLUI_StaticText *lbl_PFIF;
//	lbl_PFIF = glui->add_statictext("PBVR Filter Infomation FILE: (SERVER)");
//	GLUI_Panel *obj_panelvold = glui->add_panel("", 3);
//	obj_panelvold->set_alignment(GLUI_ALIGN_LEFT);
//	GLUI_EditText *volumedata;
//	char buff_volumedata[100] = "/work/filter/case/case";
//	//char buff_volumedata[100] = "/work/filter/binary08/binary";
//	//glui->add_statictext("pfi FILE: (SERVER)");
//	//volumedata = glui->add_edittext( "", GLUI_EDITTEXT_TEXT, buff_volumedata );
//	volumedata = glui->add_edittext_to_panel(obj_panelvold, "", GLUI_EDITTEXT_TEXT, buff_volumedata );
//	volumedata->set_w(250);
// DELETE END FP)K.YAJIMA 2015.03.02

// DELETE START FP)K.YAJIMA 2015.03.02
//	// WORK DIRECTORY (CLIENT)
//	glui->add_statictext("WORK DIR. (CLIENT):");
//	GLUI_Panel *obj_panelworkd = glui->add_panel("", 3);
//	obj_panelworkd->set_alignment(GLUI_ALIGN_LEFT);
//	GLUI_EditText *workdir;
//	//char buff_workdir[100] = "c:\\temp\0";
//	//glui->add_statictext("WORK DIR. (CLIENT):");
//	//workdir = glui->add_edittext("", GLUI_EDITTEXT_TEXT, buff_workdir );
//	//workdir->set_w(250);
//	workdir = glui->add_edittext_to_panel(obj_panelworkd, "",GLUI_EDITTEXT_TEXT,buff_workdir);
//	workdir->set_w(250);
//
//	glui->add_column_to_panel(obj_panelworkd,false);
//
//	//ワークディレクトリのディレクトリダイアログボタン
//	GLUI_Button *btn_wdd;
//	btn_wdd = glui->add_button_to_panel(obj_panelworkd, "Browse...",1,CallBackDirectoryDialog); // TO DO:機能未実装
//	btn_wdd->set_alignment(GLUI_ALIGN_CENTER);
// DELETE END FP)K.YAJIMA 2015.03.02

//	glui->add_separator();

    // ヴィズパラメータファイル
    g_glui_filepanel->add_statictext( "VIZ PARAMETER FILE:" );
    GLUI_Panel* obj_panelvizp = g_glui_filepanel->add_panel( "", 3 );
    obj_panelvizp->set_alignment( GLUI_ALIGN_LEFT );

    GLUI_EditText* paramfile;
    // char buff_paramfile[100] = "c:\\temp\\param.in";
    //glui->add_statictext("VIZ PARAMETER FILE:");
    //paramfile = glui->add_edittext("", GLUI_EDITTEXT_TEXT, buff_paramfile );
    //paramfile->set_w(250);
    paramfile = g_glui_filepanel->add_edittext_to_panel( obj_panelvizp, "", GLUI_EDITTEXT_TEXT, buff_paramfile );
    paramfile->set_w( 250 );

    g_glui_filepanel->add_column_to_panel( obj_panelvizp, false );

    //ヴィズパラメータのファイルダイアログボタン
    GLUI_Button* btn_vdd;
    btn_vdd = g_glui_filepanel->add_button_to_panel( obj_panelvizp, "Browse...", 1, CallBackFileDialog ); // TO DO:機能未実装
    btn_vdd->set_alignment( GLUI_ALIGN_CENTER );

// APPEND START FP)M.Tanaka 2015.03.02
    g_btn_vdd = btn_vdd;
    g_paramfile = paramfile;
// APPEND END   FP)M.Tanaka 2015.03.02

    GLUI_Panel* obj_panelei = g_glui_filepanel->add_panel( "", 3 );
    obj_panelei->set_alignment( GLUI_ALIGN_LEFT );

    /* 下記、エクスポートファイルボタンを右詰めにするために
    縦線設置*/
    g_glui_filepanel->add_column_to_panel( obj_panelei, false );
    g_glui_filepanel->add_column_to_panel( obj_panelei, false );
    g_glui_filepanel->add_column_to_panel( obj_panelei, false );
    g_glui_filepanel->add_column_to_panel( obj_panelei, false );
    g_glui_filepanel->add_column_to_panel( obj_panelei, false );

    //Export FILE(PARAMETER FILE SAVE)
    GLUI_Button* btn_paramfile;
    btn_paramfile = g_glui_filepanel->add_button_to_panel( obj_panelei, "Export FILE", 1, CallBackParameterFile );
    btn_paramfile->set_alignment( GLUI_ALIGN_LEFT );

    g_glui_filepanel->add_column_to_panel( obj_panelei, false );

    // Import FILE
    GLUI_Button* btn_importfile;
    btn_importfile = g_glui_filepanel->add_button_to_panel( obj_panelei, "Import FILE", 1, CallBackReadFile );
    btn_importfile->set_alignment( GLUI_ALIGN_LEFT );


// APPEND START FP)M.Tanaka 2015.03.05
    g_glui_filepanel->add_statictext( "" );
    // Close Button
    GLUI_Button* btn_fileclose;
    btn_fileclose = g_glui_filepanel->add_button( "Close", 1, CallBackFileClose );
    btn_fileclose->set_alignment( GLUI_ALIGN_CENTER );
    g_glui_filepanel = g_glui_filepanel;
// APPEND END   FP)M.Tanaka 2015.03.05

// APPEND START FP)K.YAJIMA 2015.03.11
    g_glui_filepanel->hide();
// APPEND END FP)K.YAJIMA 2015.03.11
}

// APPEND START FP)M.Tanaka 2015.03.05
void CallBackFileClose( const int num )
{
    FileCallback_Cancel = false;
    g_glui_filepanel->hide();
}
// APPEND END   FP)M.Tanaka 2015.03.05


// ファイルダイアログ
void	CallBackFileDialog( const int num )
{
// APPEND START FP)M.Tanaka 2015.03.02

    GLUI* glui = GLUI_Master.create_glui( "FileDialog" );
    FileDialog* fd = new FileDialog( glui, g_btn_vdd, 0, g_paramfile->get_text(), g_paramfile );
}
// APPEND START FP)M.Tanaka 2015.03.02

void	CallBackParticle( const int num )
{
    //glutWinFd = glutCreateWindow("FILEDialog");
    //GLUIの初期設定
    //GLUI *glui = GLUI_Master.create_glui("FILEDialog");
}
// APPEND END FP)K.YAJIMA 2015.02.20

void	CalculateCoordinateScaling( const double xmin, const double xmax,
                                    const double ymin, const double ymax,
                                    const double zmin, const double zmax,
                                    double* sides, double* CScale )
{
    /*
    [座標値]
      最小値: (x,y,z)=(-272.465, -159.000,   0.000)
      最大値: (x,y,z)=( 577.994,  159.000, 577.994)

    forkの最少／最大値
    X：-1.75　1.75
    Y： 0　7.945124149
    Z： 0　8.007808685
    */

    *sides = ( ( xmax - xmin ) > ( ymax - ymin ) ) ? ( xmax - xmin ) : ( ymax - ymin );
    *sides = ( *sides > ( zmax - zmin ) ) ? *sides : ( zmax - zmin );

    *CScale = ( PBVR_WIN3VDSCALE * 2 ) / *sides;
}

//PBVRウィンドウの座標値を3vdウィンドウの座標値に変換
void	Calculate3vdCoord( const float max, const float min, const float p, float* point )
{
//中心を3vdウィンドウの（０，０）に移動し、最大スケールの座標のスケールを掛ける
    *point = ( p - ( ( max + min ) / 2 ) ) * PBVRCoordScale;
}

//3vdウィンドウ上の座標をPBVRウィンドウの座標値に変換
void	CalculatePBVRCoord( const float max, const float min, const float p, float* point )
{
//最大スケールの座標のスケールで割り、中心(0, 0)をPBVR領域の中心に移動する
    *point = ( p / PBVRCoordScale ) + ( ( max + min ) / 2 );
}

//PBVR用パラメータ初期化
void	InitializePBVRParameter()
{
    std::cout << "void InitPBVRParameter()" << std::endl;
    /*
    [座標値]
      最小値: (x,y,z)=(-272.465, -159.000,   0.000)
      最大値: (x,y,z)=( 577.994,  159.000, 577.994)

    forkの最少／最大値
    X：-1.75	1.75
    Y： 0		7.75
    Z： -1		0.0F
    */

//#ifdef NOTOPEN
//	PBVRxmin = -272.465; PBVRymin =  -159.000;  PBVRzmin = 0.000;
//	PBVRxmax =  577.994; PBVRymax =   159.000;  PBVRzmax = 577.994;
//#else
    PBVRxmin = ( double )PVBRmincoords[0];
    PBVRymin = ( double )PVBRmincoords[1];
    PBVRzmin = ( double )PVBRmincoords[2];
    PBVRxmax = ( double )PVBRmaxcoords[0];
    PBVRymax = ( double )PVBRmaxcoords[1];
    PBVRzmax = ( double )PVBRmaxcoords[2];
//#endif

    printf( "============== InitPBVRParameter %f, %f, %f, %f, %f, %f\n", PBVRxmin, PBVRymin, PBVRzmin, PBVRxmax, PBVRymax, PBVRzmax );

//	PBVRxmin = -272.465; PBVRymin =  -159.000;  PBVRzmin = 0.000;
//	PBVRxmax =  577.994; PBVRymax =   159.000;  PBVRzmax = 577.994;

//	PBVRxmin = -1.75;	PBVRymin =  0.0F;			PBVRzmin = -1.0F;
//	PBVRxmax = 1.75;	PBVRymax =  7.75;			PBVRzmax = 0.0F;
//
    //CROP BOXの初期値をグローバルな座標のmin,maxに設定する
    crop.initializeCrop( 1 );
    /*
            double maxwidth = PBVRxmax - PBVRxmin;
            if (maxwidth < PBVRymax - PBVRymin) maxwidth = PBVRymax - PBVRymin;
            if (maxwidth < PBVRzmax - PBVRzmin) maxwidth = PBVRzmax - PBVRzmin;
            cxmaxlevel2 = (PBVRxmax - PBVRxmin)/maxwidth;
            cxminlevel2 = -cxmaxlevel2;
            cymaxlevel2 = (PBVRymax - PBVRymin)/maxwidth;
            cyminlevel2 = -cymaxlevel2;
            czmaxlevel2 = (PBVRzmax - PBVRzmin)/maxwidth;
            czminlevel2 = -czmaxlevel2;
            param3vd.CropParaSet();
    	crop.m_side_x = cxmaxlevel2 - cxminlevel2;
    	crop.m_side_y = cymaxlevel2 - cyminlevel2;
    	crop.m_side_z = czmaxlevel2 - czminlevel2;
    	crop.m_move_x = ( cxmaxlevel2 + cxminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    	crop.m_move_y = ( cymaxlevel2 + cyminlevel2 ) * ( PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    	crop.m_move_z = ( czmaxlevel2 + czminlevel2 ) *(  PBVR_VIEWPORT_WIDTH / 2 ) / 2;
    	crop.savePosition(crop.m_move_x, crop.m_move_y, crop.m_move_z);
    	save_cxmin = cxminlevel2;
    	save_cxmax = cxmaxlevel2;
    	save_cymin = cyminlevel2;
    	save_cymax = cymaxlevel2;
    	save_czmin = czminlevel2;
    	save_czmax = czmaxlevel2;
    */
    param3vd.CropParaSet();

    CalculateCoordinateScaling( PBVRxmin, PBVRxmax, PBVRymin, PBVRymax, PBVRzmin, PBVRzmax, &PBVRsides, &PBVRCoordScale );

    //最大値/最小値をセット
    Set3vdMax( PBVR_WIN3VDSCALE, -PBVR_WIN3VDSCALE, &Xmax, &Xmin );
    Set3vdMax( PBVR_WIN3VDSCALE, -PBVR_WIN3VDSCALE, &Ymax, &Ymin );
    Set3vdMax( PBVR_WIN3VDSCALE, -PBVR_WIN3VDSCALE, &Zmax, &Zmin );

    SetPBVRMax( PBVRxmax, PBVRxmin, &PBVRxmaxChk, &PBVRxminChk );
    SetPBVRMax( PBVRymax, PBVRymin, &PBVRymaxChk, &PBVRyminChk );
    SetPBVRMax( PBVRzmax, PBVRzmin, &PBVRzmaxChk, &PBVRzminChk );

    //テキストボックスデフォルト値をPBVRウィンドウの座標に変換
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, cxminlevel2, &tbcxminlevel );
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, cxmaxlevel2, &tbcxmaxlevel );
    CalculatePBVRCoord( PBVRymax, PBVRymin, cyminlevel2, &tbcyminlevel );
    CalculatePBVRCoord( PBVRymax, PBVRymin, cymaxlevel2, &tbcymaxlevel );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, czminlevel2, &tbczminlevel );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, czmaxlevel2, &tbczmaxlevel );

    Calculate3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin + ( PBVRxmax - PBVRxmin ) / 2.0F, &scenterx );
    Calculate3vdCoord( PBVRymax, PBVRymin, PBVRymin + ( PBVRymax - PBVRymin ) / 2.0F, &scentery );
    Calculate3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin + ( PBVRzmax - PBVRzmin ) / 2.0F, &scenterz );
    CalculatePBVRCoord( PBVRxmax, PBVRxmin, scenterx, &tbscenterx );
    CalculatePBVRCoord( PBVRymax, PBVRymin, scentery, &tbscentery );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, scenterz, &tbscenterz );
    /*
            sradius = (PBVRxmax-PBVRxmin)*(PBVRxmax-PBVRxmin) + (PBVRymax-PBVRymin)*(PBVRymax-PBVRymin) + (PBVRzmax-PBVRzmin)*(PBVRzmax-PBVRzmin);
            sradius = sqrt (sradius)/2.0F;
            sradius = sradius*PBVRCoordScale;
    	tbsradius = sradius / PBVRCoordScale;
    */
    sphere.initializeSphere( 1 );
    param3vd.SphereParaSet();

    /*
            pillar.radius = (PBVRxmax-PBVRxmin)*(PBVRxmax-PBVRxmin) + (PBVRymax-PBVRymin)*(PBVRymax-PBVRymin);
            pillar.radius = sqrt (pillar.radius)/2.0F;
            pillar.radius = pillar.radius*PBVRCoordScale;
            pillar.height = (PBVRzmax - PBVRzmin)*PBVRCoordScale;
            Calc3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin+(PBVRxmax-PBVRxmin)/2.0F, &pillar.m_center_x );
            Calc3vdCoord( PBVRymax, PBVRymin, PBVRymin+(PBVRymax-PBVRymin)/2.0F, &pillar.m_center_y );
            Calc3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin, &pillar.m_center_z );
    */
    obj_3d = PBVR_PILLAR;
    pillar.initializePillar( 1 );
    pillar.rem_loadPillar();

    /*
            pillarYZ.radius = (PBVRymax-PBVRymin)*(PBVRymax-PBVRymin) + (PBVRzmax-PBVRzmin)*(PBVRzmax-PBVRzmin);
            pillarYZ.radius = sqrt (pillarYZ.radius)/2.0F;
            pillarYZ.radius = pillarYZ.radius*PBVRCoordScale;
            pillarYZ.height = (PBVRxmax - PBVRxmin)*PBVRCoordScale;
            Calc3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin, &pillarYZ.m_center_x );
            Calc3vdCoord( PBVRymax, PBVRymin, PBVRymin+(PBVRymax-PBVRymin)/2.0F, &pillarYZ.m_center_y );
            Calc3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin+(PBVRzmax-PBVRzmin)/2.0F, &pillarYZ.m_center_z );
    */
    obj_3d = PBVR_PILLARYZ;
    pillarYZ.initializePillar( 1 );
    pillarYZ.rem_loadPillar();

    /*
            pillarXZ.radius = (PBVRxmax-PBVRxmin)*(PBVRxmax-PBVRxmin) + (PBVRzmax-PBVRzmin)*(PBVRzmax-PBVRzmin);
            pillarXZ.radius = sqrt (pillarXZ.radius)/2.0F;
            pillarXZ.radius = pillarXZ.radius*PBVRCoordScale;
            pillarXZ.height = (PBVRymax - PBVRymin)*PBVRCoordScale;
            Calc3vdCoord( PBVRxmax, PBVRxmin, PBVRxmin+(PBVRxmax-PBVRxmin)/2.0F, &pillarXZ.m_center_x );
            Calc3vdCoord( PBVRymax, PBVRymin, PBVRymin, &pillarXZ.m_center_y );
            Calc3vdCoord( PBVRzmax, PBVRzmin, PBVRzmin+(PBVRzmax-PBVRzmin)/2.0F, &pillarXZ.m_center_z );
    */
    obj_3d = PBVR_PILLARXZ;
    pillarXZ.initializePillar( 1 );
    pillarXZ.rem_loadPillar();
    obj_3d = PBVR_CROP;

    pradius = pillar.m_radius;
    pheight = pillar.m_height;
    pcenterx = pillar.m_center_x;
    pcentery = pillar.m_center_y;
    pcenterz = pillar.m_center_z;

    CalculatePBVRCoord( PBVRxmax, PBVRxmin, pcenterx, &tbpcenterx );
    CalculatePBVRCoord( PBVRymax, PBVRymin, pcentery, &tbpcentery );
    CalculatePBVRCoord( PBVRzmax, PBVRzmin, scenterz, &tbpcenterz );
    tbpradius = pradius / PBVRCoordScale;
    tbpheight = pheight / PBVRCoordScale;
    param3vd.PillarParaSet();
}

//3vdウィンドウ内の最大／最小値をセット
void	Set3vdMax( const float max, const float min, float* areamax, float* areamin )
{
    *areamax = max;
    *areamin = min;
}

//PBVRウィンドウ内の最大／最小値をセット
void	SetPBVRMax( const double max, const double min, double* areamax, double* areamin )
{
    *areamax =  PBVRsides + ( max + min ) / 2;
    *areamin = -PBVRsides + ( max + min ) / 2;
}

void	DrawCrop( const int type )
{
    int	i, j;

    if ( type == PBVR_WIRE )
    {
        glColor3f( 1.0, 0.0, 0.0 );
        for ( i = 0; i < 6 ; i++ )
        {
            glBegin( GL_LINE_LOOP );
            for ( j = 0; j < 4 ; j++ )
            {
                glVertex3f( verticesCrop[( ConnectionArrayCrop[i * 4 + j] ) * 3]*mapscale,
                            verticesCrop[( ConnectionArrayCrop[i * 4 + j] ) * 3 + 1]*mapscale,
                            verticesCrop[( ConnectionArrayCrop[i * 4 + j] ) * 3 + 2]*mapscale );
            }
            glEnd();
        }
    }

    if ( type == PBVR_SOLID )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glColor4f( 0.5, 0.5, 0.5, PBVR_ALPHA );
        glEnable( GL_NORMALIZE );

        glBegin( GL_QUADS );
        for ( i = 0; i < 6 ; i++ )
        {
            glNormal3fv( &NormalArrayCrop[i * 3] );
            for ( j = 0; j < 4 ; j++ )
            {
                glVertex3f( verticesCrop[( ConnectionArrayCrop[i * 4 + j] ) * 3]*mapscale,
                            verticesCrop[( ConnectionArrayCrop[i * 4 + j] ) * 3 + 1]*mapscale,
                            verticesCrop[( ConnectionArrayCrop[i * 4 + j] ) * 3 + 2]*mapscale );
            }
        }
    }
    glEnd();
    glDisable( GL_BLEND );
}

void	DrawSphere( const int type )
{

    int	i, j, sides;

    if ( type == PBVR_WIRE )
    {

        glColor3f( 1.0, 0.0, 0.0 );
        for ( i = 0, sides = 0 ; sides < ( PBVR_SIDES * 2 + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ); ++sides, ++i )
        {
            glBegin( GL_LINE_STRIP );
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 2]*mapscale );
            }
            glEnd();
        }
    }

    if ( type == PBVR_SOLID )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glColor4f( 0.5, 0.5, 0.5, PBVR_ALPHA );
        glEnable( GL_NORMALIZE );

        glBegin( GL_TRIANGLES );													//上面
        for ( i = 0, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
        {
            glNormal3fv( &NormalArraySphere[i * 3] );
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 2]*mapscale );
            }
        }
        glEnd();

        glBegin( GL_TRIANGLES );													//側面
        for ( i = PBVR_SIDES, sides = 0 ; sides < ( PBVR_SIDES * ( PBVR_SIDES - 2 ) ); ++sides, ++i )
        {
            glNormal3fv( &NormalArraySphere[i * 3] );
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 2]*mapscale );
            }
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesSphere[( ConnectionArraySphere[( i + PBVR_SIDES * ( PBVR_SIDES - 2 ) ) * 3 + j] ) * 3]*mapscale,
                            verticesSphere[( ConnectionArraySphere[( i + PBVR_SIDES * ( PBVR_SIDES - 2 ) ) * 3 + j] ) * 3 + 1]*mapscale,
                            verticesSphere[( ConnectionArraySphere[( i + PBVR_SIDES * ( PBVR_SIDES - 2 ) ) * 3 + j] ) * 3 + 2]*mapscale );
            }
        }
        glEnd();

        glBegin( GL_TRIANGLES );													//下面
        for ( i = ( PBVR_SIDES + PBVR_SIDES * 2 * ( PBVR_SIDES - 2 ) ), sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )
        {
            glNormal3fv( &NormalArraySphere[i * 3] );
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesSphere[( ConnectionArraySphere[i * 3 + j] ) * 3 + 2]*mapscale );
            }
        }
        glEnd();
        glDisable( GL_BLEND );
    }
}

void	DrawPillar( const int type )
{

    int	i, j, sides;

    if ( type == PBVR_WIRE )
    {

        glColor3f( 1.0, 0.0, 0.0 );
        for ( i = 0, sides = 0 ; sides < PBVR_SIDES * 4; ++sides, ++i )
        {
            glBegin( GL_LINE_STRIP );
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 2]*mapscale );
            }
            glEnd();
        }
    }

    if ( type == PBVR_SOLID )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glColor4f( 0.5, 0.5, 0.5, PBVR_ALPHA );
        glEnable( GL_NORMALIZE );

        glBegin( GL_TRIANGLES );
        glNormal3fv( &NormalArrayPillar[0] );
        for ( i = 0, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )											//上面
        {
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 2]*mapscale );
            }
        }
        glEnd();

        glBegin( GL_TRIANGLES );
        glNormal3fv( &NormalArrayPillar[PBVR_SIDES * 3] );
        for ( i = PBVR_SIDES, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )										//底面
        {
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 2]*mapscale );
            }
        }
        glEnd();

        for ( i = PBVR_SIDES * 2, sides = 0 ; sides < PBVR_SIDES; ++sides, ++i )									//側面
        {
            glBegin( GL_TRIANGLES );
            glNormal3fv( &NormalArrayPillar[i * 3] );
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 1]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[i * 3 + j] ) * 3 + 2]*mapscale );
            }
            for ( j = 0; j < 3; j++ )
            {
                glVertex3f( verticesPillar[( ConnectionArrayPillar[( i + PBVR_SIDES ) * 3 + j] ) * 3]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[( i + PBVR_SIDES ) * 3 + j] ) * 3 + 1]*mapscale,
                            verticesPillar[( ConnectionArrayPillar[( i + PBVR_SIDES ) * 3 + j] ) * 3 + 2]*mapscale );
            }
            glEnd();
        }

        glDisable( GL_BLEND );
    }
}

#endif //PBVR__V3_HEAD_H_INCLUDE
