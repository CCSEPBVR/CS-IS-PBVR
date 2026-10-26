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
    vismodule::UInt32    m_neighbouring_grid_index[8];   ///< neighbouring grid index
    vismodule::Real32    m_neighbouring_grid_weight[8];  ///< weight for the neighbouring grid index
    vismodule::UInt32    m_index_woSIMD[8];   ///< neighbouring grid index
    vismodule::Real32    m_weight_woSIMD[8];  ///< weight for the neighbouring grid index
    vismodule::Real32    m_scalars[8]; 
    float*               m_data;

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

    const float*         m_reference_value;
    const vismodule::StructuredVolumeObject* m_reference_volume;
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

    //template <typename T>
    void scalar( float* values ) const;

    template <typename T>
    const vismodule::Real32 scalar_woSIMD( void ) const;

    //template <typename T>
    void gradient( float* g_x, float* g_y, float* g_z ) const;

    // 2階微分。三線形の形状関数そのものの2階微分は対角が恒等的に 0 になるため、
    // 場の2階微分を節点で中心差分し、その節点場を三線形補間して求める。
    // 格子境界の節点は片側2次差分に切り替える。
    // 符号は gradient() と同じ規約で **-H** を返す。単位は物理座標系(1/cell_length^2)。
    void hessian( float* h_xx, float* h_yy, float* h_zz,
                  float* h_xy, float* h_xz, float* h_yz ) const;

private:

    const int id( const int i, const int j, const int k ) const;

    // hessian() 用。軸 a のストライド（a=0:x, 1:y, 2:z）
    int hess_stride( const int a ) const
    { return ( a == 0 ) ? 1 : ( a == 1 ) ? m_line_size : m_slice_size; }

    // 節点での 1階差分（格子単位）。c は軸 a 方向の格子添字、lim は その軸の上限。
    float hess_d1( const std::size_t base, const int a, const int c, const int lim ) const;

    // 節点での 2階微分（格子単位）。a==b なら同軸、a!=b なら交差（D_a(D_b f)）。
    float hess_d2( const std::size_t base, const int a, const int b,
                   const int ca, const int lima, const int cb, const int limb ) const;
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

inline TrilinearInterpolator::TrilinearInterpolator( const vismodule::StructuredVolumeObject& volume )
    : m_grid_index( 0, 0, 0 )
    , m_reference_volume( &volume )
    , m_resolution( 0, 0, 0 )
    , m_line_size( 0 )
    , m_slice_size( 0 )
    , m_leaf_size( 0 )
    , m_imax( 0 )
    , m_jmax( 0 )
    , m_kmax( 0 )
{
}

inline TrilinearInterpolator::TrilinearInterpolator( const float* value, const vismodule::Vector3ui resolution )
    : m_reference_value( value )
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
inline const int TrilinearInterpolator::id( const int i, const int j, const int k ) const
{

    const int I = i<0 ? 0 : i>m_imax ? m_imax : i;
    const int J = j<0 ? 0 : j>m_jmax ? m_jmax : j;
    const int K = k<0 ? 0 : k>m_kmax ? m_kmax : k;

    return I + J*m_line_size + K*m_slice_size;

//    return i + j*m_line_size + k*m_slice_size;
}

