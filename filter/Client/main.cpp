#include "kvs/Message"
#include "kvs/PointImporter"
#include "kvs/PointObject"
#include "kvs/PointRenderer"
#include "kvs/glut/Application"
#include "kvs/glut/Screen"
#include "kvs/TransferFunction"
#include "kvs/glut/LegendBar"
#include "kvs/glut/OrientationAxis"
#include "kvs/File"
#include "kvs/ExtractEdges"
#include "kvs/glut/Slider"
#include "kvs/PolygonRenderer"
#include "kvs/HydrogenVolumeData"
#include "kvs/Timer"

#include "Command.h"
#include "ParticleServer.h"
#include "TransferFunctionEditor.h"
#include "TransferFunctionEditorMain.h" // APPEND fj) 2015.03.04
#include "Controller.h"
#include "ExtendedParticleVolumeRenderer.h"
#include "TimerEvent.h"
#include "Argument.h"
#include "Label.h"
#include "LegendpanelUI.h" // APPEND by FEAST 2016.1.18
#include "ViewerpanelUI.h" // APPEND by FEAST 2016.2.18
#include "CoordinatepanelUI.h" // APPEND by FEAST 2016.2.18
#include "KeyPressEvent.h"
#include "ParticleMerger.h"

#include "kvs/Thread"
#include "kvs/Mutex"

#include "TransferFunctionEditorMain.h"
// APPEND START fp)m.tanaka 2013.09.01
#include "v2head.h"
#include "v3head.h"
#include "AnimationControl.h"
// APPEND END   fp)m.tanaka 2013.09.01
#include "MemoryMonitor.h"
//#include <string>
#include "sighandler.h"

// APPEND START BY)M.Tanaka 2015.03.03
#include "VizParameterFile.h"
kvs::visclient::StepLabel* g_label;
kvs::visclient::FPSLabel* g_fpsLabel;
kvs::visclient::LegendBar* g_legend;
kvs::glut::OrientationAxis* g_orientation_axis;
// APPEND END   BY)M.Tanaka 2015.03.03
// APPEND START FP)M.TANAKA 2015.03.16
kvs::PointObject* g_control_object;
kvs::visclient::TimerEvent* g_timer_event;
// APPEND END   FP)M.TANAKA 2015.03.16

#if 1 // FEAST modify 20160128
int g_curStep = -1;
#endif // FEAST modify 20160128 end

kvs::visclient::TransferFunctionEditorMain* ptfe_main; // APPEND Fj 2015.03.05
kvs::visclient::ParticleMergerUI m_Particle_UI; // APPEND FEAST 2016.2.5
kvs::visclient::LegendpanelUI m_Legend_UI; // APPEND FEAST 2016.2.8
kvs::visclient::CoordinatepanelUI m_Coodinate_UI; // APPEND FEAST 2016.2.8
kvs::visclient::ViewerpanelUI m_Viewer_UI; // APPEND FEAST 2016.2.18
static bool show_aui = false; // APPEND FEAST 2016.2.5

