#include "Preference.h"
#include "ui_Preference.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QPalette>

#include <kvs/ParticleBasedRenderer>
#include <kvs/StochasticPolygonRenderer>
Preference::Preference(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preference),
    m_settings( "config.ini", QSettings::IniFormat )
{
    ui->setupUi(this);
    ui->orientationTypeCBox->addItem( "Horizontal", Horizontal );
    ui->orientationTypeCBox->addItem( "Vertical", Vertical );

    ui->axisTypeCBox->addItem( "CorneredAxis", CorneredAxis );
    ui->axisTypeCBox->addItem( "CenteredAxis", CenteredAxis );
    ui->axisTypeCBox->addItem( "NoneAxis", NoneAxis );

    ui->boxTypeCBox->addItem( "WiredBox", WiredBox );
    ui->boxTypeCBox->addItem( "SolidBox", SolidBox );
    ui->boxTypeCBox->addItem( "NoneBox", NoneBox );

    ui->showFPSCBox->addItem( "Show", Show );
    ui->showFPSCBox->addItem( "Hide", Hide );

    ui->showTimeStepCBox->addItem( "Show", Show );
    ui->showTimeStepCBox->addItem( "Hide", Hide );

    connect( ui->selectedBackGroundColorCLbl, &ClickableLabel::doubleClicked, this, &Preference::onBackGroundColorDoubleClicked );
    connect( ui->selectedLabelsColorCLbl, &ClickableLabel::doubleClicked, this, &Preference::onLabelsColorDoubleClicked );
    connect( ui->applyPBtn,  &QPushButton::clicked, this, &Preference::onApplyButtonClicked  );
    connect( ui->cancelPBtn, &QPushButton::clicked, this, &Preference::onCancelButtonClicked );
    connect( ui->okPBtn,     &QPushButton::clicked, this, &Preference::onOKButtonClicked     );
}

Preference::~Preference()
{
    delete ui;
}

void Preference::initialize()
{
    //config.iniが存在する場合は設定の読み込みを行う。
    if( checkConfigFileExists() )
    {
        loadColorMapBarSettings();
        loadOrientationAxisSettings();
        loadBackGroundColorSettings();
        loadResolutionSettings();
        loadLabelsSettings();
        loadShadingSettings();
    }
    //存在しない場合はデフォルトの値を設定し保存する。
    else
    {
        setDefaultSettings();
        saveSettings();
    }

    m_color_map_bar->anchorToTopLeft();
    m_orientation_axis->anchorToBottomRight();

    applySettings( true );
}

void Preference::loadColorMapBarSettings()
{
    m_settings.beginGroup( "ColorMapBar" );
    const bool isShowing = m_settings.value( "isShowing" ).toBool();
    const QString orientationType = m_settings.value( "Orientation" ).toString();

    if( isShowing )
    {
        ui->colorMapBarGBox->setChecked( true );
    }
    else
    {
        ui->colorMapBarGBox->setChecked( false );
    }

    if( orientationType == "Horizontal" )
    {
        ui->orientationTypeCBox->setCurrentIndex( Horizontal );
    }
    else if( orientationType == "Vertical" )
    {
        ui->orientationTypeCBox->setCurrentIndex( Vertical );
    }
    m_settings.endGroup();
}

void Preference::loadOrientationAxisSettings()
{
    m_settings.beginGroup( "OrientationAxis" );
    const bool isShowing = m_settings.value( "isShowing" ).toBool();
    const QString axisType = m_settings.value( "AxisType" ).toString();
    const QString boxType = m_settings.value( "BoxType" ).toString();

    if( isShowing )
    {
        ui->orientationGBox->setChecked( true );
    }
    else
    {
        ui->orientationGBox->setChecked( false );
    }

    if( axisType == "CorneredAxis" )
    {
        ui->axisTypeCBox->setCurrentIndex( CorneredAxis );
    }
    else if( axisType == "CenteredAxis" )
    {
        ui->axisTypeCBox->setCurrentIndex( CenteredAxis );
    }
    else if( axisType == "NoneAxis" )
    {
        ui->axisTypeCBox->setCurrentIndex( NoneAxis );
    }

    if( boxType == "WiredBox" )
    {
        ui->boxTypeCBox->setCurrentIndex( WiredBox );
    }
    else if( boxType == "SolidBox" )
    {
        ui->boxTypeCBox->setCurrentIndex( SolidBox );
    }
    else if( boxType == "NoneBox" )
    {
        ui->boxTypeCBox->setCurrentIndex( NoneAxis );
    }
    m_settings.endGroup();
}

