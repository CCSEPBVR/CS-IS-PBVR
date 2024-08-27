#ifndef PBVR__ANIMATION_CONTOROL_H_INCLUDE
#define PBVR__ANIMATION_CONTOROL_H_INCLUDE
//#define NOTOPEN
/*
#include <stdio.h>
#include <string.h>
#include <string>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifdef NOTOPEN
#include <stdio.h>
#include <string.h>
#include <string>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <GL/glui.h>
#else
#include "glui.h"
#endif
*/
//#include "ScreenShot.h"

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

#include <iostream>
#include <string>
using namespace std;

#define PBVR_DEBUG

#ifdef PBVR_DEBUG
int     timestep = 0;
#endif

//ボタンの状態
#define PBVR_ON      1
#define PBVR_OFF     0

#define PBVR_CAPTURE         1
#define PBVR_KEYFRAME        1
#define PBVR_IMAGE           0
#define PBVR_NONSUMPLING     2

//デフォルトウィンドウ・サイズ
/* #define PBVR_ANIPBVR_WINDOW_W             270 */
#define PBVR_ANIPBVR_WINDOW_W             300
#define PBVR_ANIPBVR_WINDOW_H             370

//メインウィンドウ・サイズ(テスト用）
#define PBVR_MAINPBVR_WINDOW_W    300
#define PBVR_MAINPBVR_WINDOW_H    300

//ビューポート・サイズ(ウィンドウ・サイズの 1 / 2)
#define PBVR_VIEWPORT_WIDTH  330
#define PBVR_VIEWPORT_HEIGHT 330

#define PBVR_WINH_IMAGE_CAPTURE                  100
#define PBVR_WINH_KEYFRAMEANIME                 240
#define PBVR_WINH_LOG                                109

#include "timer_simple.h"

//テスト用
//---------------------------------

void    Display();
int             AniWin_main;

//---------------------------------

//ファイル保存パス
#ifndef NOTOPEN
//extern        char buff_workdir[256];
#else
//char  buff_workdir[256] = "./";
#endif

// For Close Panel 2016.2.14
extern void CallBackAnimationPanelShow( const int i );

class AnimationControlParameter
{
public:
    int                     m_mode;
    int                     m_capture;
    std::string     m_image_file;
    float           m_sampling_rate;
    std::string     m_log_file;
    int                     m_load;
};

AnimationControlParameter animation_control_parameter;


int             AnimationControlWinID;                                                          //動画生成拡張機能用ウィンドウのID

//GUIパネル用ID
int             AniContSubWinID[3];                                                             //サブウィンドウのID
GLUI*    glui_AniCont[3];                                                                       //gluiサブウィンドウのID
GLUI_StaticText* glui_total_keyf;
GLUI_StaticText* glui_total_anim;

//AnimationControlパネルの表示（本体組み込み時には、これをコールする）
void    AnimationControl( kvs::ScreenBase* screen );

void    ScreenShot(  kvs::ScreenBase* screen, const int tstep );
void    SetAnimationControlParameter();
void    RedrawAnimationControl();

//各種コールバック
void    DisplayAnimationControl();
void    ResizeAnimationControl( const int w, const int h );
void    CallBackAnimationControl( const int num );

//void  ExTransFuncEditFileinit();
void    InitializeAnimationControlWindow( const int winid  );
void    EndAnimationControlWindows( const int winid  );
void    AnimationControlPanel();

//編集可能テキストボックス用コールバック
void    CallBackTextBoxImageFile( const int num );
void    CallBackTextBoxSamplingRate( const int num );
void    CallBackTextBoxLogFile( const int num );
void    CallBackTextBoxKeyFrameFile( const int num );
void    CallBackTextBoxInterpolation( const int num );

//gluiボタン用コールバック
void    CallBackAnimationLoad( const int num );

//リストボックス用コールバック
void    CallBackListBoxImageMode( const int num );
void    CallBackListBoxCapture( const int num );

