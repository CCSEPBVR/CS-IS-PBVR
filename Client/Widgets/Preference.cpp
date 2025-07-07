#include "Preference.h"
#include "ui_Preference.h"

Preference::Preference( kvs::qt::jaea::Screen* screen,
                        kvs::StochasticRenderingCompositor* compositor,
                        kvs::ColorMapBar* colorMapBar,
                        kvs::OrientationAxis* orientationAxis,
                        kvs::Label* fpsLabel,
                        kvs::Label* timeStepLabel,
                        QWidget *parent )
    : QDialog(parent)
    , ui( new Ui::Preference )
    , m_screen( screen )
    , m_compositor( compositor )
    , m_color_map_bar( colorMapBar )
    , m_orientation_axis( orientationAxis )
    , m_fps_label( fpsLabel )
    , m_time_step_label( timeStepLabel )
    , m_settings( "config.ini", QSettings::IniFormat )
    , m_current_time_step( 0 )
{
    ui->setupUi(this);
    initialize();
}

Preference::~Preference()
{
    delete ui;
}

void Preference::closeEvent( QCloseEvent* event )
{
    loadSettings();
    event->accept();
}

void Preference::initialize()
{
    this->setFixedSize( this->sizeHint() );

    m_color_map_bar->anchorToTopLeft();
    m_orientation_axis->anchorToBottomRight();

    ui->orientationTypeComboBox->addItem( "Horizontal", Horizontal );
    ui->orientationTypeComboBox->addItem( "Vertical", Vertical );

    ui->axisTypeComboBox->addItem( "CorneredAxis", CorneredAxis );
    ui->axisTypeComboBox->addItem( "CenteredAxis", CenteredAxis );
    ui->axisTypeComboBox->addItem( "NoneAxis", NoneAxis );

    ui->boxTypeComboBox->addItem( "WiredBox", WiredBox );
    ui->boxTypeComboBox->addItem( "SolidBox", SolidBox );
    // ui->boxTypeComboBox->addItem( "NoneBox", NoneBox );

    ui->showFPSComboBox->addItem( "Show", QVariant( true ) );
    ui->showFPSComboBox->addItem( "Hide", QVariant( false ) );

    ui->showTimeStepComboBox->addItem( "Show", QVariant( true ) );
    ui->showTimeStepComboBox->addItem( "Hide", QVariant( false ) );

    QPalette backGroundColor = ui->backGroundColorClickableLabel->palette();
    backGroundColor.setColor( QPalette::Window, QColor( 128, 128, 128 ) );
    ui->backGroundColorClickableLabel->setAutoFillBackground( true );
    ui->backGroundColorClickableLabel->setPalette( backGroundColor );

    QPalette fontColor = ui->fontColorClickableLabel->palette();
    fontColor.setColor( QPalette::Window, QColor( 128, 128, 128 ) );
    ui->fontColorClickableLabel->setAutoFillBackground( true );
    ui->fontColorClickableLabel->setPalette( fontColor );

    // config.iniが存在する場合は設定の読み込みを行う。
    if( checkConfigFileExists() )
    {
        loadSettings();
    }
    else
    {
        defaultSettings();        
    }

    connect( ui->backGroundColorClickableLabel, &ClickableLabel::doubleClicked, this, &Preference::onBackGroundColorDoubleClicked );
    connect( ui->fontColorClickableLabel, &ClickableLabel::doubleClicked, this, &Preference::onLabelsColorDoubleClicked );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &Preference::onApply );
    connect( ui->defaultPushButton, &QPushButton::clicked, this, &Preference::onDefault );
    connect( ui->cancelPushButton, &QPushButton::clicked, this, &Preference::onCancel );
    connect( ui->okPushButton, &QPushButton::clicked, this, &Preference::onOK );
}

