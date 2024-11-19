/****************************************************************************/
/**
 *  @file Background.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Background.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Background.h"
#include <kvs/OpenGL>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Background::Background( void )
{
    m_type = Background::MonoColor;
}

/*==========================================================================*/
/**
 *  Constructor for mono-color background.
 *  @param color [in] background color
 */
/*==========================================================================*/
Background::Background( const kvs::RGBAColor& color )
{
    this->setColor( color );
}

/*==========================================================================*/
/**
 *  Constructor for gradation background.
 *  @param color0 [in] color on the corner #0 (bottom side color)
 *  @param color1 [in] color on the corner #1 (top side color)
 */
/*==========================================================================*/
Background::Background( const kvs::RGBAColor& color1, const kvs::RGBAColor& color2 )
{
    this->setColor( color1, color2 );
}

/*==========================================================================*/
/**
 *  Constructor for gradation background.
 *  @param color0 [in] color on the corner #0 (bottom-left color)
 *  @param color1 [in] color on the corner #1 (bottom-right color)
 *  @param color2 [in] color on the corner #2 (top-right color)
 *  @param color3 [in] color on the corner #3 (top-left color)
 */
/*==========================================================================*/
Background::Background(
    const kvs::RGBAColor& color0,
    const kvs::RGBAColor& color1,
    const kvs::RGBAColor& color2,
    const kvs::RGBAColor& color3 )
{
    this->setColor( color0, color1, color2, color3 );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Background::~Background( void )
{
}

/*==========================================================================*/
/**
 *  Copy operator.
 *  @param bg [in] background object
 */
/*==========================================================================*/
Background& Background::operator = ( const Background& bg )
{
    m_type     = bg.m_type;
    m_color[0] = bg.m_color[0];
    m_color[1] = bg.m_color[1];
    m_color[2] = bg.m_color[2];
    m_color[3] = bg.m_color[3];

    return( *this );
}

/*==========================================================================*/
/**
 *  Set the color.
 *  @param color [in] color
 */
/*==========================================================================*/
void Background::setColor( const kvs::RGBAColor& color )
{
    m_type     = Background::MonoColor;
    m_color[0] = color;
}

/*==========================================================================*/
/**
 *  Set the colors.
 *  @param color0 [in] color on the corner #0 (bottom side color)
 *  @param color1 [in] color on the corner #1 (top side color)
 */
/*==========================================================================*/
void Background::setColor( const RGBAColor& color0, const RGBAColor& color1 )
{
    m_type     = Background::TwoSideColor;
    m_color[0] = color0;
    m_color[1] = color0;
    m_color[2] = color1;
    m_color[3] = color1;
}

/*==========================================================================*/
/**
 *  Set the colors.
 *  @param color0 [in] color on the corner #0 (bottom-left color)
 *  @param color1 [in] color on the corner #1 (bottom-right color)
 *  @param color2 [in] color on the corner #2 (top-right color)
 *  @param color3 [in] color on the corner #3 (top-left color)
 */
/*==========================================================================*/
void Background::setColor(
    const kvs::RGBAColor& color0,
    const kvs::RGBAColor& color1,
    const kvs::RGBAColor& color2,
    const kvs::RGBAColor& color3 )
{
    m_type     = Background::FourCornersColor;
    m_color[0] = color0;
    m_color[1] = color1;
    m_color[2] = color2;
    m_color[3] = color3;
}

/*==========================================================================*/
/**
 *  Get the background color.
 *  @param index [in] color index (0-3)
 */
/*==========================================================================*/
const kvs::RGBColor& Background::color( size_t index ) const
{
    return( m_color[index] );
}

/*==========================================================================*/
/**
 *  
 *  @param  image   
 */
/*==========================================================================*/
/*
void Background::setImage( const ImageObject& image )
{
    m_type = BG_IMAGE;

    m_mipmap.copy( image );
    m_mipmap.bind();
}
*/

/*==========================================================================*/
/**
 *  Apply gradation color or image to the background.
 */
/*==========================================================================*/
void Background::apply( void )
{
    switch( m_type )
    {
    case Background::MonoColor:        this->apply_mono_color();      break;
    case Background::TwoSideColor:     this->apply_gradation_color(); break;
    case Background::FourCornersColor: this->apply_gradation_color(); break;
//    case BG_IMAGE:              apply_image();           break;
    default: break;
    }
    glFlush();
}

/*==========================================================================*/
/**
 *  Apply mono color to the background.
 */
/*==========================================================================*/
void Background::apply_mono_color( void )
{
    float r = static_cast<float>(m_color[0].r()) / 255.0f;
    float g = static_cast<float>(m_color[0].g()) / 255.0f;
    float b = static_cast<float>(m_color[0].b()) / 255.0f;
    glClearColor( r, g, b, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

/*==========================================================================*/
/**
 *  Apply gradation color to the background.
 */
/*==========================================================================*/
void Background::apply_gradation_color( void )
{
    // Clear bits.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    // Disable OpenGL parameters.
    glDisable( GL_DEPTH_TEST );

    // Draw a gradation plane on the background.
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    {
        glLoadIdentity();

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho( 0, 1, 0, 1, -1, 1 );

            // Gradation plane.
            glBegin( GL_QUADS );
            glColor3ub( m_color[0].r(), m_color[0].g(), m_color[0].b() );
            glVertex2d( 0.0, 0.0 );
            glColor3ub( m_color[1].r(), m_color[1].g(), m_color[1].b() );
            glVertex2d( 1.0, 0.0 );
            glColor3ub( m_color[2].r(), m_color[2].g(), m_color[2].b() );
            glVertex2d( 1.0, 1.0 );
            glColor3ub( m_color[3].r(), m_color[3].g(), m_color[3].b() );
            glVertex2d( 0.0, 1.0 );
            glEnd(); 
        }
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    glPopMatrix();

    glClearDepth(1000);
    glEnable( GL_DEPTH_TEST );

    glPopAttrib();
}

/*==========================================================================*/
/**
 *  Apply image to the background.
 */
/*==========================================================================*/
/*
void Background::apply_image( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

    // Disable OpenGL parameters.
    glDisable( GL_DEPTH_TEST );

    // Enable OpenGL parameters.
    glEnable( GL_TEXTURE_2D );

    // Bind the texture image as mipmap texture.
    m_mipmap.bind();

    // Texture mapping onto the background.
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    {
        glLoadIdentity();

        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho( 0, 1, 0, 1, -1, 1 );

            // Texture mapping onto the background.
            glBegin( GL_QUADS );
            glTexCoord2f( 0.0, 0.0 ); glVertex2f(  1.0,  1.0 );
            glTexCoord2f( 0.0, 1.0 ); glVertex2f(  1.0,  0.0 );
            glTexCoord2f( 1.0, 1.0 ); glVertex2f(  0.0,  0.0 );
            glTexCoord2f( 1.0, 0.0 ); glVertex2f(  0.0,  1.0 );
            glEnd();
        }
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    glPopMatrix();

    glClearDepth(1000);
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_TEXTURE_2D );

    glPopAttrib();
}
*/

} // end of namespace kvs

