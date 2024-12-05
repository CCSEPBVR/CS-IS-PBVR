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
#ifndef VIS_MODULE__PIPELINE_MODULE_H_INCLUDE
#define VIS_MODULE__PIPELINE_MODULE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/FilterBase>
#include <vismodule/MapperBase>
#include <vismodule/ObjectBase>
#include <vismodule/RendererBase>
#include <vismodule/Module>
#include <vismodule/Assert>
#include <vismodule/ReferenceCounter>


namespace vismodule
{

class VisualizationPipeline;

/*==========================================================================*/
/**
 *  Pipeline class.
 */
/*==========================================================================*/
class PipelineModule
{
    visModuleClassName_without_virtual( vismodule::PipelineModule );

    friend class vismodule::VisualizationPipeline;

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
        vismodule::FilterBase*   filter;   ///< pointer to the KVS filter class
        vismodule::MapperBase*   mapper;   ///< pointer to the KVS mapper class
        vismodule::ObjectBase*   object;   ///< pointer to the KVS object class
        vismodule::RendererBase* renderer; ///< pointer to the KVS renderer class
    };

protected:

    bool m_auto_delete; ///< flag whether the module is deleted or not (usually 'true')
    vismodule::ReferenceCounter* m_counter;  ///< Reference counter.
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
        this->read_module( module, typename vismodule::ModuleTraits<T>::ModuleCategory() );
    }

    PipelineModule( const PipelineModule& module );

    ~PipelineModule( void );

public:

    template <typename T>
    T* get( void ) const
    {
        return( this->get_module<T>( typename vismodule::ModuleTraits<T>::ModuleCategory() ) );
    }

    vismodule::ObjectBase* exec( const vismodule::ObjectBase& object );

public:

    PipelineModule& operator = ( const PipelineModule& module );

public:

    const Category category( void ) const;

    const Module module( void ) const;

    const vismodule::FilterBase* filter( void ) const;

    const vismodule::MapperBase* mapper( void ) const;

    const vismodule::ObjectBase* object( void ) const;

    const vismodule::RendererBase* renderer( void ) const;

    const char* name( void ) const;

    const bool unique( void ) const;

private:

    template <typename T>
    void read_module( T* module, VIS_MODULE_MODULE_FILTER )
    {
        m_category = PipelineModule::Filter;
        m_module.filter = module;
    }

    template <typename T>
    void read_module( T* module, VIS_MODULE_MODULE_MAPPER )
    {
        m_category = PipelineModule::Mapper;
        m_module.mapper = module;
    }

    template <typename T>
    void read_module( T* module, VIS_MODULE_MODULE_OBJECT )
    {
        m_category = PipelineModule::Object;
        m_module.object = module;
    }

    template <typename T>
    void read_module( T* module, VIS_MODULE_MODULE_RENDERER )
    {
        m_category = PipelineModule::Renderer;
        m_module.renderer = module;
    }

private:

    template <typename T>
    T* get_module( VIS_MODULE_MODULE_FILTER ) const
    {
        return( reinterpret_cast<T*>( m_module.filter ) );
    }

    template <typename T>
    T* get_module( VIS_MODULE_MODULE_MAPPER ) const
    {
        return( reinterpret_cast<T*>( m_module.mapper ) );
    }

    template <typename T>
    T* get_module( VIS_MODULE_MODULE_OBJECT ) const
    {
        return( reinterpret_cast<T*>( m_module.object ) );
    }

    template <typename T>
    T* get_module( VIS_MODULE_MODULE_RENDERER ) const
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

} // end of namespace vismodule

#endif // VIS_MODULE__PIPELINE_MODULE_H_INCLUDE
