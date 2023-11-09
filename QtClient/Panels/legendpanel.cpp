#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "legendpanel.h"
#include "ui_legendpanel.h"

#include <fstream>
#include <QGlue/extCommand.h>
#include <QGlue/renderarea.h>

#include "Client/LogManager.h"

LegendPanel* LegendPanel::instance = 0;
LegendPanel::LegendPanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::LegendPanel)
{
    ui->setupUi(this);
    if(instance!=0){
        qWarning("LegendPanel:: allows only one instance");
        delete instance;
    }
    instance=this;
    m_legend_display_value[0] = 0;
    m_divisionision_rgb_color.set( 0, 0, 0 );
    m_franeline_rgb_color.set( 0, 0, 0 );

    const QString qstr_caption = QString::fromStdString(m_string_caption);
    ui->caption->setText(qstr_caption);
    ui->frameThickness->setValue(this->m_frameline_thickness);
    ui->divThickness->setValue(this->m_intervals_thickness);
    ui->divInterval->setValue(this->m_intervals_division);

    m_legend_frame_color = new ColorEditDialog( this );
    m_legend_division_color =new ColorEditDialog( this );
    this->setVisible(true);


    // Selects one of multiple possible overloaded signals for this Objects
    void(QSpinBox::*intChanged)(int) = &QSpinBox::valueChanged;
    void(QComboBox::*selectionChanged)(int)= &QComboBox::currentIndexChanged;

    connect(ui->displayLegend, &QGroupBox::toggled, this, &LegendPanel::onDisplayLegendChanged);
    connect(ui->colormapFunction,selectionChanged,this,&LegendPanel::onColormapFunctionIndexChanged);
    connect(ui->layoutDirection,selectionChanged,this,&LegendPanel::setOrientation);
    //    connect(ui->caption,&QLineEdit::editingFinished,this,&LegendPanel::on_caption_textEdited);
    connect(ui->divInterval,intChanged,this,&LegendPanel::setDivision);
    connect(ui->divThickness,intChanged,this,&LegendPanel::setDivisionThickness);

    connect(ui->divColor_btn,&QPushButton::clicked,this,&LegendPanel::onDivColorChanged);
    connect(ui->frameThickness,intChanged,this,&LegendPanel::setFranelineThickness);
    connect(ui->frameColor_btn,&QPushButton::clicked,this,&LegendPanel::onFrameColorBtnClicked);
    connect(ui->setButton,&QPushButton::clicked,this,&LegendPanel::onSetBtnClicked);
}


LegendPanel::~LegendPanel()
{
    delete ui;
}

/**
 * @brief LegendPanel::bindLegendBar, bind the legendbar instance
 * @param lbar
 */
void LegendPanel::bindLegendBar(QGlue::LegendBar* lbar)
{

    this->m_legend=lbar;

    ui->layoutDirection->setCurrentIndex(m_bar_val);

    if ( m_legend )
    {
        m_legend->setDivisionLineColor( m_divisionision_rgb_color );
        m_legend->setBorderColor( m_franeline_rgb_color );
        selectTransferFunction();
        setDisplay();
        setOrientation();
        setDivision();
        setDivisionThickness();
        setFranelineThickness();
        this->setCaption();

    }

    this->populateColormapFunctions(extCommand);
}

/**
 * @brief LegendPanel::set2UI member to UI
 */
void LegendPanel::set2UI()
{
    // modify by @hira at 2016/12/01
    // m_list_color_map->set_int_val( m_selected_color_map );
    if (m_legend != NULL) {
        this->setColorMapFunction(extCommand);
    }
    ui->colormapFunction->setCurrentIndex(m_selected_color_map );
    ui->layoutDirection->setCurrentIndex(m_bar_val );
    ui->caption->setText(m_string_caption );
    ui->divInterval->setValue( m_intervals_division );
    ui->divThickness->setValue( m_intervals_thickness );
    ui->frameThickness->setValue( m_frameline_thickness );

    if ( m_legend )
    {
        selectTransferFunction();
        setDisplay();
        setOrientation();
        setDivision();
        setDivisionThickness();
        setFranelineThickness();
        this->setCaption();
        m_legend->setDivisionLineColor( m_divisionision_rgb_color );
        m_legend->setBorderColor( m_franeline_rgb_color );
    }
}
/**
 * @brief LegendPanel::populateColormapFunctions populate the colormap function list
 * @param command
 */