void Preference::loadBackGroundColorSettings()
{
    m_settings.beginGroup( "BackGroundColor" );
    QColor color( m_settings.value( "R" ).toInt(),m_settings.value( "G" ).toInt(),m_settings.value( "B" ).toInt() );
    setBackGroundColor( color );
    m_settings.endGroup();
}

void Preference::loadResolutionSettings()
{
    m_settings.beginGroup( "Resolution" );
    const int width = m_settings.value( "width" ).toInt();
    const int height = m_settings.value( "height" ).toInt();
    ui->heightSBox->setValue( width );
    ui->widthSBox->setValue( height );
    m_settings.endGroup();
}

void Preference::loadLabelsSettings()
{
    m_settings.beginGroup( "Labels" );
    const bool fpsIsShowing = m_settings.value( "fpsIsShowing" ).toBool();
    const bool timeStepIsShowing = m_settings.value( "timeStepIsShowing" ).toBool();
    QColor color( m_settings.value( "R" ).toInt(),m_settings.value( "G" ).toInt(),m_settings.value( "B" ).toInt() );

    if( fpsIsShowing )
    {
        ui->showFPSCBox->setCurrentIndex( Show );
    }
    else
    {
        ui->showFPSCBox->setCurrentIndex( Hide );
    }

    if( timeStepIsShowing )
    {
        ui->showTimeStepCBox->setCurrentIndex( Show );
    }
    else
    {
        ui->showTimeStepCBox->setCurrentIndex( Hide );
    }

    setLabelsColor( color );
    m_settings.endGroup();
}
void Preference::loadShadingSettings()
{
    m_settings.beginGroup( "Shading" );
    const bool isEnable = m_settings.value( "isEnable" ).toBool();
    const QString shadingType = m_settings.value( "ShadingType" ).toString();
    const double ka = m_settings.value( "ka" ).toDouble();
    const double kd = m_settings.value( "kd" ).toDouble();
    const double ks = m_settings.value( "ks" ).toDouble();
    const double s = m_settings.value( "s" ).toDouble();

    if( isEnable )
    {
        ui->shadingGBox->setChecked( true );
    }
    else
    {
        ui->shadingGBox->setChecked( false );
    }

    if( shadingType == "Lambert" )
    {
        ui->lambertRBtn->setChecked( true );
    }
    else if( shadingType == "Phong" )
    {
        ui->phongRBtn->setChecked( true );
    }
    else if( shadingType == "Blinn" )
    {
        ui->BlinnRBtn->setChecked( true );
    }

    ui->kaDSBox->setValue( ka );
    ui->kdDSBox->setValue( kd );
    ui->ksDSBox->setValue( ks );
    ui->sDSBox->setValue( s );
}

void Preference::setBackGroundColor( const QColor& color )
{
    if( color.isValid() )
    {
        QPalette palette = ui->selectedBackGroundColorCLbl->palette();
        palette.setColor( QPalette::Window, color );

        ui->selectedBackGroundColorCLbl->setAutoFillBackground(true);
        ui->selectedBackGroundColorCLbl->setPalette(palette);
        ui->selectedBackGroundColorCLbl->update();
    }
}

void Preference::setLabelsColor( const QColor& color )
{
    if( color.isValid() )
    {
        QPalette palette = ui->selectedLabelsColorCLbl->palette();
        palette.setColor( QPalette::Window, color );

        ui->selectedLabelsColorCLbl->setAutoFillBackground(true);
        ui->selectedLabelsColorCLbl->setPalette(palette);
        ui->selectedLabelsColorCLbl->update();
    }
}

