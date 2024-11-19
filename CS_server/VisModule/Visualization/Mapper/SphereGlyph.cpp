/*****************************************************************************/
/**
 *  @file   SphereGlyph.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SphereGlyph.cpp 875 2011-07-28 13:56:01Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "SphereGlyph.h"
#include <vismodule/OpenGL>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph( void ):
    vismodule::GlyphBase(),
    m_sphere(NULL),
    m_nslices(10),
    m_nstacks(10)
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph( const vismodule::PointObject* point ):
    vismodule::GlyphBase(),
    m_sphere(NULL),
    m_nslices(10),
    m_nstacks(10)
{
    this->exec( point );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph( const vismodule::VolumeObjectBase* volume ):
    vismodule::GlyphBase(),
    m_sphere(NULL),
    m_nslices(10),
    m_nstacks(10)
{
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 *  @param  transfer_function [in] transfer function
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph(
    const vismodule::VolumeObjectBase* volume,
    const vismodule::TransferFunction& transfer_function ):
    vismodule::GlyphBase(),
    m_sphere(NULL),
    m_nslices(10),
    m_nstacks(10)
{
    BaseClass::setTransferFunction( transfer_function );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Sphere glyph class.
 */
/*===========================================================================*/
SphereGlyph::~SphereGlyph( void )
{
    gluDeleteQuadric( m_sphere );
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of slices (subdivisions) of the sphere.
 *  @param  nslices [in] number of slices.
 */
/*===========================================================================*/
void SphereGlyph::setNSlices( const size_t nslices )
{
    m_nslices = nslices;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of stacks (subdivisions) of the sphere.
 *  @param  nstacks [in] number of stacks.
 */
/*===========================================================================*/
void SphereGlyph::setNStacks( const size_t nstacks )
{
    m_nstacks = nstacks;
}

/*===========================================================================*/
/**
 *  @brief  Creates a sphere glyph.
 *  @param  object [in] pointer to the input object (volume object)
 *  @return pointer to the created glyph object
 */
/*===========================================================================*/
SphereGlyph::BaseClass::SuperClass* SphereGlyph::exec( const vismodule::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    if ( object->objectType() == vismodule::ObjectBase::Geometry )
    {
        const vismodule::GeometryObjectBase* geometry = vismodule::GeometryObjectBase::DownCast( object );
        if ( geometry->geometryType() == vismodule::GeometryObjectBase::Point )
        {
            const vismodule::PointObject* point = vismodule::PointObject::DownCast( geometry );
            return( this->exec_point_object( point ) );
        }
    }
    else if ( object->objectType() == vismodule::ObjectBase::Volume )
    {
        const vismodule::VolumeObjectBase* volume = vismodule::VolumeObjectBase::DownCast( object );
        return( this->exec_volume_object( volume ) );
    }

    BaseClass::m_is_success = false;
    visModuleMessageError("Unsupported object.");

    return( NULL );
}

/*===========================================================================*/
/**
 *  @brief  Draw the sphere glyph.
 */
/*===========================================================================*/
void SphereGlyph::draw( void )
{
    if ( !m_sphere )
    {
        m_sphere = gluNewQuadric();
        gluQuadricDrawStyle( m_sphere, GLU_FILL );
        gluQuadricNormals( m_sphere, GLU_SMOOTH );
    }

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
 *  @brief  Creates the glyph for the point object.
 *  @param  point [in] pointer to the point object
 *  @return glyph object
 */
/*===========================================================================*/
SphereGlyph::BaseClass::SuperClass* SphereGlyph::exec_point_object( const vismodule::PointObject* point )
{
    const size_t nvertices = point->nvertices();

//    BaseClass::set_min_max_coords( point, this );
    {
        vismodule::Vector3f min_coord( 0.0f );
        vismodule::Vector3f max_coord( 0.0f );

        const vismodule::Real32* coord = point->coords().pointer();
        const vismodule::Real32* const end = coord + point->coords().size();

        vismodule::Real32 x = *( coord++ );
        vismodule::Real32 y = *( coord++ );
        vismodule::Real32 z = *( coord++ );

        min_coord.set( x, y, z );
        max_coord.set( x, y, z );

        while ( coord < end )
        {
            x = *( coord++ );
            y = *( coord++ );
            z = *( coord++ );

            min_coord.x() = vismodule::Math::Min( min_coord.x(), x );
            min_coord.y() = vismodule::Math::Min( min_coord.y(), y );
            min_coord.z() = vismodule::Math::Min( min_coord.z(), z );

            max_coord.x() = vismodule::Math::Max( max_coord.x(), x );
            max_coord.y() = vismodule::Math::Max( max_coord.y(), y );
            max_coord.z() = vismodule::Math::Max( max_coord.z(), z );
        }

        BaseClass::setMinMaxObjectCoords( min_coord, max_coord );

        if ( !( point->hasMinMaxExternalCoords() ) )
        {
            BaseClass::setMinMaxExternalCoords(
                point->minObjectCoord(),
                point->maxObjectCoord() );
        }
        else
        {
            BaseClass::setMinMaxExternalCoords(
                point->minExternalCoord(),
                point->maxExternalCoord() );
        }
    }

    BaseClass::setCoords( point->coords() );

    if ( BaseClass::m_direction_mode == BaseClass::DirectionByNormal )
    {
        if ( point->nnormals() != 0 )
        {
            BaseClass::setDirections( point->normals() );
        }
    }

    if ( point->nsizes() == 1 )
    {
        const vismodule::Real32 size = point->size();
        vismodule::ValueArray<vismodule::Real32> sizes( nvertices );
        for ( size_t i = 0; i < nvertices; i++ ) sizes[i] = size;
        BaseClass::setSizes( sizes );
    }
    else
    {
        BaseClass::setSizes( point->sizes() );
    }

    if ( point->ncolors() == 1 )
    {
        const vismodule::RGBColor color = point->color();
        vismodule::ValueArray<vismodule::UInt8> colors( nvertices * 3 );
        for ( size_t i = 0, j = 0; i < nvertices; i++, j += 3 )
        {
            colors[j]   = color.r();
            colors[j+1] = color.r();
            colors[j+2] = color.r();
        }
        BaseClass::setColors( colors );
    }
    else
    {
        BaseClass::setColors( point->colors() );
    }

    const vismodule::UInt8 opacity = static_cast<vismodule::UInt8>( 255 );
    vismodule::ValueArray<vismodule::UInt8> opacities( nvertices );
    for ( size_t i = 0; i < nvertices; i++ ) opacities[i] = opacity;
    BaseClass::setOpacities( opacities );

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Creates the glyph for the volume object.
 *  @param  volume [in] pointer to the volume object
 *  @return glyph object
 */
/*===========================================================================*/
SphereGlyph::BaseClass::SuperClass* SphereGlyph::exec_volume_object( const vismodule::VolumeObjectBase* volume )
{
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );
    BaseClass::calculate_coords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( vismodule::Int8 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int8>( volume );
        BaseClass::calculate_colors<vismodule::Int8>( volume );
        BaseClass::calculate_opacities<vismodule::Int8>( volume );
    }
    else if ( type == typeid( vismodule::Int16 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int16>( volume );
        BaseClass::calculate_colors<vismodule::Int16>( volume );
        BaseClass::calculate_opacities<vismodule::Int16>( volume );
    }
    else if ( type == typeid( vismodule::Int32 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int32>( volume );
        BaseClass::calculate_colors<vismodule::Int32>( volume );
        BaseClass::calculate_opacities<vismodule::Int32>( volume );
    }
    else if ( type == typeid( vismodule::Int64 ) )
    {
        BaseClass::calculate_sizes<vismodule::Int64>( volume );
        BaseClass::calculate_colors<vismodule::Int64>( volume );
        BaseClass::calculate_opacities<vismodule::Int64>( volume );
    }
    else if ( type == typeid( vismodule::UInt8  ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt8>( volume );
        BaseClass::calculate_colors<vismodule::UInt8>( volume );
        BaseClass::calculate_opacities<vismodule::UInt8>( volume );
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt16>( volume );
        BaseClass::calculate_colors<vismodule::UInt16>( volume );
        BaseClass::calculate_opacities<vismodule::UInt16>( volume );
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt32>( volume );
        BaseClass::calculate_colors<vismodule::UInt32>( volume );
        BaseClass::calculate_opacities<vismodule::UInt32>( volume );
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
        BaseClass::calculate_sizes<vismodule::UInt64>( volume );
        BaseClass::calculate_colors<vismodule::UInt64>( volume );
        BaseClass::calculate_opacities<vismodule::UInt64>( volume );
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        BaseClass::calculate_sizes<vismodule::Real32>( volume );
        BaseClass::calculate_colors<vismodule::Real32>( volume );
        BaseClass::calculate_opacities<vismodule::Real32>( volume );
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        BaseClass::calculate_sizes<vismodule::Real64>( volume );
        BaseClass::calculate_colors<vismodule::Real64>( volume );
        BaseClass::calculate_opacities<vismodule::Real64>( volume );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Draw the glyph element.
 *  @param  color [in] color value
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void SphereGlyph::draw_element( const vismodule::RGBColor& color, const vismodule::UInt8 opacity )
{
    glColor4ub( color.r(), color.g(), color.b(), opacity );

    const GLdouble radius = 0.5;
    gluSphere( m_sphere, radius, static_cast<GLint>(m_nslices), static_cast<GLint>(m_nstacks) );
}

/*===========================================================================*/
/**
 *  @brief  Initialize the modelview matrix.
 */
/*===========================================================================*/
void SphereGlyph::initialize( void )
{
    glDisable( GL_LINE_SMOOTH );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glShadeModel( GL_SMOOTH );

    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );
}

} // end of namespace vismodule
