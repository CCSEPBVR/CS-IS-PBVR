/****************************************************************************/
/**
 *  @file UnstructuredVolumeObject.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeObject.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "UnstructuredVolumeObject.h"


namespace
{

const std::string GetCellTypeName( const pbvr::UnstructuredVolumeObject::CellType& type )
{
    switch ( type )
    {
    case pbvr::UnstructuredVolumeObject::Tetrahedra:
        return "tetrahedra";
    case pbvr::UnstructuredVolumeObject::Hexahedra:
        return "hexahedra";
    case pbvr::UnstructuredVolumeObject::QuadraticTetrahedra:
        return "quadratic tetrahedra";
    case pbvr::UnstructuredVolumeObject::QuadraticHexahedra:
        return "quadratic hexahedra";
    case pbvr::UnstructuredVolumeObject::Prism:
        return "prism";
    case pbvr::UnstructuredVolumeObject::Pyramid:
        return "pyramid";
    case pbvr::UnstructuredVolumeObject::Triangle:
        return "triangle";
    case pbvr::UnstructuredVolumeObject::QuadraticTriangle:
        return "triangle2";
    case pbvr::UnstructuredVolumeObject::Square:
        return "quadratic";
    case pbvr::UnstructuredVolumeObject::QuadraticSquare:
        return "quadratic2";
    default:
        return "unknown cell type";
    }
}

} // end of namespace

namespace pbvr
{

/*==========================================================================*/
/**
 *  Default constructor.
 */
/*==========================================================================*/
UnstructuredVolumeObject::UnstructuredVolumeObject():
    pbvr::VolumeObjectBase(),
    m_cell_type( UnknownCellType ),
    m_nnodes( 0 ),
    m_ncells( 0 ),
    m_connections()
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param cell_type   [in] Cell type.
 *  @param nnodes      [in] Number of nodes.
 *  @param ncells      [in] Number of cells.
 *  @param veclen      [in] Vector length.
 *  @param coords      [in] Coordinate value array.
 *  @param connections [in] Connection ( Node ID ) array.
 *  @param values      [in] Node value array.
 */
/*==========================================================================*/
UnstructuredVolumeObject::UnstructuredVolumeObject(
    const CellType     cell_type,
    const size_t       nnodes,
    const size_t       ncells,
    const size_t       veclen,
    const Coords&      coords,
    const Connections& connections,
    const Values&      values ):
    pbvr::VolumeObjectBase( veclen, coords, values ),
    m_cell_type( cell_type ),
    m_nnodes( nnodes ),
    m_ncells( ncells ),
    m_connections( connections )
{
}

/*==========================================================================*/
/**
 *  Copy constructor.
 *  @param volume [in] volume object
 */