void Preference::setDefaultSettings()
{
    //ColorMapBar
    ui->orientationTypeCBox->setCurrentIndex( Horizontal );
    //OrientationAxis
    ui->axisTypeCBox->setCurrentIndex( CorneredAxis );
    ui->boxTypeCBox->setCurrentIndex( SolidBox );
    //BackGroundColor
    setBackGroundColor( QColor( 82, 87, 110 ) );
    //Resolution
    ui->widthSBox->setValue( 620 );
    ui->heightSBox->setValue( 620 );
    //Labels
    ui->showFPSCBox->setCurrentIndex( Hide );
    ui->showTimeStepCBox->setCurrentIndex( Hide );
    setLabelsColor( QColor( 0, 0, 0 ) );
    //Shading
    ui->shadingGBox->setChecked( false );
    ui->phongRBtn->setChecked( true );
}

void Preference::saveSettings()
{
    m_settings.beginGroup( "ColorMapBar" );
    m_settings.setValue( "isShowing", ui->colorMapBarGBox->isChecked() );
    m_settings.setValue( "Orientation", ui->orientationTypeCBox->currentText() );
    m_settings.endGroup();

    m_settings.beginGroup( "OrientationAxis" );
    m_settings.setValue( "isShowing", ui->orientationGBox->isChecked() );
    m_settings.setValue( "AxisType", ui->axisTypeCBox->currentText() );
    m_settings.setValue( "BoxType", ui->boxTypeCBox->currentText() );
    m_settings.endGroup();

    m_settings.beginGroup( "BackGroundColor" );
    m_settings.setValue( "R", ui->selectedBackGroundColorCLbl->palette().color(QPalette::Window).red());
    m_settings.setValue( "G", ui->selectedBackGroundColorCLbl->palette().color(QPalette::Window).green());
    m_settings.setValue( "B", ui->selectedBackGroundColorCLbl->palette().color(QPalette::Window).blue());
    m_settings.endGroup();

    m_settings.beginGroup( "Resolution" );
    m_settings.setValue( "width", ui->widthSBox->value() );
    m_settings.setValue( "height", ui->heightSBox->value() );
    m_settings.endGroup();

    m_settings.beginGroup( "Labels" );
    m_settings.setValue( "fpsIsShowing", ui->showFPSCBox->currentText() );
    m_settings.setValue( "timeStepIsShowing", ui->showTimeStepCBox->currentText() );
    m_settings.setValue( "R", ui->selectedLabelsColorCLbl->palette().color(QPalette::Window).red() );
    m_settings.setValue( "G", ui->selectedLabelsColorCLbl->palette().color(QPalette::Window).green() );
    m_settings.setValue( "B", ui->selectedLabelsColorCLbl->palette().color(QPalette::Window).blue() );
    m_settings.endGroup();

    m_settings.beginGroup( "Shading" );
    m_settings.setValue( "isEnable", ui->shadingGBox->isChecked() );
    if( ui->lambertRBtn->isChecked() )
    {
        m_settings.setValue( "ShadingType", "Lambert" );
    }
    else if( ui->phongRBtn->isChecked() )
    {
        m_settings.setValue( "ShadingType", "Phong" );
    }
    else if( ui->BlinnRBtn->isChecked() )
    {
        m_settings.setValue( "ShadingType", "Blinn" );
    }
    m_settings.setValue( "ka", ui->kaDSBox->value() );
    m_settings.setValue( "kd", ui->kdDSBox->value() );
    m_settings.setValue( "ks", ui->ksDSBox->value() );
    m_settings.setValue( "s", ui->sDSBox->value() );
    m_settings.endGroup();

    m_settings.sync();
}

void Preference::applySettings( bool isInit )
{
    applyColorMapBarSettings();
    applyOrientationAxisSettings();
    applyBackGroundColor();
    applyResolution();
    applyLabelsSettings();
    applyShadingSettings();

    if( !isInit )
    {
        saveSettings();
    }

    m_screen->update();
}

