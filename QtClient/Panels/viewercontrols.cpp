#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif // KVS_ENABLE_GLEW

#include "viewercontrols.h"
#include "ui_viewercontrols.h"
#include "coloreditdialog.h"

#include "Client/LogManager.h"
#include <kvs/Background>

ViewerControls::ViewerControls(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ViewerControls)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->colorButton,&QPushButton::clicked,
            this, &ViewerControls::on_colorButton_clicked);

}

ViewerControls::~ViewerControls()
{
    delete ui;

}

void ViewerControls::setCurrentFont(const QFont &f)
{
    ui->fontSelection->setCurrentFont(f);
}

const QFont ViewerControls::getFontSelection(){
    QFont f=ui->fontSelection->currentFont();
    //f.setPointSize(12);
    f.setKerning(true);
    f.setStyleStrategy(QFont::PreferAntialias);
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("viewercontrols.cpp",__func__,"QFont", f.toString().toStdString());
    }
    return f;
}

void ViewerControls::on_colorButton_clicked()
{
    ColorEditDialog ced(this);
    ced.setColorSelection(selected_color);
    selected_color= ced.getColorSelection();
    if( m_isRec )
    {
        LogManager lg;
        lg.Write("viewercontrols.cpp",__func__,"kvs RGB Color",std::to_string(selected_color.r()) + "," + std::to_string(selected_color.g()) + "," + std::to_string(selected_color.b()));
    }
}

void ViewerControls::logPlay(std::string funcName, std::string str[])
{
    if(funcName.compare("getFontSelection") == 0)
    {
       QFont f;
       f.setFamily(                 QString::fromStdString(str[0]));
       f.setPointSizeF(                               stof(str[1]));
       f.setPixelSize(                           std::stoi(str[2]));
       f.setStyleHint(          QFont::StyleHint(std::stoi(str[3])));
       f.setWeight(                QFont::Weight(std::stoi(str[4])));
       f.setStyle(                  QFont::Style(std::stoi(str[5])));
       f.setUnderline(                           std::stoi(str[6]));
       f.setStrikeOut(                           std::stoi(str[7]));
       f.setFixedPitch(                          std::stoi(str[8]));
       f.setCapitalization(QFont::Capitalization(std::stoi(str[9])));
       f.setLetterSpacing(   QFont::SpacingType(std::stoi(str[10])),std::stoi(str[11]));
       f.setWordSpacing(                        std::stoi(str[12]));
       f.setStretch(                            std::stoi(str[13]));
       f.setStyleStrategy( QFont::StyleStrategy(std::stoi(str[14])));
       f.setStyleName(             QString::fromStdString(str[15]));
       extCommand->m_screen->setLabelFont(f);
    }

    if(funcName.compare("on_colorButton_clicked") == 0)
    {
        extCommand->m_screen->scene()->background()->setColor(kvs::RGBColor(std::stoi(str[0]),std::stoi(str[1]),std::stoi(str[2])));
    }

    extCommand->m_screen->update();
}

