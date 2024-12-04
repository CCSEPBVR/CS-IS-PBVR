/*****************************************************************************/
/**
 *  @file   PointObject.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointObject.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PointObject.h"
//#include <vismodule/LineObject>
//#include <vismodule/PolygonObject>
#include <vismodule/Assert>


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 */
/*===========================================================================*/
PointObject::PointObject()
{
    this->setSize( 1 );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  colors [in] color value array
 *  @param  normals [in] normal vector array
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::ValueArray<vismodule::Real32>& normals,
    const vismodule::ValueArray<vismodule::Real32>& sizes ):
    pbvr::GeometryObjectBase( coords, colors, normals )
{
    this->setSizes( sizes );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  colors [in] color value array
 *  @param  normals [in] normal vector array
 *  @param  size [in] size value
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::ValueArray<vismodule::Real32>& normals,
    const vismodule::Real32                   size ):
    pbvr::GeometryObjectBase( coords, colors, normals )
{
    this->setSize( size );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  color [in] color value array
 *  @param  normals [in] normal vector array
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::ValueArray<vismodule::Real32>& normals,
    const vismodule::ValueArray<vismodule::Real32>& sizes ):
    pbvr::GeometryObjectBase( coords, color, normals )
{
    this->setSizes( sizes );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  normals [in] normal vector array
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::Real32>& normals,
    const vismodule::ValueArray<vismodule::Real32>& sizes ):
    pbvr::GeometryObjectBase( coords, normals )
{
    this->setSizes( sizes );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  color [in] color value
 *  @param  normals [in] normal vector array
 *  @param  size [in] size value
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::ValueArray<vismodule::Real32>& normals,
    const vismodule::Real32                   size ):
    pbvr::GeometryObjectBase( coords, color, normals )
{
    this->setSize( size );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  colors [in] color value array
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::ValueArray<vismodule::Real32>& sizes ):
    pbvr::GeometryObjectBase( coords, colors )
{
    this->setSizes( sizes );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  color [in] color value
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::ValueArray<vismodule::Real32>& sizes ):
    pbvr::GeometryObjectBase( coords, color )
{
    VIS_MODULE_ASSERT( coords.size() == sizes.size() * 3 );

    this->setSizes( sizes );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  colors [in] color value array
 *  @param  size [in] size value
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::ValueArray<vismodule::UInt8>&  colors,
    const vismodule::Real32                   size ):
    pbvr::GeometryObjectBase( coords, colors )
{
    this->setSize( size );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 *  @param  color [in] color value
 *  @param  size [in] size value
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords,
    const vismodule::RGBColor&                color,
    const vismodule::Real32                   size ):
    pbvr::GeometryObjectBase( coords, color )
{
    this->setSize( size );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  coords [in] coordinate value array
 */
/*===========================================================================*/
PointObject::PointObject(
    const vismodule::ValueArray<vismodule::Real32>& coords ):
    pbvr::GeometryObjectBase( coords )
{
    this->setSize( 1 );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointObject class.
 *  @param  other [in] point object
 */
/*===========================================================================*/
PointObject::PointObject( const pbvr::PointObject& other )
{
    this->shallowCopy( other );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the PointObject class.
 */
/*===========================================================================*/
PointObject::~PointObject()
{
    this->clear();
}

/*===========================================================================*/
/**
 *  @brief  Downcasts to the pointer to the point object.
 *  @param  object [in] pointer to the object
 *  @return pointer to the point object
 */
/*===========================================================================*/
pbvr::PointObject* PointObject::DownCast( pbvr::ObjectBase* object )
{
    pbvr::GeometryObjectBase* geometry = pbvr::GeometryObjectBase::DownCast( object );
    if ( !geometry ) return NULL;

    const pbvr::GeometryObjectBase::GeometryType type = geometry->geometryType();
    if ( type != pbvr::GeometryObjectBase::Point )
    {
        visModuleMessageError( "Input object is not a point object." );
        return NULL;
    }

    pbvr::PointObject* point = static_cast<pbvr::PointObject*>( geometry );

    return point;
}

/*===========================================================================*/
/**
 *  @brief  Downcasts to the pointer to the point object with 'const'.
 *  @param  object [in] pointer to the object
 *  @return pointer to the point object
 */
/*===========================================================================*/
const pbvr::PointObject* PointObject::DownCast( const pbvr::ObjectBase& object )
{
    return PointObject::DownCast( const_cast<pbvr::ObjectBase*>( &object ) );
}

/*===========================================================================*/
/**
 *  @brief  '='operator.
 *  @param  other [in] point object
 */
/*===========================================================================*/
PointObject& PointObject::operator = ( const PointObject& other )
{
    if ( this != &other )
    {
        this->shallowCopy( other );
    }

    return *this;
}

/*===========================================================================*/
/**
 *  @brief  '+=' operator.
 *  @param  other [in] point object
 */
/*===========================================================================*/
PointObject& PointObject::operator += ( const PointObject& other )
{
    this->add( other );

    return *this;
}

/*===========================================================================*/
/**
 *  @brief  '<<' operator
 */
/*===========================================================================*/
std::ostream& operator << ( std::ostream& os, const PointObject& object )
{
    os << "Object type:  " << "point object" << std::endl;
#ifdef VIS_MODULE_COMPILER_VC
//#if PBVR_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
//    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
//#endif
#else
    os << static_cast<const pbvr::GeometryObjectBase&>( object ) << std::endl;
#endif
    os << "Number of sizes:  " << object.nsizes();

    return os;
}

/*===========================================================================*/
/**
 *  @brief  Add the point object.
 *  @param  other [in] point object
 */
/*===========================================================================*/
void PointObject::add( const PointObject& other )
{
    if ( this->coords().size() == 0 )
    {
        // Copy the object.
        BaseClass::setCoords( other.coords() );
        BaseClass::setNormals( other.normals() );
        BaseClass::setColors( other.colors() );
        this->setSizes( other.sizes() );

        BaseClass::setMinMaxObjectCoords(
            other.minObjectCoord(),
            other.maxObjectCoord() );
        BaseClass::setMinMaxExternalCoords(
            other.minExternalCoord(),
            other.maxExternalCoord() );
    }
    else
    {
        if ( !BaseClass::hasMinMaxObjectCoords() )
        {
            BaseClass::updateMinMaxCoords();
        }

        vismodule::Vector3f min_object_coord( BaseClass::minObjectCoord() );
        vismodule::Vector3f max_object_coord( BaseClass::maxObjectCoord() );

        min_object_coord.x() = vismodule::Math::Min( min_object_coord.x(), other.minObjectCoord().x() );
        min_object_coord.y() = vismodule::Math::Min( min_object_coord.y(), other.minObjectCoord().y() );
        min_object_coord.z() = vismodule::Math::Min( min_object_coord.z(), other.minObjectCoord().z() );

        max_object_coord.x() = vismodule::Math::Max( max_object_coord.x(), other.maxObjectCoord().x() );
        max_object_coord.y() = vismodule::Math::Max( max_object_coord.y(), other.maxObjectCoord().y() );
        max_object_coord.z() = vismodule::Math::Max( max_object_coord.z(), other.maxObjectCoord().z() );

        BaseClass::setMinMaxObjectCoords( min_object_coord, max_object_coord );
        BaseClass::setMinMaxExternalCoords( min_object_coord, max_object_coord );

        // Integrate the coordinate values.
        vismodule::ValueArray<vismodule::Real32> coords;
        const size_t ncoords = this->coords().size() + other.coords().size();
        vismodule::Real32* pcoords = coords.allocate( ncoords );
        if ( pcoords != NULL )
        {
            // x,y,z, ... + x,y,z, ... = x,y,z, ... ,x,y,z, ...
            memcpy( pcoords, this->coords().pointer(), this->coords().byteSize() );
            memcpy( pcoords + this->coords().size(), other.coords().pointer(), other.coords().byteSize() );
        }
        BaseClass::setCoords( coords );

        // Integrate the normal vectors.
        vismodule::ValueArray<vismodule::Real32> normals;
        if ( this->normals().size() > 0 )
        {
            if ( other.normals().size() > 0 )
            {
                // nx,ny,nz, ... + nx,ny,nz, ... = nx,ny,nz, ... ,nx,ny,nz, ...
                const size_t nnormals = this->normals().size() + other.normals().size();
                vismodule::Real32* pnormals = normals.allocate( nnormals );
                if ( pnormals != NULL )
                {
                    memcpy( pnormals, this->normals().pointer(), this->normals().byteSize() );
                    memcpy( pnormals + this->normals().size(), other.normals().pointer(), other.normals().byteSize() );
                }
            }
            else
            {
                // nx,ny,nz, ... + (none) = nx,ny,nz, ... ,0,0,0, ...
                const size_t nnormals = this->normals().size() + other.coords().size();
                vismodule::Real32* pnormals = normals.allocate( nnormals );
                if ( pnormals != NULL )
                {
                    memcpy( pnormals, this->normals().pointer(), this->normals().byteSize() );
                    memset( pnormals + this->normals().size(), 0, other.coords().byteSize() );
                }
            }
        }
        else
        {
            if ( other.normals().size() > 0 )
            {
                const size_t nnormals = this->coords().size() + other.normals().size();
                vismodule::Real32* pnormals = normals.allocate( nnormals );
                if ( pnormals != NULL )
                {
                    // (none) + nx,ny,nz, ... = 0,0,0, ... ,nz,ny,nz, ...
                    memset( pnormals, 0, this->coords().byteSize() );
                    memcpy( pnormals + this->coords().size(), other.normals().pointer(), other.normals().byteSize() );
                }
            }
        }
        BaseClass::setNormals( normals );

        // Integrate the color values.
        vismodule::ValueArray<vismodule::UInt8> colors;
        if ( this->colors().size() > 1 )
        {
            if ( other.colors().size() > 1 )
            {
                // r,g,b, ... + r,g,b, ... = r,g,b, ... ,r,g,b, ...
                const size_t ncolors = this->colors().size() + other.colors().size();
                vismodule::UInt8* pcolors = colors.allocate( ncolors );
                if ( pcolors != NULL )
                {
                    memcpy( pcolors, this->colors().pointer(), this->colors().byteSize() );
                    memcpy( pcolors + this->colors().size(), other.colors().pointer(), other.colors().byteSize() );
                }
            }
            else
            {
                // r,g,b, ... + R,G,B = r,g,b, ... ,R,G,B, ... ,R,G,B
                const size_t ncolors = this->colors().size() + other.coords().size();
                vismodule::UInt8* pcolors = colors.allocate( ncolors );
                if ( pcolors != NULL )
                {
                    memcpy( pcolors, this->colors().pointer(), this->colors().byteSize() );
                    pcolors += this->colors().size();
                    const vismodule::RGBColor color = other.color();
                    for ( size_t i = 0; i < other.coords().size(); i += 3 )
                    {
                        *( pcolors++ ) = color.r();
                        *( pcolors++ ) = color.g();
                        *( pcolors++ ) = color.b();
                    }
                }
            }
        }
        else
        {
            if ( other.colors().size() > 1 )
            {
                // R,G,B + r,g,b, ... = R,G,B, ... ,R,G,B, r,g,b, ...
                const size_t ncolors = this->coords().size() + other.colors().size();
                vismodule::UInt8* pcolors = colors.allocate( ncolors );
                if ( pcolors != NULL )
                {
                    const vismodule::RGBColor color = this->color();
                    for ( size_t i = 0; i < this->coords().size(); i += 3 )
                    {
                        *( pcolors++ ) = color.r();
                        *( pcolors++ ) = color.g();
                        *( pcolors++ ) = color.b();
                    }
                    memcpy( pcolors, other.colors().pointer(), other.colors().byteSize() );
                }
            }
            else
            {
                const vismodule::RGBColor color1 = this->color();
                const vismodule::RGBColor color2 = other.color();
                if ( color1 == color2 )
                {
                    // R,G,B + R,G,B = R,G,B
                    const size_t ncolors = 3;
                    vismodule::UInt8* pcolors = colors.allocate( ncolors );
                    if ( pcolors != NULL )
                    {
                        *( pcolors++ ) = color1.r();
                        *( pcolors++ ) = color1.g();
                        *( pcolors++ ) = color1.b();
                    }
                }
                else
                {
                    // R,G,B + R,G,B = R,G,B, ... ,R,G,B, ...
                    const size_t ncolors = this->coords().size() + other.coords().size();
                    vismodule::UInt8* pcolors = colors.allocate( ncolors );
                    if ( pcolors != NULL )
                    {
                        for ( size_t i = 0; i < this->coords().size(); i += 3 )
                        {
                            *( pcolors++ ) = color1.r();
                            *( pcolors++ ) = color1.g();
                            *( pcolors++ ) = color1.b();
                        }
                        for ( size_t i = 0; i < other.coords().size(); i += 3 )
                        {
                            *( pcolors++ ) = color2.r();
                            *( pcolors++ ) = color2.g();
                            *( pcolors++ ) = color2.b();
                        }
                    }
                }
            }
        }
        BaseClass::setColors( colors );

        // Integrate the size values.
        vismodule::ValueArray<vismodule::Real32> sizes;
        if ( this->sizes().size() > 1 )
        {
            if ( other.sizes().size() > 1 )
            {
                // s, ... + s, ... = s, ... ,s, ...
                const size_t nsizes = this->sizes().size() + other.sizes().size();
                vismodule::Real32* psizes = sizes.allocate( nsizes );
                if ( psizes != NULL )
                {
                    memcpy( psizes, this->sizes().pointer(), this->sizes().byteSize() );
                    memcpy( psizes + this->sizes().size(), other.sizes().pointer(), other.sizes().byteSize() );
                }
            }
            else
            {
                // s, ... + S = s, ... ,S, ... ,S
                const size_t nsizes = this->sizes().size() + other.coords().size();
                vismodule::Real32* psizes = sizes.allocate( nsizes );
                if ( psizes != NULL )
                {
                    memcpy( psizes, this->sizes().pointer(), this->sizes().byteSize() );
                    psizes += this->colors().size();
                    const vismodule::Real32 size = other.size();
                    for ( size_t i = 0; i < other.coords().size(); i++ )
                    {
                        *( psizes++ ) = size;
                    }
                }
            }
        }
        else
        {
            if ( other.sizes().size() > 1 )
            {
                // S + s, ... = S, ... ,S, s, ...
                const size_t nsizes = this->coords().size() + other.sizes().size();
                vismodule::Real32* psizes = sizes.allocate( nsizes );
                if ( psizes != NULL )
                {
                    const vismodule::Real32 size = this->size();
                    for ( size_t i = 0; i < this->coords().size(); i++ )
                    {
                        *( psizes++ ) = size;
                    }
                    memcpy( psizes, other.sizes().pointer(), other.sizes().byteSize() );
                }
            }
#if 0
            else
            {
                const vismodule::Real32 size1 = this->size();
                const vismodule::Real32 size2 = other.size();
                if ( size1 == size2 )
                {
                    // S + S = S
                    const size_t nsizes = 1;
                    vismodule::Real32* psizes = sizes.allocate( nsizes );
                    if ( psizes != NULL )
                    {
                        *( psizes++ ) = size1;
                    }
                }
                else
                {
                    // S + S = S, ... , S, ...
                    const size_t nsizes = this->coords().size() + other.coords().size();
                    vismodule::Real32* psizes = sizes.allocate( nsizes );
                    if ( psizes != NULL )
                    {
                        for ( size_t i = 0; i < this->coords().size(); i++ )
                        {
                            *( psizes++ ) = size1;
                        }
                        for ( size_t i = 0; i < other.coords().size(); i++ )
                        {
                            *( psizes++ ) = size2;
                        }
                    }
                }
            }
#endif
        }
        this->setSizes( sizes );
    }
}

/*===========================================================================*/
/**
 *  @brief  Shallow copy the point object.
 *  @param  other [in] point object
 */
/*===========================================================================*/
void PointObject::shallowCopy( const PointObject& other )
{
    BaseClass::shallowCopy( other );
    m_sizes.shallowCopy( other.sizes() );
}

/*===========================================================================*/
/**
 *  @brief  Deep copy the point object.
 *  @param  other [in] point object
 */
/*===========================================================================*/
void PointObject::deepCopy( const PointObject& other )
{
    BaseClass::deepCopy( other );
    m_sizes.deepCopy( other.sizes() );
}

/*===========================================================================*/
/**
 *  @brief  Clear the point object.
 */
/*===========================================================================*/
void PointObject::clear()
{
    BaseClass::clear();
    m_sizes.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Sets a size value array.
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
void PointObject::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    m_sizes = sizes;
}

/*===========================================================================*/
/**
 *  @brief  Sets a size value.
 *  @param  size [in] size value
 */
/*===========================================================================*/
void PointObject::setSize( const vismodule::Real32 size )
{
    m_sizes.allocate( 1 );
    m_sizes[0] = size;
}

void PointObject::setTfnumber( const int tf_number )
{
    m_tf_number = tf_number;
}

void PointObject::setNbins( const int nbins )
{
    m_nbins = nbins;
}

/*===========================================================================*/
/**
 *  @brief  Returns the geometry type.
 *  @return geometry type
 */
/*===========================================================================*/
const PointObject::BaseClass::GeometryType PointObject::geometryType() const
{
    return BaseClass::Point;
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of size values.
 *  @return number of size values
 */
/*===========================================================================*/
const size_t PointObject::nsizes() const
{
    return m_sizes.size();
}

/*===========================================================================*/
/**
 *  @brief  Returns the size value which is specified by the index.
 *  @param  index [in] index of the size value array
 *  @return size value
 */
/*===========================================================================*/
const vismodule::Real32 PointObject::size( const size_t index ) const
{
    return m_sizes[index];
}

/*===========================================================================*/
/**
 *  @brief  Returns the size value array.
 *  @return size value array
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& PointObject::sizes() const
{
    return m_sizes;
}

/*===========================================================================*/
/**
 *  @brief  Returns the Color Histogram Table.
 *  @param  index [in] index of FrequencyTable Object array.
 *  @return FrequencyTable Object
 */
/*===========================================================================*/
const vismodule::ValueArray<pbvr::FrequencyTable>& PointObject::getColorHistogram() const
{
    return m_color_histogram;
}
/*===========================================================================*/
/**
 *  @brief  Returns the Opacity Histogram Table.
 *  @param  index [in] index of FrequencyTable Object array.
 *  @return FrequencyTable Object
 */
/*===========================================================================*/
const vismodule::ValueArray<pbvr::FrequencyTable>& PointObject::getOpacityHistogram() const
{
    return m_opacity_histogram;
}

//add by shimomura 2023
const vismodule::ValueArray<int>& PointObject::getOHistogram() const
{
    return m_o_histogram;
}

const vismodule::ValueArray<int>& PointObject::getCHistogram() const
{
    return m_c_histogram;
}

const int PointObject::getTfnumber() const
{
    return m_tf_number;
}

const int PointObject::getNbins() const
{
    return m_nbins;
}

} // end of namespace pbvr
