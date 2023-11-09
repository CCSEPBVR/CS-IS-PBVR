/****************************************************************************/
/**
 *  @file StepAggregateTypeSubvolume.cpp
 */
/****************************************************************************/
#include "StepAggregateTypeSubvolume.h"
#include <Core/FileFormat/KVSML/UnstructuredVolumeObjectTag.h>
#include <Core/FileFormat/KVSML/NodeTag.h>
#include <Core/FileFormat/KVSML/CellTag.h>
#include <Core/FileFormat/KVSML/ValueTag.h>
#include <Core/FileFormat/KVSML/CoordTag.h>
#include <Core/FileFormat/KVSML/ConnectionTag.h>
#include "DataArrayTag.h"
#include <kvs/File>
#include <kvs/AnyValueArray>
#include <kvs/Type>
#include "Types.h"
#include <kvs/IgnoreUnusedVariable>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace
{

/*===========================================================================*/
/**
 *  @brief  Returns the number of nodes per element.
 *  @param  cell_type [in] cell type as string
 *  @return number of nodes per element
 */
/*===========================================================================*/
inline const size_t GetNumberOfNodesPerElement( const std::string& cell_type )
{
    size_t nnodes = 0;
    if ( cell_type == "tetrahedra" ) nnodes = 4;
    else if ( cell_type == "quadratic tetrahedra" ) nnodes = 10;
    else if ( cell_type == "hexahedra" ) nnodes = 8;
    else if ( cell_type == "quadratic hexahedra" ) nnodes = 20;
    else if ( cell_type == "prism" ) nnodes = 6;
    else if ( cell_type == "pyramid" ) nnodes = 5;
    else if ( cell_type == "point" ) nnodes = 1;
    else if ( cell_type == "triangle" ) nnodes = 3;
    else if ( cell_type == "triangle2" ) nnodes = 6;
    else if ( cell_type == "quadratic" ) nnodes = 4;
    else if ( cell_type == "quadratic2" ) nnodes = 8;

    return nnodes;
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new GTimestep object unstructured volume object class.
 */
/*===========================================================================*/
StepAggregateTypeSubvolume::StepAggregateTypeSubvolume()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GTimestep object unstructured volume object class by reading the given file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
StepAggregateTypeSubvolume::StepAggregateTypeSubvolume( const std::string& filename,
                                                        const int st, const int vl )
{
    FILE* pfi_fp = NULL;
    char* pfi_name = NULL;
    int32_t cellType;
    int32_t m_number_ingredients;
    int32_t m_number_subvolumes;
    int32_t numNodes;
    int32_t m_number_elements;
    int32_t elemNodes;
    size_t  numPreElements;
    size_t  numPreNodes;
    size_t  offset;

    //-------------------------------------------------------------------------------------------------
    // load pfi file
    //-------------------------------------------------------------------------------------------------
    pfi_name = ( char* )malloc( filename.size() + 5 );
    sprintf( pfi_name, "%s.pfi", filename.c_str() );
    if ( ( pfi_fp = fopen( pfi_name, "rb" ) ) == NULL )
    {
        m_is_success = false;
        kvsMessageError( "Cannot read file '%s'.", pfi_name );
        free( pfi_name );
        return;
    }
    // Cell type
    fseek( pfi_fp, 4 * 2, SEEK_SET );
    fread( &cellType, 4, 1, pfi_fp );
    endian2::LittleToHost( &cellType );
    switch ( cellType )
    {
    case 2:
        elemNodes = 3;
        setCellType( "triangle" );
        break;
    case 3:
        elemNodes = 4;
        setCellType( "quadratic" );
        break;
    case 4:
        elemNodes = 4;
        setCellType( "tetrahedra" );
        break;
    case 5:
        elemNodes = 5;
        setCellType( "pyramid" );
        break;
    case 6:
        elemNodes = 6;
        setCellType( "prism" );
        break;
    case 7:
        elemNodes = 8;
        setCellType( "hexahedra" );
        break;
    case 9:
        elemNodes = 6;
        setCellType( "triangle2" );
        break;
    case 10:
        elemNodes = 8;
        setCellType( "quadratic2" );
        break;
    case 11:
        elemNodes = 10;
        setCellType( "quadratic tetrahedra" );
        break;
    case 14:
        elemNodes = 20;
        setCellType( "quadratic hexahedra" );
        break;
    default:
        kvsMessageError( "Unknown cell type '%d'.", cellType );
        break;
    }
    // number of ingredients
    fseek( pfi_fp, 4 * 5, SEEK_SET );
    fread( &m_number_ingredients, 4, 1, pfi_fp );
    endian2::LittleToHost( &m_number_ingredients );
    // number of subvolumes
    fseek( pfi_fp, 4 * 8, SEEK_SET );
    fread( &m_number_subvolumes, 4, 1, pfi_fp );
    endian2::LittleToHost( &m_number_subvolumes );
    // number of nodes
    numPreNodes = 0;
    fseek( pfi_fp, 4 * 15, SEEK_SET );
    for ( int i = 0; i < vl; i++ )
    {
        fread( &numNodes, 4, 1, pfi_fp );
        endian2::LittleToHost( &numNodes );
        numPreNodes += numNodes;
    }
    fseek( pfi_fp, 4 * ( 15 + vl ), SEEK_SET );
    fread( &numNodes, 4, 1, pfi_fp );
    endian2::LittleToHost( &numNodes );
    // number of elements
    numPreElements = 0;
    fseek( pfi_fp, 4 * ( 15 + m_number_subvolumes ), SEEK_SET );
    for ( int i = 0; i < vl; i++ )
    {
        fread( &m_number_elements, 4, 1, pfi_fp );
        endian2::LittleToHost( &m_number_elements );
        numPreElements += m_number_elements;
    }
    fseek( pfi_fp, 4 * ( 15 + m_number_subvolumes + vl ), SEEK_SET );
    fread( &m_number_elements, 4, 1, pfi_fp );
    endian2::LittleToHost( &m_number_elements );

    // printf(" cellType=%d: m_number_ingredients=%d: numSubvolumes=%d: numNodes=%d: m_number_elements=%d: elemNodes=%d\n",
    //        cellType, m_number_ingredients, m_number_subvolumes, numNodes, m_number_elements, elemNodes);
    //-------------------------------------------------------------------------------------------------
    // read connect file
    //-------------------------------------------------------------------------------------------------
    FILE* fp = NULL;
    char* fname = NULL;
    int32_t flen = filename.size() + 17;
    fname = ( char* )malloc( flen );

    sprintf( fname, "%s_connect.dat", filename.c_str() );
    if ( !( fp = fopen( fname, "rb" ) ) )
    {
        kvsMessageError( "Cannot open file '%s'.", fname );
        goto StepAggregateTypeSubvolume_ERROR;
    }
    // connecion data
    if ( !m_connections.allocate( elemNodes * m_number_elements ) )
    {
        kvsMessageError( "Cannot allocate memory for the connection data." );
        goto StepAggregateTypeSubvolume_ERROR;
    }
    offset = numPreElements * elemNodes * 4;
    // printf("connect filename (%s): offset %d\n", fname, offset);
    fseek( fp, offset, SEEK_SET );
    if ( fread( m_connections.pointer(), 4, elemNodes * m_number_elements, fp ) != elemNodes * m_number_elements )
    {
        kvsMessageError( "File read error: connection data." );
        goto StepAggregateTypeSubvolume_ERROR;
    }
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    m_connections.swapByte();
#endif
    fclose( fp );
    fp = NULL;

    //-------------------------------------------------------------------------------------------------
    // read coordinate file
    //-------------------------------------------------------------------------------------------------
    sprintf( fname, "%s_coord.dat", filename.c_str() );
    if ( !( fp = fopen( fname, "rb" ) ) )
    {
        kvsMessageError( "Cannot open file '%s'.", fname );
        goto StepAggregateTypeSubvolume_ERROR;
    }
    // coordinate data
    if ( !m_coords.allocate( numNodes * 3 ) )
    {
        kvsMessageError( "Cannot allocate memory for the coordinate data." );
        goto StepAggregateTypeSubvolume_ERROR;
    }
    offset = numPreNodes * 3 * 4;
    // printf("coordinate filename (%s): offset %d\n", fname, offset);
    fseek( fp, offset, SEEK_SET );
    if ( fread( m_coords.pointer(), 4, numNodes * 3, fp ) != numNodes * 3 )
    {
        kvsMessageError( "File read error: coordinate data." );
        goto StepAggregateTypeSubvolume_ERROR;
    }
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    m_coords.swapByte();
#endif
    fclose( fp );
    fp = NULL;

    //-------------------------------------------------------------------------------------------------
    // read value file
    //-------------------------------------------------------------------------------------------------
    sprintf( fname, "%s_%05d_value.dat", filename.c_str(), st );
    if ( !( fp = fopen( fname, "rb" ) ) )
    {
        kvsMessageError( "Cannot open file '%s'.", fname );
        goto StepAggregateTypeSubvolume_ERROR;
    }
    // value data
    if ( !m_values.allocate<kvs::Real32>( numNodes * m_number_ingredients ) )
    {
        kvsMessageError( "Cannot allocate memory for the value data." );
        goto StepAggregateTypeSubvolume_ERROR;
    }
    offset = numPreNodes * m_number_ingredients * 4;
    // printf("value filename (%s): offset %d\n", fname, offset);
    fseek( fp, offset, SEEK_SET );
    if ( fread( m_values.pointer(), 4, numNodes * m_number_ingredients, fp ) != numNodes * m_number_ingredients )
    {
        kvsMessageError( "File read error: value data." );
        goto StepAggregateTypeSubvolume_ERROR;
    }
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
    m_values.swapByte();
#endif

    // num nodes
    m_nnodes = numNodes;
    // veclen
    m_vector_length = m_number_ingredients;
    // num cells
    m_ncells = m_number_elements;

    m_is_success = true;
    if ( pfi_fp ) fclose( pfi_fp );
    if ( pfi_name ) free( pfi_name );
    if ( fp ) fclose( fp );
    if ( fname ) free( fname );
    return;

StepAggregateTypeSubvolume_ERROR:
    if ( pfi_fp ) fclose( pfi_fp );
    if ( pfi_name ) free( pfi_name );
    if ( fp ) fclose( fp );
    if ( fname ) free( fname );
    m_is_success = false;
}

/*===========================================================================*/
/**
 *  @brief  Destructs the GTimestep object unstructured volume object class.
 */
/*===========================================================================*/
StepAggregateTypeSubvolume::~StepAggregateTypeSubvolume()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the cell type as string.
 *  @return cell type
 */
/*===========================================================================*/
const std::string StepAggregateTypeSubvolume::cellType() const
{
    return m_cell_type;
}

/*===========================================================================*/
/**
 *  @brief  Returns the vector length.
 *  @return vector length
 */
/*===========================================================================*/
const size_t StepAggregateTypeSubvolume::veclen() const
{
    return m_vector_length;
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of nodes.
 *  @return number of nodes
 */
/*===========================================================================*/
const size_t StepAggregateTypeSubvolume::nnodes() const
{
    return m_nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of cells.
 *  @return number of cells
 */
/*===========================================================================*/
const size_t StepAggregateTypeSubvolume::ncells() const
{
    return m_ncells;
}

/*===========================================================================*/
/**
 *  @brief  Returns the value array.
 *  @return value array
 */
/*===========================================================================*/
const kvs::AnyValueArray& StepAggregateTypeSubvolume::values() const
{
    return m_values;
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate array.
 *  @return coordinate array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& StepAggregateTypeSubvolume::coords() const
{
    return m_coords;
}

/*===========================================================================*/
/**
 *  @brief  Returns the connection array.
 *  @return connection array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& StepAggregateTypeSubvolume::connections() const
{
    return m_connections;
}

/*===========================================================================*/
/**
 *  @brief  Sets a cell type.
 *  @param  cell_type [in] cell type
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setCellType( const std::string& cell_type )
{
    m_cell_type = cell_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a vector length.
 *  @param  veclen [in] vector length
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setVeclen( const size_t veclen )
{
    m_vector_length = veclen;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of nodes.
 *  @param  nnodes [in] number of nodes
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setNNodes( const size_t nnodes )
{
    m_nnodes = nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of cells.
 *  @param  ncells [in] number of cells
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setNCells( const size_t ncells )
{
    m_ncells = ncells;
}

/*===========================================================================*/
/**
 *  @brief  Sets a value array.
 *  @param  values [in] value array
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setValues( const kvs::AnyValueArray& values )
{
    m_values = values;
}

/*===========================================================================*/
/**
 *  @brief  Sets a coordinate array.
 *  @param  coords [in] coordinate array
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a connection array.
 *  @param  connections [in] connection array
 */
/*===========================================================================*/
void StepAggregateTypeSubvolume::setConnections( const kvs::ValueArray<kvs::UInt32>& connections )
{
    m_connections = connections;
}

const bool StepAggregateTypeSubvolume::read( const std::string& filename )
{
    return false;
}
const bool StepAggregateTypeSubvolume::write( const std::string& filename )
{
    return false;
}
} // end of namespace kvs
