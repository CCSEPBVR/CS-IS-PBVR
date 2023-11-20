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
    ui->orientationTypeCB->addItem( "Horizontal", Horizontal );
    ui->orientationTypeCB->addItem( "Vertical", Vertical );

    ui->axisTypeCB->addItem( "CorneredAxis", CorneredAxis );
    ui->axisTypeCB->addItem( "CenteredAxis", CenteredAxis );
    ui->axisTypeCB->addItem( "NoneAxis", NoneAxis );

    ui->boxTypeCB->addItem( "WiredBox", WiredBox );
    ui->boxTypeCB->addItem( "SolidBox", SolidBox );
    ui->boxTypeCB->addItem( "NoneBox", NoneBox );

    connect( ui->selectedColorLbl, &ClickableLabel::doubleClicked, this, &Preference::onSelectedColorDoubleClicked );
    connect( ui->applyBtn,  &QPushButton::clicked, this, &Preference::onApplyButtonClicked  );
    connect( ui->cancelBtn, &QPushButton::clicked, this, &Preference::onCancelButtonClicked );
    connect( ui->okBtn,     &QPushButton::clicked, this, &Preference::onOKButtonClicked     );
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

    event->accept();
}

void Preference::initialize()
{
    if ( this->checkConfigFileExists() )
    {
        loadScreenSettings();
        loadColorMapBarSettings();
        loadOrientationAxisSettings();
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
        ui->colorMapBarGB->setChecked( true );
        if(m_settings.value("Orientation").toString() == "Horizontal")
        {
            ui->orientationTypeCB->setCurrentIndex( Horizontal );
        }
        else if(m_settings.value("Orientation").toString() == "Vertical")
        {
            ui->orientationTypeCB->setCurrentIndex( Vertical );
        }
    }
    else
    {
        ui->colorMapBarGB->setChecked( false );
    }
    m_settings.endGroup();
}

void Preference::loadOrientationAxisSettings()
{
    m_settings.beginGroup("OrientationAxis");
    bool isShowing = m_settings.value("isShowing").toBool();
    if( isShowing )
    {
        ui->orientationGB->setChecked( true );
        if(m_settings.value("AxisType").toString() == "CorneredAxis")
        {
            ui->axisTypeCB->setCurrentIndex( CorneredAxis );
        }
        else if(m_settings.value("AxisType").toString() == "CenteredAxis")
        {
            ui->axisTypeCB->setCurrentIndex( CenteredAxis );
        }
        else if(m_settings.value("AxisType").toString() == "NoneAxis")
        {
            ui->axisTypeCB->setCurrentIndex( NoneAxis );
        }

        if(m_settings.value("BoxType").toString() == "WiredBox")
        {
            ui->boxTypeCB->setCurrentIndex( WiredBox );
        }
        else if(m_settings.value("BoxType").toString() == "SolidBox")
        {
            ui->boxTypeCB->setCurrentIndex( SolidBox );
        }
        else if(m_settings.value("BoxType").toString() == "NoneBox")
        {
            ui->boxTypeCB->setCurrentIndex( NoneAxis );
        }
    }
    else
    {
        ui->orientationGB->setChecked( false );
    }
    m_settings.endGroup();
}

void Preference::setDefaultSettings()
{
    QColor color( 82, 87, 110 );
    setSelectedColor( color );
    ui->orientationTypeCB->setCurrentIndex( Horizontal );
    ui->axisTypeCB->setCurrentIndex( CorneredAxis );
    ui->boxTypeCB->setCurrentIndex( SolidBox );
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
                                      ui->selectedColorLbl->palette().color(QPalette::Window).red(),
                                      ui->selectedColorLbl->palette().color(QPalette::Window).green(),
                                      ui->selectedColorLbl->palette().color(QPalette::Window).blue())
                                  );
}

void Preference::applyColorMapBarSettings()
{
    if(ui->colorMapBarGB->isChecked())
    {
        OrientationType selectedOrientationType = static_cast<OrientationType>( ui->orientationTypeCB->currentIndex());
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
    if(ui->orientationGB->isChecked())
    {        
        AxisType selectedAxisType = static_cast<AxisType>( ui->axisTypeCB->currentIndex());;
        BoxType selectedBoxType   = static_cast<BoxType>( ui->boxTypeCB->currentIndex());
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
        std::cout << "TEST" << std::endl;
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
        QPalette palette = ui->selectedColorLbl->palette();
        palette.setColor(QPalette::Window, color);
        ui->selectedColorLbl->setAutoFillBackground(true);
        ui->selectedColorLbl->setPalette(palette);
        ui->selectedColorLbl->update();
    }
}

void Preference::saveSettings()
{
        m_settings.beginGroup( "Screen" );
        m_settings.setValue( "BackGroundColor_R", ui->selectedColorLbl->palette().color(QPalette::Window).red());
        m_settings.setValue( "BackGroundColor_G", ui->selectedColorLbl->palette().color(QPalette::Window).green());
        m_settings.setValue( "BackGroundColor_B", ui->selectedColorLbl->palette().color(QPalette::Window).blue());
        m_settings.endGroup();

        m_settings.beginGroup( "ColorMapBar" );
        m_settings.setValue( "isShowing", ui->colorMapBarGB->isChecked() );
        m_settings.setValue( "Orientation", ui->orientationTypeCB->currentText() );
        m_settings.endGroup();

        m_settings.beginGroup( "OrientationAxis" );
        m_settings.setValue( "isShowing", ui->orientationGB->isChecked() );
        m_settings.setValue( "AxisType", ui->axisTypeCB->currentText() );
        m_settings.setValue( "BoxType", ui->boxTypeCB->currentText() );
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
