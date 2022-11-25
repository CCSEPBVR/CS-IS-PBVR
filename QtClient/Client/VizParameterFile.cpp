#include "VizParameterFile.h"

#include "string"
#include "App/pbvrgui.h"


float g_ro[3][3];
float g_tr[3];
float g_sc[3];

extern int subpixellevela; //Command.cpp, ExtCommand.cpp
extern int subpixelleveld; //Command.cpp, ExtCommand.cpp
extern int repeatleveld;    //ExtCommand.cpp
extern int samplingtypelevel;//ExtCommand
extern float lookat_upvectory_level;//ExtCommand
extern float lookat_upvectorz_level;//ExtCommand
extern char filter_parameter_filename[256];		//ExtCommand , Command



// Shared with font settings, but not used in Qt
std::string param_fonttype;
kvs::RGBColor crgb( 200, 200, 210 );     // Background Color Data

//GLUI_EditText* rotation_x_axis;
//GLUI_EditText* rotation_y_axis;
//GLUI_EditText* rotation_z_axis;
//GLUI_EditText* translate_x;
//GLUI_EditText* translate_y;
//GLUI_EditText* translate_z;
//GLUI_EditText* scaling;
//GLUI_EditText* lookat_upvectory;
//GLUI_EditText* lookat_upvectorz;
//GLUI_EditText* resolution_width;
//GLUI_EditText* resolution_height;

//GLUI_EditText* subpixeld;
//GLUI_EditText* repeatd;
//GLUI_EditText* plimit;



extern char g_volumedata[256];  //ComandQ /* for PFI file */
extern bool g_volumedata_active;//ComandQ

// add by @hira at 2016/12/01
#define PBVR_FILTER_PARAMFILENAME    "FILTER_PARAMETER_FILENAME"

// Read ParameterFile

