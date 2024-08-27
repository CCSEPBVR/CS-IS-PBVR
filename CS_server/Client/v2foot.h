#ifndef PBVR__V2FOOT_H_INCLUDE
#define PBVR__V2FOOT_H_INCLUDE

#include <stdio.h>

#include "VizParameterFile.h" // APPEND BY)M.Tanaka 2015.03.03
#include "TransferFunctionEditorMain.h" // APPEND Fj 2015.03.05
#include <kvs/RotationMatrix33>

//#ifdef WIN32
#if 0
#include "kvs/include/Core/Utility/SystemInformation.h"
#else
#include "Core/Utility/SystemInformation.h"
#endif

#ifndef _SC_PHYS_PAGES
#define _SC_PHYS_PAGES 0x0062
#endif

#ifndef _SC_PAGE_SIZE
#define _SC_PAGE_SIZE  0x0028
#endif
// APPEND START FP)K.YAJIMA 2015.03.11
#define PBVR_SIZE 620; //PBVR画面の縦横サイズの初期値はともに620

int g_i_w_v2foot = PBVR_SIZE;
int g_i_h_v2foot = PBVR_SIZE;
// APPEND END FP)K.YAJIMA 2015.03.11

void CallBackTransferFunctionEditorPanelShow( const int i )
{
    if ( ptfe_main )
    {
        if ( i == 1 )
        {
            if ( ptfe_main->getShowTransferFunctionEditor() ) ptfe_main->hide();
            else ptfe_main->show();
        }
    }
}

void CallBackParticlePanelShow( const int i )
{
    if ( i == 1 )
    {
        if ( m_Particle_UI.getShowParticleMergerUI() )
        {
            m_Particle_UI.m_glui_particle_main->hide();
            m_Particle_UI.changeShowParticleMergerUI();
        }
        else
        {
            m_Particle_UI.m_glui_particle_main->show();
            m_Particle_UI.changeShowParticleMergerUI();
        }
    }
}

void CallBackAnimationPanelShow( const int i )
{
    if ( i == 1 )
    {
        glutSetWindow( AnimationControlWinID );
        if ( show_aui ) glutHideWindow();
        else glutShowWindow();
        show_aui = ! show_aui;
    }
}

void CallBackViewerPanelShow( const int i )
{
    if ( m_Viewer_UI.getShowUI() )
    {
        m_Viewer_UI.m_glui_viewer_main->hide();
        m_Viewer_UI.changeShowUI();
    }
    else
    {
        m_Viewer_UI.m_glui_viewer_main->show();
        m_Viewer_UI.changeShowUI();
    }
}

void CallBackLagendPanelShow( const int i )
{
    if ( i == 1 )
    {
        if ( m_Legend_UI.getShowUI() )
        {
            m_Legend_UI.m_glui_legend_main->hide();
            m_Legend_UI.changeShowUI();
        }
        else
        {
            m_Legend_UI.m_glui_legend_main->show();
            m_Legend_UI.changeShowUI();
        }
    }
}

void CallBackCoordinatePanelShow( const int i )
{
    if ( m_Coodinate_UI.getShowUI() )
    {
        m_Coodinate_UI.m_glui_coordinate_main->hide();
        m_Coodinate_UI.changeShowUI();
    }
    else
    {
        m_Coodinate_UI.m_glui_coordinate_main->show();
        m_Coodinate_UI.changeShowUI();
    }
}

void CallBackParticleLimitLevel( const int i )
{

}

void CallBackParticleDensityLevel( const int i )
{

}

void CallBackLookAtEyeX( const int i )
{
}

void CallBackLookAtEyeY( const int i )
{
}

void CallBackLookAtEyeZ( const int i )
{
}

void CallBackLookAtCenterX( const int i )
{
}

void CallBackLookAtCenterY( const int i )
{
}

void CallBackLookAtCenterZ( const int i )
{
}

void CallBackLookAtUpvectorX( const int i )
{
}

void CallBackLookAtUpvectorY( const int i )
{
}

