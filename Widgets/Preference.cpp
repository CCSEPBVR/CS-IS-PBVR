#include "Preference.h"
#include "ui_Preference.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QPalette>

#include <kvs/ParticleBasedRenderer>

#include <kvs/StochasticPolygonRenderer>
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
#include <kvs/StochasticTexturedPolygonRenderer>
#endif
Preference::Preference(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preference),
    m_screen( nullptr ),
    m_compositor( nullptr ),
    m_color_map_bar( nullptr ),
    m_orientation_axis( nullptr ),
    m_fps_label( nullptr ),
    m_time_step_label( nullptr ),
    m_settings( "config.ini", QSettings::IniFormat ),
    m_current_time_step( 0 )
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

    ui->showFPSCBox->addItem( "Show", QVariant( true ) );
    ui->showFPSCBox->addItem( "Hide", QVariant( false ) );

    ui->showTimeStepCBox->addItem( "Show", QVariant( true ) );
    ui->showTimeStepCBox->addItem( "Hide", QVariant( false ) );

    connect( ui->backGroundColorCLbl, &ClickableLabel::doubleClicked, this, &Preference::onBackGroundColorDoubleClicked );
    connect( ui->fontColorCLbl, &ClickableLabel::doubleClicked, this, &Preference::onLabelsColorDoubleClicked );
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
        loadFontSettings();
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

Preference::ShaderType Preference::getShaderType()
{
    if( ui->shadingGBox->isChecked() == false )
    {
        return Preference::NoShader;
    }
    else if( ui->lambertRBtn->isChecked() )
    {
        return Preference::LambertShading;
    }
    else if( ui->phongRBtn->isChecked() )
    {
        return Preference::Phong;
    }
    else if( ui->BlinnRBtn->isChecked() )
    {
        return Preference::BlinnPhong;
    }
    else
    {
        return Preference::NoShader;
    }
}

void Preference::applyShading(kvs::RendererBase*& rendererBase)
{
    if (auto* stochasticRenderer = dynamic_cast<kvs::StochasticRendererBase*>(rendererBase))
    {
        if (auto* stochasticPolygonRenderer = dynamic_cast<kvs::StochasticPolygonRenderer*>(stochasticRenderer))
        {
            kvs::StochasticPolygonRenderer* copy = new kvs::StochasticPolygonRenderer;
            copy->DownCast(stochasticPolygonRenderer);
            switch (getShaderType())
            {
            case Preference::ShaderType::LambertShading:
                copy->setShader(getLambertShader());
                break;
            case Preference::ShaderType::Phong:
                copy->setShader(getPhongShader());
                break;
            case Preference::ShaderType::BlinnPhong:
                copy->setShader(getBlinnPhongShader());
                break;
            case Preference::ShaderType::NoShader:
            default:
                copy->setShader(kvs::Shader::Lambert(1, 0));
                break;
            }
            rendererBase = copy; // copyオブジェクトをrendererBaseに代入
        }
        else if (auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>(stochasticRenderer))
        {
            kvs::glsl::ParticleBasedRenderer* copy = new kvs::glsl::ParticleBasedRenderer;
            copy->DownCast(particleRenderer);
            switch (getShaderType())
            {
            case Preference::ShaderType::LambertShading:
                copy->setShader(getLambertShader());
                break;
            case Preference::ShaderType::Phong:
                copy->setShader(getPhongShader());
                break;
            case Preference::ShaderType::BlinnPhong:
                copy->setShader(getBlinnPhongShader());
                break;
            case Preference::ShaderType::NoShader:
            default:
                copy->setShader(kvs::Shader::Lambert(1, 0));
                break;
            }
            copy->enableShuffle();
            rendererBase = copy; // copyオブジェクトをrendererBaseに代入
        }
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
        else if (auto* stochasticTexturedPolygonRenderer = dynamic_cast<kvs::StochasticTexturedPolygonRenderer*>(stochasticRenderer) )
        {
            kvs::StochasticTexturedPolygonRenderer* copy = new kvs::StochasticTexturedPolygonRenderer;
            copy->DownCast(stochasticPolygonRenderer);
            switch (getShaderType())
            {
            case Preference::ShaderType::LambertShading:
                copy->setShader(getLambertShader());
                break;
            case Preference::ShaderType::Phong:
                copy->setShader(getPhongShader());
                break;
            case Preference::ShaderType::BlinnPhong:
                copy->setShader(getBlinnPhongShader());
                break;
            case Preference::ShaderType::NoShader:
            default:
                copy->setShader(kvs::Shader::Lambert(1, 0));
                break;
            }
            rendererBase = copy; // copyオブジェクトをrendererBaseに代入
        }
#endif
    }
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
    ui->widthSBox->setValue( width );
    ui->heightSBox->setValue( height );

    m_settings.endGroup();
}

