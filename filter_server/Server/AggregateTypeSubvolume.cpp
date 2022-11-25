/****************************************************************************/
/**
 *  @file AggregateTypeSubvolume.cpp
 */
/****************************************************************************/
#include "AggregateTypeSubvolume.h"
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
 *  @brief  Constructs a new GSubvolume object unstructured volume object class.
 */
/*===========================================================================*/
AggregateTypeSubvolume::AggregateTypeSubvolume()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new GSubvolume object unstructured volume object class by reading the given file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
AggregateTypeSubvolume::AggregateTypeSubvolume( const std::string& filename,
                                                const int st, const int vl )
{
    FILE* pfi_fp = NULL;
    char* pfi_name = NULL;
    int32_t cellType;
    int32_t numFiles;
    int32_t number_ingredients;
    int32_t number_subvolumes;
    int32_t numNodes;
    int32_t number_elements;
    int32_t startTimestep;
    int32_t elemNodes;
    size_t  offsetSubvol;
    size_t  offsetTimestep;
    size_t  offsetHalf;
    size_t  numTotalNodes;

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
    // number of files
    fseek( pfi_fp, 4 * 4, SEEK_SET );
    fread( &numFiles, 4, 1, pfi_fp );
    endian2::LittleToHost( &numFiles );
    // number of ingredients
    fseek( pfi_fp, 4 * 5, SEEK_SET );
    fread( &number_ingredients, 4, 1, pfi_fp );
    endian2::LittleToHost( &number_ingredients );
    // start of Timestep
    fseek( pfi_fp, 4 * 6, SEEK_SET );
    fread( &startTimestep, 4, 1, pfi_fp );
    endian2::LittleToHost( &startTimestep );
    // number of subvolumes
    fseek( pfi_fp, 4 * 8, SEEK_SET );
    fread( &number_subvolumes, 4, 1, pfi_fp );
    endian2::LittleToHost( &number_subvolumes );
    // number of nodes
    fseek( pfi_fp, 4 * ( 15 + vl ), SEEK_SET );
    fread( &numNodes, 4, 1, pfi_fp );
    endian2::LittleToHost( &numNodes );
    // number of elements
    fseek( pfi_fp, 4 * ( 15 + number_subvolumes + vl ), SEEK_SET );
    fread( &number_elements, 4, 1, pfi_fp );
    endian2::LittleToHost( &number_elements );

    //-------------------------------------------------------------------------------------------------
    // search vol file
    //-------------------------------------------------------------------------------------------------
    FILE* fp = NULL;
    char* fname = NULL;
    int32_t flen = filename.size() + 21;
    kvs::ValueArray<kvs::Int32> range( ( size_t )2 );

    fname = ( char* )malloc( flen );
    if ( numFiles != number_subvolumes )
    {
        for ( int i = 1; i <= numFiles; i++ )
        {
            sprintf( fname, "%s_%07d_%07d.dat", filename.c_str(), i, numFiles );
            if ( fp = fopen( fname, "rb" ) )
            {
                fread( range.pointer(), 4, 2, fp );
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
                range.swapByte();
#endif
                if ( range.at( 0 ) <= vl && range.at( 1 ) >= vl )
                {
                    // estimate file offset
                    int32_t nnode, nelem;
                    offsetSubvol   = 0;
                    offsetHalf     = 2;
                    offsetTimestep = 0;
                    numTotalNodes  = 0;
                    for ( int j = range.at( 0 ); j <= range.at( 1 ); j++ )
                    {
                        if ( j == vl )
                        {
                            offsetSubvol = offsetHalf;
                            offsetTimestep = numTotalNodes;
                        }
                        // number of nodes
                        fseek( pfi_fp, 4 * ( 15 + j ), SEEK_SET );
                        fread( &nnode, 4, 1, pfi_fp );
                        endian2::LittleToHost( &nnode );
                        // number of elements
                        fseek( pfi_fp, 4 * ( 15 + number_subvolumes + j ), SEEK_SET );
                        fread( &nelem, 4, 1, pfi_fp );
                        endian2::LittleToHost( &nelem );
                        offsetHalf += elemNodes * nelem + nnode * 3;
                        numTotalNodes += nnode * number_ingredients;
                    }
                    offsetSubvol *= 4;
                    offsetTimestep += offsetHalf + ( st - startTimestep ) * numTotalNodes;
                    offsetTimestep *= 4;
                    break;
                }
                fclose( fp );
                fp = NULL;
            }
            else
            {
                fprintf( stderr, "File not exist. (%s)\n", fname );
            }
        }
    }
    else
    {
        sprintf( fname, "%s_%07d_%07d.dat", filename.c_str(), vl + 1, numFiles );
        if ( fp = fopen( fname, "rb" ) )
        {
            fread( range.pointer(), 4, 2, fp );
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
            range.swapByte();
#endif
            if ( range.at( 0 ) <= vl && range.at( 1 ) >= vl )
            {
                // estimate file offset
                int32_t nnode, nelem;
                offsetSubvol   = 0;
                offsetHalf     = 2;
                offsetTimestep = 0;
                numTotalNodes  = 0;
                for ( int j = range.at( 0 ); j <= range.at( 1 ); j++ )
                {
                    if ( j == vl )
                    {
                        offsetSubvol = offsetHalf;
                        offsetTimestep = numTotalNodes;
                    }
                    // number of nodes
                    fseek( pfi_fp, 4 * ( 15 + j ), SEEK_SET );
                    fread( &nnode, 4, 1, pfi_fp );
                    endian2::LittleToHost( &nnode );
                    // number of elements
                    fseek( pfi_fp, 4 * ( 15 + number_subvolumes + j ), SEEK_SET );
                    fread( &nelem, 4, 1, pfi_fp );
                    endian2::LittleToHost( &nelem );
                    offsetHalf += elemNodes * nelem + nnode * 3;
                    numTotalNodes += nnode * number_ingredients;
                }
                offsetSubvol *= 4;
                offsetTimestep += offsetHalf + ( st - startTimestep ) * numTotalNodes;
                offsetTimestep *= 4;
            }
            else
            {
                fclose( fp );
                fp = NULL;
            }
        }
        else
        {
            fprintf( stderr, "File not exist. (%s)\n", fname );
        }
    }

    //-------------------------------------------------------------------------------------------------
    // read vol file
    //-------------------------------------------------------------------------------------------------
    if ( fp )
    {
        // printf("input filename (%s): offset %d %d\n", fname, offsetSubvol, offsetTimestep);
        // printf(" timestep = %d: subvolume = %d: range = [%d, %d]\n", st, vl, range.at(0), range.at(1));
        // printf(" cellType=%d: numFiles=%d: number_ingredients=%d: numSubvolumes=%d: numNodes=%d: number_elements=%d\n",
        //        cellType, numFiles, number_ingredients, number_subvolumes, numNodes, number_elements);
        // connecion data
        if ( !m_connections.allocate( elemNodes * number_elements ) )
        {
            kvsMessageError( "Cannot allocate memory for the connection data." );
            goto AggregateTypeSubvolume_ERROR;
        }
        fseek( fp, offsetSubvol, SEEK_SET );
        if ( fread( m_connections.pointer(), 4, elemNodes * number_elements, fp ) != elemNodes * number_elements )
        {
            kvsMessageError( "File read error: connection data." );
        }
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
        m_connections.swapByte();
#endif
        // coordinate data
        if ( !m_coords.allocate( numNodes * 3 ) )
        {
            kvsMessageError( "Cannot allocate memory for the coordinate data." );
            goto AggregateTypeSubvolume_ERROR;
        }
        if ( fread( m_coords.pointer(), 4, numNodes * 3, fp ) != numNodes * 3 )
        {
            kvsMessageError( "File read error: coordinate data." );
        }
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
        m_coords.swapByte();
#endif
        // value data
        if ( !m_values.allocate<kvs::Real32>( numNodes * number_ingredients ) )
        {
            kvsMessageError( "Cannot allocate memory for the value data." );
            goto AggregateTypeSubvolume_ERROR;
        }
        fseek( fp, offsetTimestep, SEEK_SET );
        if ( fread( m_values.pointer(), 4, numNodes * number_ingredients, fp ) != numNodes * number_ingredients )
        {
            kvsMessageError( "File read error: value data." );
        }
#if PBVR_BYTE_ORDER == PBVR_BIG_ENDIAN
        m_values.swapByte();
#endif
        // num nodes
        m_nnodes = numNodes;
        // veclen
        m_vector_length = number_ingredients;
        // num cells
        m_ncells = number_elements;

    }
    else
    {
        kvsMessageError( "Cannot open file '%s'.", fname );
        goto AggregateTypeSubvolume_ERROR;
    }

    m_is_success = true;
    if ( pfi_fp ) fclose( pfi_fp );
    if ( pfi_name ) free( pfi_name );
    if ( fp ) fclose( fp );
    if ( fname ) free( fname );
    return;

AggregateTypeSubvolume_ERROR:
    if ( pfi_fp ) fclose( pfi_fp );
    if ( pfi_name ) free( pfi_name );
    if ( fp ) fclose( fp );
    if ( fname ) free( fname );
    m_is_success = false;
}

