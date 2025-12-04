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
#include <vismodule/UnstructuredVolumeObject>


namespace
{

const std::string GetCellTypeName( const vismodule::UnstructuredVolumeObject::CellType& type )
{
    switch ( type )
    {
    case vismodule::UnstructuredVolumeObject::Tetrahedra:
        return "tetrahedra";
    case vismodule::UnstructuredVolumeObject::Hexahedra:
        return "hexahedra";
    case vismodule::UnstructuredVolumeObject::QuadraticTetrahedra:
        return "quadratic tetrahedra";
    case vismodule::UnstructuredVolumeObject::QuadraticHexahedra:
        return "quadratic hexahedra";
    case vismodule::UnstructuredVolumeObject::Prism:
        return "prism";
    case vismodule::UnstructuredVolumeObject::Pyramid:
        return "pyramid";
    case vismodule::UnstructuredVolumeObject::Triangle:
        return "triangle";
    case vismodule::UnstructuredVolumeObject::QuadraticTriangle:
        return "triangle2";
    case vismodule::UnstructuredVolumeObject::Square:
        return "quadratic";
    case vismodule::UnstructuredVolumeObject::QuadraticSquare:
        return "quadratic2";
    default:
        return "unknown cell type";
    }
}

} // end of namespace

namespace vismodule
{

/*==========================================================================*/
/**
 *  Default constructor.
 */
/*==========================================================================*/
UnstructuredVolumeObject::UnstructuredVolumeObject():
    vismodule::VolumeObjectBase(),
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
    vismodule::VolumeObjectBase( veclen, coords, values ),
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
    vismodule::VolumeObjectBase( other ),
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

vismodule::UnstructuredVolumeObject* UnstructuredVolumeObject::DownCast( vismodule::ObjectBase* object )
{
    vismodule::VolumeObjectBase* volume = vismodule::VolumeObjectBase::DownCast( object );
    if ( !&volume ) return NULL;

    const vismodule::VolumeObjectBase::VolumeType type = volume->volumeType();
    if ( type != vismodule::VolumeObjectBase::Unstructured )
    {
        visModuleMessageError( "Input object is not a unstructured volume object." );
        return NULL;
    }

    vismodule::UnstructuredVolumeObject* unstructured = static_cast<vismodule::UnstructuredVolumeObject*>( volume );

    return unstructured;
}

const vismodule::UnstructuredVolumeObject* UnstructuredVolumeObject::DownCast( const vismodule::ObjectBase& object )
{
    return UnstructuredVolumeObject::DownCast( const_cast<vismodule::ObjectBase*>( &object ) );
}

std::ostream& operator << ( std::ostream& os, const UnstructuredVolumeObject& object )
{
    if ( !object.hasMinMaxValues() ) object.updateMinMaxValues();

    os << "Object type:  " << "unstructured volume object" << std::endl;
#ifdef VIS_MODULE_COMPILER_VC
//#if VIS_MODULE_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
//    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
//#endif
#else
    // os << static_cast<const vismodule::VolumeObjectBase&>( object ) << std::endl;
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
    vismodule::Vector3f min_coord( 0.0f );
    vismodule::Vector3f max_coord( 0.0f );

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