void CallBackLookAtUpvectorZ( const int i )
{
}

// APPEND START FP)K.YAJIMA 2015.03.02
void CallBackResolutionWidth( const int i )
{
}
void CallBackResolutionHeight( const int i )
{
}
// APPEND END FP)K.YAJIMA 2015.03.02

void InitializePanel()
{

    subpixellevela = PBVR_SUB_PIXEL_LEVEL_ABSTRACT;
    subpixelleveld = PBVR_SUB_PIXEL_LEVEL_DETAILED;

    repeatlevela = PBVR_REPEAT_LEVEL_ABSTRACT;
    repeatleveld = PBVR_REPEAT_LEVEL_DETAILED;

    plimitlevel = PBVR_PARTICLE_LIMIT;
    pdensitylevel = PBVR_PARTICLE_DENSITY;

    rotation_x_axis_level = PBVR_ROTATION_X_AXIS_LEVEL;
    rotation_y_axis_level = PBVR_ROTATION_Y_AXIS_LEVEL;
    rotation_z_axis_level = PBVR_ROTATION_Z_AXIS_LEVEL;
    translate_x_level = PBVR_TRANSLATE_X_LEVEL;
    translate_y_level = PBVR_TRANSLATE_Y_LEVEL;
    translate_z_level = PBVR_TRANSLATE_Z_LEVEL;
    scaling_level = PBVR_SCALING_LEVEL;
    lookat_upvectory_level = PBVR_LOOKAT_UPVECTORY_LEVEL;
    lookat_upvectorz_level = PBVR_LOOKAT_UPVECTORZ_LEVEL;
    // APPEND START FP)K.YAJIMA 2015.02.27
    resolution_width_level = PBVR_RESOLUTION_WIDTH_LEVEL;
    resolution_height_level = PBVR_RESOLUTION_HEIGHT_LEVEL;
    // APPEND END FP)K.YAJIMA 2015.02.27

    cxmaxlevel = PBVR_CROP_CXMAX;
    cxminlevel = PBVR_CROP_CXMIN;
    cymaxlevel = PBVR_CROP_CYMAX;
    cyminlevel = PBVR_CROP_CYMIN;
    czmaxlevel = PBVR_CROP_CZMAX;
    czminlevel = PBVR_CROP_CZMIN;

    return;
}

void CallBackNoRepeatSampling( const int i )
{
    /*
    if (m_no_repsampling->get_int_val() == 0)
    {
        start_store_step = -1;
    }
    */
}

