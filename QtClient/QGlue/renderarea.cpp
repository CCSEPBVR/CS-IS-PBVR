#include "screen.h"
#include "QGlue/renderarea.h"

#include "Client/KeyFrameAnimation.h"
#include "Panels/animationcontrols.h"

#include <Client/timer_simple.h>
#include <QDebug>
#include <QKeyEvent>

#include <Client/v3defines.h>

//KVS2.7.0
//ADD BY)T.Osaki 2020.06.04
#include <kvs/Background>
//#include <kvs/ObjectManager>

// 20210902 yodo
#include "PBVRObjectManager.h"
#include "Scene.h"

#include "objnameutil.h"

#ifdef PBVR_DEBUG
int     timestep = 0;
#endif

//extern QMutex paint_mutex;
char RenderArea::shadinglevel[256] = "";
kvs::visclient::TimerEvent* RenderArea::g_timer_event=0;


RenderArea::PointObjectProxy RenderArea::m_point_object;
RenderArea::RenderArea( QWidget* parent_surface):
    m_obj_id_pair (-1,-1)
{

    Q_UNUSED( parent_surface);
    pixelRatio = parent_surface->devicePixelRatio();

    this->m_scene->background()->setColor( kvs::RGBAColor(0,0,22,1.0f) );
    this->i_w= 620;//Qthis->width();
    this->i_h= 620;//Qthis->height();
    this->setSize(i_w,i_h);
    std::cout<< "RenderArea::RenderArea"<<std::endl;
    m_stepLabel =new QGlue::StepLabel(this,extCommand);
    m_stepLabel->setPosition(150*pixelRatio,50);
    m_labels.append(m_stepLabel);

#ifdef SHOW_ORIENTATION_AXIS
    int size=90*pixelRatio;
    m_orientation_axis=new QGlue::OrientationAxis( this);
    m_orientation_axis->setPosition( ScreenBase::width() -(size + 10),10);
    m_orientation_axis->setBoxType( QGlue::OrientationAxis::SolidBox );
    m_orientation_axis->setSize(size);
    m_orientation_axis->show();
#endif // SHOW_ORIENTATION_AXIS

    // Setup Legend bar
    g_legend= new QGlue::LegendBar( this, *extCommand );
    g_legend->setOrientation( QGlue::LegendBar::Horizontal );
    g_legend->setPosition( 10, ScreenBase::height() -10 );
    g_legend->setWidth(50*pixelRatio);
    g_legend->setHeight(180*pixelRatio);

    g_legend->screenResizedAfterSelectTransferFunction( 1 );
    g_legend->show();

}

/**
 * @brief RenderArea::enableRendererShading, enable render shading
 */
void RenderArea::enableRendererShading(){
//    m_renderer->enableShading();
    m_renderer.enableShading();
}

/**
 * @brief RenderArea::storeCurrentXForm stores the current ObjectManager XForm
 */
void  RenderArea::storeCurrentXForm(){
    m_stored_xf=m_scene->objectManager()->xform();
}
/**
 * @brief RenderArea::restoreXForm restores the saved xform
 */
void RenderArea::restoreXForm()
{
    // Restore Saved X form
    m_scene->objectManager()->translate(m_stored_xf.translation());
    m_scene->objectManager()->scale(m_stored_xf.scaling());
    m_scene->objectManager()->rotate(m_stored_xf.rotation());
}

/**
 * @brief RenderArea::setRenderRepetitionLevel, set render repetition level.
 * @param level
 */
void RenderArea::setRenderRepetitionLevel(int level)
{
//    for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , start (level=" << level << ")" << std::endl;
//    nstart++;

    level=level>1?level:1;
//    if(level != m_renderer.getRepetitionLevel()){

    // 20220909 : yodo fixed to set repetition level to compositor, not to renderer.
    m_compositor->setRepetitionLevel(level);
        //m_renderer.setRepetitionLevel( level );

//    std::cerr << "replace renderer to apply new repeat level to renderer." << std::endl;

        // Replacement of renderer is required to get correct output with
        // kvs::glsl::ParticleBasedRenderer when changing repetition level after the
        // the renderers first render function call. (kvs 2.9.0)
        m_scene->replaceRenderer(m_obj_id_pair.second, m_renderer.pbr_pointer(), false);

        // The lines below are a work around for m_initial_modelview problem that occurs
        // when replacing the renderer while zoom is applied.
        // in kvs::glsl::ParticleBasedRenderer.  (kvs 2.9.0)

//        std::cerr << "call makeCurrent()" << std::endl;

//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

//         GLenum err = glGetError();
//         assert(err == GL_NO_ERROR);
        makeCurrent();
        // Save current X form

//        std::cerr << "call storeCurrentXForm()" << std::endl;
//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

        storeCurrentXForm();
        // Reset object Manager's XForm

//        std::cerr << "call ObjectManager::resetXform()" << std::endl;
//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

        m_scene->objectManager()->resetXform();

//        std::cerr << "call Compositor::update() (1)" << std::endl;
//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

        // Manually call  paint function, to set m_initial_xform in Renderer
//        m_scene->paintFunction();
        m_compositor->update();

//        std::cerr << "call restoreXForm()" << std::endl;
//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

        // Restore xform to original scene xform
        restoreXForm();

//        std::cerr << "call Compositor::update() (2)" << std::endl;
//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

        // Paint the frame again, to erase previous frame
//        m_scene->paintFunction();
        m_compositor->update();

//        std::cerr << "call doneCurrent()" << std::endl;
//        {
//            float tmp_mp[16]; m_point_object->xform().toArray( tmp_mp );
//            kvs::Mat4 mp (tmp_mp);
//            float tmp_mm[16]; m_scene->objectManager()->xform().toArray( tmp_mm );
//            kvs::Mat4 mm (tmp_mm);
//            std::cerr << "m(p)=" << std::endl;
//            std::cerr << mp << std::endl;
//            std::cerr << "m(m)=" << std::endl;
//            std::cerr << mm << std::endl;
//        }

        doneCurrent(); // This line may cause an OpenGL error.
//         err = glGetError();
//         assert(err == GL_NO_ERROR);
//    }
//        for(int nindent = 0; nindent < nstart-1; nindent++) std::cout << "    ";
//        std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , end" << std::endl;
//        nstart--;
}


