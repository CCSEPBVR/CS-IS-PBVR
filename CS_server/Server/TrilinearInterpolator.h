/****************************************************************************/
/**
 *  @file TrilinearInterpolator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TrilinearInterpolator.h 653 2010-10-29 14:17:13Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TRILINEAR_INTERPOLATOR_H_INCLUDE
#define KVS__TRILINEAR_INTERPOLATOR_H_INCLUDE

#include <kvs/ClassName>
//#include <kvs/StructuredVolumeObject>
#include "StructuredVolumeObject.h"
#include <kvs/Vector3>
#include <kvs/Assert>
//#include "../kvs_wrapper.h"

#ifndef SIMDW 
#define SIMDW 128
#endif


namespace TFS
{

/*==========================================================================*/
/**
 *  Trilinear interpolation class.
 *高速化のためアクセス範囲のクランプ処理を削除
 *補間関数のアクセス範囲は呼び出し側で制御すること
 */
/*==========================================================================*/
class TrilinearInterpolator
{
    //kvsClassName_without_virtual( kvs::TrilinearInterpolator );

private:

    kvs::Vector3ui m_grid_index; ///< grid index
//    kvs::UInt32    m_index[8];   ///< neighbouring grid index
//    kvs::Real32    m_weight[8];  ///< weight for the neighbouring grid index
    kvs::Real32    m_scalars[8]; 
    float*          m_data;

    kvs::UInt32 m_grid_index_i[SIMDW]; ///< grid index
    kvs::UInt32 m_grid_index_j[SIMDW]; ///< grid index
    kvs::UInt32 m_grid_index_k[SIMDW]; ///< grid index
    kvs::UInt32 m_leaf_index;
    kvs::Real32 m_cell_length;
    kvs::UInt32 m_index[8][SIMDW];   ///< neighbouring grid index
    kvs::Real32 m_weight[8][SIMDW];  ///< weight for the neighbouring grid index
    //kvs::Real32 m_differential_functions[24][SIMDW];
    kvs::Real32 m_dNdx[8][SIMDW];
    kvs::Real32 m_dNdy[8][SIMDW];
    kvs::Real32 m_dNdz[8][SIMDW];

    const float*         m_reference_volume;
//    const pbvr::StructuredVolumeObject& m_reference_volume;
    const kvs::Vector3ui m_resolution; ///< resolution 3D
    const int m_line_size;
    const int m_slice_size;
    const int m_leaf_size;
    const int m_imax;
    const int m_jmax;
    const int m_kmax;

public:

    TrilinearInterpolator( const float* volume, const kvs::Vector3ui resolution );
//    TrilinearInterpolator( const pbvr::StructuredVolumeObject& volume );
//    TrilinearInterpolator( T* values,
//              float* coords, int ncoords, int ncells);

public:

    void setLeafIndex( const int leaf_index );

    void setCellLength( const float cell_length );

    void attachPoint( const float* p_x, const float* p_y, const float* p_z );

    //const kvs::UInt32* indices( void ) const;

    //template <typename T>
    void scalar( float* values ) const;

    //template <typename T>
    void gradient( float* g_x, float* g_y, float* g_z ) const;

private:

    const int id( const int i, const int j, const int k ) const;
};

//inline TrilinearInterpolator::TrilinearInterpolator( const pbvr::StructuredVolumeObject& volume ) 
//    : m_grid_index( 0, 0, 0 )
//    , m_reference_volume( volume )
//{
//    m_data = new float[m_reference_volume.nnodes()* m_reference_volume.veclen()];
//    for ( int j = 0; j < m_reference_volume.veclen(); j++ )
//    {
//        for ( int i = 0; i < m_reference_volume.nnodes(); i++ )
//        {
//            int  it = j * m_reference_volume.nnodes() + i;
//            m_data[it] = (float)(m_reference_volume.values().at<double>(it));  
//        }
//    }
//
//}

inline TrilinearInterpolator::TrilinearInterpolator( const float* volume, const kvs::Vector3ui resolution )
    : m_reference_volume( volume )
    , m_resolution( resolution )
    , m_line_size ( resolution.x() )
    , m_slice_size( resolution.x() * resolution.y() )
    , m_leaf_size ( resolution.x() * resolution.y() * resolution.z() )
    , m_imax( resolution.x() - 1 )
    , m_jmax( resolution.y() - 1 )
    , m_kmax( resolution.z() - 1 )
{
}

inline void TrilinearInterpolator::setLeafIndex( const int leaf_index )
{
    m_leaf_index = leaf_index;
}

inline void TrilinearInterpolator::setCellLength( const float cell_length )
{
    m_cell_length = cell_length;
}

//高速化のためアクセス範囲のクランプ処理を削除
//補間関数のアクセス範囲は呼び出し側で制御すること
#pragma ivdep
inline const int TrilinearInterpolator::id( const int i, const int j, const int k ) const
{

    const int I = i<0 ? 0 : i>m_imax ? m_imax : i;
    const int J = j<0 ? 0 : j>m_jmax ? m_jmax : j;
    const int K = k<0 ? 0 : k>m_kmax ? m_kmax : k;

    return I + J*m_line_size + K*m_slice_size;

//    return i + j*m_line_size + k*m_slice_size;
}

