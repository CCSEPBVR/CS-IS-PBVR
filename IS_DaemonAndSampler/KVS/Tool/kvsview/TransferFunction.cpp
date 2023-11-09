/*****************************************************************************/
/**
 *  @file   TransferFunction.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunction.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TransferFunction.h"
#include <kvs/DebugNew>
#include <kvs/TransferFunction>
#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/InitializeEventListener>
#include <kvs/PaintEventListener>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include "CommandName.h"


namespace kvsview
{

namespace TransferFunction
{

/*===========================================================================*/
/**
 *  @brief  Parameter set.
 */
/*===========================================================================*/
struct Parameters
{
    kvs::TransferFunction transfer_function; ///< transfer function
    kvs::Texture1D        color_map_texture; ///< color map texture
    kvs::Texture2D        checkerboard_texture; ///< checkerboard texture
    bool                  has_color_map_option; ///< check flag for color map
    bool                  has_opacity_map_option; ///< check flag for opacity map

    Parameters( Argument& arg )
    {
        has_color_map_option = arg.hasColorMapOption();
        has_opacity_map_option = arg.hasOpacityMapOption();
    }

    void initializeColorMapTexture( void )
    {
        const size_t nchannels  = 4; // rgba
        const size_t width = transfer_function.colorMap().resolution();
        const kvs::UInt8* color_map = transfer_function.colorMap().table().pointer();
        const kvs::Real32* opacity_map = transfer_function.opacityMap().table().pointer();

        GLubyte* data = new GLubyte [ width * nchannels ];
        if ( !data )
        {
            kvsMessageError("Cannot allocate for the color map texture.");
            return;
        }

        for ( size_t i = 0, i3 = 0; i < width; i++, i3 += 3 )
        {
            *(data++) = static_cast<GLubyte>(color_map[i3]);
            *(data++) = static_cast<GLubyte>(color_map[i3+1]);
            *(data++) = static_cast<GLubyte>(color_map[i3+2]);
            *(data++) = static_cast<GLubyte>(int(opacity_map[i] * 255.0f + 0.5));
        }
        data -= width * nchannels;

        color_map_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
        color_map_texture.setMinFilter( GL_LINEAR );
        color_map_texture.setMagFilter( GL_LINEAR );
        color_map_texture.create( width );
        color_map_texture.download( width, data );

        delete [] data;
    }

    void initializeCheckerboardTexture( void )
    {
        const size_t nchannels = 3;
        const int width = 32;
        const int height = 32;

        GLubyte* data = new GLubyte [ width * height * nchannels ];
        if ( !data )
        {
            kvsMessageError("Cannot allocate for the checkerboard texture.");
            return;
        }

        const int c1 = 255; // checkerboard color (gray value) 1
        const int c2 = 230; // checkerboard color (gray value) 2
        for ( int i = 0; i < height; i++ )
        {
            for ( int j = 0; j < width; j++ )
            {
                int c = ((((i&0x8)==0)^((j&0x8)==0))) * c1;
                c = ( c == 0 ) ? c2 : c;
                *(data++) = static_cast<GLubyte>(c);
                *(data++) = static_cast<GLubyte>(c);
                *(data++) = static_cast<GLubyte>(c);
            }
        }
        data -= width * height * nchannels;

        checkerboard_texture.setPixelFormat( nchannels, sizeof( kvs::UInt8 ) );
        checkerboard_texture.setMinFilter( GL_NEAREST );
        checkerboard_texture.setMagFilter( GL_NEAREST );
        checkerboard_texture.setWrapS( GL_REPEAT );
        checkerboard_texture.setWrapT( GL_REPEAT );
        checkerboard_texture.create( width, height );
        checkerboard_texture.download( width, height, data );

        delete [] data;
    }
};

/*===========================================================================*/
/**
 *  @brief  Initialize event.
 */
/*===========================================================================*/
class InitializeEvent : public kvs::InitializeEventListener
{
private:

    kvsview::TransferFunction::Parameters* m_parameters;

public:

    InitializeEvent( kvsview::TransferFunction::Parameters* parameters ):
        m_parameters( parameters ) {}

    void update( void )
    {
        m_parameters->initializeColorMapTexture();
        m_parameters->initializeCheckerboardTexture();
    }
};

/*===========================================================================*/
/**
 *  @brief  Paint event.
 */
/*===========================================================================*/
class PaintEvent : public kvs::PaintEventListener
{
private:

    kvsview::TransferFunction::Parameters* m_parameters;

public:

    PaintEvent( kvsview::TransferFunction::Parameters* parameters ):
        m_parameters( parameters ) {}