/**
 * @brief RenderArea::setRenderSubPixelLevel, set render sub pixel level.
 *                    When this is called in GPU mode level will be squared before applied.
 *                    When this is called in GPU mode, renderer instance will be recreated and replaced in scene.
 * @param level
 */
void RenderArea::setRenderSubPixelLevel(int level)
{
//    for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , start" << std::endl;
//    nstart++;

    level=level>1?level:1;
    setRenderRepetitionLevel(level*level);

//    for(int nindent = 0; nindent < nstart-1; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , end" << std::endl;
//    nstart--;
}


///**
// * @brief RenderArea::setRenderSubPixelLevel, set render sub pixel level.
// * @param level
// */
//void RenderArea::setRenderSubPixelLevel(int level){
//#ifdef CPUMODE
//    m_renderer->setSubpixelLevel( level );
//#else // CPUMODE
//#ifdef DEBUG_COMPOSITOR
//    std::cerr << "RenderArea::setRenderSubPxelLevel(" << level << ") : set repetition level to compositor." << std::endl;
//#endif // DEBUG_COMPOSITOR
//    m_compositor->setRepetitionLevel(level);
//#endif // CPUMODE
//}

///**
// * @brief RenderArea::setRenderRepetionlLevel, set render repetion level
// *              Only has effect in CPUMODE, zero-op in GPUMODE
// * @param level, int.
// */
//void RenderArea::setRenderRepetionlLevel(int level){
//#ifndef CPUMODE
//    m_renderer.setRepetitionLevel( level );
//#endif
//}
///**
// * @brief RenderArea::recreateRenderImageBuffer, recreate the renderers image buffer.
// *              Only has effect in CPUMODE, zero-op in GPUMODE
// */
//void RenderArea::recreateRenderImageBuffer()
//{
//    // 20210818 yodo modified : If CPUMODE not defined, this fuction is no operations.
//    //#ifndef CPUMODE
//#ifdef CPUMODE
//    m_renderer->recreateImageBuffer();
//#endif
//}

/**
 * @brief RenderArea::getPointObjectXform, get point object transform from private active point object
 * @return
 */
kvs::Xform RenderArea::getPointObjectXform()
{
    return  m_point_object->xform();
}
/**
 * @brief RenderArea::setPointObjectXform, set point object transfor of private active point object.
 * @param xf
 */
void RenderArea::setPointObjectXform(kvs::Xform xf)
{
    m_point_object->setXform(xf);
}

/**
 * @brief RenderArea::updateCommandInfo
 * @param extCommand
 */
