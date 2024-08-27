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
#ifndef TFS__TRILINEAR_INTERPOLATOR_H_INCLUDE
#define TFS__TRILINEAR_INTERPOLATOR_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/StructuredVolumeObject>
#include <kvs/Vector3>
#include <kvs/Assert>
#include "../kvs_wrapper.h"

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

    kvs::UInt32 m_grid_index_i[SIMDW]; ///< grid index
    kvs::UInt32 m_grid_index_j[SIMDW]; ///< grid index
    kvs::UInt32 m_grid_index_k[SIMDW]; ///< grid index
    kvs::UInt32 m_index[8][SIMDW];   ///< neighbouring grid index
    kvs::Real32 m_weight[8][SIMDW];  ///< weight for the neighbouring grid index

    const float* m_reference_volume;
    const kvs::Vector3ui m_resolution;

public:

    //template <typename T>
    TrilinearInterpolator( const float* volume, const kvs::Vector3ui resolution );

public:

    void attachPoint( const float* p_x, const float* p_y, const float* p_z );

    //const kvs::UInt32* indices( void ) const;

    //template <typename T>
    void scalar( float* values ) const;

    //template <typename T>
    void gradient( float* g_x, float* g_y, float* g_z ) const;

private:

    const int m_line_size;
    const int m_slice_size;
    const int m_imax;
    const int m_jmax;
    const int m_kmax;
    const int id( int i, int j, int k ) const;
};

inline TrilinearInterpolator::TrilinearInterpolator( const float* volume, const kvs::Vector3ui resolution )
    : m_reference_volume( volume )
    , m_resolution( resolution )
    , m_line_size(  resolution.x() )
    , m_slice_size( resolution.x()*resolution.y() )
    , m_imax( resolution.x() - 1 )
    , m_jmax( resolution.y() - 1 )
    , m_kmax( resolution.z() - 1 )

{
}

//高速化のためアクセス範囲のクランプ処理を削除
//補間関数のアクセス範囲は呼び出し側で制御すること
//#pragma ivdep
inline const int TrilinearInterpolator::id( int i, int j, int k ) const
{

    const int I = i<0 ? 0 : i>m_imax ? m_imax : i;
    const int J = j<0 ? 0 : j>m_jmax ? m_jmax : j;
    const int K = k<0 ? 0 : k>m_kmax ? m_kmax : k;

    return I + J*m_line_size + K*m_slice_size;

//    return i + j*m_line_size + k*m_slice_size;
}

//inline void TrilinearInterpolator::attachPoint( const kvs::Vector3f& point )
inline void TrilinearInterpolator::attachPoint( const float* p_x, const float* p_y, const float* p_z )
{
    const kvs::Vector3ui resolution = m_resolution;

    #pragma ivdep
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

        m_index[0][I] = i + j * m_line_size + k * m_slice_size;
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

        m_weight[0][I] = 1.0f - x - y - z + xy + yz + zx - xyz;
        m_weight[1][I] = x - xy - zx + xyz;
        m_weight[2][I] = xy - xyz;
        m_weight[3][I] = y - xy - yz + xyz;
        m_weight[4][I] = z - zx - yz + xyz;
        m_weight[5][I] = zx - xyz;
        m_weight[6][I] = xyz;
        m_weight[7][I] = yz - xyz;

    }
}
/*
inline const kvs::UInt32* TrilinearInterpolator::indices( void ) const
{
    return( m_index );
}
*/

inline void TrilinearInterpolator::scalar( float* values ) const
{
    const float* const data = m_reference_volume;

    #pragma ivdep
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


inline void TrilinearInterpolator::gradient( float* g_x, float* g_y, float* g_z ) const
{
    const float* const s =  m_reference_volume;

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

} // end of namespace kvs

#endif // KVS__TRILINEAR_INTERPOLATOR_H_INCLUDE
