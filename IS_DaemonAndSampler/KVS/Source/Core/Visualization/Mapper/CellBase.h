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
#ifndef KVS__CELL_BASE_H_INCLUDE
#define KVS__CELL_BASE_H_INCLUDE

#include <kvs/DebugNew>
#include <kvs/ClassName>
#include <kvs/Type>
#include <kvs/Vector4>
#include <kvs/Matrix44>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Message>


namespace kvs
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

    const kvs::UnstructuredVolumeObject* m_reference_volume; ///< reference unstructured volume

public:

    CellBase( const kvs::UnstructuredVolumeObject* volume );

    virtual ~CellBase( void );

public:

    virtual const kvs::Real32* interpolationFunctions( const kvs::Vector3f& point ) const = 0;

    virtual const kvs::Real32* differentialFunctions( const kvs::Vector3f& point ) const = 0;

    virtual void bindCell( const kvs::UInt32 index );

    virtual void setGlobalPoint( const kvs::Vector3f& point ) const;

    virtual void setLocalPoint( const kvs::Vector3f& point ) const;

    virtual const kvs::Vector3f transformGlobalToLocal( const kvs::Vector3f& point ) const;

    virtual const kvs::Vector3f transformLocalToGlobal( const kvs::Vector3f& point ) const;

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
    const kvs::UnstructuredVolumeObject* volume ):
    m_nnodes( volume->cellType() ),
    m_global_point( 0, 0, 0 ),
    m_local_point( 0, 0, 0 ),
    m_reference_volume( volume )
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
    const kvs::UnstructuredVolumeObject* volume = m_reference_volume;
    const kvs::UInt32* const connections = volume->connections().pointer();
    const kvs::Real32* const coords = volume->coords().pointer();
    const T* const values = static_cast<const T*>( volume->values().pointer() );

    const size_t nnodes = m_nnodes;
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

} // end of namespace kvs

#endif // KVS__CELL_BASE_H_INCLUDE
