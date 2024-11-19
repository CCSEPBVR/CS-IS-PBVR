/*****************************************************************************/
/**
 *  @file   DiamondGlyph.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DiamondGlyph.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "DiamondGlyph.h"
#include <vismodule/OpenGL>


namespace
{

const vismodule::UInt32 Connections[24] =
{
    4, 0, 1,
    4, 1, 2,
    4, 2, 3,
    4, 3, 0,
    5, 1, 0,
    5, 2, 1,
    5, 3, 2,
    5, 0, 3
};

const vismodule::Real32 Vertices[18] =
{
     0.5f,  0.0f,  0.0f,
     0.0f,  0.0f, -0.5f,
    -0.5f,  0.0f,  0.0f,
     0.0f,  0.0f,  0.5f,
     0.0f,  0.5f,  0.0f,
     0.0f, -0.5f,  0.0f
};

const vismodule::Real32 Normals[24] =
{
     0.577f,  0.577f, -0.577f,
    -0.577f,  0.577f, -0.577f,
    -0.577f,  0.577f,  0.577f,
     0.577f,  0.577f,  0.577f,
     0.577f, -0.577f, -0.577f,
    -0.577f, -0.577f, -0.577f,
    -0.577f, -0.577f,  0.577f,
     0.577f, -0.577f,  0.577f
};

} // end of namespace


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new DiamondGlyph class.
 */
/*===========================================================================*/
DiamondGlyph::DiamondGlyph( void ):
    vismodule::GlyphBase()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new DiamondGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
DiamondGlyph::DiamondGlyph( const vismodule::VolumeObjectBase* volume ):
    vismodule::GlyphBase()
{
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new DiamondGlyph class.
 *  @param  volume [in] pointer to the volume object
 *  @param  transfer_function [in] transfer function
 */
/*===========================================================================*/
DiamondGlyph::DiamondGlyph(
    const vismodule::VolumeObjectBase* volume,
    const vismodule::TransferFunction& transfer_function ):
    vismodule::GlyphBase()
{
    BaseClass::setTransferFunction( transfer_function );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Diamond glyph class.
 */
/*===========================================================================*/
DiamondGlyph::~DiamondGlyph( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Creates a diamond glyph.
 *  @param  object [in] pointer to the input object (volume object)
 *  @return pointer to the created glyph object
 */
/*===========================================================================*/
DiamondGlyph::BaseClass::SuperClass* DiamondGlyph::exec( const vismodule::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::VolumeObjectBase* volume = vismodule::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not volume dat.");
        return( NULL );
    }

    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );
    BaseClass::calculate_coords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( vismodule::Int8 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int8>( volume );
        BaseClass::calculate_directions<vismodule::Int8>( volume );
        BaseClass::calculate_colors<vismodule::Int8>( volume );
        BaseClass::calculate_opacities<vismodule::Int8>( volume );
    }
    else if ( type == typeid( vismodule::Int16 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int16>( volume );
        BaseClass::calculate_directions<vismodule::Int16>( volume );
        BaseClass::calculate_colors<vismodule::Int16>( volume );
        BaseClass::calculate_opacities<vismodule::Int16>( volume );
    }
    else if ( type == typeid( vismodule::Int32 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int32>( volume );
        BaseClass::calculate_directions<vismodule::Int32>( volume );
        BaseClass::calculate_colors<vismodule::Int32>( volume );
        BaseClass::calculate_opacities<vismodule::Int32>( volume );
    }
    else if ( type == typeid( vismodule::Int64 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int64>( volume );
        BaseClass::calculate_directions<vismodule::Int64>( volume );
        BaseClass::calculate_colors<vismodule::Int64>( volume );
        BaseClass::calculate_opacities<vismodule::Int64>( volume );
    }
    else if ( type == typeid( vismodule::UInt8  ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt8>( volume );
        BaseClass::calculate_directions<vismodule::UInt8>( volume );
        BaseClass::calculate_colors<vismodule::UInt8>( volume );
        BaseClass::calculate_opacities<vismodule::UInt8>( volume );
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt16>( volume );
        BaseClass::calculate_directions<vismodule::UInt16>( volume );
        BaseClass::calculate_colors<vismodule::UInt16>( volume );
        BaseClass::calculate_opacities<vismodule::UInt16>( volume );
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt32>( volume );
        BaseClass::calculate_directions<vismodule::UInt32>( volume );
        BaseClass::calculate_colors<vismodule::UInt32>( volume );
        BaseClass::calculate_opacities<vismodule::UInt32>( volume );
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt64>( volume );
        BaseClass::calculate_directions<vismodule::UInt64>( volume );
        BaseClass::calculate_colors<vismodule::UInt64>( volume );
        BaseClass::calculate_opacities<vismodule::UInt64>( volume );
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        BaseClass::calculate_sizes<vismodule::Real32>( volume );
        BaseClass::calculate_directions<vismodule::Real32>( volume );
        BaseClass::calculate_colors<vismodule::Real32>( volume );
        BaseClass::calculate_opacities<vismodule::Real32>( volume );
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        BaseClass::calculate_sizes<vismodule::Real64>( volume );
        BaseClass::calculate_directions<vismodule::Real64>( volume );
        BaseClass::calculate_colors<vismodule::Real64>( volume );
        BaseClass::calculate_opacities<vismodule::Real64>( volume );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Draw the diamond glyph.
 */
/*===========================================================================*/
void DiamondGlyph::draw( void )
{
    this->initialize();
    BaseClass::applyMaterial();

    const size_t npoints = BaseClass::m_coords.size() / 3;

    if ( BaseClass::m_directions.size() == 0 )
    {
        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const vismodule::Vector3f position( BaseClass::m_coords.pointer() + index );
            const vismodule::Real32 size = BaseClass::m_sizes[i];
            const vismodule::RGBColor color( BaseClass::m_colors.pointer() + index );
            const vismodule::UInt8 opacity = BaseClass::m_opacities[i];
            glPushMatrix();
            {
                BaseClass::transform( position, size );
                this->draw_element( color, opacity );
            }
            glPopMatrix();
        }
    }
    else
    {
        for( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const vismodule::Vector3f position( BaseClass::m_coords.pointer() + index );
            const vismodule::Vector3f direction( BaseClass::m_directions.pointer() + index );
            const vismodule::Real32 size = BaseClass::m_sizes[i];
            const vismodule::RGBColor color( BaseClass::m_colors.pointer() + index );
            const vismodule::UInt8 opacity = BaseClass::m_opacities[i];
            glPushMatrix();
            {
                BaseClass::transform( position, direction, size );
                this->draw_element( color, opacity );
            }
            glPopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Draw the glyph element.
 *  @param  color [in] color value
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void DiamondGlyph::draw_element( const vismodule::RGBColor& color, const vismodule::UInt8 opacity )
{
    glColor4ub( color.r(), color.g(), color.b(), opacity );

    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0, index = 0; i < 8; i++, index += 3 )
        {
            const vismodule::UInt32 offset0 = ::Connections[index] * 3;
            const vismodule::UInt32 offset1 = ::Connections[index+1] * 3;
            const vismodule::UInt32 offset2 = ::Connections[index+2] * 3;
            glNormal3fv( ::Normals + offset0 );
            glVertex3fv( ::Vertices + offset0 );
            glVertex3fv( ::Vertices + offset1 );
            glVertex3fv( ::Vertices + offset2 );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Initialize the modelview matrix.
 */
/*===========================================================================*/
void DiamondGlyph::initialize( void )
{
    glDisable( GL_LINE_SMOOTH );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glShadeModel( GL_SMOOTH );

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
}

} // end of namespace vismodule
