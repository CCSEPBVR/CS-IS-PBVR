#include "AnimationControl.h"
#include "ui_AnimationControl.h"

AnimationControl::AnimationControl( QWidget *parent )
    : QDockWidget(parent)
    , ui(new Ui::AnimationControl)
{
    initialize();
}

AnimationControl::~AnimationControl()
{
    delete ui;
}

void AnimationControl::initialize()
{
    ui->setupUi( this );
}

