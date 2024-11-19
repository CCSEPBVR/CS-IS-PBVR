/*****************************************************************************/
/**
 *  @file   GeometryObjectBase.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GeometryObjectBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "GeometryObjectBase.h"
#include <vismodule/Assert>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 *  @param  coords [in] coordinate value array
 *  @param  colors [in] color value array
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::ValueArray<vismodule::Real32>& normals )
{
    VIS_MODULE_ASSERT( coords.size() == normals.size() );
    VIS_MODULE_ASSERT( coords.size() == colors.size() );

    this->setCoords( coords );
    this->setColors( colors );
    this->setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 *  @param  coords [in] coordinate value array
 *  @param  color [in] color value
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::ValueArray<vismodule::Real32>& normals )
{
    VIS_MODULE_ASSERT( coords.size() == normals.size() );

    this->setCoords( coords );
    this->setColor( color );
    this->setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 *  @param  coords [in] coordinate value array
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::Real32>& normals )
{
    VIS_MODULE_ASSERT( coords.size() == normals.size() );

    this->setCoords( coords );
    this->setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 *  @param  coords [in] coordinate value array
 *  @param  colors [in] color value array
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors )
{
    VIS_MODULE_ASSERT( coords.size() == colors.size() );

    this->setCoords( coords );
    this->setColors( colors );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 *  @param  coords [in] coordinate value array
 *  @param  color [in] color value
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color )
{
    this->setCoords( coords );
    this->setColor( color );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GeometryObjectBase class.
 *  @param  coords [in] coordinate value array
 */
