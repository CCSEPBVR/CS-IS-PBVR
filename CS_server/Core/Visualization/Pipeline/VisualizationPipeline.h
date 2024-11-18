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
#ifndef KVS__VISUALIZATION_PIPELINE_H_INCLUDE
#define KVS__VISUALIZATION_PIPELINE_H_INCLUDE

#include <iostream>
#include <string>
#include <list>
#include <kvs/ClassName>
#include <kvs/ObjectBase>
#include <kvs/GeometryObjectBase>
#include <kvs/VolumeObjectBase>
#include <kvs/RendererBase>
#include <kvs/Module>
#include <kvs/PipelineModule>


namespace kvs
{

/*==========================================================================*/
/**
 *  Visualization pipeline class.
 */
/*==========================================================================*/
class VisualizationPipeline
{
    kvsClassName( kvs::VisualizationPipeline );

public:

    typedef std::list<kvs::PipelineModule> ModuleList;

private:

    size_t      m_id;          ///< pipeline ID
    std::string m_filename;    ///< filename
    bool        m_cache;       ///< cache mode (DISABLE NOW)
    ModuleList  m_module_list; ///< pipeline module list

    const kvs::ObjectBase*   m_object;   ///< pointer to the object inserted to the manager
    const kvs::RendererBase* m_renderer; ///< pointer to the renderer inserted to the manager

private:

    VisualizationPipeline( void );

public:

    explicit VisualizationPipeline( const std::string& filename );

    explicit VisualizationPipeline( kvs::ObjectBase* object );

    virtual ~VisualizationPipeline( void );

public:

    VisualizationPipeline& connect( kvs::PipelineModule& module );

    bool import( void );

    bool exec( void );

public:

    bool cache( void ) const;

    void enableCache( void );

    void disableCache( void );

    bool hasObject( void ) const;

    bool hasRenderer( void ) const;

    const kvs::ObjectBase* object( void ) const;

    const kvs::RendererBase* renderer( void ) const;

    void print( void ) const;

public:

    friend std::string& operator << ( std::string& str, const VisualizationPipeline& pipeline );

    friend std::ostream& operator << ( std::ostream& os, const VisualizationPipeline& pipeline );

private:

    bool create_object_module( const std::string& filename );

    bool create_renderer_module( const kvs::ObjectBase* object );

    bool create_renderer_module( const kvs::GeometryObjectBase* geometry );

    bool create_renderer_module( const kvs::VolumeObjectBase* volume );

private:

    ModuleList::iterator find_module( const kvs::PipelineModule::Category category );

    const size_t count_module( const kvs::PipelineModule::Category category ) const;
};

} // end of namespace kvs

#endif // KVS__VISUALIZATION_PIPELINE_H_INCLUDE
