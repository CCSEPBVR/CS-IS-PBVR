#ifndef PBVR__V2__NEW_PANEL_H_INCLUDE
#define PBVR__V2__NEW_PANEL_H_INCLUDE

//    hc = screen;
kvs::glut::Screen m_screen3( &app );
//    m_screen3.setGeometry( 980, 0, 512, 300 );
// MODIFY START fp)K.YAJIMA 2015.03.16
//      m_screen3.setGeometry( 980, 0, 512, 250 );
m_screen3.setGeometry( 980, 0, 512, 340 );
// MODIFY END fp)K.YAJIMA 2015.03.16
//	m_screen3.setTitle( "TFE Sub screen" );
m_screen3.setTitle( "Time panel" );

// MODIFY START fp)m.takizawa 2014.04.17
//	GLUI *glui = GLUI_Master.create_glui( "TFE Sub screen 2", 1, 600, 600 );
GLUI* glui = GLUI_Master.create_glui( "PBVR Main panel", 1, 600, 500 );  // MODIFY Fj 2015.03.05
//	GLUI *glui = GLUI_Master.create_glui( "TFE Sub screen 2", 1, 10, 10 );
// MODIFY END fp)m.takizawa 2014.04.17

// DELETE START FP)K.YAJIMA 2015.03.02
//   :
// DELETE END FP)K.YAJIMA 2015.03.02

// APPEND START FP)K.YAJIMA 2015.03.02
glui->add_statictext( "SYSTEM STATUS" );
glui->add_separator();

// SENDING/RECIEVING
lbl_sending = glui->add_statictext( "SENDING  ... **********" );
lbl_recieving = glui->add_statictext( "RECIEVING... **********" );

glui->add_separator();

// PERFORMANCE
lbl_smemory = glui->add_statictext( "SYSTEM MEMORY 1500/4000 38%" );
lbl_gmemory = glui->add_statictext( "GPU MEMORY 2042/2097 97%" );
// APPEND END FP)K.YAJIMA 2015.03.02

glui->add_separator();

// Display Particle Number
lbl_numParticles = glui->add_statictext( "Display Particle Number : 0" );
glui->add_separator();


// VOLUME DATA FILE (SERVER)
/* GLUI_EditText *volumedata; */
char buff_volumedata[100] = "";
//	//char buff_volumedata[100] = "/work/filter/binary08/binary";
// glui->add_statictext( "PFI file path (SERVER):" );
glui->add_statictext( "Filter Parameter File Path (SERVER):" );
g_volumedata = glui->add_edittext( "", GLUI_EDITTEXT_TEXT, buff_volumedata );
g_volumedata->set_w( 250 );
if ( argument.m_header != "" )
{
    g_volumedata->disable();
}

m_no_repsampling = glui->add_checkbox( "no resampling until Transfer Functions be edited.", NULL, -1, CallBackNoRepeatSampling );
if ( argument.m_header != "" )
{
    m_no_repsampling->disable();
}

interval_msec = glui->add_edittext( "sampling/load inverval [msec/step]", GLUI_EDITTEXT_INT, NULL );
#ifdef WIN32
interval_msec->set_int_limits( 1000, 1000000, GLUI_LIMIT_CLAMP );
interval_msec->set_int_val( 1000 );
#else
interval_msec->set_int_limits( 1, 1000000, GLUI_LIMIT_CLAMP );
interval_msec->set_int_val( 100 );
#endif
interval_msec->set_w( 250 );

// PARALELL LEVEL
//GLUI_EditText *parallel;
//int parallellevel = 0;
//p/arallel = glui->add_edittext( "PARALLEL LEVEL:", GLUI_EDITTEXT_INT, &parallellevel, 1, cbparallellevel );
//parallel->set_alignment(GLUI_ALIGN_RIGHT);
//parallel->set_w(180);
//parallel->set_int_limits(0, 99, GLUI_LIMIT_CLAMP);

//	glui->add_separator();

//glui->add_checkbox("foo", 0, 1, cbworkdir);




//GLUI_Panel *obj_panela = glui->add_panel("ALL/PART", 1);
//obj_panela->set_alignment(GLUI_ALIGN_LEFT);

//GLUI_EditText *amin;
//int aminlevel = 0;
//glui->add_edittext_to_panel(obj_panela, "MIN:", GLUI_EDITTEXT_INT, &aminlevel, 1, cbaminlevel );

