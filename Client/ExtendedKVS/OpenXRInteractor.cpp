/*****************************************************************************/
/**
 *  @file   OpenXRInteractor.cpp
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#include "OpenXRInteractor.h"
#include <kvs/Assert>
#include <kvs/ObjectManager>
#include <kvs/StochasticTexturedPolygonRenderer>
#include <kvs/StochasticLineRenderer>
#include "EventTimer.h"
#include "VRControllerVisualSettings.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cmath>
#ifdef ASSIMP
#include "../../Shared/TexturedPolygonImporter.h"
#endif

namespace
{
namespace visual = kvs::openxr::controller_visual;

kvs::Mat3 VisualRotation( const kvs::Vec3& degrees )
{
    return kvs::Mat3::RotationZ( degrees.z() ) *
           kvs::Mat3::RotationY( degrees.y() ) * kvs::Mat3::RotationX( degrees.x() );
}

kvs::Xform CalibratedControllerXform( const kvs::Xform& grip, const kvs::UInt32 side )
{
    // Rotate locally without changing the tracked position or ControllerStatus.
    // Art-specific scale/translation/rotation must not affect fly-through.
    return grip * kvs::Xform::Rotation(
        VisualRotation( visual::settings.pose[side].rotation_degrees ) );
}

kvs::Xform VisualCorrection( const kvs::UInt32 side )
{
    const auto& correction = visual::settings.model[side];
    return kvs::Xform( correction.translation, correction.scale,
                       VisualRotation( correction.rotation_degrees ) );
}

void VisualBounds( kvs::ObjectBase* object, const kvs::Vec3& min, const kvs::Vec3& max )
{
    // These are normalization metadata, not the mesh's physical size. Rendering
    // uses its raw vertices and the controller xform, as the existing points do.
    object->setMinMaxObjectCoords( min, max );
    object->setMinMaxExternalCoords( min, max );
}

std::unique_ptr<kvs::TexturedPolygonObject> ControllerLabels(
    const visual::Label* labels, const size_t count )
{
    auto object = std::make_unique<kvs::TexturedPolygonObject>();
    object->setPolygonTypeToTriangle();
    object->setColorTypeToVertex();
    object->setNormalTypeToVertex();
    std::vector<kvs::Real32> coords, normals, uvs;
    std::vector<kvs::UInt32> ids, connections;
    const auto& settings = visual::settings;
    const kvs::Mat3 rotation = VisualRotation( settings.label_rotation_degrees );
    const kvs::Vec3 normal = rotation * kvs::Vec3( 0.0f, 0.0f, 1.0f );
    const float half_width = settings.label_size.x() * 0.5f;
    const float half_height = settings.label_size.y() * 0.5f;
    const kvs::Vec3 corners[] = {
        { -half_width, -half_height, 0.0f }, { half_width, -half_height, 0.0f },
        { half_width, half_height, 0.0f }, { -half_width, half_height, 0.0f }
    };
    const kvs::Vec2 texcoords[] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
    const int triangles[] = { 0, 1, 2, 0, 2, 3 };
    for ( size_t i = 0; i < count; ++i )
    {
        QImage image( settings.texture_width, settings.texture_height, QImage::Format_RGBA8888 );
        if ( image.isNull() ) { throw std::runtime_error( "Cannot allocate controller label image" ); }
        image.fill( Qt::black );
        {
            QPainter painter( &image );
            painter.setRenderHint( QPainter::TextAntialiasing );
            QFont font( QString::fromUtf8( settings.font_family ) );
            font.setPixelSize( settings.font_pixels );
            painter.setFont( font );
            painter.setPen( Qt::white );
            const int margin = settings.text_margin_pixels;
            painter.drawText( image.rect().adjusted( margin, margin, -margin, -margin ),
                              Qt::AlignCenter, QString::fromUtf8( labels[i].text ) );
        }
        kvs::ValueArray<kvs::UInt8> pixels( size_t( image.width() ) * image.height() * 4 );
        for ( int y = 0; y < image.height(); ++y )
        {
            std::copy_n( image.constScanLine( y ), size_t( image.width() ) * 4,
                         pixels.data() + size_t( y ) * image.width() * 4 );
        }
        object->addColorArray( static_cast<kvs::UInt32>( i ), pixels, image.width(), image.height() );
        for ( const int corner : triangles )
        {
            const kvs::Vec3 p = labels[i].centre + rotation * corners[corner];
            coords.insert( coords.end(), { p.x(), p.y(), p.z() } );
            normals.insert( normals.end(), { normal.x(), normal.y(), normal.z() } );
            uvs.insert( uvs.end(), { texcoords[corner].x(), texcoords[corner].y() } );
            ids.push_back( static_cast<kvs::UInt32>( i ) );
            // KVS's textured renderer expands triangles through connections.
            connections.push_back( static_cast<kvs::UInt32>( connections.size() ) );
        }
    }
    object->setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    object->setNormals( kvs::ValueArray<kvs::Real32>( normals ) );
    object->setTexture2DCoords( kvs::ValueArray<kvs::Real32>( uvs ) );
    object->setTextureIds( kvs::ValueArray<kvs::UInt32>( ids ) );
    object->setConnections( kvs::ValueArray<kvs::UInt32>( connections ) );
    object->hide();
    return object;
}

std::unique_ptr<kvs::LineObject> ControllerLeaders( const visual::Label* labels, const size_t count )
{
    std::vector<kvs::Real32> coords;
    std::vector<kvs::UInt32> connections;
    for ( size_t i = 0; i < count; ++i )
    {
        for ( const auto& p : { labels[i].leader_start, labels[i].button } )
        {
            coords.insert( coords.end(), { p.x(), p.y(), p.z() } );
        }
        connections.push_back( static_cast<kvs::UInt32>( i * 2 ) );
        connections.push_back( static_cast<kvs::UInt32>( i * 2 + 1 ) );
    }
    auto object = std::make_unique<kvs::LineObject>();
    object->setLineTypeToSegment();
    object->setColorTypeToLine();
    object->setColor( kvs::RGBColor::White() );
    object->setSize( visual::settings.leader_width_pixels );
    object->setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    object->setConnections( kvs::ValueArray<kvs::UInt32>( connections ) );
    object->hide();
    return object;
}
}

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new OpenXRInteractor class.
 */
