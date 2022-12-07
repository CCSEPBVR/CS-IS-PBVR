/*****************************************************************************/
/**
 *  @file   ParticleVolumeRenderer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ParticleVolumeRenderer.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ParticleVolumeRenderer.h"
#include <kvs/DebugNew>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/File>
#include <kvs/PipelineModule>
#include <kvs/VisualizationPipeline>
#include <kvs/CellByCellMetropolisSampling>
#include <kvs/CellByCellUniformSampling>
#include <kvs/CellByCellRejectionSampling>
#include <kvs/CellByCellLayeredSampling>
#include <kvs/ParticleVolumeRenderer>
#include <kvs/Bounds>
#include <kvs/PaintEventListener>
#include <kvs/MouseDoubleClickEventListener>
#include <kvs/KeyPressEventListener>
#include <kvs/KeyPressEventListener>
#include <kvs/Key>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/glut/TransferFunctionEditor>
#if defined( KVS_SUPPORT_GLEW )
#include <kvs/glew/ParticleVolumeRenderer>
#endif
#include "CommandName.h"
#include "ObjectInformation.h"
#include "FileChecker.h"
#include "Widget.h"


namespace { bool Shown = false; }
namespace { const std::string ObjectName( "ParticleObject" ); }
namespace { const std::string RendererName( "ParticleRenderer" ); }


namespace
{

/*===========================================================================*/
/**
 *  @brief  Returns revised subpixel level.
 *  @param  subpixel_level [in] subpixel level
 *  @param  repetition_level [in] repetition level
 *  @return revised subpixel level
 */
/*===========================================================================*/
inline const size_t GetRevisedSubpixelLevel(
    const size_t subpixel_level,
    const size_t repetition_level )
{
    return( static_cast<size_t>( subpixel_level * sqrtf( static_cast<float>(repetition_level) ) + 0.5f ) );
}

} // end of namespace


namespace kvsview
{

namespace ParticleVolumeRenderer
{

/*===========================================================================*/
/**
 *  @brief  Set up particle volume renderer.
 *  @param  arg [in] command line argument
 *  @param  tfunc [in] transfer function
 *  @param  renderer [in] renderer module
 */
/*===========================================================================*/
template <typename Renderer>
const void SetupRenderer(
    const kvsview::ParticleVolumeRenderer::Argument& arg,
    const kvs::TransferFunction& tfunc,
    Renderer* renderer )
{
    // Renderer name.
    renderer->setName( ::RendererName );

    // Transfer function.
    renderer->setTransferFunction( tfunc );

    // Shading on/off.
    const bool noshading = arg.noShading();
    if ( noshading ) renderer->disableShading();
    else renderer->enableShading();

    // Shader type.
    const float ka = arg.ambient();
    const float kd = arg.diffuse();
    const float ks = arg.specular();
    const float n = arg.shininess();
    const int shader = arg.shader();
    switch ( shader )
    {
    case 0: // Lambert shading
    {
        renderer->setShader( kvs::Shader::Lambert( ka, kd ) );
        break;
    }
    case 1: // Phong shading
    {
        renderer->setShader( kvs::Shader::Phong( ka, kd, ks, n ) );
        break;
    }
    case 2: // Blinn-phong shading
    {
        renderer->setShader( kvs::Shader::BlinnPhong( ka, kd, ks, n ) );
        break;
    }
    default: break;
    }
}

/*===========================================================================*/
/**
 *  @brief  Set up cell-by-cell particle sampler.
 *  @param  arg [in] command line argument
 *  @param  tfunc [in] transfer function
 *  @param  screen [in] screen
 *  @param  mapper [in] mapper module
 */
/*===========================================================================*/
template <typename Mapper>
const void SetupMapper(
    const kvsview::ParticleVolumeRenderer::Argument& arg,
    const kvs::TransferFunction& tfunc,
    /* const */ kvs::ScreenBase& screen,
    Mapper* mapper )
{
    const size_t subpixel_level = arg.subpixelLevel();
    const size_t repetition_level = arg.repetitionLevel();
    const size_t level = ::GetRevisedSubpixelLevel( subpixel_level, repetition_level );
    const float step = 0.5f;
    const float depth = 0.0f;
    mapper->attachCamera( screen.camera() );
    mapper->setTransferFunction( tfunc );
    mapper->setSubpixelLevel( level );
    mapper->setSamplingStep( step );
    mapper->setObjectDepth( depth );
}

/*===========================================================================*/
/**
 *  @brief  Transfer function editor class.
 */
/*===========================================================================*/
class TransferFunctionEditor : public kvs::glut::TransferFunctionEditor
{
    kvsview::ParticleVolumeRenderer::Argument* m_arg; ///< pointer to the argument
    kvs::glut::LegendBar* m_legend_bar; ///< pointer to the legend bar
    const kvs::VolumeObjectBase* m_volume; ///< pointer to the volume object

public:

    TransferFunctionEditor( kvs::glut::Screen* screen, kvsview::ParticleVolumeRenderer::Argument* arg ):
        kvs::glut::TransferFunctionEditor( screen ),
        m_arg( arg ),
        m_legend_bar( NULL ),
        m_volume( NULL ) {}

    void apply( void )
    {
        // Erase the object and renderer.
        const kvs::ObjectBase* obj = screen()->objectManager()->object( ::ObjectName );
        const kvs::Xform xform = obj->xform();
        const int obj_id = screen()->objectManager()->objectID( obj );
        screen()->IDManager()->eraseByObjectID( obj_id );
        screen()->objectManager()->erase( ::ObjectName );
        screen()->rendererManager()->erase( ::RendererName );

        // Current transfer function.
        kvs::TransferFunction tfunc( transferFunction() );

        // Set mapper (CellByCellXXXSampling).
        kvs::PointObject* object = NULL;
        switch ( m_arg->sampling() )
        {
        case 1: // Metropolis sampling
        {
            kvs::CellByCellMetropolisSampling* mapper = new kvs::CellByCellMetropolisSampling();
            kvsview::ParticleVolumeRenderer::SetupMapper( *m_arg, tfunc, *screen(), mapper );
            object = mapper->exec( m_volume );
            object->setName( ::ObjectName );
            object->setXform( xform );
            break;
        }
        case 2: // Rejection sampling
        {
            kvs::CellByCellRejectionSampling* mapper = new kvs::CellByCellRejectionSampling();
            kvsview::ParticleVolumeRenderer::SetupMapper( *m_arg, tfunc, *screen(), mapper );
            object = mapper->exec( m_volume );
            object->setName( ::ObjectName );
            object->setXform( xform );
            break;
        }
        case 3: // Layered sampling
        {
            kvs::CellByCellLayeredSampling* mapper = new kvs::CellByCellLayeredSampling();
            kvsview::ParticleVolumeRenderer::SetupMapper( *m_arg, tfunc, *screen(), mapper );
            object = mapper->exec( m_volume );
            object->setName( ::ObjectName );
            object->setXform( xform );
            break;
        }
        default: // Uniform sampling
        {
            kvs::CellByCellUniformSampling* mapper = new kvs::CellByCellUniformSampling();
            kvsview::ParticleVolumeRenderer::SetupMapper( *m_arg, tfunc, *screen(), mapper );
            object = mapper->exec( m_volume );
            object->setName( ::ObjectName );
            object->setXform( xform );
            break;
        }
        }

        // Create new particle volume renderer.
        if ( m_arg->noGPU() )
        {
            kvs::ParticleVolumeRenderer* renderer = new kvs::ParticleVolumeRenderer();
            kvsview::ParticleVolumeRenderer::SetupRenderer( *m_arg, tfunc, renderer );

            // Subpixel level.
            const size_t subpixel_level = m_arg->subpixelLevel();
            const size_t repetition_level = m_arg->repetitionLevel();
            const size_t level = ::GetRevisedSubpixelLevel( subpixel_level, repetition_level );
            renderer->setSubpixelLevel( level );

            screen()->registerObject( object, renderer );
        }
#if defined( KVS_SUPPORT_GLEW )
        else
        {
            kvs::glew::ParticleVolumeRenderer* renderer = new kvs::glew::ParticleVolumeRenderer();
            kvsview::ParticleVolumeRenderer::SetupRenderer( *m_arg, tfunc, renderer );

            // Subpixel level and repetition level.
            const size_t subpixel_level = m_arg->subpixelLevel();
            const size_t repetition_level = m_arg->repetitionLevel();
            renderer->setSubpixelLevel( subpixel_level );
            renderer->setRepetitionLevel( repetition_level );

            if ( !m_arg->noLOD() ) renderer->enableLODControl();

            screen()->registerObject( object, renderer );
        }
#endif

        m_legend_bar->setColorMap( transferFunction().colorMap() );

        screen()->redraw();
    }