void CallBackApply( const int i )
{


// apply change item only
// subpixellevel
    if ( wk_subpixellevela != subpixellevela )
    {
        g_command->m_parameter.m_abstract_subpixel_level = subpixellevela;
        wk_subpixellevela = subpixellevela;
        std::cout << "*** new param.m_abstract_subpixel_level = " << subpixellevela << std::endl;
    }
    if ( wk_subpixelleveld != subpixelleveld )
    {
        g_command->m_parameter.m_detailed_subpixel_level = subpixelleveld;
        wk_subpixelleveld = subpixelleveld;
        std::cout << "*** new param.m_detailed_subpixel_level = " << subpixelleveld << std::endl;
    }
// repeatlevel
    if ( wk_repeatlevela != repeatlevela )
    {
        g_command->m_parameter.m_abstract_repeat_level = repeatlevela;
        wk_repeatlevela = repeatlevela;
        std::cout << "*** new param.m_abstract_repeat_level = " << repeatlevela << std::endl;
    }
    if ( wk_repeatleveld != repeatleveld )
    {
        g_command->m_parameter.m_detailed_repeat_level = repeatleveld;
        wk_repeatleveld = repeatleveld;
        std::cout << "*** new param.m_detailed_repeat_level = " << repeatleveld << std::endl;
    }
// plimitlevel
    if ( wk_plimitlevel != plimitlevel )
    {
        g_command->m_parameter.m_particle_limit = plimitlevel;
        wk_plimitlevel = plimitlevel;
        std::cout << "*** new param.m_particle_limit = " << plimitlevel << std::endl;
    }

// pdensitylevel
    if ( wk_pdensitylevel != pdensitylevel )
    {
        g_command->m_parameter.m_particle_density = pdensitylevel;
        wk_pdensitylevel = pdensitylevel;
        std::cout << "*** new param.m_particle_density = " << pdensitylevel << std::endl;
    }

// m_camera parametervoid CallBackLookAtEyeX(int i) {
//	if ( wk_rotation_x_axis_level != rotation_x_axis_level ) {
//		g_command->param.xxxxx = rotation_x_axis_level;
//		wk_rotation_x_axis_level = rotation_x_axis_level;
//		std::cout<<"*** new param.xxxxx = " << rotation_x_axis_level << std::endl;
//	}

    kvs::Vector3f v3;
    if ( ( wk_translate_x_level != translate_x_level )
            || ( wk_translate_y_level != translate_y_level )
            || ( wk_translate_z_level != translate_z_level ) )
    {
        v3.set( translate_x_level, translate_y_level, translate_z_level );
        wk_translate_x_level = translate_x_level;
        wk_translate_y_level = translate_y_level;
        wk_translate_z_level = translate_z_level;
        std::cout << "*** new param.lookut_center_level = " <<
                  translate_x_level << ", " <<
                  translate_y_level << ", " <<
                  translate_z_level << std::endl;
    }
    if ( ( wk_rotation_x_axis_level != rotation_x_axis_level )
            || ( wk_rotation_y_axis_level != rotation_y_axis_level )
            || ( wk_rotation_z_axis_level != rotation_z_axis_level ) )
    {
        v3.set( rotation_x_axis_level, rotation_y_axis_level, rotation_z_axis_level );
        wk_rotation_x_axis_level = rotation_x_axis_level;
        wk_rotation_y_axis_level = rotation_y_axis_level;
        wk_rotation_z_axis_level = rotation_z_axis_level;
        std::cout << "*** new param.lookut_eye_level = " <<
                  rotation_x_axis_level << ", " <<
                  rotation_y_axis_level << ", " <<
                  rotation_z_axis_level << std::endl;
    }
    if ( ( wk_scaling_level != scaling_level )
            || ( wk_lookat_upvectory_level != lookat_upvectory_level )
            || ( wk_lookat_upvectorz_level != lookat_upvectorz_level ) )
    {
        v3.set( scaling_level, lookat_upvectory_level, lookat_upvectorz_level );
        wk_scaling_level = scaling_level;
        wk_lookat_upvectory_level = lookat_upvectory_level;
        wk_lookat_upvectorz_level = lookat_upvectorz_level;
        std::cout << "*** new param.lookut_upvector_level = " <<
                  scaling_level << ", " <<
                  lookat_upvectory_level << ", " <<
                  lookat_upvectorz_level << std::endl;
    }
    g_screen->reset(); //camera()->resetXform();
    g_screen->objectManager()->translate( kvs::Vector3f(
                                      translate_x_level,
                                      translate_y_level,
                                      translate_z_level) );
    g_screen->objectManager()->rotate( kvs::RPYRotationMatrix33<float>(
                                         rotation_z_axis_level,
                                         rotation_y_axis_level,
                                         rotation_x_axis_level) );
    float s = scaling_level;
    g_screen->objectManager()->scale( kvs::Vector3f( s, s, s ) );

// APPEND START FP)K.YAJIMA & M.Tanaka 2015.03.03
    if ( wk_resolution_width_level != resolution_width_level )
    {
        g_command->m_parameter.m_resolution_width = resolution_width_level;
        wk_resolution_width_level = resolution_width_level;
        std::cout << "*** new param.m_resolution_width = " << resolution_width_level << std::endl;
    }
    if ( wk_resolution_height_level != resolution_height_level )
    {
        g_command->m_parameter.m_resolution_height = resolution_height_level;
        wk_resolution_height_level = resolution_height_level;
        std::cout << "*** new param.m_resolution_height = " << resolution_height_level << std::endl;
    }
// DELETE START FP)K.YAJIMA 2015.03.11
    //AnimationControl( g_screen );
// DELETE END FP)K.YAJIMA 2015.03.11
// DELETE START FP)K.YAJIMA 2015.03.17
//      g_screen->redraw();
//      g_screen->hide();
// DELETE END FP)K.YAJIMA 2015.03.17

// PBVR画面の大きさを動的に変化させる処理
// MODIFY START FP)K.YAJIMA 2015.03.11
    //int i_w = (int)((long)resolution_width_level);
    //int i_h = (int)((long)resolution_height_level);
    //g_screen->setGeometry( 0, 0, i_w, i_h);

    int i_w = ( int )( ( long )resolution_width_level );
    int i_h = ( int )( ( long )resolution_height_level );
#if 0
    if ( i_h > resolution_height_device - PBVR_DOCK_H )
    {
        i_h = resolution_height_device - PBVR_DOCK_H;
        resolution_height_level = ( float )i_h;
    }
#endif
// DELETE START FP)K.YAJIMA 2015.03.17
//      if(g_i_w_v2foot == i_w && g_i_h_v2foot == i_h)
//      {
//              printf("何もしない\n");
//              printf("g_i_w_v2foot = %d , g_i_h_v2foot = %d\n",g_i_w_v2foot,g_i_h_v2foot);
//              printf("i_w = %d , i_h = %d\n",i_w,i_h);
//              //何もしない
//              //画面の幅および高さの値が現在と同じ場合は動的に変化させない
//      }
//      else
//      {
//              printf("動的に変化させる\n");
//              printf("g_i_w_v2foot = %d , g_i_h_v2foot = %d\n",g_i_w_v2foot,g_i_h_v2foot);
//              printf("i_w = %d , i_h = %d\n",i_w,i_h);
//              //動的に変化させる
//              g_screen->setGeometry( 0, 0, i_w, i_h);
//      }
// DELETE END FP)K.YAJIMA 2015.03.17

// APPEND START FP)K.YAJIMA 2015.03.17
    if ( g_i_w_v2foot != i_w || g_i_h_v2foot != i_h )
    {
        // 幅、高さのどちらかが現解像度と異なった場合
        // hide(),show()必須

        g_screen->hide();

        printf( "動的に変化させる\n" );
        printf( "g_i_w_v2foot = %d , g_i_h_v2foot = %d\n", g_i_w_v2foot, g_i_h_v2foot );
        printf( "i_w = %d , i_h = %d\n", i_w, i_h );
        //動的に変化させる

        int x_0 = glutGet( ( GLenum )GLUT_WINDOW_X );
        int y_0 = glutGet( ( GLenum )GLUT_WINDOW_Y );
//#if not defined(WIN32) && not defined(__APPLE__)
#if !defined(WIN32) && !defined(__APPLE__)
		y_0 = y_0 - 28;
#endif
        g_screen->setGeometry( x_0, y_0, i_w, i_h );

        /* 以下3文は、Viewerの可視パラメータである。
           　解像度変更に合わせて正しい位置に表示できるように計算している */
//	    g_label->setPosition(i_w * 1 / 31, i_h * 55 / 62); // time step
//	    g_fpsLabel->setPosition(i_w * 51 / 124, i_h * 55 / 62); // fps
//	    g_orientation_axis->setPosition(i_w * 53 / 62 - 30, i_h *53 / 62 - 30); // 座標軸

        int yl0 = i_h - PBVR_I_H_OFFSET;
        g_label->setPosition( i_w * 1 / 31, yl0 ); // time step
        g_fpsLabel->setPosition( i_w * 51 / 124, yl0 ); // fps
        g_orientation_axis->setPosition( i_w * 53 / 62 - 30, yl0 - 30 ); // 座標軸
        g_screen->show();
    }
// APPEND END FP)K.YAJIMA 2015.03.17

    g_i_w_v2foot = i_w;
    g_i_h_v2foot = i_h;

// MODIFY END FP)K.YAJIMA 2015.03.11

// DELETE START FP)K.YAJIMA 2015.03.17
//      g_label->setPosition(i_w * 1 / 31, i_h * 55 / 62);
//      g_fpsLabel->setPosition(i_w * 51 / 124, i_h * 55 / 62);
//      g_orientation_axis->setPosition(i_w * 53 / 62 - 30, i_h *53 / 62 - 30);
//
//      g_screen->show();
// DELETE END   FP)K.YAJIMA 2015.03.17
// APPEND END   FP)K.YAJIMA & M.Tanaka 2015.03.03

    /* change pfi file */
    if (g_command->m_parameter.m_hasfin) {
        if ( strlen( g_volumedata->get_text() ) > 0 &&
                strcmp( filter_parameter_filename, g_volumedata->get_text() ) != 0 )
        {
            sprintf( filter_parameter_filename, "%s", g_volumedata->get_text() );
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
        if ( strlen( g_volumedata->get_text() ) > 0 &&
                strcmp( pfi_path_server, g_volumedata->get_text() ) != 0 )
        {
            sprintf( pfi_path_server, "%s", g_volumedata->get_text() );
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
        strcpy( g_command->param.xxxxx, volumedata );
        wk_volumedata = volumedata;
        std::cout << "*** new param.xxxxx = " << volumedata << std::endl;
    }
    if ( wk_cxmaxlevel != cxmaxlevel )
    {
        g_command->param.xxxxx = cxmaxlevel;
        wk_cxmaxlevel = cxmaxlevel;
        std::cout << "*** new param.xxxxx = " << cxmaxlevel << std::endl;
    }
    if ( wk_cxminlevel != cxminlevel )
    {
        g_command->param.xxxxx = cxminlevel;
        wk_cxminlevel = cxminlevel;
        std::cout << "*** new param.xxxxx = " << cxminlevel << std::endl;
    }
    if ( wk_cymaxlevel != cymaxlevel )
    {
        g_command->param.xxxxx = cymaxlevel;
        wk_cymaxlevel = cymaxlevel;
        std::cout << "*** new param.xxxxx = " << cymaxlevel << std::endl;
    }
    if ( wk_cyminlevel != cyminlevel )
    {
        g_command->param.xxxxx = cyminlevel;
        wk_cyminlevel = cyminlevel;
        std::cout << "*** new param.xxxxx = " << cyminlevel << std::endl;
    }
    if ( wk_czmaxlevel != czmaxlevel )
    {
        g_command->param.xxxxx = czmaxlevel;
        wk_czmaxlevel = czmaxlevel;
        std::cout << "*** new param.xxxxx = " << czmaxlevel << std::endl;
    }
    if ( wk_czminlevel != czminlevel )
    {
        g_command->param.xxxxx = czminlevel;
        wk_czminlevel = czminlevel;
        std::cout << "*** new param.xxxxx = " << czminlevel << std::endl;
    }

    if ( wk_translate_x_level != translate_x_level )
    {
        g_command->param.xxxxx = translate_x_level;
        wk_translate_x_level = translate_x_level;
        std::cout << "*** new param.xxxxx = " << translate_x_level << std::endl;
    }
    // APPEND START FP)K.YAJIMA 2015.02.27
    if ( wk_resolution_width_level != resolution_width_level )
    {
        g_command->param.xxxxx = resolution_width_level;
        wk_resolution_width_level = resolution_width_level;
        std::cout << "*** new param.xxxxx = " << resolution_width_level << std::endl;
    }
    if ( wk_resolution_height_level != resolution_height_level )
    {
        g_command->param.xxxxx = resolution_height_level;
        wk_resolution_height_level = resolution_height_level;
        std::cout << "*** new param.xxxxx = " << resolution_height_level << std::endl;
    }
    // APPEND END FP)K.YAJIMA 2015.02.27
#endif

    g_command->m_parameter.m_no_repsampling = m_no_repsampling->get_int_val();
    g_command->m_glut_timer->setInterval( interval_msec->get_int_val() );

    return;
}

//
// write parameter to file
//
void CallBackParameterFile( const int i )
{

// APPEND START FP)M.TANAKA 2015.03.16
    void ConversionClassToFloat();

    ConversionClassToFloat();
// APPEND END   FP)M.TANAKA 2015.03.16

    VizParameterFile::WriteParamFile( buff_paramfile ); // APPEND BY)M.Tanaka 2015.03.03
    ptfe_main->exportFile( buff_paramfile, true ); // APPEND Fj 2015.03.05
    m_Legend_UI.exportFile( buff_paramfile );
}

//
// read paramter from file
//
void CallBackReadFile( const int i )
{
// APPEND START FP)M.TANAKA 2015.03.16
    void ConversionFloatToClass();
// APPEND END   FP)M.TANAKA 2015.03.16

    VizParameterFile::ReadParamFile( buff_paramfile ); // APPEND BY)M.Tanaka 2015.03.03
    ptfe_main->importFile( buff_paramfile ); // APPEND Fj 2015.03.05
    //ptfe_main->functionName(); // APPEND Fj 2015.04.21 for UI(Color) update  // delete by @hira at 2016/12/01
    m_Coodinate_UI.setUISynthesizer();
    m_Viewer_UI.applyFontType();
    m_Viewer_UI.applyBackgroundColor();
    m_Legend_UI.inportFile( buff_paramfile );
    m_Legend_UI.set2UI();

// APPEND START FP)M.TANAKA 2015.03.16
    ConversionFloatToClass();
// APPEND END   FP)M.TANAKA 2015.03.16

}

// APPEND START FP)M.TANAKA 2015.03.16
void ConversionClassToFloat()
{

    /* kvs::ValueArray<kvs::Real32> m_coords(3); */
    /* kvs::RGBColor m_color(0,0,0); */
    /* kvs::ValueArray<kvs::Real32> m_normals(3); */
    /* m_normals[0] = 1.0f; */
    /* m_normals[1] = 0.0f; */
    /* m_normals[2] = 0.0f; */

//        g_screen->registerObject(out_object, g_command->m_renderer);
    /*
        kvs::Xform outxform = out_object->xform();
        kvs::Matrix33f rotation_out( outxform.rotation() );
        kvs::Vector3f translation_out( outxform.translation() );
        kvs::Vector3f scaling_out( outxform.scaling() );
            delete out_object;
    */
    /* kvs::Xform outxform = g_control_object->xform(); */
    kvs::PointObject* object = NULL;
    object = g_timer_event->getPointObject();
    if ( object )
    {
        kvs::Xform outxform = object->xform();
        kvs::Matrix33f rotation_out( outxform.rotation() );
        kvs::Vector3f translation_out( outxform.translation() );
        kvs::Vector3f scaling_out( outxform.scaling() );

        g_ro[0][0] = rotation_out[0][0];
        g_ro[0][1] = rotation_out[0][1];
        g_ro[0][2] = rotation_out[0][2];
        g_ro[1][0] = rotation_out[1][0];
        g_ro[1][1] = rotation_out[1][1];
        g_ro[1][2] = rotation_out[1][2];
        g_ro[2][0] = rotation_out[2][0];
        g_ro[2][1] = rotation_out[2][1];
        g_ro[2][2] = rotation_out[2][2];

        g_tr[0] = translation_out[0];
        g_tr[1] = translation_out[1];
        g_tr[2] = translation_out[2];

        g_sc[0] = scaling_out[0];
        g_sc[1] = scaling_out[1];
        g_sc[2] = scaling_out[2];
    }
    else
    {
        g_ro[0][0] = 1.0;
        g_ro[0][1] = 0.0;
        g_ro[0][2] = 0.0;
        g_ro[1][0] = 0.0;
        g_ro[1][1] = 1.0;
        g_ro[1][2] = 0.0;
        g_ro[2][0] = 0.0;
        g_ro[2][1] = 0.0;
        g_ro[2][2] = 1.0;

        g_tr[0] = 0.0;
        g_tr[1] = 0.0;
        g_tr[2] = 0.0;

        g_sc[0] = 1.0;
        g_sc[1] = 1.0;
        g_sc[2] = 1.0;
    }
}

void ConversionFloatToClass()
{
    printf( "r :%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
            g_ro[0][0], g_ro[0][1], g_ro[0][2],
            g_ro[1][0], g_ro[1][1], g_ro[1][2],
            g_ro[2][0], g_ro[2][1], g_ro[2][2] );
    printf( "s :%f,%f,%f\n", g_sc[0], g_sc[1], g_sc[2] );
    printf( "t :%f,%f,%f\n", g_tr[0], g_tr[1], g_tr[2] );

    kvs::Vector3f translation_in( g_tr );
    kvs::Vector3f scaling_in( g_sc );
    float rr[9] = {g_ro[0][0], g_ro[0][1], g_ro[0][2],
                   g_ro[1][0], g_ro[1][1], g_ro[1][2],
                   g_ro[2][0], g_ro[2][1], g_ro[2][2]
                  };
    kvs::Matrix33f rotation_in( rr );
    kvs::Xform readxform( translation_in, scaling_in, rotation_in );
    /* g_control_object->setXform(readxform); */
    kvs::PointObject* object = g_timer_event->getPointObject();
    if ( object )
    {
        object->setXform( readxform );
        g_screen->redraw();
    }

}

// APPEND END   FP)M.TANAKA 2015.03.16

//
// MCWP[^XV
// SENDING  ... **********");
//
static int sending_cnt = 0;
void UpdateSending()
{
    char buff[512];

    buff[0] = '\0';
    char msg[64] = "SENDING ... \0";
    strcat( buff,  msg );
    for ( int i = 0; i < sending_cnt; i++ )
    {
        strcat( buff, "*" );
    }
    lbl_sending->name = buff;

    sending_cnt++;
    if ( sending_cnt > 19 )  sending_cnt = 0;

    lbl_sending->redraw();

    return;
}

//
// óMCWP[^XV
// RECIEVING  ... **********");
//
static int recieving_cnt = 0;
void UpdateRecieving()
{
    char buff[512];

    buff[0] = '\0';
    char msg[64] = "RECIEVING ... \0";
    strcat( buff,  msg );
    for ( int i = 0; i < recieving_cnt; i++ )
    {
        strcat( buff, "*" );
    }
    lbl_recieving->name = buff;

    recieving_cnt++;
    if ( recieving_cnt > 19 )  recieving_cnt = 0;

    lbl_recieving->redraw();

    return;
}
#if 0
//
// CPUXV
// CPU MEMORY 2042/2097 97%
//
void UpdateCPUMemory::run()
{

#ifdef NOTOPEN
    char buff[512];
    char buf[256];
    while ( 1 )
    {
        mutex->lock();
        fprintf( stdout, "called UpdateCPUMemory\n" );

        int t = kvs::SystemInformation::totalMemorySize() / 1024;
        int c = kvs::SystemInformation::freeMemorySize() / 1024;

        c = t - c;
        int p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
        mutex->unlock();
    }
#endif

#ifdef WIN32

    char buff[512];
    int c;
    int t;
    while ( 1 )
    {
        mutex->lock();
        MEMORYSTATUS stat;
        GlobalMemoryStatus ( &stat );

        t = ( stat.dwTotalPhys / 1024 / 1024 );
        c = ( stat.dwAvailPhys / 1024 / 1024 );
        c = t - c;

        int p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
        mutex->unlock();
    }
#else

//#ifdef MAC
#ifdef __APPLE__
    char* cmdline1 = "vm_stat |grep 'Pages free:' |cut -c36-42";
    char* cmdline2 = "sysctl -a |grep 'hw.memsize:' |cut -c12-19";
    FILE* fp;
    char buff[512];
    char buf[256];
    int c;
    int t;
    while ( 1 )
    {
        mutex->lock();
        fp = popen( cmdline1, "r" );
        fgets( buf, 256, fp );
        pclose( fp );

        fprintf( stdout, "Free  MEMORY : %s", buf );

        sscanf( buf, "%d", &c );

        fp = popen( cmdline2, "r" );
        fgets( buf, 256, fp );
        pclose( fp );

        fprintf( stdout, "Total MEMORY : %s", buf );

        sscanf( buf, "%d", &t );

        c = c * 4 / 1024;
        t = t / 1024;
        c = t - c;
        int p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
        mutex->unlock();
    }
#else
    char* cmdline = "free |grep '\\-\\/+ buff' |sed 's/\\s\\{1,\\}/ /g' | cut -d' ' -f4";
    FILE* fp;
    char buff[512];
    char buf[256];
    GLint c;
    GLint t;
    while ( 1 )
    {
        mutex->lock();
        fp = popen( cmdline, "r" );
        fgets( buf, 256, fp );
        pclose( fp );

        fprintf( stdout, "Free MEMORY : %s", buf );

        sscanf( buf, "%d", &c );

        long pages = sysconf( _SC_PHYS_PAGES );
        long page_size = sysconf( PBVR_SC_PAGE_SIZE );

        t = pages * page_size / 1024;
        fprintf( stdout, "Total MEMORY : %ld\n", t );

        c = t - c;
        GLint p = c * 100 / t;

        sprintf( buff, "CPU MEMORY %d/%d %d%%", c, t, p );
        lbl_smemory->name = buff;
        mutex->unlock();
    }
#endif
#endif

}
#endif
//
// GPUXV
// GPU MEMORY 2042/2097 97%
//
void UpdateGPUMemory()
{
    char buff[512];
#ifndef CPUMODE
    int availableKB[] = { -1, -1, -1, -1 };

    bool f = false;
    if ( GLEW_NVX_gpu_memory_info )
    {
        GLint c;
        GLint t;
        glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableKB[0] );
//		fprintf(stdout, "NVidia card : %d %d %d %d\n", availableKB[0], availableKB[1], availableKB[2], availableKB[3]);
        c = availableKB[0] / 1024;
        glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &availableKB[0] );
//		fprintf(stdout, "NVidia card : %d %d %d %d\n", availableKB[0], availableKB[1], availableKB[2], availableKB[3]);
        t = availableKB[0] / 1024;
        GLint p = c * 100 / t;

        sprintf( buff, "GPU MEMORY %d/%d %d%%", c, t, p );
        lbl_gmemory->name = buff;
        //lbl_gmemory->redraw();

        f = true;
    }
    int temp = GLEW_ATI_meminfo;
    if ( GLEW_ATI_meminfo )
    {
        GLint c;
        GLint t;

#ifdef ATI
        GLuint uNoOfGPUs = wglGetGPUIDsAMD( 0, 0 );
        GLuint* uGPUIDs = new GLuint[uNoOfGPUs];
        wglGetGPUIDsAMD( uNoOfGPUs, uGPUIDs );

        GLuint uTotalMemoryInMB = 0;
        wglGetGPUInfoAMD( uGPUIDs[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof( GLuint ), &uTotalMemoryInMB );
//		t = uTotalMemoryInMB * 1024;
        t = uTotalMemoryInMB;
#endif

        glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, availableKB );
        fprintf( stdout, "ATI card\n" );

        c = availableKB[0] / 1024;
#ifdef ATI
        GLint p = c * 100 / t;
        sprintf( buff, "GPU MEMORY %d/%d %d%%", c, t, p );
#else
        sprintf( buff, "GPU MEMORY %d/- -\%%", c );
#endif
        lbl_gmemory->name = buff;
        //lbl_gmemory->redraw();

        f = true;
    }
    if ( !f )
    {
//		fprintf(stdout, "no card\n");
        sprintf( buff, "GPU MEMORY -/- -\%%" );

        lbl_gmemory->name = buff;
    }
#else
//	fprintf(stdout, "no card\n");
	sprintf(buff, "GPU MEMORY -/- -%%");

    lbl_gmemory->name = buff;
#endif
    return;
}

#endif //PBVR__V2FOOT_H_INCLUDE