//glui用ポインタ
GLUI_EditText* tbImageFile;
GLUI_EditText* tbSamplingRate;
GLUI_EditText* tbLogFile;
GLUI_EditText* tbKeyFrameFile;
GLUI_EditText* tbInterpolation;
GLUI_Listbox* lbImageMode;
GLUI_Listbox* lbCapture;

//テキストボックス入力用変数
char            ValueImageFile[256];
float           ValueSamplingRate       = 1.0;
char            ValueLogFile[256] =  "client_log";
char            ValueKeyFrameFile[256] = "./xform.dat";
int             ValueInterpolation = 10;
int             ValueNumKeyFrames = 0;

//リストボックス入力用変数
int             ValueMode                       = 0;
int             ValueCapture            = 0;

int             AniWindowWidth          = PBVR_ANIPBVR_WINDOW_W;                  //現在のウィンドウ・サイズ(幅）
int             AniWindowHeight         = PBVR_ANIPBVR_WINDOW_H;                  //現在のウィンドウ・サイズ(高さ）

kvs::ScreenBase* SSscreen;

//ウィンドウリサイズ用コールバック
void ResizeAnimationControl( const int w, const int h )
{
    AniWindowWidth  = w;
    AniWindowHeight = h;

    InitializeAnimationControlWindow( AnimationControlWinID );
    EndAnimationControlWindows( AnimationControlWinID );
}

void    AnimationControl( kvs::ScreenBase* screen )
{

    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );

    //AnimationControlウィンドウの生成
    glutInitWindowSize( PBVR_ANIPBVR_WINDOW_W, PBVR_ANIPBVR_WINDOW_H );
    glutInitWindowPosition( 1130, 750 );
//      glutInitWindowPosition(1130,360);
//        AnimationControlWinID = glutCreateWindow("Animation Control Panel");
    AnimationControlWinID = glutCreateWindow( "Animation Control Panel" );

    //コールバック関数を登録
    glutDisplayFunc( DisplayAnimationControl );
    glutReshapeFunc( ResizeAnimationControl );

    SetAnimationControlParameter();
    SSscreen = screen;

    //guiパネルの生成
    glutSetWindow( AnimationControlWinID );
    AnimationControlPanel();


#ifndef NOTOPEN
//      glutSetWindow( AnimationControlWinID );
//      glutHideWindow();
#endif

}

#ifdef NOTOPEN
void    exit()
{
    exit( 0 );
}
//テスト用メインウィンドウ--------------------------
int main( int argc, char* argv[] )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );

    glutInitWindowSize( PBVR_MAINPBVR_WINDOW_W, PBVR_MAINPBVR_WINDOW_H );
    glutInitWindowPosition( 300, 300 );
    AniWin_main = glutCreateWindow( "Main Win" );

    glutDisplayFunc( Display );
    glClearColor( 0.0, 0.0, 0.0, 0.0 );

    glutInitWindowPosition( 600, 600 );
    GLUI* gluitest = GLUI_Master.create_glui( "control", 0, 450, 630 );

    AnimationControl();
    gluitest->add_button( "AnimationControl", 0, AniContCallback );
    gluitest->add_button( "Quit", 0, ( GLUI_Update_CB )exit );

    glutMainLoop();
    return 0;
}

//ディスプレイ・ファンクション(テスト用）
void Display()
{

    glutSetWindow( AniWin_main );

    glClear( GL_COLOR_BUFFER_BIT );

    glutSwapBuffers();
    glutPostRedisplay();
}
#endif
//--------------------------------------------------



