#include "ShadingControl.h"
#include "ui_ShadingControl.h"

ShadingControl::ShadingControl( kvs::qt::jaea::Screen* screen, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::ShadingControl )
    , m_screen( screen )
{
    ui->setupUi( this );

    QButtonGroup *radioButtonGroup = new QButtonGroup();
    radioButtonGroup->addButton( ui->noneRadioButton );
    radioButtonGroup->addButton( ui->phongRadioButton );
    radioButtonGroup->addButton( ui->lambertRadioButton );
    radioButtonGroup->addButton( ui->blinnPhongRadioButton );

    connect( radioButtonGroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( onChangedShadingParameter() ) );
    connect( ui->AmbientDoubleSpinBox  , &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
    connect( ui->DiffuseDoubleSpinBox  , &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
    connect( ui->SpecularDoubleSpinBox , &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
    connect( ui->ShininessDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ShadingControl::onChangedShadingParameter );
}

ShadingControl::~ShadingControl()
{
    delete ui;
}

void ShadingControl::onShading( kvs::RendererBase* rendererBase )
{
    auto* stochastic = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase );
    if( !stochastic ) { return; }

    const float ambient   = static_cast<float>( ui->AmbientDoubleSpinBox->value() );
    const float diffuse   = static_cast<float>( ui->DiffuseDoubleSpinBox->value() );
    const float specular  = static_cast<float>( ui->SpecularDoubleSpinBox->value() );
    const float shininess = static_cast<float>( ui->ShininessDoubleSpinBox->value() );

    auto applyShader = [&]( auto* r )
    {
        if( ui->noneRadioButton->isChecked() )
        {
            r->setShader( kvs::Shader::Lambert( 1, 0 ) );
        }
        else if( ui->phongRadioButton->isChecked() )
        {
            r->setShader( kvs::Shader::Phong( ambient, diffuse, specular, shininess ) );
        }
        else if ( ui->lambertRadioButton->isChecked() )
        {
            r->setShader( kvs::Shader::Lambert( ambient, diffuse ) );
        }
        else if( ui->blinnPhongRadioButton->isChecked() )
        {
            r->setShader( kvs::Shader::BlinnPhong( ambient, diffuse, specular, shininess ) );
        }
    };

    if( auto* polygon = dynamic_cast<kvs::StochasticPolygonRenderer*>( stochastic ) )
    {
        applyShader( polygon );
        return;
    }

    if( auto* particle = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochastic ) )
    {
        if ( !particle->isShuffleEnabled() ) { particle->enableShuffle(); }
        applyShader( particle );
        return;
    }

    if( auto* textured = dynamic_cast<kvs::StochasticTexturedPolygonRenderer*>( stochastic ) )
    {
        applyShader( textured );
        return;
    }
}

void ShadingControl::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ShadingControl::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void ShadingControl::onChangedShadingParameter()
{
    const int size = m_screen->scene()->IDManager()->size();
    for( int index = 0; index < size; ++index )
    {
        auto id = m_screen->scene()->IDManager()->id( index );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );
        if( rendererBase )
        {
            onShading( rendererBase );
        }
    }
    m_screen->update();
}