void RenderArea::updateCommandInfo(ExtCommand* extCommand)
{
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    //extCommand->m_parameter.m_camera = this->m_scene->camera();
    // mod by K.Yodo 2021.1.29.
    extCommand->m_parameter.m_camera = this->scene()->camera();
    // Setup Controll Object
    //    kvs::PointObject* object1 = extCommand->m_abstract_particles[0];
    kvs::PointObject* object1 = extCommand->m_detailed_particles[0];
    const kvs::Vector3f& min = object1->minObjectCoord();
    const kvs::Vector3f& max = object1->maxObjectCoord();
    m_point_object->setMinMaxObjectCoords( min, max );
    m_point_object->setMinMaxExternalCoords( min, max );

    // Setup Extended Particle Volume Renderer
     int sp_level= extCommand->m_parameter.m_detailed_subpixel_level;
     setRenderSubPixelLevel(sp_level);

#if 0
    // Setup Extended Particle Volume Renderer
#ifdef CPUMODE
    m_renderer = new kvs::visclient::ExtendedParticleVolumeRenderer( *object1, extCommand->m_parameter.m_detailed_subpixel_level, extCommand->m_parameter.m_detailed_repeat_level );
#else // CPUMODE

#ifdef USE_HARDCODE_MV_MATRIX
    // from KVS sample
    kvs::Mat4 mvmat (
                +1.935484e-01,  +0.000000e+00,  +0.000000e+00,  -3.000000e+00,
                +0.000000e+00,  +1.935484e-01,  +0.000000e+00,  -3.000000e+00,
                +0.000000e+00,  +0.000000e+00,  +1.935484e-01,  -1.500000e+01,
                +0.000000e+00,  +0.000000e+00,  +0.000000e+00,  +1.000000e+00
                );
    // from OPBVR test code
    kvs::Mat4 pmat (
                +1.200088e+00,  +0.000000e+00,  -1.470005e-01,  +0.000000e+00,
                +0.000000e+00,  +1.008082e+00,  -1.125376e-01,  +0.000000e+00,
                +0.000000e+00,  +0.000000e+00,  -1.000200e+00,  -2.000400e-01,
                +0.000000e+00,  +0.000000e+00,  -1.000000e+00,  +0.000000e+00
                );
    // from OPBVR test code
    kvs::Vec4 viewport (+0.000000e+00, +0.000000e+00, +1.328000e+03, +1.584000e+03);

    m_renderer = new kvs::glsl::ParticleBasedRenderer(mvmat, pmat, viewport);
#else // USE_HARDCODE_MV_MATRIX
    m_renderer = new kvs::glsl::ParticleBasedRenderer();
    //*object1, extCommand->m_parameter.m_detailed_subpixel_level, extCommand->m_parameter.m_detailed_repeat_level );
#endif // USE_HARDCODE_MV_MATRIX

#endif // CPUMODE
    if ( !m_renderer )
    {
        kvsMessageError( "Cannot create a point m_renderer." );
    }

    // 20210818 yodo add to get model/projection matrices & viewport vector.
    //int repeats = extCommand->m_parameter.m_detailed_repeat_level;
    int repeats = 100;
    m_dummy_object = new kvs::PointObject();
    m_dummy_object->setName( "dummy" );
    m_dummy_object->setCoords( kvs::ValueArray<kvs::Real32>::Random( repeats * 3 ) );
    m_dummy_object->setColors( kvs::ValueArray<kvs::UInt8>::Random( repeats * 3 ) );
    m_dummy_object->setNormals( kvs::ValueArray<kvs::Real32>::Random( repeats * 3 ) );
    m_dummy_object->setSize( 1.0f );
    m_dummy_object->hide();

    m_renderer->setRepetitionLevel(repeats);
    m_obj_id_pair = this->m_scene->registerObject(m_dummy_object,m_renderer);
#endif // 0


    // Setup FPS Label
    m_fpsLabel  =new QGlue::FPSLabel(this,*(m_renderer.pbr_pointer()));
//    m_fpsLabel  =new QGlue::FPSLabel(this,*m_renderer);
    m_fpsLabel->setPosition(50*pixelRatio,50);
    m_labels.append(m_fpsLabel);

    m_renderer.setShadingString(shadinglevel);
//    this->setShaderParams();
    Screen::update();
    this->setFocus();
}

/**
 * @brief RenderArea::onInitializeGL, overrides virtual onInitializeGL of PBVR Screen class.
 *                     PBVR Screen will call onInitializeGL when initializeGL is successful.
 */
void RenderArea::onInitializeGL( void )
{
    qInfo("KVSRenderArea::initializeGL( void )");

#ifdef SHOW_ORIENTATION_AXIS
    m_orientation_axis->initializeOpenGLFunctions();
#endif // SHOW_ORIENTATION_AXIS

    g_legend->initializeOpenGLFunctions();
    this->setAutoFillBackground(false);

    m_scene->light()->on();
    m_scene->mouse()->attachCamera(m_scene->camera());

    // 20220805
    m_point_object.swap();
    m_point_object->setName(OBJ_NAME_PARTICLE);
    m_point_object.swap();
    m_point_object->setName(OBJ_NAME_PARTICLE);
}
/**
 * @brief RenderArea::onResizeGL, GL Surface resized handler overrides PBVR Screen onResizeGL.
 *                               PBVR Screen will call onResizeGL at the end of Screen::resizeGL
 * @param w int width
 * @param h int height
 */
void RenderArea::onResizeGL(int w, int h)
{
    //    MOD BY)T.Osaki 2020.04.28
    float scale= QPaintDevice::devicePixelRatioF();
    int h_scaled = h * scale;
    int w_scaled = w  * scale;

    m_stepLabel->setPosition(150*scale,50);
    if (m_fpsLabel)
        m_fpsLabel->setPosition(50*scale,50);
#ifdef SHOW_ORIENTATION_AXIS
    int size=90*scale;
    m_orientation_axis->setPosition( w_scaled -(size + 10),10);
#endif // SHOW_ORIENTATION_AXIS
    g_legend->setPosition( 10, h_scaled -10 );

}
/**
 * @brief RenderArea::onPaintGL GL paint handler overrides PBVR Screen onPaintGL.
 *                               PBVR Screen will call onResizeGL at the end of Screen::paintGL
 */
void RenderArea::onPaintGL(void)
{
#ifdef SHOW_ORIENTATION_AXIS
    if (m_orientation_axis){
        glPushMatrix();
        m_orientation_axis->paintEvent();
        glPopMatrix();
    }
    this->drawLabelList(m_labels,QColor(0,0,0,255));
#endif // SHOW_ORIENTATION_AXIS

    if(g_legend){
        glPushMatrix();
        g_legend->paintEvent();
        glPopMatrix();
    }
}