//inline void TrilinearInterpolator::attachPoint( const kvs::Vector3f& point )
#pragma ivdep
inline void TrilinearInterpolator::attachPoint( const float* p_x, const float* p_y, const float* p_z )
{
    const kvs::Vector3ui resolution = m_resolution;

    for( int I=0; I < SIMDW; I++ )
    {
        // Temporary index.
        const size_t ti = static_cast<size_t>( p_x[I] );
        const size_t tj = static_cast<size_t>( p_y[I] );
        const size_t tk = static_cast<size_t>( p_z[I] );

        // Addjustment index for boundary.
        const size_t i = ( ti >= resolution.x() - 1 ) ? resolution.x() - 2 : ti;
        const size_t j = ( tj >= resolution.y() - 1 ) ? resolution.y() - 2 : tj;
        const size_t k = ( tk >= resolution.z() - 1 ) ? resolution.z() - 2 : tk;

        // Calculate index.
        m_grid_index_i[I] = i;
        m_grid_index_j[I] = j;
        m_grid_index_k[I] = k;

        m_index[0][I] = i + j * m_line_size + k * m_slice_size + m_leaf_index * m_leaf_size;
        m_index[1][I] = m_index[0][I] + 1;
        m_index[2][I] = m_index[1][I] + m_line_size;
        m_index[3][I] = m_index[0][I] + m_line_size;
        m_index[4][I] = m_index[0][I] + m_slice_size;
        m_index[5][I] = m_index[1][I] + m_slice_size;
        m_index[6][I] = m_index[2][I] + m_slice_size;
        m_index[7][I] = m_index[3][I] + m_slice_size;

        // Calculate local coordinate.
        const float x = p_x[I] - i;
        const float y = p_y[I] - j;
        const float z = p_z[I] - k;

        const float xy = x * y;
        const float yz = y * z;
        const float zx = z * x;

        const float xyz = xy * z;

        m_weight[0][I] = 1 - x - y - z + xy + yz + zx - xyz;
        m_weight[1][I] =     x         - xy      - zx + xyz;
        m_weight[2][I] =                 xy           - xyz;
        m_weight[3][I] =         y     - xy - yz      + xyz;
        m_weight[4][I] =             z      - yz - zx + xyz;
        m_weight[5][I] =                           zx - xyz;
        m_weight[6][I] =                                xyz;
        m_weight[7][I] =                      yz      - xyz;

        // dNdx
        m_dNdx[ 0][I] = - 1 + y + z - yz;
        m_dNdx[ 1][I] =   1 - y - z + yz;
        m_dNdx[ 2][I] =       y     - yz;
        m_dNdx[ 3][I] =     - y     + yz;
        m_dNdx[ 4][I] =         - z + yz;
        m_dNdx[ 5][I] =           z - yz;
        m_dNdx[ 6][I] =               yz;
        m_dNdx[ 7][I] =             - yz;

        // dNdy
        m_dNdy[ 0][I] = - 1 + x + z - zx;
        m_dNdy[ 1][I] =     - x     + zx;
        m_dNdy[ 2][I] =       x     - zx;
        m_dNdy[ 3][I] =   1 - x - z + zx;
        m_dNdy[ 4][I] =         - z + zx;
        m_dNdy[ 5][I] =             - zx;
        m_dNdy[ 6][I] =               zx;
        m_dNdy[ 7][I] =           z - zx;

        // dNdz
        m_dNdz[ 0][I] = - 1 + y + x - xy;
        m_dNdz[ 1][I] =         - x + xy;
        m_dNdz[ 2][I] =             - xy;
        m_dNdz[ 3][I] =     - y     + xy;
        m_dNdz[ 4][I] =   1 - y - x + xy;
        m_dNdz[ 5][I] =           x - xy;
        m_dNdz[ 6][I] =               xy;
        m_dNdz[ 7][I] =       y     - xy;
    }
}
/*
inline const kvs::UInt32* TrilinearInterpolator::indices( void ) const
{
    return( m_index );
}
*/
//template <typename T>
#pragma ivdep
inline void TrilinearInterpolator::scalar( float* values ) const
{
    //const T* const data = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );
    const float* const data = m_reference_volume;

    for( int I = 0; I < SIMDW; I++ )
    {
        values[I] =
            static_cast<float>(
                data[ m_index[0][I] ] * m_weight[0][I] +
                data[ m_index[1][I] ] * m_weight[1][I] +
                data[ m_index[2][I] ] * m_weight[2][I] +
                data[ m_index[3][I] ] * m_weight[3][I] +
                data[ m_index[4][I] ] * m_weight[4][I] +
                data[ m_index[5][I] ] * m_weight[5][I] +
                data[ m_index[6][I] ] * m_weight[6][I] +
                data[ m_index[7][I] ] * m_weight[7][I] );
    }
}

