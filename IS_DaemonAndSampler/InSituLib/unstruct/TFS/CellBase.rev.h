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
#include <kvs/ClassName>
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Message>

// Fj add
#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 512
#endif
// Fj add end


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
    kvsClassName( kvs::CellBase );

protected:

    size_t         m_nnodes; ///< number of nodes
    kvs::Vector3f* m_vertices; ///< coordinates of the nodes
    T*             m_scalars; ///< scalar values of the nodes
    kvs::Real32*   m_interpolation_functions; ///< interpolation functions
    kvs::Real32*   m_differential_functions; ///< differential functions
    mutable kvs::Vector3f  m_global_point; ///< sampling point in the global coordinate
    mutable kvs::Vector3f  m_local_point;  ///< sampling point in the local coordinate

    ///< reference unstructured volume
    T*           m_values;
    float*       m_coords;
    kvs::UInt32* m_connections;
    int          m_ncoords;
    int          m_ncells;

// Fj add
    //T m_scalars_array[ SIMD_BLK_SIZE ][8];
    //kvs::Vector3f m_vertices_array[ SIMD_BLK_SIZE ][8];
    //kvs::Real32   m_interpolation_functions_array[ SIMD_BLK_SIZE ][8];
    //kvs::Real32   m_differential_functions_array[ SIMD_BLK_SIZE ][24];
    T m_scalars_array[8][ SIMD_BLK_SIZE ];
    kvs::Vector3f m_vertices_array[8][ SIMD_BLK_SIZE ];
    kvs::Real32   m_interpolation_functions_array[8][ SIMD_BLK_SIZE ];
    kvs::Real32   m_differential_functions_array[24][ SIMD_BLK_SIZE ];
// Fj add end

public:

    CellBase( T* values,
              float* coords, int ncoords,
              unsigned int* connections, int ncells, int cell_type);

    virtual ~CellBase( void );

public:

    virtual const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const = 0;

    virtual const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const = 0;

    virtual void bindCell( const kvs::UInt32 index );

    virtual void setGlobalPoint( const kvs::Vector3f& point ) const;

    virtual void setLocalPoint( const kvs::Vector3f& point ) const;

    virtual const kvs::Vector3f transformGlobalToLocal( const kvs::Vector3f& point ) const;

    virtual const kvs::Vector3f transformLocalToGlobal( const kvs::Vector3f& point ) const;

    //Fj add
    virtual void bindCellArray( const int loop_cnt, const kvs::UInt32 *cell_index );
    virtual void setLocalPointArray( const int loop_cnt, const kvs::Vector3f *local_array );
    virtual void transformLocalToGlobalArray( const int loop_cnt, const kvs::Vector3f *local_array, kvs::Vector3f *global_array);

/*
    virtual void CalcScalarGrad(
        const int loop_cnt,
        const int *cell_index,
        const float *local_coord_x,
        const float *local_coord_y,
        const float *local_coord_z,
        float *scalar_array,
        float *grad_array_x,
        float *grad_array_y,
        float *grad_array_z ) const;
*/
    virtual void CalcScalarGrad(
        const int loop_cnt,
        float *scalar_array,
        float *grad_array_x,
        float *grad_array_y,
        float *grad_array_z );
    //Fj add end

public:

    virtual const kvs::Vector3f randomSampling( void ) const;

    virtual const kvs::Real32 volume( void ) const;

    virtual const kvs::Real32 averagedScalar( void ) const;

    virtual const kvs::Real32 scalar( void ) const;

    virtual const kvs::Vector3f gradient( void ) const;

public:

    const kvs::Vector3f* vertices( void ) const;

    const T* scalars( void ) const;

    const kvs::Vector3f globalPoint( void ) const;

    const kvs::Vector3f localPoint( void ) const;

    const kvs::Matrix33f JacobiMatrix( void ) const;

    const kvs::Real32 randomNumber( void ) const;

};

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
    }
    catch( char* error_message )
    {
        kvsMessageError( error_message );
        return;
    }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the CellBase class.
 */
/*===========================================================================*/
template <typename T>
inline CellBase<T>::~CellBase( void )
{
    if ( m_vertices ) delete [] m_vertices;
    if ( m_scalars ) delete [] m_scalars;
    if ( m_interpolation_functions ) delete [] m_interpolation_functions;
    if ( m_differential_functions ) delete [] m_differential_functions;
}

/*===========================================================================*/
/**
 *  @brief  Binds a cell indicated by the index.
 *  @param  index [in] cell index
 */
