#include "Preference.h"
#include "ui_Preference.h"

#include <QColorDialog>
#include <QMessageBox>
#include <QPalette>
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

    connect( ui->selectedColorCLbl, &ClickableLabel::doubleClicked, this, &Preference::onSelectedColorDoubleClicked );
    connect( ui->applyPBtn,  &QPushButton::clicked, this, &Preference::onApplyButtonClicked  );
    connect( ui->cancelPBtn, &QPushButton::clicked, this, &Preference::onCancelButtonClicked );
    connect( ui->okPBtn,     &QPushButton::clicked, this, &Preference::onOKButtonClicked     );
}

Preference::~Preference()
{
    delete ui;
}

void Preference::closeEvent(QCloseEvent *event)
{
    loadScreenSettings();
    loadColorMapBarSettings();
    loadOrientationAxisSettings();
    loadResolutionSettings();

    event->accept();
}

void Preference::initialize()
{
    if ( this->checkConfigFileExists() )
    {
        loadScreenSettings();
        loadColorMapBarSettings();
        loadOrientationAxisSettings();
        loadResolutionSettings();
    }
    else
    {
        setDefaultSettings();
        saveSettings();
    }

    m_color_map_bar->anchorToTopLeft();
    m_orientation_axis->anchorToBottomRight();
    this->applySettings( true );
}

void Preference::loadScreenSettings()
{
    m_settings.beginGroup("Screen");
    QColor color(
                m_settings.value("BackGroundColor_R").toInt(),
                m_settings.value("BackGroundColor_G").toInt(),
                m_settings.value("BackGroundColor_B").toInt()
                );
    setSelectedColor(color);
    m_settings.endGroup();
}

void Preference::loadColorMapBarSettings()
{
    m_settings.beginGroup("ColorMapBar");
    bool isShowing = m_settings.value("isShowing").toBool();
    if( isShowing )
    {
        ui->colorMapBarGBox->setChecked( true );
        if(m_settings.value("Orientation").toString() == "Horizontal")
        {
            ui->orientationTypeCBox->setCurrentIndex( Horizontal );
        }
        else if(m_settings.value("Orientation").toString() == "Vertical")
        {
            ui->orientationTypeCBox->setCurrentIndex( Vertical );
        }
    }
    else
    {
        ui->colorMapBarGBox->setChecked( false );
    }
    m_settings.endGroup();
}

void Preference::loadOrientationAxisSettings()
{
    m_settings.beginGroup("OrientationAxis");
    bool isShowing = m_settings.value("isShowing").toBool();
    if( isShowing )
    {
        ui->orientationGBox->setChecked( true );
        if(m_settings.value("AxisType").toString() == "CorneredAxis")
        {
            ui->axisTypeCBox->setCurrentIndex( CorneredAxis );
        }
        else if(m_settings.value("AxisType").toString() == "CenteredAxis")
        {
            ui->axisTypeCBox->setCurrentIndex( CenteredAxis );
        }
        else if(m_settings.value("AxisType").toString() == "NoneAxis")
        {
            ui->axisTypeCBox->setCurrentIndex( NoneAxis );
        }

        if(m_settings.value("BoxType").toString() == "WiredBox")
        {
            ui->boxTypeCBox->setCurrentIndex( WiredBox );
        }
        else if(m_settings.value("BoxType").toString() == "SolidBox")
        {
            ui->boxTypeCBox->setCurrentIndex( SolidBox );
        }
        else if(m_settings.value("BoxType").toString() == "NoneBox")
        {
            ui->boxTypeCBox->setCurrentIndex( NoneAxis );
        }
    }
    else
    {
        ui->orientationGBox->setChecked( false );
    }
    m_settings.endGroup();
}

void Preference::loadResolutionSettings()
{
    m_settings.beginGroup( "Resolution" );
    ui->heightSBox->setValue(m_settings.value("height").toInt());
    ui->widthSBox->setValue(m_settings.value("width").toInt());
    m_settings.endGroup();
}