    void attachVolume( const kvs::VolumeObjectBase* volume )
    {
        m_volume = volume;
    }

    void attachLegendBar( kvs::glut::LegendBar* legend_bar )
    {
        m_legend_bar = legend_bar;
    }
};

/*===========================================================================*/
/**
 *  @brief  Key press event.
 */
/*===========================================================================*/
class KeyPressEvent : public kvs::KeyPressEventListener
{
    TransferFunctionEditor* m_editor; ///!< pointer to the transfer function

public:

    void update( kvs::KeyEvent* event )
    {
        switch ( event->key() )
        {
        case kvs::Key::o: screen()->controlTarget() = kvs::ScreenBase::TargetObject; break;
        case kvs::Key::l: screen()->controlTarget() = kvs::ScreenBase::TargetLight; break;
        case kvs::Key::c: screen()->controlTarget() = kvs::ScreenBase::TargetCamera; break;
        case kvs::Key::t:
        {
            if ( ::Shown ) m_editor->hide();
            else m_editor->showWindow();

            ::Shown = !::Shown;
            break;
        }
        default: break;
        }
    }

    void attachTransferFunctionEditor( TransferFunctionEditor* editor )
    {
        m_editor = editor;
    }
};

/*===========================================================================*/
/**
 *  @brief  Mouse double-click event.
 */
/*===========================================================================*/
class MouseDoubleClickEvent : public kvs::MouseDoubleClickEventListener
{
    TransferFunctionEditor* m_editor; ///!< pointer to the transfer function

public:

    void update( kvs::MouseEvent* event )
    {
        if ( ::Shown ) m_editor->hide();
        else m_editor->showWindow();

        ::Shown = !::Shown;
    }

