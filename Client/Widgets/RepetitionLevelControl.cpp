#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

RepetitionLevelControl::RepetitionLevelControl( kvs::qt::jaea::Screen* screen, kvs::StochasticRenderingCompositor* compositor, QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::RepetitionLevelControl )
    , m_screen( screen )
    , m_compositor( compositor )
{
    ui->setupUi(this);
    connect( ui->applyPushButton, &QPushButton::clicked, this, &RepetitionLevelControl::onApply );
}

RepetitionLevelControl::~RepetitionLevelControl()
{
    delete ui;
}

void RepetitionLevelControl::onUpdateCurrentRepetitionLevel()
{
    const size_t currentRepetitionLevel = m_compositor->repetitionLevel();
    ui->dispCurrentRepetitionLevel->setText( QString::number( currentRepetitionLevel ) );
}

void RepetitionLevelControl::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void RepetitionLevelControl::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void RepetitionLevelControl::onApply()
{
    const size_t repetitionLevel = static_cast<size_t>( ui->spinBoxNewRepetitionLevel->value() );

    const size_t size = m_screen->scene()->IDManager()->size();
    for( int index = 0; index < size; index++ )
    {
        auto ids = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->object( ids.first );
        auto* renderer = m_screen->scene()->renderer( ids.second );
        if( auto* particleRenderer = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( renderer ) )
        {
            kvs::glsl::ParticleBasedRenderer* particleBasedRenderer = new kvs::glsl::ParticleBasedRenderer();
            particleBasedRenderer->enableShuffle();
            emit shading( particleBasedRenderer );
            m_screen->scene()->replaceRenderer( ids.second, particleBasedRenderer );
        }
    }

    m_compositor->setRepetitionLevel( repetitionLevel );
    m_compositor->screen()->redraw();
    onUpdateCurrentRepetitionLevel();
}