/*===========================================================================*/
template <typename T>
inline void CellBase<T>::bindCell( const kvs::UInt32 index )
{
    // Aliases.
    const kvs::UInt32* const connections = m_connections;
    const kvs::Real32* const coords = m_coords;
    const T* const values = m_values;

    const size_t nnodes = m_nnodes;//num of polyhedra vertices
    const kvs::UInt32 connection_index = nnodes * index;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        const kvs::UInt32 node_index = connections[ connection_index + i ];
        m_scalars[i] = values[ node_index ];

        const kvs::UInt32 coord_index = 3 * node_index;
        m_vertices[i].set( coords[ coord_index ],
                           coords[ coord_index + 1 ],
                           coords[ coord_index + 2 ] );
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

#pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {
        const kvs::UInt32 connection_index = nnodes * cell_index[i];

        //m_scalars_array[i][0] = values[ connections[ connection_index + 0 ] ];
        //m_scalars_array[i][1] = values[ connections[ connection_index + 1 ] ];
        //m_scalars_array[i][2] = values[ connections[ connection_index + 2 ] ];
        //m_scalars_array[i][3] = values[ connections[ connection_index + 3 ] ];
        //m_scalars_array[i][4] = values[ connections[ connection_index + 4 ] ];
        //m_scalars_array[i][5] = values[ connections[ connection_index + 5 ] ];
        //m_scalars_array[i][6] = values[ connections[ connection_index + 6 ] ];
        //m_scalars_array[i][7] = values[ connections[ connection_index + 7 ] ];
        m_scalars_array[0][i] = values[ connections[ connection_index + 0 ] ];
        m_scalars_array[1][i] = values[ connections[ connection_index + 1 ] ];
        m_scalars_array[2][i] = values[ connections[ connection_index + 2 ] ];
        m_scalars_array[3][i] = values[ connections[ connection_index + 3 ] ];
        m_scalars_array[4][i] = values[ connections[ connection_index + 4 ] ];
        m_scalars_array[5][i] = values[ connections[ connection_index + 5 ] ];
        m_scalars_array[6][i] = values[ connections[ connection_index + 6 ] ];
        m_scalars_array[7][i] = values[ connections[ connection_index + 7 ] ];
/*
        m_vertices_array[i][0].set( coords[ 3 * connections[ connection_index + 0 ] ],
                                    coords[ 3 * connections[ connection_index + 0 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 0 ] + 2 ] );

        m_vertices_array[i][1].set( coords[ 3 * connections[ connection_index + 1 ] ],
                                    coords[ 3 * connections[ connection_index + 1 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 1 ] + 2 ] );

        m_vertices_array[i][2].set( coords[ 3 * connections[ connection_index + 2 ] ],
                                    coords[ 3 * connections[ connection_index + 2 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 2 ] + 2 ] );

        m_vertices_array[i][3].set( coords[ 3 * connections[ connection_index + 3 ] ],
                                    coords[ 3 * connections[ connection_index + 3 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 3 ] + 2 ] );

        m_vertices_array[i][4].set( coords[ 3 * connections[ connection_index + 4 ] ],
                                    coords[ 3 * connections[ connection_index + 4 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 4 ] + 2 ] );

        m_vertices_array[i][5].set( coords[ 3 * connections[ connection_index + 5 ] ],
                                    coords[ 3 * connections[ connection_index + 5 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 5 ] + 2 ] );

        m_vertices_array[i][6].set( coords[ 3 * connections[ connection_index + 6 ] ],
                                    coords[ 3 * connections[ connection_index + 6 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 6 ] + 2 ] );

        m_vertices_array[i][7].set( coords[ 3 * connections[ connection_index + 7 ] ],
                                    coords[ 3 * connections[ connection_index + 7 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 7 ] + 2 ] );
*/
        m_vertices_array[0][i].set( coords[ 3 * connections[ connection_index + 0 ] ],
                                    coords[ 3 * connections[ connection_index + 0 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 0 ] + 2 ] );
        m_vertices_array[1][i].set( coords[ 3 * connections[ connection_index + 1 ] ],
                                    coords[ 3 * connections[ connection_index + 1 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 1 ] + 2 ] );
        m_vertices_array[2][i].set( coords[ 3 * connections[ connection_index + 2 ] ],
                                    coords[ 3 * connections[ connection_index + 2 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 2 ] + 2 ] );
        m_vertices_array[3][i].set( coords[ 3 * connections[ connection_index + 3 ] ],
                                    coords[ 3 * connections[ connection_index + 3 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 3 ] + 2 ] );
        m_vertices_array[4][i].set( coords[ 3 * connections[ connection_index + 4 ] ],
                                    coords[ 3 * connections[ connection_index + 4 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 4 ] + 2 ] );
        m_vertices_array[5][i].set( coords[ 3 * connections[ connection_index + 5 ] ],
                                    coords[ 3 * connections[ connection_index + 5 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 5 ] + 2 ] );
        m_vertices_array[6][i].set( coords[ 3 * connections[ connection_index + 6 ] ],
                                    coords[ 3 * connections[ connection_index + 6 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 6 ] + 2 ] );
        m_vertices_array[7][i].set( coords[ 3 * connections[ connection_index + 7 ] ],
                                    coords[ 3 * connections[ connection_index + 7 ] + 1 ],
                                    coords[ 3 * connections[ connection_index + 7 ] + 2 ] );
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

    for( int i = 0; i < loop_cnt; i++)
    {
        //this->interpolationFunctions( local );
        const float x = local_array[i].x();
        const float y = local_array[i].y();
        const float z = local_array[i].z();

        const float xy = x * y;
        const float yz = y * z;
        const float zx = z * x;

        const float xyz = xy * z;

        //m_interpolation_functions_array[i][0] = z - zx - yz + xyz;
        //m_interpolation_functions_array[i][1] = zx - xyz;
        //m_interpolation_functions_array[i][2] = xyz;
        //m_interpolation_functions_array[i][3] = yz - xyz;
        //m_interpolation_functions_array[i][4] = 1.0f - x - y - z + xy + yz + zx - xyz;
        //m_interpolation_functions_array[i][5] = x - xy - zx + xyz;
        //m_interpolation_functions_array[i][6] = xy - xyz;
        //m_interpolation_functions_array[i][7] = y - xy - yz + xyz;
        m_interpolation_functions_array[0][i] = z - zx - yz + xyz;
        m_interpolation_functions_array[1][i] = zx - xyz;
        m_interpolation_functions_array[2][i] = xyz;
        m_interpolation_functions_array[3][i] = yz - xyz;
        m_interpolation_functions_array[4][i] = 1.0f - x - y - z + xy + yz + zx - xyz;
        m_interpolation_functions_array[5][i] = x - xy - zx + xyz;
        m_interpolation_functions_array[6][i] = xy - xyz;
        m_interpolation_functions_array[7][i] = y - xy - yz + xyz;

        //this->differentialFunctions( local );
        // dNdx
        //m_differential_functions_array[i][ 0] =  - z + yz;
        //m_differential_functions_array[i][ 1] =  z - yz;
        //m_differential_functions_array[i][ 2] =  yz;
        //m_differential_functions_array[i][ 3] =  - yz;
        //m_differential_functions_array[i][ 4] =  - 1.0f + y +z - yz;
        //m_differential_functions_array[i][ 5] =  1.0f - y - z + yz;
        //m_differential_functions_array[i][ 6] =  y - yz;
        //m_differential_functions_array[i][ 7] =  - y + yz;
        m_differential_functions_array[ 0][i] =  - z + yz;
        m_differential_functions_array[ 1][i] =  z - yz;
        m_differential_functions_array[ 2][i] =  yz;
        m_differential_functions_array[ 3][i] =  - yz;
        m_differential_functions_array[ 4][i] =  - 1.0f + y +z - yz;
        m_differential_functions_array[ 5][i] =  1.0f - y - z + yz;
        m_differential_functions_array[ 6][i] =  y - yz;
        m_differential_functions_array[ 7][i] =  - y + yz;
        // dNdy
        //m_differential_functions_array[i][ 8] =  - z + zx;
        //m_differential_functions_array[i][ 9] =  - zx;
        //m_differential_functions_array[i][10] =  zx;
        //m_differential_functions_array[i][11] =  z - zx;
        //m_differential_functions_array[i][12] =  - 1.0f + x + z - zx;
        //m_differential_functions_array[i][13] =  - x + zx;
        //m_differential_functions_array[i][14] =  x - zx;
        //m_differential_functions_array[i][15] =  1.0f - x - z + zx;
        m_differential_functions_array[ 8][i] =  - z + zx;
        m_differential_functions_array[ 9][i] =  - zx;
        m_differential_functions_array[10][i] =  zx;
        m_differential_functions_array[11][i] =  z - zx;
        m_differential_functions_array[12][i] =  - 1.0f + x + z - zx;
        m_differential_functions_array[13][i] =  - x + zx;
        m_differential_functions_array[14][i] =  x - zx;
        m_differential_functions_array[15][i] =  1.0f - x - z + zx;
        // dNdz
        //m_differential_functions_array[i][16] =  1.0f - y - x + xy;
        //m_differential_functions_array[i][17] =  x - xy;
        //m_differential_functions_array[i][18] =  xy;
        //m_differential_functions_array[i][19] =  y - xy;
        //m_differential_functions_array[i][20] =  - 1.0f + y + x - xy;
        //m_differential_functions_array[i][21] =  - x + xy;
        //m_differential_functions_array[i][22] =  - xy;
        //m_differential_functions_array[i][23] =  - y + xy;
        m_differential_functions_array[16][i] =  1.0f - y - x + xy;
        m_differential_functions_array[17][i] =  x - xy;
        m_differential_functions_array[18][i] =  xy;
        m_differential_functions_array[19][i] =  y - xy;
        m_differential_functions_array[20][i] =  - 1.0f + y + x - xy;
        m_differential_functions_array[21][i] =  - x + xy;
        m_differential_functions_array[22][i] =  - xy;
        m_differential_functions_array[23][i] =  - y + xy;

    }
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

    return( x0 );
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

    return( kvs::Vector3f( X, Y, Z ) );
}

