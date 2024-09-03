#include "Preference.h"
#include "ui_Preference.h"

#include "App/pbvrgui.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QPalette>

#include <kvs/ParticleBasedRenderer>

#include <kvs/StochasticPolygonRenderer>
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
#include <kvs/StochasticTexturedPolygonRenderer>
#endif
Preference::Preference(QWidget *parent, PBVRGUI *pbvr_gui) :
    QDialog(parent),
    ui(new Ui::Preference),
    m_pbvr_gui( pbvr_gui ),
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
    }
    //存在しない場合はデフォルトの値を設定し保存する。
    else
    {
        setDefaultSettings();
        saveSettings();
    }

    m_pbvr_gui->colorMapBar()->anchorToTopLeft();
    m_pbvr_gui->orientationAxis()->anchorToBottomRight();

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
}

void Preference::saveSettings()
{
    saveColorMapBarSettings();
    saveOrientationAxisSettings();
    saveBackGroundColorSettings();
    saveResolutionSettings();
    saveLabelsSettings();
    saveFontSettings();

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

void Preference::applySettings( bool isInit )
{
    applyColorMapBarSettings();
    applyOrientationAxisSettings();
    applyBackGroundColor();
    applyResolution();
    applyLabelsSettings();
    applyFontSettings();

    if( !isInit )
    {
        saveSettings();
    }

    m_pbvr_gui->screen()->update();
}

void Preference::applyColorMapBarSettings()
{
    const bool isShowing = ui->colorMapBarGBox->isChecked();
    const OrientationType orientationType = static_cast<OrientationType>( ui->orientationTypeCBox->currentIndex() );

#ifdef Q_OS_WIN
    m_pbvr_gui->colorMapBar()->setCaption( ui->captionLEdit->text().toLocal8Bit().constData() );
#else
    m_pbvr_gui->colorMapBar()->setCaption( ui->captionLEdit->text().toStdString() );
#endif

    if( isShowing )
    {
        switch ( orientationType )
        {
        case Horizontal:
            m_pbvr_gui->colorMapBar()->setOrientationToHorizontal();
            m_pbvr_gui->colorMapBar()->setWidth( 200 );
            m_pbvr_gui->colorMapBar()->setHeight( 30 );
            break;
        case Vertical:
            m_pbvr_gui->colorMapBar()->setOrientationToVertical();
            m_pbvr_gui->colorMapBar()->setWidth( 30 );
            m_pbvr_gui->colorMapBar()->setHeight( 200 );
            break;
        default:
            break;
        }
        m_pbvr_gui->colorMapBar()->show();
    }
    else
    {
        m_pbvr_gui->colorMapBar()->hide();
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
            m_pbvr_gui->orientationAxis()->setAxisTypeToCornered();
            break;
        case CenteredAxis:
            m_pbvr_gui->orientationAxis()->setAxisTypeToCentered();
            break;
        case NoneAxis:
            m_pbvr_gui->orientationAxis()->setAxisType( kvs::OrientationAxis::NoneAxis );
            break;
        default:
            break;
        }

        switch ( boxType )
        {
        case WiredBox:
            m_pbvr_gui->orientationAxis()->setBoxTypeToWired();
            break;
        case SolidBox:
            m_pbvr_gui->orientationAxis()->setBoxTypeToSolid();
            break;
        case NoneBox:
            m_pbvr_gui->orientationAxis()->setBoxType( kvs::OrientationAxis::NoneBox );
        default:
            break;
        }
        m_pbvr_gui->orientationAxis()->show();
    }
    else
    {
        m_pbvr_gui->orientationAxis()->hide();
    }
}

void Preference::applyBackGroundColor()
{
    const kvs::RGBColor backGroundColor(
        ui->backGroundColorCLbl->palette().color(QPalette::Window).red(),
        ui->backGroundColorCLbl->palette().color(QPalette::Window).green(),
        ui->backGroundColorCLbl->palette().color(QPalette::Window).blue());

    m_pbvr_gui->screen()->setBackgroundColor( kvs::RGBColor( backGroundColor ) );
}

void Preference::applyResolution()
{
    const int width = ui->widthSBox->value();
    const int height = ui->heightSBox->value();
    m_pbvr_gui->screen()->setSize( width, height );
    m_pbvr_gui->screen()->setFixedSize( width, height );
    m_pbvr_gui->screen()->scene()->resizeFunction( width, height, m_pbvr_gui->screen()->screen()->devicePixelRatio() );
}

void Preference::applyLabelsSettings()
{
    const bool fpsIsShowing = ui->showFPSCBox->currentData().toBool();
    const bool timeStepIsShowing = ui->showTimeStepCBox->currentData().toBool();

    if( fpsIsShowing )
    {
        m_pbvr_gui->fpsLabel()->setPosition( 20, m_pbvr_gui->screen()->height() - 40 );
        m_pbvr_gui->fpsLabel()->screenUpdated( [&]()
                                   {
                                       const auto fps = kvs::String::From( m_pbvr_gui->compositor()->timer().fps(), 4 );
                                       m_pbvr_gui->fpsLabel()->setText( std::string( "FPS:" + fps).c_str());
                                   });
        m_pbvr_gui->fpsLabel()->show();
    }
    else
    {
        m_pbvr_gui->fpsLabel()->hide();
    }

    if( timeStepIsShowing )
    {
        m_pbvr_gui->timeStepLabel()->setPosition( 100, m_pbvr_gui->screen()->height() - 40 );
        m_pbvr_gui->timeStepLabel()->setText( "Time step: " + std::to_string( m_current_time_step ) );
        m_pbvr_gui->timeStepLabel()->show();
    }
    else
    {
        m_pbvr_gui->timeStepLabel()->hide();
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
    m_pbvr_gui->colorMapBar()->setFont( font );
    m_pbvr_gui->fpsLabel()->setFont( font );
    m_pbvr_gui->timeStepLabel()->setFont( font );
}

void Preference::closeEvent( QCloseEvent* event )
{
    loadColorMapBarSettings();
    loadOrientationAxisSettings();
    loadBackGroundColorSettings();
    loadResolutionSettings();
    loadLabelsSettings();
    loadFontSettings();

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
