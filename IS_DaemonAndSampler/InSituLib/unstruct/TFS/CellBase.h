/****************************************************************************/
/**
 *  @file CellBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellBase.h 840 2011-06-07 11:17:16Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#ifndef PBVR__CELL_BASE_H_INCLUDE
#define PBVR__CELL_BASE_H_INCLUDE

#include <kvs/DebugNew>
#include "ClassName.h"
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>

#include "UnstructuredVolumeObject.h"
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Message>
#include <kvs/MersenneTwister> 
//#include "SFMT/SFMT.h" 
#include <kvs/Timer>

#ifdef KMATH
#include <kmath_random.h>
extern KMATH_Random km_random;
#endif

#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 128
#endif

namespace pbvr
{
/*===========================================================================*/
/**
 *  @brief  CellBase class.
 */
/*===========================================================================*/
template <typename T>
class CellBase
{
    kvsClassName( pbvr::CellBase );

protected:

    size_t         m_nnodes; ///< number of nodes
    kvs::Vector3f* m_vertices; ///< coordinates of the nodes
    T*             m_scalars; ///< scalar values of the nodes
    kvs::Real32*   m_interpolation_functions; ///< interpolation functions
    kvs::Real32*   m_differential_functions; ///< differential functions
    kvs::Real32*   m_vertices_vec; ///< coordinates of the nodes
    mutable kvs::Vector3f  m_global_point; ///< sampling point in the global coordinate
    mutable kvs::Vector3f  m_local_point;  ///< sampling point in the local coordinate

    const pbvr::UnstructuredVolumeObject* m_reference_volume; ///< reference unstructured volume

    T*           m_values;
    float*       m_coords;
    kvs::UInt32* m_connections;
    int          m_ncoords;
    int          m_ncells;
    // cal time
    double      cal_time[6];

    //T m_scalars_array[8][ SIMD_BLK_SIZE ];
    //kvs::Vector3f m_vertices_array[8][ SIMD_BLK_SIZE ];
    //kvs::Real32   m_interpolation_functions_array[8][ SIMD_BLK_SIZE ];
    //kvs::Real32   m_differential_functions_array[24][ SIMD_BLK_SIZE ];
    T** m_scalars_array;
    kvs::Vector3f** m_vertices_array;
    kvs::Real32**   m_interpolation_functions_array;
    kvs::Real32**   m_differential_functions_array;

    void allocate();
    void deallocate();

public:

    CellBase( const pbvr::UnstructuredVolumeObject& volume );
    CellBase( const pbvr::UnstructuredVolumeObject& volume, const size_t cell_type );
    CellBase( T* values,
              float* coords, int ncoords,
              unsigned int* connections, int ncells, int cell_type);


    virtual ~CellBase();

public:

    virtual const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const = 0;
    
    virtual const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const = 0;
 
//    virtual const kvs::Real32** interpolationFunctions_array( const kvs::Vector3f* point, const int loop_cnt ) const = 0;
    virtual void interpolationFunctions_array( const kvs::Vector3f* point, const int loop_cnt ) const = 0;
    
//    virtual const kvs::Real32** differentialFunctions_array( const kvs::Vector3f* point, const int loop_cnt ) const = 0;
    virtual void differentialFunctions_array( const kvs::Vector3f* point, const int loop_cnt ) const = 0;
    
    virtual void scalar_ary( float* scalar_array, const int loop_cnt ) const = 0;
    
    virtual void grad_ary( float* grad_array_x, float* grad_array_y, float* grad_array_z,  const int loop_cnt ) const = 0;
    
//    virtual void bindCell_wVolume( const kvs::UInt32 index, const size_t n = 0 );
    
    virtual void bindCell( const kvs::UInt32 index ,const size_t n = 0);

    virtual void setGlobalPoint( const kvs::Vector3f& point ) const;

    virtual void setLocalPoint( const kvs::Vector3f& point ) const;

    virtual void setLocalGravityPoint() const = 0;

    virtual void bindCellArray( const int loop_cnt, const kvs::UInt32 *cell_index );
    virtual void setLocalPointArray( const int loop_cnt, const kvs::Vector3f *local_array );
    virtual void transformLocalToGlobalArray( const int loop_cnt, const kvs::Vector3f *local_array, kvs::Vector3f *global_array);
    virtual void CalcScalarGrad(
            const int loop_cnt,
            float *scalar_array,
            float *grad_array_x,
            float *grad_array_y,
            float *grad_array_z );

