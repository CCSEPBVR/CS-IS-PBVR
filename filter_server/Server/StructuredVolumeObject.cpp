/****************************************************************************/
/**
 *  @file StructuredVolumeObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeObject.cpp 765 2011-05-20 02:42:17Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "StructuredVolumeObject.h"


namespace
{

const std::string GetGridTypeName( const pbvr::StructuredVolumeObject::GridType& type )
{
    switch ( type )
    {
    case pbvr::StructuredVolumeObject::Uniform:
        return "uniform";
    case pbvr::StructuredVolumeObject::Rectilinear:
        return "rectiliear";
    case pbvr::StructuredVolumeObject::Curvilinear:
        return "curvilinear";
    case pbvr::StructuredVolumeObject::Irregular:
        return "irregular";
    default:
        return "unknown grid type";
    }
}

} // end of namespace


namespace pbvr
{

/*==========================================================================*/
/**
 *  Constructs a new empty StructuredVolumeObject.
 */
/*==========================================================================*/
StructuredVolumeObject::StructuredVolumeObject():
    pbvr::VolumeObjectBase(),
    m_grid_type( UnknownGridType ),
    m_resolution( kvs::Vector3ui( 0, 0, 0 ) )
{
}

/*==========================================================================*/
/**
 *  Constructs a new StructuredVolumeObject with uniform grid type.
 *
 *  @param resolution [in] Node resolution.
 *  @param veclen     [in] Vector length.
 *  @param values     [in] Value array.
 */
/*==========================================================================*/
StructuredVolumeObject::StructuredVolumeObject(
    const kvs::Vector3ui& resolution,
    const size_t          veclen,
    const Values&         values ):
    pbvr::VolumeObjectBase( veclen, Coords( 0 ), values ),
    m_grid_type( Uniform ),
    m_resolution( resolution )
{
}

/*==========================================================================*/
/**
 *  Constructs a new StructuredVolumeObject.
 *
 *  @param grid_type  [in] Grid type.
 *  @param resolution [in] Node resolution.
 *  @param veclen     [in] Vector length.
 *  @param coords     [in] Coordinate array.
 *  @param values     [in] Value array.
 */
/*==========================================================================*/
StructuredVolumeObject::StructuredVolumeObject(
    const GridType        grid_type,
    const kvs::Vector3ui& resolution,
    const size_t          veclen,
    const Coords&         coords,
    const Values&         values ):
    pbvr::VolumeObjectBase( veclen, coords, values ),
    m_grid_type( grid_type ),
    m_resolution( resolution )
{
}

/*==========================================================================*/
/**
 *  Constructs a copy of other.
 *
 *  @param other [in] Structured volume.
 */
/*==========================================================================*/
StructuredVolumeObject::StructuredVolumeObject( const StructuredVolumeObject& other ):
    pbvr::VolumeObjectBase( other ),
    m_grid_type( other.m_grid_type ),
    m_resolution( other.m_resolution )
{
}

/*==========================================================================*/
/**
 *  Destroys the StructuredVolumeObject.
 */
/*==========================================================================*/
StructuredVolumeObject::~StructuredVolumeObject()
{
}

pbvr::StructuredVolumeObject* StructuredVolumeObject::DownCast( pbvr::ObjectBase* object )
{
    pbvr::VolumeObjectBase* volume = pbvr::VolumeObjectBase::DownCast( object );
    if ( !volume ) return NULL;

    const pbvr::VolumeObjectBase::VolumeType type = volume->volumeType();
    if ( type != pbvr::VolumeObjectBase::Structured )
    {
        kvsMessageError( "Input object is not a structured volume object." );
        return NULL;
    }

    pbvr::StructuredVolumeObject* structured = static_cast<pbvr::StructuredVolumeObject*>( volume );

    return structured;
}

const pbvr::StructuredVolumeObject* StructuredVolumeObject::DownCast( const pbvr::ObjectBase& object )
{
    return StructuredVolumeObject::DownCast( const_cast<pbvr::ObjectBase*>( &object ) );
}

StructuredVolumeObject& StructuredVolumeObject::operator = ( const StructuredVolumeObject& object )
{
    if ( this != &object )
    {
        this->shallowCopy( object );
    }

    return *this;
}

std::ostream& operator << ( std::ostream& os, const StructuredVolumeObject& object )
{
    if ( !object.hasMinMaxValues() ) object.updateMinMaxValues();

    os << "Object type:  " << "structured volume object" << std::endl;
#ifdef KVS_COMPILER_VC
//#if PBVR_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
//    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
//#endif
#else
#ifdef __FUJITSU
#else
    os << static_cast<const pbvr::VolumeObjectBase&>( object ) << std::endl;
#endif
#endif
    os << "Grid type:  " << ::GetGridTypeName( object.gridType() ) << std::endl;
    os << "m_resolution:  " << object.resolution() << std::endl;
    os << "Number of nodes:  " << object.nnodes() << std::endl;
    os << "Min. value:  " << object.minValue() << std::endl;
    os << "Max. value:  " << object.maxValue();

    return os;
}