//GLUI_EditText *amax;
//int amaxlevel = 0;
//glui->add_edittext_to_panel(obj_panela, "MAX:", GLUI_EDITTEXT_INT, &amaxlevel, 1, cbamaxlevel );

//glui->add_separator();
//glui->add_column(true);

// MODIFY START fp)m.takizawa 2014.04.16
#ifdef NOTOPEN
/*	GLUI_Panel *obj_panelcr = glui->add_panel("CROP", 1);
    obj_panelcr->set_alignment(GLUI_ALIGN_LEFT);

    //GLUI_EditText *crop_cxmin;
    //float cxminlevel = 0.0F;
    crop_cxmin = glui->add_edittext_to_panel(obj_panelcr, "XMIN:", GLUI_EDITTEXT_FLOAT, &cxminlevel, 1, cbcxminlevel );
    crop_cxmin->set_alignment(GLUI_ALIGN_RIGHT);
    crop_cxmin->set_w(80);

    //GLUI_EditText *crop_cxmax;
    //float cxmaxlevel = 0.0F;
    crop_cxmax = glui->add_edittext_to_panel(obj_panelcr, "XMAX:", GLUI_EDITTEXT_FLOAT, &cxmaxlevel, 1, cbcxmaxlevel );
    crop_cxmax->set_alignment(GLUI_ALIGN_RIGHT);
    crop_cxmax->set_w(80);

    //GLUI_EditText *crop_cymin;
    //float cyminlevel = 0.0F;
    crop_cymin = glui->add_edittext_to_panel(obj_panelcr, "YMIN:", GLUI_EDITTEXT_FLOAT, &cyminlevel, 1, cbcyminlevel );
    crop_cymin->set_alignment(GLUI_ALIGN_RIGHT);
    crop_cymin->set_w(80);

    //GLUI_EditText *crop_cymax;
    //float cymaxlevel = 0.0F;
    crop_cymax = glui->add_edittext_to_panel(obj_panelcr, "YMAX:", GLUI_EDITTEXT_FLOAT, &cymaxlevel, 1, cbcymaxlevel );
    crop_cymax->set_alignment(GLUI_ALIGN_RIGHT);
    crop_cymax->set_w(80);

    //GLUI_EditText *crop_czmin;
    //float czminlevel = 0.0F;
    crop_czmin = glui->add_edittext_to_panel(obj_panelcr, "ZMIN:", GLUI_EDITTEXT_FLOAT, &czminlevel, 1, cbczminlevel );
    crop_czmin->set_alignment(GLUI_ALIGN_RIGHT);
    crop_czmin->set_w(80);

    //GLUI_EditText *crop_czmax;
    //float czmaxlevel = 0.0F;
    crop_czmax = glui->add_edittext_to_panel(obj_panelcr, "ZMAX:", GLUI_EDITTEXT_FLOAT, &czmaxlevel, 1, cbczmaxlevel );
    crop_czmax->set_alignment(GLUI_ALIGN_RIGHT);
    crop_czmax->set_w(80);
*/
#endif

// DELETE START FP)K.YAJIMA 2015.02.19
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
//	glui->add_statictext("");
////	glui->add_statictext("");
// DELETE END FP)K.YAJIMA 2015.02.19

// APPEND START FP)K.YAJIMA 2015.02.19
// pfi infomation
GLUI_Panel* obj_panelpfi = glui->add_panel( "PBVR Filter infomation", 1 );
obj_panelpfi->set_alignment( GLUI_ALIGN_LEFT );

lbl_numNodes = glui->add_statictext_to_panel( obj_panelpfi, "All node:" );
lbl_numElements = glui->add_statictext_to_panel( obj_panelpfi, "All element:" );
lbl_elemType = glui->add_statictext_to_panel( obj_panelpfi, "Element type:" );
lbl_fileType = glui->add_statictext_to_panel( obj_panelpfi, "File type:" );
lbl_numIngredients = glui->add_statictext_to_panel( obj_panelpfi, "Vector num:" );
lbl_numStep = glui->add_statictext_to_panel( obj_panelpfi, "Step num:" );
lbl_numVolDiv = glui->add_statictext_to_panel( obj_panelpfi, "SUB volume:" );
glui->add_statictext_to_panel( obj_panelpfi, "Extent 3D space -----" );
lbl_objectCoord0 = glui->add_statictext_to_panel( obj_panelpfi, " X-Axis: min=...., max=...   " );
lbl_objectCoord1 = glui->add_statictext_to_panel( obj_panelpfi, " Y-Axis: min=...., max=...   " );
lbl_objectCoord2 = glui->add_statictext_to_panel( obj_panelpfi, " Z-Axis: min=...., max=...   " );
glui->add_statictext_to_panel( obj_panelpfi, "" );
glui->add_statictext_to_panel( obj_panelpfi, "" );
glui->add_statictext_to_panel( obj_panelpfi, "" );
glui->add_statictext_to_panel( obj_panelpfi, "" );
// APPEND END FP)K.YAJIMA 2015.02.19

