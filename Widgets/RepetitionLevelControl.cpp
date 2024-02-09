#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

#include <kvs/ParticleBasedRenderer>
RepetitionLevelControl::RepetitionLevelControl(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RepetitionLevelControl),
    m_screen( nullptr ),
    m_compositor( nullptr )
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


    for(int i = 1; i < m_screen->scene()->numberOfObjects(); i++ )
    {
        if( m_screen->scene()->hasObject(i) )
        {
            if( strcmp( "kvs::glsl::ParticleBasedRenderer", m_screen->scene()->renderer(i)->moduleName()) == 0 )
            {
                kvs::glsl::ParticleBasedRenderer* particleBasedRenderer = new  kvs::glsl::ParticleBasedRenderer();
                particleBasedRenderer->enableShuffle();
                particleBasedRenderer->setRepetitionLevel( ui->nextRepetitionLevelSBox->value() );
                m_screen->scene()->replaceRenderer( i, particleBasedRenderer );
            }
        }
    }
        m_compositor->setRepetitionLevel( ui->nextRepetitionLevelSBox->value() );
        m_compositor->screen()->redraw();
}