void StructuredVolumeObject::shallowCopy( const StructuredVolumeObject& object )
{
    BaseClass::shallowCopy( object );
    this->m_grid_type = object.gridType();
    this->m_resolution = object.resolution();
}

void StructuredVolumeObject::deepCopy( const StructuredVolumeObject& object )
{
    BaseClass::deepCopy( object );
    this->m_grid_type = object.gridType();
    this->m_resolution = object.resolution();
}

/*==========================================================================*/
/**
 *  Sets the grid type.
 */
/*==========================================================================*/
void StructuredVolumeObject::setGridType( const GridType grid_type )
{
    m_grid_type = grid_type;
}

/*==========================================================================*/
/**
 *  Sets the node resolution.
 */
/*==========================================================================*/
void StructuredVolumeObject::setm_resolution( const kvs::Vector3ui& resolution )
{
    m_resolution = resolution;
}

/*==========================================================================*/
/**
 *  Returns the volume type.
 */
/*==========================================================================*/
const StructuredVolumeObject::VolumeType StructuredVolumeObject::volumeType() const
{
    return Structured;
}

/*==========================================================================*/
/**
 *  Returns the grid type.
 */
/*==========================================================================*/
const StructuredVolumeObject::GridType StructuredVolumeObject::gridType() const
{
    return m_grid_type;
}

/*==========================================================================*/
/**
 *  Returns the cell type.
 */
/*==========================================================================*/
const StructuredVolumeObject::CellType StructuredVolumeObject::cellType() const
{
    return Hexahedra;
}

/*==========================================================================*/
/**
 *  Returns the node resolution.
 */
/*==========================================================================*/
const kvs::Vector3ui& StructuredVolumeObject::resolution() const
{
    return m_resolution;
}

/*==========================================================================*/
/**
 *  Returns the number of nodes per line.
 */
/*==========================================================================*/
const size_t StructuredVolumeObject::nnodesPerLine() const
{
    return m_resolution.x();
}

/*==========================================================================*/
/**
 *  Returns the number of nodes per slice.
 */
/*==========================================================================*/
const size_t StructuredVolumeObject::nnodesPerSlice() const
{
    return this->nnodesPerLine() * m_resolution.y();
}

/*==========================================================================*/
/**
 *  Returns the number of nodes.
 */
/*==========================================================================*/
const size_t StructuredVolumeObject::nnodes() const
{
    return this->nnodesPerSlice() * m_resolution.z();
}

/*==========================================================================*/
/**
 *  Update the min/max node coordinates.
 */
/*==========================================================================*/
void StructuredVolumeObject::updateMinMaxCoords()
{
    this->calculateMinMaxCoords();
}

/*==========================================================================*/
/**
 *  Calculate the min/max coordinate values.
 */
/*==========================================================================*/
void StructuredVolumeObject::calculateMinMaxCoords()
{
    kvs::Vector3f min_coord( 0.0f, 0.0f, 0.0f );
    kvs::Vector3f max_coord( 0.0f, 0.0f, 0.0f );

    switch ( m_grid_type )
    {
    case Uniform:
    {
        min_coord.set( 0.0f, 0.0f, 0.0f );
        max_coord.set(
            static_cast<float>( m_resolution.x() ) - 1.0f,
            static_cast<float>( m_resolution.y() ) - 1.0f,
            static_cast<float>( m_resolution.z() ) - 1.0f );

        break;
    }
    case Rectilinear:
    {
        const float* const coord = this->coords().pointer();

        min_coord.set(
            *( coord ),
            *( coord + m_resolution.x() ),
            *( coord + m_resolution.x() + m_resolution.y() ) );

        max_coord.set(
            *( coord + m_resolution.x() - 1 ),
            *( coord + m_resolution.x() + m_resolution.y() - 1 ),
            *( coord + m_resolution.x() + m_resolution.y() + m_resolution.z() - 1 ) );

        break;
    }
    case Curvilinear:
    {
        const float*       coord = this->coords().pointer();
        const float* const end   = coord + this->coords().size();

        float x = *( coord++ );
        float y = *( coord++ );
        float z = *( coord++ );

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

        break;
    }
    default:
    {
        break;
    }
    }

    this->setMinMaxObjectCoords( min_coord, max_coord );

    if ( !( this->hasMinMaxExternalCoords() ) )
    {
        this->setMinMaxExternalCoords(
            this->minObjectCoord(),
            this->maxObjectCoord() );
    }
}

} // end of namespace pbvr