//サブウィンドウにguiパネルを表示
void    AnimationControlPanel()
{

//---- Image capture -----------
//サブウィンドウ生成
    AniContSubWinID[0] = glutCreateSubWindow( AnimationControlWinID, 0, 0, PBVR_ANIPBVR_WINDOW_W, PBVR_WINH_IMAGE_CAPTURE );              //左上原点
    glui_AniCont[0] = GLUI_Master.create_glui_subwindow( AniContSubWinID[0], GLUI_SUBWINDOW_TOP );
    glutDisplayFunc( DisplayAnimationControl );

//   glui_3vd->set_main_gfx_window( SubWinID );
//   glutDisplayFunc(DisplayControl);

    /* lbImageMode = glui_AniCont[0]->add_listbox( "mode     ", &ValueMode, -1, CallBackListBoxImageMode ); */
    /* lbImageMode->add_item( 0,  "Image" ); */
//        lbImageMode->add_item( 1,  "Keyframe"); /* removed 2014.07.20 */
    /* lbImageMode->set_int_val( ValueMode ); */
    glui_AniCont[0]->add_statictext( "Image capture" );
    glui_AniCont[0]->add_statictext( " " );

//---- capture -----------
    lbImageMode = glui_AniCont[0]->add_listbox( "capture  ", &ValueCapture, -1, CallBackListBoxCapture );
    lbImageMode->add_item( 0,  "off" );
    lbImageMode->add_item( 1,  "on" );
    lbImageMode->set_int_val( ValueCapture );

//---- image file -----------
    tbImageFile = glui_AniCont[0]->add_edittext( "image file   ", GLUI_EDITTEXT_TEXT, ValueImageFile, 1, CallBackTextBoxImageFile );
    tbImageFile->set_alignment( GLUI_ALIGN_LEFT );
    tbImageFile->set_w( 200 );
    glui_AniCont[0]->add_statictext( "(base name)" );

    /*------ KeyFrameAnimation ------*/
    AniContSubWinID[1] = glutCreateSubWindow( AnimationControlWinID, 0, PBVR_WINH_IMAGE_CAPTURE, PBVR_ANIPBVR_WINDOW_W, PBVR_WINH_KEYFRAMEANIME );         //左上原点
    glui_AniCont[1] = GLUI_Master.create_glui_subwindow( AniContSubWinID[1], GLUI_SUBWINDOW_TOP );
    glutDisplayFunc( DisplayAnimationControl );

    glui_AniCont[1]->add_statictext( "Key frame animation" );
    glui_AniCont[1]->add_statictext( " " );

    /* file */
    tbKeyFrameFile = glui_AniCont[1]->add_edittext( "file   ", GLUI_EDITTEXT_TEXT, ValueKeyFrameFile, 1, CallBackTextBoxKeyFrameFile );
    tbKeyFrameFile->set_alignment( GLUI_ALIGN_LEFT );
    tbKeyFrameFile->set_w( 200 );
    /* interpolation */
    tbInterpolation = glui_AniCont[1]->add_edittext( "interpolation   ", GLUI_EDITTEXT_INT, &ValueInterpolation, 1, CallBackTextBoxInterpolation );
    tbInterpolation->set_alignment( GLUI_ALIGN_LEFT );
    tbInterpolation->set_w( 200 );
    /* key frames */
    char tf_buf[256];
    sprintf( tf_buf, "total key frames: %d", ValueNumKeyFrames );
    glui_total_keyf = glui_AniCont[1]->add_statictext( tf_buf );
    /* total frames */
    if ( ValueNumKeyFrames <= 0 )
    {
        sprintf( tf_buf, "total animation frames: 0" );
    }
    else
    {
        sprintf( tf_buf, "total animation frames: %d", ( ValueNumKeyFrames - 1 )*ValueInterpolation );
    }
    glui_total_anim = glui_AniCont[1]->add_statictext( tf_buf );
    /* key assign */
    glui_AniCont[1]->add_statictext( " " );
    glui_AniCont[1]->add_statictext( "(Key assign)" );
    glui_AniCont[1]->add_statictext( "x : capture current view as keyframe" );
    glui_AniCont[1]->add_statictext( "d : delete last keyframe" );
    glui_AniCont[1]->add_statictext( "D : delete all keyframes" );
    glui_AniCont[1]->add_statictext( "M : playback/pause keyframe animation" );
    glui_AniCont[1]->add_statictext( "S : save all keyframes to file" );
    glui_AniCont[1]->add_statictext( "F : load all keyframes from file and playback" );
    /*------ Close Button ------*/
    AniContSubWinID[2] = glutCreateSubWindow( AnimationControlWinID, 0, PBVR_WINH_IMAGE_CAPTURE + PBVR_WINH_KEYFRAMEANIME, PBVR_ANIPBVR_WINDOW_W, 30 );
    glui_AniCont[2] = GLUI_Master.create_glui_subwindow( AniContSubWinID[2], GLUI_SUBWINDOW_TOP );
    glutDisplayFunc( DisplayAnimationControl );
    glui_AniCont[2]->add_button( "Close", 1, CallBackAnimationPanelShow );

// removed sampling, log file, base name, LOAD 2014.07.20 */
#if 0
//---- sampling rate (sec) -----------
    AniContSubWinID[2] = glutCreateSubWindow( AnimationControlWinID, 0, WINH_IMAGEPBVR_CAPTURE + WINH_PBVR_KEYFRAMEANIME, PBVR_WINDOW_W, PBVR_WINH_LOG );              //左上原点
    glui_AniCont[2] = GLUI_Master.create_glui_subwindow( AniContSubWinID[2], GLUI_SUBWINDOW_TOP );
    glutDisplayFunc( DisplayAnimationControl );
    tbImageFile = glui_AniCont[2]->add_edittext( "sampling     ", GLUI_EDITTEXT_FLOAT, &ValueSamplingRate, 1, CallBackTextBoxSamplingRate );
    tbImageFile->set_alignment( GLUI_ALIGN_LEFT );
    tbImageFile->set_w( 80 );
    glui_AniCont[2]->add_statictext( "rate (sec)" );

    tbLogFile = glui_AniCont[2]->add_edittext( "log file        ", GLUI_EDITTEXT_TEXT, ValueLogFile, 1, CallBackTextBoxLogFile );
    tbLogFile->set_alignment( GLUI_ALIGN_LEFT );
    tbLogFile->set_w( 200 );
    glui_AniCont[2]->add_statictext( "(base name)" );

    glui_AniCont[2]->add_button( "LOAD", 0, cbAniLoad );
#endif
//      glui_AniContt[2]->add_column( false );

}