void Preference::defaultSettings()
{
    //ColorMapBar
    ui->orientationTypeComboBox->setCurrentIndex( Horizontal );
    //OrientationAxis
    ui->axisTypeComboBox->setCurrentIndex( CorneredAxis );
    ui->boxTypeComboBox->setCurrentIndex( SolidBox );
    //BackGroundColor
    setColor( *ui->backGroundColorClickableLabel, QColor( 82, 87, 110 ) );
    //Resolution
    ui->widthSpinBox->setValue( 620 );
    ui->heightSpinBox->setValue( 620 );
    //Labels
    ui->showFPSComboBox->setCurrentText( "Hide" );
    ui->showTimeStepComboBox->setCurrentText( "Hide" );
    //Font
    setColor( *ui->fontColorClickableLabel, QColor( 0, 0, 0 ) );
}

void Preference::loadSettings()
{
    loadColorMapBarSetting();
    loadOrientationAxisSetting();
    loadBackgroundColorSetting();
    loadResolutionSetting();
    loadLabelSetting();
    loadFontColorSetting();
}

void Preference::loadColorMapBarSetting()
{
    m_settings.beginGroup( "ColorMapBar" );
    const bool isShowing = m_settings.value( "isShowing" ).toBool();
    const QString orientationType = m_settings.value( "Orientation" ).toString();

    if( isShowing )
    {
        ui->colorMapBarGroupBox->setChecked( true );
    }
    else
    {
        ui->colorMapBarGroupBox->setChecked( false );
    }

    if( orientationType == "Horizontal" )
    {
        ui->orientationTypeComboBox->setCurrentIndex( Horizontal );
    }
    else if( orientationType == "Vertical" )
    {
        ui->orientationTypeComboBox->setCurrentIndex( Vertical );
    }

    m_settings.endGroup();
}

void Preference::loadOrientationAxisSetting()
{
    m_settings.beginGroup( "OrientationAxis" );
    const bool isShowing = m_settings.value( "isShowing" ).toBool();
    const QString axisType = m_settings.value( "AxisType" ).toString();
    const QString boxType = m_settings.value( "BoxType" ).toString();

    if( isShowing )
    {
        ui->orientationAxisGroupBox->setChecked( true );
    }
    else
    {
        ui->orientationAxisGroupBox->setChecked( false );
    }

    if( axisType == "CorneredAxis" )
    {
        ui->axisTypeComboBox->setCurrentIndex( CorneredAxis );
    }
    else if( axisType == "CenteredAxis" )
    {
        ui->axisTypeComboBox->setCurrentIndex( CenteredAxis );
    }
    else if( axisType == "NoneAxis" )
    {
        ui->axisTypeComboBox->setCurrentIndex( NoneAxis );
    }

    if( boxType == "WiredBox" )
    {
        ui->boxTypeComboBox->setCurrentIndex( WiredBox );
    }
    else if( boxType == "SolidBox" )
    {
        ui->boxTypeComboBox->setCurrentIndex( SolidBox );
    }
    // else if( boxType == "NoneBox" )
    // {
    //     ui->boxTypeComboBox->setCurrentIndex( NoneAxis );
    // }

    m_settings.endGroup();
}

void Preference::loadBackgroundColorSetting()
{
    m_settings.beginGroup( "BackGroundColor" );
    QColor color( m_settings.value( "R" ).toInt(),m_settings.value( "G" ).toInt(),m_settings.value( "B" ).toInt() );

    setColor( *ui->backGroundColorClickableLabel, color );

    m_settings.endGroup();
}

void Preference::loadResolutionSetting()
{
    m_settings.beginGroup( "Resolution" );
    const int width = m_settings.value( "width" ).toInt();
    const int height = m_settings.value( "height" ).toInt();

    ui->widthSpinBox->setValue( width );
    ui->heightSpinBox->setValue( height );

    m_settings.endGroup();
}