#pragma ivdep
inline void TrilinearInterpolator::gradient( float* g_x, float* g_y, float* g_z ) const
{
    // Calculate a gradient vector in the local coordinate.
    const float* const data = m_reference_volume;
    const kvs::UInt32 nnodes = 8;
    const float inv_Jacobi = 1.0 / m_cell_length;

    for( int I = 0; I < SIMDW; I++ )
    {
        float dsdx = 0.0f;
        float dsdy = 0.0f;
        float dsdz = 0.0f;

        for ( size_t i = 0; i < nnodes; i++ )
        {
            dsdx += data[ m_index[i][I] ] * m_dNdx[i][I];
            dsdy += data[ m_index[i][I] ] * m_dNdy[i][I];
            dsdz += data[ m_index[i][I] ] * m_dNdz[i][I];
        }

        g_x[I] = inv_Jacobi * dsdx;
        g_y[I] = inv_Jacobi * dsdy;
        g_z[I] = inv_Jacobi * dsdz;
    }
}
/*
template <typename T>
    inline void TrilinearInterpolator::gradient( float* g_x, float* g_y, float* g_z ) const
{
    const T* const s = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );

    float dsdx[8][SIMDW], dsdy[8][SIMDW], dsdz[8][SIMDW];

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[0]
        const int i = m_grid_index_i[I];
        const int j = m_grid_index_j[I];
        const int k = m_grid_index_k[I];

        dsdx[0][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[0][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[0][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[1]
        const int i = m_grid_index_i[I]+1;
        const int j = m_grid_index_j[I];
        const int k = m_grid_index_k[I];

        dsdx[1][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[1][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[1][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[2]
        const int i = m_grid_index_i[I]+1;
        const int j = m_grid_index_j[I]+1;
        const int k = m_grid_index_k[I];

        dsdx[2][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[2][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[2][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[3]
        const int i = m_grid_index_i[I];
        const int j = m_grid_index_j[I]+1;
        const int k = m_grid_index_k[I];

        dsdx[3][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[3][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[3][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[4]
        const int i = m_grid_index_i[I];
        const int j = m_grid_index_j[I];
        const int k = m_grid_index_k[I]+1;

        dsdx[4][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[4][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[4][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[5]
        const int i = m_grid_index_i[I]+1;
        const int j = m_grid_index_j[I];
        const int k = m_grid_index_k[I]+1;

        dsdx[5][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[5][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[5][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[6]
        const int i = m_grid_index_i[I]+1;
        const int j = m_grid_index_j[I]+1;
        const int k = m_grid_index_k[I]+1;

        dsdx[6][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[6][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[6][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I=0; I<SIMDW; I++ )
    {
        //m_index[7]
        const int i = m_grid_index_i[I];
        const int j = m_grid_index_j[I]+1;
        const int k = m_grid_index_k[I]+1;

        dsdx[7][I] = (float)s[id(i+1,j,k)] - (float)s[id(i-1,j,k)];
        dsdy[7][I] = (float)s[id(i,j+1,k)] - (float)s[id(i,j-1,k)];
        dsdz[7][I] = (float)s[id(i,j,k+1)] - (float)s[id(i,j,k-1)];
    }

    #pragma ivdep
    for( int I = 0; I < SIMDW; I++ )
    {
        g_x[I] =
            - m_weight[0][I] * dsdx[0][I] - m_weight[1][I] * dsdx[1][I]
            - m_weight[2][I] * dsdx[2][I] - m_weight[3][I] * dsdx[3][I] 
            - m_weight[4][I] * dsdx[4][I] - m_weight[5][I] * dsdx[5][I] 
            - m_weight[6][I] * dsdx[6][I] - m_weight[7][I] * dsdx[7][I];
    }

    #pragma ivdep
    for( int I = 0; I < SIMDW; I++ )
    {
        g_y[I] = 
            - m_weight[0][I] * dsdy[0][I] - m_weight[1][I] * dsdy[1][I]
            - m_weight[2][I] * dsdy[2][I] - m_weight[3][I] * dsdy[3][I]
            - m_weight[4][I] * dsdy[4][I] - m_weight[5][I] * dsdy[5][I]
            - m_weight[6][I] * dsdy[6][I] - m_weight[7][I] * dsdy[7][I];
    }

    #pragma ivdep
    for( int I = 0; I < SIMDW; I++ )
    {
        g_z[I] =
            - m_weight[0][I] * dsdz[0][I] - m_weight[1][I] * dsdz[1][I]
            - m_weight[2][I] * dsdz[2][I] - m_weight[3][I] * dsdz[3][I]
            - m_weight[4][I] * dsdz[4][I] - m_weight[5][I] * dsdz[5][I]
            - m_weight[6][I] * dsdz[6][I] - m_weight[7][I] * dsdz[7][I];
    }
}
*/
} // end of namespace kvs

#endif // KVS__TRILINEAR_INTERPOLATOR_H_INCLUDE
