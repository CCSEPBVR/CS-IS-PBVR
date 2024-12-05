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
#ifndef VIS_MODULE__TRILINEAR_INTERPOLATOR_H_INCLUDE
#define VIS_MODULE__TRILINEAR_INTERPOLATOR_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/Vector3>
#include <vismodule/Assert>

#ifndef SIMDW 
#define SIMDW 128
#endif


namespace vismodule
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
    visModuleClassName_without_virtual( vismodule::TrilinearInterpolator );

private:

    vismodule::Vector3ui m_grid_index; ///< grid index
    vismodule::UInt32    m_index_woSIMD[8];   ///< neighbouring grid index
    vismodule::Real32    m_weight_woSIMD[8];  ///< weight for the neighbouring grid index
    vismodule::Real32    m_scalars[8]; 
    float*          m_data;

    vismodule::UInt32 m_grid_index_i[SIMDW]; ///< grid index
    vismodule::UInt32 m_grid_index_j[SIMDW]; ///< grid index
    vismodule::UInt32 m_grid_index_k[SIMDW]; ///< grid index
    vismodule::UInt32 m_leaf_index;
    vismodule::Real32 m_cell_length;
    vismodule::UInt32 m_index[8][SIMDW];   ///< neighbouring grid index
    vismodule::Real32 m_weight[8][SIMDW];  ///< weight for the neighbouring grid index
    //vismodule::Real32 m_differential_functions[24][SIMDW];
    vismodule::Real32 m_dNdx[8][SIMDW];
    vismodule::Real32 m_dNdy[8][SIMDW];
    vismodule::Real32 m_dNdz[8][SIMDW];

    const float*         m_reference_volume;
//    const vismodule::StructuredVolumeObject& m_reference_volume;
//    const vismodule::StructuredVolumeObject& m_reference_object;
    const vismodule::Vector3ui m_resolution; ///< resolution 3D
    const int m_line_size;
    const int m_slice_size;
    const int m_leaf_size;
    const int m_imax;
    const int m_jmax;
    const int m_kmax;

public:

    TrilinearInterpolator( const vismodule::StructuredVolumeObject& volume );
    TrilinearInterpolator( const float* value, const vismodule::Vector3ui resolution );
//    TrilinearInterpolator( T* values,
//              float* coords, int ncoords, int ncells);

public:

    void attachPoint( const vismodule::Vector3f& point );

    const vismodule::UInt32* indices( void ) const;
    
    template <typename T>
    const vismodule::Real32 scalar( void ) const;

    template <typename T>
    const vismodule::Vector3f gradient( void ) const;

    void setLeafIndex( const int leaf_index );

    void setCellLength( const float cell_length );

    void attachPoint( const float* p_x, const float* p_y, const float* p_z );

    void attachPoint_woSIMD( const vismodule::Vector3f& point );

    //const vismodule::UInt32* indices( void ) const;

    //template <typename T>
    void scalar( float* values ) const;

    template <typename T>
    const vismodule::Real32 scalar_woSIMD( void ) const;

    //template <typename T>
    void gradient( float* g_x, float* g_y, float* g_z ) const;

private:

    const int id( const int i, const int j, const int k ) const;
};

//inline TrilinearInterpolator::TrilinearInterpolator( const vismodule::StructuredVolumeObject& volume ) 
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

//inline TrilinearInterpolator::TrilinearInterpolator( const vismodule::StructuredVolumeObject* volume )
//    : m_grid_index( 0, 0, 0 )
//    , m_reference_object( volume )
//{
//}

inline TrilinearInterpolator::TrilinearInterpolator( const vismodule::StructuredVolumeObject& volume ) 
    : m_grid_index( 0, 0, 0 )
    , m_reference_volume( &volume )
    , m_resolution( volume.resolution() )
    , m_line_size ( m_resolution.x() )
    , m_slice_size( m_resolution.x() * m_resolution.y() )
    , m_leaf_size ( m_resolution.x() * m_resolution.y() * m_resolution.z() )
    , m_imax( m_resolution.x() - 1 )
    , m_jmax( m_resolution.y() - 1 )
    , m_kmax( m_resolution.z() - 1 )
{
}