    void attachTransferFunctionEditor( TransferFunctionEditor* editor )
    {
        m_editor = editor;
    }
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvsview::Argument::Common( argc, argv, "ParticleVolumeRenderer")
{
    // Parameters for the ParticleVolumeRenderer class.
    add_option( kvsview::ParticleVolumeRenderer::CommandName, kvsview::ParticleVolumeRenderer::Description, 0 );
    add_option( "s", "Subpixel level. (default: 1)", 1, false );
    add_option( "r", "Repetition level. (default: 1)", 1, false );
    add_option( "t", "Transfer function file. (optional: <filename>)", 1, false );
    add_option( "T", "Transfer function file with range adjustment. (optional: <filename>)", 1, false );
    add_option( "noshading", "Disable shading. (optional)", 0, false );
    add_option( "nolod", "Disable Level-of-Detail control. (optional)", 0, false );
    add_option( "nogpu", "Disable GPU rendering. (optional)", 0, false );
    add_option( "nozooming", "Disable particle resizing. (optional)", 0, false );
    add_option( "sampling", "Sampling type. (default: 0)\n"
                "\t      0 = Cell-by-cell uniform sampling\n"
                "\t      1 = Cell-by-cell metropolis sampling\n"
                "\t      2 = Cell-by-cell rejection sampling\n"
                "\t      3 = Cell-by-cell layered sampling", 1, false );
    add_option( "ka", "Coefficient of the ambient color. (default: lambert=0.4, phong=0.3)", 1, false );
    add_option( "kd", "Coefficient of the diffuse color. (default: lambert=0.6, phong=0.5)", 1, false );
    add_option( "ks", "Coefficient of the specular color. (default: 0.8)", 1, false );
    add_option( "n", "Shininess. (default: 100.0)", 1, false );
    add_option( "shader", "Shader type. (default: 0)\n"
                "\t      0 = Lambert shading\n"
                "\t      1 = Phong shading\n"
                "\t      2 = Blinn-Phong shading", 1, false );
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling method number.
 *  @return sampling method number
 */
/*===========================================================================*/
const int Argument::sampling( void ) const
{
    const int default_value = 0;

    if ( this->hasOption("sampling") ) return( this->optionValue<int>("sampling") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the shader number.
 *  @return shader number
 */
/*===========================================================================*/
const int Argument::shader( void ) const
{
    const int default_value = 0;

    if ( this->hasOption("shader") ) return( this->optionValue<int>("shader") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the "noshading" option is specified or not.
 *  @return true, if the "noshading" option is specified
 */
/*===========================================================================*/
const bool Argument::noShading( void ) const
{
    return( this->hasOption("noshading") );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the "nolod" option is specified or not.
 *  @return true, if the "nolod" option is specified
 */
/*===========================================================================*/
const bool Argument::noLOD( void ) const
{
    return( this->hasOption("nolod") );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the "nogpu" option is specified or not.
 *  @return true, if the "nogpu" option is specified
 */
/*===========================================================================*/
const bool Argument::noGPU( void ) const
{
#if defined( KVS_SUPPORT_GLEW )
    return( this->hasOption("nogpu") );
#else
    return( true );
#endif
}

const bool Argument::noZooming( void ) const
{
    return( this->hasOption("nozooming") );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coefficient for ambient color.
 *  @return coefficient for ambient color
 */
/*===========================================================================*/
const float Argument::ambient( void ) const
{
    const float default_value = this->shader() == 0 ? 0.4f : 0.3f;

    if ( this->hasOption("ka") ) return( this->optionValue<float>("ka") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coefficient for diffuse color.
 *  @return coefficient for diffuse color
 */
/*===========================================================================*/
const float Argument::diffuse( void ) const
{
    const float default_value = this->shader() == 0 ? 0.6f : 0.5f;

    if ( this->hasOption("kd") ) return( this->optionValue<float>("kd") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coefficient for specular color.
 *  @return coefficient for specular color
 */
/*===========================================================================*/
const float Argument::specular( void ) const
{
    const float default_value = 0.8f;

    if ( this->hasOption("ks") ) return( this->optionValue<float>("ks") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coefficient for shininess.
 *  @return coefficient for shininess
 */
/*===========================================================================*/
const float Argument::shininess( void ) const
{
    const float default_value = 100.0f;

    if ( this->hasOption("n") ) return( this->optionValue<float>("n") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the subpixle level.
 *  @return subpixel level
 */
/*===========================================================================*/
const size_t Argument::subpixelLevel( void ) const
{
    const size_t default_value = 1;

    if ( this->hasOption("s") ) return( this->optionValue<size_t>("s") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the repetition level.
 *  @return repetition level
 */
/*===========================================================================*/
const size_t Argument::repetitionLevel( void ) const
{
    const size_t default_value = 1;

    if ( this->hasOption("r") ) return( this->optionValue<size_t>("r") );
    else return( default_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns a transfer function.
 *  @param  volume [in] pointer to the volume object
 *  @return transfer function
 */
/*===========================================================================*/
const kvs::TransferFunction Argument::transferFunction( const kvs::VolumeObjectBase* volume ) const
{
    if ( this->hasOption("t") )
    {
        const std::string filename = this->optionValue<std::string>("t");
        return( kvs::TransferFunction( filename ) );
    }
    else if ( this->hasOption("T") )
    {
        const std::string filename = this->optionValue<std::string>("T");
        kvs::TransferFunction tfunc( filename );
        tfunc.adjustRange( volume );
        return( tfunc );
    }
    else
    {
        const size_t resolution = 256;
        return( kvs::TransferFunction( resolution ) );
    }
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Main class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Main::Main( int argc, char** argv )
{
    m_argc = argc;
    m_argv = argv;
}

/*===========================================================================*/
/**
 *  @brief  Executes main process.
 */
/*===========================================================================*/
const bool Main::exec( void )
{
    // GLUT viewer application.
    kvs::glut::Application app( m_argc, m_argv );

    // Parse specified arguments.
    kvsview::ParticleVolumeRenderer::Argument arg( m_argc, m_argv );
    if( !arg.parse() ) return( false );

    // Events.
    kvsview::ParticleVolumeRenderer::KeyPressEvent key_press_event;
    kvsview::ParticleVolumeRenderer::MouseDoubleClickEvent mouse_double_click_event;

    // Create screen.
    kvs::glut::Screen screen( &app );
    screen.setSize( 512, 512 );
    screen.addKeyPressEvent( &key_press_event );
    screen.addMouseDoubleClickEvent( &mouse_double_click_event );
    screen.setTitle( kvsview::CommandName + " - " + kvsview::ParticleVolumeRenderer::CommandName );

    // Check the input point or volume data.
    bool is_volume = false; // check flag whether the input data is volume data
    m_input_name = arg.value<std::string>();
    if ( !kvsview::FileChecker::ImportablePoint( m_input_name ) )
    {
        if ( !( kvsview::FileChecker::ImportableStructuredVolume( m_input_name ) ||
                kvsview::FileChecker::ImportableUnstructuredVolume( m_input_name ) ) )
        {
            kvsMessageError("%s is not volume data.", m_input_name.c_str());
            return( false );
        }

        is_volume = true;
    }

    // Visualization pipeline.
    kvs::VisualizationPipeline pipe( m_input_name );
    pipe.import();

    // Verbose information.
    if ( arg.verboseMode() )
    {
        std::cout << "IMPORTED OBJECT" << std::endl;
        std::cout << kvsview::ObjectInformation( pipe.object() ) << std::endl;
        std::cout << std::endl;
    }

    // Pointer to the volume object data.
    const kvs::VolumeObjectBase* volume = kvs::VolumeObjectBase::DownCast( pipe.object() );

    // Transfer function.
    const kvs::TransferFunction tfunc = arg.transferFunction( volume );

    // Label (fps).
    kvsview::Widget::FPSLabel label( &screen, ::RendererName );
    label.show();

    // Legend bar.
    kvsview::Widget::LegendBar legend_bar( &screen );
    legend_bar.setColorMap( tfunc.colorMap() );
    if ( !tfunc.hasRange() )
    {
        const kvs::Real32 min_value = static_cast<kvs::Real32>( volume->minValue() );
        const kvs::Real32 max_value = static_cast<kvs::Real32>( volume->maxValue() );
        legend_bar.setRange( min_value, max_value );
    }
    legend_bar.show();

    // Orientation axis.
    kvsview::Widget::OrientationAxis orientation_axis( &screen );
    orientation_axis.show();

    // For bounding box.
    if ( arg.hasOption("bounds") )
    {
        kvs::LineObject* bounds = new kvs::Bounds( pipe.object() );
        bounds->setColor( kvs::RGBColor( 0, 0, 0 ) );
        if ( arg.hasOption("bounds_color") )
        {
            const kvs::UInt8 r( static_cast<kvs::UInt8>(arg.optionValue<int>("bounds_color",0)) );
            const kvs::UInt8 g( static_cast<kvs::UInt8>(arg.optionValue<int>("bounds_color",1)) );
            const kvs::UInt8 b( static_cast<kvs::UInt8>(arg.optionValue<int>("bounds_color",2)) );
            bounds->setColor( kvs::RGBColor( r, g, b ) );
        }

        kvs::VisualizationPipeline p( bounds );
        p.exec();
        p.renderer()->disableShading();

        screen.registerObject( &p );
    }

    // Set a mapper (cell-by-cell sampling)
    switch ( arg.sampling() )
    {
    case 1: // Metropolis sampling
    {
        kvs::PipelineModule mapper( new kvs::CellByCellMetropolisSampling );
        kvs::CellByCellMetropolisSampling* pmapper = mapper.get<kvs::CellByCellMetropolisSampling>();
        kvsview::ParticleVolumeRenderer::SetupMapper( arg, tfunc, screen, pmapper );
        pipe.connect( mapper );
        break;
    }
    case 2: // Rejection sampling
    {
        kvs::PipelineModule mapper( new kvs::CellByCellRejectionSampling );
        kvs::CellByCellRejectionSampling* pmapper = mapper.get<kvs::CellByCellRejectionSampling>();
        kvsview::ParticleVolumeRenderer::SetupMapper( arg, tfunc, screen, pmapper );
        pipe.connect( mapper );
        break;
    }
    case 3: // Layered sampling
    {
        kvs::PipelineModule mapper( new kvs::CellByCellLayeredSampling );
        kvs::CellByCellLayeredSampling* pmapper = mapper.get<kvs::CellByCellLayeredSampling>();
        kvsview::ParticleVolumeRenderer::SetupMapper( arg, tfunc, screen, pmapper );
        pipe.connect( mapper );
        break;
    }
    default: // Uniform sampling
    {
        kvs::PipelineModule mapper( new kvs::CellByCellUniformSampling );
        kvs::CellByCellUniformSampling* pmapper = mapper.get<kvs::CellByCellUniformSampling>();
        kvsview::ParticleVolumeRenderer::SetupMapper( arg, tfunc, screen, pmapper );
        pipe.connect( mapper );
        break;
    }
    }

    // Set a renderer (ParticleVolumeRenderer).
    if ( arg.noGPU() )
    {
        kvs::PipelineModule renderer( new kvs::ParticleVolumeRenderer );
        kvs::ParticleVolumeRenderer* prenderer = renderer.get<kvs::ParticleVolumeRenderer>();
        kvsview::ParticleVolumeRenderer::SetupRenderer( arg, tfunc, prenderer );

        // Subpixel level.
        const size_t subpixel_level = arg.subpixelLevel();
        const size_t repetition_level = arg.repetitionLevel();
        const size_t level = ::GetRevisedSubpixelLevel( subpixel_level, repetition_level );
        prenderer->setSubpixelLevel( level );

        pipe.connect( renderer );
    }
#if defined( KVS_SUPPORT_GLEW )
    else
    {
        kvs::PipelineModule renderer( new kvs::glew::ParticleVolumeRenderer );
        kvs::glew::ParticleVolumeRenderer* prenderer = renderer.get<kvs::glew::ParticleVolumeRenderer>();
        kvsview::ParticleVolumeRenderer::SetupRenderer( arg, tfunc, prenderer );

        // Subpixel level and repetition level.
        const size_t subpixel_level = arg.subpixelLevel();
        const size_t repetition_level = arg.repetitionLevel();
        prenderer->setSubpixelLevel( subpixel_level );
        prenderer->setRepetitionLevel( repetition_level );

        if ( !arg.noLOD() ) prenderer->enableLODControl();
        else prenderer->disableLODControl();

        if ( !arg.noZooming() ) prenderer->enableZooming();
        else prenderer->disableZooming();

        pipe.connect( renderer );
    }
#endif

    // Construct the visualization pipeline.
    if ( !pipe.exec() )
    {
        kvsMessageError("Cannot execute the visulization pipeline.");
        return( false );
    }

    // Register object.
    const_cast<kvs::ObjectBase*>( pipe.object() )->setName( ::ObjectName );
    screen.registerObject( &pipe );

    // Verbose information.
    if ( arg.verboseMode() )
    {
        std::cout << "RENDERERED OBJECT" << std::endl;
        std::cout << kvsview::ObjectInformation( pipe.object() ) << std::endl;
        std::cout << std::endl;
        std::cout << "VISUALIZATION PIPELINE" << std::endl;
        std::cout << pipe << std::endl;
    }

    // Apply the specified parameters to the global and the visualization pipeline.
    arg.applyTo( screen, pipe );
    arg.applyTo( screen );

    // Show the screen.
    screen.show();

    // Create transfer function editor.
    kvsview::ParticleVolumeRenderer::TransferFunctionEditor editor( &screen, &arg );
    editor.setVolumeObject( volume );
    editor.setTransferFunction( arg.transferFunction( volume ) );
    editor.attachVolume( volume );
    editor.attachLegendBar( &legend_bar );
    editor.show();
    editor.hide();
    ::Shown = false;

    // Attach the transfer function editor to the key press event and the mouse double-click event.
    mouse_double_click_event.attachTransferFunctionEditor( &editor );
    key_press_event.attachTransferFunctionEditor( &editor );

    return( arg.clear(), app.run() );
}

} // end of namespace ParticleVolumeRenderer

} // end of namespace kvsview
