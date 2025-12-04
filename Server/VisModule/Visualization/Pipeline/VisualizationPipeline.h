/****************************************************************************/
/**
 *  @file VisualizationPipeline.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VisualizationPipeline.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__VISUALIZATION_PIPELINE_H_INCLUDE
#define VIS_MODULE__VISUALIZATION_PIPELINE_H_INCLUDE

#include <iostream>
#include <string>
#include <list>
#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/GeometryObjectBase>
#include <vismodule/VolumeObjectBase>
#include <vismodule/RendererBase>
#include <vismodule/Module>
#include <vismodule/PipelineModule>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Visualization pipeline class.
 */
/*==========================================================================*/
class VisualizationPipeline
{
    visModuleClassName( vismodule::VisualizationPipeline );

public:

    typedef std::list<vismodule::PipelineModule> ModuleList;

private:

    size_t      m_id;          ///< pipeline ID
    std::string m_filename;    ///< filename
    bool        m_cache;       ///< cache mode (DISABLE NOW)
    ModuleList  m_module_list; ///< pipeline module list

    const vismodule::ObjectBase*   m_object;   ///< pointer to the object inserted to the manager
    const vismodule::RendererBase* m_renderer; ///< pointer to the renderer inserted to the manager

private:

    VisualizationPipeline( void );

public:

    explicit VisualizationPipeline( const std::string& filename );

    explicit VisualizationPipeline( vismodule::ObjectBase* object );

    virtual ~VisualizationPipeline( void );

public:

    VisualizationPipeline& connect( vismodule::PipelineModule& module );

    bool import( void );

    bool exec( void );

public:

    bool cache( void ) const;

    void enableCache( void );

    void disableCache( void );

    bool hasObject( void ) const;

    bool hasRenderer( void ) const;

    const vismodule::ObjectBase& object( void ) const;

    const vismodule::RendererBase* renderer( void ) const;

    void print( void ) const;

public:

    friend std::string& operator << ( std::string& str, const VisualizationPipeline& pipeline );

    friend std::ostream& operator << ( std::ostream& os, const VisualizationPipeline& pipeline );

private:

    bool create_object_module( const std::string& filename );

    bool create_renderer_module( const vismodule::ObjectBase& object );

    bool create_renderer_module( const vismodule::GeometryObjectBase* geometry );

    bool create_renderer_module( const vismodule::VolumeObjectBase& volume );

private:

    ModuleList::iterator find_module( const vismodule::PipelineModule::Category category );

    const size_t count_module( const vismodule::PipelineModule::Category category ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__VISUALIZATION_PIPELINE_H_INCLUDE
