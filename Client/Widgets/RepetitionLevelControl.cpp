#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

#include <algorithm>
#include <QJsonDocument>

#include "../../Shared/JsonKeys.h"

RepetitionLevelControl::RepetitionLevelControl(
    kvs::qt::jaea::Screen* screen,
    kvs::StochasticRenderingCompositor* compositor,
    WebSocketPair* websockets,
    Viz::Mode* vizMode,
    QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::RepetitionLevelControl )
    , m_screen( screen )
    , m_compositor( compositor )
    , m_web_sockets( websockets )
    , m_viz_mode( vizMode )
    , m_is_operator( false )
{
    ui->setupUi( this );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &RepetitionLevelControl::onApply );
}

RepetitionLevelControl::~RepetitionLevelControl()
{
    delete ui;
}

void RepetitionLevelControl::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;
    ui->spinBoxNewRepetitionLevel->setEnabled( m_is_operator );
    ui->applyPushButton->setEnabled( m_is_operator );
}

void RepetitionLevelControl::onUpdateCurrentRepetitionLevel()
{
    const std::size_t currentRepetitionLevel = m_compositor->repetitionLevel();
    ui->dispCurrentRepetitionLevel->setText( QString::number( currentRepetitionLevel ) );
}

void RepetitionLevelControl::onReceiveRepetitionLevelParameter( const QJsonObject& payload )
{
    const QString repeatLevelKey = QString::fromUtf8( Protocol::Key::RepeatLevel );
    if( !payload.value( repeatLevelKey ).isDouble() ) { return; }

    int repetitionLevel = payload.value( repeatLevelKey ).toInt();
    repetitionLevel = std::max( ui->spinBoxNewRepetitionLevel->minimum(), repetitionLevel );
    repetitionLevel = std::min( ui->spinBoxNewRepetitionLevel->maximum(), repetitionLevel );

    ui->spinBoxNewRepetitionLevel->setValue( repetitionLevel );
    ui->dispCurrentRepetitionLevel->setText( QString::number( repetitionLevel ) );
    const int size = m_screen->scene()->IDManager()->size();
    const size_t newRepetitionLevel = static_cast<size_t>( repetitionLevel );

#ifdef OPENXR_SCREEN
    const kvs::Xform initialXform
        (
            kvs::Mat4(
                1, 0, 0, 0 ,
                0, 1, 0, 0 ,
                0, 0, 1, 12,
                0, 0, 0, 1
                )
            );
    kvs::Vec3 translationOffset = m_screen->scene()->camera()->xform().translation() - initialXform.translation();
#endif

    for( int index = 0; index < size; ++index )
    {
        const auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );

        if( !object || !rendererBase ) { continue; }

        auto* pointObject = dynamic_cast<kvs::PointObject*>( object );
        if ( !pointObject ) { continue; }

        auto* stochastic = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase );
        if ( !stochastic ) { continue; }

        auto* particle = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochastic );
        if ( !particle ) { continue; }

        auto* newRenderer = new kvs::glsl::ParticleBasedRenderer();
        newRenderer->enableShuffle();
#ifdef OPENXR_SCREEN
        newRenderer->setTranslationOffset( translationOffset );
        newRenderer->setObjectDepth( m_screen->scene()->objectManager()->xform().scaling().z() / m_screen->scene()->camera()->xform().scaling().z() );
#endif
        // NOTE:レンダラーを作り直したのでシェーディングも設定しなおす必要がある
        emit shading( newRenderer );

        m_screen->scene()->replaceRenderer( id.second, newRenderer );
    }

    m_compositor->setRepetitionLevel( newRepetitionLevel );
    m_compositor->screen()->redraw();

    onUpdateCurrentRepetitionLevel();
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
    const std::size_t repetitionLevel = static_cast<size_t>( ui->spinBoxNewRepetitionLevel->value() );

    const int size = m_screen->scene()->IDManager()->size();

#ifdef OPENXR_SCREEN
    const kvs::Xform initialXform
        (
            kvs::Mat4(
                1, 0, 0, 0 ,
                0, 1, 0, 0 ,
                0, 0, 1, 12,
                0, 0, 0, 1
                )
            );
    kvs::Vec3 translationOffset = m_screen->scene()->camera()->xform().translation() - initialXform.translation();
#endif

    for( int index = 0; index < size; ++index )
    {
        const auto id = m_screen->scene()->IDManager()->id( index );
        auto* object = m_screen->scene()->objectManager()->object( id.first );
        auto* rendererBase = m_screen->scene()->rendererManager()->renderer( id.second );

        if( !object || !rendererBase ) { continue; }

        auto* pointObject = dynamic_cast<kvs::PointObject*>( object );
        if ( !pointObject ) { continue; }

        auto* stochastic = dynamic_cast<kvs::StochasticRendererBase*>( rendererBase );
        if ( !stochastic ) { continue; }

        auto* particle = dynamic_cast<kvs::glsl::ParticleBasedRenderer*>( stochastic );
        if ( !particle ) { continue; }

        auto* newRenderer = new kvs::glsl::ParticleBasedRenderer();
        newRenderer->enableShuffle();
#ifdef OPENXR_SCREEN
        newRenderer->setTranslationOffset( translationOffset );
        newRenderer->setObjectDepth( m_screen->scene()->objectManager()->xform().scaling().z() / m_screen->scene()->camera()->xform().scaling().z() );
#endif
        // NOTE:レンダラーを作り直したのでシェーディングも設定しなおす必要がある
        emit shading( newRenderer );

        m_screen->scene()->replaceRenderer( id.second, newRenderer );
    }

    m_compositor->setRepetitionLevel( repetitionLevel );
    m_compositor->screen()->redraw();

    onUpdateCurrentRepetitionLevel();

    if( !m_web_sockets || !m_web_sockets->isConnected() ) { return; }
    if( !m_viz_mode || *m_viz_mode != Viz::Mode::RemoteInSitu ) { return; }
    if( !m_is_operator ) { return; }

    QJsonObject repetitionLevelParameter;
    repetitionLevelParameter[QString::fromUtf8( Protocol::Key::Event )] = QString::fromUtf8( Protocol::Events::RepetitionLevelParameter );
    repetitionLevelParameter[QString::fromUtf8( Protocol::Key::RepeatLevel )] = static_cast<int>( repetitionLevel );

    m_web_sockets->text()->sendTextMessage( QJsonDocument( repetitionLevelParameter ).toJson( QJsonDocument::Compact ) );
}
