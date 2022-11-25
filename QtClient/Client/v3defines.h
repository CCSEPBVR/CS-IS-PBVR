#ifndef V3DEFINES_H
#define V3DEFINES_H

#define PBVR_SIZE 330 //PBVR画面の縦横サイズの初期値はともに620


// V2 Defines below

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
#define PBVR_PARTICLE_DATA_SIZE_LIMIT 20.0
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
#define PBVR_NOCROP          0


#define PBVR_DATAFILE "data"  // /tmp/data_20140311101010.dat
#define PBVR_LIMIT_DATA_FILE "limit" // /tmp/limit.dat
#define PBVR_DOCK_H 40  // for right or left Dock of Mac and Linux
//#define DOCK_H 132 // for bottom Dock of Mac
#define PBVR_I_H_OFFSET 55


//  V3 Defines below

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
#define	PBVR_MAINPBVR_WINDOW_W	660
#define	PBVR_MAINPBVR_WINDOW_H	660

//デフォルトウィンドウ・サイズ
#define	PBVR_WINDOW_W		660
#define	PBVR_WINDOW_H		660

//ビューポート・サイズ(ウィンドウ・サイズの 1 / 2)
#define	PBVR_VIEWPORT_WIDTH     330
#define	PBVR_VIEWPORT_HEIGHT    330

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

#endif // V3DEFINES_H