void Preference::loadLabelSetting()
{
    m_settings.beginGroup( "Labels" );
    const bool fpsIsShowing = m_settings.value( "fpsIsShowing" ).toBool();
    const bool timeStepIsShowing = m_settings.value( "timeStepIsShowing" ).toBool();

    if( fpsIsShowing )
    {
        ui->showFPSComboBox->setCurrentText( "Show" );
    }
    else
    {
        ui->showFPSComboBox->setCurrentText( "Hide" );
    }

    if( timeStepIsShowing )
    {
        ui->showTimeStepComboBox->setCurrentText( "Show" );
    }
    else
    {
        ui->showTimeStepComboBox->setCurrentText( "Hide" );
    }

    m_settings.endGroup();
}

void Preference::loadFontColorSetting()
{
    m_settings.beginGroup( "Font" );
    QColor color( m_settings.value( "R" ).toInt(),m_settings.value( "G" ).toInt(),m_settings.value( "B" ).toInt() );

    setColor( *ui->fontColorClickableLabel, color );

    m_settings.endGroup();
}

void Preference::saveSettings()
{
    saveColorMapBarSetting();
    saveOrientationAxisSetting();
    saveBackgroundColorSetting();
    saveResolutionSetting();
    saveLabelSetting();
    saveFontColorSetting();

    m_settings.sync();
}

void Preference::saveColorMapBarSetting()
{
    m_settings.beginGroup( "ColorMapBar" );
    m_settings.setValue( "isShowing", ui->colorMapBarGroupBox->isChecked() );
    m_settings.setValue( "Orientation", ui->orientationTypeComboBox->currentText() );
    m_settings.endGroup();
}

void Preference::saveOrientationAxisSetting()
{
    m_settings.beginGroup( "OrientationAxis" );
    m_settings.setValue( "isShowing", ui->orientationAxisGroupBox->isChecked() );
    m_settings.setValue( "AxisType", ui->axisTypeComboBox->currentText() );
    m_settings.setValue( "BoxType", ui->boxTypeComboBox->currentText() );
    m_settings.endGroup();

}

void Preference::saveBackgroundColorSetting()
{
    m_settings.beginGroup( "BackGroundColor" );
    m_settings.setValue( "R", ui->backGroundColorClickableLabel->palette().color(QPalette::Window).red());
    m_settings.setValue( "G", ui->backGroundColorClickableLabel->palette().color(QPalette::Window).green());
    m_settings.setValue( "B", ui->backGroundColorClickableLabel->palette().color(QPalette::Window).blue());
    m_settings.endGroup();
}

void Preference::saveResolutionSetting()
{
    m_settings.beginGroup( "Resolution" );
    m_settings.setValue( "width", ui->widthSpinBox->value() );
    m_settings.setValue( "height", ui->heightSpinBox->value() );
    m_settings.endGroup();
}

void Preference::saveLabelSetting()
{
    m_settings.beginGroup( "Labels" );
    m_settings.setValue( "fpsIsShowing", ui->showFPSComboBox->currentData().toBool() );
    m_settings.setValue( "timeStepIsShowing", ui->showTimeStepComboBox->currentData().toBool() );
    m_settings.endGroup();
}

void Preference::saveFontColorSetting()
{
    m_settings.beginGroup( "Font" );
    m_settings.setValue( "R", ui->fontColorClickableLabel->palette().color(QPalette::Window).red() );
    m_settings.setValue( "G", ui->fontColorClickableLabel->palette().color(QPalette::Window).green() );
    m_settings.setValue( "B", ui->fontColorClickableLabel->palette().color(QPalette::Window).blue() );
    m_settings.endGroup();
}

void Preference::applySettings()
{
    applyColorMapBarSetting();
    applyOrientationAxisSetting();
    applyBackgroundColorSetting();
    applyResolutionSetting();
    applyLabelSetting();
    applyFontColorSetting();

    saveSettings();

    m_screen->update();
}

