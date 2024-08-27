/*****************************************************************************/
/**
 *  @file   Argument.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Argument.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Argument.h"
#include <kvs/DebugNew>
#include <kvs/Bounds>
#include <kvs/AxisObject>
#include <kvs/PointRenderer>
#include <kvs/LineRenderer>
#include <kvs/PolygonRenderer>
#include <kvs/glut/AxisRenderer>
#include "CommandName.h"
#include "PointRenderer.h"
#include "LineRenderer.h"
#include "PolygonRenderer.h"
#include "Isosurface.h"
#include "SlicePlane.h"
#include "OrthoSlice.h"
#include "TransferFunction.h"
#include "ExtractEdges.h"
#include "ExtractVertices.h"
#include "ExternalFaces.h"
#include "RayCastingRenderer.h"
#include "ParticleVolumeRenderer.h"
#include "Histogram.h"


namespace kvsview
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument class for kvsview.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvs::CommandLine( argc, argv, kvsview::CommandName )
{
    add_help_option();

    add_option( "help", "Output help message for each process. (ex. -help Isosurface)", 1 );
    add_option( kvsview::PointRenderer::CommandName, kvsview::PointRenderer::Description, 0 );
    add_option( kvsview::LineRenderer::CommandName, kvsview::LineRenderer::Description, 0 );
    add_option( kvsview::PolygonRenderer::CommandName, kvsview::PolygonRenderer::Description, 0 );
    add_option( kvsview::Isosurface::CommandName, kvsview::Isosurface::Description, 0 );
    add_option( kvsview::SlicePlane::CommandName, kvsview::SlicePlane::Description, 0 );
    add_option( kvsview::OrthoSlice::CommandName, kvsview::OrthoSlice::Description, 0 );
    add_option( kvsview::TransferFunction::CommandName, kvsview::TransferFunction::Description, 0 );
    add_option( kvsview::ExtractEdges::CommandName, kvsview::ExtractEdges::Description, 0 );
    add_option( kvsview::ExtractVertices::CommandName, kvsview::ExtractVertices::Description, 0 );
    add_option( kvsview::ExternalFaces::CommandName, kvsview::ExternalFaces::Description, 0 );
    add_option( kvsview::RayCastingRenderer::CommandName, kvsview::RayCastingRenderer::Description, 0 );
    add_option( kvsview::ParticleVolumeRenderer::CommandName, kvsview::ParticleVolumeRenderer::Description, 0 );
    add_option( kvsview::Histogram::CommandName, kvsview::Histogram::Description, 0 );

    add_value( "input data file", false );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument::Common class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Common::Common( int argc, char** argv ):
    kvs::CommandLine( argc, argv, kvsview::CommandName )
{
    this->set_options();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument::Common class.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 *  @param  command [in] command name
 */
/*===========================================================================*/
Argument::Common::Common( int argc, char** argv, const std::string& command ):
    kvs::CommandLine( argc, argv, kvsview::CommandName + " -" + command )
{
    this->set_options();
}

/*===========================================================================*/
/**
 *  @brief  Sets common options.
 */
/*===========================================================================*/
void Argument::Common::set_options( void )
{
    add_help_option("help");

    add_option("antialiasing", "Enable anti-aliasing. (optional)", 0, false );
    add_option("axis", "Show axis. (optional)", 0, false );
    add_option("axis_color", "Set axis color. (default: 0 0 0)", 3, false );
    add_option("axis_tag_color", "Set axis tag color. (default: 0 0 0)", 3, false );
    add_option("axis_line_width", "Set axis line width. (default: 2)", 1, false );
    add_option("axis_subline_width", "Set axis subline width. (default: 1)", 1, false );
    add_option("axis_x_tag", "Set tag for the x-axis. (default: X)", 1, false );
    add_option("axis_y_tag", "Set tag for the y-axis. (default: Y)", 1, false );
    add_option("axis_z_tag", "Set tag for the z-axis. (default: Z)", 1, false );
    add_option("axis_sublines", "Set number of sublines for each axis. (default: 5 5 5)", 3, false );
    add_option("background_color", "Set background color. (default: 212 221 229)", 3, false );
    add_option("background_color2", "Set top and bottom side color on the background. (optional: <top: r g b> <bottom: r g b>)", 6, false );
    add_option("bounds", "Show bounding box. (optional)", 0, false );
    add_option("bounds_color", "Set bounding box color. (default: 0 0 0)", 3, false );
    add_option("screen_position", "Set screen position. (default: 0 0)", 2, false );
    add_option("screen_size", "Set screen size. (default: 512 512)", 2, false );
    add_option("output", "Output filename. (optional: <filename>)", 1, false );
    add_option("verbose", "Enable additional output message. (optional)", 0, false);

    add_value("input data file");
}

