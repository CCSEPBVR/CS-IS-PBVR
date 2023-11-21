#include "AnimationControls.h"
#include "ui_AnimationControls.h"

AnimationControls::AnimationControls(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::AnimationControls)
{
    ui->setupUi(this);
}

AnimationControls::~AnimationControls()
{
    delete ui;
}