void LegendPanel::populateColormapFunctions(const Command* command)
{

    if (command == NULL) return;
    const kvs::visclient::ExtendedTransferFunctionMessage *m_doc =
            &command->m_parameter.m_parameter_extend_transfer_function;
#ifdef COMM_MODE_CS
    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
            m_doc->m_color_transfer_function;
#else
    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
//            m_doc->m_transfer_function;
            m_doc->m_color_transfer_function;
#endif
    int max_color = 0;
    std::vector<NamedTransferFunctionParameter>::const_iterator itr;
    for (itr=m_color_transfer_function.begin(); itr!=m_color_transfer_function.end(); itr++) {
        const char *name = itr->m_name.c_str();
        int n_color = atoi(name+1);
        if (max_color < n_color) max_color = n_color;
        ui->colormapFunction->addItem(name,(int)NULL);
    }
}


/**
 * @brief LegendPanel::setColorMapFunction, sets the colormap function
 * @param command
 */
void LegendPanel::setColorMapFunction(const kvs::visclient::Command* command)
{
    if (command == NULL) return;
    const kvs::visclient::ExtendedTransferFunctionMessage *m_doc =
            &command->m_parameter.m_parameter_extend_transfer_function;
#ifdef COMM_MODE_CS
    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
            m_doc->m_color_transfer_function;
#else
    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
//            m_doc->m_transfer_function;
            m_doc->m_color_transfer_function;
#endif
    int max_color = 0;
    std::vector<NamedTransferFunctionParameter>::const_iterator itr;
    ui->colormapFunction->clear();
    for (itr=m_color_transfer_function.begin(); itr!=m_color_transfer_function.end(); itr++) {
        const char *name = itr->m_name.c_str();
        int n_color = atoi(name+1);
        if (max_color < n_color) max_color = n_color;
        ui->colormapFunction->addItem(name);
    }
    //    this->m_spinner_color_map->set_int_limits(1, max_color);
    extCommand->m_screen->update();
}

/**
 * @brief LegendPanel::importFile import parameter file
 * @param fname
 * @return
 */