void Preference::loadLabelsSettings()
{
    m_settings.beginGroup( "Labels" );
    const bool fpsIsShowing = m_settings.value( "fpsIsShowing" ).toBool();
    const bool timeStepIsShowing = m_settings.value( "timeStepIsShowing" ).toBool();

    if( fpsIsShowing )
    {
        ui->showFPSCBox->setCurrentText( "Show" );
    }
    else
    {
        ui->showFPSCBox->setCurrentText( "Hide" );
    }

    if( timeStepIsShowing )
    {
        ui->showTimeStepCBox->setCurrentText( "Show" );
    }
    else
    {
        ui->showTimeStepCBox->setCurrentText( "Hide" );
    }

    m_settings.endGroup();
}

void Preference::loadFontSettings()
{
    m_settings.beginGroup( "Font" );
    QColor color( m_settings.value( "R" ).toInt(),m_settings.value( "G" ).toInt(),m_settings.value( "B" ).toInt() );
    setFontColor( color );
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

    m_lambert_shader.Ka = ka;
    m_lambert_shader.Kd = kd;
    m_lambert_shader.Ks = ks;
    m_lambert_shader.S = s;

    m_phong_shader.Ka = ka;
    m_phong_shader.Kd = kd;
    m_phong_shader.Ks = ks;
    m_phong_shader.S = s;

    m_blinn_phong_shader.Ka = ka;
    m_blinn_phong_shader.Kd = kd;
    m_blinn_phong_shader.Ks = ks;
    m_blinn_phong_shader.S = s;

    m_settings.endGroup();
}

void Preference::setBackGroundColor( const QColor& color )
{
    if( color.isValid() )
    {
        QPalette palette = ui->backGroundColorCLbl->palette();
        palette.setColor( QPalette::Window, color );

        ui->backGroundColorCLbl->setAutoFillBackground(true);
        ui->backGroundColorCLbl->setPalette(palette);
        ui->backGroundColorCLbl->update();
    }
}