// Fj add
template <typename T>
inline void CellBase<T>::transformLocalToGlobalArray( const int loop_cnt, const kvs::Vector3f *local_array, kvs::Vector3f *global_array)
{

#pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {
        kvs::IgnoreUnusedVariable( local_array[i] );

       /* float X = m_interpolation_functions_array[i][0] * m_vertices_array[i][0].x()
                + m_interpolation_functions_array[i][1] * m_vertices_array[i][1].x()
                + m_interpolation_functions_array[i][2] * m_vertices_array[i][2].x()
                + m_interpolation_functions_array[i][3] * m_vertices_array[i][3].x()
                + m_interpolation_functions_array[i][4] * m_vertices_array[i][4].x()
                + m_interpolation_functions_array[i][5] * m_vertices_array[i][5].x()
                + m_interpolation_functions_array[i][6] * m_vertices_array[i][6].x()
                + m_interpolation_functions_array[i][7] * m_vertices_array[i][7].x();*/
        float X = m_interpolation_functions_array[0][i] * m_vertices_array[0][i].x()
                + m_interpolation_functions_array[1][i] * m_vertices_array[1][i].x()
                + m_interpolation_functions_array[2][i] * m_vertices_array[2][i].x()
                + m_interpolation_functions_array[3][i] * m_vertices_array[3][i].x()
                + m_interpolation_functions_array[4][i] * m_vertices_array[4][i].x()
                + m_interpolation_functions_array[5][i] * m_vertices_array[5][i].x()
                + m_interpolation_functions_array[6][i] * m_vertices_array[6][i].x()
                + m_interpolation_functions_array[7][i] * m_vertices_array[7][i].x();

        /*float Y = m_interpolation_functions_array[i][0] * m_vertices_array[i][0].y()
                + m_interpolation_functions_array[i][1] * m_vertices_array[i][1].y()
                + m_interpolation_functions_array[i][2] * m_vertices_array[i][2].y()
                + m_interpolation_functions_array[i][3] * m_vertices_array[i][3].y()
                + m_interpolation_functions_array[i][4] * m_vertices_array[i][4].y()
                + m_interpolation_functions_array[i][5] * m_vertices_array[i][5].y()
                + m_interpolation_functions_array[i][6] * m_vertices_array[i][6].y()
                + m_interpolation_functions_array[i][7] * m_vertices_array[i][7].y();*/
        float Y = m_interpolation_functions_array[0][i] * m_vertices_array[0][i].y()
                + m_interpolation_functions_array[1][i] * m_vertices_array[1][i].y()
                + m_interpolation_functions_array[2][i] * m_vertices_array[2][i].y()
                + m_interpolation_functions_array[3][i] * m_vertices_array[3][i].y()
                + m_interpolation_functions_array[4][i] * m_vertices_array[4][i].y()
                + m_interpolation_functions_array[5][i] * m_vertices_array[5][i].y()
                + m_interpolation_functions_array[6][i] * m_vertices_array[6][i].y()
                + m_interpolation_functions_array[7][i] * m_vertices_array[7][i].y();

        /*float Z = m_interpolation_functions_array[i][0] * m_vertices_array[i][0].z()
                + m_interpolation_functions_array[i][1] * m_vertices_array[i][1].z()
                + m_interpolation_functions_array[i][2] * m_vertices_array[i][2].z()
                + m_interpolation_functions_array[i][3] * m_vertices_array[i][3].z()
                + m_interpolation_functions_array[i][4] * m_vertices_array[i][4].z()
                + m_interpolation_functions_array[i][5] * m_vertices_array[i][5].z()
                + m_interpolation_functions_array[i][6] * m_vertices_array[i][6].z()
                + m_interpolation_functions_array[i][7] * m_vertices_array[i][7].z();*/
        float Z = m_interpolation_functions_array[0][i] * m_vertices_array[0][i].z()
                + m_interpolation_functions_array[1][i] * m_vertices_array[1][i].z()
                + m_interpolation_functions_array[2][i] * m_vertices_array[2][i].z()
                + m_interpolation_functions_array[3][i] * m_vertices_array[3][i].z()
                + m_interpolation_functions_array[4][i] * m_vertices_array[4][i].z()
                + m_interpolation_functions_array[5][i] * m_vertices_array[5][i].z()
                + m_interpolation_functions_array[6][i] * m_vertices_array[6][i].z()
                + m_interpolation_functions_array[7][i] * m_vertices_array[7][i].z();

        global_array[i] = kvs::Vector3f( X, Y, Z );

    }

}
/*===========================================================================*/
/**
 *  @brief  Returns the sampled point randomly in the cell.
 *  @return coordinate value of the sampled point
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::randomSampling( void ) const
{
    kvsMessageError("'randomSampling' is not implemented.");
    return( kvs::Vector3f( 0.0f, 0.0f, 0.0f ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the volume of the cell.
 *  @return volume of the cell
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::volume( void ) const
{
    kvsMessageError("'volume' is not implemented.");
    return( kvs::Real32( 0.0f ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns the averaged scalar value.
 *  @return averaged scalar value
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::averagedScalar( void ) const
{
    const size_t nnodes = m_nnodes;
    const kvs::Real32 w = 1.0f / nnodes;

    kvs::Real32 S = 0;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        S += static_cast<kvs::Real32>( m_scalars[i] );
    }

    return( S * w );
}

/*===========================================================================*/
/**
 *  @brief  Returns the interpolated scalar value at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::scalar( void ) const
{
    const size_t nnodes = m_nnodes;
    const float* N = m_interpolation_functions;
    const T* s = m_scalars;

    kvs::Real32 S = 0;
    for ( size_t i = 0; i < nnodes; i++ )
    {
        S += static_cast<kvs::Real32>( N[i] * s[i] );
    }

    return( S );
}

/*===========================================================================*/
/**
 *  @brief  Returns the gradient vector at the attached point.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::gradient( void ) const
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

    return( kvs::Math::IsZero( determinant ) ? kvs::Vector3f( 0.0f, 0.0f, 0.0f ) : G );
}

template <typename T>
inline const kvs::Vector3f* CellBase<T>::vertices( void ) const
{
    return( m_vertices );
}

template <typename T>
inline const T* CellBase<T>::scalars( void ) const
{
    return( m_scalars );
}

/*===========================================================================*/
/**
 *  @brief  Returns the global point.
 *  @param  coordinate value of the global point
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::globalPoint( void ) const
{
    return( m_global_point );
}

/*===========================================================================*/
/**
 *  @brief  Returns the local point.
 *  @param  coordinate value of the local point
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Vector3f CellBase<T>::localPoint( void ) const
{
    return( m_local_point );
}

/*===========================================================================*/
/**
 *  @brief  Returns the jacobi matrix.
 *  @return Jacobi matrix
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Matrix33f CellBase<T>::JacobiMatrix( void ) const
{
    const kvs::UInt32 nnodes = m_nnodes;
    const float* dNdx = m_differential_functions;
    const float* dNdy = m_differential_functions + nnodes;
    const float* dNdz = m_differential_functions + nnodes * 2;
    const kvs::Vector3f* V = m_vertices;

    float dXdx = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dXdx += dNdx[i]*V[i].x();

    float dYdx = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dYdx += dNdx[i]*V[i].y();

    float dZdx = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dZdx += dNdx[i]*V[i].z();


    float dXdy = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dXdy += dNdy[i]*V[i].x();

    float dYdy = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dYdy += dNdy[i]*V[i].y();

    float dZdy = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dZdy += dNdy[i]*V[i].z();


    float dXdz = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dXdz += dNdz[i]*V[i].x();

    float dYdz = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dYdz += dNdz[i]*V[i].y();

    float dZdz = 0;
    for ( size_t i = 0; i < nnodes; i++ ) dZdz += dNdz[i]*V[i].z();

    return( kvs::Matrix33f( dXdx, dYdx, dZdx, dXdy, dYdy, dZdy, dXdz, dYdz, dZdz ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns a random number that is generated by using the xorshift.
 */
