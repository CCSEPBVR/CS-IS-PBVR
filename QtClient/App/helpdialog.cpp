#include "helpdialog.h"
#include "ui_helpdialog.h"

HelpDialog::HelpDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);
    ui->textBrowser->setAcceptRichText(true);
    ui->textBrowser->setSource(QUrl("qrc:/Resources/gui_manual.html"));
//    ui.textBrowser->setSource(QUrl("manual.html"));

}

HelpDialog::~HelpDialog()
{
    delete ui;
}