void Preference::setFontColor( const QColor& color )
{
    if( color.isValid() )
    {
        QPalette palette = ui->fontColorCLbl->palette();
        palette.setColor( QPalette::Window, color );

        ui->fontColorCLbl->setAutoFillBackground(true);
        ui->fontColorCLbl->setPalette(palette);
        ui->fontColorCLbl->update();
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
    ui->showFPSCBox->setCurrentText( "Hide" );
    ui->showTimeStepCBox->setCurrentText( "Hide" );
    //Font
    setFontColor( QColor( 0, 0, 0 ) );
    //Shading
    ui->shadingGBox->setChecked( false );
    ui->phongRBtn->setChecked( true );
}

void Preference::saveSettings()
{
    saveColorMapBarSettings();
    saveOrientationAxisSettings();
    saveBackGroundColorSettings();
    saveResolutionSettings();
    saveLabelsSettings();
    saveFontSettings();
    saveShadingSettings();

    m_settings.sync();
}

void Preference::saveColorMapBarSettings()
{
    m_settings.beginGroup( "ColorMapBar" );
    m_settings.setValue( "isShowing", ui->colorMapBarGBox->isChecked() );
    m_settings.setValue( "Orientation", ui->orientationTypeCBox->currentText() );
    m_settings.endGroup();
}

void Preference::saveOrientationAxisSettings()
{
    m_settings.beginGroup( "OrientationAxis" );
    m_settings.setValue( "isShowing", ui->orientationGBox->isChecked() );
    m_settings.setValue( "AxisType", ui->axisTypeCBox->currentText() );
    m_settings.setValue( "BoxType", ui->boxTypeCBox->currentText() );
    m_settings.endGroup();

}

void Preference::saveBackGroundColorSettings()
{
    m_settings.beginGroup( "BackGroundColor" );
    m_settings.setValue( "R", ui->backGroundColorCLbl->palette().color(QPalette::Window).red());
    m_settings.setValue( "G", ui->backGroundColorCLbl->palette().color(QPalette::Window).green());
    m_settings.setValue( "B", ui->backGroundColorCLbl->palette().color(QPalette::Window).blue());
    m_settings.endGroup();
}

void Preference::saveResolutionSettings()
{
    m_settings.beginGroup( "Resolution" );
    m_settings.setValue( "width", ui->widthSBox->value() );
    m_settings.setValue( "height", ui->heightSBox->value() );
    m_settings.endGroup();
}

void Preference::saveLabelsSettings()
{
    m_settings.beginGroup( "Labels" );
    m_settings.setValue( "fpsIsShowing", ui->showFPSCBox->currentData().toBool() );
    m_settings.setValue( "timeStepIsShowing", ui->showTimeStepCBox->currentData().toBool() );
    m_settings.endGroup();
}

void Preference::saveFontSettings()
{
    m_settings.beginGroup( "Font" );
    m_settings.setValue( "R", ui->fontColorCLbl->palette().color(QPalette::Window).red() );
    m_settings.setValue( "G", ui->fontColorCLbl->palette().color(QPalette::Window).green() );
    m_settings.setValue( "B", ui->fontColorCLbl->palette().color(QPalette::Window).blue() );
    m_settings.endGroup();
}

void Preference::saveShadingSettings()
{
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
}

void Preference::applySettings( bool isInit )
{
    applyColorMapBarSettings();
    applyOrientationAxisSettings();
    applyBackGroundColor();
    applyResolution();
    applyLabelsSettings();
    applyFontSettings();
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

    m_color_map_bar->setCaption( ui->captionLEdit->text().toStdString() );

    if( isShowing )
    {
        switch ( orientationType )
        {
        case Horizontal:
            m_color_map_bar->setOrientationToHorizontal();
            m_color_map_bar->setWidth( 200 );
            m_color_map_bar->setHeight( 30 );
            break;
        case Vertical:
            m_color_map_bar->setOrientationToVertical();
            m_color_map_bar->setWidth( 30 );
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
        ui->backGroundColorCLbl->palette().color(QPalette::Window).red(),
        ui->backGroundColorCLbl->palette().color(QPalette::Window).green(),
        ui->backGroundColorCLbl->palette().color(QPalette::Window).blue());

    m_screen->setBackgroundColor( kvs::RGBColor( backGroundColor ) );
}

void Preference::applyResolution()
{
    const int width = ui->widthSBox->value();
    const int height = ui->heightSBox->value();
    m_screen->setSize( width, height );
    m_screen->setFixedSize( width, height );
    m_screen->scene()->resizeFunction( width, height, m_screen->screen()->devicePixelRatio() );
}

void Preference::applyLabelsSettings()
{
    const bool fpsIsShowing = ui->showFPSCBox->currentData().toBool();
    const bool timeStepIsShowing = ui->showTimeStepCBox->currentData().toBool();

    if( fpsIsShowing )
    {
        m_fps_label->setPosition( 20, 580 );        
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
        m_time_step_label->setText( "Time step: " + std::to_string( m_current_time_step ) );
        m_time_step_label->show();
    }
    else
    {
        m_time_step_label->hide();
    }    
}

void Preference::applyFontSettings()
{
    const kvs::RGBColor labelsColor(
        ui->fontColorCLbl->palette().color(QPalette::Window).red(),
        ui->fontColorCLbl->palette().color(QPalette::Window).green(),
        ui->fontColorCLbl->palette().color(QPalette::Window).blue());

    kvs::Font font;
    font.setColor( labelsColor );
    m_color_map_bar->setFont( font );
    m_fps_label->setFont( font );
    m_time_step_label->setFont( font );
}

void Preference::applyShadingSettings()
{
    m_lambert_shader.Ka = ui->kaDSBox->value();
    m_lambert_shader.Kd = ui->kdDSBox->value();

    m_phong_shader.Ka = ui->kaDSBox->value();
    m_phong_shader.Kd = ui->kdDSBox->value();
    m_phong_shader.Ks = ui->ksDSBox->value();
    m_phong_shader.S = ui->sDSBox->value();

    m_blinn_phong_shader.Ka = ui->kaDSBox->value();
    m_blinn_phong_shader.Kd = ui->kdDSBox->value();
    m_blinn_phong_shader.Ks = ui->ksDSBox->value();
    m_blinn_phong_shader.S = ui->sDSBox->value();

    if( m_screen->scene()->objectManager()->hasObject() )
    {
        const int size = m_screen->scene()->IDManager()->size();
        for( int index = 0; index < size; index++ )
        {
            auto id = m_screen->scene()->IDManager()->id( index );
            auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );
            applyShading( rendererBase );
            m_screen->scene()->replaceRenderer( id.second, rendererBase );
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
    loadFontSettings();
    loadShadingSettings();

    event->accept();
}

void Preference::onBackGroundColorDoubleClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"));
    setBackGroundColor( color );

    raise();
}

void Preference::onLabelsColorDoubleClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"));
    setFontColor( color );

    raise();
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
