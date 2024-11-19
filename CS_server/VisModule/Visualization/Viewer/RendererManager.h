/****************************************************************************/
/**
 *  @file RendererManager.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RendererManager.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__RENDERER_MANAGER_H_INCLUDE
#define VIS_MODULE__RENDERER_MANAGER_H_INCLUDE

#include <string>
#include <list>
#include <map>
#include <vismodule/ClassName>
#include <vismodule/RendererBase>


namespace vismodule
{

typedef std::list<vismodule::RendererBase*> RendererManagerBase;

/*==========================================================================*/
/**
*  Renderer manager class.
*/
/*==========================================================================*/
class RendererManager : public RendererManagerBase
{
    visModuleClassName( vismodule::RendererManager );

public:

    typedef RendererManagerBase::iterator  RendererIterator;
    typedef std::map<int,RendererIterator> RendererMap;

protected:

    RendererMap m_renderer_map; ///< renderer map

public:

    RendererManager( void );

    virtual ~RendererManager( void );

public:

    int insert( vismodule::RendererBase* renderer );

    void erase( bool delete_flg = true );

    void erase( int renderer_id, bool delete_flg = true );

    void erase( std::string renderer_name, bool delete_flg = true );

    void change( int renderer_id, vismodule::RendererBase* renderer, bool delete_flg = true );

    void change( std::string renderer_name, vismodule::RendererBase* renderer, bool delete_flg = true );

    const int nrenderers( void ) const;

    vismodule::RendererBase* renderer( void );

    vismodule::RendererBase* renderer( int renderer_id );

    vismodule::RendererBase* renderer( std::string renderer_name );

    const bool hasRenderer( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__RENDERER_MANAGER_H_INCLUDE