inline TrilinearInterpolator::TrilinearInterpolator( const float* volume, const vismodule::Vector3ui resolution )
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

inline void TrilinearInterpolator::attachPoint_woSIMD( const vismodule::Vector3f& point )
{
    //const vismodule::Vector3ui resolution = m_reference_volume->resolution();
    const vismodule::Vector3ui resolution = m_resolution;
    VIS_MODULE_ASSERT( 0.0f <= point.x() && point.x() <= resolution.x() - 1.0f );
    VIS_MODULE_ASSERT( 0.0f <= point.y() && point.y() <= resolution.y() - 1.0f );
    VIS_MODULE_ASSERT( 0.0f <= point.z() && point.z() <= resolution.z() - 1.0f );

    // Temporary index.
    const size_t ti = static_cast<size_t>( point.x() );
    const size_t tj = static_cast<size_t>( point.y() );
    const size_t tk = static_cast<size_t>( point.z() );

    // Addjustment index for boundary.
    const size_t i = ( ti >= resolution.x() - 1 ) ? resolution.x() - 2 : ti;
    const size_t j = ( tj >= resolution.y() - 1 ) ? resolution.y() - 2 : tj;
    const size_t k = ( tk >= resolution.z() - 1 ) ? resolution.z() - 2 : tk;

    //const size_t line_size  = m_reference_volume->nnodesPerLine();
    //const size_t slice_size = m_reference_volume->nnodesPerSlice();
    const size_t line_size  = m_line_size ;
    const size_t slice_size = m_slice_size;

    // Calculate index.
    m_grid_index.set( i, j, k );

    m_index_woSIMD[0] = i + j * line_size + k * slice_size;
    m_index_woSIMD[1] = m_index_woSIMD[0] + 1;
    m_index_woSIMD[2] = m_index_woSIMD[1] + line_size;
    m_index_woSIMD[3] = m_index_woSIMD[0] + line_size;
    m_index_woSIMD[4] = m_index_woSIMD[0] + slice_size;
    m_index_woSIMD[5] = m_index_woSIMD[1] + slice_size;
    m_index_woSIMD[6] = m_index_woSIMD[2] + slice_size;
    m_index_woSIMD[7] = m_index_woSIMD[3] + slice_size;

    // Calculate local coordinate.
    const float x = point.x() - i;
    const float y = point.y() - j;
    const float z = point.z() - k;

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    const float xyz = xy * z;

    m_weight_woSIMD[0] = 1.0f - x - y - z + xy + yz + zx - xyz;
    m_weight_woSIMD[1] = x - xy - zx + xyz;
    m_weight_woSIMD[2] = xy - xyz;
    m_weight_woSIMD[3] = y - xy - yz + xyz;
    m_weight_woSIMD[4] = z - zx - yz + xyz;
    m_weight_woSIMD[5] = zx - xyz;
    m_weight_woSIMD[6] = xyz;
    m_weight_woSIMD[7] = yz - xyz;
}

//inline void TrilinearInterpolator::attachPoint( const vismodule::Vector3f& point )
#pragma ivdep
inline void TrilinearInterpolator::attachPoint( const float* p_x, const float* p_y, const float* p_z )
{
    const vismodule::Vector3ui resolution = m_resolution;

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

        m_index[0][I] = i + j * m_line_size + k * m_slice_size; //+ m_leaf_index * m_leaf_size;
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

inline const vismodule::UInt32* TrilinearInterpolator::indices( void ) const
{
    return( m_neighbouring_grid_index );
}

template <typename T>
inline const float TrilinearInterpolator::scalar( void ) const
{
    const T* const data = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );

    return(
        static_cast<float>(
            data[ m_neighbouring_grid_index[0] ] * m_neighbouring_grid_weight[0] +
            data[ m_neighbouring_grid_index[1] ] * m_neighbouring_grid_weight[1] +
            data[ m_neighbouring_grid_index[2] ] * m_neighbouring_grid_weight[2] +
            data[ m_neighbouring_grid_index[3] ] * m_neighbouring_grid_weight[3] +
            data[ m_neighbouring_grid_index[4] ] * m_neighbouring_grid_weight[4] +
            data[ m_neighbouring_grid_index[5] ] * m_neighbouring_grid_weight[5] +
            data[ m_neighbouring_grid_index[6] ] * m_neighbouring_grid_weight[6] +
            data[ m_neighbouring_grid_index[7] ] * m_neighbouring_grid_weight[7] ) );
}