int LegendPanel::importFile( const char* fname )
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

        if ( strncmp( s, "LEGEND", 6 ) != 0 ) // LEGENDで始まらないキーワードはpassする
        {
            continue;
        }
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
        if ( strcmp( k[i], "LEGEND_DISPLAY" ) == 0 )
        {
            int tmp = atoi( v[i] );
            m_legend_display_value[0] = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FUNCTION" ) == 0 )
        {
            // m_selected_color_map = atoi( v[i] );
            m_selected_color_map = atoi( v[i]+1 );
        }
        if ( strcmp( k[i], "LEGEND_POSITION" ) == 0 )
        {
            m_bar_val = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_CAPTION" ) == 0 )
        {
            strcpy( m_string_caption, v[i] );
        }
        if ( strcmp( k[i], "LEGEND_INTERVALS" ) == 0 )
        {
            m_intervals_division = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_THICKNESS" ) == 0 )
        {
            m_intervals_thickness = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_COLOR_R" ) == 0 )
        {
            kvs::UInt8 r = atoi( v[i] );
            kvs::RGBColor tmp( r, m_divisionision_rgb_color.g(), m_divisionision_rgb_color.b() );
            m_divisionision_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_COLOR_G" ) == 0 )
        {
            kvs::UInt8 g = atoi( v[i] );
            kvs::RGBColor tmp( m_divisionision_rgb_color.r(), g, m_divisionision_rgb_color.b() );
            m_divisionision_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_INTERVAL_COLOR_B" ) == 0 )
        {
            kvs::UInt8 b = atoi( v[i] );
            kvs::RGBColor tmp( m_divisionision_rgb_color.r(), m_divisionision_rgb_color.g(), b );
            m_divisionision_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_THICKNESS" ) == 0 )
        {
            m_frameline_thickness = atoi( v[i] );
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_COLOR_R" ) == 0 )
        {
            kvs::UInt8 r = atoi( v[i] );
            kvs::RGBColor tmp( r, m_franeline_rgb_color.g(), m_franeline_rgb_color.b() );
            m_franeline_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_COLOR_G" ) == 0 )
        {
            kvs::UInt8 g = atoi( v[i] );
            kvs::RGBColor tmp( m_franeline_rgb_color.r(), g, m_franeline_rgb_color.b() );
            m_franeline_rgb_color = tmp;
        }
        if ( strcmp( k[i], "LEGEND_FRAMELINE_COLOR_B" ) == 0 )
        {
            kvs::UInt8 b = atoi( v[i] );
            kvs::RGBColor tmp( m_franeline_rgb_color.r(), m_franeline_rgb_color.g(), b );
            m_franeline_rgb_color = tmp;
        }
    }
    extCommand->m_screen->update();
    return 0;
}

/**
 * @brief LegendPanel::selectTransferFunction, sync transfer function
 */
void LegendPanel::selectTransferFunction()
{
    m_legend->screenResizedAfterSelectTransferFunction( ui->colormapFunction->currentIndex()+1);
    extCommand->m_screen->update();
}

/**
 * @brief LegendPanel::setDisplay sync display of legend
 */
void LegendPanel::setDisplay()
{
    if ( m_legend_display_value[0] == 0 )
    {
        //        m_legend->hide();
    }
    else
    {
        m_legend->show();
    }
    extCommand->m_screen->update();
}

/**
 * @brief LegendPanel::setOrientation sync orientation of legend
 */
void LegendPanel::setOrientation()
{
    LogManager lg;
    if ( ui->layoutDirection->currentIndex()==0 )
    {
        m_legend->setOrientation( QGlue::LegendBar::Vertical );
        if( m_isRec )
        {
            lg.Write("legendpanel.cpp", __func__, "LegendOrientation", "Vertical");
        }
    }
    else
    {
        m_legend->setOrientation( QGlue::LegendBar::Horizontal );
        if( m_isRec )
        {
            lg.Write("legendpanel.cpp", __func__, "LegendOrientation", "Horizontal");
        }
    }
    extCommand->m_screen->update();
}

/**
 * @brief LegendPanel::setDivision
 */
void LegendPanel::setDivision()
{
    int val = ui->divInterval->value();
    m_legend->setNumberOfDivisions( val );
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("legendpanel.cpp", __func__, "divisionInterval", std::to_string(val));
    }
}

/**
 * @brief LegendPanel::setDivisionThickness
 */
void LegendPanel::setDivisionThickness()
{
    int val = ui->divThickness->value();
    m_legend->setDivisionLineWidth( val );
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("legendpanel.cpp", __func__, "divisionThickness", std::to_string(val));
    }
}

/**
 * @brief LegendPanel::setDivisionColor
 */
void LegendPanel::setDivisionColor()
{
    m_divisionision_rgb_color = m_legend_division_color->color();
    m_legend->setDivisionLineColor( m_divisionision_rgb_color );
    extCommand->m_screen->update();
}

/**
 * @brief LegendPanel::setFranelineThickness
 */
void LegendPanel::setFranelineThickness()
{
    int val = ui->frameThickness->value();
    m_legend->setBorderWidth( val );
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("legendpanel.cpp", __func__, "FramelineThickness", std::to_string(val));
    }
}

/**
 * @brief LegendPanel::setFranelineColor
 */
void LegendPanel::setFranelineColor()
{
    m_franeline_rgb_color = m_legend_frame_color->color();
    m_legend->setBorderColor( m_franeline_rgb_color );

}

/**
 * @brief LegendPanel::setCaption
 */
void LegendPanel::setCaption()
{
    QString qs= ui->caption->text();
    if (qs.length()>0){
        m_caption=qs.toStdString();
    }
    else{
        m_caption="";
    }
    m_legend->setCaption( m_caption);
    extCommand->m_screen->update();
}

//void LegendPanel::colsePanel()
//{
//    m_show_ui = false;
//    this->setHidden(true);
//}


/**
 * @brief LegendPanel::exportFile export parameter file
 * @param fname
 */
void LegendPanel::exportFile( const std::string& fname )
{
    std::ofstream ofs;
    ofs.open( fname.c_str(), std::ofstream::app );
    if ( ofs.fail() )
    {
        std::cerr << "Error: open file " << fname << std::endl;
        return;
    }
    int n_selected_color = ui->colormapFunction->currentIndex();
    char color_name[8] = {0x00};
    sprintf(color_name, "C%d", n_selected_color);

    ofs << "LEGEND_DISPLAY = " << m_legend_display_value[0] << std::endl;
    // modify by @hira at 2016/12/01 : LEGEND_FUNCTIONを数値から色関数名に変更する
    // ofs << "LEGEND_FUNCTION = " << m_list_color_map->get_int_val() << std::endl;
    ofs << "LEGEND_FUNCTION = " << std::string(color_name) << std::endl;
    ofs << "LEGEND_POSITION = " << ui->layoutDirection->currentIndex() << std::endl;
    ofs << "LEGEND_CAPTION = " << m_string_caption << std::endl;
    ofs << "LEGEND_INTERVALS = " << ui->divInterval->value() << std::endl;
    ofs << "LEGEND_INTERVAL_THICKNESS = " << ui->divThickness->value() << std::endl;
    ofs << "LEGEND_INTERVAL_COLOR_R = " << ( unsigned int )m_divisionision_rgb_color.r() << std::endl;
    ofs << "LEGEND_INTERVAL_COLOR_G = " << ( unsigned int )m_divisionision_rgb_color.g() << std::endl;
    ofs << "LEGEND_INTERVAL_COLOR_B = " << ( unsigned int )m_divisionision_rgb_color.b() << std::endl;
    ofs << "LEGEND_FRAMELINE_THICKNESS = " << ui->frameThickness->value() << std::endl;
    ofs << "LEGEND_FRAMELINE_COLOR_R = " << ( unsigned int )m_franeline_rgb_color.r() << std::endl;
    ofs << "LEGEND_FRAMELINE_COLOR_G = " << ( unsigned int )m_franeline_rgb_color.g() << std::endl;
    ofs << "LEGEND_FRAMELINE_COLOR_B = " << ( unsigned int )m_franeline_rgb_color.b() << std::endl;

    ofs.close();
}

void LegendPanel::onColormapFunctionIndexChanged(int index)
{
    qDebug("on_colormapFunction_currentIndexChanged");
    m_legend->screenResizedAfterSelectTransferFunction( ui->colormapFunction->currentIndex() +1 );
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("legendpanel.cpp", __func__, "ColormapFunction", std::to_string(index));
    }
}

void LegendPanel::onHorizontalToggled(bool checked)
{
    qDebug("on_horizontal_btn_toggled");
    extCommand->m_screen->update();
}

void LegendPanel::onCaptionEdited()
{
    qDebug("on_caption_textEdited");
    this->setCaption();
}

void LegendPanel::onDivIntervalChanged(int val)
{
    qDebug("on_divInterval_valueChanged");
    m_legend->setNumberOfDivisions( val );
    extCommand->m_screen->update();
}

void LegendPanel::onDivThicknessChanged(int val)
{
    qDebug("on_divThickness_valueChanged");
    m_legend->setDivisionLineWidth(val);
    extCommand->m_screen->update();
}

void LegendPanel::onDivColorChanged()
{
    qDebug("on_divColor_btn_clicked");
    kvs::RGBColor m_divisionision_rgb_color=this->m_legend_division_color->getColorSelection();
    m_legend->setDivisionLineColor( m_divisionision_rgb_color );
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write( "legendpanel.cpp",__func__, "kvs RGB Color", std::to_string(m_divisionision_rgb_color.r()) + "," + std::to_string(m_divisionision_rgb_color.g()) + "," + std::to_string(m_divisionision_rgb_color.b()));
    }
}

void LegendPanel::onFrameThicknessChanged(int val)
{
    qDebug("on_frameThickness_valueChanged");
    m_legend->setBorderWidth(val);
    extCommand->m_screen->update();
}

void LegendPanel::onFrameColorBtnClicked()
{
    qDebug("on_frameColor_btn_clicked");
    m_franeline_rgb_color=this->m_legend_division_color->getColorSelection();
    m_legend->setBorderColor( m_franeline_rgb_color );
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write( "legendpanel.cpp",__func__, "kvs RGB Color", std::to_string(m_franeline_rgb_color.r()) + "," + std::to_string(m_franeline_rgb_color.g()) + "," + std::to_string(m_franeline_rgb_color.b()));
    }
}

void LegendPanel::onSetBtnClicked()
{
    this->setDisplay();
    this->setOrientation();
    this->setCaption();
    this->setDivision();
    this->setDivisionThickness();
    this->setDivisionColor();
    this->setFranelineThickness();
    this->setFranelineColor();
    this->selectTransferFunction();
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write( "legendpanel.cpp",__func__, "setButton", "clicked");
    }
}