/**
 * @brief RenderArea::setupEventHandlers,
 */
void RenderArea::setupEventHandlers()
{

    std::string interval( "TIMER_EVENT_INTERVAL" );

    if ( getenv( interval.c_str() ) != NULL )
    {
        msec = std::atoi( getenv( interval.c_str() ) );
    }
    else {
        msec = DEFAULT_MSEC;
    }

    // Timer

    qt_timer = new QGlue::Timer( msec ,extCommand);
    //    qt_timer->setRenderer(this->m_renderer);

    extCommand->m_glut_timer = qt_timer;
    //    qt_timer->setScreen(this);

    g_timer_event=new kvs::visclient::TimerEvent( extCommand,&extCommand->comthread );
    g_timer_event->setScreen( this );
    qt_timer->setEventListener( g_timer_event );
    qt_timer->start();
}
/**
 * @brief RenderArea::attachPointObject
 *        Attaches a new point to the scene, using the apropriate registerObject, or
 *        replaceObject, depending on wether a prevous object has been registered.
 *
 *        Ensures context current by explict makeCurrent() call.
 *
 *        Creates local copy of point object, to ensure life time.
 * @param point, the point to be attached
 */
void RenderArea::attachPointObject(const kvs::PointObject* point, int sp_level)
{
//    for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , start" << std::endl;
//    nstart++;

    if (QThread::currentThread() != this->thread()) {
        qWarning("RenderArea::attachPointObject was not called from main thread, ignoring point object");
        return;
    }
//    if (point->numberOfVertices() < 1){
//        // PointObject with no point will crush the process at renderer engine creation.
//        // So the point must be ignored.
//        return;
//    }
    if (point == NULL || point->coords().size() == 0){
        qCritical("RenderArea::attachPointObject. !!!!!!! NULL OR ZERO OBJECT - INTERPRETED AS CLEAR !!!!!! t\n" );
        m_scene->removeObject(m_obj_id_pair.first,false,false);
        m_obj_id_pair.first=-1;
        return;
    }
    if (point->coords().size() <= 3){
        qCritical("RenderArea::attachPointObject.  PointObject with single point attached t\n" );

    }

//    {
//        kvs::oculus::jaea::PBVRObjectManager* objectManager = m_scene->objectManager();
//        kvs::Vec3 minExtCrd = objectManager->minExternalCoord();
//        kvs::Vec3 maxExtCrd = objectManager->maxExternalCoord();
//        kvs::Vec3 minObjCrd = objectManager->minObjectCoord();
//        kvs::Vec3 maxObjCrd = objectManager->maxObjectCoord();
//        kvs::Mat4 m = objectManager->modelingMatrix();

//        std::cerr << "RenderArea::attachPointObject() : current status of object manager:" << std::endl;
//        std::cerr << "minExtCrd = " << minExtCrd << std::endl;
//        std::cerr << "maxExtCrd = " << maxExtCrd << std::endl;
//        std::cerr << "minObjCrd = " << minObjCrd << std::endl;
//        std::cerr << "maxObjCrd = " << maxObjCrd << std::endl;
//        std::cerr << "m=" << std::endl;
//        std::cerr << m << std::endl;
//    }

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : before sawp : m=" << std::endl;
//        std::cerr << m << std::endl;
//    }


    m_point_object.swap();

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after sawp : " << std::endl;
//        std::cerr << "  m_point_object.m = " << std::endl;
//        std::cerr << m << std::endl;
//        std::cerr << "  m_point_object.hasObjCrds = " << m_point_object->hasMinMaxObjectCoords() << std::endl;
//        std::cerr << "  m_point_object.objCrds = " << m_point_object->minObjectCoord() << " ... " << m_point_object->maxObjectCoord() << std::endl;
//        std::cerr << "  m_point_object.extCrds = " << m_point_object->minExternalCoord() << " ... " << m_point_object->maxExternalCoord() << std::endl;
//    }


    m_point_object->clear();

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after clear : " << std::endl;
//        std::cerr << "m_point_object.m=" << std::endl;
//        std::cerr << m << std::endl;
//        std::cerr << "  m_point_object.hasObjCrds = " << m_point_object->hasMinMaxObjectCoords() << std::endl;
//        std::cerr << "  m_point_object.objCrds = " << m_point_object->minObjectCoord() << " ... " << m_point_object->maxObjectCoord() << std::endl;
//        std::cerr << "  m_point_object.extCrds = " << m_point_object->minExternalCoord() << " ... " << m_point_object->maxExternalCoord() << std::endl;
//        std::cerr << "  point->hasObjCrds = " << point->hasMinMaxObjectCoords() << std::endl;
//        std::cerr << "  point->objCrds = " << point->minObjectCoord() << " ... " << point->maxObjectCoord() << std::endl;
//        std::cerr << "  point->extCrds = " << point->minExternalCoord() << " ... " << point->maxExternalCoord() << std::endl;
//    }


    m_point_object->add(*point);

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after add : m=" << std::endl;
//        std::cerr << m << std::endl;
//        std::cerr << "  objCrds = " << m_point_object->minObjectCoord() << " ... " << m_point_object->maxObjectCoord() << std::endl;
//        std::cerr << "  extCrds = " << m_point_object->minExternalCoord() << " ... " << m_point_object->maxExternalCoord() << std::endl;
//    }


    // Make sure context is current
    makeCurrent();

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after makeCurrent() : m=" << std::endl;
//        std::cerr << m << std::endl;
//    }

    // Update subpixel/renderrepetitoion level
    this->setRenderRepetitionLevel(sp_level * sp_level);
//        this->setRenderRepetitionLevel(sp_level * sp_level * sp_level * sp_level);

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after setRenderRepetitionLevel() : m=" << std::endl;
//        std::cerr << m << std::endl;
//    }

    if (m_obj_id_pair.first ==-1 && m_obj_id_pair.second == -1){
        for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
        std::cout << "m_obj_id_pair=this->m_scene->registerObject(m_point_object,m_renderer.pbr_pointer())" << std::endl;

        std::cerr << "m_point_object has set coords = " << m_point_object->hasMinMaxObjectCoords() << std::endl;

        m_obj_id_pair=this->m_scene->registerObject(m_point_object,m_renderer.pbr_pointer());


//        {
//            float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//            kvs::Mat4 m (tmp_m);
//            std::cerr << "RenderArea::attachPointObject() : after register(1) : m=" << std::endl;
//            std::cerr << m << std::endl;
//        }



    }
    else if (m_obj_id_pair.first ==-1){
//        for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//        std::cout << "m_obj_id_pair=this->m_scene->registerObject(m_point_object,m_renderer.pbr_pointer())" << std::endl;

        m_obj_id_pair = m_scene->registerObject(m_point_object,m_renderer.pbr_pointer());

//        {
//            float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//            kvs::Mat4 m (tmp_m);
//            std::cerr << "RenderArea::attachPointObject() : after register(2) : m=" << std::endl;
//            std::cerr << m << std::endl;
//        }

    }
    else {
//        for(int nindent = 0; nindent < nstart; nindent++) std::cout << "    ";
//        std::cout << "this->m_scene->replaceObject(m_obj_id_pair.first,m_point_object,false)" << std::endl;

        this->m_scene->replaceObject(m_obj_id_pair.first,m_point_object,false);

//        {
//            float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//            kvs::Mat4 m (tmp_m);
//            std::cerr << "RenderArea::attachPointObject() : after replace() : m=" << std::endl;
//            std::cerr << m << std::endl;
//        }

    }

    // 20220912 : moved here -> faild (no error, but no rendering)
//    // Update subpixel/renderrepetitoion level
//    this->setRenderRepetitionLevel(sp_level * sp_level);

#ifdef SHOW_ORIENTATION_AXIS
    m_orientation_axis->setObject( m_point_object);
#endif // SHOW_ORIENTATION_AXIS

    // Let Qt know we are done using the context.
    doneCurrent();

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after doneCurrent() : m=" << std::endl;
//        std::cerr << m << std::endl;
//    }


    // Trigger a paint event
    this->update();

//    {
//        float tmp_m[16]; m_point_object->xform().toArray( tmp_m );
//        kvs::Mat4 m (tmp_m);
//        std::cerr << "RenderArea::attachPointObject() : after update()) : m=" << std::endl;
//        std::cerr << m << std::endl;
//    }



//    for(int nindent = 0; nindent < nstart-1; nindent++) std::cout << "    ";
//    std::cout << __FILE__ << ", Line:" << __LINE__ << ", " << __func__ << " , end" << std::endl;
//    nstart--;
}

