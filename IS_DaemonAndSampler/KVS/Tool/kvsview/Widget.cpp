/*****************************************************************************/
/**
 *  @file   Widget.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Widget.cpp 620 2010-09-30 08:03:52Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Widget.h"
#include <kvs/RendererBase>


namespace kvsview
{

namespace Widget
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new FPSLabel.
 *  @param  screen [in] pointer to the screen
 *  @param  renderer_name [in] renderer name
 */
/*===========================================================================*/
FPSLabel::FPSLabel( kvs::ScreenBase* screen, const std::string renderer_name ):
    kvs::glut::Label( screen ),
    m_renderer_name( renderer_name )
{
    setMargin( 10 );
}

/*===========================================================================*/
/**
 *  @brief  Update FPS string.
 */
/*===========================================================================*/
void FPSLabel::screenUpdated( void )
{
    const kvs::RendererBase* renderer = screen()->rendererManager()->renderer( m_renderer_name );

    std::stringstream fps;
    fps << std::setprecision(4) << renderer->timer().fps();
    setText( std::string( "fps: " + fps.str() ).c_str() );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new LegendBar.
 *  @param  screen [in] pointer to the screen
 */
/*===========================================================================*/
LegendBar::LegendBar( kvs::ScreenBase* screen ):
    kvs::glut::LegendBar( screen )
{
    setWidth( 200 );
    setHeight( 50 );
}

/*===========================================================================*/
/**
 *  @brief  Anchor the LegendBar to the left-bottom on the screen.
 */
/*===========================================================================*/
void LegendBar::screenResized( void )
{
    setX( screen()->width() - width() );
    setY( screen()->height() - height() );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new OrientationAxis.
 *  @param  screen [in] pointer to the screen
 */
/*===========================================================================*/
OrientationAxis::OrientationAxis( kvs::ScreenBase* screen ):
    kvs::glut::OrientationAxis( screen )
{
    setMargin( 10 );
    setSize( 90 );
    setBoxType( kvs::glut::OrientationAxis::SolidBox );
    enableAntiAliasing();
}

/*===========================================================================*/
/**
 *  @brief  Anchor the OrientationAxis to the left-bottom on the screen.
 */
/*===========================================================================*/
void OrientationAxis::screenResized( void )
{
    setY( screen()->height() - height() );
}

} // end of namespace Widget

} // end of namespace kvsview