//AnimationControウィンドウのディスプレイ・ファンクション
void DisplayAnimationControl()
{
    glutSetWindow( AnimationControlWinID );
    InitializeAnimationControlWindow( AnimationControlWinID );

    EndAnimationControlWindows( AnimationControlWinID );
}

//AnimationControウィンドウの初期化
void InitializeAnimationControlWindow( const int winid )
{

    glutSetWindow( winid );
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear( GL_COLOR_BUFFER_BIT );

//      glPushMatrix();
}

//AnimationControウィンドウの終了処理
void    EndAnimationControlWindows( const int winid )
{

//      glPopMatrix();
//      glutSetWindow(winid);
    glutSwapBuffers();
//      glutPostRedisplay();
}

//AnimationControボタンコールバック
void    CallBackAnimationControl( const int num )
{
    glutSetWindow( AnimationControlWinID );
    glutShowWindow();
}


//テキストボックス用コールバック
void    CallBackTextBoxImageFile( const int num )
{

    animation_control_parameter.m_image_file  = string( buff_workdir );
    animation_control_parameter.m_image_file += '/';
    animation_control_parameter.m_image_file += string( ValueImageFile );
}

void    CallBackTextBoxSamplingRate( const int num )
{
    animation_control_parameter.m_sampling_rate = ValueSamplingRate;
}

void    CallBackTextBoxLogFile( const int num )
{

    animation_control_parameter.m_log_file  = string( buff_workdir );
    animation_control_parameter.m_log_file += string( ValueLogFile );
    animation_control_parameter.m_log_file += ".log";
}

void    CallBackTextBoxKeyFrameFile( const int num )
{
    return;
}

void    CallBackTextBoxInterpolation( const int num )
{
    RedrawAnimationControl();
    return;
}

//gluiボタン用コールバック
void    CallBackAnimationLoad( const int num )
{
    //if ( animation_control_parameter.m_load == true )
    //{
    //}
}