///**
// * @brief RenderArea::attachPointObject
// *        Attaches a new point to the scene, using the apropriate registerObject, or
// *        replaceObject, depending on wether a prevous object has been registered.
// *
// *        Ensures context current by explict makeCurrent() call.
// *
// *        Creates local copy of point object, to ensure life time.
// * @param point, the point to be attached
// */
//void RenderArea::attachPointObject(const kvs::PointObject* point)
//{

//    //    std::cout<<"attachPointObject"<<std::endl;
//    if (QThread::currentThread() != this->thread()) {
//        qWarning("RenderArea::attachPointObject was not called from main thread, ignoring point object");
//        return;
//    }
//    if (point->coords().size() <= 3){
//        qCritical("RenderArea::attachPointObject.  PointObject with single point attached t\n" );

//    }
//    m_point_object.swap();
//    m_point_object->clear();
//    m_point_object->add(*point);
//    // Make sure context is current
//    makeCurrent();
//    if (m_obj_id_pair.first ==-1 && m_obj_id_pair.second == -1){
//        m_obj_id_pair=this->m_scene->registerObject(m_point_object,m_renderer);
//    }
//    else {
//        this->m_scene->replaceObject(m_obj_id_pair.first,m_point_object,false);
//    }
//#ifdef SHOW_ORIENTATION_AXIS
//    m_orientation_axis->setObject( m_point_object);
//#endif // SHOW_ORIENTATION_AXIS
//    // Let Qt know we are done using the context.
//    doneCurrent();
//    // Trigger a paint event
//    this->update();
//    //    std::cout<<"attachPointObject end"<<std::endl;
//}