    virtual void CalcAveragedScalarGrad(
            const int loop_cnt,
            float *scalar_array,
            float *grad_array_x,
            float *grad_array_y,
            float *grad_array_z );

    
    virtual const kvs::Vector3f transformGlobalToLocal( const kvs::Vector3f& point ) const;

    virtual const kvs::Vector3f transformLocalToGlobal( const kvs::Vector3f& point ) const;

public:

    virtual const kvs::Vector3f randomSampling() const;
    
    virtual const kvs::Vector3f randomSampling_MT( kvs::MersenneTwister* MT ) const;

//    virtual void  randomSampling_SFMT( sfmt_t *sfmt, kvs::Vector3f *local_array, const int loop_cnt, std::vector<double> track);
//    virtual const kvs::Vector3f randomSampling_SFMT( sfmt_t *sfmt ) const;

    virtual const kvs::Real32 volume() const;

    virtual const kvs::Real32 averagedScalar() const;

    virtual const kvs::Real32 scalar() const;

    virtual const kvs::Vector3f gradient() const;
    
    virtual const kvs::Real32 localGravityPointValue() const;

public:

    const kvs::Vector3f* vertices() const;

    const T* scalars() const;

    T* setScalars();

    const kvs::Vector3f globalPoint() const;

    const kvs::Vector3f localPoint() const;

    const size_t numberOfNodes() const;

    const kvs::Matrix33f JacobiMatrix() const;

    const kvs::Real32 randomNumber() const;

    const double* output_time();
    void  reset_time();
};

/*===========================================================================*/
/**
 *  @brief  Allocate memory.
 */
/*===========================================================================*/
template <typename T>
inline void CellBase<T>::allocate()
{
    const size_t dimension = 3;
    const size_t nnodes = m_nnodes;
    try
    {
        m_vertices = new kvs::Vector3f [nnodes];
        if ( !m_vertices ) throw "Cannot allocate memory for 'm_vertices'";
        memset( m_vertices, 0, sizeof( kvs::Vector3f ) * nnodes );

        m_scalars = new T [nnodes];
        if ( !m_scalars ) throw "Cannot allocate memory for 'm_scalars'";
        memset( m_scalars, 0, sizeof( T ) * nnodes );

        m_interpolation_functions = new kvs::Real32 [nnodes];
        if ( !m_interpolation_functions ) throw "Cannot allocate memory for 'm_interpolation_functions'";
        memset( m_interpolation_functions, 0, sizeof( kvs::Real32 ) * nnodes );

        m_differential_functions  = new kvs::Real32 [nnodes * dimension];
        if ( !m_differential_functions ) throw "Cannot allocate memory for 'm_differential_functions'";
        memset( m_differential_functions, 0, sizeof( kvs::Real32 ) * nnodes * dimension );

        m_vertices_vec = new kvs::Real32 [nnodes * dimension];
        if ( !m_vertices_vec ) throw "Cannot allocate memory for 'm_vertices_vec'";
        memset( m_vertices_vec, 0, sizeof( kvs::Real32 ) * nnodes * dimension );
    }
    catch ( char* error_message )
    {
        kvsMessageError( error_message );
        return;
    }
}

/*===========================================================================*/
/**
 *  @brief  Allocate memory.
 */
