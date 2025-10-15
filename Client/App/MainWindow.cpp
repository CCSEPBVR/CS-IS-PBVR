#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow( kvs::qt::Application& app, QWidget *parent )
    : QMainWindow(parent)
    , ui( new Ui::MainWindow )    
    , m_screen( new kvs::qt::jaea::Screen( &app ) )
    , m_compositor( new kvs::StochasticRenderingCompositor( m_screen->scene() ) )
    , m_color_map_bar( nullptr )
    , m_orientation_axis( nullptr )
    , m_fps_label( nullptr )
    , m_time_step_label( nullptr )
    , m_web_binary_socket( new QWebSocket() )
    , m_web_text_socket( new QWebSocket() )
    , m_communication( new Communication( m_screen, m_web_binary_socket, m_web_text_socket, this ) )
{
    initialize();
}

MainWindow::~MainWindow()
{
    delete m_web_text_socket;
    delete m_web_binary_socket;
    delete m_time_step_label;
    delete m_fps_label;
    delete m_orientation_axis;
    delete m_color_map_bar;
    delete m_compositor;
    delete m_screen;
    delete ui;
}

void MainWindow::initialize()
{    
    ui->setupUi( this ); // UIのセットアップ
    setWindowTitle( QString("pbvr_client - %1").arg( GIT_BRANCH_NAME ) );

    m_compositor->setRepetitionLevel( 4 ); // コンポジターのリピートレベルを設定 初期値:4
    m_screen->setEvent( m_compositor );

    m_screen->setFixedSize( 620, 620 );
    ui->screenArea->addWidget( m_screen );

    m_color_map_bar = new kvs::ColorMapBar( m_screen );
    m_orientation_axis = new kvs::OrientationAxis( m_screen, m_screen->scene() );
    m_fps_label = new kvs::Label( m_screen );
    m_time_step_label = new kvs::Label( m_screen );

    m_web_binary_socket->setParent( this );
    m_web_text_socket->setParent( this );

    communicationInitialize();

#ifdef OPENXR_SCREEN
    m_vr_listener = new VRHandControllerListener( m_screen );
    m_screen->addEvent( m_vr_listener );
    // connect( m_vr_listener, &VRHandControllerListener::drawPlotOverLine, m_plot_over_line, &PlotOverLine::drawPlotOverLine );
    // connect( m_vr_listener, &VRHandControllerListener::showHidePlotOverLine, m_plot_over_line, &PlotOverLine::showHidePlotOverLine );
    connect( m_vr_listener, &VRHandControllerListener::sendVRSharePoint, m_communication, &Communication::onVRSharePoint );
#endif

    this->show();

    QString shaderDIR = QDir( QCoreApplication::applicationDirPath() ).filePath( "Shader" );
    kvs::ShaderSource::AddSearchPath( shaderDIR.toUtf8().constData() );

    QString fontDIR = QDir(QCoreApplication::applicationDirPath()).filePath("Font/");

    struct FontInfo
    {
        const char* name;
        const char* file;
    };

    FontInfo fonts[] = {
        { "SansRegular",      "NotoSans-Regular.ttf" },
        { "SansItalic",       "NotoSans-Italic.ttf" },
        { "SansBold",         "NotoSans-Bold.ttf" },
        { "SansBoldItalic",   "NotoSans-BoldItalic.ttf" },
        { "SerifRegular",     "NotoSerif-Regular.ttf" },
        { "SerifItalic",      "NotoSerif-Italic.ttf" },
        { "SerifBold",        "NotoSerif-Bold.ttf" },
        { "SerifBoldItalic",  "NotoSerif-BoldItalic.ttf" },
        { "Icon",             "entypo.ttf" }
    };

    for( const auto& f : fonts )
    {
        QString fullPath = fontDIR + f.file;
        m_screen->paintDevice()->textEngine()->addFont( f.name, fullPath.toUtf8().constData() );
    }
}

void MainWindow::communicationInitialize()
{
    if( m_communication != nullptr )
    {
        m_communication->adjustSize();
        addDockWidget( Qt::RightDockWidgetArea, m_communication );
    }
}
