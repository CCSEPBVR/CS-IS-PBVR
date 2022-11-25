//
// VizParameterFile class
//
// Created by FP)M.tanaka 2015.03.03
//

#include "VizParameterFile.h"
extern GLUI_EditText* g_volumedata;    /* for PFI file */


// add by @hira at 2016/12/01
#define PBVR_FILTER_PARAMFILENAME    "FILTER_PARAMETER_FILENAME"

// Read ParameterFile
int VizParameterFile::ReadParamFile( const char* fname, const int readflag )
{

    char s[256], *p;
    char k[100][256];
    char v[100][256];
    int cnt = 0;

    FILE* fp;
    if ( ( fp = fopen( fname, "r" ) ) == NULL )
    {
        return -1;
    }
    cnt = 0;
    while ( fgets( s, 256, fp ) != NULL )
    {
        //printf("%s", s);

        p = strstr( s, " = " );
        if ( p == 0 )
        {
            continue;
        }
        *p = '\0';
        strcpy( k[cnt], s );
        strcpy( v[cnt], p + 3 );

        p = strrchr( v[cnt], '\r' );
        if ( p != 0 )  *p = '\0';
        p = strrchr( v[cnt], '\n' );
        if ( p != 0 )  *p = '\0';

        printf( " ==> %s=%s\n", k[cnt], v[cnt] );
        cnt++;
    }
    fclose( fp );

    // set val to internal variable and textbox
    for ( int i = 0; i < cnt; i++ )
    {
//		if (strcmp(k[i], "PBVR_SUB_PIXEL_LEVEL_DETAILED") == 0) {
        if ( strcmp( k[i], "SUB_PIXEL_LEVEL" ) == 0 )
        {
            subpixelleveld = atoi( v[i] );
            if ( subpixeld != NULL )
                subpixeld->set_text( v[i] );
        }
//		if (strcmp(k[i], "PBVR_REPEAT_LEVEL_DETAILED") == 0) {
        if ( strcmp( k[i], "REPEAT_LEVEL" ) == 0 )
        {
            repeatleveld = atoi( v[i] );
            if ( repeatd != NULL )
                repeatd->set_text( v[i] );
        }
        if ( strcmp( k[i], "PBVR_PARTICLE_LIMIT" ) == 0 )
        {
            plimitlevel = atoi( v[i] );
            if ( plimit != NULL )
                plimit->set_text( v[i] );
            plimit->deactivate();
        }
        if ( strcmp( k[i], "SAMPLING_TYPE" ) == 0 )
        {
            samplingtypelevel = atoi( v[i] );
        }
        if ( strcmp( k[i], "ROTATION_X_AXIS" ) == 0 )
        {
            rotation_x_axis_level = atof( v[i] );
            if ( rotation_x_axis != NULL )
                rotation_x_axis->set_text( v[i] );
            rotation_x_axis->deactivate();
        }
        if ( strcmp( k[i], "ROTATION_Y_AXIS" ) == 0 )
        {
            rotation_y_axis_level = atof( v[i] );
            if ( rotation_y_axis != NULL )
                rotation_y_axis->set_text( v[i] );
            rotation_y_axis->deactivate();
        }
        if ( strcmp( k[i], "ROTATION_Z_AXIS" ) == 0 )
        {
            rotation_z_axis_level = atof( v[i] );
            if ( rotation_z_axis != NULL )
                rotation_z_axis->set_text( v[i] );
            rotation_z_axis->deactivate();
        }
        if ( strcmp( k[i], "TRANSLATION_X" ) == 0 )
        {
            translate_x_level = atof( v[i] );
            if ( translate_x != NULL )
                translate_x->set_text( v[i] );
            translate_x->deactivate();
        }
        if ( strcmp( k[i], "TRANSLATION_Y" ) == 0 )
        {
            translate_y_level = atof( v[i] );
            if ( translate_y != NULL )
                translate_y->set_text( v[i] );
            translate_y->deactivate();
        }
        if ( strcmp( k[i], "TRANSLATION_Z" ) == 0 )
        {
            translate_z_level = atof( v[i] );
            if ( translate_z != NULL )
                translate_z->set_text( v[i] );
            translate_z->deactivate();
        }
        if ( strcmp( k[i], "SCALING" ) == 0 )
        {
            scaling_level = atof( v[i] );
            if ( scaling != NULL )
                scaling->set_text( v[i] );
            scaling->deactivate();
        }
        //if ( strcmp( k[i], "UP_VECTOR_Y" ) == 0 )
        //{
        //    lookat_upvectory_level = atof( v[i] );
        //    if ( lookat_upvectory != NULL )
        //        lookat_upvectory->set_text( v[i] );
        //}
        //if ( strcmp( k[i], "UP_VECTOR_Z" ) == 0 )
        //{
        //    lookat_upvectorz_level = atof( v[i] );
        //    if ( lookat_upvectorz != NULL )
        //        lookat_upvectorz->set_text( v[i] );
        //}
        if ( strcmp( k[i], "RESOLUTION_WIDTH" ) == 0 )
        {
            resolution_width_level = atof( v[i] );
            if ( resolution_width != NULL )
                resolution_width->set_text( v[i] );
            resolution_width->deactivate();
        }
        if ( strcmp( k[i], "RESOLUTION_HEIGHT" ) == 0 )
        {
            resolution_height_level = atof( v[i] );
            if ( resolution_height != NULL )
                resolution_height->set_text( v[i] );
            resolution_height->deactivate();
        }
        // delete by @hira at 2016/12/01
        //if ( strcmp( k[i], "PFI_PATH_SERVER" ) == 0 )
        //{
        //    strcpy( pfi_path_server, v[i] );
        //    g_volumedata->set_text( pfi_path_server );
        //}
        // add by @hira at 2016/12/01
        if ( strcmp( k[i], PBVR_FILTER_PARAMFILENAME ) == 0 )
        {
            strcpy( filter_parameter_filename, v[i] );
            g_volumedata->set_text( v[i] );
            g_volumedata->deactivate();
        }


//#ifndef __APPLE__
        if ( readflag == 1 )
        {
            if ( strcmp( k[i], "CROP_CXMIN" ) == 0 )
            {
                tbcxminlevel = atof( v[i] );
                glui_3vd->sync_live();
                CallBackCropMinLevelX( 0 );
            }
            if ( strcmp( k[i], "CROP_CXMAX" ) == 0 )
            {
                tbcxmaxlevel = atof( v[i] );
                glui_3vd->sync_live();
                CallBackCropMaxLevelX( 0 );
            }
            if ( strcmp( k[i], "CROP_CYMIN" ) == 0 )
            {
                tbcyminlevel = atof( v[i] );
                glui_3vd->sync_live();
                CallBackCropMinLevelY( 0 );
            }
            if ( strcmp( k[i], "CROP_CYMAX" ) == 0 )
            {
                tbcymaxlevel = atof( v[i] );
                glui_3vd->sync_live();
                CallBackCropMaxLevelY( 0 );
            }
            if ( strcmp( k[i], "CROP_CZMIN" ) == 0 )
            {
                tbczminlevel = atof( v[i] );
                glui_3vd->sync_live();
                CallBackCropMinLevelZ( 0 );
            }
            if ( strcmp( k[i], "CROP_CZMAX" ) == 0 )
            {
                tbczmaxlevel = atof( v[i] );
                glui_3vd->sync_live();
                CallBackCropMaxLevelZ( 0 );
            }

            if ( strcmp( k[i], "CROP_PBVR_SRADIUS" ) == 0 )
            {
                tbsradius = atof( v[i] );
                glui_3vd->sync_live();
                CallBackSphereRadius( 0 );
            }
            if ( strcmp( k[i], "CROP_SCENTERX" ) == 0 )
            {
                tbscenterx = atof( v[i] );
                glui_3vd->sync_live();
                CallBackSphereCenterX( 0 );
            }
            if ( strcmp( k[i], "CROP_SCENTERY" ) == 0 )
            {
                tbscentery = atof( v[i] );
                glui_3vd->sync_live();
                CallBackSphereCenterY( 0 );
            }
            if ( strcmp( k[i], "CROP_SCENTERZ" ) == 0 )
            {
                tbscenterz = atof( v[i] );
                glui_3vd->sync_live();
                CallBackSphereCenterZ( 0 );
            }

            if ( strcmp( k[i], "CROP_PBVR_PRADIUS" ) == 0 )
            {
                tbpradius = atof( v[i] );
                glui_3vd->sync_live();
                CallBackPillarRadius( 0 );
            }
            if ( strcmp( k[i], "CROP_PBVR_PHEIGHT" ) == 0 )
            {
                tbpheight = atof( v[i] );
                glui_3vd->sync_live();
                CallBackPillarHeight( 0 );
            }
            if ( strcmp( k[i], "CROP_PCENTERX" ) == 0 )
            {
                tbpcenterx = atof( v[i] );
                glui_3vd->sync_live();
                CallBackPillarCenterX( 0 );
            }
            if ( strcmp( k[i], "CROP_PCENTERY" ) == 0 )
            {
                tbpcentery = atof( v[i] );
                glui_3vd->sync_live();
                CallBackPillarCenterY( 0 );
            }
            if ( strcmp( k[i], "CROP_PCENTERZ" ) == 0 )
            {
                tbpcenterz = atof( v[i] );
                glui_3vd->sync_live();
                CallBackPillarCenterZ( 0 );
            }
        }
//#endif
// APPEND START FP)M.TANAKA 2015.03.16
        if ( strcmp( k[i], "ROTATION1X" ) == 0 )
        {
            g_ro[0][0] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION1Y" ) == 0 )
        {
            g_ro[0][1] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION1Z" ) == 0 )
        {
            g_ro[0][2] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION2X" ) == 0 )
        {
            g_ro[1][0] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION2Y" ) == 0 )
        {
            g_ro[1][1] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION2Z" ) == 0 )
        {
            g_ro[1][2] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION3X" ) == 0 )
        {
            g_ro[2][0] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION3Y" ) == 0 )
        {
            g_ro[2][1] = atof( v[i] );
        }
        if ( strcmp( k[i], "ROTATION3Z" ) == 0 )
        {
            g_ro[2][2] = atof( v[i] );
        }
        if ( strcmp( k[i], "TRANSLATIONX" ) == 0 )
        {
            g_tr[0] = atof( v[i] );
        }
        if ( strcmp( k[i], "TRANSLATIONY" ) == 0 )
        {
            g_tr[1] = atof( v[i] );
        }
        if ( strcmp( k[i], "TRANSLATIONZ" ) == 0 )
        {
            g_tr[2] = atof( v[i] );
        }
        if ( strcmp( k[i], "SCALINGX" ) == 0 )
        {
            g_sc[0] = atof( v[i] );
        }
        if ( strcmp( k[i], "SCALINGY" ) == 0 )
        {
            g_sc[1] = atof( v[i] );
        }
        if ( strcmp( k[i], "SCALINGZ" ) == 0 )
        {
            g_sc[2] = atof( v[i] );
        }
// APPEND END   FP)M.TANAKA 2015.03.16
        if ( strcmp( k[i], "COORD1_SYNTH" ) == 0 )
        {
            param_x_synthesis = std::string( v[i] );
        }
        if ( strcmp( k[i], "COORD2_SYNTH" ) == 0 )
        {
            param_y_synthesis = std::string( v[i] );
        }
        if ( strcmp( k[i], "COORD3_SYNTH" ) == 0 )
        {
            param_z_synthesis = std::string( v[i] );
        }
        if ( strcmp( k[i], "FONTTYPE" ) == 0 )
        {
            param_fonttype = std::string( v[i] );
        }
        if ( strcmp( k[i], "BACKGROUND_COLOR_R" ) == 0 )
        {
            kvs::UInt8 r = atoi( v[i] );
            kvs::RGBColor tmp( r, crgb.g(), crgb.b() );
            crgb = tmp;
        }
        if ( strcmp( k[i], "BACKGROUND_COLOR_G" ) == 0 )
        {
            kvs::UInt8 g = atoi( v[i] );
            kvs::RGBColor tmp( crgb.r(), g, crgb.b() );
            crgb = tmp;
        }
        if ( strcmp( k[i], "BACKGROUND_COLOR_B" ) == 0 )
        {
            kvs::UInt8 b = atoi( v[i] );
            kvs::RGBColor tmp( crgb.r(), crgb.g(), b );
            crgb = tmp;
        }

        GLUI_Master.sync_live_all();
    }

    return 0;
}

int VizParameterFile::UpdateWin3v( const char* fname )
{
    char s[256], *p;
    char k[100][256];
    char v[100][256];
    int cnt = 0;

    FILE* fp;
    if ( ( fp = fopen( fname, "r" ) ) == NULL )
    {
        return -1;
    }
    cnt = 0;
    while ( fgets( s, 256, fp ) != NULL )
    {
        p = strstr( s, " = " );
        if ( p == 0 )
        {
            continue;
        }
        *p = '\0';
        strcpy( k[cnt], s );
        strcpy( v[cnt], p + 3 );

        p = strrchr( v[cnt], '\r' );
        if ( p != 0 )  *p = '\0';
        p = strrchr( v[cnt], '\n' );
        if ( p != 0 )  *p = '\0';

        printf( " ==> %s=%s\n", k[cnt], v[cnt] );
        cnt++;
    }
    fclose( fp );

    for ( int i = 0; i < cnt; i++ )
    {
        if ( strcmp( k[i], "CROP_CXMIN" ) == 0 )
        {
            tbcxminlevel = atof( v[i] );
            glui_3vd->sync_live();
            CallBackCropMinLevelX( 0 );
        }
        if ( strcmp( k[i], "CROP_CXMAX" ) == 0 )
        {
            tbcxmaxlevel = atof( v[i] );
            glui_3vd->sync_live();
            CallBackCropMaxLevelX( 0 );
        }
        if ( strcmp( k[i], "CROP_CYMIN" ) == 0 )
        {
            tbcyminlevel = atof( v[i] );
            glui_3vd->sync_live();
            CallBackCropMinLevelY( 0 );
        }
        if ( strcmp( k[i], "CROP_CYMAX" ) == 0 )
        {
            tbcymaxlevel = atof( v[i] );
            glui_3vd->sync_live();
            CallBackCropMaxLevelY( 0 );
        }
        if ( strcmp( k[i], "CROP_CZMIN" ) == 0 )
        {
            tbczminlevel = atof( v[i] );
            glui_3vd->sync_live();
            CallBackCropMinLevelZ( 0 );
        }
        if ( strcmp( k[i], "CROP_CZMAX" ) == 0 )
        {
            tbczmaxlevel = atof( v[i] );
            glui_3vd->sync_live();
            CallBackCropMaxLevelZ( 0 );
        }
        if ( strcmp( k[i], "CROP_PBVR_SRADIUS" ) == 0 )
        {
            tbsradius = atof( v[i] );
            glui_3vd->sync_live();
            CallBackSphereRadius( 0 );
        }
        if ( strcmp( k[i], "CROP_SCENTERX" ) == 0 )
        {
            tbscenterx = atof( v[i] );
            glui_3vd->sync_live();
            CallBackSphereCenterX( 0 );
        }
        if ( strcmp( k[i], "CROP_SCENTERY" ) == 0 )
        {
            tbscentery = atof( v[i] );
            glui_3vd->sync_live();
            CallBackSphereCenterY( 0 );
        }
        if ( strcmp( k[i], "CROP_SCENTERZ" ) == 0 )
        {
            tbscenterz = atof( v[i] );
            glui_3vd->sync_live();
            CallBackSphereCenterZ( 0 );
        }
        if ( strcmp( k[i], "CROP_PBVR_PRADIUS" ) == 0 )
        {
            tbpradius = atof( v[i] );
            glui_3vd->sync_live();
            CallBackPillarRadius( 0 );
        }
        if ( strcmp( k[i], "CROP_PBVR_PHEIGHT" ) == 0 )
        {
            tbpheight = atof( v[i] );
            glui_3vd->sync_live();
            CallBackPillarHeight( 0 );
        }
        if ( strcmp( k[i], "CROP_PCENTERX" ) == 0 )
        {
            tbpcenterx = atof( v[i] );
            glui_3vd->sync_live();
            CallBackPillarCenterX( 0 );
        }
        if ( strcmp( k[i], "CROP_PCENTERY" ) == 0 )
        {
            tbpcentery = atof( v[i] );
            glui_3vd->sync_live();
            CallBackPillarCenterY( 0 );
        }
        if ( strcmp( k[i], "CROP_PCENTERZ" ) == 0 )
        {
            tbpcenterz = atof( v[i] );
            glui_3vd->sync_live();
            CallBackPillarCenterZ( 0 );
        }
        GLUI_Master.sync_live_all();
    }
    return 0;
}

// Write to ParameterFile
int VizParameterFile::WriteParamFile( const char* fname )
{

    std::ofstream ofs;
    ofs.open( fname, std::ios::out | std::ios::trunc  );

//	ofs << "PBVR_SUB_PIXEL_LEVEL_ABSTRACT = " << subpixellevela << std::endl;
//	ofs << "PBVR_SUB_PIXEL_LEVEL_DETAILED = " << subpixelleveld << std::endl;
    ofs << "SUB_PIXEL_LEVEL = " << subpixelleveld << std::endl;
//	ofs << "PBVR_REPEAT_LEVEL_ABSTRACT = " << repeatlevela << std::endl;
//	ofs << "PBVR_REPEAT_LEVEL_DETAILED = " << repeatleveld << std::endl;
//	ofs << "REPEAT_LEVEL = " << repeatleveld << std::endl;
    ofs << "PBVR_PARTICLE_LIMIT = " << plimitlevel << std::endl;
    ofs << "SAMPLING_TYPE = " << samplingtypelevel << std::endl;
    ofs << "ROTATION_X_AXIS = " << rotation_x_axis_level << std::endl;
    ofs << "ROTATION_Y_AXIS = " << rotation_y_axis_level << std::endl;
    ofs << "ROTATION_Z_AXIS = " << rotation_z_axis_level << std::endl;
    ofs << "TRANSLATION_X = " << translate_x_level << std::endl;
    ofs << "TRANSLATION_Y = " << translate_y_level << std::endl;
    ofs << "TRANSLATION_Z = " << translate_z_level << std::endl;
    ofs << "SCALING = " << scaling_level << std::endl;
    ofs << "RESOLUTION_WIDTH = " << resolution_width_level << std::endl;
    ofs << "RESOLUTION_HEIGHT = " << resolution_height_level << std::endl;
    // delete by @hira at 2016/12/01
    // ofs << "PFI_PATH_SERVER = " << pfi_path_server << std::endl;
    // add by @hira at 2016/12/01
    ofs << PBVR_FILTER_PARAMFILENAME << " = " << filter_parameter_filename << std::endl;


//#ifndef __APPLE__
    switch ( obj_3d )
    {
    case 1:
        ofs << "CROP_TYPE = 1" << std::endl;
        break;
    case 2:
        ofs << "CROP_TYPE = 2" << std::endl;
        break;
    case 3:
        ofs << "CROP_TYPE = 3" << std::endl;
        break;
    case 4:
        ofs << "CROP_TYPE = 4" << std::endl;
        break;
    case 5:
        ofs << "CROP_TYPE = 5" << std::endl;
        break;
    default:
        ofs << "CROP_TYPE = 0" << std::endl;
        break;
    }
    ofs << "CROP_CXMIN = " << tbcxminlevel << std::endl;
    ofs << "CROP_CXMAX = " << tbcxmaxlevel << std::endl;
    ofs << "CROP_CYMIN = " << tbcyminlevel << std::endl;
    ofs << "CROP_CYMAX = " << tbcymaxlevel << std::endl;
    ofs << "CROP_CZMIN = " << tbczminlevel << std::endl;
    ofs << "CROP_CZMAX = " << tbczmaxlevel << std::endl;

    ofs << "CROP_PBVR_SRADIUS = " << tbsradius << std::endl;
    ofs << "CROP_SCENTERX = " << tbscenterx << std::endl;
    ofs << "CROP_SCENTERY = " << tbscentery << std::endl;
    ofs << "CROP_SCENTERZ = " << tbscenterz << std::endl;

    ofs << "CROP_PBVR_PRADIUS = " << tbpradius << std::endl;
    ofs << "CROP_PBVR_PHEIGHT = " << tbpheight << std::endl;
    ofs << "CROP_PCENTERX = " << tbpcenterx << std::endl;
    ofs << "CROP_PCENTERY = " << tbpcentery << std::endl;
    ofs << "CROP_PCENTERZ = " << tbpcenterz << std::endl;
//#endif
// APPEND START FP)M.TANAKA 2015.03.16
    ofs << "ROTATION1X = " << g_ro[0][0] << std::endl;
    ofs << "ROTATION1Y = " << g_ro[0][1] << std::endl;
    ofs << "ROTATION1Z = " << g_ro[0][2] << std::endl;
    ofs << "ROTATION2X = " << g_ro[1][0] << std::endl;
    ofs << "ROTATION2Y = " << g_ro[1][1] << std::endl;
    ofs << "ROTATION2Z = " << g_ro[1][2] << std::endl;
    ofs << "ROTATION3X = " << g_ro[2][0] << std::endl;
    ofs << "ROTATION3Y = " << g_ro[2][1] << std::endl;
    ofs << "ROTATION3Z = " << g_ro[2][2] << std::endl;

    ofs << "TRANSLATIONX = " << g_tr[0] << std::endl;
    ofs << "TRANSLATIONY = " << g_tr[1] << std::endl;
    ofs << "TRANSLATIONZ = " << g_tr[2] << std::endl;
    ofs << "SCALINGX = " << g_sc[0] << std::endl;
    ofs << "SCALINGY = " << g_sc[1] << std::endl;
    ofs << "SCALINGZ = " << g_sc[2] << std::endl;
// APPEND END   FP)M.TANAKA 2015.03.16
    if ( !param_x_synthesis.empty() )
    {
        ofs << "COORD1_SYNTH = " << param_x_synthesis << std::endl;
    }
    if ( !param_y_synthesis.empty() )
    {
        ofs << "COORD2_SYNTH = " << param_y_synthesis << std::endl;
    }
    if ( !param_z_synthesis.empty() )
    {
        ofs << "COORD3_SYNTH = " << param_z_synthesis << std::endl;
    }
    ofs << "FONTTYPE = " << param_fonttype << std::endl;
    ofs << "BACKGROUND_COLOR_R = " << ( unsigned int )crgb.r() << std::endl;
    ofs << "BACKGROUND_COLOR_G = " << ( unsigned int )crgb.g() << std::endl;
    ofs << "BACKGROUND_COLOR_B = " << ( unsigned int )crgb.b() << std::endl;

    ofs.close();

    return 0;
}