//#ifndef __APPLE__
glui->add_statictext( "" );
GLUI_Button* btn_crop;
btn_crop = glui->add_button( "CROP", 0, CallBackCrop );
if ( argument.m_header != "" )
{
    btn_crop->disable();
}
glui->add_statictext( "" );
//#endif
//	glui->add_button("Trans Func Editer", 0, ExTransFuncEditCallback );
//	glui->add_statictext("");
//	glui->add_separator();
// MODIFY END fp)m.takizawa 2014.04.16

// APPEND START FP)K.YAJIMA 2015.02.19

// DELETE START FP)K.YAJIMA 2015.03.02
// TO DO:いずれ元に戻す
// PARTICLEボタン
//	GLUI_Button *btn_par;
//	btn_par = glui->add_button("PARTICLE", 0,ParticleCallback); //TO DO:PARTICLEパネルが開くようにする
//	btn_par->disable(); //TO DO:いずれ削除する(ボタン無効化)

//	glui->add_statictext("");
// DELETE END FP)K.YAJIMA 2015.03.02

// FILEボタン
GLUI_Button* btn_file;
btn_file = glui->add_button( "FILE", 0, CallBackFile ); //TO DO:Fileパネルが開くようにする

glui->add_statictext( "" );

// APPEND END FP)K.YAJIMA 2015.02.19

// MODIFY START FP)K.YAJIMA 2015.03.02
//	glui->add_button("APPLY", 1, CallBackm_apply);
glui->add_button( "Set Parameter", 1, CallBackApply );
// MODIFY END FP)K.YAJIMA 2015.03.02

glui->add_column( true );

// DELETE START FP)K.YAJIMA 2015.03.02
// :
// DELETE END FP)K.YAJIMA 2015.03.02

// APPEND START FP)K.YAJIMA 2015.03.02
#if 0
// Sub Pixel Level
GLUI_Panel* obj_pixellevel = glui->add_panel( "SUB PIXEL LEVEL", 1 );
obj_pixellevel->set_alignment( GLUI_ALIGN_LEFT );
#if 0
//GLUI_EditText *subpixela;
//int subpixellevela = 2;
subpixela = glui->add_edittext_to_panel( obj_pixellevel, "ABSTRACT:", GLUI_EDITTEXT_INT, &subpixellevela, 1, cbsubpixellevela );
subpixela->set_alignment( GLUI_ALIGN_RIGHT );
subpixela->set_w( 130 );
subpixela->set_int_limits( 1, 99, GLUI_LIMIT_CLAMP );
#endif
//GLUI_EditText *subpixeld;
//int subpixelleveld = 2;
//subpixeld = glui->add_edittext_to_panel(obj_pixellevel, "DETAILED:", GLUI_EDITTEXT_INT, &subpixelleveld, 1, cbsubpixelleveld );
subpixeld = glui->add_edittext_to_panel( obj_pixellevel, "", GLUI_EDITTEXT_INT, &subpixelleveld, 1, cbsubpixelleveld );
subpixeld->set_alignment( GLUI_ALIGN_RIGHT );
subpixeld->set_w( 130 );
subpixeld->set_int_limits( 1, 99, GLUI_LIMIT_CLAMP );
#endif
// Particle Density
pdensity = glui->add_edittext( "PARTICLE DENSITY:", GLUI_EDITTEXT_FLOAT, &pdensitylevel, 1, CallBackParticleDensityLevel );
pdensity->set_alignment( GLUI_ALIGN_LEFT );
pdensity->set_w( 130 );
pdensity->set_int_limits( 1, 99, GLUI_LIMIT_CLAMP );
if ( argument.m_header != "" )
{
    pdensity->disable();
}