void LegendPanel::onDisplayLegendChanged(int val)
{
    m_legend->setVisible(val);
    extCommand->m_screen->update();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("legendpanel.cpp", __func__, "DisplayLegend", std::to_string(val));
    }
}

void LegendPanel::changeTFValue()
{
    if(!instance) return;
    Ui::LegendPanel* ui=instance->ui;

    int currentIndex = ui->colormapFunction->currentIndex();
    ui->colormapFunction->clear();
    if (extCommand == NULL) return;
    const kvs::visclient::ExtendedTransferFunctionMessage *m_doc =
            &extCommand->m_parameter.m_parameter_extend_transfer_function;
#ifdef COMM_MODE_CS
    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
            m_doc->m_color_transfer_function;
#else
    const std::vector<NamedTransferFunctionParameter> m_color_transfer_function =
            //            m_doc->m_transfer_function;
            m_doc->m_color_transfer_function;
#endif

    int max_color = 0;
    std::vector<NamedTransferFunctionParameter>::const_iterator itr;
    for (itr=m_color_transfer_function.begin(); itr!=m_color_transfer_function.end(); itr++) {
        const char *name = itr->m_name.c_str();
        int n_color = atoi(name+1);
        if (max_color < n_color) max_color = n_color;
        ui->colormapFunction->addItem(name,(int)NULL);
    }
    ui->colormapFunction->setCurrentIndex(currentIndex);
}