inline void TrilinearInterpolator::attachPoint( const Vector3f& point )
{
    const Vector3ui resolution = m_reference_volume->resolution();
    VIS_MODULE_ASSERT( 0.0f <= point.x() && point.x() <= resolution.x() - 1.0f );
    VIS_MODULE_ASSERT( 0.0f <= point.y() && point.y() <= resolution.y() - 1.0f );
    VIS_MODULE_ASSERT( 0.0f <= point.z() && point.z() <= resolution.z() - 1.0f );

    // Temporary index.
    const std::size_t ti = static_cast<size_t>( point.x() );
    const std::size_t tj = static_cast<size_t>( point.y() );
    const std::size_t tk = static_cast<size_t>( point.z() );

    // Addjustment index for boundary.
    const std::size_t i = ( ti >= resolution.x() - 1 ) ? resolution.x() - 2 : ti;
    const std::size_t j = ( tj >= resolution.y() - 1 ) ? resolution.y() - 2 : tj;
    const std::size_t k = ( tk >= resolution.z() - 1 ) ? resolution.z() - 2 : tk;

    const std::size_t line_size  = m_reference_volume->nnodesPerLine();
    const std::size_t slice_size = m_reference_volume->nnodesPerSlice();

    // Calculate index.
    m_grid_index.set( i, j, k );

    m_neighbouring_grid_index[0] = i + j * line_size + k * slice_size;
    m_neighbouring_grid_index[1] = m_neighbouring_grid_index[0] + 1;
    m_neighbouring_grid_index[2] = m_neighbouring_grid_index[1] + line_size;
    m_neighbouring_grid_index[3] = m_neighbouring_grid_index[0] + line_size;
    m_neighbouring_grid_index[4] = m_neighbouring_grid_index[0] + slice_size;
    m_neighbouring_grid_index[5] = m_neighbouring_grid_index[1] + slice_size;
    m_neighbouring_grid_index[6] = m_neighbouring_grid_index[2] + slice_size;
    m_neighbouring_grid_index[7] = m_neighbouring_grid_index[3] + slice_size;

    // Calculate local coordinate.
    const float x = point.x() - i;
    const float y = point.y() - j;
    const float z = point.z() - k;

    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;

    const float xyz = xy * z;

    m_neighbouring_grid_weight[0] = 1.0f - x - y - z + xy + yz + zx - xyz;
    m_neighbouring_grid_weight[1] = x - xy - zx + xyz;
    m_neighbouring_grid_weight[2] = xy - xyz;
    m_neighbouring_grid_weight[3] = y - xy - yz + xyz;
    m_neighbouring_grid_weight[4] = z - zx - yz + xyz;
    m_neighbouring_grid_weight[5] = zx - xyz;
    m_neighbouring_grid_weight[6] = xyz;
    m_neighbouring_grid_weight[7] = yz - xyz;
}

