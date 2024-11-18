/*****************************************************************************/
/**
 *  @file   MeshData.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MeshData.cpp 865 2011-07-22 01:41:18Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "MeshData.h"
#include "File.h"


namespace kvs
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  Construct a new MeshData class.
 */
/*===========================================================================*/
MeshData::MeshData( void ):
    m_dimensions( 0 ),
    m_nnodes_per_element( 0 ),
    m_nnodes( 0 ),
    m_nelements( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Construct a new MeshData class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
MeshData::MeshData( const std::string filename ):
    m_dimensions( 0 ),
    m_nnodes_per_element( 0 ),
    m_nnodes( 0 ),
    m_nelements( 0 )
{
    this->read( filename );
}

/*===========================================================================*/
/**
 *  @brief  Return dimensions.
 *  @return dimensions
 */
/*===========================================================================*/
const size_t MeshData::dimensions( void ) const
{
    return( m_dimensions );
}

/*===========================================================================*/
/**
 *  @brief  Return number of nodes per element.
 *  @return number of nodes per element
 */
/*===========================================================================*/
const size_t MeshData::nnodesPerElement( void ) const
{
    return( m_nnodes_per_element );
}

/*===========================================================================*/
/**
 *  @brief  Return number of nodes.
 *  @return number of nodes
 */
/*===========================================================================*/
const size_t MeshData::nnodes( void ) const
{
    return( m_nnodes );
}

/*===========================================================================*/
/**
 *  @brief  Return number of elements.
 *  @return number of elements
 */
/*===========================================================================*/
const size_t MeshData::nelements( void ) const
{
    return( m_nelements );
}

/*===========================================================================*/
/**
 *  @brief  Return coordinate values.
 *  @return coordinate values
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::Real32>& MeshData::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Return connection values.
 *  @return connection values
 */
/*===========================================================================*/
const kvs::ValueArray<kvs::UInt32>& MeshData::connections( void ) const
{
    return( m_connections );
}

/*===========================================================================*/
/**
 *  @brief  Read mesh data file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool MeshData::read( const std::string filename )
{
    kvs::gf::File file;
    if ( !file.read( filename ) )
    {
        kvsMessageError("Cannot read mesh data file.");
        return( false );
    }

    if ( file.dataSetList().size() != 1 )
    {
        kvsMessageError("Multiple data set is not supported in kvs::gf::MeshData.");
        return( false );
    }

    const kvs::gf::DataSet& data_set = file.dataSet(0);
    for ( size_t i = 0; i < data_set.dataList().size(); i++ )
    {
        const kvs::gf::Data& data = data_set.data(i);
        const std::string& keyword = data.keyword();
        const std::string& type = data.arrayTypeHeader();

        // Read coordinate values.
        if ( keyword == "*GRID_2D" || keyword == "*GRID_3D" )
        {
            if ( type == "#FLT_ARY" )
            {
                m_dimensions = data.num();
                m_nnodes = data.num2();
                m_coords = data.fltArray();
            }
        }

        // Read connection values.
        if ( keyword == "*NODE_2D" || keyword == "*NODE_3D" )
        {
            if ( type == "#INT_ARY" )
            {
                m_nnodes_per_element = data.num();
                m_nelements = data.num2();
                const kvs::Int32* src = data.intArray().pointer();
                const size_t size = data.intArray().size();
                kvs::UInt32* dst = m_connections.allocate( size );
                for ( size_t i = 0; i < size; i++ ) dst[i] = static_cast<kvs::UInt32>( src[i] - 1 );
            }
        }
    }

    return( true );
}

} // end of namespace gf

} // end of namespace kvs
