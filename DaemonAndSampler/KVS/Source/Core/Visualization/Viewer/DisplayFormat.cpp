/*****************************************************************************/
/**
 *  @file   DisplayFormat.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DisplayFormat.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "DisplayFormat.h"


namespace kvs
{

DisplayFormat::DisplayFormat( void ):
    m_double_buffer(true),
    m_color_buffer(true),
    m_depth_buffer(true),
    m_accumulation_buffer(false),
    m_stencil_buffer(false),
    m_stereo_buffer(false),
    m_multisample_buffer(false),
    m_alpha_channel(false)
{
}

DisplayFormat::DisplayFormat( const DisplayFormat& format ):
    m_double_buffer( format.doubleBuffer() ),
    m_color_buffer( format.colorBuffer() ),
    m_depth_buffer( format.depthBuffer() ),
    m_accumulation_buffer( format.accumulationBuffer() ),
    m_stencil_buffer( format.stencilBuffer() ),
    m_stereo_buffer( format.stereoBuffer() ),
    m_multisample_buffer( format.multisampleBuffer() ),
    m_alpha_channel( format.alphaChannel() )
{
}

DisplayFormat::~DisplayFormat( void )
{
}

const bool DisplayFormat::doubleBuffer( void ) const
{
    return( m_double_buffer );
}

const bool DisplayFormat::colorBuffer( void ) const
{
    return( m_color_buffer );
}

const bool DisplayFormat::depthBuffer( void ) const
{
    return( m_depth_buffer );
}

const bool DisplayFormat::accumulationBuffer( void ) const
{
    return( m_accumulation_buffer );
}

const bool DisplayFormat::stencilBuffer( void ) const
{
    return( m_stencil_buffer );
}

const bool DisplayFormat::stereoBuffer( void ) const
{
    return( m_stereo_buffer );
}

const bool DisplayFormat::multisampleBuffer( void ) const
{
    return( m_multisample_buffer );
}

const bool DisplayFormat::alphaChannel( void ) const
{
    return( m_alpha_channel );
}

void DisplayFormat::setDoubleBuffer( const bool enable )
{
    m_depth_buffer = enable;
}

void DisplayFormat::setColorBuffer( const bool enable )
{
    m_color_buffer = enable;
}

void DisplayFormat::setDepthBuffer( const bool enable )
{
    m_depth_buffer = enable;
}

void DisplayFormat::setAccumulationBuffer( const bool enable )
{
    m_accumulation_buffer = enable;
}

void DisplayFormat::setStencilBuffer( const bool enable )
{
    m_stencil_buffer = enable;
}

void DisplayFormat::setStereoBuffer( const bool enable )
{
    m_stereo_buffer = enable;
}

void DisplayFormat::setMultisampleBuffer( const bool enable )
{
    m_multisample_buffer = enable;
}

void DisplayFormat::setAlphaChannel( const bool enable )
{
    m_alpha_channel = enable;
}

DisplayFormat& DisplayFormat::operator = ( const DisplayFormat& format )
{
    m_double_buffer = format.doubleBuffer();
    m_color_buffer = format.colorBuffer();
    m_depth_buffer = format.depthBuffer();
    m_accumulation_buffer = format.accumulationBuffer();
    m_stencil_buffer = format.stencilBuffer();
    m_stereo_buffer = format.stereoBuffer();
    m_multisample_buffer = format.multisampleBuffer();
    m_alpha_channel = format.alphaChannel();

    return( *this );
}

} // end of namespace kvs