int main( int argc, char** argv )
{
    int ret;
    kvs::visclient::Argument argument( argc, argv );
    kvs::visclient::ParticleServer server;
    kvs::visclient::Command command( &server );
    kvs::visclient::ComThread comthread( &command );

    SetCropParameter( &command.m_parameter.m_parameter_crop );
    InitializeSignalHandler();


// APPEND START fp)m.tanaka 2013.09.01
    g_command = &command;
//	kvs::visclient::Command command3( &server);
// APPEND END   fp)m.tanaka 2013.09.01

// APPEND START BY)M.Tanaka 2015.03.03
    if ( argument.m_repeat_level_detailed_flag )
    {
        repeatleveld = argument.m_repeat_level_detailed;
    }
    if ( argument.m_subpixel_level_detailed_flag )
    {
        subpixelleveld = argument.m_subpixel_level_detailed;
    }
    if ( argument.m_particle_limit_flag )
    {
        plimitlevel = argument.m_particle_limit;
    }
    if ( argument.m_particle_density_flag )
    {
        pdensitylevel = argument.m_particle_density;
    }
// APPEND START BY)M.Tanaka 2015.03.11
    if ( argument.m_sampling_type_f )
    {
        if ( strcmp( argument.m_sampling_type.c_str(), "u" ) == 0 )
        {
//printf("***** samplingtype:u\n");
            samplingtypelevel = kvs::visclient::VisualizationParameter::UniformSampling;
        }
        else if ( strcmp( argument.m_sampling_type.c_str(), "r" ) == 0 )
        {
//printf("***** samplingtype:r\n");
            samplingtypelevel = kvs::visclient::VisualizationParameter::RejectionSampling;
        }
        else if ( strcmp( argument.m_sampling_type.c_str(), "m" ) == 0 )
        {
//printf("***** samplingtype:m\n");
            samplingtypelevel = kvs::visclient::VisualizationParameter::MetropolisSampling;
        }
    }
    if ( argument.m_detailed_particle_transfer_type_flag )
    {
        if ( strcmp( argument.m_detailed_particle_transfer_type.c_str(), "all" ) == 0 )
        {
//printf("***** transfertype:all\n");
            detailed_particle_transfer_typelevel = kvs::visclient::VisualizationParameter::Summalized;
        }
        else if ( strcmp( argument.m_detailed_particle_transfer_type.c_str(), "div" ) == 0 )
        {
//printf("***** transfertype:div\n");
            detailed_particle_transfer_typelevel = kvs::visclient::VisualizationParameter::Divided;
        }
    }
    printf( "\n\n\n***** 0 shading:%s\n", shadinglevel );
    if ( argument.m_shading_type_flag )
    {
        strcpy( shadinglevel, argument.m_shading_type.c_str() );
        printf( "***** 1 shading:%s\n", shadinglevel );
    }
// APPEND END   BY)M.Tanaka 2015.03.11

    command.m_parameter.m_particle_limit = plimitlevel;
    command.m_parameter.m_particle_density = pdensitylevel ;
    command.m_parameter.m_detailed_repeat_level = repeatleveld;
    command.m_parameter.m_detailed_subpixel_level = subpixelleveld;
// APPEND START BY)M.Tanaka 2015.03.11
    command.m_parameter.m_sampling_type = static_cast<kvs::visclient::VisualizationParameter::SamplingType>( samplingtypelevel );
    command.m_parameter.m_detailed_transfer_type = static_cast<kvs::visclient::VisualizationParameter::DetailedParticleTransferType>( detailed_particle_transfer_typelevel );
// APPEND END   BY)M.Tanaka 2015.03.11

    sscanf( argument.m_viewer.c_str(), "%fx%f", &resolution_width_level, &resolution_height_level );
// APPEND END   BY)M.Tanaka 2015.03.03

    command.m_parameter.m_abstract_repeat_level = argument.m_repeat_level_abstract;
    command.m_parameter.m_abstract_subpixel_level = argument.m_subpixel_level_abstract;
    command.m_parameter.m_transfer_function = argument.m_transfer_function;

    command.m_parameter.m_hostname = argument.m_hostname;
    command.m_parameter.m_port = argument.m_port;
    command.m_parameter.m_shading_type_flag = argument.m_shading_type_flag;

    /*
    #if defined(_WIN32)
            resolution_height_device = PBVR_RESOLUTION_HEIGHT_DEVICE;
    #else
    #ifdef __APPLE__
            char *cmdline1 = "xrandr -q | grep maximum |awk '{print $14}'";
    #else
    */
//#if not defined(WIN32) && not defined(__APPLE__)
#if !defined(WIN32) && !defined(__APPLE__)
	const char* cmdline1 = "timeout 5 xrandr -q | grep maximum |awk '{print $14}'";
//#endif
    FILE* fp;
    char buf[128];
    fp = popen( cmdline1, "r" );
    fgets( buf, 128, fp );
    pclose( fp );
    int nitem = sscanf( buf, "%d", &resolution_height_device );
//        std::cout<<"nitem " << nitem << std::endl;
//        std::cout<<"resolution_height_device " << resolution_height_device << std::endl;
    if ( nitem < 1 )
    {
        std::cout << "No X-Server." << std::endl;
        exit( 1 );
    }
#endif
#if 0
    if ( resolution_height_level > resolution_height_device - PBVR_DOCK_H )
    {
        resolution_height_level = resolution_height_device - PBVR_DOCK_H;
    }
#endif
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );

// MODIFIED START BY)M.Tanaka 2015.03.03
//  screen.setGeometry( 0, 0, 620, 620 );
    int i_w = ( int )( ( long )resolution_width_level );
    int i_h = ( int )( ( long )resolution_height_level );
    screen.setGeometry( 0, 0, i_w, i_h );
