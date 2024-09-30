#include "ShadingController.h"
#include "ui_ShadingController.h"
#include "App/pbvrgui.h"
#include <QButtonGroup>

#include <kvs/StochasticPolygonRenderer>
#include <kvs/ParticleBasedRenderer>
#include <kvs/StochasticTexturedPolygonRenderer>
#include <kvs/IDManager>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
ShadingController::ShadingController(QWidget *parent, PBVRGUI *pbvr_gui) :
    QDockWidget(parent),    
    ui(new Ui::ShadingController),
    m_pbvr_gui( pbvr_gui )
{
    ui->setupUi(this);

    QButtonGroup *radioButtonGroup = new QButtonGroup();
    radioButtonGroup->addButton( ui->noneRBtn );
    radioButtonGroup->addButton( ui->phongRBtn );
    radioButtonGroup->addButton( ui->lambertRBtn );
    radioButtonGroup->addButton( ui->blinnPhongRBtn );

    connect( radioButtonGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( onChangeShaderParameter() ) );
    connect( ui->kaDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShaderParameter );
    connect( ui->kdDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShaderParameter );
    connect( ui->ksDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShaderParameter );
    connect( ui->sDSBox, &QDoubleSpinBox::valueChanged, this, &ShadingController::onChangeShaderParameter );
}

ShadingController::~ShadingController()
{
    delete ui;
}

void ShadingController::applyShading(kvs::RendererBase*& rendererBase)
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
            if( !particleRenderer->isShuffleEnabled() )
            {
                particleRenderer->enableShuffle();
            }

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
        else if (auto* stochasticTexturedPolygonRenderer = dynamic_cast<kvs::StochasticTexturedPolygonRenderer*>(stochasticRenderer))
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

void ShadingController::onChangeShaderParameter()
{
    const int size = m_pbvr_gui->screen()->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto id = m_pbvr_gui->screen()->scene()->IDManager()->id( index );
        auto* object = m_pbvr_gui->screen()->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_pbvr_gui->screen()->scene()->rendererManager()->renderer( id.second );
//        if( object->isVisible() && rendererBase )
        {
            applyShading( rendererBase );
        }
    }
    m_pbvr_gui->screen()->update();
}