/*===========================================================================*/
template <typename T>
inline const kvs::Real32 CellBase<T>::randomNumber( void ) const
{
    // xorshift RGNs with period at least 2^128 - 1.
    static kvs::Real32 t24 = 1.0/16777216.0; /* 0.5**24 */
    static kvs::UInt32 x=123456789,y=362436069,z=521288629,w=88675123;
    kvs::UInt32 t;
    t=(x^(x<<11));
    x=y;y=z;z=w;
    w=(w^(w>>19))^(t^(t>>8));

    return( t24*static_cast<kvs::Real32>(w>>8) );
}

//Fj add
/*
template <typename T>
void CellBase<T>::CalcScalarGrad(
    const int loop_cnt,
    const int *cell_index,
    const float *local_coord_x,
    const float *local_coord_y,
    const float *local_coord_z,
    float *scalar_array,
    float *grad_array_x,
    float *grad_array_y,
    float *grad_array_z ) const
{

    //m_nnodes = 8 として変数を作成している
    const int nnodes = m_nnodes;

    for( int i = 0; i < loop_cnt; i++ )
    {
        //////////////////////////// bindCell ////////////////////////////

        const T scalars_0 = m_values[ m_connections[ nnodes * cell_index[i] + 0 ] ];
        const T scalars_1 = m_values[ m_connections[ nnodes * cell_index[i] + 1 ] ];
        const T scalars_2 = m_values[ m_connections[ nnodes * cell_index[i] + 2 ] ];
        const T scalars_3 = m_values[ m_connections[ nnodes * cell_index[i] + 3 ] ];
        const T scalars_4 = m_values[ m_connections[ nnodes * cell_index[i] + 4 ] ];
        const T scalars_5 = m_values[ m_connections[ nnodes * cell_index[i] + 5 ] ];
        const T scalars_6 = m_values[ m_connections[ nnodes * cell_index[i] + 6 ] ];
        const T scalars_7 = m_values[ m_connections[ nnodes * cell_index[i] + 7 ] ];

        const kvs::UInt32 coord_index_0 = 3 * m_connections[ nnodes * cell_index[i] + 0 ];
        const kvs::UInt32 coord_index_1 = 3 * m_connections[ nnodes * cell_index[i] + 1 ];
        const kvs::UInt32 coord_index_2 = 3 * m_connections[ nnodes * cell_index[i] + 2 ];
        const kvs::UInt32 coord_index_3 = 3 * m_connections[ nnodes * cell_index[i] + 3 ];
        const kvs::UInt32 coord_index_4 = 3 * m_connections[ nnodes * cell_index[i] + 4 ];
        const kvs::UInt32 coord_index_5 = 3 * m_connections[ nnodes * cell_index[i] + 5 ];
        const kvs::UInt32 coord_index_6 = 3 * m_connections[ nnodes * cell_index[i] + 6 ];
        const kvs::UInt32 coord_index_7 = 3 * m_connections[ nnodes * cell_index[i] + 7 ];

        const kvs::Vector3f vertices_0 
            = kvs::Vector3f ( m_coords[coord_index_0], m_coords[coord_index_0 +1], m_coords[coord_index_0 +2] );

        const kvs::Vector3f vertices_1
            = kvs::Vector3f ( m_coords[coord_index_1], m_coords[coord_index_1 +1], m_coords[coord_index_1 +2] );

        const kvs::Vector3f vertices_2
            = kvs::Vector3f ( m_coords[coord_index_2], m_coords[coord_index_2 +1], m_coords[coord_index_2 +2] );

        const kvs::Vector3f vertices_3
            = kvs::Vector3f ( m_coords[coord_index_3], m_coords[coord_index_3 +1], m_coords[coord_index_3 +2] );

        const kvs::Vector3f vertices_4
            = kvs::Vector3f ( m_coords[coord_index_4], m_coords[coord_index_4 +1], m_coords[coord_index_4 +2] );

        const kvs::Vector3f vertices_5
            = kvs::Vector3f ( m_coords[coord_index_5], m_coords[coord_index_5 +1], m_coords[coord_index_5 +2] );

        const kvs::Vector3f vertices_6
            = kvs::Vector3f ( m_coords[coord_index_6], m_coords[coord_index_6 +1], m_coords[coord_index_6 +2] );

        const kvs::Vector3f vertices_7
            = kvs::Vector3f ( m_coords[coord_index_7], m_coords[coord_index_7 +1], m_coords[coord_index_7 +2] );

        //////////////////////////// bindCell ////////////////////////////

        ///////////////////// interpolationFunctions /////////////////////

        const float x = local_coord_x[i];
        const float y = local_coord_y[i];
        const float z = local_coord_z[i];

        const float xy = x * y;
        const float yz = y * z;
        const float zx = z * x;

        const float xyz = xy * z;

        //変数を追加
        const float interpolation_functions_0 = z - zx - yz + xyz;
        const float interpolation_functions_1 = zx - xyz;
        const float interpolation_functions_2 = xyz;
        const float interpolation_functions_3 = yz - xyz;
        const float interpolation_functions_4 = 1.0f - x - y - z + xy + yz + zx - xyz;
        const float interpolation_functions_5 = x - xy - zx + xyz;
        const float interpolation_functions_6 = xy - xyz;
        const float interpolation_functions_7 = y - xy - yz + xyz;

        ///////////////////// interpolationFunctions /////////////////////

        ///////////////////// differentialFunctions /////////////////////

        //変数の追加
        // dNdx
        const float differential_functions_0  =  - z + yz;
        const float differential_functions_1  =  z - yz;
        const float differential_functions_2  =  yz;
        const float differential_functions_3  =  - yz;
        const float differential_functions_4  =  - 1.0f + y +z - yz;
        const float differential_functions_5  =  1.0f - y - z + yz;
        const float differential_functions_6  =  y - yz;
        const float differential_functions_7  =  - y + yz;
        // dNdy
        const float differential_functions_8  =  - z + zx;
        const float differential_functions_9  =  - zx;
        const float differential_functions_10 =  zx;
        const float differential_functions_11 =  z - zx;
        const float differential_functions_12 =  - 1.0f + x + z - zx;
        const float differential_functions_13 =  - x + zx;
        const float differential_functions_14 =  x - zx;
        const float differential_functions_15 =  1.0f - x - z + zx;
        // dNdz
        const float differential_functions_16 =  1.0f - y - x + xy;
        const float differential_functions_17 =  x - xy;
        const float differential_functions_18 =  xy;
        const float differential_functions_19 =  y - xy;
        const float differential_functions_20 =  - 1.0f + y + x - xy;
        const float differential_functions_21 =  - x + xy;
        const float differential_functions_22 =  - xy;
        const float differential_functions_23 =  - y + xy;

        ///////////////////// differentialFunctions /////////////////////

        //////////////////////////// scalar ////////////////////////////

        scalar_array[i] 
             = static_cast<kvs::Real32> ( ( interpolation_functions_0 ) * scalars_0 )
                                      + ( ( interpolation_functions_1 ) * scalars_1 )
                                      + ( ( interpolation_functions_2 ) * scalars_2 )
                                      + ( ( interpolation_functions_3 ) * scalars_3 )
                                      + ( ( interpolation_functions_4 ) * scalars_4 )
                                      + ( ( interpolation_functions_5 ) * scalars_5 )
                                      + ( ( interpolation_functions_6 ) * scalars_6 )
                                      + ( ( interpolation_functions_7 ) * scalars_7 );

        //////////////////////////// scalar ////////////////////////////

        /////////////////////////// gradient ///////////////////////////

        const float dsdx 
            = static_cast<float>( scalars_0 * differential_functions_0  )
                              + ( scalars_1 * differential_functions_1  )
                              + ( scalars_2 * differential_functions_2  )
                              + ( scalars_3 * differential_functions_3  )
                              + ( scalars_4 * differential_functions_4  )
                              + ( scalars_5 * differential_functions_5  )
                              + ( scalars_6 * differential_functions_6  )
                              + ( scalars_7 * differential_functions_7  );

        const float dsdy 
            = static_cast<float>( scalars_0 * differential_functions_8  )
                              + ( scalars_1 * differential_functions_9  )
                              + ( scalars_2 * differential_functions_10 )
                              + ( scalars_3 * differential_functions_11 )
                              + ( scalars_4 * differential_functions_12 )
                              + ( scalars_5 * differential_functions_13 )
                              + ( scalars_6 * differential_functions_14 )
                              + ( scalars_7 * differential_functions_15 );

        const float dsdz 
            = static_cast<float>( scalars_0 * differential_functions_16 )
                              + ( scalars_1 * differential_functions_17 )
                              + ( scalars_2 * differential_functions_18 )
                              + ( scalars_3 * differential_functions_19 )
                              + ( scalars_4 * differential_functions_20 )
                              + ( scalars_5 * differential_functions_21 )
                              + ( scalars_6 * differential_functions_22 )
                              + ( scalars_7 * differential_functions_23 );

        const kvs::Vector3f g( dsdx, dsdy, dsdz );

        ///////////////////////// JacobiMatrix /////////////////////////

          const float dXdx = ( differential_functions_0  * vertices_0.x() )
                           + ( differential_functions_1  * vertices_1.x() )
                           + ( differential_functions_2  * vertices_2.x() )
                           + ( differential_functions_3  * vertices_3.x() )
                           + ( differential_functions_4  * vertices_4.x() )
                           + ( differential_functions_5  * vertices_5.x() )
                           + ( differential_functions_6  * vertices_6.x() )
                           + ( differential_functions_7  * vertices_7.x() );

          const float dYdx = ( differential_functions_0  * vertices_0.y() )
                           + ( differential_functions_1  * vertices_1.y() )
                           + ( differential_functions_2  * vertices_2.y() )
                           + ( differential_functions_3  * vertices_3.y() )
                           + ( differential_functions_4  * vertices_4.y() )
                           + ( differential_functions_5  * vertices_5.y() )
                           + ( differential_functions_6  * vertices_6.y() )
                           + ( differential_functions_7  * vertices_7.y() );

          const float dZdx = ( differential_functions_0  * vertices_0.z() )
                           + ( differential_functions_1  * vertices_1.z() )
                           + ( differential_functions_2  * vertices_2.z() )
                           + ( differential_functions_3  * vertices_3.z() )
                           + ( differential_functions_4  * vertices_4.z() )
                           + ( differential_functions_5  * vertices_5.z() )
                           + ( differential_functions_6  * vertices_6.z() )
                           + ( differential_functions_7  * vertices_7.z() );


          const float dXdy = ( differential_functions_8  * vertices_0.x() )
                           + ( differential_functions_9  * vertices_1.x() )
                           + ( differential_functions_10 * vertices_2.x() )
                           + ( differential_functions_11 * vertices_3.x() )
                           + ( differential_functions_12 * vertices_4.x() )
                           + ( differential_functions_13 * vertices_5.x() )
                           + ( differential_functions_14 * vertices_6.x() )
                           + ( differential_functions_15 * vertices_7.x() );

          const float dYdy = ( differential_functions_8  * vertices_0.y() )
                           + ( differential_functions_9  * vertices_1.y() )
                           + ( differential_functions_10 * vertices_2.y() )
                           + ( differential_functions_11 * vertices_3.y() )
                           + ( differential_functions_12 * vertices_4.y() )
                           + ( differential_functions_13 * vertices_5.y() )
                           + ( differential_functions_14 * vertices_6.y() )
                           + ( differential_functions_15 * vertices_7.y() );

          const float dZdy = ( differential_functions_8  * vertices_0.z() )
                           + ( differential_functions_9  * vertices_1.z() )
                           + ( differential_functions_10 * vertices_2.z() )
                           + ( differential_functions_11 * vertices_3.z() )
                           + ( differential_functions_12 * vertices_4.z() )
                           + ( differential_functions_13 * vertices_5.z() )
                           + ( differential_functions_14 * vertices_6.z() )
                           + ( differential_functions_15 * vertices_7.z() );


          const float dXdz = ( differential_functions_16 * vertices_0.x() )
                           + ( differential_functions_17 * vertices_1.x() )
                           + ( differential_functions_18 * vertices_2.x() )
                           + ( differential_functions_19 * vertices_3.x() )
                           + ( differential_functions_20 * vertices_4.x() )
                           + ( differential_functions_21 * vertices_5.x() )
                           + ( differential_functions_22 * vertices_6.x() )
                           + ( differential_functions_23 * vertices_7.x() );

          const float dYdz =( differential_functions_16 * vertices_0.y() )
                           + ( differential_functions_17 * vertices_1.y() )
                           + ( differential_functions_18 * vertices_2.y() )
                           + ( differential_functions_19 * vertices_3.y() )
                           + ( differential_functions_20 * vertices_4.y() )
                           + ( differential_functions_21 * vertices_5.y() )
                           + ( differential_functions_22 * vertices_6.y() )
                           + ( differential_functions_23 * vertices_7.y() );

          const float dZdz = ( differential_functions_16 * vertices_0.z() )
                           + ( differential_functions_17 * vertices_1.z() )
                           + ( differential_functions_18 * vertices_2.z() )
                           + ( differential_functions_19 * vertices_3.z() )
                           + ( differential_functions_20 * vertices_4.z() )
                           + ( differential_functions_21 * vertices_5.z() )
                           + ( differential_functions_22 * vertices_6.z() )
                           + ( differential_functions_23 * vertices_7.z() );

          const kvs::Matrix33f J 
              = kvs::Matrix33f( dXdx, dYdx, dZdx, dXdy, dYdy, dZdy, dXdz, dYdz, dZdz );

        ///////////////////////// JacobiMatrix /////////////////////////

         float determinant = 0.0f;
         const kvs::Vector3f G = J.inverse( &determinant ) * g;

         grad_array_x[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.x();
         grad_array_y[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.y();
         grad_array_z[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.z();

        /////////////////////////// gradient ///////////////////////////

    }  //end of for i

}
*/
//Fj add
template <typename T>
void CellBase<T>::CalcScalarGrad(
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

        scalar_array[i] 
             /*= static_cast<kvs::Real32> ( ( m_interpolation_functions_array[i][0] ) * m_scalars_array[i][0] )
                                      + ( ( m_interpolation_functions_array[i][1] ) * m_scalars_array[i][1] )
                                      + ( ( m_interpolation_functions_array[i][2] ) * m_scalars_array[i][2] )
                                      + ( ( m_interpolation_functions_array[i][3] ) * m_scalars_array[i][3] )
                                      + ( ( m_interpolation_functions_array[i][4] ) * m_scalars_array[i][4] )
                                      + ( ( m_interpolation_functions_array[i][5] ) * m_scalars_array[i][5] )
                                      + ( ( m_interpolation_functions_array[i][6] ) * m_scalars_array[i][6] )
                                      + ( ( m_interpolation_functions_array[i][7] ) * m_scalars_array[i][7] );*/
             = static_cast<kvs::Real32> ( ( m_interpolation_functions_array[0][i] ) * m_scalars_array[0][i] )
                                      + ( ( m_interpolation_functions_array[1][i] ) * m_scalars_array[1][i] )
                                      + ( ( m_interpolation_functions_array[2][i] ) * m_scalars_array[2][i] )
                                      + ( ( m_interpolation_functions_array[3][i] ) * m_scalars_array[3][i] )
                                      + ( ( m_interpolation_functions_array[4][i] ) * m_scalars_array[4][i] )
                                      + ( ( m_interpolation_functions_array[5][i] ) * m_scalars_array[5][i] )
                                      + ( ( m_interpolation_functions_array[6][i] ) * m_scalars_array[6][i] )
                                      + ( ( m_interpolation_functions_array[7][i] ) * m_scalars_array[7][i] );
    }    
        //////////////////////////// scalar ////////////////////////////

        /////////////////////////// gradient ///////////////////////////