/**
 * @brief RenderArea::setCoordinateBoundaries
 * @param crd
 */
void RenderArea::setCoordinateBoundaries(float  crd[6])
{
    kvs::Vector3f min_t( crd[0], crd[1], crd[2] );
    kvs::Vector3f max_t( crd[3], crd[4], crd[5] );
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.07.20
    //    this->m_scene->objectManager()->object()->setMinMaxObjectCoords( min_t, max_t );
    //    this->m_scene->objectManager()->object()->setMinMaxExternalCoords( min_t, max_t );
    m_point_object->setMinMaxObjectCoords(min_t,max_t);
    m_point_object->setMinMaxExternalCoords(min_t,max_t);

    this->m_scene->objectManager()->updateExternalCoords();
//    if(m_reset_count == 0){
//        m_reset_count++;
//    }else{
//        this->m_scene->reset();
//    }
    std::cout << " !!!!!!!!!!!!!!!!!!! Reset Viewer Scale !!!!!!!!!!!!!!!!!!!!!!!! " << std::endl;
}

/**
 * @brief RenderArea::drawLabelList, draws the label list
 * @param list
 * @param fontColor
 */
void RenderArea::drawLabelList( QList<QGlue::Label*> list,QColor fontColor)
{
    const float front = 0.0;
    const float back = 1.0;
    const float left = 0.0f;
    const float bottom = 0.0f;
    const float right = 300.0f;
    const float top = 300.0f;

    glPushMatrix();
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glViewport( 0, 0, 300, 300);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( left, right, bottom, top, front, back );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    for (int i = 0; i < list.size(); ++i) {
        QGlue::Label* item= list.at(i);
        item->contentUpdate();
        item->renderBitMap(item->x(),item->y(),0.0);
    }
    glPopAttrib();
    glPopMatrix();
}

/**
 * @brief RenderArea::setLabelFont
 * @param f
 */
void RenderArea::setLabelFont(const QFont& f)
{
    for (int i = 0; i < m_labels.size(); ++i) {
        QGlue::Label* item= m_labels.at(i);
        item->setFont(f);
    }
    g_legend->setFont(f);

}

///**
// * @brief RenderArea::setShaderParams
// */
//void RenderArea::setShaderParams( )
//{
//    // For shading.
//    m_renderer->disableShading();
//    // APPEND START BY)M.Tanaka 2015.03.11
//    float sd_ka, sd_kd, sd_ks, sd_n;
//    if ( strcmp( shadinglevel, "L" ) == 0 )
//    {
//        printf( "***** shading : L\n" );
//        m_renderer->enableShading();
//        m_renderer->setShader( kvs::Shader::Lambert() );
//    }
//    else if ( strcmp( shadinglevel, "P" ) == 0 )
//    {
//        printf( "***** shading : P\n" );
//        m_renderer->enableShading();
//        m_renderer->setShader( kvs::Shader::Phong() );
//    }
//    else if ( strcmp( shadinglevel, "B" ) == 0 )
//    {
//        printf( "***** shading : B\n" );
//        m_renderer->enableShading();
//        m_renderer->setShader( kvs::Shader::BlinnPhong() );
//    }
//    else if ( strncmp( shadinglevel, "L,", 2 ) == 0 )
//    {
//        m_renderer->enableShading();
//        sscanf( &shadinglevel[2], "%f,%f", &sd_ka, &sd_kd );
//        printf( "***** shading : L %f %f\n", sd_ka, sd_kd );
//        m_renderer->setShader( kvs::Shader::Lambert( sd_ka, sd_kd ) );
//    }
//    else if ( strncmp( shadinglevel, "P,", 2 ) == 0 )
//    {
//        m_renderer->enableShading();
//        sscanf( &shadinglevel[2], "%f,%f,%f,%f", &sd_ka, &sd_kd, &sd_ks, &sd_n );
//        printf( "***** shading : P %f %f %f %f\n", sd_ka, sd_kd, sd_ks, sd_n );
//        m_renderer->setShader( kvs::Shader::Phong( sd_ka, sd_kd, sd_ks, sd_n ) );
//    }
//    else if ( strncmp( shadinglevel, "B,", 2 ) == 0 )
//    {
//        m_renderer->enableShading();
//        sscanf( &shadinglevel[2], "%f,%f,%f,%f", &sd_ka, &sd_kd, &sd_ks, &sd_n );
//        printf( "***** shading : B %f %f %f %f\n", sd_ka, sd_kd, sd_ks, sd_n );
//        m_renderer->setShader( kvs::Shader::BlinnPhong( sd_ka, sd_kd, sd_ks, sd_n ) );
//    }
//#ifndef CPUMODE
//    this->m_renderer->disableLODControl();
//#endif
//}

/**
 * @brief RenderArea::keyPressEvent
 * @param kbEvent
 */
