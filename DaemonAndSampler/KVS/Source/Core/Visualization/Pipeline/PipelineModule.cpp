/****************************************************************************/
/**
 *  @file PipelineModule.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PipelineModule.cpp 651 2010-10-25 07:54:11Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "PipelineModule.h"


namespace kvs
{

PipelineModule::PipelineModule( void ):
    m_auto_delete( true ),
    m_counter( 0 ),
    m_category( Empty )
{
    memset( &m_module, 0, sizeof( Module ) );
}

PipelineModule::PipelineModule( const PipelineModule& module ):
    m_auto_delete( true ),
    m_counter( 0 ),
    m_category( Empty )
{
    memset( &m_module, 0, sizeof( Module ) );
    this->shallow_copy( module );
}

PipelineModule::~PipelineModule( void )
{
    this->unref();
}

kvs::ObjectBase* PipelineModule::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        return( m_category == PipelineModule::Object ? m_module.object : NULL );
    }

    switch( m_category )
    {
    case PipelineModule::Filter: return( m_module.filter->exec( object ) );
    case PipelineModule::Mapper: return( m_module.mapper->exec( object ) );
    default: break;
    }

    return( NULL );
}

PipelineModule& PipelineModule::operator = ( const PipelineModule& module )
{
    if ( this != &module )
    {
        if ( m_category == module.m_category )
        {
            this->shallow_copy( module );
        }
        else
        {
            this->deep_copy( module );
        }
    }

    return( *this );
}

const PipelineModule::Category PipelineModule::category( void ) const
{
    return( m_category );
}

const PipelineModule::Module PipelineModule::module( void ) const
{
    return( m_module );
}

const kvs::FilterBase* PipelineModule::filter( void ) const
{
    return( m_module.filter );
}

const kvs::MapperBase* PipelineModule::mapper( void ) const
{
    return( m_module.mapper );
}

const kvs::ObjectBase* PipelineModule::object( void ) const
{
    return( m_module.object );
}

const kvs::RendererBase* PipelineModule::renderer( void ) const
{
    return( m_module.renderer );
}

const char* PipelineModule::name( void ) const
{
    switch ( m_category )
    {
    case PipelineModule::Filter:   return( m_module.filter->className() );
    case PipelineModule::Mapper:   return( m_module.mapper->className() );
    case PipelineModule::Object:   return( m_module.object->className() );
    case PipelineModule::Renderer: return( m_module.renderer->className() );
    default: break;
    }

    return( "Unknown class" );
}

const bool PipelineModule::unique( void ) const
{
    return( m_counter ? m_counter->value() == 1 : true );
}

void PipelineModule::enable_auto_delete( void )
{
    m_auto_delete = true;
}

void PipelineModule::disable_auto_delete( void )
{
    m_auto_delete = false;
}

void PipelineModule::delete_module( void )
{
    // WARNING: If m_auto_delete is 'false', the module is not deleted in this
    // class. In this case, you have to delete the module by calling the delete_module
    // method after calling the enable_auto_delete method. However, you can only
    // call these methods in the VisualizationPipeline class since these methods
    // is defined as private method and the VisualizationPipeline class is specified
    // as friend class in this class definition.
    if ( !m_auto_delete ) return;

    switch( m_category )
    {
    case PipelineModule::Filter:   if ( m_module.filter   ) delete m_module.filter;  m_module.filter   = NULL; break;
    case PipelineModule::Mapper:   if ( m_module.mapper   ) delete m_module.mapper;  m_module.mapper   = NULL; break;
    case PipelineModule::Object:   if ( m_module.object   ) delete m_module.object;  m_module.object   = NULL; break;
    case PipelineModule::Renderer: if ( m_module.renderer ) delete m_module.renderer;m_module.renderer = NULL; break;
    default: break;
    }
}

void PipelineModule::shallow_copy( const PipelineModule& module )
{
    this->unref();
    m_counter  = module.m_counter;
    m_category = module.m_category;
    m_module   = module.m_module;
    this->ref();
}

void PipelineModule::deep_copy( const PipelineModule& module )
{
    this->unref();
    this->create_counter();
    m_category = module.m_category;
    m_module   = module.m_module;
}

void PipelineModule::create_counter( const size_t counter )
{
    m_counter = new kvs::ReferenceCounter( counter );
    KVS_ASSERT( m_counter != NULL );
}

void PipelineModule::ref( void )
{
    if ( m_counter ) { m_counter->increment(); }
}

void PipelineModule::unref( void )
{
    if ( m_counter )
    {
        m_counter->decrement();

        if ( m_counter->value() == 0 )
        {
            this->delete_module();
            if ( m_counter ) { delete m_counter; }
        }
    }

    m_counter = 0;
    m_category = PipelineModule::Empty;
}

} // end of namespace kvs