void Preference::setDefaultSettings()
{
    QColor color( 82, 87, 110 );
    setSelectedColor( color );
    ui->orientationTypeCBox->setCurrentIndex( Horizontal );
    ui->axisTypeCBox->setCurrentIndex( CorneredAxis );
    ui->boxTypeCBox->setCurrentIndex( SolidBox );
    ui->widthSBox->setValue( 620 );
    ui->heightSBox->setValue( 620 );
}

void Preference::applySettings( bool isInit )
{
    applyScreenSettings();
    applyColorMapBarSettings();
    applyOrientationAxisSettings();
    if( !isInit )
    {
        saveSettings();
    }
    m_screen->update();
}

void Preference::applyScreenSettings()
{
    m_screen->setBackgroundColor( kvs::RGBColor(
        ui->selectedColorCLbl->palette().color(QPalette::Window).red(),
        ui->selectedColorCLbl->palette().color(QPalette::Window).green(),
        ui->selectedColorCLbl->palette().color(QPalette::Window).blue())
                                  );


        m_screen->setFixedSize( ui->widthSBox->value(), ui->heightSBox->value() );
        m_screen->setGeometry( 0, 0, ui->widthSBox->value(), ui->heightSBox->value() );
}

void Preference::applyColorMapBarSettings()
{
    if(ui->colorMapBarGBox->isChecked())
    {
        OrientationType selectedOrientationType = static_cast<OrientationType>( ui->orientationTypeCBox->currentIndex());
        switch ( selectedOrientationType )
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
    if(ui->orientationGBox->isChecked())
    {
        AxisType selectedAxisType = static_cast<AxisType>( ui->axisTypeCBox->currentIndex());;
        BoxType selectedBoxType   = static_cast<BoxType>( ui->boxTypeCBox->currentIndex());
        switch ( selectedAxisType )
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

        switch ( selectedBoxType ) {
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

void Preference::setSelectedColor(const QColor& color)
{
    if( color.isValid() )
    {
        QPalette palette = ui->selectedColorCLbl->palette();
        palette.setColor(QPalette::Window, color);
        ui->selectedColorCLbl->setAutoFillBackground(true);
        ui->selectedColorCLbl->setPalette(palette);
        ui->selectedColorCLbl->update();
    }
}

void Preference::saveSettings()
{
    m_settings.beginGroup( "Screen" );
    m_settings.setValue( "BackGroundColor_R", ui->selectedColorCLbl->palette().color(QPalette::Window).red());
    m_settings.setValue( "BackGroundColor_G", ui->selectedColorCLbl->palette().color(QPalette::Window).green());
    m_settings.setValue( "BackGroundColor_B", ui->selectedColorCLbl->palette().color(QPalette::Window).blue());
    m_settings.endGroup();

    m_settings.beginGroup( "ColorMapBar" );
    m_settings.setValue( "isShowing", ui->colorMapBarGBox->isChecked() );
    m_settings.setValue( "Orientation", ui->orientationTypeCBox->currentText() );
    m_settings.endGroup();

    m_settings.beginGroup( "OrientationAxis" );
    m_settings.setValue( "isShowing", ui->orientationGBox->isChecked() );
    m_settings.setValue( "AxisType", ui->axisTypeCBox->currentText() );
    m_settings.setValue( "BoxType", ui->boxTypeCBox->currentText() );
    m_settings.endGroup();

    m_settings.beginGroup( "Resolution" );
    m_settings.setValue( "width", ui->widthSBox->value() );
    m_settings.setValue( "height", ui->heightSBox->value() );
    m_settings.endGroup();

    m_settings.sync();
}

void Preference::onSelectedColorDoubleClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"));
    setSelectedColor( color );
}

void Preference::onApplyButtonClicked()
{
    this->applySettings( false );
}

void Preference::onCancelButtonClicked()
{
    this->close();
}

void Preference::onOKButtonClicked()
{
    applySettings( false );
    saveSettings();
    close();
}
