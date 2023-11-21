#include "Connect.h"
#include "ui_Connect.h"

Connect::Connect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Connect)
{
    ui->setupUi(this);
}

Connect::~Connect()
{
    delete ui;
}