/*===========================================================================*/
OpenXRInteractor::OpenXRInteractor( kvs::openxr::OpenXRScreen* screen ) :
    m_openxr_screen( screen )
{
    kvsMessageDebug( "OpenXRInteractor::OpenXRInteractor" );

    KVS_ASSERT( m_openxr_screen != nullptr );
    BaseClass::addEventType( kvs::EventBase::KeyPressEvent );
    BaseClass::addEventType( kvs::EventBase::InitializeEvent );
    BaseClass::addEventType( kvs::EventBase::PaintEvent );
    BaseClass::addEventType( kvs::EventBase::TimerEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerPressEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerReleaseEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerMoveEvent );
    BaseClass::addEventType( kvs::EventBase::ControllerAxisEvent );
    BaseClass::setTimerInterval( 1 );
}

/*===========================================================================*/
/**
 *  @brief  Destructs a OpenXRInteractor class.
 */
/*===========================================================================*/
OpenXRInteractor::~OpenXRInteractor()
{
    kvsMessageDebug( "OpenXRInteractor::~OpenXRInteractor" );
}

/*===========================================================================*/
/**
 *  @brief  Initialize event.
 */
/*===========================================================================*/
void OpenXRInteractor::initializeEvent()
{
    KVS_ASSERT( m_openxr_screen != nullptr );
    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

#if 0
    {
        kvs::ValueArray<kvs::Real32> x_coords =
        {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
        };
        m_axis_x = new kvs::LineObject();
        m_axis_x->setLineTypeToStrip();
        m_axis_x->setCoords(x_coords);
        m_axis_x->setColorTypeToLine();
        m_axis_x->setColor(kvs::RGBColor::Red());
        m_axis_x->setSize(30.0f);
        s->registerObject(m_axis_x, new kvs::StochasticLineRenderer());

        kvs::ValueArray<kvs::Real32> y_coords =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
        };
        m_axis_y = new kvs::LineObject();
        m_axis_y->setLineTypeToStrip();
        m_axis_y->setCoords(y_coords);
        m_axis_y->setColorTypeToLine();
        m_axis_y->setColor(kvs::RGBColor::Green());
        m_axis_y->setSize(30.0f);
        s->registerObject(m_axis_y, new kvs::StochasticLineRenderer());

        kvs::ValueArray<kvs::Real32> z_coords =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
        };
        m_axis_z = new kvs::LineObject();
        m_axis_z->setLineTypeToStrip();
        m_axis_z->setCoords(z_coords);
        m_axis_z->setColorTypeToLine();
        m_axis_z->setColor(kvs::RGBColor::Blue());
        m_axis_z->setSize(30.0f);
        s->registerObject(m_axis_z, new kvs::StochasticLineRenderer());
    }