#if 0
// Repeat Level
GLUI_Panel* obj_repeat = glui->add_panel( "REPEAT LEVEL", 1 );
obj_repeat->set_alignment( GLUI_ALIGN_LEFT );
#if 0
//GLUI_EditText *repeata;
//int repeatlevela = 1;
repeata = glui->add_edittext_to_panel( obj_repeat, "ABSTRACT:", GLUI_EDITTEXT_INT, &repeatlevela, 1, cbrepeatlevela );
repeata->set_alignment( GLUI_ALIGN_RIGHT );
repeata->set_w( 150 );
repeata->set_int_limits( 1, 99999, GLUI_LIMIT_CLAMP );
#endif
//GLUI_EditText *repeatd;
//int repeatleveld = 16;
//repeatd = glui->add_edittext_to_panel(obj_repeat, "DETAILED:", GLUI_EDITTEXT_INT, &repeatleveld, 1, cbrepeatleveld );
repeatd = glui->add_edittext_to_panel( obj_repeat, "", GLUI_EDITTEXT_INT, &repeatleveld, 1, cbrepeatleveld );
repeatd->set_alignment( GLUI_ALIGN_RIGHT );
repeatd->set_w( 150 );
repeatd->set_int_limits( 1, 99999, GLUI_LIMIT_CLAMP );
#endif
// Particle Limite
//GLUI_EditText *plimit;
//int plimitlevel = 999999;
plimit = glui->add_edittext( "PARTICLE LIMIT:  ", GLUI_EDITTEXT_INT, &plimitlevel, 1, CallBackParticleLimitLevel );
plimit->set_alignment( GLUI_ALIGN_LEFT );
//	plimit->set_w(180);
plimit->set_w( 200 );
plimit->set_int_limits( 1, 999999999, GLUI_LIMIT_CLAMP );
// APPEND END FP)K.YAJIMA 2015.03.02
if ( argument.m_header != "" )
{
    plimit->disable();
}

//	glui->add_statictext("");  // DELETE FP)K.YAJIMA 2015.02.19

glui->add_separator();
// DELETE START FP)K.YAJIMA 2015.03.02
// :
// DELETE END FP)K.YAJIMA 2015.02.19

#ifdef NOTOPEN
// KVS GPU/CPU MODE
GLUI_Panel* obj_panel = glui->add_panel ( "GPU/CPU MODE (CLIENT)" );
obj_panel->set_alignment( GLUI_ALIGN_LEFT );

GLUI_RadioGroup* gpucpumode;
gpucpumode = glui->add_radiogroup_to_panel( obj_panel, 0, 2, cbgpucpumode );
glui->add_radiobutton_to_group( gpucpumode, "GPU" );
glui->add_radiobutton_to_group( gpucpumode, "CPU" );
//glui->add_rotation("aaa", 0, 0);

glui->add_separator();
#endif

// LOOKAT
GLUI_Panel* obj_panele = glui->add_panel( "Rotation", 1 );
obj_panele->set_alignment( GLUI_ALIGN_LEFT );

//	GLUI_EditText *rotation_x_axis; // DELETE BY)M.Tanaka 2015.03.03
//float rotation_x_axis_level = 0.0F;
rotation_x_axis = glui->add_edittext_to_panel( obj_panele, "X-axis:", GLUI_EDITTEXT_FLOAT, &rotation_x_axis_level, 1, CallBackLookAtEyeX );
rotation_x_axis->set_alignment( GLUI_ALIGN_RIGHT );
rotation_x_axis->set_w( 80 );
rotation_x_axis->set_int_limits( -360.0f, 360.0f, GLUI_LIMIT_CLAMP );

//	GLUI_EditText *rotation_y_axis; // DELETE BY)M.Tanaka 2015.03.03
//float rotation_y_axis_level = 0.0F;
rotation_y_axis = glui->add_edittext_to_panel( obj_panele, "Y-axis:", GLUI_EDITTEXT_FLOAT, &rotation_y_axis_level, 1, CallBackLookAtEyeY );
rotation_y_axis->set_alignment( GLUI_ALIGN_RIGHT );
rotation_y_axis->set_w( 80 );
rotation_y_axis->set_int_limits( -360.0f, 360.0f, GLUI_LIMIT_CLAMP );