/*===========================================================================*/
/**
 *  @brief  Checks whether the verbose mode option is specified or not.
 *  @return true, if the verbose mode option is specified
 */
/*===========================================================================*/
bool Argument::Common::verboseMode( void )
{
    return( this->hasOption("verbose") );
}

/*===========================================================================*/
/**
 *  @brief  Applys the specified options to the screen information.
 *  @param  screen [in] screen
 */
/*===========================================================================*/
void Argument::Common::applyTo( kvs::glut::Screen& screen )
{
    int x = 0;
    int y = 0;
    if ( this->hasOption("screen_position") )
    {
        x = this->optionValue<int>("screen_position",0);
        y = this->optionValue<int>("screen_position",1);
    }

    int width = 512;
    int height = 512;
    if ( this->hasOption("screen_size") )
    {
        width  = this->optionValue<int>("screen_size",0);
        height = this->optionValue<int>("screen_size",1);
    }

    screen.setPosition( x, y );
    screen.setSize( width, height );
}

/*===========================================================================*/
/**
 *  @brief  Applys the specified options to the screen information.
 *  @param  screen [in] screen parameters
 *  @param  pipe [in] visualization pipeline
 */
/*===========================================================================*/
void Argument::Common::applyTo( kvs::glut::Screen& screen, kvs::VisualizationPipeline& pipe )
{
    // Axis.
    if ( this->hasOption("axis") )
    {
        kvs::AxisObject* axis = new kvs::AxisObject();

        axis->setLineColor( kvs::RGBColor( 0, 0, 0 ) );
        if ( this->hasOption("axis_color") )
        {
            const kvs::UInt8 r( static_cast<kvs::UInt8>(this->optionValue<int>("axis_color",0)) );
            const kvs::UInt8 g( static_cast<kvs::UInt8>(this->optionValue<int>("axis_color",1)) );
            const kvs::UInt8 b( static_cast<kvs::UInt8>(this->optionValue<int>("axis_color",2)) );
            axis->setLineColor( kvs::RGBColor( r, g, b ) );
        }

        axis->setTagColor( kvs::RGBColor( 0, 0, 0 ) );
        if ( this->hasOption("axis_tag_color") )
        {
            const kvs::UInt8 r( static_cast<kvs::UInt8>(this->optionValue<int>("axis_tag_color",0)) );
            const kvs::UInt8 g( static_cast<kvs::UInt8>(this->optionValue<int>("axis_tag_color",1)) );
            const kvs::UInt8 b( static_cast<kvs::UInt8>(this->optionValue<int>("axis_tag_color",2)) );
            axis->setTagColor( kvs::RGBColor( r, g, b ) );
        }

        axis->setLineWidth( 2.0f );
        if ( this->hasOption("axis_line_width") )
        {
            const kvs::Real32 width( this->optionValue<kvs::Real32>("axis_line_width") );
            axis->setLineWidth( width );
        }

        axis->setSublineWidth( 1.0f );
        if ( this->hasOption("axis_subline_width") )
        {
            const kvs::Real32 width( this->optionValue<kvs::Real32>("axis_subline_width") );
            axis->setSublineWidth( width );
        }

        axis->setXTag("X");
        if ( this->hasOption("axis_x_tag") )
        {
            const std::string tag( this->optionValue<std::string>("axis_x_tag") );
            axis->setXTag( tag );
        }

        axis->setYTag("Y");
        if ( this->hasOption("axis_y_tag") )
        {
            const std::string tag( this->optionValue<std::string>("axis_y_tag") );
            axis->setYTag( tag );
        }

        axis->setZTag("Z");
        if ( this->hasOption("axis_z_tag") )
        {
            const std::string tag( this->optionValue<std::string>("axis_z_tag") );
            axis->setZTag( tag );
        }

        axis->setNSublines( kvs::Vector3ui( 5, 5, 5 ) );
        if ( this->hasOption("axis_sublines") )
        {
            const unsigned int nx( this->optionValue<unsigned int>("axis_sublines",0) );
            const unsigned int ny( this->optionValue<unsigned int>("axis_sublines",1) );
            const unsigned int nz( this->optionValue<unsigned int>("axis_sublines",2) );
            axis->setNSublines( kvs::Vector3ui( nx, ny, nz ) );
        }

        axis->create( pipe.object() );

        kvs::VisualizationPipeline pipeline( axis );
        kvs::PipelineModule renderer( new kvs::glut::AxisRenderer() );
        renderer.get<kvs::glut::AxisRenderer>()->disableShading();

        if ( this->hasOption("antialiasing") )
        {
            renderer.get<kvs::glut::AxisRenderer>()->enableAntiAliasing();
        }

        pipeline.connect( renderer );
        pipeline.exec();

        screen.registerObject( &pipeline );
    }

    // Bounding box.
    if ( this->hasOption("bounds") )
    {
        kvs::LineObject* bounds = new kvs::Bounds( pipe.object() );

        bounds->setColor( kvs::RGBColor( 0, 0, 0 ) );
        if ( this->hasOption("bounds_color") )
        {
            const kvs::UInt8 r( static_cast<kvs::UInt8>(this->optionValue<int>("bounds_color",0)) );
            const kvs::UInt8 g( static_cast<kvs::UInt8>(this->optionValue<int>("bounds_color",1)) );
            const kvs::UInt8 b( static_cast<kvs::UInt8>(this->optionValue<int>("bounds_color",2)) );
            bounds->setColor( kvs::RGBColor( r, g, b ) );
        }

        kvs::VisualizationPipeline pipeline( bounds );
        kvs::PipelineModule renderer( new kvs::LineRenderer() );
        renderer.get<kvs::LineRenderer>()->disableShading();

        if ( this->hasOption("antialiasing") )
        {
            renderer.get<kvs::LineRenderer>()->enableAntiAliasing();
        }

        pipeline.connect( renderer );
        pipeline.exec();

        screen.registerObject( &pipeline );
    }

    // Background color.
    if ( this->hasOption("background_color") )
    {
        const kvs::UInt8 r( static_cast<kvs::UInt8>(this->optionValue<int>("background_color",0)) );
        const kvs::UInt8 g( static_cast<kvs::UInt8>(this->optionValue<int>("background_color",1)) );
        const kvs::UInt8 b( static_cast<kvs::UInt8>(this->optionValue<int>("background_color",2)) );
        screen.background()->setColor( kvs::RGBColor( r, g, b ) );
    }
    else if ( this->hasOption("background_color2") )
    {
        const kvs::UInt8 r1( static_cast<kvs::UInt8>(this->optionValue<int>("background_color2",0)) );
        const kvs::UInt8 g1( static_cast<kvs::UInt8>(this->optionValue<int>("background_color2",1)) );
        const kvs::UInt8 b1( static_cast<kvs::UInt8>(this->optionValue<int>("background_color2",2)) );
        const kvs::RGBColor top_color( r1, g1, b1 );

        const kvs::UInt8 r2( static_cast<kvs::UInt8>(this->optionValue<int>("background_color2",3)) );
        const kvs::UInt8 g2( static_cast<kvs::UInt8>(this->optionValue<int>("background_color2",4)) );
        const kvs::UInt8 b2( static_cast<kvs::UInt8>(this->optionValue<int>("background_color2",5)) );
        const kvs::RGBColor bottom_color( r2, g2, b2 );

        screen.background()->setColor( bottom_color, top_color );
    }

    // Anti-aliasing.
    if ( this->hasOption("antialiasing") )
    {
        // Enable multisampling.
        bool multisample = false;
        if ( multisample )
        {
            kvs::DisplayFormat format( screen.displayFormat() );
            format.setMultisampleBuffer( true );
            screen.setDisplayFormat( format );
        }

        // Apply to the geometry object if the antialiasing option is given.
        if ( pipe.object()->objectType() == kvs::ObjectBase::Geometry )
        {
            switch ( kvs::GeometryObjectBase::DownCast( pipe.object() )->geometryType() )
            {
            case kvs::GeometryObjectBase::Point:
            {
                kvs::RendererBase* renderer = const_cast<kvs::RendererBase*>( pipe.renderer() );
                static_cast<kvs::PointRenderer*>( renderer )->enableAntiAliasing( multisample );
                break;
            }
            case kvs::GeometryObjectBase::Line:
            {
                kvs::RendererBase* renderer = const_cast<kvs::RendererBase*>( pipe.renderer() );
                static_cast<kvs::LineRenderer*>( renderer )->enableAntiAliasing( multisample );
                break;
            }
            case kvs::GeometryObjectBase::Polygon:
            {
                kvs::RendererBase* renderer = const_cast<kvs::RendererBase*>( pipe.renderer() );
                static_cast<kvs::PolygonRenderer*>( renderer )->enableAntiAliasing( multisample );
                break;
            }
            default: break;
            }
        }
    }

/*
    if ( this->hasOption("output") )
    {
        const std::string filename = this->optionValue<std::string>("output");
    }
*/
}

} // end of namespace kvsview