void LegendPanel::logPlay(std::string funcName,std::string str[])
{
    if(funcName.compare("setOrientation") == 0)
    {
        ui->layoutDirection->setCurrentIndex(0);
        this->setOrientation();
    }

    if(funcName.compare("setDivision") == 0)
    {
        ui->divInterval->setValue(std::stoi(str[0]));
        this->setDivision();
    }

    if(funcName.compare("setDivisionThickness") == 0)
    {
        ui->divThickness->setValue(std::stoi(str[0]));
        this->setDivisionThickness();
    }

    if(funcName.compare("setFranelineThickness") == 0)
    {
        ui->frameThickness->setValue(std::stoi(str[0]));
        this->setFranelineThickness();
    }

    if(funcName.compare("onColormapFunctionIndexChanged") == 0)
    {
        ui->colormapFunction->setCurrentIndex(std::stoi(str[0]));
    }

    if(funcName.compare("onDivColorChanged") == 0)
    {
        m_legend->setDivisionLineColor( kvs::RGBColor(std::stoi(str[0]),std::stoi(str[1]),std::stoi(str[2])) );
    }

    if(funcName.compare("onFrameColorBtnClicked") == 0)
    {
        m_legend->setBorderColor( kvs::RGBColor(std::stoi(str[0]),std::stoi(str[1]),std::stoi(str[2])) );
    }

    if(funcName.compare("onSetBtnClicked") == 0)
    {
        this->onSetBtnClicked();
    }

    if(funcName.compare("onDisplayLegendChanged") == 0)
    {
        this->onDisplayLegendChanged(std::stoi(str[0]));
    }
}

