/****************************************************************************/
/**
 *  @file RendererBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RendererBase.cpp 619 2010-09-27 09:57:28Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "RendererBase.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
RendererBase::RendererBase( void ) :
    m_name("unknown"),
    m_shading_flag( true )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
RendererBase::~RendererBase( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Set renderer name.
 *  @param  name [in] renderer name
 */
/*===========================================================================*/
void RendererBase::setName( const std::string& name )
{
    m_name = name;
}

/*===========================================================================*/
/**
 *  @brief  Returns renderer name.
 *  @return renderer name
 */
/*===========================================================================*/
const std::string& RendererBase::name( void ) const
{
    return( m_name );
}

/*==========================================================================*/
/**
 *  Get the rendering timer.
 */
/*==========================================================================*/
const kvs::Timer& RendererBase::timer( void ) const
{
    return( m_timer );
}

/*==========================================================================*/
/**
 *  Get the rendering timer.
 */
/*==========================================================================*/
kvs::Timer& RendererBase::timer( void )
{
    return( m_timer );
}

/*==========================================================================*/
/**
 *  Test whether the shading is enable or disable.
 *  @return true, if the shading is enable.
 */
/*==========================================================================*/
const bool RendererBase::isShading( void ) const
{
    return( m_shading_flag );
}

/*==========================================================================*/
/**
 *  Enable to shading.
 */
/*==========================================================================*/
void RendererBase::enableShading( void ) const
{
    m_shading_flag = true;
}

/*==========================================================================*/
/**
 *  Disable to shading.
 */
/*==========================================================================*/
void RendererBase::disableShading( void ) const
{
    m_shading_flag = false;
}

/*==========================================================================*/
/**
 *  Initialize the renderer.
 */
/*==========================================================================*/
void RendererBase::initialize( void )
{
    initialize_projection();
    initialize_modelview();
}

/*==========================================================================*/
/**
 *  Initialize the projection information.
 */
/*==========================================================================*/
void RendererBase::initialize_projection( void )
{
}

/*==========================================================================*/
/**
 *  Initialize about the modelview information.
 */
/*==========================================================================*/
void RendererBase::initialize_modelview( void )
{
}

} // end of namespace kvs
