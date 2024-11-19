/****************************************************************************/
/**
 *  @file VisualizationPipeline.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VisualizationPipeline.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "VisualizationPipeline.h"
#include <vismodule/DebugNew>
#include <vismodule/ObjectImporter>
#include <vismodule/File>
#include <vismodule/ImageRenderer>
#include <vismodule/PointRenderer>
#include <vismodule/LineRenderer>
#include <vismodule/PolygonRenderer>
#include <vismodule/RayCastingRenderer>


// Static parameters.
namespace { size_t Counter = 0; }
namespace { bool Flag = true; }
namespace { const size_t MaxNumberOfPipelines = 256; }
namespace { vismodule::VisualizationPipeline* context[::MaxNumberOfPipelines]; }

namespace
{

/*===========================================================================*/
/**
 *  @brief  Function that is called when the application is terminated.
 */
/*===========================================================================*/
void ExitFunction( void )
{
    for ( size_t i = 0; i < ::MaxNumberOfPipelines; i++)
    {
        if ( ::context[i] ) ::context[i]->~VisualizationPipeline();
    }
}

} // end of namespace


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new VisualizationPipeline class.
 */
/*===========================================================================*/
VisualizationPipeline::VisualizationPipeline( void ):
    m_id( ::Counter++ ),
    m_filename(""),
    m_cache( true ),
    m_object( NULL ),
    m_renderer( NULL )
{
    ::context[ m_id ] = this;
    if ( ::Flag ) { atexit( ::ExitFunction ); ::Flag = false; }
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new visualization pipeline and sets the input data filename.
 *  @param  filename [in] input data filename
 */
/*===========================================================================*/
VisualizationPipeline::VisualizationPipeline( const std::string& filename ):
    m_id( ::Counter++ ),
    m_filename( filename ),
    m_cache( true ),
    m_object( NULL ),
    m_renderer( NULL )
{
    ::context[ m_id ] = this;
    if ( ::Flag ) { atexit( ::ExitFunction ); ::Flag = false; }
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new visualization pipeline and sets the object module.
 *  @param  object [in] pointer to the object
 */
/*===========================================================================*/
VisualizationPipeline::VisualizationPipeline( vismodule::ObjectBase* object ):
    m_id( ::Counter++ ),
    m_filename(""),
    m_cache( true ),
    m_object( NULL ),
    m_renderer( NULL )
{
    ::context[ m_id ] = this;
    if ( ::Flag ) { atexit( ::ExitFunction ); ::Flag = false; }

    vismodule::PipelineModule module( object );
    m_module_list.push_front( module );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the visualization pipeline
 */
/*===========================================================================*/
VisualizationPipeline::~VisualizationPipeline( void )
{
    m_module_list.clear();
    ::context[ m_id ] = 0;
}

/*===========================================================================*/
/**
 *  @brief  Connects a module to the visualization pipeline.
 *  @param  module [in] pipeline module
 */
/*===========================================================================*/
VisualizationPipeline& VisualizationPipeline::connect( vismodule::PipelineModule& module )
{
    m_module_list.push_back( module );

    return( *this );
}

/*===========================================================================*/
/**
 *  @brief  Import the data file that is specified by the filename.
 *  @return true, if the import process is done successfully
 */
/*===========================================================================*/
bool VisualizationPipeline::import( void )
{
    if ( !this->hasObject() )
    {
        // Check filename.
        if ( m_filename.empty() )
        {
            visModuleMessageError( "Input data is not specified." );
            return( false );
        }

        // Create object module.
        if ( !this->create_object_module( m_filename ) )
        {
            visModuleMessageError( "Cannot create a object from '%s'.", m_filename.c_str() );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Execute the visualization pipeline.
 *  @return true, if the visualization pipeline is executed successfully.
 */
/*===========================================================================*/
bool VisualizationPipeline::exec( void )
{
    // Setup object.
    if ( !this->import() )
    {
        visModuleMessageError( "Cannot import the object." );
        return( false );
    }

    vismodule::ObjectBase* object = NULL;
    ModuleList::iterator module = m_module_list.begin();
    ModuleList::iterator last   = m_module_list.end();

    // Skip the renderer module since the renderer is executed in the display function.
    if ( this->hasRenderer() ) --last;

    // Execute the filter or the mapper module.
    while( module != last )
    {
        object = module->exec( object );
        if ( !object )
        {
            visModuleMessageError("Cannot execute '%s'.", module->name() );
            return( false );
        }

        ++module;
    }

    // WARNING: The object module that is registerd in the VISMODULE's object manager
    // is disconnected from the pipeline module for the object with the reference counter.
    // The pointer to the disconnected object (m_object) is transfered to the object
    // manager using the insert method in the VISMODULE's global class. As a result, the ownership
    // of the object is changed to the object manager from the pipeline module.
    if ( module == last )
    {
        --module;
        module->disable_auto_delete();
    }

    // Attache the pointer to the object that is registered in the object manager.
    m_object = object;

    // Setup renderer.
    if ( !this->hasRenderer() )
    {
        // create renderer module.
        if ( !this->create_renderer_module( object ) )
        {
            visModuleMessageError( "Cannot create a renderer for '%s'.", m_filename.c_str() );
            return( false );
        }
    }

    // Find the renderer module.
    ModuleList::iterator renderer_module = this->find_module( vismodule::PipelineModule::Renderer );
    if ( renderer_module != m_module_list.end() )
    {
        // WARNING: The renderer module is disconnected from the pipeline module
        // as well as the object module.
        renderer_module->disable_auto_delete();

        // Attach the pointer to the renderer that is registered in the renderer manager.
        m_renderer = renderer_module->renderer();
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the cache mechanism is enable or disable. (currently N/A)
 *  @return true, if the cache is enable.
 */
/*===========================================================================*/
bool VisualizationPipeline::cache( void ) const
{
    return( m_cache );
}

/*===========================================================================*/
/**
 *  @brief  Enable the cache mechanism.
 */
/*===========================================================================*/
void VisualizationPipeline::enableCache( void )
{
    m_cache = true;
}

/*===========================================================================*/
/**
 *  @brief  Disable the cache mechanism.
 */
/*===========================================================================*/
void VisualizationPipeline::disableCache( void )
{
    m_cache = false;
}

/*===========================================================================*/
/**
 *  @brief  Check whether the object module is included in the pipeline.
 *  @return true, if the object module is included.
 */
/*===========================================================================*/
bool VisualizationPipeline::hasObject( void ) const
{
    return( this->count_module( vismodule::PipelineModule::Object ) > 0 );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the renderer module is included in the pipeline.
 *  @return true, if the renderer module is included.
 */
/*===========================================================================*/
bool VisualizationPipeline::hasRenderer( void ) const
{
    return( this->count_module( vismodule::PipelineModule::Renderer ) > 0 );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the object.
 *  @return pointer to the object
 */
/*===========================================================================*/
const vismodule::ObjectBase* VisualizationPipeline::object( void ) const
{
    return( m_object );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the renderer.
 *  @return pointer to the renderer
 */
/*===========================================================================*/
const vismodule::RendererBase* VisualizationPipeline::renderer( void ) const
{
    return( m_renderer );
}

/*===========================================================================*/
/**
 *  @brief  Prints the visualization pipeline as string.
 */
/*===========================================================================*/
void VisualizationPipeline::print( void ) const
{
    std::cout << *this << std::endl;
}

std::string& operator << ( std::string& str, const VisualizationPipeline& pipeline )
{
    str = vismodule::File( pipeline.m_filename ).fileName();

    const std::string separator = " >> ";
    VisualizationPipeline::ModuleList::const_iterator module = pipeline.m_module_list.begin();
    VisualizationPipeline::ModuleList::const_iterator end = pipeline.m_module_list.end();
    while( module != end )
    {
        str += separator + module->name();
        ++module;
    }

    return( str );
}

std::ostream& operator << ( std::ostream& os, const VisualizationPipeline& pipeline )
{
    std::string p; p << pipeline; os << p;

    return( os );
}

/*===========================================================================*/
/**
 *  @brief  Creates a object module.
 *  @param  filename [in] input data filename
 *  @return true, if the object is created successfully.
 */
/*===========================================================================*/
bool VisualizationPipeline::create_object_module( const std::string& filename )
{
    // Read a file and import a object.
    vismodule::ObjectImporter importer( filename );
    vismodule::ObjectBase* object = importer.import();
    if ( !object )
    {
        visModuleMessageError( "Cannot import a object." );
        return( false );
    }

    // Store the imported object to the module list.
    vismodule::PipelineModule module( object );
    m_module_list.push_front( module );

    // Attache the imported object.
    m_object = object;

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Create a renderer module according to the rendering object.
 *  @param  object [in] pointer to the object
 *  @return true, if the renderer is created successfully.
 */
/*===========================================================================*/
bool VisualizationPipeline::create_renderer_module( const vismodule::ObjectBase* object )
{
    switch ( object->objectType() )
    {
    case vismodule::ObjectBase::Geometry:
    {
        const vismodule::GeometryObjectBase* geometry =
            reinterpret_cast<const vismodule::GeometryObjectBase*>( object );

        return( this->create_renderer_module( geometry ) );
    }
    case vismodule::ObjectBase::Volume:
    {
        const vismodule::VolumeObjectBase* volume =
            reinterpret_cast<const vismodule::VolumeObjectBase*>( object );

        return( this->create_renderer_module( volume ) );
    }
    case vismodule::ObjectBase::Image:
    {
        vismodule::PipelineModule module( new vismodule::ImageRenderer );
        m_module_list.push_back( module );
        return( true );
    }
    default: break;
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Creates a renderer module for the geometry object.
 *  @param  geometry [in] pointer to the geometry object
 *  @return true, if the renderer is created successfully.
 */
/*===========================================================================*/
bool VisualizationPipeline::create_renderer_module( const vismodule::GeometryObjectBase* geometry )
{
    bool ret = true;

    switch ( geometry->geometryType() )
    {
    case vismodule::GeometryObjectBase::Point:
    {
        vismodule::PipelineModule module( new vismodule::PointRenderer );
        m_module_list.push_back( module );
        break;
    }
    case vismodule::GeometryObjectBase::Line:
    {
        vismodule::PipelineModule module( new vismodule::LineRenderer );
        m_module_list.push_back( module );
        break;
    }
    case vismodule::GeometryObjectBase::Polygon:
    {
        vismodule::PipelineModule module( new vismodule::PolygonRenderer );
        m_module_list.push_back( module );
        break;
    }
    default: ret = false; break;
    }

    return( ret );
}

/*===========================================================================*/
/**
 *  @brief  Creates a renderer module for the volume object.
 *  @param  volume [in] pointer to the volume object
 *  @return true, if the renderer is created successfully.
 */
/*===========================================================================*/
bool VisualizationPipeline::create_renderer_module( const vismodule::VolumeObjectBase* volume )
{
    bool ret = true;

    switch ( volume->volumeType() )
    {
    case vismodule::VolumeObjectBase::Structured:
    {
        vismodule::PipelineModule module( new vismodule::RayCastingRenderer );
        m_module_list.push_back( module );
        break;
    }
    case vismodule::VolumeObjectBase::Unstructured:
    {
/*
        vismodule::PipelineModule module( new vismodule::RayCastingRenderer );
        m_module_list.push_back( module );
*/
        ret = false;
        break;
    }
    default: ret = false; break;
    }

    return( ret );
}

/*===========================================================================*/
/**
 *  @brief  Finds the module in the specified category.
 *  @param  category [in] module category
 *  @return interator to the found module.
 */
/*===========================================================================*/
VisualizationPipeline::ModuleList::iterator VisualizationPipeline::find_module(
    const vismodule::PipelineModule::Category category )
{
    ModuleList::iterator module = m_module_list.begin();
    ModuleList::iterator end = m_module_list.end();
    while( module != end )
    {
        if ( module->category() == category )
        {
            return( module );
        }

        ++module;
    }

    return( end );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of the modules in the specified category.
 *  @param  category [in] module category
 *  @return number of the modules
 */
/*===========================================================================*/
const size_t VisualizationPipeline::count_module(
    const vismodule::PipelineModule::Category category ) const
{
    size_t counter = 0;

    ModuleList::const_iterator module = m_module_list.begin();
    ModuleList::const_iterator end = m_module_list.end();
    while( module != end )
    {
        if ( module->category() == category )
        {
            ++counter;
        }

        ++module;
    }

    return( counter );
}

} // end of namespace vismodule
