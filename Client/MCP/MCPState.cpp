#include "MCPState.h"
#include "ObjectEditor.h"
#include "TimeStepControlToolBar.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/Camera>
#include <QDateTime>
#include <QThread>
#include <QCoreApplication>
#include <cmath>

namespace
{
QVariant Number( double value ) { return std::isfinite( value ) ? QVariant( value ) : QVariant(); }
}

QVariantList MCPState::vector( const kvs::Vec3& value )
{
    return { Number( value.x() ), Number( value.y() ), Number( value.z() ) };
}

QVariantList MCPState::matrix( const kvs::Mat4& value )
{
    QVariantList rows;
    for( int row = 0; row < 4; ++row )
    {
        QVariantList columns;
        for( int col = 0; col < 4; ++col ) columns.append( Number( value[row][col] ) );
        rows.append( QVariant( columns ) );
    }
    return rows;
}

QVariantMap MCPState::collect( kvs::qt::jaea::Screen* screen,
                              kvs::StochasticRenderingCompositor* compositor,
                              Viz::Mode mode, const ObjectEditor* objects,
                              const TimeStepControlToolBar* time )
{
    Q_ASSERT( QThread::currentThread() == QCoreApplication::instance()->thread() );
    QString modeName;
    switch( mode )
    {
    case Viz::Mode::Local: modeName = "local"; break;
    case Viz::Mode::LocalClientAndServer: modeName = "local_client_server"; break;
    case Viz::Mode::RemoteClientAndServer: modeName = "remote_client_server"; break;
    case Viz::Mode::RemoteInSitu: modeName = "remote_in_situ"; break;
    }
    auto* camera = screen->scene()->camera();
    const auto objectList = objects->mcpObjects();
    const auto step = time->appliedTimeStep();
    const QVariantMap cameraState{
        { "position", vector( camera->position() ) },
        { "look_at", vector( camera->lookAt() ) },
        { "up", vector( camera->upVector() ) },
        { "xform", matrix( camera->xform().toMatrix() ) },
        { "view_matrix", matrix( camera->viewingMatrix() ) },
        { "projection_type", int( camera->projectionType() ) },
        { "field_of_view_degrees", Number( camera->fieldOfView() ) },
        { "near", Number( camera->front() ) }, { "far", Number( camera->back() ) },
        { "left", Number( camera->left() ) }, { "right", Number( camera->right() ) },
        { "bottom", Number( camera->bottom() ) }, { "top", Number( camera->top() ) },
        { "projection_matrix", matrix( camera->projectionMatrix() ) }
    };
    return {
        { "captured_at", QDateTime::currentDateTimeUtc().toString( Qt::ISODateWithMs ) },
        { "visualization_mode", modeName.isEmpty() ? QVariant() : QVariant( modeName ) },
        { "viewport", QVariantMap{ { "width", int( screen->width() ) },
                                   { "height", int( screen->height() ) },
                                   { "device_pixel_ratio", screen->devicePixelRatioF() } } },
        { "camera", cameraState },
        { "object_manager_xform", matrix( screen->scene()->objectManager()->xform().toMatrix() ) },
        { "repetition_level", QVariant::fromValue( qulonglong( compositor->repetitionLevel() ) ) },
        { "current_time_step", objectList.isEmpty() || step < 0 ? QVariant() : QVariant( step ) },
        { "objects", objectList }
    };
}
