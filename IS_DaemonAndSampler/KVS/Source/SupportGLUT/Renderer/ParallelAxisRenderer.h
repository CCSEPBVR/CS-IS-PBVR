/*****************************************************************************/
/**
 *  @file   ParallelAxisRenderer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__GLUT__PARALLEL_AXIS_RENDERER_H_INCLUDE
#define KVS__GLUT__PARALLEL_AXIS_RENDERER_H_INCLUDE

#include <kvs/RendererBase>
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/RGBColor>


namespace kvs
{

class ObjectBase;
class Camera;
class Light;

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  ParallelAxisRenderer class.
 */
/*===========================================================================*/
class ParallelAxisRenderer : public kvs::RendererBase
{
    // Class name.
    kvsClassName( kvs::glut::ParallelAxisRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::RendererBase );

protected:

    size_t m_top_margin; ///< top margin
    size_t m_bottom_margin; ///< bottom margin
    size_t m_left_margin; ///< left margin
    size_t m_right_margin; ///< right margin
    kvs::Real32   m_axis_width; ///< axis width
    kvs::RGBColor m_axis_color; ///< axis color
    kvs::RGBColor m_value_color; ///< value color
    kvs::RGBColor m_label_color; ///< label color

public:

    ParallelAxisRenderer( void );

public:

    size_t topMargin( void ) const;

    size_t bottomMargin( void ) const;

    size_t leftMargin( void ) const;

    size_t rightMargin( void ) const;

    kvs::Real32 axisWidth( void ) const;

    kvs::RGBColor axisColor( void ) const;

    kvs::RGBColor valueColor( void ) const;

    kvs::RGBColor labelColor( void ) const;

    void setTopMargin( const size_t top_margin );

    void setBottomMargin( const size_t bottom_margin );

    void setLeftMargin( const size_t left_margin );

    void setRightMargin( const size_t right_margin );

    void setAxisWidth( const kvs::Real32 axis_width );

    void setAxisColor( const kvs::RGBColor axis_color );

    void setValueColor( const kvs::RGBColor value_color );

    void setLabelColor( const kvs::RGBColor label_color );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__PARALLEL_AXIS_RENDERER_H_INCLUDE
