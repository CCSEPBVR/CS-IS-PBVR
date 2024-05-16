#include "ShadingController.h"
#include "ui_ShadingController.h"
#include <QButtonGroup>

#include <kvs/StochasticPolygonRenderer>
#include <kvs/ParticleBasedRenderer>
#include "ExtendedKVS/StochasticTexturedPolygonRenderer.h"
ShadingController::ShadingController(QWidget *parent) :
    QDockWidget(parent),
    m_screen( nullptr ),
    ui(new Ui::ShadingController)
{
    ui->setupUi(this);

    QButtonGroup *radioButtonGroup = new QButtonGroup();
    radioButtonGroup->addButton( ui->noneRBtn );
    radioButtonGroup->addButton( ui->phongRBtn );
    radioButtonGroup->addButton( ui->lambertRBtn );
    radioButtonGroup->addButton( ui->blinnPhongRBtn );

    connect( radioButtonGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( onChangeShader() ) );
    connect( ui->kaDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShader );
    connect( ui->kdDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShader );
    connect( ui->ksDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShader );
    connect( ui->sDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShader );
}

ShadingController::~ShadingController()
{
    delete ui;
}

void ShadingController::onChangeShader()
{
    const int size = m_screen->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );
        if( object->isVisible() && rendererBase )
        {
            if (auto* stochasticRenderer = dynamic_cast<kvs::StochasticRendererBase*>(rendererBase))
            {
                if (auto* stochasticPolygonRenderer = dynamic_cast<kvs::StochasticPolygonRenderer*>(stochasticRenderer))
                {
                    if( ui->noneRBtn->isChecked() )
                    {
                        stochasticPolygonRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
                    }
                    else if( ui->phongRBtn->isChecked() )
                    {
                        stochasticPolygonRenderer->setShader( kvs::Shader::Phong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value() ) );
                    }
                    else if( ui->lambertRBtn->isChecked() )
                    {
                        stochasticPolygonRenderer->setShader( kvs::Shader::Lambert( ui->kaDSBox->value(), ui->kdDSBox->value() ) );
                    }
                    else if( ui->blinnPhongRBtn->isChecked() )
                    {
                        stochasticPolygonRenderer->setShader( kvs::Shader::BlinnPhong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value() ) );
                    }
                }
                else if (auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>(stochasticRenderer))
                {
                    if( ui->noneRBtn->isChecked() )
                    {
                        particleRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
                    }
                    else if( ui->phongRBtn->isChecked() )
                    {
                        particleRenderer->setShader( kvs::Shader::Phong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value() ) );
                    }
                    else if( ui->lambertRBtn->isChecked() )
                    {
                        particleRenderer->setShader( kvs::Shader::Lambert( ui->kaDSBox->value(), ui->kdDSBox->value() ) );
                    }
                    else if( ui->blinnPhongRBtn->isChecked() )
                    {
                        particleRenderer->setShader( kvs::Shader::BlinnPhong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value() ) );
                    }
                }
                else if (auto* stochasticTexturedPolygonRenderer = dynamic_cast<kvs::mod::StochasticTexturedPolygonRenderer*>(stochasticRenderer))
                {
                    if( ui->noneRBtn->isChecked() )
                    {
                        stochasticTexturedPolygonRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
                    }
                    else if( ui->phongRBtn->isChecked() )
                    {
                        stochasticTexturedPolygonRenderer->setShader( kvs::Shader::Phong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value() ) );
                    }
                    else if( ui->lambertRBtn->isChecked() )
                    {
                        stochasticTexturedPolygonRenderer->setShader( kvs::Shader::Lambert( ui->kaDSBox->value(), ui->kdDSBox->value() ) );
                    }
                    else if( ui->blinnPhongRBtn->isChecked() )
                    {
                        stochasticTexturedPolygonRenderer->setShader( kvs::Shader::BlinnPhong( ui->kaDSBox->value(), ui->kdDSBox->value(), ui->ksDSBox->value(), ui->sDSBox->value() ) );
                    }
                }
            }
        }
    }
    m_screen->update();
}
