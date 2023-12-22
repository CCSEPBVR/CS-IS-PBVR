#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

RepetitionLevelControl::RepetitionLevelControl(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RepetitionLevelControl)
{
    ui->setupUi(this);
    connect( ui->applyPBtn, &QPushButton::clicked, this, &RepetitionLevelControl::onApplyButtonClicked );
}

RepetitionLevelControl::~RepetitionLevelControl()
{
    delete ui;
}


void RepetitionLevelControl::onApplyButtonClicked()
{
//    qInfo() << m_compositor->repetitionLevel();
//    m_compositor->setRepetitionLevel( 6 );
}
