/****************************************************************************/
/**
 *  @file PipelineModule.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PipelineModule.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__PIPELINE_MODULE_H_INCLUDE
#define KVS__PIPELINE_MODULE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/FilterBase>
#include <kvs/MapperBase>
#include <kvs/ObjectBase>
#include <kvs/RendererBase>
#include <kvs/Module>
#include <kvs/Assert>
#include <kvs/ReferenceCounter>


namespace kvs
{

class VisualizationPipeline;

/*==========================================================================*/
/**
 *  Pipeline class.
 */
/*==========================================================================*/
class PipelineModule
{
    kvsClassName_without_virtual( kvs::PipelineModule );

    friend class kvs::VisualizationPipeline;

public:

    enum Category
    {
        Empty = 0, ///< empty module
        Filter,    ///< filter module
        Mapper,    ///< mapper module
        Object,    ///< object module
        Renderer   ///< renderer module
    };

    union Module
    {
        kvs::FilterBase*   filter;   ///< pointer to the KVS filter class
        kvs::MapperBase*   mapper;   ///< pointer to the KVS mapper class
        kvs::ObjectBase*   object;   ///< pointer to the KVS object class
        kvs::RendererBase* renderer; ///< pointer to the KVS renderer class
    };

protected:

    bool m_auto_delete; ///< flag whether the module is deleted or not (usually 'true')
    kvs::ReferenceCounter* m_counter;  ///< Reference counter.
    Category               m_category; ///< module category
    Module                 m_module;   ///< pointer to the module (SHARED)

public:

    PipelineModule( void );

    template <typename T>
    explicit PipelineModule( T* module ):
        m_auto_delete( true ),
        m_counter( 0 ),
        m_category( Empty )
    {
        memset( &m_module, 0, sizeof( Module ) );
        this->create_counter( 1 );
        this->read_module( module, typename kvs::ModuleTraits<T>::ModuleCategory() );
    }

    PipelineModule( const PipelineModule& module );

    ~PipelineModule( void );

public:

    template <typename T>
    T* get( void ) const
    {
        return( this->get_module<T>( typename kvs::ModuleTraits<T>::ModuleCategory() ) );
    }

    kvs::ObjectBase* exec( const kvs::ObjectBase* object = NULL );

public:

    PipelineModule& operator = ( const PipelineModule& module );

public:

    const Category category( void ) const;

    const Module module( void ) const;

    const kvs::FilterBase* filter( void ) const;

    const kvs::MapperBase* mapper( void ) const;

    const kvs::ObjectBase* object( void ) const;

    const kvs::RendererBase* renderer( void ) const;

    const char* name( void ) const;

    const bool unique( void ) const;

private:

    template <typename T>
    void read_module( T* module, KVS_MODULE_FILTER )
    {
        m_category = PipelineModule::Filter;
        m_module.filter = module;
    }

    template <typename T>
    void read_module( T* module, KVS_MODULE_MAPPER )
    {
        m_category = PipelineModule::Mapper;
        m_module.mapper = module;
    }

    template <typename T>
    void read_module( T* module, KVS_MODULE_OBJECT )
    {
        m_category = PipelineModule::Object;
        m_module.object = module;
    }

    template <typename T>
    void read_module( T* module, KVS_MODULE_RENDERER )
    {
        m_category = PipelineModule::Renderer;
        m_module.renderer = module;
    }

private:

    template <typename T>
    T* get_module( KVS_MODULE_FILTER ) const
    {
        return( reinterpret_cast<T*>( m_module.filter ) );
    }

    template <typename T>
    T* get_module( KVS_MODULE_MAPPER ) const
    {
        return( reinterpret_cast<T*>( m_module.mapper ) );
    }

    template <typename T>
    T* get_module( KVS_MODULE_OBJECT ) const
    {
        return( reinterpret_cast<T*>( m_module.object ) );
    }

    template <typename T>
    T* get_module( KVS_MODULE_RENDERER ) const
    {
        return( reinterpret_cast<T*>( m_module.renderer ) );
    }

private:

    void enable_auto_delete( void );

    void disable_auto_delete( void );

    void delete_module( void );

private:

    void shallow_copy( const PipelineModule& module );

    void deep_copy( const PipelineModule& module );

    void create_counter( const size_t counter = 1 );

    void ref( void );

    void unref( void );
};

} // end of namespace kvs

#endif // KVS__PIPELINE_MODULE_H_INCLUDE