// MODIFIED END   BY)M.Tanaka 2015.03.03

    command.m_screen = &screen;
    command.m_parameter.m_camera = screen.camera();

// APPEND START fp)m.tanaka 2013.09.01
#include "v2newpanel.h"
    if ( argument.m_parameter_file != "" )
    {
        VizParameterFile::ReadParamFile( argument.m_parameter_file.c_str(), 0 );
        m_Legend_UI.inportFile( argument.m_parameter_file.c_str() );
    }
//	Win3vd();
//	extfedit.create_window();
    kvs::visclient::TransferFunctionEditorMain tfe( &command, &screen );
    ptfe_main = &tfe; // Add Fj 2015.03.05
    tfe.show();
    tfe.hide(); // Add FEAST 2016.2.5
    // APPEND START Fj 2015.03.16
    if ( argument.m_parameter_file != "" )
    {
        ptfe_main->importFileWithInit( argument.m_parameter_file.c_str() );
    }
    if ( argument.m_transfer_function_file_flag )
    {
        ptfe_main->importFileWithInit( argument.m_transfer_function_file_name );
    }
    // APPEND END Fj 2015.03.16
// APPEND END   fp)m.tanaka 2013.09.01
// APPEND START fp)m.tanaka 2013.09.01
    g_screen = &screen;
// APPEND END   fp)m.tanaka 2013.09.01
    if ( argument.m_server_subvolume_directory != "" )
    {
        command.m_parameter.m_input_directory = argument.m_server_subvolume_directory;
    }
    else
    {
        command.m_parameter.m_input_directory = buff_volumedata;
    }

    if (!command.m_parameter.m_input_directory.empty()) {
    // APPEND START BY)M.Tanaka 2015.03.03
        strcpy( pfi_path_server, command.m_parameter.m_input_directory.c_str() );
    // APPEND END   BY)M.Tanaka 2015.03.03
        g_volumedata->set_text( command.m_parameter.m_input_directory.c_str() );
        command.m_parameter.m_hasfin = false;
    }
    else {
        pfi_path_server[0] = 0x00;
    }

    // add by @hira at 2016/12/01
    command.m_parameter.m_filter_parameter_filename = argument.m_filter_parameter_filename;
    if (!command.m_parameter.m_filter_parameter_filename.empty()) {
        strcpy( filter_parameter_filename, command.m_parameter.m_filter_parameter_filename.c_str() );
        g_volumedata->set_text( command.m_parameter.m_filter_parameter_filename.c_str() );
        command.m_parameter.m_hasfin = true;
    }
    else {
        filter_parameter_filename[0] = 0x00;
    }

    if ( argument.m_header != "" )
    {
        command.m_parameter.m_particle_directory = argument.m_header;
        command.m_parameter.m_client_server_mode = 0;
    }

    // pin引数を反映してUIを作成
    for ( int i = 0; i < 10; i++ )
    {
        if ( argument.m_pin_option_directory[i].empty() != true )
        {
            command.m_parameter.m_particle_merge_param.m_particles[i + 1].m_file_path.append( argument.m_pin_option_directory[i] );
        }
    }
    m_Particle_UI.createPanel( command.m_parameter.m_client_server_mode, &command.m_parameter.m_particle_merge_param );
    // 粒子の表示・非表示情報を内部変数に反映
    m_Particle_UI.setParam( &command.m_parameter.m_particle_merge_param );
    command.m_particle_merge_ui = &m_Particle_UI;
    // Serverのstep範囲情報を得る
    ret = command.getServerParticleInfomation( &command.m_parameter, &command.m_result );
    if (ret < 0) {
        exit(1);		// add by @hira at 2016/12/01 : 接続エラーの場合終了する
    }
    ptfe_main->apply();

    // pin引数で指定されたファイルを全てチェックしてstepの範囲を得る
    kvs::visclient::ParticleMerger tmp_merger;
    tmp_merger.setParam( command.m_parameter.m_particle_merge_param, command.m_parameter.m_min_server_time_step, command.m_parameter.m_max_server_time_step );
    /*
    // 表示するものだけフラグを立てる(表示するファイルだけstep範囲の情報がほしいため)
    for (int i = 0; i < 10; i++)
    {
        if (argument.m_pin_option_directory[i].empty() != true)
        {
            tmp_merger.setenable( i + 1 );
        }
    }
    */
    int local_time_start = tmp_merger.getMergedInitialTimeStep();
    int local_time_end = tmp_merger.getMergedFinalTimeStep();