void RenderArea::keyPressEvent(QKeyEvent *kbEvent){

    // unicode() returns const QChar*. QChar::unicode returns ushort
    ushort ucode=kbEvent->text().unicode()->unicode();
    qInfo()<<"@@@ KVSRenderArea::keyPressEvent :"<<ucode;

    switch (ucode)
    {
    case kvs::Key::l:
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        m_scene->controlTarget() = m_scene->TargetLight;
        break;
    case kvs::Key::c:
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        m_scene->controlTarget() = m_scene->TargetCamera;
        break;
    case kvs::Key::o:
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        m_scene->controlTarget() = m_scene->TargetObject;
        break;

    case kvs::Key::x:
        qInfo(" [debug] 'x' pressed. (add Xform)");
        KeyFrameAnimationAdd(this->getPointObjectXform());
        break;
    case kvs::Key::d:
        qInfo(" [debug] 'd' pressed. (delete last Xform)");
        KeyFrameAnimationErase();
        break;
    case kvs::Key::D:
        qInfo(" [debug] 'D' pressed. (delete all Xform)");
        KeyFrameAnimationDelete();
        break;
    case kvs::Key::M:
        qInfo(" [debug] 'M' pressed. (toggle animation with Xform)");
        /* KeyFrameAnimationStart(); */
        KeyFrameAnimationToggle();
        break;
        /* case kvs::Key::m: */
        /*    printf(" [debug] 'm' pressed. (stop animation with Xform)"); */
        /*    KeyFrameAnimationStop(); */
        /*    break; */
    case kvs::Key::S:
        qInfo(" [debug] 'S' pressed. (write Xform to file)");
        KeyFrameAnimationWrite();
        break;
    case kvs::Key::F:
        qInfo(" [debug] 'F' pressed. (read Xform from file)");
        KeyFrameAnimationRead();
        break;
    default:
        break;
    }
    if(kbEvent->key()==Qt::Key_Home){
        qInfo(" [debug] 'HOME' pressed. (Reset the viewer)");
#if 0
        //MOD BY)T.Osaki 2020.06.29
        m_scene->reset();
#else
        // 20210902 yodo
        kvs::oculus::jaea::Scene* my_scene = static_cast<kvs::oculus::jaea::Scene*>(m_scene);
        my_scene->resetObjects();
#endif
        //kvs::ScreenBase::reset();
        this->update();
    }
}


/**
 * @brief RenderArea::ScreenShot
 * @param screen
 * @param tstep
 */
//MOD BY)T.Osaki 2020.06.29
//void    RenderArea::ScreenShot( kvs::ScreenBase* screen, const int tstep )
void    RenderArea::ScreenShot( kvs::oculus::jaea::Scene* scene, const int tstep )
{

    std::stringstream step;
    AnimationControlParameter ac_params= AnimationControls::getParameters();
#ifndef _TIMER_
    if ( ( ac_params.m_capture == PBVR_ON ) &&
         ( ac_params.m_mode == PBVR_IMAGE ) )
    {
#endif
        PBVR_TIMER_STA( 160 );

        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        scene->screen()->redraw();

        step << '.' << std::setw( 5 ) << std::setfill( '0' ) << tstep;

#ifdef  PBVR_DEBUG
        std::string filename = AnimationControls::getImageFileName(step.str(), ".bmp");
#else
        std::string filename = ac_params.m_image_file + step.str() + ".bmp";
#endif

        //        kvs::ColorImage image( screen->camera()->snapshot() );
        //        image.write( filename.c_str() );
        QImage image= extCommand->m_screen->grabFramebuffer();
        image.save(QString(filename.c_str()));

#ifdef  PBVR_DEBUG
        std::cout << "Snapshot >>" << filename << std::endl;
        timestep++;
        if ( timestep > 10 )
            timestep = 0;
#endif
        PBVR_TIMER_END( 160 );
#ifndef _TIMER_
    }
#endif
}

/**
 * @brief RenderArea::ScreenShotKeyFrame
 * @param screen
 * @param tstep
 */
//MOD BY)T.Osaki 2020.06.29
//void    RenderArea::ScreenShotKeyFrame( kvs::ScreenBase* screen, const int tstep )
void    RenderArea::ScreenShotKeyFrame( kvs::oculus::jaea::Scene* scene, const int tstep )
{

    std::stringstream step;
    AnimationControlParameter ac_params= AnimationControls::getParameters();
#ifndef _TIMER_
    if ( ( ac_params.m_capture == PBVR_ON ) &&
         ( ac_params.m_mode == PBVR_IMAGE ) )
    {
#endif
        PBVR_TIMER_STA( 160 );

        //KVS2.7.0
        //MOD BY)T.Osaki 2020.07.20
        scene->screen()->redraw();

        step << '.' << std::setw( 6 ) << std::setfill( '0' ) << tstep;

#ifdef  PBVR_DEBUG
        std::string filename =AnimationControls::getImageFileName( "_k" + step.str() , ".bmp");
#else
        std::string filename = ac_params.m_image_file + "_k" + step.str() + ".bmp";
#endif

        QImage image= extCommand->m_screen->grabFramebuffer();
        image.save(QString(filename.c_str()));

        PBVR_TIMER_END( 160 );
#ifndef _TIMER_
    }
#endif
}