    void update( void )
    {
        const kvs::RGBColor white( 255, 255, 255 );
        screen()->background()->setColor( white );

        int vp[4]; glGetIntegerv( GL_VIEWPORT, (GLint*)vp );
        const int left = vp[0];
        const int bottom = vp[1];
        const int right = vp[2];
        const int top = vp[3];

        glPushAttrib( GL_CURRENT_BIT | GL_ENABLE_BIT );

        glDisable( GL_LIGHTING );
        glDisable( GL_DEPTH_TEST );
        glEnable( GL_BLEND );

        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        {
            glLoadIdentity();

            glMatrixMode( GL_PROJECTION );
            glPushMatrix();
            {
                glLoadIdentity();

                const float front = 0.0f;
                const float back = 2000.0f;
                glOrtho( left, right, bottom, top, front, back );

                const bool has_color_map_option = m_parameters->has_color_map_option;
                const bool has_opacity_map_option = m_parameters->has_opacity_map_option;
                if ( has_color_map_option && !has_opacity_map_option )
                {
                    // Draw the color map witout opacity.
                    this->draw_color_map_texture( GL_ONE, GL_ZERO );
                }
                else if ( !has_color_map_option && has_opacity_map_option )
                {
                    // Draw the opacity map on the checkerboard.
                    this->draw_checkerboard_texture( GL_ONE, GL_ZERO );
                    this->draw_color_map_texture( GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
                }
                else
                {
                    // Draw the color map with the opacity on the checkerboard.
                    this->draw_checkerboard_texture( GL_ONE, GL_ZERO );
                    this->draw_color_map_texture( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                }
            }
            glPopMatrix();
            glMatrixMode( GL_MODELVIEW );
        }
        glPopMatrix();

        glPopAttrib();
    }

private:

    void draw_checkerboard_texture( const GLenum src_factor, const GLenum dst_factor )
    {
        // Since the checkerboard is background, GL_ONE and GL_ZERO are always specified
        // for src_factor and dst_factor, rescpectively.

        glDisable( GL_TEXTURE_1D );
        glEnable( GL_TEXTURE_2D );
        glBlendFunc( src_factor, dst_factor );
        m_parameters->checkerboard_texture.bind();
        {
            const float scale = 0.6f;
            const float texture_width = ( screen()->width() / 32.0f ) * scale;
            const float texture_height = ( screen()->height() / 32.0f ) * scale;
            this->draw_texture( texture_width, texture_height );
        }
        m_parameters->checkerboard_texture.unbind();
    }

    void draw_color_map_texture( const GLenum src_factor, const GLenum dst_factor )
    {
        glEnable( GL_TEXTURE_1D );
        glDisable( GL_TEXTURE_2D );
        glBlendFunc( src_factor, dst_factor );
        m_parameters->color_map_texture.bind();
        {
            const float texture_width = 1.0f;
            const float texture_height = 1.0f;
            this->draw_texture( texture_width, texture_height );
        }
        m_parameters->color_map_texture.unbind();
    }

    void draw_texture( const float texture_width, const float texture_height )
    {
        const GLfloat x = 0;
        const GLfloat y = 0;
        const GLfloat width = static_cast<GLfloat>(screen()->width());
        const GLfloat height = static_cast<GLfloat>(screen()->height());

        glBegin( GL_QUADS );
        {
            glTexCoord2f( 0.0f,          0.0f );           glVertex2f( x,         y );
            glTexCoord2f( texture_width, 0.0f );           glVertex2f( x + width, y );
            glTexCoord2f( texture_width, texture_height ); glVertex2f( x + width, y + height );
            glTexCoord2f( 0.0f,          texture_height ); glVertex2f( x,         y + height );
        }
        glEnd();
    }
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument class for a transfer function renderer.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvsview::Argument::Common( argc, argv, kvsview::TransferFunction::CommandName )
{
    // Parameters for the transfer function renderer class.
    add_option( kvsview::TransferFunction::CommandName, kvsview::TransferFunction::Description, 0 );
    add_option("c", "Display color map. (optional)", 0, false );
    add_option("a", "Display opacity (alpha) map. (optional)", 0, false );
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the color map option is specified or not.
 *  @return true, if the color map option is specified
 */
/*===========================================================================*/
const bool Argument::hasColorMapOption( void )
{
    return( this->hasOption("c") );
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the opacity map option is specified or not.
 *  @return true, if the opacity map option is specified
 */
/*===========================================================================*/
const bool Argument::hasOpacityMapOption( void )
{
    return( this->hasOption("a") );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Main class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Main::Main( int argc, char** argv )
{
    m_argc = argc;
    m_argv = argv;
}

/*===========================================================================*/
/**
 *  @brief  Executes main process.
 */
/*===========================================================================*/
const bool Main::exec( void )
{
    // Setup GLUT viewer application.
    kvs::glut::Application app( m_argc, m_argv );

    // Commandline arguments.
    kvsview::TransferFunction::Argument arg( m_argc, m_argv );
    if ( !arg.parse() ) exit( EXIT_FAILURE );
    m_input_name = arg.value<std::string>();

    // Parameters.
    kvsview::TransferFunction::Parameters params( arg );
    if ( !params.transfer_function.read( m_input_name ) ) exit( EXIT_FAILURE );

    // Verbose information.
    if ( arg.verboseMode() )
    {
        std::cout << "IMPORTED OBJECT" << std::endl;
        std::cout << "resolution: " << params.transfer_function.resolution() << std::endl;
    }

    // Events.
    kvsview::TransferFunction::InitializeEvent initialize_event( &params );
    kvsview::TransferFunction::PaintEvent paint_event( &params );

    // Create and show the rendering screen.
    kvs::glut::Screen screen( &app );
    screen.addInitializeEvent( &initialize_event );
    screen.addPaintEvent( &paint_event );
    screen.setGeometry( 0, 0, 512, 150 );
    screen.setTitle( kvsview::CommandName + " - " + kvsview::TransferFunction::CommandName );
    screen.show();

    return( arg.clear(), app.run() );
}

} // end of namespace TransferFunction

} // end of namespace kvsview