// MODIFIED START  fp)m.tanaka 2013.09.01
    command.initializeAbstractParticles( &command.m_parameter, &command.m_result, local_time_start, local_time_end );
//    command.generateAbstractParticles( command.m_parameter, &command.result, local_time_start, local_time_end ); /* modified 20140123 */
    command.generateDetailedParticles();
//	command3.generateAbstractParticles();
//	command3.generateDetailedParticles();
// MODIFIED END    fp)m.tanaka 2013.09.01
//#ifndef __APPLE__
    Win3vd();
    std::cout << " ### Win3vd() done " << std::endl;
    if ( argument.m_parameter_file != "" )
    {
        VizParameterFile::UpdateWin3v( argument.m_parameter_file.c_str() );
    }
//#endif
    FILEpanelCreate();

    kvs::glut::Screen screen2( &app );
    screen2.setTitle( "Communication" );
// MODIFIED START  fp)m.tanaka 2013.09.01
    //screen2.setGeometry( 0, 620, 512, 620 );
    //screen2.setGeometry( 0, 620, 256, 250 );
    screen2.setGeometry( 0, 620, 286, 250 );
// MODIFIED END    fp)m.tanaka 2013.09.01
    //screen2.show(); // Comment out by Fj 2015.03.04

// MODIFIED START  fp)m.tanaka 2013.09.01
//	kvs::visclient::Controller m_controller( &screen2, &command );
    kvs::visclient::Controller m_controller( &screen2, &m_screen3, &command );
// tantan
    command.m_screen3 = &m_screen3;
// MODIFIED END    fp)m.tanaka 2013.09.01
    m_controller.show();

    command.m_controller = &m_controller;

// Comment Out by Fj 20150304
//	kvs::visclient::TransferFunctionEditor m_transfer_functionEditor(&screen);
//	kvs::StructuredVolumeObject* dummyObject = new kvs::HydrogenVolumeData( kvs::Vector3ui(32, 32, 32) );
//	m_transfer_functionEditor.setVolumeObject( dummyObject );
//	m_transfer_functionEditor.show();

    command.startDetailedAnalysis(); /* same as press detaild button 2014.07.21 */

// MODIFY START fp)m.tanaka 2013.09.01
//	std::auto_ptr<kvs::visclient::ProgressBar> m_progress555( new kvs::visclient::ProgressBar( &screen ) );
    std::unique_ptr<kvs::visclient::ProgressBar> m_progress( new kvs::visclient::ProgressBar( &m_screen3 ) );
// MODIFY END   fp)m.tanaka 2013.09.01
    m_progress->setX( 0 );
    m_progress->setY( 0 );
    m_progress->setWidth( 500 );
    m_progress->setValue( 0 );
    m_progress->setCaption( "Progress" );
    m_progress->setTextColor( kvs::RGBColor( 0, 0, 0 ) );
    m_progress->show();

// MODIFY START fp)m.tanaka 2013.09.01
//	std::auto_ptr<kvs::visclient::TimeStepSlider> m_time_slider( new kvs::visclient::TimeStepSlider( &screen, &command ) );
    std::unique_ptr<kvs::visclient::TimeStepSlider> m_time_slider( new kvs::visclient::TimeStepSlider( &m_screen3, &command ) );
// MODIFY END   fp)m.tanaka 2013.09.01
    m_time_slider->setX( 0 );
// MODIFY START fp)K.YAJIMA 2015.03.16
    //m_time_slider->setY( 50 );
    m_time_slider->setY( 70 );
// MODIFY END fp)K.YAJIMA 2015.03.16
    m_time_slider->setWidth( 500 );
    m_time_slider->setValue( 0 );
//	m_time_slider->setRange(0,99);
// 	m_time_slider->setRange(0,command.m_parameter.m_max_time_step); // MODIFY by Fj 2015.03.04
    m_time_slider->setRange( command.m_parameter.m_min_time_step, command.m_parameter.m_max_time_step ); // MODIFY by Fj 2015.03.04
    m_time_slider->setCaption( "Time step" );
    m_time_slider->setTextColor( kvs::RGBColor( 0, 0, 0 ) );
    m_time_slider->show();

