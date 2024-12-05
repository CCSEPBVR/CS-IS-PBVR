/*****************************************************************************/
/**
 *  @file   TetrahedraToTetrahedra.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TetrahedraToTetrahedra.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "TetrahedraToTetrahedra.h"
#include <algorithm>
#include <map>
#include <vismodule/AnyValueArray>


namespace
{

/*===========================================================================*/
/**
 *  @brief  ID map class for removing the duplicated nodes.
 */
/*===========================================================================*/
class IDMap
{
public:

    typedef vismodule::UInt32 Key;
    typedef vismodule::UInt32 Value;
    typedef std::map<Key,Value> Bucket;

private:

    size_t m_index; ///< index of the connection ID
    Bucket m_bucket; ///< bucket for the IDs

public:

    IDMap( void );

public:

    const Bucket& bucket( void ) const;

    void insert( const vismodule::UInt32 id );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new IDMap class.
 */
/*===========================================================================*/
IDMap::IDMap( void ):
    m_index( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the ID bucket.
 *  @return ID bucket
 */
/*===========================================================================*/
const IDMap::Bucket& IDMap::bucket( void ) const
{
    return( m_bucket );
}

/*===========================================================================*/
/**
 *  @brief  Inserts a ID.
 *  @param  id [in] ID
 */
/*===========================================================================*/
void IDMap::insert( const vismodule::UInt32 id )
{
    const Key key = id;
    const Value value = m_index;

    Bucket::iterator i = m_bucket.find( key );
    Bucket::const_iterator last = m_bucket.end();
    if ( i == last )
    {
        m_bucket.insert( Bucket::value_type( key, value ) );
        m_index++;
    }
}

} // end of namespace


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TetrahedraToTetrahedra class.
 */
/*===========================================================================*/
TetrahedraToTetrahedra::TetrahedraToTetrahedra( void ):
    m_method( TetrahedraToTetrahedra::Subdivision8 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TetrahedraToTetrahedra class.
 *  @param  volume [in] pointer to a volume data
 *  @param  method [in] conversion method
 */
/*===========================================================================*/
TetrahedraToTetrahedra::TetrahedraToTetrahedra(
    const vismodule::UnstructuredVolumeObject& volume,
    const TetrahedraToTetrahedra::Method method )
{
    this->setMethod( method );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the TetrahedraToTetrahedra class.
 */
/*===========================================================================*/
TetrahedraToTetrahedra::~TetrahedraToTetrahedra( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes this class.
 *  @param  object [in] pointer to the object
 *  @return pointer to the converted object
 */
/*===========================================================================*/
TetrahedraToTetrahedra::SuperClass* TetrahedraToTetrahedra::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::UnstructuredVolumeObject* volume = vismodule::UnstructuredVolumeObject::DownCast( object );
    if ( !&volume )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not supported.");
        return( NULL );
    }

    if ( volume->cellType() == vismodule::UnstructuredVolumeObject::QuadraticTetrahedra )
    {
        if ( m_method == TetrahedraToTetrahedra::Subdivision8 )
        {
            const std::type_info& type = volume->values().typeInfo()->type();
            if (      type == typeid( vismodule::Int8   ) ) this->subdivide_8_tetrahedra<vismodule::Int8>(   *volume );
            else if ( type == typeid( vismodule::Int16  ) ) this->subdivide_8_tetrahedra<vismodule::Int16>(  *volume );
            else if ( type == typeid( vismodule::Int32  ) ) this->subdivide_8_tetrahedra<vismodule::Int32>(  *volume );
            else if ( type == typeid( vismodule::Int64  ) ) this->subdivide_8_tetrahedra<vismodule::Int64>(  *volume );
            else if ( type == typeid( vismodule::UInt8  ) ) this->subdivide_8_tetrahedra<vismodule::UInt8>(  *volume );
            else if ( type == typeid( vismodule::UInt16 ) ) this->subdivide_8_tetrahedra<vismodule::UInt16>( *volume );
            else if ( type == typeid( vismodule::UInt32 ) ) this->subdivide_8_tetrahedra<vismodule::UInt32>( *volume );
            else if ( type == typeid( vismodule::UInt64 ) ) this->subdivide_8_tetrahedra<vismodule::UInt64>( *volume );
            else if ( type == typeid( vismodule::Real32 ) ) this->subdivide_8_tetrahedra<vismodule::Real32>( *volume );
            else if ( type == typeid( vismodule::Real64 ) ) this->subdivide_8_tetrahedra<vismodule::Real64>( *volume );
            else
            {
                BaseClass::m_is_success = false;
                visModuleMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
                return( NULL );
            }
        }
        else if ( m_method == TetrahedraToTetrahedra::Removal )
        {
            const std::type_info& type = volume->values().typeInfo()->type();
            if (      type == typeid( vismodule::Int8   ) ) this->remove_quadratic_nodes<vismodule::Int8>(   *volume );
            else if ( type == typeid( vismodule::Int16  ) ) this->remove_quadratic_nodes<vismodule::Int16>(  *volume );
            else if ( type == typeid( vismodule::Int32  ) ) this->remove_quadratic_nodes<vismodule::Int32>(  *volume );
            else if ( type == typeid( vismodule::Int64  ) ) this->remove_quadratic_nodes<vismodule::Int64>(  *volume );
            else if ( type == typeid( vismodule::UInt8  ) ) this->remove_quadratic_nodes<vismodule::UInt8>(  *volume );
            else if ( type == typeid( vismodule::UInt16 ) ) this->remove_quadratic_nodes<vismodule::UInt16>( *volume );
            else if ( type == typeid( vismodule::UInt32 ) ) this->remove_quadratic_nodes<vismodule::UInt32>( *volume );
            else if ( type == typeid( vismodule::UInt64 ) ) this->remove_quadratic_nodes<vismodule::UInt64>( *volume );
            else if ( type == typeid( vismodule::Real32 ) ) this->remove_quadratic_nodes<vismodule::Real32>( *volume );
            else if ( type == typeid( vismodule::Real64 ) ) this->remove_quadratic_nodes<vismodule::Real64>( *volume );
            else
            {
                BaseClass::m_is_success = false;
                visModuleMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
                return( NULL );
            }
        }
    }
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not tetrahedral cells.");
        return( NULL );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Sets conversion method.
 *  @param  method [in] conversion method
 */
/*===========================================================================*/
void TetrahedraToTetrahedra::setMethod( const TetrahedraToTetrahedra::Method method )
{
    m_method = method;
}

/*===========================================================================*/
/**
 *  @brief  Subdivides a qudratic tetrahedron into eight tetrahedra.
 *  @param  volume [in] pointer to the volume data
 */
/*===========================================================================*/
template <typename T>
void TetrahedraToTetrahedra::subdivide_8_tetrahedra( const vismodule::UnstructuredVolumeObject& volume )
{
    // Quadratic tetrahedral cells.
    const size_t tet2_ncells = volume.ncells();
    const vismodule::UInt32* tet2_pconnections = volume.connections().pointer();

    // Tetrahedral cells.
    const size_t ndivisions = 8;
    const size_t tet_ncells = tet2_ncells * ndivisions;
    vismodule::ValueArray<vismodule::UInt32> tet_connections( tet_ncells * 4 );
    vismodule::UInt32* tet_pconnections = tet_connections.pointer();
    for ( size_t i = 0; i < tet2_ncells; i++ )
    {
        const vismodule::UInt32 id0 = *(tet2_pconnections++);
        const vismodule::UInt32 id1 = *(tet2_pconnections++);
        const vismodule::UInt32 id2 = *(tet2_pconnections++);
        const vismodule::UInt32 id3 = *(tet2_pconnections++);
        const vismodule::UInt32 id4 = *(tet2_pconnections++);
        const vismodule::UInt32 id5 = *(tet2_pconnections++);
        const vismodule::UInt32 id6 = *(tet2_pconnections++);
        const vismodule::UInt32 id7 = *(tet2_pconnections++);
        const vismodule::UInt32 id8 = *(tet2_pconnections++);
        const vismodule::UInt32 id9 = *(tet2_pconnections++);

        *(tet_pconnections++) = id0;
        *(tet_pconnections++) = id4;
        *(tet_pconnections++) = id5;
        *(tet_pconnections++) = id6;

        *(tet_pconnections++) = id4;
        *(tet_pconnections++) = id1;
        *(tet_pconnections++) = id7;
        *(tet_pconnections++) = id9;

        *(tet_pconnections++) = id5;
        *(tet_pconnections++) = id7;
        *(tet_pconnections++) = id2;
        *(tet_pconnections++) = id8;

        *(tet_pconnections++) = id6;
        *(tet_pconnections++) = id9;
        *(tet_pconnections++) = id8;
        *(tet_pconnections++) = id3;

        *(tet_pconnections++) = id5;
        *(tet_pconnections++) = id6;
        *(tet_pconnections++) = id4;
        *(tet_pconnections++) = id9;

        *(tet_pconnections++) = id5;
        *(tet_pconnections++) = id9;
        *(tet_pconnections++) = id4;
        *(tet_pconnections++) = id7;

        *(tet_pconnections++) = id5;
        *(tet_pconnections++) = id8;
        *(tet_pconnections++) = id9;
        *(tet_pconnections++) = id7;

        *(tet_pconnections++) = id5;
        *(tet_pconnections++) = id6;
        *(tet_pconnections++) = id9;
        *(tet_pconnections++) = id8;
    }

    if ( volume.hasMinMaxExternalCoords() )
    {
        const vismodule::Vector3f min_coord( volume.minExternalCoord() );
        const vismodule::Vector3f max_coord( volume.maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( volume.hasMinMaxObjectCoords() )
    {
        const vismodule::Vector3f min_coord( volume.minObjectCoord() );
        const vismodule::Vector3f max_coord( volume.maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    if ( volume.hasMinMaxValues() )
    {
        const vismodule::Real64 min_value( volume.minValue() );
        const vismodule::Real64 max_value( volume.maxValue() );
        SuperClass::setMinMaxValues( min_value, max_value );
    }

    SuperClass::setVeclen( volume.veclen() );
    SuperClass::setNNodes( volume.nnodes() );
    SuperClass::setNCells( tet_ncells );
    SuperClass::setCellType( vismodule::UnstructuredVolumeObject::Tetrahedra );
    SuperClass::setCoords( volume.coords() );
    SuperClass::setConnections( tet_connections );
    SuperClass::setValues( volume.values() );
}

/*===========================================================================*/
/**
 *  @brief  Removes the quadratic nodes.
 *  @param  volume [in] pointer to the volume data
 */
/*===========================================================================*/
template <typename T>
void TetrahedraToTetrahedra::remove_quadratic_nodes( const vismodule::UnstructuredVolumeObject& volume )
{
    const size_t tet2_ncells = volume.ncells();
    const vismodule::UInt32* tet2_pconnections = volume.connections().pointer();
    const T* tet2_pvalues = static_cast<const T*>( volume.values().pointer() );
    const vismodule::Real32* tet2_pcoords = volume.coords().pointer();

    ::IDMap id_map;
    for ( size_t i = 0; i < tet2_ncells; i++ )
    {
        const vismodule::UInt32 id0 = tet2_pconnections[ 10 * i + 0 ];
        const vismodule::UInt32 id1 = tet2_pconnections[ 10 * i + 1 ];
        const vismodule::UInt32 id2 = tet2_pconnections[ 10 * i + 2 ];
        const vismodule::UInt32 id3 = tet2_pconnections[ 10 * i + 3 ];

        id_map.insert( id0 );
        id_map.insert( id1 );
        id_map.insert( id2 );
        id_map.insert( id3 );
    }

    const size_t tet_ncells = tet2_ncells;
    vismodule::ValueArray<vismodule::UInt32> tet_connections( tet_ncells * 4 );
    vismodule::UInt32* tet_pconnections = tet_connections.pointer();
    for ( size_t i = 0; i < tet2_ncells; i++ )
    {
        const vismodule::UInt32 id0 = tet2_pconnections[ 10 * i + 0 ];
        const vismodule::UInt32 id1 = tet2_pconnections[ 10 * i + 1 ];
        const vismodule::UInt32 id2 = tet2_pconnections[ 10 * i + 2 ];
        const vismodule::UInt32 id3 = tet2_pconnections[ 10 * i + 3 ];

        *(tet_pconnections++) = id_map.bucket().find( id0 )->second;
        *(tet_pconnections++) = id_map.bucket().find( id1 )->second;
        *(tet_pconnections++) = id_map.bucket().find( id2 )->second;
        *(tet_pconnections++) = id_map.bucket().find( id3 )->second;
    }

    const size_t tet_veclen = volume.veclen();
    const size_t tet_nnodes = id_map.bucket().size();
    vismodule::ValueArray<T> tet_values( tet_nnodes * tet_veclen );
    vismodule::ValueArray<vismodule::Real32> tet_coords( tet_nnodes * 3 );
    T* tet_pvalues = tet_values.pointer();
    vismodule::Real32* tet_pcoords = tet_coords.pointer();
    ::IDMap::Bucket::const_iterator id = id_map.bucket().begin();
    while ( id != id_map.bucket().end() )
    {
        // Value array.
        for ( size_t i = 0; i < tet_veclen; i++ )
        {
            tet_pvalues[ id->second * tet_veclen + i ] = tet2_pvalues[ id->first * tet_veclen + i ];
        }

        // Coordinate data array.
        tet_pcoords[ id->second * 3 + 0 ] = tet2_pcoords[ id->first * 3 + 0 ];
        tet_pcoords[ id->second * 3 + 1 ] = tet2_pcoords[ id->first * 3 + 1 ];
        tet_pcoords[ id->second * 3 + 2 ] = tet2_pcoords[ id->first * 3 + 2 ];

        ++id;
    }

    if ( volume.hasMinMaxExternalCoords() )
    {
        const vismodule::Vector3f min_coord( volume.minExternalCoord() );
        const vismodule::Vector3f max_coord( volume.maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( volume.hasMinMaxObjectCoords() )
    {
        const vismodule::Vector3f min_coord( volume.minObjectCoord() );
        const vismodule::Vector3f max_coord( volume.maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    if ( volume.hasMinMaxValues() )
    {
        const vismodule::Real64 min_value( volume.minValue() );
        const vismodule::Real64 max_value( volume.maxValue() );
        SuperClass::setMinMaxValues( min_value, max_value );
    }

    SuperClass::setVeclen( volume.veclen() );
    SuperClass::setNNodes( tet_nnodes );
    SuperClass::setNCells( tet_ncells );
    SuperClass::setCellType( vismodule::UnstructuredVolumeObject::Tetrahedra );
    SuperClass::setCoords( tet_coords );
    SuperClass::setConnections( tet_connections );
    SuperClass::setValues( vismodule::AnyValueArray( tet_values ) );
}

} // end of namespace vismodule