void Preference::applyColorMapBarSetting()
{
    const bool isShowing = ui->colorMapBarGroupBox->isChecked();
    const OrientationType orientationType = static_cast<OrientationType>( ui->orientationTypeComboBox->currentIndex() );

#ifdef Q_OS_WIN
    m_color_map_bar->setCaption( ui->captionLineEdit->text().toLocal8Bit().constData() );
#else
    m_color_map_bar->setCaption( ui->captionLineEdit->text().toStdString() );
#endif

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

void Preference::applyOrientationAxisSetting()
{
    const bool isShowing = ui->orientationAxisGroupBox->isChecked();
    const AxisType axisType = static_cast<AxisType>( ui->axisTypeComboBox->currentIndex() );
    const BoxType boxType = static_cast<BoxType>( ui->boxTypeComboBox->currentIndex() );

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
        // case NoneBox:
        //     m_orientation_axis->setBoxType( kvs::OrientationAxis::NoneBox );
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

void Preference::applyBackgroundColorSetting()
{
    const kvs::RGBColor backGroundColor(
        ui->backGroundColorClickableLabel->palette().color(QPalette::Window).red(),
        ui->backGroundColorClickableLabel->palette().color(QPalette::Window).green(),
        ui->backGroundColorClickableLabel->palette().color(QPalette::Window).blue() );

    m_screen->setBackgroundColor( kvs::RGBColor( backGroundColor ) );
}

void Preference::applyResolutionSetting()
{
    const int width = ui->widthSpinBox->value();
    const int height = ui->heightSpinBox->value();
    m_screen->setSize( width, height );
    m_screen->setFixedSize( width, height );
}

void Preference::applyLabelSetting()
{
    const bool fpsIsShowing = ui->showFPSComboBox->currentData().toBool();
    const bool timeStepIsShowing = ui->showTimeStepComboBox->currentData().toBool();

    if( fpsIsShowing )
    {
        m_fps_label->setPosition( 20, m_screen->height() - 40 );
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
        m_time_step_label->setPosition( 100, m_screen->height() - 40 );
        m_time_step_label->setText( "Time step: " + std::to_string( m_current_time_step ) );
        m_time_step_label->show();
    }
    else
    {
        m_time_step_label->hide();
    }
}

void Preference::applyFontColorSetting()
{
    const kvs::RGBColor labelsColor(
        ui->fontColorClickableLabel->palette().color(QPalette::Window).red(),
        ui->fontColorClickableLabel->palette().color(QPalette::Window).green(),
        ui->fontColorClickableLabel->palette().color(QPalette::Window).blue() );

    kvs::Font font;
    font.setColor( labelsColor );
    m_color_map_bar->setFont( font );
    m_fps_label->setFont( font );
    m_time_step_label->setFont( font );
}

void Preference::setColor( ClickableLabel& clickableLabel, const QColor& color )
{
    QPalette palette = clickableLabel.palette();
    palette.setColor( QPalette::Window, color );
    clickableLabel.setAutoFillBackground( true );
    clickableLabel.setPalette( palette );
    clickableLabel.update();
}

void Preference::mergingFinish( int requestTimeStep )
{
    m_current_time_step = requestTimeStep;
    m_time_step_label->setText( "Time step: " + std::to_string( m_current_time_step ) );
}

void Preference::onBackGroundColorDoubleClicked()
{
    QColorDialog colorDialog;
    colorDialog.adjustSize();

    if( colorDialog.exec() == QDialog::Accepted )
    {
        QColor color = colorDialog.selectedColor();
        setColor( *ui->backGroundColorClickableLabel, color );
    }
    raise();
}

void Preference::onLabelsColorDoubleClicked()
{
    QColorDialog colorDialog;
    colorDialog.adjustSize();

    if( colorDialog.exec() == QDialog::Accepted )
    {
        QColor color = colorDialog.selectedColor();
        setColor( *ui->fontColorClickableLabel, color );
    }
    raise();
}

void Preference::onApply()
{
    applySettings();
}

void Preference::onDefault()
{
    defaultSettings();
}

void Preference::onCancel()
{
    close();
}

void Preference::onOK()
{
    applySettings();
    close();
}