/**
* @brief RenderArea::mousePressEvent
* @param event
*/
void RenderArea::mousePressEvent( QMouseEvent *event)
{

    //    ADD BY)T.Osaki 2020.03.03
    QWidget::setFocus();
    bool MOD_Shift = event->modifiers() & Qt::SHIFT;
    bool MOD_Ctrl  = event->modifiers() & Qt::CTRL;

    bool BTN_LEFT  = event->buttons()   & Qt::LeftButton;
    bool BTN_RIGHT = event->buttons()   & Qt::RightButton;
    bool BTN_MID = event->buttons() & Qt::MiddleButton;

    int mode =-1;

    if    (   (BTN_LEFT && MOD_Ctrl ) || BTN_RIGHT){
        mode=kvs::Mouse::Translation;
    }
    else if ( (BTN_LEFT && MOD_Shift) || BTN_MID  ){
        mode= kvs::Mouse::Scaling ;
    }
    else if ( BTN_LEFT ){
        mode = kvs::Mouse::Rotation;
        //KVS2.7.0
        //MOD BY)T.Osaki 2020.06.04
        makeCurrent();
        m_scene->updateCenterOfRotation();
        doneCurrent();
        //kvs::ScreenBase::updateCenterOfRotation();
    }
    const int x = event->x()*pixelRatio;
    const int y = event->y()*pixelRatio;
    if (mode >=0){

        //KVS2.7.0
        //MOD BY)T.Osaki 2020.06.04
        kvs::Mouse::OperationMode kvsMode=(kvs::Mouse::OperationMode)mode;
        m_scene->mousePressFunction(x,y,kvsMode);
        //kvs::Mouse::TransMode kvsMode=(kvs::Mouse::TransMode)mode;
        //kvs::ScreenBase::mousePressFunction(x,y,kvsMode );
    }
    std::cout<<"mousePressEvent::"<<mode<<" " << x<<","<<y<<std::endl;
    update();
}

/**
 * @brief RenderArea::mouseMoveEvent
 * @param event
 */
void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    //    std::cout<<"mouseMoveEvent"<<std::endl;
    //    BaseClass::eventHandler()->notify( event );
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    //if( kvs::ScreenBase::controlTarget() == kvs::ScreenBase::TargetObject )
    // 2021.1.29. K.Yodo modified
    //if( m_scene->controlTarget() == m_scene->TargetObject );
    if( m_scene->controlTarget() == m_scene->TargetObject )
    {
        //if( !kvs::ScreenBase::objectManager()->isEnableAllMove() )
        if( !m_scene->isEnabledObjectOperation())
        {
            //if( !kvs::ScreenBase::objectManager()->hasActiveObject() )
            if( !m_scene->objectManager()->hasActiveObject() )
            {
                return;
            }

        }
    }

    //const int x = event->x()*pixelRatio;
    //const int y = event->y()*pixelRatio;
    int x = event->x()*pixelRatio;
    int y = event->y()*pixelRatio;
    if(m_scene->mouse()->operationMode() == kvs::Mouse::Scaling){
        x *= 10;
        y *= 10;
    }
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    m_scene->mouseMoveFunction(x,y);
    //kvs::ScreenBase::mouseMoveFunction(x,y );
    update();
}

/**
 * @brief RenderArea::mouseReleaseEvent
 * @param event
 */
void RenderArea::mouseReleaseEvent(QMouseEvent *event)
{
    std::cout<<"mouseReleaseEvent"<<std::endl;
    //    BaseClass::eventHandler()->notify( event );
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
    m_scene->mouseReleaseFunction(event->x(),event->y());
    //kvs::ScreenBase::mouseReleaseFunction( event->x(), event->y() );
}

/**
 * @brief RenderArea::setSize
 * @param width
 * @param height
 */
void RenderArea::setSize( const int width, const int height )
{
    int h_scaled = height * pixelRatio;
    int w_scaled = width  * pixelRatio;

    kvs::ScreenBase::setSize( w_scaled, h_scaled);
    //KVS2.7.0
    //MOD BY)T.Osaki 2020.06.04
//    if ( m_scene->camera() ) m_scene->camera()->setWindowSize( w_scaled, h_scaled);
//    if ( m_scene->mouse() ) m_scene->camera()->setWindowSize( w_scaled, h_scaled );
    // mod by K.Yodo 2021.01.29.
        if ( scene()->camera() ) scene()->camera()->setWindowSize( w_scaled, h_scaled);
        if ( scene()->mouse() ) scene()->camera()->setWindowSize( w_scaled, h_scaled );
    //if ( kvs::ScreenBase::camera() ) kvs::ScreenBase::camera()->setWindowSize( w_scaled, h_scaled);
    //if ( kvs::ScreenBase::mouse()  ) kvs::ScreenBase::mouse()->setWindowSize( w_scaled, h_scaled);
    QOpenGLWidget::resize( w_scaled, h_scaled);
    this->update();
}

/**
 * @brief RenderArea::setGeometry
 * @param geom
 */
void RenderArea::setGeometry( QRect geom )
{
    QOpenGLWidget::setGeometry(geom);
    this->setMinimumSize(geom.width(), geom.height());
    this->setMaximumSize(geom.width(), geom.height());
    this->setSize( geom.width(), geom.height() );
    this->update();
}