// MODIFY START fp)m.tanaka 2013.09.01
//	std::auto_ptr<kvs::visclient::TimeSegmentMinSlider> segment1( new kvs::visclient::TimeSegmentMinSlider( &screen, &command ) );
    std::unique_ptr<kvs::visclient::TimeSegmentMinSlider> segment1( new kvs::visclient::TimeSegmentMinSlider( &m_screen3, &command ) );
// MODIFY END   fp)m.tanaka 2013.09.01
    segment1->setX( 0 );
// MODIFY START fp)m.tanaka 2013.09.01
    //segment1->setY( 450 );
// MODIFY START fp)K.YAJIMA 2015.03.16
    //segment1->setY( 100 );
    segment1->setY( 140 );
// MODIFY END fp)K.YAJIMA 2015.03.16
// MODIFY END   fp)m.tanaka 2013.09.01
    segment1->setWidth( 500 );
    segment1->setValue( command.m_parameter.m_min_time_step );
//	segment1->setRange(0,98);
//	segment1->setRange(0,command.m_parameter.m_max_time_step); // MODIFY by Fj 2015.03.04
    segment1->setRange( command.m_parameter.m_min_time_step, command.m_parameter.m_max_time_step ); // MODIFY by Fj 2015.03.04
//	segment1->setCaption("Min Time(Detailed)");
    segment1->setCaption( "Min Time" );
    segment1->setTextColor( kvs::RGBColor( 0, 0, 0 ) );
    segment1->show();

// MODIFY START fp)m.tanaka 2013.09.01
//	std::auto_ptr<kvs::visclient::TimeSegmentMaxSlider> segment2( new kvs::visclient::TimeSegmentMaxSlider( &screen, &command ) );
    std::unique_ptr<kvs::visclient::TimeSegmentMaxSlider> segment2( new kvs::visclient::TimeSegmentMaxSlider( &m_screen3, &command ) );
// MODIFY END   fp)m.tanaka 2013.09.01

    segment2->setX( 0 );
// MODIFY START fp)m.tanaka 2013.09.01
    //segment2->setY( 500 );
// MODIFY START fp)K.YAJIMA 2015.03.16
    //segment2->setY( 150 );
    segment2->setY( 210 );
// MODIFY END fp)K.YAJIMA 2015.03.16
// MODIFY END   fp)m.tanaka 2013.09.01
    segment2->setWidth( 500 );
//	segment2->setValue(1);
    segment2->setValue( command.m_parameter.m_max_time_step );
//	segment2->setRange(1,99);
//	segment2->setRange(0,command.m_parameter.m_max_time_step); // MODIFY by Fj 2015.03.04
    segment2->setRange( command.m_parameter.m_min_time_step, command.m_parameter.m_max_time_step ); // MODIFY by Fj 2015.03.04
//	segment2->setCaption("Max Time(Detailed)");
    segment2->setCaption( "Max Time" );
    segment2->setTextColor( kvs::RGBColor( 0, 0, 0 ) );
    segment2->show();

// MODIFY START  fp)h.arai 2015.02.26
    std::unique_ptr<kvs::visclient::StartDetailedAnalysisButton> detail( new kvs::visclient::StartDetailedAnalysisButton( &m_screen3, &command ) );
    detail->setButtonColor( ::Gray );
    detail->setTextColor( kvs::RGBColor( 255, 255, 255 ) );
    detail->setX( 0 );
    detail->setY( 230 );
    detail->setWidth( 250 );
    detail->setMargin( 10 );
    detail->setCaption( "Detailed analysis" );

    std::unique_ptr<kvs::visclient::StartAbstractAnalysisButton> abst( new kvs::visclient::StartAbstractAnalysisButton( &m_screen3, &command ) );
    abst->setButtonColor( ::Gray );
    abst->setTextColor( kvs::RGBColor( 255, 255, 255 ) );
    abst->setX( 245 );
    abst->setY( 230 );
    abst->setWidth( detail->width() );
    abst->setMargin( 10 );
    abst->setCaption( "Abstract analysis" );

    std::unique_ptr<kvs::visclient::StopButton> stop( new kvs::visclient::StopButton( &m_screen3, &command ) );
    if ( command.m_parameter.m_client_server_mode == 1 )
    {
        stop->setButtonColor( ::Red );
    }
    else
    {
        stop->setButtonColor( ::Gray );
    }
    stop->setTextColor( kvs::RGBColor( 255, 255, 255 ) );
    stop->setX( 0 );
