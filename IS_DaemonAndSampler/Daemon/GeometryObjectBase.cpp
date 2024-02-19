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
#include <kvs/Assert>


namespace pbvr
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
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt8>&  colors,
    const kvs::ValueArray<kvs::Real32>& normals )
{
    KVS_ASSERT( coords.size() == normals.size() );
    KVS_ASSERT( coords.size() == colors.size() );

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
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::RGBColor&                color,
    const kvs::ValueArray<kvs::Real32>& normals )
{
    KVS_ASSERT( coords.size() == normals.size() );

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
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::Real32>& normals )
{
    KVS_ASSERT( coords.size() == normals.size() );

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
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::ValueArray<kvs::UInt8>&  colors )
{
    KVS_ASSERT( coords.size() == colors.size() );

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
    const kvs::ValueArray<kvs::Real32>& coords,
    const kvs::RGBColor&                color )
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
    const kvs::ValueArray<kvs::Real32>& coords )
{
    this->setCoords( coords );
    this->setColor( kvs::RGBColor( 255, 255, 255 ) );
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
pbvr::GeometryObjectBase* GeometryObjectBase::DownCast( pbvr::ObjectBase* object )
{
    const pbvr::ObjectBase::ObjectType type = object->objectType();
    if ( type != pbvr::ObjectBase::Geometry )
    {
        kvsMessageError( "Input object is not a geometry object." );
        return ( NULL );
    }

    pbvr::GeometryObjectBase* geometry = static_cast<pbvr::GeometryObjectBase*>( object );

    return ( geometry );
}

/*===========================================================================*/
/**
 *  @brief  Downcast to GeometryObjectBase with 'const'.
 *  @param  object [in] pointer to the object base
 *  @return pointer to the geometry object base
 */
/*===========================================================================*/
const pbvr::GeometryObjectBase* GeometryObjectBase::DownCast( const pbvr::ObjectBase* object )
{
    return ( GeometryObjectBase::DownCast( const_cast<pbvr::ObjectBase*>( object ) ) );
}

/*===========================================================================*/
/**
 *  @brief  '<<' operator.
 */
/*===========================================================================*/
std::ostream& operator << ( std::ostream& os, const pbvr::GeometryObjectBase& object )
{
#ifdef KVS_COMPILER_VC
#if KVS_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
    os << static_cast<const pbvr::ObjectBase&>( object ) << std::endl;
#endif
    os << "Number of vertices:  " << object.nvertices() << std::endl;
    os << "Number of colors:  " << object.ncolors() << std::endl;
    os << "Number of normal vectors:  " << object.nnormals();

    return ( os );
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
void GeometryObjectBase::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color value array.
 *  @param  colors [in] color value array
 */
/*===========================================================================*/
void GeometryObjectBase::setColors( const kvs::ValueArray<kvs::UInt8>& colors )
{
    m_colors = colors;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color value.
 *  @param  color [in] color value
 */
/*===========================================================================*/
void GeometryObjectBase::setColor( const kvs::RGBColor& color )
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
void GeometryObjectBase::setNormals( const kvs::ValueArray<kvs::Real32>& normals )
{
    m_normals = normals;
}

/*===========================================================================*/
/**
 *  @brief  Returns the object type.
 *  @return object type
 */
/*===========================================================================*/
const pbvr::ObjectBase::ObjectType GeometryObjectBase::objectType( void ) const
{
    return ( pbvr::ObjectBase::Geometry );
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
    return ( m_coords.size() / dimension );
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
    return ( m_colors.size() / nchannels );
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
    return ( m_normals.size() / dimension );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value.
 *  @param  index [in] index of the coordinate value
 *  @return coordinate value
 */
/*===========================================================================*/
const kvs::Vector3f GeometryObjectBase::coord( const size_t index ) const
{
    const size_t dimension = 3;
    return ( kvs::Vector3f( m_coords.pointer() + dimension * index ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value.
 *  @param  index [in] index of the color value
 *  @return color value
 */
/*===========================================================================*/
const kvs::RGBColor GeometryObjectBase::color( const size_t index ) const
{
    const size_t nchannels = 3;
    return ( kvs::RGBColor( m_colors.pointer() + nchannels * index ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normal vector.
 *  @param  index [in] index of the normal vector
 *  @return normal vector
 */
/*===========================================================================*/
const kvs::Vector3f GeometryObjectBase::normal( const size_t index ) const
{
    const size_t dimension = 3;
    return ( kvs::Vector3f( m_normals.pointer() + dimension * index ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value array.
 *  @return coordinate value array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& GeometryObjectBase::coords( void ) const
{
    return ( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value array.
 *  @return color value array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt8>& GeometryObjectBase::colors( void ) const
{
    return ( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normal vector array.
 *  @return normal vector array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& GeometryObjectBase::normals( void ) const
{
    return ( m_normals );
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
    kvs::Vector3f min_coord( 0.0f );
    kvs::Vector3f max_coord( 0.0f );

    const kvs::Real32* coord = this->coords().pointer();
    const kvs::Real32* const end = coord + this->coords().size();

    kvs::Real32 x = *( coord++ );
    kvs::Real32 y = *( coord++ );
    kvs::Real32 z = *( coord++ );

    min_coord.set( x, y, z );
    max_coord.set( x, y, z );

    while ( coord < end )
    {
        x = *( coord++ );
        y = *( coord++ );
        z = *( coord++ );

        min_coord.x() = kvs::Math::Min( min_coord.x(), x );
        min_coord.y() = kvs::Math::Min( min_coord.y(), y );
        min_coord.z() = kvs::Math::Min( min_coord.z(), z );

        max_coord.x() = kvs::Math::Max( max_coord.x(), x );
        max_coord.y() = kvs::Math::Max( max_coord.y(), y );
        max_coord.z() = kvs::Math::Max( max_coord.z(), z );
    }

    this->setMinMaxObjectCoords( min_coord, max_coord );

    if ( !( this->hasMinMaxExternalCoords() ) )
    {
        this->setMinMaxExternalCoords(
            this->minObjectCoord(),
            this->maxObjectCoord() );
    }
}

} // end of namespace kvs