#endif

    // Use the same normalization bounds as the coordinate points. Copying these
    // before registration prevents controller art/labels from resizing the data.
    kvs::Vec3 min = s->objectManager()->minObjectCoord();
    kvs::Vec3 max = s->objectManager()->maxObjectCoord();
    if ( min.x() > max.x() || min.y() > max.y() || min.z() > max.z() )
    {
        min = s->objectManager()->minExternalCoord();
        max = s->objectManager()->maxExternalCoord();
    }
    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        const QString path = QDir( QCoreApplication::applicationDirPath() ).absoluteFilePath(
            QString::fromUtf8( visual::settings.model_path[i] ) );
        const char* side = i == kvs::Side::Left ? "left" : "right";
#ifdef ASSIMP
        try
        {
            if ( !QFileInfo( path ).isFile() )
            {
                qWarning().noquote() << "Touch Pro" << side << "FBX missing; using PointObject:" << path;
            }
            else
            {
                // Reuse Shared's FBX -> Assimp -> TexturedPolygonImporter route.
                // Relative textures are anchored to this FBX, independent of cwd.
                auto model = std::make_unique<kvs::TexturedPolygonImporter>(
                    path.toUtf8().toStdString(), true, visual::settings.base_color_texture[i] );
                if ( model->isFailure() || model->coords().empty() || model->connections().empty() )
                {
                    qWarning().noquote() << "Touch Pro" << side << "FBX load failed; using PointObject:" << path;
                }
                else
                {
                    // The textured renderer draws by texture ID; reject unusable geometry.
                    bool valid = model->coords().size() % 3 == 0 &&
                        model->connections().size() % 3 == 0 &&
                        model->normals().size() == model->coords().size() &&
                        model->textureIds().size() == model->coords().size() / 3 &&
                        model->texture2DCoords().size() == model->coords().size() / 3 * 2;
                    for ( const auto index : model->connections() )
                    {
                        if ( index >= model->coords().size() / 3 ) { valid = false; break; }
                    }
                    for ( const auto id : model->textureIds() )
                    {
                        if ( model->mapIdToColorArray().count( id ) == 0 ) { valid = false; break; }
                    }
                    for ( const auto value : model->coords() )
                    {
                        if ( !std::isfinite( value ) ) { valid = false; break; }
                    }
                    if ( !valid )
                    {
                        qWarning() << "Touch Pro" << side << "FBX geometry/textures invalid; using PointObject";
                    }
                    else
                    {
                        model->setColorTypeToVertex();
                        model->setName( i == kvs::Side::Left ? "TouchProLeft" : "TouchProRight" );
                        model->hide();
                        VisualBounds( model.get(), min, max );
                        auto renderer = std::make_unique<kvs::StochasticTexturedPolygonRenderer>();
                        renderer->setShader( kvs::Shader::Phong() );
                        s->registerObject( model.get(), renderer.get() );
                        renderer.release();
                        m_controller_model[i] = model.release();
                    }
                }
            }
        }
        catch ( const std::exception& error )
        {
            qWarning() << "Touch Pro" << side << "FBX load failed; using PointObject:" << error.what();
        }
#else
        qWarning().noquote() << "Touch Pro" << side << "requires ASSIMP; using PointObject:" << path;