void Preference::applyColorMapBarSettings()
{
    const bool isShowing = ui->colorMapBarGBox->isChecked();
    const OrientationType orientationType = static_cast<OrientationType>( ui->orientationTypeCBox->currentIndex() );

    if( isShowing )
    {
        switch ( orientationType )
        {
        case Horizontal:
            m_color_map_bar->setOrientationToHorizontal();
            m_color_map_bar->setWidth( 200 );
            m_color_map_bar->setHeight( 60 );
            break;
        case Vertical:
            m_color_map_bar->setOrientationToVertical();
            m_color_map_bar->setWidth( 60 );
            m_color_map_bar->setHeight( 200 );
            break;
        default:
            break;
        }
        m_color_map_bar->show();
    }
    else
    {
        m_color_map_bar->hide();
    }
}

void Preference::applyOrientationAxisSettings()
{
    const bool isShowing = ui->orientationGBox->isChecked();
    const AxisType axisType = static_cast<AxisType>( ui->axisTypeCBox->currentIndex() );
    const BoxType boxType = static_cast<BoxType>( ui->boxTypeCBox->currentIndex() );

    if( isShowing )
    {
        switch ( axisType )
        {
        case CorneredAxis:
            m_orientation_axis->setAxisTypeToCornered();
            break;
        case CenteredAxis:
            m_orientation_axis->setAxisTypeToCentered();
            break;
        case NoneAxis:
            m_orientation_axis->setAxisType( kvs::OrientationAxis::NoneAxis );
            break;
        default:
            break;
        }

        switch ( boxType )
        {
        case WiredBox:
            m_orientation_axis->setBoxTypeToWired();
            break;
        case SolidBox:
            m_orientation_axis->setBoxTypeToSolid();
            break;
        case NoneBox:
            m_orientation_axis->setBoxType( kvs::OrientationAxis::NoneBox );
        default:
            break;
        }
        m_orientation_axis->show();
    }
    else
    {
        m_orientation_axis->hide();
    }
}

void Preference::applyBackGroundColor()
{
    const kvs::RGBColor backGroundColor(
        ui->selectedBackGroundColorCLbl->palette().color(QPalette::Window).red(),
        ui->selectedBackGroundColorCLbl->palette().color(QPalette::Window).green(),
        ui->selectedBackGroundColorCLbl->palette().color(QPalette::Window).blue());

    m_screen->setBackgroundColor( kvs::RGBColor( backGroundColor ) );
}

void Preference::applyResolution()
{
    const int width = ui->widthSBox->value();
    const int height = ui->heightSBox->value();

    m_screen->setFixedSize( width, height );
}

void Preference::applyLabelsSettings()
{
    const bool fpsIsShowing = static_cast<LabelsVisible>(ui->showFPSCBox->currentIndex());
    const bool timeStepIsShowing = static_cast<LabelsVisible>(ui->showTimeStepCBox->currentIndex());

    const kvs::RGBColor labelsColor(
        ui->selectedLabelsColorCLbl->palette().color(QPalette::Window).red(),
        ui->selectedLabelsColorCLbl->palette().color(QPalette::Window).green(),
        ui->selectedLabelsColorCLbl->palette().color(QPalette::Window).blue());

    kvs::Font font;
    font.setColor( labelsColor );

    if( fpsIsShowing )
    {
        m_fps_label->setPosition( 20, 580 );
        m_fps_label->setFont( font );
        m_fps_label->screenUpdated( [&]()
                                   {
                                       const auto fps = kvs::String::From( m_compositor->timer().fps(), 4 );
                                       m_fps_label->setText( std::string( "FPS:" + fps).c_str());
                                   });
        m_fps_label->show();
    }
    else
    {
        m_fps_label->hide();
    }

    if( timeStepIsShowing )
    {
        m_time_step_label->setPosition( 100, 580 );
        m_time_step_label->setFont( font );
        m_time_step_label->setText("Time step: ");
        m_time_step_label->show();
    }
    else
    {
        m_time_step_label->hide();
    }
}

