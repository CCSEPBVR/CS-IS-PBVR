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

void AnimationControls::InitializeKeyFrame()
{

}

void AnimationControls::addKeyFrameAdd( kvs::Xform xform )
{
    qInfo() << __LINE__;
//    m_xforms.push_back( xform );
}

void AnimationControls::removeLasrKeyFrame()
{
    qInfo() << __LINE__;
//    if( m_xforms.size() > 1 )
//    {
//        m_xforms.pop_back();
//    }
}

void AnimationControls::clearKeyFrame()
{
    qInfo() << __LINE__;
//    m_xforms.clear();
}

void AnimationControls::playKeyFrame()
{
qInfo() << __LINE__;
}

void AnimationControls::loadKeyFrameFile()
{
qInfo() << __LINE__;
}

void AnimationControls::saveKeyFrameFile()
{
qInfo() << __LINE__;
}