#endif
        // Instructions are useful with either the model or the point fallback.
        const auto* labels = i == kvs::Side::Left ? visual::left_labels : visual::right_labels;
        const size_t count = i == kvs::Side::Left ?
            sizeof( visual::left_labels ) / sizeof( visual::left_labels[0] ) :
            sizeof( visual::right_labels ) / sizeof( visual::right_labels[0] );
        for ( const auto visibility : { visual::LabelVisibility::Always,
                                       visual::LabelVisibility::GripPressed,
                                       visual::LabelVisibility::GripReleased } )
        {
            std::vector<visual::Label> group;
            for ( size_t j = 0; j < count; ++j )
            {
                if ( labels[j].visibility == visibility ) { group.push_back( labels[j] ); }
            }
            if ( group.empty() ) { continue; }
            auto label_object = ControllerLabels( group.data(), group.size() );
            auto leader_object = ControllerLeaders( group.data(), group.size() );
            VisualBounds( label_object.get(), min, max );
            VisualBounds( leader_object.get(), min, max );
            auto label_renderer = std::make_unique<kvs::StochasticTexturedPolygonRenderer>();
            label_renderer->disableShading(); // Keep text white and background black.
            label_renderer->setBilinearInterpolation();
            s->registerObject( label_object.get(), label_renderer.get() );
            label_renderer.release();
            m_controller_annotations[i].push_back( { label_object.release(), visibility } );
            s->registerObject( leader_object.get(), new kvs::StochasticLineRenderer() );
            m_controller_annotations[i].push_back( { leader_object.release(), visibility } );
        }
    }

    const auto& pointer_origin = visual::settings.pointer_origin;
    kvs::ValueArray<kvs::Real32> pointer_coords =
        {
            pointer_origin.x(), pointer_origin.y(), pointer_origin.z(),
            pointer_origin.x(), pointer_origin.y(), pointer_origin.z() - visual::settings.pointer_length,
        };

    m_pointer = new kvs::LineObject();
    m_pointer->setLineTypeToStrip();
    m_pointer->setCoords( pointer_coords );
    m_pointer->setColorTypeToLine();
    m_pointer->setColor( kvs::RGBColor::Green() );
    m_pointer->setSize( visual::settings.pointer_width_pixels );
    VisualBounds( m_pointer, min, max );
    m_pointer->hide();
    s->registerObject( m_pointer, new kvs::StochasticLineRenderer() );

    BaseClass::setEventTimer( new kvs::qt::EventTimer( this ) );
    BaseClass::eventTimer()->start( BaseClass::timerInterval() );
}

void OpenXRInteractor::setControllerVisualBounds( const kvs::Vec3& min, const kvs::Vec3& max )
{
    for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
    {
        if ( m_controller_model[i] ) { VisualBounds( m_controller_model[i], min, max ); }
        for ( const auto& annotation : m_controller_annotations[i] ) { VisualBounds( annotation.object, min, max ); }
    }
    if ( m_pointer ) { VisualBounds( m_pointer, min, max ); }
}

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
void OpenXRInteractor::paintEvent()
{
    KVS_ASSERT( m_openxr_screen != nullptr );

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    s->updateGLProjectionMatrix();
    s->updateGLViewingMatrix();
}

/*===========================================================================*/
/**
 *  @brief  Time event.
 *  @param  e [in] time event
 */
/*===========================================================================*/
void OpenXRInteractor::timerEvent( kvs::TimeEvent* e )
{
    BaseClass::screen()->redraw();
}

/*===========================================================================*/
/**
 *  @brief  controller press event.
 *  @param  e [in] controller event
 */
/*===========================================================================*/
void OpenXRInteractor::controllerPressEvent( kvs::ControllerEvent* e )
{
    KVS_ASSERT( m_openxr_screen != nullptr );

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
    if ( e->type() == kvs::EventBase::ControllerPressEvent )
    {
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            for( kvs::UInt32 j = 0; j < kvs::Controller::Button::Max; ++j )
            {
                if ( cs.button_status[i][j].pressed )
                {
                    if ( j == kvs::Controller::Button::Trigger )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Trigger", i );
                    }
                    if ( j == kvs::Controller::Button::Menu )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Menu", i );
                    }
                    if ( j == kvs::Controller::Button::A )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed A", i );
                    }
                    if ( j == kvs::Controller::Button::B )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed B", i );                    
                    }
                    if ( j == kvs::Controller::Button::X )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed X", i );
                    }
                    if ( j == kvs::Controller::Button::Y )
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Pressed Y", i );
                    }
                }
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Release event.
 *  @param  e [in] controller event
 */
/*===========================================================================*/
void OpenXRInteractor::controllerReleaseEvent(kvs::ControllerEvent* e)
{
    KVS_ASSERT( m_openxr_screen != nullptr );

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();

    if ( e->type() == kvs::EventBase::ControllerReleaseEvent )
    {
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            for ( kvs::UInt32 j = 0; j < kvs::Controller::Button::Max; ++j )
            {
                bool released = cs.button_status[i][j].released;
                if ( released )
                {
                    if (j == kvs::Controller::Button::Trigger)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released Trigger", i );
                    }
                    if (j == kvs::Controller::Button::Menu)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released Menu", i );
                    }
                    if (j == kvs::Controller::Button::A)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released A", i );
                    }
                    if (j == kvs::Controller::Button::B)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released B", i );
                    }
                    if (j == kvs::Controller::Button::X)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released X", i );
                    }
                    if (j == kvs::Controller::Button::Y)
                    {
                        kvsMessageDebug( "OpenXRInteractor::controllerPressEvent() controller(%d) Released Y", i );
                    }
                }
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Release event.
 *  @param  e [in] controller event
 */