//	GLUI_EditText *rotation_z_axis; // DELETE BY)M.Tanaka 2015.03.03
//float rotation_z_axis_level = 0.0F;
rotation_z_axis = glui->add_edittext_to_panel( obj_panele, "Z-axis:", GLUI_EDITTEXT_FLOAT, &rotation_z_axis_level, 1, CallBackLookAtEyeZ );
rotation_z_axis->set_alignment( GLUI_ALIGN_RIGHT );
rotation_z_axis->set_w( 80 );
rotation_z_axis->set_int_limits( -360.0f, 360.0f, GLUI_LIMIT_CLAMP );


GLUI_Panel* obj_panelc = glui->add_panel( "Translation", 1 );
obj_panelc->set_alignment( GLUI_ALIGN_LEFT );

//	GLUI_EditText *translate_x; // DELETE BY)M.Tanaka 2015.03.03
//float translate_x_level = 0.0F;
translate_x = glui->add_edittext_to_panel( obj_panelc, "X:", GLUI_EDITTEXT_FLOAT, &translate_x_level, 1, CallBackLookAtCenterX );
translate_x->set_alignment( GLUI_ALIGN_RIGHT );
translate_x->set_w( 80 );
translate_x->set_int_limits( -100.0f, 100.0f, GLUI_LIMIT_CLAMP );

//	GLUI_EditText *translate_y; // DELETE BY)M.Tanaka 2015.03.03
//float translate_y_level = 0.0F;
translate_y = glui->add_edittext_to_panel( obj_panelc, "Y:", GLUI_EDITTEXT_FLOAT, &translate_y_level, 1, CallBackLookAtCenterY );
translate_y->set_alignment( GLUI_ALIGN_RIGHT );
translate_y->set_w( 80 );
translate_y->set_int_limits( -100.0f, 100.0f, GLUI_LIMIT_CLAMP );

//	GLUI_EditText *translate_z; // DELETE BY)M.Tanaka 2015.03.03
//float translate_z_level = 0.0F;
translate_z = glui->add_edittext_to_panel( obj_panelc, "Z:", GLUI_EDITTEXT_FLOAT, &translate_z_level, 1, CallBackLookAtCenterZ );
translate_z->set_alignment( GLUI_ALIGN_RIGHT );
translate_z->set_w( 80 );
translate_z->set_int_limits( -100.0f, 100.0f, GLUI_LIMIT_CLAMP );


//GLUI_Panel* obj_panelv = glui->add_panel( "Scaling", 1 );
//obj_panelv->set_alignment( GLUI_ALIGN_LEFT );
//
////	GLUI_EditText *scaling; // DELETE BY)M.Tanaka 2015.03.03
////float scaling_level = 0.0F;
//scaling = glui->add_edittext_to_panel( obj_panelv, "s:", GLUI_EDITTEXT_FLOAT, &scaling_level, 1, CallBackLookAtUpvectorX );
//scaling->set_alignment( GLUI_ALIGN_RIGHT );
//scaling->set_w( 80 );
//scaling->set_int_limits( 0.0f, 100.0f, GLUI_LIMIT_CLAMP );

scaling = glui->add_edittext( "Scaling:    ", GLUI_EDITTEXT_FLOAT, &scaling_level, 1, CallBackLookAtUpvectorX );
scaling->set_alignment( GLUI_ALIGN_LEFT );
//	plimit->set_w(180);
scaling->set_w( 80 );
scaling->set_float_limits( 0.01f, 100.0f, GLUI_LIMIT_CLAMP );
//	GLUI_EditText *lookat_upvectory; // DELETE BY)M.Tanaka 2015.03.03
//float lookat_upvectory_level = 1.0F;
//lookat_upvectory = glui->add_edittext_to_panel( obj_panelv, "Y:", GLUI_EDITTEXT_FLOAT, &lookat_upvectory_level, 1, CallBackLookAtUpvectorY ); lookat_upvectory->set_alignment( GLUI_ALIGN_RIGHT );
//lookat_upvectory->set_w( 80 );
//lookat_upvectory->set_int_limits( -1.0f, 1.0f, GLUI_LIMIT_CLAMP );