// MODIFY START fp)K.YAJIMA 2015.03.16
    //stop->setY( 205 );
    stop->setY( 285 );
// MODIFY END fp)K.YAJIMA 2015.03.16
    stop->setWidth( abst->width() );
    stop->setMargin( 10 );
    stop->setCaption( "Start/Stop" );
    stop->show();
// MODIFY END  fp)h.arai 2015.02.26
    m_screen3.show();

    command.m_progress = m_progress.get();
    command.m_time_slider = m_time_slider.get();
    command.m_min_time_slider = segment1.get();
    command.m_max_time_slider = segment2.get();
    command.m_stop_button = stop.get();

    // Create Time Step Label
    kvs::visclient::StepLabel label( &command );
    int yl0 = i_h - PBVR_I_H_OFFSET;
    label.setPosition( i_w * 1 / 31, yl0 );
    g_label = &label;

    label.setTextColor( kvs::RGBColor( 255, 255, 255 ) );
    label.show();

    kvs::PointObject* object1 = command.m_abstract_particles[0];

    kvs::ValueArray<kvs::Real32> coords( 3 );
    kvs::RGBColor color( 0, 0, 0 );
    kvs::ValueArray<kvs::Real32> normals( 3 );
    kvs::PointObject* control_object = new kvs::PointObject( coords, color, normals, 1.0 );
    const kvs::Vector3f& min = object1->minObjectCoord();
    const kvs::Vector3f& max = object1->maxObjectCoord();
    control_object->setMinMaxObjectCoords( min, max );
    control_object->setMinMaxExternalCoords( min, max );

// APPEND START FP)M.TANAKA 2015.03.16
    g_control_object = control_object;
// APPEND END   FP)M.TANAKA 2015.03.16

    command.m_renderer = new kvs::visclient::ExtendedParticleVolumeRenderer( *object1, command.m_parameter.m_detailed_subpixel_level, command.m_parameter.m_detailed_repeat_level );
    if ( !command.m_renderer )
    {
        kvsMessageError( "Cannot creat a point m_renderer." );
    }

    kvs::visclient::FPSLabel fpsLabel( &screen, *command.m_renderer );
    fpsLabel.setPosition( i_w * 51 / 124, yl0 );
    fpsLabel.setTextColor( kvs::RGBAColor( 255, 255, 255 ) );
    fpsLabel.show();
    g_fpsLabel = &fpsLabel;

    // For shading.
    command.m_renderer->disableShading();
// APPEND START BY)M.Tanaka 2015.03.11
    float sd_ka, sd_kd, sd_ks, sd_n;
    if ( strcmp( shadinglevel, "L" ) == 0 )
    {
        printf( "***** shading : L\n" );
        command.m_renderer->enableShading();
        command.m_renderer->setShader( kvs::Shader::Lambert() );
    }
    else if ( strcmp( shadinglevel, "P" ) == 0 )
    {
        printf( "***** shading : P\n" );
        command.m_renderer->enableShading();
        command.m_renderer->setShader( kvs::Shader::Phong() );
    }
    else if ( strcmp( shadinglevel, "B" ) == 0 )
    {
        printf( "***** shading : B\n" );
        command.m_renderer->enableShading();
        command.m_renderer->setShader( kvs::Shader::BlinnPhong() );
    }
    else if ( strncmp( shadinglevel, "L,", 2 ) == 0 )
    {
        command.m_renderer->enableShading();
        sscanf( &shadinglevel[2], "%f,%f", &sd_ka, &sd_kd );
        printf( "***** shading : L %f %f\n", sd_ka, sd_kd );
        command.m_renderer->setShader( kvs::Shader::Lambert( sd_ka, sd_kd ) );
    }
    else if ( strncmp( shadinglevel, "P,", 2 ) == 0 )
    {
        command.m_renderer->enableShading();
        sscanf( &shadinglevel[2], "%f,%f,%f,%f", &sd_ka, &sd_kd, &sd_ks, &sd_n );
        printf( "***** shading : P %f %f %f %f\n", sd_ka, sd_kd, sd_ks, sd_n );
        command.m_renderer->setShader( kvs::Shader::Phong( sd_ka, sd_kd, sd_ks, sd_n ) );
    }
    else if ( strncmp( shadinglevel, "B,", 2 ) == 0 )
    {
        command.m_renderer->enableShading();
        sscanf( &shadinglevel[2], "%f,%f,%f,%f", &sd_ka, &sd_kd, &sd_ks, &sd_n );
        printf( "***** shading : B %f %f %f %f\n", sd_ka, sd_kd, sd_ks, sd_n );
        command.m_renderer->setShader( kvs::Shader::BlinnPhong( sd_ka, sd_kd, sd_ks, sd_n ) );
    }
