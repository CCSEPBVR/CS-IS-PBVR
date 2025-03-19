/****************************************************************************/
/**
 *  @file RendererBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RendererBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__RENDERER_BASE_H_INCLUDE
#define VIS_MODULE__RENDERER_BASE_H_INCLUDE

#include <string>
#include <vismodule/ClassName>
#include <vismodule/Timer>
#include <vismodule/Module>


namespace vismodule
{

class ObjectBase;
class Camera;
class Light;

/*==========================================================================*/
/**
 *  Renderer base class.
 */
/*==========================================================================*/
class RendererBase
{
    visModuleClassName( vismodule::RendererBase );

    visModuleBase;

protected:

    std::string  m_name;         ///< renderer name
    vismodule::Timer   m_timer;        ///< timer
    mutable bool m_shading_flag; ///< shading flag

public:

    RendererBase( void );

    virtual ~RendererBase( void );

public:

    void setName( const std::string& name );

    const std::string& name( void ) const;

    const vismodule::Timer& timer( void ) const;

protected:

    vismodule::Timer& timer( void );

public:

    const bool isShading( void ) const;

    void enableShading( void ) const;

    void disableShading( void ) const;

public:

    void initialize( void );

    virtual void exec(
        vismodule::ObjectBase* object,
        vismodule::Camera*     camera = NULL,
        vismodule::Light*      light  = NULL ) = 0;

protected:

    virtual void initialize_projection( void );

    virtual void initialize_modelview( void );
};


} // end of namespace vismodule

#endif // VIS_MODULE__RENDERER_BASE_H_INCLUDE