/*===========================================================================*/
void OpenXRInteractor::controllerMoveEvent( kvs::ControllerEvent* e )
{
    KVS_ASSERT( m_openxr_screen != nullptr);

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    if ( m_axis_x ) { m_axis_x->resetXform(); }
    if ( m_axis_y ) { m_axis_y->resetXform(); }
    if ( m_axis_z ) { m_axis_z->resetXform(); }

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
    if ( e->type() == kvs::EventBase::ControllerMoveEvent )
    {
        kvs::Xform walkthrough_xform = m_openxr_screen->walkthrough();
        kvs::Xform head_xform = walkthrough_xform * m_openxr_screen->headXform();
        kvs::UInt32 ops_controller_index = m_openxr_screen->opsHand();
        kvs::Xform ops_cotroller_xform = walkthrough_xform * cs.xform[ops_controller_index];
        bool pressed = cs.button_status[ops_controller_index][kvs::Controller::Button::Trigger].pressed;
        bool pressing = cs.button_status[ops_controller_index][kvs::Controller::Button::Trigger].pressing;
        bool released = cs.button_status[ops_controller_index][kvs::Controller::Button::Trigger].released;

        if ( cs.button_status[kvs::Side::Left][kvs::Controller::Button::Trigger].pressing &&
            cs.button_status[kvs::Side::Right][kvs::Controller::Button::Trigger].pressing )
        {
            kvs::Xform left_c = walkthrough_xform * cs.xform[kvs::Side::Left];
            kvs::Xform left_p = walkthrough_xform * cs.last_xform[kvs::Side::Left];
            kvs::Xform right_c = walkthrough_xform * cs.xform[kvs::Side::Right];
            kvs::Xform right_p = walkthrough_xform * cs.last_xform[kvs::Side::Right];

            // scaling
            float dist_c = ( left_c.translation() - right_c.translation() ).length();
            float dist_p = ( left_p.translation() - right_p.translation() ).length();
            float scale = dist_c / dist_p;

            // translation
            kvs::Vec3 head_pos = head_xform.translation();
            kvs::Mat4 m = head_xform.toMatrix();
            kvs::Vec3 head_dir = -kvs::Vec3( m[0][2], m[1][2], m[2][2] );
            head_dir.normalize();

            kvs::Vec3 center_c = (left_c.translation() + right_c.translation()) / 2.0f;
            kvs::Vec3 center_p = (left_p.translation() + right_p.translation()) / 2.0f;
            kvs::Vec3 center_diff = center_c - center_p;
            kvs::Vec3 center_diff_xy = kvs::Vec3( center_diff.x(), center_diff.y(), 0.0f );
            kvs::Vec3 cont_to_head = center_p - head_pos;
            float cont_distance = kvs::Math::Abs( cont_to_head.dot( head_dir ) );
            kvs::Vec3 object_pos = s->objectManager()->xform().translation();
            kvs::Vec3 object_to_head = object_pos - head_pos;
            float obj_distance = kvs::Math::Abs( object_to_head.dot( head_dir ) );
            float factor = cont_distance < 1e-100 ? 1.0f : obj_distance / cont_distance;
            kvs::Vec3 translation = center_diff * factor;

            // rotation
            kvs::Vec3 rot_c = left_c.translation() - right_c.translation();
            kvs::Vec3 rot_p = left_p.translation() - right_p.translation();
            kvs::Vec3 rot_axis = ( rot_c.cross( rot_p ).normalized() ) * -1.0f;
            float cos = ( rot_c.dot( rot_p ) / ( rot_c.length() * rot_p.length() ) );
            cos = kvs::Math::Clamp( cos, -1.0f, 1.0f );
            float deg = kvs::Math::Rad2Deg( std::acos( cos ) );
            kvs::Mat3 rot_mat = kvs::Mat3::Rotation( rot_axis, deg );

            s->objectManager()->scale( kvs::Vec3( scale, scale, scale ) );
            s->objectManager()->translate( translation );
            s->objectManager()->rotate( rot_mat );
        }
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            kvs::Xform xform = walkthrough_xform * cs.xform[i];
            const kvs::Xform controller_xform = CalibratedControllerXform( cs.xform[i], i );
            const kvs::Xform calibrated_xform = walkthrough_xform * controller_xform;
            const kvs::Xform visual_xform = calibrated_xform * VisualCorrection( i );
            if ( m_controller_model[i] )
            {
                m_controller_model[i]->setVisible(cs.is_active[i]);
                m_controller_model[i]->setXform( visual_xform * kvs::Xform::Scaling(
                    kvs::Vec3::Constant( visual::settings.source_unit_to_metres ) ) );
            }
            const bool grip_pressed = cs.is_active[kvs::Side::Right] &&
                m_openxr_screen->rightGripValue() >= visual::settings.grip_press_threshold;
            for ( const auto& annotation : m_controller_annotations[i] )
            {
                const bool show = annotation.visibility == visual::LabelVisibility::Always ||
                    ( annotation.visibility == visual::LabelVisibility::GripPressed && grip_pressed ) ||
                    ( annotation.visibility == visual::LabelVisibility::GripReleased && !grip_pressed );
                annotation.object->setVisible( cs.is_active[i] && show );
                annotation.object->setXform( visual_xform );
            }
            if ( i == kvs::Side::Right && m_pointer )
            {
                m_pointer->setVisible( cs.is_active[i] );
                // The same calibrated -Z as fly-through, before art correction.
                m_pointer->setXform( calibrated_xform );
            }
            if( i == 0 )
            {
                if( m_start_point != nullptr )
                {
                    m_start_point->setXform( xform );
                    m_start_point->setVisible( cs.is_active[i] && !m_controller_model[i] );
                }
            }
            else
            {
                if( m_end_point != nullptr )
                {
                    m_end_point->setXform( xform );
                    m_end_point->setVisible( cs.is_active[i] && !m_controller_model[i] );
                }
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Controller Axis event.
 *  @param  e [in] controller event
 */
/*===========================================================================*/
void OpenXRInteractor::controllerAxisEvent( kvs::ControllerEvent* e )
{
    KVS_ASSERT( m_openxr_screen != nullptr);

    auto* s = BaseClass::scene();
    KVS_ASSERT( s != nullptr );

    m_elapsed_timer.stop();
    float elapsedTime = static_cast<float>( m_elapsed_timer.sec() );
    m_elapsed_timer.start();

    const kvs::Controller::ControllerStatus& cs = e->controllerStatus();
    if ( e->type() == kvs::EventBase::ControllerAxisEvent )
    {
        for ( kvs::UInt32 i = 0; i < kvs::Side::Max; ++i )
        {
            kvs::Vec2 axis = cs.axis_value[i];
            if ( ( kvs::Math::Abs( axis.x() ) > 0 || kvs::Math::Abs( axis.y() ) > 0 ) && elapsedTime > 0.0f )
            {
                kvs::Xform walkthrough_xform = m_openxr_screen->walkthrough();
                // Use the same calibrated controller frame as the pointer.
                // Keep the vertical component so pitching the hand changes travel direction.
                kvs::Mat4 m = CalibratedControllerXform( cs.xform[i], i ).toMatrix();
                kvs::Vec3 dir = -kvs::Vec3( m[0][2], m[1][2], m[2][2] );
                dir.normalize();

                kvs::Vec3 translation = dir * axis[1] * 3.0f * elapsedTime;
                float heading = -axis[0] * 3.0f * elapsedTime;
                kvs::Mat3 rotation;
                rotation[0][0] = std::cosf( heading );
                rotation[0][1] = 0.0f;
                rotation[0][2] = std::sinf( heading );
                rotation[1][0] = 0.0f;
                rotation[1][1] = 1.0f;
                rotation[1][2] = 0.0f;
                rotation[2][0] = -std::sinf( heading );
                rotation[2][1] = 0.0f;
                rotation[2][2] = std::cosf( heading );

                kvs::Xform move_xform = kvs::Xform( translation, kvs::Vec3::Ones(), rotation );
                m_openxr_screen->setWalkthrough( walkthrough_xform * move_xform );
                break;
            }
        }
    }
}

} // end of namespace openxr

} // end of namespace kvs