#pragma ivdep
    for( int i = 0; i < loop_cnt; i++ )
    {

        const float dsdx 
            /*= static_cast<float>( m_scalars_array[i][0] * m_differential_functions_array[i][0]  )
                              + ( m_scalars_array[i][1] * m_differential_functions_array[i][1]  )
                              + ( m_scalars_array[i][2] * m_differential_functions_array[i][2]  )
                              + ( m_scalars_array[i][3] * m_differential_functions_array[i][3]  )
                              + ( m_scalars_array[i][4] * m_differential_functions_array[i][4]  )
                              + ( m_scalars_array[i][5] * m_differential_functions_array[i][5]  )
                              + ( m_scalars_array[i][6] * m_differential_functions_array[i][6]  )
                              + ( m_scalars_array[i][7] * m_differential_functions_array[i][7]  );*/
            = static_cast<float>( m_scalars_array[0][i] * m_differential_functions_array[0][i]  )
                              + ( m_scalars_array[1][i] * m_differential_functions_array[1][i]  )
                              + ( m_scalars_array[2][i] * m_differential_functions_array[2][i]  )
                              + ( m_scalars_array[3][i] * m_differential_functions_array[3][i]  )
                              + ( m_scalars_array[4][i] * m_differential_functions_array[4][i]  )
                              + ( m_scalars_array[5][i] * m_differential_functions_array[5][i]  )
                              + ( m_scalars_array[6][i] * m_differential_functions_array[6][i]  )
                              + ( m_scalars_array[7][i] * m_differential_functions_array[7][i]  );

        const float dsdy 
            /*= static_cast<float>( m_scalars_array[i][0] * m_differential_functions_array[i][8]   )
                              + ( m_scalars_array[i][1] * m_differential_functions_array[i][9]   )
                              + ( m_scalars_array[i][2] * m_differential_functions_array[i][10]  )
                              + ( m_scalars_array[i][3] * m_differential_functions_array[i][11]  )
                              + ( m_scalars_array[i][4] * m_differential_functions_array[i][12]  )
                              + ( m_scalars_array[i][5] * m_differential_functions_array[i][13]  )
                              + ( m_scalars_array[i][6] * m_differential_functions_array[i][14]  )
                              + ( m_scalars_array[i][7] * m_differential_functions_array[i][15]  );*/
            = static_cast<float>( m_scalars_array[0][i] * m_differential_functions_array[8][i]  )
                              + ( m_scalars_array[1][i] * m_differential_functions_array[9][i]  )
                              + ( m_scalars_array[2][i] * m_differential_functions_array[10][i]  )
                              + ( m_scalars_array[3][i] * m_differential_functions_array[11][i]  )
                              + ( m_scalars_array[4][i] * m_differential_functions_array[12][i]  )
                              + ( m_scalars_array[5][i] * m_differential_functions_array[13][i]  )
                              + ( m_scalars_array[6][i] * m_differential_functions_array[14][i]  )
                              + ( m_scalars_array[7][i] * m_differential_functions_array[15][i]  );

        const float dsdz 
            /*= static_cast<float>( m_scalars_array[i][0] * m_differential_functions_array[i][16]  )
                              + ( m_scalars_array[i][1] * m_differential_functions_array[i][17]  )
                              + ( m_scalars_array[i][2] * m_differential_functions_array[i][18]  )
                              + ( m_scalars_array[i][3] * m_differential_functions_array[i][19]  )
                              + ( m_scalars_array[i][4] * m_differential_functions_array[i][20]  )
                              + ( m_scalars_array[i][5] * m_differential_functions_array[i][21]  )
                              + ( m_scalars_array[i][6] * m_differential_functions_array[i][22]  )
                              + ( m_scalars_array[i][7] * m_differential_functions_array[i][23]  );*/
            = static_cast<float>( m_scalars_array[0][i] * m_differential_functions_array[16][i]  )
                              + ( m_scalars_array[1][i] * m_differential_functions_array[17][i]  )
                              + ( m_scalars_array[2][i] * m_differential_functions_array[18][i]  )
                              + ( m_scalars_array[3][i] * m_differential_functions_array[19][i]  )
                              + ( m_scalars_array[4][i] * m_differential_functions_array[20][i]  )
                              + ( m_scalars_array[5][i] * m_differential_functions_array[21][i]  )
                              + ( m_scalars_array[6][i] * m_differential_functions_array[22][i]  )
                              + ( m_scalars_array[7][i] * m_differential_functions_array[23][i]  );

        const kvs::Vector3f g( dsdx, dsdy, dsdz );

        ///////////////////////// JacobiMatrix /////////////////////////

        /*const float dXdx = ( m_differential_functions_array[i][0]  * m_vertices_array[i][0].x() )
                         + ( m_differential_functions_array[i][1]  * m_vertices_array[i][1].x() )
                         + ( m_differential_functions_array[i][2]  * m_vertices_array[i][2].x() )
                         + ( m_differential_functions_array[i][3]  * m_vertices_array[i][3].x() )
                         + ( m_differential_functions_array[i][4]  * m_vertices_array[i][4].x() )
                         + ( m_differential_functions_array[i][5]  * m_vertices_array[i][5].x() )
                         + ( m_differential_functions_array[i][6]  * m_vertices_array[i][6].x() )
                         + ( m_differential_functions_array[i][7]  * m_vertices_array[i][7].x() );*/
        const float dXdx = ( m_differential_functions_array[0][i]  * m_vertices_array[0][i].x() )
                         + ( m_differential_functions_array[1][i]  * m_vertices_array[1][i].x() )
                         + ( m_differential_functions_array[2][i]  * m_vertices_array[2][i].x() )
                         + ( m_differential_functions_array[3][i]  * m_vertices_array[3][i].x() )
                         + ( m_differential_functions_array[4][i]  * m_vertices_array[4][i].x() )
                         + ( m_differential_functions_array[5][i]  * m_vertices_array[5][i].x() )
                         + ( m_differential_functions_array[6][i]  * m_vertices_array[6][i].x() )
                         + ( m_differential_functions_array[7][i]  * m_vertices_array[7][i].x() );

        /*const float dYdx = ( m_differential_functions_array[i][0]  * m_vertices_array[i][0].y() )
                         + ( m_differential_functions_array[i][1]  * m_vertices_array[i][1].y() )
                         + ( m_differential_functions_array[i][2]  * m_vertices_array[i][2].y() )
                         + ( m_differential_functions_array[i][3]  * m_vertices_array[i][3].y() )
                         + ( m_differential_functions_array[i][4]  * m_vertices_array[i][4].y() )
                         + ( m_differential_functions_array[i][5]  * m_vertices_array[i][5].y() )
                         + ( m_differential_functions_array[i][6]  * m_vertices_array[i][6].y() )
                         + ( m_differential_functions_array[i][7]  * m_vertices_array[i][7].y() );*/
        const float dYdx = ( m_differential_functions_array[0][i]  * m_vertices_array[0][i].y() )
                         + ( m_differential_functions_array[1][i]  * m_vertices_array[1][i].y() )
                         + ( m_differential_functions_array[2][i]  * m_vertices_array[2][i].y() )
                         + ( m_differential_functions_array[3][i]  * m_vertices_array[3][i].y() )
                         + ( m_differential_functions_array[4][i]  * m_vertices_array[4][i].y() )
                         + ( m_differential_functions_array[5][i]  * m_vertices_array[5][i].y() )
                         + ( m_differential_functions_array[6][i]  * m_vertices_array[6][i].y() )
                         + ( m_differential_functions_array[7][i]  * m_vertices_array[7][i].y() );

        /*const float dZdx = ( m_differential_functions_array[i][0]  * m_vertices_array[i][0].z() )
                         + ( m_differential_functions_array[i][1]  * m_vertices_array[i][1].z() )
                         + ( m_differential_functions_array[i][2]  * m_vertices_array[i][2].z() )
                         + ( m_differential_functions_array[i][3]  * m_vertices_array[i][3].z() )
                         + ( m_differential_functions_array[i][4]  * m_vertices_array[i][4].z() )
                         + ( m_differential_functions_array[i][5]  * m_vertices_array[i][5].z() )
                         + ( m_differential_functions_array[i][6]  * m_vertices_array[i][6].z() )
                         + ( m_differential_functions_array[i][7]  * m_vertices_array[i][7].z() );*/
        const float dZdx = ( m_differential_functions_array[0][i]  * m_vertices_array[0][i].z() )
                         + ( m_differential_functions_array[1][i]  * m_vertices_array[1][i].z() )
                         + ( m_differential_functions_array[2][i]  * m_vertices_array[2][i].z() )
                         + ( m_differential_functions_array[3][i]  * m_vertices_array[3][i].z() )
                         + ( m_differential_functions_array[4][i]  * m_vertices_array[4][i].z() )
                         + ( m_differential_functions_array[5][i]  * m_vertices_array[5][i].z() )
                         + ( m_differential_functions_array[6][i]  * m_vertices_array[6][i].z() )
                         + ( m_differential_functions_array[7][i]  * m_vertices_array[7][i].z() );


        /*const float dXdy = ( m_differential_functions_array[i][8]  * m_vertices_array[i][0].x() )
                         + ( m_differential_functions_array[i][9]  * m_vertices_array[i][1].x() )
                         + ( m_differential_functions_array[i][10] * m_vertices_array[i][2].x() )
                         + ( m_differential_functions_array[i][11] * m_vertices_array[i][3].x() )
                         + ( m_differential_functions_array[i][12] * m_vertices_array[i][4].x() )
                         + ( m_differential_functions_array[i][13] * m_vertices_array[i][5].x() )
                         + ( m_differential_functions_array[i][14] * m_vertices_array[i][6].x() )
                         + ( m_differential_functions_array[i][15] * m_vertices_array[i][7].x() );*/
        const float dXdy = ( m_differential_functions_array[8][i]  * m_vertices_array[0][i].x() )
                         + ( m_differential_functions_array[9][i]  * m_vertices_array[1][i].x() )
                         + ( m_differential_functions_array[10][i] * m_vertices_array[2][i].x() )
                         + ( m_differential_functions_array[11][i] * m_vertices_array[3][i].x() )
                         + ( m_differential_functions_array[12][i] * m_vertices_array[4][i].x() )
                         + ( m_differential_functions_array[13][i] * m_vertices_array[5][i].x() )
                         + ( m_differential_functions_array[14][i] * m_vertices_array[6][i].x() )
                         + ( m_differential_functions_array[15][i] * m_vertices_array[7][i].x() );

        /*const float dYdy = ( m_differential_functions_array[i][8]  * m_vertices_array[i][0].y() )
                         + ( m_differential_functions_array[i][9]  * m_vertices_array[i][1].y() )
                         + ( m_differential_functions_array[i][10] * m_vertices_array[i][2].y() )
                         + ( m_differential_functions_array[i][11] * m_vertices_array[i][3].y() )
                         + ( m_differential_functions_array[i][12] * m_vertices_array[i][4].y() )
                         + ( m_differential_functions_array[i][13] * m_vertices_array[i][5].y() )
                         + ( m_differential_functions_array[i][14] * m_vertices_array[i][6].y() )
                         + ( m_differential_functions_array[i][15] * m_vertices_array[i][7].y() );*/
        const float dYdy = ( m_differential_functions_array[8][i]  * m_vertices_array[0][i].y() )
                         + ( m_differential_functions_array[9][i]  * m_vertices_array[1][i].y() )
                         + ( m_differential_functions_array[10][i] * m_vertices_array[2][i].y() )
                         + ( m_differential_functions_array[11][i] * m_vertices_array[3][i].y() )
                         + ( m_differential_functions_array[12][i] * m_vertices_array[4][i].y() )
                         + ( m_differential_functions_array[13][i] * m_vertices_array[5][i].y() )
                         + ( m_differential_functions_array[14][i] * m_vertices_array[6][i].y() )
                         + ( m_differential_functions_array[15][i] * m_vertices_array[7][i].y() );

        /*const float dZdy = ( m_differential_functions_array[i][8]  * m_vertices_array[i][0].z() )
                         + ( m_differential_functions_array[i][9]  * m_vertices_array[i][1].z() )
                         + ( m_differential_functions_array[i][10] * m_vertices_array[i][2].z() )
                         + ( m_differential_functions_array[i][11] * m_vertices_array[i][3].z() )
                         + ( m_differential_functions_array[i][12] * m_vertices_array[i][4].z() )
                         + ( m_differential_functions_array[i][13] * m_vertices_array[i][5].z() )
                         + ( m_differential_functions_array[i][14] * m_vertices_array[i][6].z() )
                         + ( m_differential_functions_array[i][15] * m_vertices_array[i][7].z() );*/
        const float dZdy = ( m_differential_functions_array[8][i]  * m_vertices_array[0][i].z() )
                         + ( m_differential_functions_array[9][i]  * m_vertices_array[1][i].z() )
                         + ( m_differential_functions_array[10][i] * m_vertices_array[2][i].z() )
                         + ( m_differential_functions_array[11][i] * m_vertices_array[3][i].z() )
                         + ( m_differential_functions_array[12][i] * m_vertices_array[4][i].z() )
                         + ( m_differential_functions_array[13][i] * m_vertices_array[5][i].z() )
                         + ( m_differential_functions_array[14][i] * m_vertices_array[6][i].z() )
                         + ( m_differential_functions_array[15][i] * m_vertices_array[7][i].z() );


        /*const float dXdz = ( m_differential_functions_array[i][16] * m_vertices_array[i][0].x() )
                         + ( m_differential_functions_array[i][17] * m_vertices_array[i][1].x() )
                         + ( m_differential_functions_array[i][18] * m_vertices_array[i][2].x() )
                         + ( m_differential_functions_array[i][19] * m_vertices_array[i][3].x() )
                         + ( m_differential_functions_array[i][20] * m_vertices_array[i][4].x() )
                         + ( m_differential_functions_array[i][21] * m_vertices_array[i][5].x() )
                         + ( m_differential_functions_array[i][22] * m_vertices_array[i][6].x() )
                         + ( m_differential_functions_array[i][23] * m_vertices_array[i][7].x() );*/
        const float dXdz = ( m_differential_functions_array[16][i] * m_vertices_array[0][i].x() )
                         + ( m_differential_functions_array[17][i] * m_vertices_array[1][i].x() )
                         + ( m_differential_functions_array[18][i] * m_vertices_array[2][i].x() )
                         + ( m_differential_functions_array[19][i] * m_vertices_array[3][i].x() )
                         + ( m_differential_functions_array[20][i] * m_vertices_array[4][i].x() )
                         + ( m_differential_functions_array[21][i] * m_vertices_array[5][i].x() )
                         + ( m_differential_functions_array[22][i] * m_vertices_array[6][i].x() )
                         + ( m_differential_functions_array[23][i] * m_vertices_array[7][i].x() );

        /*const float dYdz = ( m_differential_functions_array[i][16] * m_vertices_array[i][0].y() )
                         + ( m_differential_functions_array[i][17] * m_vertices_array[i][1].y() )
                         + ( m_differential_functions_array[i][18] * m_vertices_array[i][2].y() )
                         + ( m_differential_functions_array[i][19] * m_vertices_array[i][3].y() )
                         + ( m_differential_functions_array[i][20] * m_vertices_array[i][4].y() )
                         + ( m_differential_functions_array[i][21] * m_vertices_array[i][5].y() )
                         + ( m_differential_functions_array[i][22] * m_vertices_array[i][6].y() )
                         + ( m_differential_functions_array[i][23] * m_vertices_array[i][7].y() );*/
        const float dYdz = ( m_differential_functions_array[16][i] * m_vertices_array[0][i].y() )
                         + ( m_differential_functions_array[17][i] * m_vertices_array[1][i].y() )
                         + ( m_differential_functions_array[18][i] * m_vertices_array[2][i].y() )
                         + ( m_differential_functions_array[19][i] * m_vertices_array[3][i].y() )
                         + ( m_differential_functions_array[20][i] * m_vertices_array[4][i].y() )
                         + ( m_differential_functions_array[21][i] * m_vertices_array[5][i].y() )
                         + ( m_differential_functions_array[22][i] * m_vertices_array[6][i].y() )
                         + ( m_differential_functions_array[23][i] * m_vertices_array[7][i].y() );

        /*const float dZdz = ( m_differential_functions_array[i][16] * m_vertices_array[i][0].z() )
                         + ( m_differential_functions_array[i][17] * m_vertices_array[i][1].z() )
                         + ( m_differential_functions_array[i][18] * m_vertices_array[i][2].z() )
                         + ( m_differential_functions_array[i][19] * m_vertices_array[i][3].z() )
                         + ( m_differential_functions_array[i][20] * m_vertices_array[i][4].z() )
                         + ( m_differential_functions_array[i][21] * m_vertices_array[i][5].z() )
                         + ( m_differential_functions_array[i][22] * m_vertices_array[i][6].z() )
                         + ( m_differential_functions_array[i][23] * m_vertices_array[i][7].z() );*/
        const float dZdz = ( m_differential_functions_array[16][i] * m_vertices_array[0][i].z() )
                         + ( m_differential_functions_array[17][i] * m_vertices_array[1][i].z() )
                         + ( m_differential_functions_array[18][i] * m_vertices_array[2][i].z() )
                         + ( m_differential_functions_array[19][i] * m_vertices_array[3][i].z() )
                         + ( m_differential_functions_array[20][i] * m_vertices_array[4][i].z() )
                         + ( m_differential_functions_array[21][i] * m_vertices_array[5][i].z() )
                         + ( m_differential_functions_array[22][i] * m_vertices_array[6][i].z() )
                         + ( m_differential_functions_array[23][i] * m_vertices_array[7][i].z() );

        ///////////////////////// JacobiMatrix /////////////////////////

        /////////////////////////   inverse   /////////////////////////

        const T det22[9] = {
        dYdy * dZdz - dZdy * dYdz,
        dXdy * dZdz - dZdy * dXdz,
        dXdy * dYdz - dYdy * dXdz,
        dYdx * dZdz - dZdx * dYdz,
        dXdx * dZdz - dZdx * dXdz,
        dXdx * dYdz - dYdx * dXdz,
        dYdx * dZdy - dZdx * dYdy,
        dXdx * dZdy - dZdx * dXdy,
        dXdx * dYdy - dYdx * dXdy, };

        const T det33 = 
            dXdx * (dYdy * dZdz - dZdy * dYdz)
          - dYdx * (dXdy * dZdz - dZdy * dXdz)
          + dZdx * (dXdy * dYdz - dYdy * dXdz);

        float determinant = (float)det33;

        kvs::Matrix33f J;
        J.set( ( dYdy * dZdz - dZdy * dYdz ), ( dYdx * dZdz - dZdx * dYdz ), ( dXdx * dYdz - dYdx * dXdz ),
               ( dXdy * dZdz - dZdy * dXdz ), ( dXdx * dZdz - dZdx * dXdz ), ( dXdx * dZdy - dZdx * dXdy ),
               ( dXdy * dYdz - dYdy * dXdz ), ( dXdx * dYdz - dYdx * dXdz ), ( dXdx * dYdy - dYdx * dXdy ) );

        const T det_inverse = static_cast<T>( 1.0 / det33 );

        J *= det_inverse;
        const kvs::Vector3f G = J * g;

        /////////////////////////   inverse   /////////////////////////
        
        grad_array_x[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.x();
        grad_array_y[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.y();
        grad_array_z[i] =  kvs::Math::IsZero( determinant ) ? 0.0f : G.z();

        /////////////////////////// gradient ///////////////////////////

    }  //end of for i

}
} // end of namespace kvs

#endif // KVS__CELL_BASE_H_INCLUDE
