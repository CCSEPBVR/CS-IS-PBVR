/*****************************************************************************/
/**
 *  @file   AxisRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AxisRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__AXIS_RENDERER_H_INCLUDE
#define KVS__GLUT__AXIS_RENDERER_H_INCLUDE

#include <kvs/LineRenderer>


namespace kvs
{

class AxisObject;

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Axis renderer.
 */
/*===========================================================================*/
class AxisRenderer : public kvs::LineRenderer
{
    // Class name.
    kvsClassName( kvs::glut::AxisRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleSuperClass( kvs::LineRenderer );

protected:

    bool m_enable_anti_aliasing;

public:

    AxisRenderer( void );

    virtual ~AxisRenderer( void );

public:

    void enableAntiAliasing( void );

    void disableAntiAliasing( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

private:

    void draw_tag( const kvs::AxisObject* axis );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__AXIS_RENDERER_H_INCLUDE