//リストボックス用コールバック
void    CallBackListBoxImageMode( const int num )
{
    animation_control_parameter.m_mode = ValueMode;

    //時系列動画生成モードなら、LOAD ボタンを無効に
    if ( animation_control_parameter.m_mode == PBVR_IMAGE )
        animation_control_parameter.m_load = false;
}

void    CallBackListBoxCapture( const int num )
{
    animation_control_parameter.m_capture = ValueCapture;

    //m_capture = ONなら、LOAD ボタンを無効に
    if ( animation_control_parameter.m_capture == PBVR_ON )
        animation_control_parameter.m_load = false;

}

void    SetAnimationControlParameter()
{

    animation_control_parameter.m_mode = ValueMode;
    animation_control_parameter.m_capture = ValueCapture;
    animation_control_parameter.m_image_file = string( buff_workdir );
    animation_control_parameter.m_image_file +=  "/";
    animation_control_parameter.m_image_file += string( ValueImageFile );
    animation_control_parameter.m_sampling_rate = ValueSamplingRate;
    animation_control_parameter.m_log_file = string( buff_workdir );
    animation_control_parameter.m_log_file = string( ValueLogFile );
    animation_control_parameter.m_log_file = ".log";
    animation_control_parameter.m_load = false;
}

void    ScreenShot( kvs::ScreenBase* screen, const int tstep )
{

    std::stringstream step;

#ifndef _TIMER_
    if ( ( animation_control_parameter.m_capture == PBVR_ON ) && ( animation_control_parameter.m_mode == PBVR_IMAGE ) )
    {
#endif
        PBVR_TIMER_STA( 160 );

        screen->redraw();

        step << '.' << std::setw( 5 ) << std::setfill( '0' ) << tstep;

#ifdef  PBVR_DEBUG
        std::string filename = ValueImageFile + step.str() + ".bmp";
#else
        std::string filename = AnimationControlParameter.m_image_file + step.str() + ".bmp";
#endif

        kvs::ColorImage image( screen->camera()->snapshot() );
        image.write( filename.c_str() );

#ifdef  PBVR_DEBUG
        std::cout << "Snapshot >>" << filename << std::endl;
        timestep++;
        if ( timestep > 10 )
            timestep = 0;
#endif
        PBVR_TIMER_END( 160 );
#ifndef _TIMER_
    }
#endif
}

void    ScreenShotKeyFrame( kvs::ScreenBase* screen, const int tstep )
{

    std::stringstream step;

#ifndef _TIMER_
    if ( ( animation_control_parameter.m_capture == PBVR_ON ) && ( animation_control_parameter.m_mode == PBVR_IMAGE ) )
    {
#endif
        PBVR_TIMER_STA( 160 );

        screen->redraw();

        step << '.' << std::setw( 6 ) << std::setfill( '0' ) << tstep;

#ifdef  PBVR_DEBUG
        std::string filename = std::string( ValueImageFile ) + "_k" + step.str() + ".bmp";
#else
        std::string filename = AnimationControlParameter.m_image_file + "_k" + step.str() + ".bmp";
#endif

        kvs::ColorImage image( screen->camera()->snapshot() );
        image.write( filename.c_str() );

        PBVR_TIMER_END( 160 );
#ifndef _TIMER_
    }
#endif
}

void RedrawAnimationControl()
{
    /* key frames */
    char tf_buf[256];
    sprintf( tf_buf, "total key frames: %d", ValueNumKeyFrames );
    glui_total_keyf->set_text( tf_buf );
    /* total frames */
    if ( ValueNumKeyFrames <= 0 )
    {
        sprintf( tf_buf, "total animation frames: 0" );
    }
    else
    {
        sprintf( tf_buf, "total animation frames: %d", ( ValueNumKeyFrames - 1 )*ValueInterpolation );
    }
    glui_total_anim->set_text( tf_buf );
}

#endif //PBVR__ANIMATION_CONTOROL_H_INCLUDE