/*==========================================================================*/
UnstructuredVolumeObject::UnstructuredVolumeObject( const UnstructuredVolumeObject& other ):
    pbvr::VolumeObjectBase( other ),
    m_cell_type( other.m_cell_type ),
    m_nnodes( other.m_nnodes ),
    m_ncells( other.m_ncells ),
    m_connections( other.m_connections )
{
    // this->shallowCopy( other );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
UnstructuredVolumeObject::~UnstructuredVolumeObject()
{
}

/*==========================================================================*/
/**
 *  '=' operator.
 *  @param volume [in] volume data
 */
/*==========================================================================*/
UnstructuredVolumeObject& UnstructuredVolumeObject::operator =( const UnstructuredVolumeObject& object )
{
    if ( this != &object )
    {
        this->shallowCopy( object );
    }

    return *this;
}

pbvr::UnstructuredVolumeObject* UnstructuredVolumeObject::DownCast( pbvr::ObjectBase* object )
{
    pbvr::VolumeObjectBase* volume = pbvr::VolumeObjectBase::DownCast( object );
    if ( !volume ) return NULL;

    const pbvr::VolumeObjectBase::VolumeType type = volume->volumeType();
    if ( type != pbvr::VolumeObjectBase::Unstructured )
    {
        kvsMessageError( "Input object is not a unstructured volume object." );
        return NULL;
    }

    pbvr::UnstructuredVolumeObject* unstructured = static_cast<pbvr::UnstructuredVolumeObject*>( volume );

    return unstructured;
}

const pbvr::UnstructuredVolumeObject* UnstructuredVolumeObject::DownCast( const pbvr::ObjectBase& object )
{
    return UnstructuredVolumeObject::DownCast( const_cast<pbvr::ObjectBase*>( &object ) );
}

std::ostream& operator << ( std::ostream& os, const UnstructuredVolumeObject& object )
{
    if ( !object.hasMinMaxValues() ) object.updateMinMaxValues();

    os << "Object type:  " << "unstructured volume object" << std::endl;
#ifdef KVS_COMPILER_VC
//#if PBVR_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
//    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
//#endif
#else
    // os << static_cast<const pbvr::VolumeObjectBase&>( object ) << std::endl;
#endif
    os << "Cell type:  " << ::GetCellTypeName( object.cellType() ) << std::endl;
    os << "Number of nodes:  " << object.nnodes() << std::endl;
    os << "Number of cells:  " << object.ncells() << std::endl;
    os << "Min. value:  " << object.minValue() << std::endl;
    os << "Max. value:  " << object.maxValue();

    return os;
}

void UnstructuredVolumeObject::shallowCopy( const UnstructuredVolumeObject& object )
{
    BaseClass::shallowCopy( object );
    this->m_cell_type = object.cellType();
    this->m_nnodes = object.nnodes();
    this->m_ncells = object.ncells();
    this->m_connections.shallowCopy( object.connections() );
}

void UnstructuredVolumeObject::deepCopy( const UnstructuredVolumeObject& object )
{
    BaseClass::deepCopy( object );
    this->m_cell_type = object.cellType();
    this->m_nnodes = object.nnodes();
    this->m_ncells = object.ncells();
    this->m_connections.deepCopy( object.connections() );
}

/*==========================================================================*/
/**
 *  Set the cell type.
 */
/*==========================================================================*/
void UnstructuredVolumeObject::setCellType( const CellType& cell_type )
{
    m_cell_type = cell_type;
}

/*==========================================================================*/
/**
 *  Set the number of nodes.
 *  @param nnodes [in] number of nodes
 */
/*==========================================================================*/
void UnstructuredVolumeObject::setNNodes( const size_t nnodes )
{
    m_nnodes = nnodes;
}

/*==========================================================================*/
/**
 *  Set the number of cells.
 *  @param ncells [in] number of cells
 */
/*==========================================================================*/
void UnstructuredVolumeObject::setNCells( const size_t ncells )
{
    m_ncells = ncells;
}

/*==========================================================================*/
/**
 *  Set the connection id array.
 *  @param connections [in] coordinate array
 */
/*==========================================================================*/
void UnstructuredVolumeObject::setConnections( const Connections& connections )
{
    m_connections = connections;
}

/*==========================================================================*/
/**
 *  Get the volume type.
 */
/*==========================================================================*/
const UnstructuredVolumeObject::VolumeType UnstructuredVolumeObject::volumeType() const
{
    return Unstructured;
}

/*==========================================================================*/
/**
 *  Get the grid type.
 */
/*==========================================================================*/
const UnstructuredVolumeObject::GridType UnstructuredVolumeObject::gridType() const
{
    return Irregular;
}

/*==========================================================================*/
/**
 *  Get the cell type.
 */
/*==========================================================================*/
const UnstructuredVolumeObject::CellType UnstructuredVolumeObject::cellType() const
{
    return m_cell_type;
}

/*==========================================================================*/
/**
 *  Get the number of nodes.
 *  @return number of nodes
 */
/*==========================================================================*/
const size_t UnstructuredVolumeObject::nnodes() const
{
    return m_nnodes;
}

/*==========================================================================*/
/**
 *  Get the number of cells.
 *  @return number of cells
 */
/*==========================================================================*/
const size_t UnstructuredVolumeObject::ncells() const
{
    return m_ncells;
}

/*==========================================================================*/
/**
 *  Get the connection id array.
 *  @return connection id array
 */
/*==========================================================================*/
const UnstructuredVolumeObject::Connections& UnstructuredVolumeObject::connections() const
{
    return m_connections;
}

/*==========================================================================*/
/**
 *  Update the min/max node coordinates.
 */
/*==========================================================================*/
void UnstructuredVolumeObject::updateMinMaxCoords()
{
    this->calculateMinMaxCoords();
}

/*==========================================================================*/
/**
 *  Calculate the min/max coordinate values.
 */
/*==========================================================================*/
void UnstructuredVolumeObject::calculateMinMaxCoords()
{
    kvs::Vector3f min_coord( 0.0f );
    kvs::Vector3f max_coord( 0.0f );

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

    this->setMinMaxObjectCoords( min_coord, max_coord );

    if ( !( this->hasMinMaxExternalCoords() ) )
    {
        this->setMinMaxExternalCoords(
            this->minObjectCoord(),
            this->maxObjectCoord() );
    }
}

} // end of namespace pbvr
