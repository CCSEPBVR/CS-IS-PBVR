#include "viewercontrols.h"
#include "ui_viewercontrols.h"
#include "coloreditdialog.h"

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
    return f;
}

void ViewerControls::on_colorButton_clicked()
{
    ColorEditDialog ced(this);
    ced.setColorSelection(selected_color);
    selected_color= ced.getColorSelection();
}