inline void TrilinearInterpolator::attachPoint_woSIMD( const vismodule::Vector3f& point )
{
    //const vismodule::Vector3ui resolution = m_reference_volume->resolution();
    const vismodule::Vector3ui resolution = m_resolution;
    VIS_MODULE_ASSERT( 0.0f <= point.x() && point.x() <= resolution.x() - 1.0f );
    VIS_MODULE_ASSERT( 0.0f <= point.y() && point.y() <= resolution.y() - 1.0f );
    VIS_MODULE_ASSERT( 0.0f <= point.z() && point.z() <= resolution.z() - 1.0f );

    // Temporary index.
    const std::size_t ti = static_cast<size_t>( point.x() );
    const std::size_t tj = static_cast<size_t>( point.y() );
    const std::size_t tk = static_cast<size_t>( point.z() );

    // Addjustment index for boundary.
    const std::size_t i = ( ti >= resolution.x() - 1 ) ? resolution.x() - 2 : ti;
    const std::size_t j = ( tj >= resolution.y() - 1 ) ? resolution.y() - 2 : tj;
    const std::size_t k = ( tk >= resolution.z() - 1 ) ? resolution.z() - 2 : tk;

    //const std::size_t line_size  = m_reference_volume->nnodesPerLine();
    //const std::size_t slice_size = m_reference_volume->nnodesPerSlice();
    const std::size_t line_size  = m_line_size ;
    const std::size_t slice_size = m_slice_size;

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

inline void TrilinearInterpolator::attachPoint( const float* p_x, const float* p_y, const float* p_z )
{
    const vismodule::Vector3ui resolution = m_resolution;

    #pragma ivdep
    for( int I=0; I < SIMDW; I++ )
    {
        // Temporary index.
        const std::size_t ti = static_cast<size_t>( p_x[I] );
        const std::size_t tj = static_cast<size_t>( p_y[I] );
        const std::size_t tk = static_cast<size_t>( p_z[I] );

        // Addjustment index for boundary.
        const std::size_t i = ( ti >= resolution.x() - 1 ) ? resolution.x() - 2 : ti;
        const std::size_t j = ( tj >= resolution.y() - 1 ) ? resolution.y() - 2 : tj;
        const std::size_t k = ( tk >= resolution.z() - 1 ) ? resolution.z() - 2 : tk;

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

inline void TrilinearInterpolator::scalar( float* values ) const
{
    //const T* const data = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );
    const float* const data = m_reference_value;

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

template <typename T>
inline const float TrilinearInterpolator::scalar_woSIMD( void ) const
{
    //const T* const data = reinterpret_cast<const T*>( m_reference_object->values().pointer() );
    const float* const data = m_reference_value;

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


inline void TrilinearInterpolator::gradient( float* g_x, float* g_y, float* g_z ) const
{
    // Calculate a gradient vector in the local coordinate.
    const float* const data = m_reference_value;
    const vismodule::UInt32 nnodes = 8;
    const float inv_Jacobi = 1.0 / m_cell_length;

    #pragma ivdep
    for( int I = 0; I < SIMDW; I++ )
    {
        float dsdx = 0.0f;
        float dsdy = 0.0f;
        float dsdz = 0.0f;

        for ( std::size_t i = 0; i < nnodes; i++ )
        {
            dsdx += data[ m_index[i][I] ] * m_dNdx[i][I];
            dsdy += data[ m_index[i][I] ] * m_dNdy[i][I];
            dsdz += data[ m_index[i][I] ] * m_dNdz[i][I];
        }

        g_x[I] = -inv_Jacobi * dsdx;
        g_y[I] = -inv_Jacobi * dsdy;
        g_z[I] = -inv_Jacobi * dsdz;
    }
}

/*==========================================================================*/
/**
 *  @brief  節点での 1階差分（格子単位）。境界は片側2次差分。
 */
/*==========================================================================*/
inline float TrilinearInterpolator::hess_d1(
    const std::size_t base, const int a, const int c, const int lim ) const
{
    const float* const d = m_reference_value;
    const int st = this->hess_stride( a );
    if ( c <= 0 )
    {
        return ( -3.0f * d[base] + 4.0f * d[base + st] - d[base + 2 * st] ) * 0.5f;
    }
    if ( c >= lim )
    {
        return (  3.0f * d[base] - 4.0f * d[base - st] + d[base - 2 * st] ) * 0.5f;
    }
    return ( d[base + st] - d[base - st] ) * 0.5f;
}

/*==========================================================================*/
/**
 *  @brief  節点での 2階微分（格子単位）。
 *
 *  同軸(a==b)は 3点の中心差分、交差(a!=b)は b 方向の 1階差分を a 方向へ
 *  もう一度差分する。どちらも境界の節点では片側2次に切り替える。
 *  交差では a 方向にずらしても b 方向の格子添字は変わらないので、
 *  b 側の境界判定には cb をそのまま使う。
 */
/*==========================================================================*/
inline float TrilinearInterpolator::hess_d2(
    const std::size_t base, const int a, const int b,
    const int ca, const int lima, const int cb, const int limb ) const
{
    const int sta = this->hess_stride( a );

    if ( a == b )
    {
        const float* const d = m_reference_value;
        if ( ca <= 0 )
        {
            return d[base] - 2.0f * d[base + sta] + d[base + 2 * sta];
        }
        if ( ca >= lima )
        {
            return d[base] - 2.0f * d[base - sta] + d[base - 2 * sta];
        }
        return d[base + sta] - 2.0f * d[base] + d[base - sta];
    }

    if ( ca <= 0 )
    {
        return ( -3.0f * this->hess_d1( base,               b, cb, limb )
               +  4.0f * this->hess_d1( base +     sta,     b, cb, limb )
               -         this->hess_d1( base + 2 * sta,     b, cb, limb ) ) * 0.5f;
    }
    if ( ca >= lima )
    {
        return (  3.0f * this->hess_d1( base,               b, cb, limb )
               -  4.0f * this->hess_d1( base -     sta,     b, cb, limb )
               +         this->hess_d1( base - 2 * sta,     b, cb, limb ) ) * 0.5f;
    }
    return ( this->hess_d1( base + sta, b, cb, limb )
           - this->hess_d1( base - sta, b, cb, limb ) ) * 0.5f;
}

/*==========================================================================*/
/**
 *  @brief  2階微分（ヘッセ行列の独立6成分）。
 *
 *  三線形の形状関数の2階微分は対角成分が恒等的に 0 で、交差成分もセル境界で
 *  不連続になる。そこで「場の2階微分を節点で差分し、その節点場を三線形補間する」
 *  方式をとる。節点場を補間するので値はセル境界で連続になり、対角成分も出る。
 *
 *  精度は 2次（検証: 刻み半分で誤差 1/4）。F=|grad q| の法線の向きのずれは
 *  平均 0.2〜0.3 度で、形状関数の2階微分を使う場合(34.6度)より大幅に良い。
 *
 *  読む格子点は 1セルあたり最大 4x4x4 の範囲で、gradient() の 8点より多い。
 *  2階微分は数式が微分量(dq)を参照するときだけ必要なので、常時の費用ではない。
 *
 *  費用の実測（256^3 = 16,581,375 セル/ランク、4MPI x 1スレッド、棄却前 322万粒子、
 *  数式 sqrt(dq1x^2+dq1y^2+dq1z^2)、アンサンブル4、袖交換なし）:
 *    粒子生成一式  2階微分なし 5.20 秒 -> あり 9.33 秒（2階微分の分が 4.13 秒）
 *    参考: 三次Bスプライン 18.40 秒（2階微分は厳密だが約2倍かかる）
 *  分岐なし経路の導入前は 37.02 秒だったので、そこから 3.97 倍速くなっている
 *  （内訳: 参照されない変数を飛ばす側で 2.70 倍、この経路で 1.78 倍）。
 */
/*==========================================================================*/
inline void TrilinearInterpolator::hessian(
    float* h_xx, float* h_yy, float* h_zz,
    float* h_xy, float* h_xz, float* h_yz ) const
{
    // attachPoint() が決めたセルの 8頂点。局所位置は m_weight の並びと同じ。
    static const int NOFF[8][3] = {
        {0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1}
    };
    // 成分と軸の対応（xx,yy,zz,xy,xz,yz）
    static const int PAIR[6][2] = { {0,0},{1,1},{2,2},{0,1},{0,2},{1,2} };

    const int lim[3] = { m_imax, m_jmax, m_kmax };
    const float inv_h2 = 1.0f / ( m_cell_length * m_cell_length );
    float* out[6] = { h_xx, h_yy, h_zz, h_xy, h_xz, h_yz };

    const float* const d  = m_reference_value;
    const std::size_t   sx = 1;
    const std::size_t   sy = static_cast<std::size_t>( m_line_size );
    const std::size_t   sz = static_cast<std::size_t>( m_slice_size );

    #pragma ivdep
    for ( int I = 0; I < SIMDW; I++ )
    {
        const int gi = static_cast<int>( m_grid_index_i[I] );
        const int gj = static_cast<int>( m_grid_index_j[I] );
        const int gk = static_cast<int>( m_grid_index_k[I] );

        float acc[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

        // セルの 8頂点すべてが中心差分で済む（= 片側差分への切り替えが起きない）なら、
        // 境界判定の分岐と関数呼び出しを省いた経路を通る。8頂点は添字 g..g+1 にあり、
        // 中心差分は ±1 を見るので、1 <= g かつ g+1 <= lim-1 が条件。
        // 256^3 では内部セルが 97.7% を占めるので、ここがほぼ常に選ばれる。
        //
        // 式は下の else 節（hess_d2/hess_d1 経由）と同一だが、**結果はビット一致しない**。
        // 分岐が無くなることで icpx が FMA と再結合をより積極的に適用するためで、
        // 括弧では制御できない（既定で浮動小数の再結合が有効なので括弧は無視される）。
        // 差の実測（256^3, 4MPI, 数式 sqrt(dq1x^2+dq1y^2+dq1z^2)）:
        //   法線の成分の最大絶対差 2.3e-04（値の最大 1.3e+02 に対し相対 1.7e-06）
        //   正規化後の法線の向きの差 平均 0.00004 度 / 最大 0.009 度
        //   0.01 度を超える粒子は 368,442 本中 0 本
        // 向きの差は陰影に影響しない水準。速度は分岐つき経路の 1.78 倍。
        const bool inner = ( gi >= 1 && gi + 2 <= m_imax )
                        && ( gj >= 1 && gj + 2 <= m_jmax )
                        && ( gk >= 1 && gk + 2 <= m_kmax );

        if ( inner )
        {
            for ( int n = 0; n < 8; n++ )
            {
                const std::size_t b =
                      static_cast<std::size_t>( gi + NOFF[n][0] )
                    + static_cast<std::size_t>( gj + NOFF[n][1] ) * sy
                    + static_cast<std::size_t>( gk + NOFF[n][2] ) * sz;
                const float w  = m_weight[n][I];
                const float d0 = d[b];

                // 同軸: hess_d2 の中心差分と同じ式・同じ順序
                acc[0] += w * ( d[b + sx] - 2.0f * d0 + d[b - sx] );
                acc[1] += w * ( d[b + sy] - 2.0f * d0 + d[b - sy] );
                acc[2] += w * ( d[b + sz] - 2.0f * d0 + d[b - sz] );

                // 交差: hess_d1 を ±1 で評価してから差分する順序をそのまま展開。
                // 括弧は hess_d2 経由と同じ結合の形を残す意図で付けてあるが、
                // icpx 既定では再結合されるため、これで一致が取れるわけではない。
                const float dy_p = ( d[b + sx + sy] - d[b + sx - sy] ) * 0.5f;
                const float dy_m = ( d[b - sx + sy] - d[b - sx - sy] ) * 0.5f;
                acc[3] += w * ( ( dy_p - dy_m ) * 0.5f );

                const float dz_p = ( d[b + sx + sz] - d[b + sx - sz] ) * 0.5f;
                const float dz_m = ( d[b - sx + sz] - d[b - sx - sz] ) * 0.5f;
                acc[4] += w * ( ( dz_p - dz_m ) * 0.5f );

                const float dzy_p = ( d[b + sy + sz] - d[b + sy - sz] ) * 0.5f;
                const float dzy_m = ( d[b - sy + sz] - d[b - sy - sz] ) * 0.5f;
                acc[5] += w * ( ( dzy_p - dzy_m ) * 0.5f );
            }
        }
        else
        {
            // 格子の外周に接するセル。片側差分への切り替えが要るので分岐つきの経路。
            for ( int n = 0; n < 8; n++ )
            {
                const int c[3] = { gi + NOFF[n][0], gj + NOFF[n][1], gk + NOFF[n][2] };
                const std::size_t base = static_cast<std::size_t>( c[0] )
                                       + static_cast<std::size_t>( c[1] ) * sy
                                       + static_cast<std::size_t>( c[2] ) * sz;
                const float w = m_weight[n][I];

                for ( int s = 0; s < 6; s++ )
                {
                    const int a = PAIR[s][0];
                    const int b = PAIR[s][1];
                    acc[s] += w * this->hess_d2( base, a, b, c[a], lim[a], c[b], lim[b] );
                }
            }
        }

        // gradient() が -grad を返すのと同じ規約で -H を返す
        for ( int s = 0; s < 6; s++ ) out[s][I] = -inv_h2 * acc[s];
    }
}

template <typename T>
inline const vismodule::Vector3f TrilinearInterpolator::gradient( void ) const
{
    // Calculate the point's gradient.
    float dx[8], dy[8], dz[8];

    const T* const data = reinterpret_cast<const T*>( m_reference_volume->values().pointer() );

    const vismodule::Vector3ui resolution = m_reference_volume->resolution();
    const std::size_t line_size  = m_reference_volume->nnodesPerLine();
    const std::size_t slice_size = m_reference_volume->nnodesPerSlice();

    const std::size_t i = m_grid_index.x();
    const std::size_t j = m_grid_index.y();
    const std::size_t k = m_grid_index.z();

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
} // end of namespace vismodule

#endif // VIS_MODULE__TRILINEAR_INTERPOLATOR_H_INCLUDE