/*===========================================================================*/
GeometryObjectBase::GeometryObjectBase(
    const vismodule::ValueArray<vismodule::Real32>& coords )
{
    this->setCoords( coords );
    this->setColor( vismodule::RGBColor( 255, 255, 255 ) );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the GeometryObjectBase class.
 */
/*===========================================================================*/
GeometryObjectBase::~GeometryObjectBase( void )
{
    this->clear();
}

/*===========================================================================*/
/**
 *  @brief  Downcast to GeometryObjectBase.
 *  @param  object [in] pointer to the object base
 *  @return pointer to the geometry object base
 */
/*===========================================================================*/
vismodule::GeometryObjectBase* GeometryObjectBase::DownCast( vismodule::ObjectBase* object )
{
    const vismodule::ObjectBase::ObjectType type = object->objectType();
    if ( type != vismodule::ObjectBase::Geometry )
    {
        visModuleMessageError("Input object is not a geometry object.");
        return( NULL );
    }

    vismodule::GeometryObjectBase* geometry = static_cast<vismodule::GeometryObjectBase*>( object );

    return( geometry );
}

/*===========================================================================*/
/**
 *  @brief  Downcast to GeometryObjectBase with 'const'.
 *  @param  object [in] pointer to the object base
 *  @return pointer to the geometry object base
 */
/*===========================================================================*/
const vismodule::GeometryObjectBase* GeometryObjectBase::DownCast( const vismodule::ObjectBase* object )
{
    return( GeometryObjectBase::DownCast( const_cast<vismodule::ObjectBase*>( object ) ) );
}

/*===========================================================================*/
/**
 *  @brief  '<<' operator.
 */
/*===========================================================================*/
std::ostream& operator << ( std::ostream& os, const vismodule::GeometryObjectBase& object )
{
#ifdef VIS_MODULE_COMPILER_VC
#if VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
    os << static_cast<const vismodule::ObjectBase&>( object ) << std::endl;
#endif
    os << "Number of vertices:  " << object.nvertices() << std::endl;
    os << "Number of colors:  " << object.ncolors() << std::endl;
    os << "Number of normal vectors:  " << object.nnormals();

    return( os );
}

/*===========================================================================*/
/**
 *  @brief  Shallow copy.
 *  @param  object [in] object
 */
/*===========================================================================*/
void GeometryObjectBase::shallowCopy( const GeometryObjectBase& object )
{
    BaseClass::operator=( object );
    m_coords.shallowCopy( object.coords() );
    m_colors.shallowCopy( object.colors() );
    m_normals.shallowCopy( object.normals() );
}

/*===========================================================================*/
/**
 *  @brief  Deep copy.
 *  @param  object [in] object
 */
/*===========================================================================*/
void GeometryObjectBase::deepCopy( const GeometryObjectBase& object )
{
    BaseClass::operator=( object );
    m_coords.deepCopy( object.coords() );
    m_colors.deepCopy( object.colors() );
    m_normals.deepCopy( object.normals() );
}

/*===========================================================================*/
/**
 *  @brief  Clears the object.
 */
/*===========================================================================*/
void GeometryObjectBase::clear( void )
{
    m_coords.deallocate();
    m_colors.deallocate();
    m_normals.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Sets a coordinate value array.
 *  @param  coords [in] coordinate value array
 */
/*===========================================================================*/
void GeometryObjectBase::setCoords( const vismodule::ValueArray<vismodule::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color value array.
 *  @param  colors [in] color value array
 */
/*===========================================================================*/
void GeometryObjectBase::setColors( const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    m_colors = colors;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color value.
 *  @param  color [in] color value
 */
/*===========================================================================*/
void GeometryObjectBase::setColor( const vismodule::RGBColor& color )
{
    m_colors.allocate( 3 );
    m_colors[0] = color.r();
    m_colors[1] = color.g();
    m_colors[2] = color.b();
}

/*===========================================================================*/
/**
 *  @brief  Sets a normal vector array.
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
void GeometryObjectBase::setNormals( const vismodule::ValueArray<vismodule::Real32>& normals )
{
    m_normals = normals;
}

/*===========================================================================*/
/**
 *  @brief  Returns the object type.
 *  @return object type
 */
/*===========================================================================*/
const vismodule::ObjectBase::ObjectType GeometryObjectBase::objectType( void ) const
{
    return( vismodule::ObjectBase::Geometry );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of the vertices.
 *  @return number of the vertices
 */
/*===========================================================================*/
const size_t GeometryObjectBase::nvertices( void ) const
{
    const size_t dimension = 3;
    return( m_coords.size() / dimension );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of the colors.
 *  @return number of the colors
 */
/*===========================================================================*/
const size_t GeometryObjectBase::ncolors( void ) const
{
    const size_t nchannels = 3;
    return( m_colors.size() / nchannels );
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of the normal vectors.
 *  @return number of the normal vectors
 */
/*===========================================================================*/
const size_t GeometryObjectBase::nnormals( void ) const
{
    const size_t dimension = 3;
    return( m_normals.size() / dimension );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value.
 *  @param  index [in] index of the coordinate value
 *  @return coordinate value
 */
/*===========================================================================*/
const vismodule::Vector3f GeometryObjectBase::coord( const size_t index ) const
{
    const size_t dimension = 3;
    return( vismodule::Vector3f( m_coords.pointer() + dimension * index ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value.
 *  @param  index [in] index of the color value
 *  @return color value
 */
/*===========================================================================*/
const vismodule::RGBColor GeometryObjectBase::color( const size_t index ) const
{
    const size_t nchannels = 3;
    return( vismodule::RGBColor( m_colors.pointer() + nchannels * index ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normal vector.
 *  @param  index [in] index of the normal vector
 *  @return normal vector
 */
/*===========================================================================*/
const vismodule::Vector3f GeometryObjectBase::normal( const size_t index ) const
{
    const size_t dimension = 3;
    return( vismodule::Vector3f( m_normals.pointer() + dimension * index ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value array.
 *  @return coordinate value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& GeometryObjectBase::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value array.
 *  @return color value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::UInt8>& GeometryObjectBase::colors( void ) const
{
    return( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normal vector array.
 *  @return normal vector array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& GeometryObjectBase::normals( void ) const
{
    return( m_normals );
}
/*==========================================================================*/
/**
 *  @brief  Updates the min/max coordinates.
 */
/*==========================================================================*/
void GeometryObjectBase::updateMinMaxCoords( void )
{
    this->calculate_min_max_coords();
}

/*==========================================================================*/
/**
 *  @brief  Calculates the min/max coordinate values.
 */
/*==========================================================================*/
void GeometryObjectBase::calculate_min_max_coords( void )
{
    vismodule::Vector3f min_coord( 0.0f );
    vismodule::Vector3f max_coord( 0.0f );

    const vismodule::Real32* coord = this->coords().pointer();
    const vismodule::Real32* const end = coord + this->coords().size();

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

    this->setMinMaxObjectCoords( min_coord, max_coord );

    if ( !( this->hasMinMaxExternalCoords() ) )
    {
        this->setMinMaxExternalCoords(
            this->minObjectCoord(),
            this->maxObjectCoord() );
    }
}

} // end of namespace vismodule
