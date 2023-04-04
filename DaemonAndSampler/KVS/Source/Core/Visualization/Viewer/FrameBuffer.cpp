/****************************************************************************/
/**
 *  @file FrameBuffer.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrameBuffer.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "FrameBuffer.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
FrameBuffer::FrameBuffer( void ):
    m_format( 0 ),
    m_type( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param format [in] pixel data format
 *  @param type [in] pixel data type
 */
/*==========================================================================*/
FrameBuffer::FrameBuffer( const GLenum format, const GLenum type ):
    m_format( format ),
    m_type( type )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
FrameBuffer::~FrameBuffer( void )
{
}

/*==========================================================================*/
/**
 *  Set the pixel data format.
 *  @param format [in] pixel data format
 */
/*==========================================================================*/
void FrameBuffer::setFormat( const GLenum format )
{
    /* The following values are accepted:
     *     GL_COLOR_INDEX
     *     GL_STENCIL_INDEX
     *     GL_DEPTH_COMPONENT
     *     GL_RED
     *     GL_GREEN
     *     GL_BLUE
     *     GL_ALPHA
     *     GL_RGB
     *     GL_RGBA
     *     GL_LUMINANCE
     *     GL_LUMINANCE_ALPHA
     */
    m_format = format;
}

/*==========================================================================*/
/**
 *  Set the pixel data type.
 *  @param type [in] pixel data type
 */
/*==========================================================================*/
void FrameBuffer::setType( const GLenum type )
{
    /* The following values are accepted:
     *     GL_UNSIGNED_BYTE
     *     GL_BYTE
     *     GL_BITMAP
     *     GL_UNSIGNED_SHORT
     *     GL_SHORT
     *     GL_UNSIGNED_INT
     *     GL_INT
     *     GL_FLOAT
     */
    m_type = type;
}

/*==========================================================================*/
/**
 *  Read the pixel data from the frame buffer.
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param  pixels [out] pointer to an address of the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::read(
    const int    width,
    const int    height,
    void**       pixels,
    const GLenum buffer )
{
    this->read( 0, 0, width, height, m_format, m_type, pixels, buffer );
}

/*==========================================================================*/
/**
 *  Read the pixel data form the frame buffer.
 *  @param x [in] x value of the window coordinates
 *  @param y [in] y value of the window coordinates
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param  pixels [out] pointer to an address of the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::read(
    const int    x,
    const int    y,
    const int    width,
    const int    height,
    void**       pixels,
    const GLenum buffer )
{
    this->read( x, y, width, height, m_format, m_type, pixels, buffer );
}

/*==========================================================================*/
/**
 *  Read the pixel data from the frame buffer.
 *  @param x [in] x value of the window coordinates
 *  @param y [in] y value of the window coordinates
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param format [in] pixel data format
 *  @param type [in] pixel data type
 *  @param  pixels [out] pointer to an address of the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::read(
    const int    x,
    const int    y,
    const int    width,
    const int    height,
    const GLenum format,
    const GLenum type,
    void**       pixels,
    const GLenum buffer )
{
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );

    if( buffer != 0 )
    {
        GLint current_buffer;
        glGetIntegerv( GL_READ_BUFFER, &current_buffer );
        glReadBuffer( buffer );
        glReadPixels( x, y, width, height, format, type, *pixels );
        glReadBuffer( current_buffer );
    }
    else
    {
        glReadPixels( x, y, width, height, format, type, *pixels );
    }
}

/*==========================================================================*/
/**
 *  Draw the pixel data to the frame buffer.
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param pixels [in] pointer to the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::draw(
    const int    width,
    const int    height,
    const void*  pixels,
    const GLenum buffer )
{
    int viewport[4]; glGetIntegerv( GL_VIEWPORT, (GLint*)viewport );
    const int x = viewport[0];
    const int y = viewport[1];

    this->draw( x, y, width, height, m_format, m_type, viewport, pixels, buffer );
}

/*==========================================================================*/
/**
 *  Draw the pixel data to the frame buffer.
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param viewport [in] viewport
 *  @param pixels [in] pointer to the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::draw(
    const int    width,
    const int    height,
    const int*   viewport,
    const void*  pixels,
    const GLenum buffer )
{
    const int x = viewport[0];
    const int y = viewport[1];

    this->draw( x, y, width, height, m_format, m_type, viewport, pixels, buffer );
}

/*==========================================================================*/
/**
 *  Draw the pixel data to the frame buffer.
 *  @param x [in] x value of the window coordinates
 *  @param y [in] y value of the window coordinates
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param pixels [in] pointer to the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::draw(
    const int    x,
    const int    y,
    const int    width,
    const int    height,
    const void*  pixels,
    const GLenum buffer )
{
    int viewport[4]; glGetIntegerv( GL_VIEWPORT, (GLint*)viewport );

    this->draw( x, y, width, height, m_format, m_type, viewport, pixels, buffer );
}

/*==========================================================================*/
/**
 *  Draw the pixel data to the frame buffer.
 *  @param x [in] x value of the window coordinates
 *  @param y [in] y value of the window coordinates
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pxiel rectangle
 *  @param viewport [in] viewport
 *  @param pixels [in] pointer to the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::draw(
    const int    x,
    const int    y,
    const int    width,
    const int    height,
    const int*   viewport,
    const void*  pixels,
    const GLenum buffer )
{
    this->draw( x, y, width, height, m_format, m_type, viewport, pixels, buffer );
}

/*==========================================================================*/
/**
 *  Draw the pixel data to the frame buffer.
 *  @param x [in] x value of the window coordinates
 *  @param y [in] y value of the window coordinates
 *  @param width [in] width of the pixel rectangle
 *  @param height [in] height of the pixel rectangle
 *  @param format [in] pixel data format
 *  @param type [in] pixel data type
 *  @param viewport [in] viewport
 *  @param pixels [in] pointer to the pixel data
 *  @param buffer [in] color buffer that is read from the frame buffer
 */
/*==========================================================================*/
void FrameBuffer::draw(
    const int    x,
    const int    y,
    const int    width,
    const int    height,
    const GLenum format,
    const GLenum type,
    const int*   viewport,
    const void*  pixels,
    const GLenum buffer )
{
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );
#if defined(GL_TEXTURE_3D)
    glDisable( GL_TEXTURE_3D );
#endif

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    GLint current_buffer = 0;
    if( buffer != 0 )
    {
        glGetIntegerv( GL_DRAW_BUFFER, &current_buffer );
        glDrawBuffer( buffer );
    }

    const int left   = viewport[0];
    const int bottom = viewport[1];
    const int right  = viewport[2];
    const int top    = viewport[3];

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    {
        glLoadIdentity();

        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        {
            glLoadIdentity();
            glOrtho( left, right, bottom, top, -1, 1 );

            glRasterPos2i( x, y );
            glDrawPixels( width, height, format, type, pixels );
        }
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    glPopMatrix();

    if( buffer != 0 ) glDrawBuffer( current_buffer );
}

} // end of namespace kvs