#pragma ivdep
inline void TrilinearInterpolator::scalar( float* values ) const
{
    //const T* const data = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );
    const float* const data = m_reference_value;

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

template <typename T>
inline const float TrilinearInterpolator::scalar_woSIMD( void ) const
{
    //const T* const data = reinterpret_cast<const T*>( m_reference_object->values().pointer() );
    const float* const data = m_reference_volume;

    return(
        static_cast<float>(
            data[ m_index_woSIMD[0] ] * m_weight_woSIMD[0] +
            data[ m_index_woSIMD[1] ] * m_weight_woSIMD[1] +
            data[ m_index_woSIMD[2] ] * m_weight_woSIMD[2] +
            data[ m_index_woSIMD[3] ] * m_weight_woSIMD[3] +
            data[ m_index_woSIMD[4] ] * m_weight_woSIMD[4] +
            data[ m_index_woSIMD[5] ] * m_weight_woSIMD[5] +
            data[ m_index_woSIMD[6] ] * m_weight_woSIMD[6] +
            data[ m_index_woSIMD[7] ] * m_weight_woSIMD[7] ) );
}


#pragma ivdep
inline void TrilinearInterpolator::gradient( float* g_x, float* g_y, float* g_z ) const
{
    // Calculate a gradient vector in the local coordinate.
    const float* const data = m_reference_value;
    const vismodule::UInt32 nnodes = 8;
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

template <typename T>
inline const vismodule::Vector3f TrilinearInterpolator::gradient( void ) const
{
    // Calculate the point's gradient.
    float dx[8], dy[8], dz[8];

    const T* const data = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );

    const vismodule::Vector3ui resolution = m_reference_volume->resolution();
    const size_t line_size  = m_reference_volume->nnodesPerLine();
    const size_t slice_size = m_reference_volume->nnodesPerSlice();

    const size_t i = m_grid_index.x();
    const size_t j = m_grid_index.y();
    const size_t k = m_grid_index.z();

    if ( i == 0 )
    {
        dx[0] = static_cast<float>( data[ m_neighbouring_grid_index[1]     ] );
        dx[1] = static_cast<float>( data[ m_neighbouring_grid_index[1] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0]     ] );
        dx[2] = static_cast<float>( data[ m_neighbouring_grid_index[2] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3]     ] );
        dx[3] = static_cast<float>( data[ m_neighbouring_grid_index[2]     ] );
        dx[4] = static_cast<float>( data[ m_neighbouring_grid_index[5]     ] );
        dx[5] = static_cast<float>( data[ m_neighbouring_grid_index[5] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4]     ] );
        dx[6] = static_cast<float>( data[ m_neighbouring_grid_index[6] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[7]     ] );
        dx[7] = static_cast<float>( data[ m_neighbouring_grid_index[6]     ] );
    }
    else if ( i == resolution.x() - 2 )
    {
        dx[0] = static_cast<float>( data[ m_neighbouring_grid_index[1]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0] - 1 ] );
        dx[1] =                                              - static_cast<float>( data[ m_neighbouring_grid_index[0]     ] );
        dx[2] =                                              - static_cast<float>( data[ m_neighbouring_grid_index[3]     ] );
        dx[3] = static_cast<float>( data[ m_neighbouring_grid_index[2]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3] - 1 ] );
        dx[4] = static_cast<float>( data[ m_neighbouring_grid_index[5]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4] - 1 ] );
        dx[5] =                                              - static_cast<float>( data[ m_neighbouring_grid_index[4]     ] );
        dx[6] =                                              - static_cast<float>( data[ m_neighbouring_grid_index[7]     ] );
        dx[7] = static_cast<float>( data[ m_neighbouring_grid_index[6]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[7] - 1 ] );
    }
    else
    {
        dx[0] = static_cast<float>( data[ m_neighbouring_grid_index[1]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0] - 1 ] );
        dx[1] = static_cast<float>( data[ m_neighbouring_grid_index[1] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0]     ] );
        dx[2] = static_cast<float>( data[ m_neighbouring_grid_index[2] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3]     ] );
        dx[3] = static_cast<float>( data[ m_neighbouring_grid_index[2]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3] - 1 ] );
        dx[4] = static_cast<float>( data[ m_neighbouring_grid_index[5]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4] - 1 ] );
        dx[5] = static_cast<float>( data[ m_neighbouring_grid_index[5] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4]     ] );
        dx[6] = static_cast<float>( data[ m_neighbouring_grid_index[6] + 1 ] ) - static_cast<float>( data[ m_neighbouring_grid_index[7]     ] );
        dx[7] = static_cast<float>( data[ m_neighbouring_grid_index[6]     ] ) - static_cast<float>( data[ m_neighbouring_grid_index[7] - 1 ] );
    }

    if ( j == 0 )
    {
        dy[0] = static_cast<float>( data[ m_neighbouring_grid_index[3]             ] );
        dy[1] = static_cast<float>( data[ m_neighbouring_grid_index[2]             ] );
        dy[2] = static_cast<float>( data[ m_neighbouring_grid_index[2] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1]             ] );
        dy[3] = static_cast<float>( data[ m_neighbouring_grid_index[3] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0]             ] );
        dy[4] = static_cast<float>( data[ m_neighbouring_grid_index[7]             ] );
        dy[5] = static_cast<float>( data[ m_neighbouring_grid_index[6]             ] );
        dy[6] = static_cast<float>( data[ m_neighbouring_grid_index[6] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[5]             ] );
        dy[7] = static_cast<float>( data[ m_neighbouring_grid_index[7] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4]             ] );
    }
    else if ( j == resolution.y() - 2 )
    {
        dy[0] = static_cast<float>( data[ m_neighbouring_grid_index[3]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0] - line_size ] );
        dy[1] = static_cast<float>( data[ m_neighbouring_grid_index[2]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1] - line_size ] );
        dy[2] =                                                      - static_cast<float>( data[ m_neighbouring_grid_index[1]             ] );
        dy[3] =                                                      - static_cast<float>( data[ m_neighbouring_grid_index[0]             ] );
        dy[4] = static_cast<float>( data[ m_neighbouring_grid_index[7]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4] - line_size ] );
        dy[5] = static_cast<float>( data[ m_neighbouring_grid_index[6]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[5] - line_size ] );
        dy[6] =                                                      - static_cast<float>( data[ m_neighbouring_grid_index[5]             ] );
        dy[7] =                                                      - static_cast<float>( data[ m_neighbouring_grid_index[4]             ] );
    }
    else
    {
        dy[0] = static_cast<float>( data[ m_neighbouring_grid_index[3]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0] - line_size ] );
        dy[1] = static_cast<float>( data[ m_neighbouring_grid_index[2]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1] - line_size ] );
        dy[2] = static_cast<float>( data[ m_neighbouring_grid_index[2] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1]             ] );
        dy[3] = static_cast<float>( data[ m_neighbouring_grid_index[3] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0]             ] );
        dy[4] = static_cast<float>( data[ m_neighbouring_grid_index[7]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4] - line_size ] );
        dy[5] = static_cast<float>( data[ m_neighbouring_grid_index[6]             ] ) - static_cast<float>( data[ m_neighbouring_grid_index[5] - line_size ] );
        dy[6] = static_cast<float>( data[ m_neighbouring_grid_index[6] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[5]             ] );
        dy[7] = static_cast<float>( data[ m_neighbouring_grid_index[7] + line_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[4]             ] );
    }

    if ( k == 0 )
    {
        dz[0] = static_cast<float>( data[ m_neighbouring_grid_index[4]              ] );
        dz[1] = static_cast<float>( data[ m_neighbouring_grid_index[5]              ] );
        dz[2] = static_cast<float>( data[ m_neighbouring_grid_index[6]              ] );
        dz[3] = static_cast<float>( data[ m_neighbouring_grid_index[7]              ] );
        dz[4] = static_cast<float>( data[ m_neighbouring_grid_index[4] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0]              ] );
        dz[5] = static_cast<float>( data[ m_neighbouring_grid_index[5] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1]              ] );
        dz[6] = static_cast<float>( data[ m_neighbouring_grid_index[6] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[2]              ] );
        dz[7] = static_cast<float>( data[ m_neighbouring_grid_index[7] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3]              ] );
    }
    else if ( k == resolution.z() - 2 )
    {
        dz[0] = static_cast<float>( data[ m_neighbouring_grid_index[4]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0] - slice_size ] );
        dz[1] = static_cast<float>( data[ m_neighbouring_grid_index[5]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1] - slice_size ] );
        dz[2] = static_cast<float>( data[ m_neighbouring_grid_index[6]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[2] - slice_size ] );
        dz[3] = static_cast<float>( data[ m_neighbouring_grid_index[7]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3] - slice_size ] );
        dz[4] =                                                       - static_cast<float>( data[ m_neighbouring_grid_index[0]              ] );
        dz[5] =                                                       - static_cast<float>( data[ m_neighbouring_grid_index[1]              ] );
        dz[6] =                                                       - static_cast<float>( data[ m_neighbouring_grid_index[2]              ] );
        dz[7] =                                                       - static_cast<float>( data[ m_neighbouring_grid_index[3]              ] );
    }
    else
    {
        dz[0] = static_cast<float>( data[ m_neighbouring_grid_index[4]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0] - slice_size ] );
        dz[1] = static_cast<float>( data[ m_neighbouring_grid_index[5]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1] - slice_size ] );
        dz[2] = static_cast<float>( data[ m_neighbouring_grid_index[6]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[2] - slice_size ] );
        dz[3] = static_cast<float>( data[ m_neighbouring_grid_index[7]              ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3] - slice_size ] );
        dz[4] = static_cast<float>( data[ m_neighbouring_grid_index[4] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[0]              ] );
        dz[5] = static_cast<float>( data[ m_neighbouring_grid_index[5] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[1]              ] );
        dz[6] = static_cast<float>( data[ m_neighbouring_grid_index[6] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[2]              ] );
        dz[7] = static_cast<float>( data[ m_neighbouring_grid_index[7] + slice_size ] ) - static_cast<float>( data[ m_neighbouring_grid_index[3]              ] );
    }

    const float x =
        dx[0] * m_neighbouring_grid_weight[0] +
        dx[1] * m_neighbouring_grid_weight[1] +
        dx[2] * m_neighbouring_grid_weight[2] +
        dx[3] * m_neighbouring_grid_weight[3] +
        dx[4] * m_neighbouring_grid_weight[4] +
        dx[5] * m_neighbouring_grid_weight[5] +
        dx[6] * m_neighbouring_grid_weight[6] +
        dx[7] * m_neighbouring_grid_weight[7];

    const float y =
        dy[0] * m_neighbouring_grid_weight[0] +
        dy[1] * m_neighbouring_grid_weight[1] +
        dy[2] * m_neighbouring_grid_weight[2] +
        dy[3] * m_neighbouring_grid_weight[3] +
        dy[4] * m_neighbouring_grid_weight[4] +
        dy[5] * m_neighbouring_grid_weight[5] +
        dy[6] * m_neighbouring_grid_weight[6] +
        dy[7] * m_neighbouring_grid_weight[7];

    const float z =
        dz[0] * m_neighbouring_grid_weight[0] +
        dz[1] * m_neighbouring_grid_weight[1] +
        dz[2] * m_neighbouring_grid_weight[2] +
        dz[3] * m_neighbouring_grid_weight[3] +
        dz[4] * m_neighbouring_grid_weight[4] +
        dz[5] * m_neighbouring_grid_weight[5] +
        dz[6] * m_neighbouring_grid_weight[6] +
        dz[7] * m_neighbouring_grid_weight[7];

//    return( vismodule::Vector3f( x, y, z ) );
    return( vismodule::Vector3f( -x, -y, -z ) );
}

} // end of namespace vismodule

#endif // VIS_MODULE__TRILINEAR_INTERPOLATOR_H_INCLUDE