//	GLUI_EditText *lookat_upvectorz; // DELETE BY)M.Tanaka 2015.03.03
//float lookat_upvectorz_level = 0.0F;
//lookat_upvectorz = glui->add_edittext_to_panel( obj_panelv, "Z:", GLUI_EDITTEXT_FLOAT, &lookat_upvectorz_level, 1, CallBackLookAtUpvectorZ );
//lookat_upvectorz->set_alignment( GLUI_ALIGN_RIGHT );
//lookat_upvectorz->set_w( 80 );
//lookat_upvectorz->set_int_limits( -1.0f, 1.0f, GLUI_LIMIT_CLAMP );


glui->add_separator();

// DELETE START FP)K.YAJIMA 2015.02.19
//	GLUI_Button *cbtn;
//	cbtn = glui->add_button("PBVR_CAPTURE", 1, cbm_capture);
//	cbtn->set_alignment(GLUI_ALIGN_LEFT);

//    // SAVE IMAGE FILE NAME
//	GLUI_EditText *m_capturefile;
//	glui->add_statictext("IMAGE BMP FILE (CLIENT):");
//	m_capturefile = glui->add_edittext("", GLUI_EDITTEXT_TEXT, buff_capfile );
//	m_capturefile->set_w(250);
// DELETE END FP)K.YAJIMA 2015.02.19

// APPEND START FP)K.YAJIMA 2015.02.19
// レゾリューション
GLUI_Panel* obj_panelres = glui->add_panel( "RESOLUTION", 1 );
obj_panelres->set_alignment( GLUI_ALIGN_LEFT );

resolution_width = glui->add_edittext_to_panel( obj_panelres, "WIDTH", GLUI_EDITTEXT_FLOAT, &resolution_width_level, 1, CallBackResolutionWidth );
resolution_height = glui->add_edittext_to_panel( obj_panelres, "HEIGHT", GLUI_EDITTEXT_FLOAT, &resolution_height_level, 1, CallBackResolutionHeight );
// APPEND END FP)K.YAJIMA 2015.02.19

GLUI_Panel* pn_panelsw = glui->add_panel( "Panel Switch" );
pn_panelsw->set_alignment( GLUI_ALIGN_LEFT );
GLUI_Button* bt_panelsw[6];
int panelsw[6] = { 0, 0, 0, 0, 0 };
bt_panelsw[0] = glui->add_button_to_panel( pn_panelsw, "Transfer Function Editor", 1, CallBackTransferFunctionEditorPanelShow );
bt_panelsw[0]->set_w( 180 );
if ( argument.m_header != "" )
{
    bt_panelsw[0]->disable();
}
bt_panelsw[1] = glui->add_button_to_panel( pn_panelsw, "Particle Panel", 1, CallBackParticlePanelShow );
bt_panelsw[1]->set_w( 180 );
bt_panelsw[2] = glui->add_button_to_panel( pn_panelsw, "Legend Panel", 1, CallBackLagendPanelShow );
bt_panelsw[2]->set_w( 180 );
if ( argument.m_header != "" )
{
    bt_panelsw[2]->disable();
}
bt_panelsw[5] = glui->add_button_to_panel( pn_panelsw, "Coordinate Panel", 1, CallBackCoordinatePanelShow );
bt_panelsw[5]->set_w( 180 );
if ( argument.m_header != "" )
{
    bt_panelsw[5]->disable();
}
bt_panelsw[4] = glui->add_button_to_panel( pn_panelsw, "Viewer Control Panel", 1, CallBackViewerPanelShow );
bt_panelsw[4]->set_w( 180 );
bt_panelsw[3] = glui->add_button_to_panel( pn_panelsw, "Animation Control Panel", 1, CallBackAnimationPanelShow );
bt_panelsw[3]->set_w( 180 );

//	m_screen3.show();


// for datadump
time_t now = time( NULL );
struct tm* pnow = localtime( &now );
sprintf( data_filename, "%s/%s_%04d%02d%02d%02d%02d%02d", buff_workdir, PBVR_DATAFILE,
         pnow->tm_year + 1900,
         pnow->tm_mon + 1,
         pnow->tm_mday,
         pnow->tm_hour,
         pnow->tm_min,
         pnow->tm_sec );

remove( data_filename );


// for limit particle data
sprintf( limitdata_filename, "%s/%s.dat", buff_workdir, PBVR_LIMIT_DATA_FILE );

remove( limitdata_filename );

//	if (getenv("PARTICLE_OUTPUT") != NULL) {
//		data_output = true;
//	}

#endif //PBVR__V2__NEW_PANEL_H_INCLUDE