/*===========================================================================*/
/**
 *  @brief  Destructs the GSubvolume object unstructured volume object class.
 */
/*===========================================================================*/
AggregateTypeSubvolume::~AggregateTypeSubvolume()
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the cell type as string.
 *  @return cell type
 */
/*===========================================================================*/
const std::string AggregateTypeSubvolume::cellType() const
{
    return m_cell_type;
}

/*===========================================================================*/
/**
 *  @brief  Returns the vector length.
 *  @return vector length
 */
/*===========================================================================*/
const size_t AggregateTypeSubvolume::veclen() const
{
    return m_vector_length;
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of nodes.
 *  @return number of nodes
 */
/*===========================================================================*/
const size_t AggregateTypeSubvolume::nnodes() const
{
    return m_nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Returns the number of cells.
 *  @return number of cells
 */
/*===========================================================================*/
const size_t AggregateTypeSubvolume::ncells() const
{
    return m_ncells;
}

/*===========================================================================*/
/**
 *  @brief  Returns the value array.
 *  @return value array
 */
/*===========================================================================*/
const kvs::AnyValueArray& AggregateTypeSubvolume::values() const
{
    return m_values;
}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate array.
 *  @return coordinate array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& AggregateTypeSubvolume::coords() const
{
    return m_coords;
}

/*===========================================================================*/
/**
 *  @brief  Returns the connection array.
 *  @return connection array
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& AggregateTypeSubvolume::connections() const
{
    return m_connections;
}

/*===========================================================================*/
/**
 *  @brief  Sets a cell type.
 *  @param  cell_type [in] cell type
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setCellType( const std::string& cell_type )
{
    m_cell_type = cell_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a vector length.
 *  @param  veclen [in] vector length
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setVeclen( const size_t veclen )
{
    m_vector_length = veclen;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of nodes.
 *  @param  nnodes [in] number of nodes
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setNNodes( const size_t nnodes )
{
    m_nnodes = nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Sets a number of cells.
 *  @param  ncells [in] number of cells
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setNCells( const size_t ncells )
{
    m_ncells = ncells;
}

/*===========================================================================*/
/**
 *  @brief  Sets a value array.
 *  @param  values [in] value array
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setValues( const kvs::AnyValueArray& values )
{
    m_values = values;
}

/*===========================================================================*/
/**
 *  @brief  Sets a coordinate array.
 *  @param  coords [in] coordinate array
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Sets a connection array.
 *  @param  connections [in] connection array
 */
/*===========================================================================*/
void AggregateTypeSubvolume::setConnections( const kvs::ValueArray<kvs::UInt32>& connections )
{
    m_connections = connections;
}

const bool AggregateTypeSubvolume::read( const std::string& filename )
{
    return false;
}
const bool AggregateTypeSubvolume::write( const std::string& filename )
{
    return false;
}
} // end of namespace kvs