void Preference::applyShadingSettings()
{
    bool isEnable = ui->shadingGBox->isChecked();

    if( m_initialized )
    {
        if( isEnable )
        {
            //Sceneに登録されているオブジェクトの数分ループする
            for(int i = 1; i < m_screen->scene()->numberOfObjects(); i++ )
            {
                if( m_screen->scene()->hasObject(i) )
                {
                    //PointObjectの場合
                    if( strcmp( "kvs::glsl::ParticleBasedRenderer", m_screen->scene()->renderer(i)->moduleName()) == 0 )
                    {
                        kvs::glsl::ParticleBasedRenderer* particleBasedRenderer = new  kvs::glsl::ParticleBasedRenderer();
                        if( ui->lambertRBtn->isChecked() )
                        {
                            particleBasedRenderer->setShader( kvs::Shader::Lambert( ui->kaDSBox->value(), ui->kdDSBox->value() ) );
                        }
                        else if( ui->phongRBtn->isChecked() )
                        {
                            particleBasedRenderer->setShader( kvs::Shader::Phong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value()) );

                        }
                        else if( ui->BlinnRBtn->isChecked() )
                        {
                            particleBasedRenderer->setShader( kvs::Shader::BlinnPhong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value()) );
                        }
                        m_screen->scene()->replaceRenderer( i, particleBasedRenderer );
                    }
                    //PolygonObjectの場合
                    else if(strcmp("kvs::StochasticPolygonRenderer", m_screen->scene()->renderer(i)->moduleName()) == 0)
                    {
                        kvs::StochasticPolygonRenderer* stochasticPolygonRenderer = new kvs::StochasticPolygonRenderer();
                        if( ui->lambertRBtn->isChecked() )
                        {
                            stochasticPolygonRenderer->setShader( kvs::Shader::Lambert( ui->kaDSBox->value(), ui->kdDSBox->value() ) );
                        }
                        else if( ui->phongRBtn->isChecked() )
                        {
                            stochasticPolygonRenderer->setShader( kvs::Shader::Phong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value()) );

                        }
                        else if( ui->BlinnRBtn->isChecked() )
                        {
                            stochasticPolygonRenderer->setShader( kvs::Shader::BlinnPhong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value()) );
                        }
                        m_screen->scene()->replaceRenderer( i, stochasticPolygonRenderer );
                    }
                }
            }
        }
        //シェーディングをオフにする。
        else
        {
            //Sceneに登録されているオブジェクトの数分ループする
            for(int i = 1; i < m_screen->scene()->numberOfObjects(); i++ )
            {
                //PointObjectの場合
                if( strcmp( "kvs::glsl::ParticleBasedRenderer", m_screen->scene()->renderer(i)->moduleName()) == 0 )
                {
                    kvs::glsl::ParticleBasedRenderer* particleBasedRenderer = new  kvs::glsl::ParticleBasedRenderer();
                    particleBasedRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
                    m_screen->scene()->replaceRenderer( i, particleBasedRenderer );
                }
                //PolygonObjectの場合
                else if( strcmp("kvs::StochasticPolygonRenderer", m_screen->scene()->renderer(i)->moduleName()) == 0 )
                {
                    kvs::StochasticPolygonRenderer* stochasticPolygonRenderer = new kvs::StochasticPolygonRenderer();
                    stochasticPolygonRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
                    m_screen->scene()->replaceRenderer( i, stochasticPolygonRenderer );
                }
            }
        }
    }
}

void Preference::closeEvent( QCloseEvent* event )
{
    loadColorMapBarSettings();
    loadOrientationAxisSettings();
    loadBackGroundColorSettings();
    loadResolutionSettings();
    loadLabelsSettings();
    loadShadingSettings();

    event->accept();
}

void Preference::onBackGroundColorDoubleClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"));
    setBackGroundColor( color );
}

void Preference::onLabelsColorDoubleClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"));
    setLabelsColor( color );
}

void Preference::onApplyButtonClicked()
{
    applySettings( false );
}

void Preference::onCancelButtonClicked()
{
    close();
}

void Preference::onOKButtonClicked()
{
    applySettings( false );
    close();
}