/*===========================================================================*/
template <typename T>
inline void CellBase<T>::deallocate()
{
    if ( m_vertices ) delete [] m_vertices;
    if ( m_scalars ) delete [] m_scalars;
    if ( m_interpolation_functions ) delete [] m_interpolation_functions;
    if ( m_differential_functions ) delete [] m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellBase class.
 *  @param  volume [in] pointer to the unstructured volume object.
 */
/*===========================================================================*/
template <typename T>
inline CellBase<T>::CellBase(
    const pbvr::UnstructuredVolumeObject& volume ):
    m_nnodes( volume.cellType() ),
    m_global_point( 0, 0, 0 ),
    m_local_point( 0, 0, 0 ),
    m_reference_volume( &volume )
{
    this->allocate();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellBase class.
 *  @param  volume [in] pointer to the unstructured volume object.
 */
/*===========================================================================*/
template <typename T>
inline CellBase<T>::CellBase(
    const pbvr::UnstructuredVolumeObject& volume, const size_t cell_type ):
    m_nnodes( cell_type ),
    m_global_point( 0, 0, 0 ),
    m_local_point( 0, 0, 0 ),
    m_reference_volume( &volume )
{
    this->allocate();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellBase class.
 *  @param  volume [in] pointer to the unstructured volume object.
 */
/*===========================================================================*/
template <typename T>
inline CellBase<T>::CellBase(
    T* values,
    float* coords, int ncoords,
    unsigned int* connections, int ncells, int cell_type ):
    m_values( values ),
    m_coords( coords ), m_ncoords( ncoords ),
    m_connections( connections ), m_ncells( ncells ),
    m_nnodes( cell_type ),//num of hex vertices.
    m_global_point( 0, 0, 0 ),
    m_local_point( 0, 0, 0 )
{
    const size_t dimension = 3;
    const size_t nnodes = m_nnodes;
    try
    {
        m_vertices = new kvs::Vector3f [nnodes];
        if ( !m_vertices ) throw "Cannot allocate memory for 'm_vertices'";
        memset( m_vertices, 0, sizeof( kvs::Vector3f ) * nnodes );

        m_scalars = new T [nnodes];
        if ( !m_scalars ) throw "Cannot allocate memory for 'm_scalars'";
        memset( m_scalars, 0, sizeof( T ) * nnodes );

        m_interpolation_functions = new kvs::Real32 [nnodes];
        if ( !m_interpolation_functions ) throw "Cannot allocate memory for 'm_interpolation_functions'";
        memset( m_interpolation_functions, 0, sizeof( kvs::Real32 ) * nnodes );

        m_differential_functions  = new kvs::Real32 [nnodes*dimension];
        if ( !m_differential_functions ) throw "Cannot allocate memory for 'm_differential_functions'";
        memset( m_differential_functions, 0, sizeof( kvs::Real32 ) * nnodes * dimension );
        
        m_vertices_vec = new kvs::Real32 [nnodes * dimension];
        if ( !m_vertices_vec ) throw "Cannot allocate memory for 'm_vertices_vec'";
        memset( m_vertices_vec, 0, sizeof( kvs::Real32 ) * nnodes * dimension );

//        m_vertices_array  = new kvs::Vector3f* [SIMD_BLK_SIZE]; //[nnodes];
//        m_scalars_array   = new T* [SIMD_BLK_SIZE]; //[nnodes];  
//        m_interpolation_functions_array = new kvs::Real32* [SIMD_BLK_SIZE]; // [nnodes];
//        m_differential_functions_array  = new kvs::Real32* [SIMD_BLK_SIZE]; // [nnodes*3];
//   
//        for (int i = 0; i<SIMD_BLK_SIZE; i++ )
//        {
//        m_vertices_array[i]  = new kvs::Vector3f[nnodes];
//        m_scalars_array[i]   = new T[nnodes] ;  
//        m_interpolation_functions_array[i] = new kvs::Real32[nnodes] ;
//        m_differential_functions_array[i]  = new kvs::Real32[nnodes*dimension] ;
//        } 

        m_vertices_array  = new kvs::Vector3f* [nnodes]; //[nnodes];
        m_scalars_array   = new T* [nnodes]; //[nnodes];  
        m_interpolation_functions_array = new kvs::Real32* [nnodes]; // [nnodes];
        m_differential_functions_array  = new kvs::Real32* [nnodes*dimension]; // [nnodes*3];
   
        for (int i = 0; i<nnodes; i++ )
        {
        m_vertices_array[i]  = new kvs::Vector3f[SIMD_BLK_SIZE];
        m_scalars_array[i]   = new T[SIMD_BLK_SIZE] ;  
        m_interpolation_functions_array[i] = new kvs::Real32[SIMD_BLK_SIZE] ;
        m_differential_functions_array[3*i]  = new kvs::Real32[SIMD_BLK_SIZE] ;
        m_differential_functions_array[3*i+1]  = new kvs::Real32[SIMD_BLK_SIZE] ;
        m_differential_functions_array[3*i+2]  = new kvs::Real32[SIMD_BLK_SIZE] ;
        } 

    }
    catch( char* error_message )
    {
        kvsMessageError( error_message );
        return;
    }
    // cal test
    cal_time[0] =  0; 
    cal_time[1] =  0; 
    cal_time[2] =  0; 
    cal_time[3] =  0; 
    cal_time[4] =  0; 
    cal_time[5] =  0; 
}

/*===========================================================================*/
/**
 *  @brief  Destructs the CellBase class.
 */
/*===========================================================================*/
template <typename T>
inline CellBase<T>::~CellBase()
{
    this->deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Binds a cell indicated by the index.
 *  @param  index [in] cell index
 */
/*===========================================================================*/
//template <typename T>  /// using UnstructuredVolumeObject!!
//inline void CellBase<T>::bindCell_wVolume( const kvs::UInt32 index, const size_t n )
//{
//    // Aliases.
//    const pbvr::UnstructuredVolumeObject* volume = m_reference_volume;
//    const kvs::UInt32* const connections = volume->connections().pointer();
//    const kvs::Real32* const coords = volume->coords().pointer();
//    const T* const values = static_cast<const T*>( volume->values().pointer() );
//    const size_t vnodes = volume->nnodes();
//
//
//    const size_t nnodes = m_nnodes;
////    std::cout << "vnodes =" << vnodes << ", nnodes = " <<  nnodes  << std::endl;
//    const kvs::UInt32 connection_index = nnodes * index;
//    for ( size_t i = 0, j = 0; i < nnodes; i++, j+=3 )
//    {
//        const kvs::UInt32 node_index = connections[ connection_index + i ];
//        m_scalars[i] = values[ vnodes * n + node_index ];
//
//        const kvs::UInt32 coord_index = 3 * node_index;
//        // modify by @hira at 2016/12/01
//        // m_vertices[i].set( coords[ coord_index ],
//        //                   coords[ coord_index + 1 ],
//        //                   coords[ coord_index + 2 ] );
//        // m_vertices_vec[j]     = coords[ coord_index ];
//        // m_vertices_vec[j + 1] = coords[ coord_index + 1 ];
//        // m_vertices_vec[j + 2] = coords[ coord_index + 2 ];
//        m_vertices[i].set(coords+coord_index, 3);
//        memcpy(m_vertices_vec+j, coords+coord_index, sizeof(kvs::Real32)*3);
//    }
//
//}

template <typename T>
inline void CellBase<T>::bindCell( const kvs::UInt32 index, const size_t n )
{
    // Aliases.
    const kvs::UInt32* const connections = m_connections;
    const kvs::Real32* const coords = m_coords;
    const T* const values = m_values;

    const size_t nnodes = m_nnodes;//num of polyhedra vertices
    const kvs::UInt32 connection_index = nnodes * index;
    for ( size_t i = 0, j=0 ;i < nnodes; i++, j += 3 )
    {
        const kvs::UInt32 node_index = connections[ connection_index + i ];
        m_scalars[i] = values[ node_index ];

        const kvs::UInt32 coord_index = 3 * node_index;
        //m_vertices[i].set( coords[ coord_index ],
        //                   coords[ coord_index + 1 ],
        //                   coords[ coord_index + 2 ] );
        //m_vertices_vec[i]     = coords[ coord_index ];
        //m_vertices_vec[i + 1] = coords[ coord_index + 1 ];
        //m_vertices_vec[i + 2] = coords[ coord_index + 2 ];
        m_vertices[i].set(coords+coord_index, 3);
        memcpy(m_vertices_vec+j, coords+coord_index, sizeof(kvs::Real32)*3);
    }
}

// Fj add
template <typename T>
inline void CellBase<T>::bindCellArray( const int loop_cnt, const kvs::UInt32 *cell_index )
{
    // Aliases.
    const kvs::UInt32* const connections = m_connections;
    const kvs::Real32* const coords = m_coords;
    const T* const values = m_values;
    const size_t nnodes = m_nnodes;//num of polyhedra vertices

    for ( size_t j = 0; j < nnodes; j++ )
    {
        for (int i =0; i<  loop_cnt; i++ )
        { 
            const kvs::UInt32 connection_index = nnodes * cell_index[i];
            m_scalars_array[j][i] = values[ connections[ connection_index + j ] ];

            m_vertices_array[j][i].set( coords[ 3 * connections[ connection_index + j ] ],
                    coords[ 3 * connections[ connection_index + j ] + 1 ],
                    coords[ 3 * connections[ connection_index + j ] + 2 ] );
//            m_scalars_array[i][j] = values[ connections[ connection_index + j ] ];
//
//            m_vertices_array[i][j].set( coords[ 3 * connections[ connection_index + j ] ],
//                    coords[ 3 * connections[ connection_index + j ] + 1 ],
//                    coords[ 3 * connections[ connection_index + j ] + 2 ] );
        }
    }
}
/*===========================================================================*/
/**
 *  @brief  Sets a point in the global coordinate.
 *  @param  global [in] coordinate value in the global coordinate
 */
/*===========================================================================*/
template <typename T>
inline void CellBase<T>::setGlobalPoint( const kvs::Vector3f& global ) const
{
    m_global_point = global;

    this->setLocalPoint( this->transformGlobalToLocal( global ) );
}

/*===========================================================================*/
/**
 *  @brief  Sets a point in the local coordinate.
 *  @param  local [in] coordinate value in the local coordinate
 */
/*===========================================================================*/
template <typename T>
inline void CellBase<T>::setLocalPoint( const kvs::Vector3f& local ) const
{
    m_local_point = local;

    this->interpolationFunctions( local );
    this->differentialFunctions( local );
}

// Fj add
template <typename T>
inline void CellBase<T>::setLocalPointArray( const int loop_cnt, const kvs::Vector3f *local_array )
{
////    kvs::Timer timer( kvs::Timer::Start );
//    for( int i = 0; i < loop_cnt; i++)
//    {
////        timer.start();
//        kvs::Vector3f local = local_array[i];
//        this->interpolationFunctions( local);
//        this->differentialFunctions( local );
////        timer.stop();
////        cal_time[0] += timer.sec();
////        timer.start();
//         
//        for (int j =0; j< m_nnodes; j++ )
//        {
//              m_interpolation_functions_array[j][i]   = m_interpolation_functions[j];
//              m_differential_functions_array [j][i]    = m_differential_functions[j];
//              m_differential_functions_array [j+1][i]  = m_differential_functions[j+1];
//              m_differential_functions_array [j+2][i]  = m_differential_functions[j+2];
//        }
////        timer.stop();
////        cal_time[1] += timer.sec();
//    }
    
              // add by shimomura 2023/06/23
            this->interpolationFunctions_array( local_array, loop_cnt);
            this->differentialFunctions_array(  local_array, loop_cnt);
}
/*===========================================================================*/
/**
 *  @brief  Transforms the global to the local coordinate.
 *  @param  global [in] point in the global coodinate
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::transformGlobalToLocal( const kvs::Vector3f& global ) const
{
    const kvs::Vector3f X( global );

    // Calculate the coordinate of 'global' in the local coordinate
    // by using Newton-Raphson method.
    const float TinyValue = static_cast<float>( 1.e-6 );
    const size_t MaxLoop = 100;
    kvs::Vector3f x0( 0.25f, 0.25f, 0.25f ); // Initial point in local coordinate.
    for ( size_t i = 0; i < MaxLoop; i++ )
    {
        this->setLocalPoint( x0 );
        const kvs::Vector3f X0( this->transformLocalToGlobal( x0 ) );
        const kvs::Vector3f dX( X - X0 );

        const kvs::Matrix33f J( this->JacobiMatrix() );
        const kvs::Vector3f dx = J.transpose().inverse() * dX;
        if ( dx.length() < TinyValue ) break; // Converged.

        x0 += dx;
    }

    return x0;
}

/*===========================================================================*/
/**
 *  @brief  Transforms the local to the global coordinate.
 *  @param  point [in] point in the local coordinate
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::transformLocalToGlobal( const kvs::Vector3f& local ) const
{
    kvs::IgnoreUnusedVariable( local );

    const float* N = m_interpolation_functions;
    const kvs::Vector3f* V = m_vertices;
    const size_t nnodes = m_nnodes;

    float X = 0;
    for ( size_t i = 0; i < nnodes; i++ ) X += N[i] * V[i].x();

    float Y = 0;
    for ( size_t i = 0; i < nnodes; i++ ) Y += N[i] * V[i].y();

    float Z = 0;
    for ( size_t i = 0; i < nnodes; i++ ) Z += N[i] * V[i].z();

    return kvs::Vector3f( X, Y, Z );
}

template <typename T>
inline void CellBase<T>::transformLocalToGlobalArray( const int loop_cnt, const kvs::Vector3f *local_array, kvs::Vector3f *global_array)
{

    #pragma ivdep
    for( int j = 0; j < loop_cnt; j++ )
    {
    kvs::IgnoreUnusedVariable( local_array[j] );
//    const float* N = m_interpolation_functions_array[j];
//    const kvs::Vector3f* V = m_vertices_array[j];
    //const kvs::Vector3f* V = m_vertices;
    const size_t nnodes = m_nnodes;


        float X = 0;
        for ( size_t i = 0; i < nnodes; i++ ) X += m_interpolation_functions_array[i][j] * m_vertices_array[i][j].x(); 
        float Y = 0;
        for ( size_t i = 0; i < nnodes; i++ ) Y += m_interpolation_functions_array[i][j] * m_vertices_array[i][j].y(); 
        float Z = 0;
        for ( size_t i = 0; i < nnodes; i++ ) Z += m_interpolation_functions_array[i][j] * m_vertices_array[i][j].z(); 
        global_array[j] = kvs::Vector3f( X, Y, Z );
    }
}
/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::randomSampling() const
{
    kvsMessageError( "'randomSampling' is not implemented." );
    return kvs::Vector3f( 0.0f, 0.0f, 0.0f );
}

//template <typename T>
////inline const kvs::Vector3f CellBase<T>::randomSampling_SFMT(sfmt_t &sfmt, const kvs::Vector3f *local_array, const int loop_cnt) const
////void CellBase<T>::randomSampling_SFMT(sfmt_t *sfmt, kvs::Vector3f *local_array, const int loop_cnt, std::vector<double> track)
////void CellBase<T>::randomSampling_SFMT(sfmt_t *sfmt, uint32_t *local_array, const int loop_cnt, std::vector<double> track)
//inline const kvs::Vector3f CellBase<T>::randomSampling_SFMT( sfmt_t *sfmt ) const
//{
//    kvsMessageError( "'randomSampling_SFMT' is not implemented." );
////    return kvs::Vector3f( 0.0f, 0.0f, 0.0f );
//}

template <typename T>
inline const kvs::Vector3f CellBase<T>::randomSampling_MT( kvs::MersenneTwister* MT ) const
{
    kvsMessageError( "'randomSampling_MT' is not implemented." );
    return kvs::Vector3f( 0.0f, 0.0f, 0.0f );
}
/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::volume() const
{
    kvsMessageError( "'volume' is not implemented." );
    return kvs::Real32( 0.0f );
}

/*===========================================================================*/
/**
 *  @brief  Returns the averaged scalar value.
 *  @return averaged scalar value
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::averagedScalar() const
{
    const size_t nnodes = m_nnodes;
    const kvs::Real32 w = 1.0f / nnodes;

    kvs::Real32 S = 0;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        S += static_cast<kvs::Real32>( m_scalars[i] );
    }

    return S * w;
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::scalar() const
{
    const size_t nnodes = m_nnodes;
    const float* N = m_interpolation_functions;
    const T* s = m_scalars;

    kvs::Real32 S = 0;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        S += static_cast<kvs::Real32>( N[i] * s[i] );
        //std::cout << "m_interpolation_functions[" << i <<"] =" << N[i] << ", m_scalars["<< i <<"] = " << s[i] <<std::endl;
    }

    return S;
}

/*===========================================================================*/
/**
 *  @brief  Returns the gradient vector at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::gradient() const
{
    // Calculate a gradient vector in the local coordinate.
    const kvs::UInt32 nnodes = m_nnodes;
    const float* dNdx = m_differential_functions;
    const float* dNdy = m_differential_functions + nnodes;
    const float* dNdz = m_differential_functions + nnodes * 2;
    const T* s = m_scalars;

    float dsdx = 0.0f;
    float dsdy = 0.0f;
    float dsdz = 0.0f;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        dsdx += static_cast<float>( s[i] * dNdx[i] );
        dsdy += static_cast<float>( s[i] * dNdy[i] );
        dsdz += static_cast<float>( s[i] * dNdz[i] );
    }

    const kvs::Vector3f g( dsdx, dsdy, dsdz );

    // Calculate a gradient vector in the global coordinate.
    const kvs::Matrix33f J = this->JacobiMatrix();

    float determinant = 0.0f;
    const kvs::Vector3f G = J.inverse( &determinant ) * g;

    return kvs::Math::IsZero( determinant ) ? kvs::Vector3f( 0.0f, 0.0f, 0.0f ) : G;
}

/*===========================================================================*/
/**
 *  @brief  Returns the glavity point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::localGravityPointValue() const
{
    this->setLocalGravityPoint();
    return this->scalar();
}

template <typename T>
inline const kvs::Vector3f* CellBase<T>::vertices() const
{
    return m_vertices;
}

template <typename T>
inline const T* CellBase<T>::scalars() const
{
    return m_scalars;
}

template <typename T>
inline T* CellBase<T>::setScalars()
{
    return m_scalars;
}
/*===========================================================================*/
/**
 *  @brief  Returns the global point.
 *  @param  coordinate value of the global point
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::globalPoint() const
{
    return m_global_point;
}

/*===========================================================================*/
/**
 *  @brief  Returns the local point.
 *  @param  coordinate value of the local point
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::localPoint() const
{
    return m_local_point;
}

/*===========================================================================*/
/**
 *  @brief  Returns number of nodes
 *  @param  coordinate value of the local point
 */
/*===========================================================================*/
template <typename T>
inline const size_t CellBase<T>::numberOfNodes() const
{
    return m_nnodes;
}

/*===========================================================================*/
/**
 *  @brief  Returns the jacobi matrix.
 *  @return Jacobi matrix
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Matrix33f CellBase<T>::JacobiMatrix() const
{
    const kvs::UInt32 nnodes = m_nnodes;
    const float* dNdx = m_differential_functions;
    const float* dNdy = m_differential_functions + nnodes;
    const float* dNdz = m_differential_functions + nnodes * 2;
    const float* vec  = m_vertices_vec;
    const kvs::Vector3f* V = m_vertices;

    float dXdx = 0;
    float dYdx = 0;
    float dZdx = 0;
    float dXdy = 0;
    float dYdy = 0;
    float dZdy = 0;
    float dXdz = 0;
    float dYdz = 0;
    float dZdz = 0;
    for ( size_t i = 0, j = 0; i < nnodes; i++, j+=3 )
    {
        dXdx += dNdx[i] * V[i].x();
        dYdx += dNdx[i] * V[i].y();
        dZdx += dNdx[i] * V[i].z();
        dXdy += dNdy[i] * V[i].x();
        dYdy += dNdy[i] * V[i].y();
        dZdy += dNdy[i] * V[i].z();
        dXdz += dNdz[i] * V[i].x();
        dYdz += dNdz[i] * V[i].y();
        dZdz += dNdz[i] * V[i].z();
    }
    
    return kvs::Matrix33f( dXdx, dYdx, dZdx, dXdy, dYdy, dZdy, dXdz, dYdz, dZdz );
}

/*===========================================================================*/
/**
 *  @brief  Returns a random number that is generated by using the xorshift.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::randomNumber() const
{


#ifdef KMATH
    double rv;
    /* #pragma omp critical(random) */
    /* { */
    km_random.get( rv );
    /* } */
    return static_cast<kvs::Real32>( rv - 1.0 );
#else
    // xorshift RGNs with period at least 2^128 - 1.
    static kvs::Real32 t24 = 1.0 / 16777216.0; /* 0.5**24 */
    static kvs::UInt32 x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    kvs::UInt32 t;
    t = ( x ^ ( x << 11 ) );
    x = y;
    y = z;
    z = w;
    w = ( w ^ ( w >> 19 ) ) ^ ( t ^ ( t >> 8 ) );

    return t24 * static_cast<kvs::Real32>( w >> 8 );
#endif
}

template <typename T>
void CellBase<T>::CalcScalarGrad(
    const int loop_cnt,
    float *scalar_array,
    float *grad_array_x,
    float *grad_array_y,
    float *grad_array_z )
{

//    kvs::Timer timer( kvs::Timer::Start );
//        //////////////////////////// scalar ////////////////////////////
//    #pragma ivdep
//    for( int i = 0; i < loop_cnt; i++ )
//    {
//        timer.start();
//        for (int j =0; j< m_nnodes; j++ )
//        {
//         m_interpolation_functions[j]  =  m_interpolation_functions_array[j][i];
//         m_scalars[j]                  =  m_scalars_array[j][i];   
//        }
//        timer.stop();
//        cal_time[2] += timer.sec();
//        timer.start();
//        scalar_array[i]  = this -> scalar();
//        timer.stop();
//        cal_time[3] += timer.sec();
//    }
// 
//
//    #pragma ivdep
//    for( int i = 0; i < loop_cnt; i++ )
//    {
//        timer.start();
//        for (int j =0; j< m_nnodes; j++ )
//        {
//         m_interpolation_functions[j]  =  m_interpolation_functions_array[j][i];
//         m_differential_functions[j]   =  m_differential_functions_array[j][i];   
//         m_vertices[j]                 =  m_vertices_array[j][i];
//        }
//      
//        timer.stop();
//        cal_time[4] += timer.sec();
//        timer.start();
//        const kvs::Vector3f grad_array = this -> gradient(); 
//        grad_array_x[i]  = grad_array.x();
//        grad_array_y[i]  = grad_array.y();
//        grad_array_z[i]  = grad_array.z();
//        timer.stop();
//        cal_time[5] += timer.sec();
//    }

//    timer.start();
//    this -> scalar_ary( scalar_array, loop_cnt);
//    timer.stop();
//    cal_time[2] += timer.sec();
//    timer.stop();
//    timer.start();
//    this -> grad_ary( grad_array_x, grad_array_y, grad_array_z, loop_cnt);
//    timer.stop();
//    cal_time[3] += timer.sec();

    this -> scalar_ary( scalar_array, loop_cnt);
    this -> grad_ary( grad_array_x, grad_array_y, grad_array_z, loop_cnt);
}

template <typename T>
void CellBase<T>::CalcAveragedScalarGrad(
    const int loop_cnt,
    float *scalar_array,
    float *grad_array_x,
    float *grad_array_y,
    float *grad_array_z )
{

       //////////////////////////// scalar ////////////////////////////
    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {
        for (int j =0; j< m_nnodes; j++ )
        {
//         m_interpolation_functions[j]  =  m_interpolation_functions_array[i][j];
//         m_scalars[j]                  =  m_scalars_array[i][j];
         m_interpolation_functions[j]  =  m_interpolation_functions_array[j][i];
         m_scalars[j]                  =  m_scalars_array[j][i];
        }
        scalar_array[i]  = this -> averagedScalar();
    }

    #pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {
        for (int j =0; j< m_nnodes; j++ )
        {
//         m_interpolation_functions[j]  =  m_interpolation_functions_array[i][j];
//         m_differential_functions[j]   =  m_differential_functions_array[i][j];
//         m_vertices[j]                 =  m_vertices_array[i][j];
         m_interpolation_functions[j]  =  m_interpolation_functions_array[j][i];
         m_differential_functions[j]   =  m_differential_functions_array[j][i];
         m_vertices[j]                 =  m_vertices_array[j][i];
        }

        const kvs::Vector3f grad_array = this -> gradient();
        grad_array_x[i]  = grad_array.x();
        grad_array_y[i]  = grad_array.y();
        grad_array_z[i]  = grad_array.z();
    }

}

template <typename T>
const double* CellBase<T>::output_time()
{
    return cal_time;
}

template <typename T>
void  CellBase<T>::reset_time()
{
   cal_time[0] = 0;
   cal_time[1] = 0;
}


} // end of namespace pbvr

#endif // KVS__TEST_CELL_BASE_H_INCLUDE