// APPEND END   BY)M.Tanaka 2015.03.11
#ifndef CPUMODE
    command.m_renderer->disableLODControl();
#endif
    kvs::visclient::TimerEvent timer_event( &command, &comthread );

    // Legend (Add by FEAST 2016.02.08)
    kvs::visclient::LegendBar legendBar( &screen, command );
    legendBar.setOrientation( kvs::glut::LegendBar::Vertical );
    legendBar.screenResizedAfterSelectTransferFunction( 0 );
    legendBar.setEventType(
        kvs::EventBase::PaintEvent |
        kvs::EventBase::ResizeEvent );
    legendBar.hide();
    g_legend = &legendBar;
    m_Legend_UI.createPanel( &legendBar, &screen );
    // Create Orientation Box
    kvs::glut::OrientationAxis orientation_axis( &screen );
    g_orientation_axis = &orientation_axis;
    orientation_axis.setPosition( i_w * 53 / 62 - 30, yl0 - 30 );
    orientation_axis.setBoxType( kvs::glut::OrientationAxis::SolidBox );
    orientation_axis.show();
    // Create ViewerSetting Panel
    m_Viewer_UI.createPanel( g_label, g_fpsLabel, g_legend, g_orientation_axis, &screen );
    m_Coodinate_UI.createPanel( &screen );
    command.m_coord_panel_ui = &m_Coodinate_UI;

    // Timer
    int msec = 100;
    std::string interval( "TIMER_EVENT_INTERVAL" );

    if ( getenv( interval.c_str() ) != NULL )
    {
        msec = std::atoi( getenv( interval.c_str() ) );
    }

    interval_msec->set_int_val( msec );
    kvs::glut::Timer* glut_timer = new kvs::glut::Timer( msec );
    glut_timer->setInterval( msec );
    glut_timer->start();

    command.m_glut_timer = glut_timer;

    // Create KeyPressEvent
    kvs::visclient::KeyPressEvent key_press_event;
    screen.addKeyPressEvent( &key_press_event );

// APPEND START FP)M.TANAKA 2015.03.16
    g_timer_event = &timer_event;
// APPEND START FP)M.TANAKA 2015.03.16

#ifdef CPUMODE
    std::pair<int, int> id_pair;
    id_pair = screen.registerObject( control_object, command.m_renderer );
    timer_event.setObject( id_pair.first );
    timer_event.setRenderer( command.m_renderer );
#else
    screen.registerObject( control_object, command.m_renderer );
#endif
    screen.addTimerEvent( &timer_event, glut_timer );
    screen.setTitle( "PBVR" );
//    screen.background()->setColor( kvs::RGBColor( 200, 200, 210 ) );
    screen.background()->setColor( crgb );
    screen.show();

// APPEND START fp)m.tanaka 2014.05.20
    PaintEvent  paint_event;
    screen.addPaintEvent( &paint_event );

// APPEND START fp)m.takizawa 2014.05.28
//        AnimationControl( &screen );
// APPEND END fp)m.takizawa 2014.05.28

    strcpy( ValueImageFile, argument.m_image_base_name.c_str() );
// APPEND START fp)m.takizawa 2014.05.28
    AnimationControl( &screen );
// APPEND END fp)m.takizawa 2014.05.28

    glutSetWindow( AnimationControlWinID );
    glutHideWindow();

// System Memory Monitor
    UpdateCPUMemory update_smemory;
//   UpdateGPUMemory update_gmemory;
    kvs::Mutex mutex;
    update_smemory.m_mutex = &mutex;
//   update_gmemory.mutex = &mutex;
    update_smemory.start();
//   update_gmemory.start();
    ret = app.run();
    return ret;
}

// APPEND START fp)m.tanaka 2013.09.01
#include "v2foot.h"
// APPEND END   fp)m.tanaka 2013.09.01