kvs::RGBColor VizParameterFile::getBackgroundColor()
{
    return  crgb;
}

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
        }
        //		if (strcmp(k[i], "PBVR_REPEAT_LEVEL_DETAILED") == 0) {
        if ( strcmp( k[i], "REPEAT_LEVEL" ) == 0 )
        {
            repeatleveld = atoi( v[i] );
        }
        if ( strcmp( k[i], "PBVR_PARTICLE_LIMIT" ) == 0 )
        {
            RenderoptionPanel::setParticleLimit( v[i] );
        }
        if ( strcmp( k[i], "SAMPLING_TYPE" ) == 0 )
        {
            samplingtypelevel = atoi( v[i] );
        }

        if ( strcmp( k[i], "ROTATION_X_AXIS" ) == 0 )
        {
            TransformPanel::setRotationAxisX( v[i] );
        }
        if ( strcmp( k[i], "ROTATION_Y_AXIS" ) == 0 )
        {
            TransformPanel::setRotationAxisY( v[i] );
        }
        if ( strcmp( k[i], "ROTATION_Z_AXIS" ) == 0 )
        {
            TransformPanel::setRotationAxisZ( v[i] );
        }
        if ( strcmp( k[i], "TRANSLATION_X" ) == 0 )
        {
            TransformPanel::setTranslationX( v[i] );
        }
        if ( strcmp( k[i], "TRANSLATION_Y" ) == 0 )
        {
            TransformPanel::setTranslationY( v[i] );
        }
        if ( strcmp( k[i], "TRANSLATION_Z" ) == 0 )
        {
            TransformPanel::setTranslationZ( v[i] );
        }
        if ( strcmp( k[i], "SCALING" ) == 0 )
        {
            TransformPanel::setScaling( v[i] );
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
            RenderoptionPanel::setResolutionWidth( v[i] );
        }
        if ( strcmp( k[i], "RESOLUTION_HEIGHT" ) == 0 )
        {
            RenderoptionPanel::setResolutionHeight( v[i] );
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
            strcpy( g_volumedata, v[i] );
            g_volumedata_active=false;
            //            g_volumedata->set_text( v[i] );
            //            g_volumedata->deactivate();
        }

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
            CoordinatePanel::param_x_synthesis = std::string( v[i] );
        }
        if ( strcmp( k[i], "COORD2_SYNTH" ) == 0 )
        {
            CoordinatePanel::param_y_synthesis = std::string( v[i] );
        }
        if ( strcmp( k[i], "COORD3_SYNTH" ) == 0 )
        {
            CoordinatePanel::param_z_synthesis = std::string( v[i] );
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
    ofs << "PBVR_PARTICLE_LIMIT = " << RenderoptionPanel::plimitlevel << std::endl;
    ofs << "SAMPLING_TYPE = " << samplingtypelevel << std::endl;
    ofs << "ROTATION_X_AXIS = " << TransformPanel::param.rotationAxisX << std::endl;
    ofs << "ROTATION_Y_AXIS = " << TransformPanel::param.rotationAxisY << std::endl;
    ofs << "ROTATION_Z_AXIS = " << TransformPanel::param.rotationAxisZ << std::endl;
    ofs << "TRANSLATION_X = " << TransformPanel::param.translateX << std::endl;
    ofs << "TRANSLATION_Y = " << TransformPanel::param.translateY << std::endl;
    ofs << "TRANSLATION_Z = " << TransformPanel::param.translateZ << std::endl;
    ofs << "SCALING = " << TransformPanel::param.scalingLevel << std::endl;
    ofs << "RESOLUTION_WIDTH = " << RenderoptionPanel::resolution_width_level << std::endl;
    ofs << "RESOLUTION_HEIGHT = " << RenderoptionPanel::resolution_height_level << std::endl;
    // delete by @hira at 2016/12/01
    // ofs << "PFI_PATH_SERVER = " << pfi_path_server << std::endl;
    // add by @hira at 2016/12/01
    ofs << PBVR_FILTER_PARAMFILENAME << " = " << filter_parameter_filename << std::endl;



#ifdef ENABLE_CROP
    switch (CropTool::getGeometryType())
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
    ofs << "CROP_CXMIN = " << box.tbcxminlevel << std::endl;
    ofs << "CROP_CXMAX = " << box.tbcxmaxlevel << std::endl;
    ofs << "CROP_CYMIN = " << box.tbcyminlevel << std::endl;
    ofs << "CROP_CYMAX = " << box.tbcymaxlevel << std::endl;
    ofs << "CROP_CZMIN = " << box.tbczminlevel << std::endl;
    ofs << "CROP_CZMAX = " << box.tbczmaxlevel << std::endl;

    ofs << "CROP_PBVR_SRADIUS = " << sphere.tbsradius << std::endl;
    ofs << "CROP_SCENTERX = " << sphere.tbscenterx << std::endl;
    ofs << "CROP_SCENTERY = " << sphere.tbscentery << std::endl;
    ofs << "CROP_SCENTERZ = " << sphere.tbscenterz << std::endl;

    ofs << "CROP_PBVR_PRADIUS = " << cylinder.tbpradius << std::endl;
    ofs << "CROP_PBVR_PHEIGHT = " << cylinder.tbpheight << std::endl;
    ofs << "CROP_PCENTERX = " << cylinder.tbpcenterx << std::endl;
    ofs << "CROP_PCENTERY = " << cylinder.tbpcentery << std::endl;
    ofs << "CROP_PCENTERZ = " << cylinder.tbpcenterz << std::endl;
#endif
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
    if ( !CoordinatePanel::param_x_synthesis.empty() )
    {
        ofs << "COORD1_SYNTH = " << CoordinatePanel::param_x_synthesis << std::endl;
    }
    if ( !CoordinatePanel::param_y_synthesis.empty() )
    {
        ofs << "COORD2_SYNTH = " << CoordinatePanel::param_y_synthesis << std::endl;
    }
    if ( !CoordinatePanel::param_z_synthesis.empty() )
    {
        ofs << "COORD3_SYNTH = " << CoordinatePanel::param_z_synthesis << std::endl;
    }
    ofs << "FONTTYPE = " << param_fonttype << std::endl;
    ofs << "BACKGROUND_COLOR_R = " << ( unsigned int )crgb.r() << std::endl;
    ofs << "BACKGROUND_COLOR_G = " << ( unsigned int )crgb.g() << std::endl;
    ofs << "BACKGROUND_COLOR_B = " << ( unsigned int )crgb.b() << std::endl;

    ofs.close();

    return 0;
}

kvs::Xform VizParameterFile::ConversionFloatToClass( )
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

    return readxform;
//    kvs::PointObject* object = RenderArea::g_timer_event->getPointObject();
//    if ( object )
//    {
//        object->setXform( readxform );
//    }
//#endif
}


void VizParameterFile::ConversionClassToFloat(kvs::Xform outxform)
{
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
//    }
//    else
//    {
//        g_ro[0][0] = 1.0;
//        g_ro[0][1] = 0.0;
//        g_ro[0][2] = 0.0;
//        g_ro[1][0] = 0.0;
//        g_ro[1][1] = 1.0;
//        g_ro[1][2] = 0.0;
//        g_ro[2][0] = 0.0;
//        g_ro[2][1] = 0.0;
//        g_ro[2][2] = 1.0;

//        g_tr[0] = 0.0;
//        g_tr[1] = 0.0;
//        g_tr[2] = 0.0;

//        g_sc[0] = 1.0;
//        g_sc[1] = 1.0;
//        g_sc[2] = 1.0;
//    }
//#endif
}
