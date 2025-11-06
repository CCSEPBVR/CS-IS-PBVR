#include "ShadingControl.h"
#include "ui_ShadingControl.h"

ShadingControl::ShadingControl( kvs::qt::jaea::Screen* screen,
                                QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::ShadingControl )
    , m_screen( screen )
{
    initialize();
}

ShadingControl::~ShadingControl()
{
    delete ui;
}

void ShadingControl::shading( kvs::RendererBase* rendererBase )
{
    if( auto* stochasticRenderer = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase ) )
    {
        if( auto* stochasticPolygonRenderer = dynamic_cast<kvs::StochasticPolygonRenderer*>( stochasticRenderer ) )
        {
            if( ui->noneRadioButton->isChecked() )
            {
                stochasticPolygonRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
            }
            else if( ui->phongRadioButton->isChecked() )
            {
                stochasticPolygonRenderer->setShader( kvs::Shader::Phong( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value(), ui->SpecularDoubleSpinBox->value(), ui->ShininessDoubleSpinBox->value() ) );
            }
            else if( ui->lambertRadioButton->isChecked() )
            {
                stochasticPolygonRenderer->setShader( kvs::Shader::Lambert( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value() ) );
            }
            else if( ui->blinnPhongRadioButton->isChecked() )
            {
                stochasticPolygonRenderer->setShader( kvs::Shader::BlinnPhong( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value(), ui->SpecularDoubleSpinBox->value(), ui->ShininessDoubleSpinBox->value() ) );
            }
        }
        else if( auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochasticRenderer ) )
        {
            if( !particleRenderer->isShuffleEnabled() )
            {
                particleRenderer->enableShuffle();
            }

            if( ui->noneRadioButton->isChecked() )
            {
                particleRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
            }
            else if( ui->phongRadioButton->isChecked() )
            {
                particleRenderer->setShader( kvs::Shader::Phong( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value(), ui->SpecularDoubleSpinBox->value(), ui->ShininessDoubleSpinBox->value() ) );
            }
            else if( ui->lambertRadioButton->isChecked() )
            {
                particleRenderer->setShader( kvs::Shader::Lambert( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value() ) );
            }
            else if( ui->blinnPhongRadioButton->isChecked() )
            {
                particleRenderer->setShader( kvs::Shader::BlinnPhong( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value(), ui->SpecularDoubleSpinBox->value(), ui->ShininessDoubleSpinBox->value() ) );
            }
        }
        else if( auto* stochasticTexturedPolygonRenderer = dynamic_cast<kvs::StochasticTexturedPolygonRenderer*>( stochasticRenderer ) )
        {
            if( ui->noneRadioButton->isChecked() )
            {
                stochasticTexturedPolygonRenderer->setShader( kvs::Shader::Lambert( 1, 0 ) );
            }
            else if( ui->phongRadioButton->isChecked() )
            {
                stochasticTexturedPolygonRenderer->setShader( kvs::Shader::Phong( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value(), ui->SpecularDoubleSpinBox->value(), ui->ShininessDoubleSpinBox->value() ) );
            }
            else if( ui->lambertRadioButton->isChecked() )
            {
                stochasticTexturedPolygonRenderer->setShader( kvs::Shader::Lambert( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value() ) );
            }
            else if( ui->blinnPhongRadioButton->isChecked() )
            {
                stochasticTexturedPolygonRenderer->setShader( kvs::Shader::BlinnPhong( ui->AmbientDoubleSpinBox->value(), ui->DiffuseDoubleSpinBox->value(), ui->SpecularDoubleSpinBox->value(), ui->ShininessDoubleSpinBox->value() ) );
            }
        }
    }
}

void ShadingControl::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ShadingControl::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ShadingControl::initialize()
{
    ui->setupUi( this );
    QButtonGroup *radioButtonGroup = new QButtonGroup();
    radioButtonGroup->addButton( ui->noneRadioButton );
    radioButtonGroup->addButton( ui->phongRadioButton );
    radioButtonGroup->addButton( ui->lambertRadioButton );
    radioButtonGroup->addButton( ui->blinnPhongRadioButton );

    connect( radioButtonGroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( onChangedShadingParameter() ) );
    connect( ui->AmbientDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
    connect( ui->DiffuseDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
    connect( ui->SpecularDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
    connect( ui->ShininessDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
}

void ShadingControl::onChangedShadingParameter()
{
    const int size = m_screen->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );
        if( rendererBase )
        {
            shading( rendererBase );
        }
    }
    m_screen->update();
}
