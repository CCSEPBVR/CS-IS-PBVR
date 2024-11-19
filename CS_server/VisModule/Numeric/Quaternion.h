/****************************************************************************/
/**
 *  @file Quaternion.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Quaternion.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__QUATERNION_H_INCLUDE
#define KVS__QUATERNION_H_INCLUDE

#include <iostream>
#include <iomanip>
#include <kvs/ClassName>
#include <kvs/Math>
#include <kvs/Assert>
#include <kvs/Vector3>
#include <kvs/Matrix33>
#include <kvs/Matrix44>


namespace kvs
{

/*==========================================================================*/
/**
*  Quaternion class
*/
/*==========================================================================*/
template <typename T>
class Quaternion
{
    kvsClassName_without_virtual( kvs::Quaternion );

protected:

    /* Quaternion: Q
       Q = w + xi + yj + zk
         x = m_elements[0],
         y = m_elements[1],
         z = m_elements[2],
         w = m_elements[3]
     */
    T m_elements[4];

public:

    Quaternion<T>( void );

    Quaternion<T>( T x, T y, T z, T w );

    Quaternion<T>( const Quaternion<T>& q );

    Quaternion<T>( const Vector3<T>& axis, T angle );

    Quaternion<T>( const Matrix33<T>& mat );

    Quaternion<T>( const Matrix44<T>& mat );

    explicit Quaternion<T>( const T elements[4] );

public:

    void set( T x, T y, T z, T w );

    T& x( void );

    const T& x( void ) const;

    T& y( void );

    const T& y( void ) const;

    T& z( void );

    const T& z( void ) const;

    T& w( void );

    const T& w( void ) const;

public:

    Quaternion<T>& zero( void );

    const Quaternion<T> zero( void ) const;

    Quaternion<T>& identity( void );

    const Quaternion<T> identity( void ) const;

    Quaternion<T>& conjunction( void );

    const Quaternion<T> conjunction( void ) const;

    Quaternion<T>& normalize( void );

    const Quaternion<T> normalize( void ) const;

    Quaternion<T>& inverse( void );

    const Quaternion<T> inverse( void ) const;

    Quaternion<T>& log( void );

    const Quaternion<T> log( void ) const;

    Quaternion<T>& exp( void );

    const Quaternion<T> exp( void ) const;

    const double dot( const Quaternion<T>& q ) const;

    const double length( void ) const;

    const double length2( void ) const;

public:

    T& operator [] ( size_t index );

    const T operator [] ( size_t index ) const;

    Quaternion<T>& operator = ( const Quaternion<T>& q );

    Quaternion<T>& operator += ( const Quaternion<T>& q );

    Quaternion<T>& operator -= ( const Quaternion<T>& q );

    Quaternion<T>& operator *= ( T a );

    Quaternion<T>& operator /= ( T a );

    Quaternion<T>& operator *= ( const Quaternion<T>& q );

public:

    friend bool operator == ( const Quaternion<T>& a, const Quaternion<T>& b )
    {
        return( kvs::Math::Equal( a[0], b[0] ) &&
                kvs::Math::Equal( a[1], b[1] ) &&
                kvs::Math::Equal( a[2], b[2] ) &&
                kvs::Math::Equal( a[3], b[3] ) );
    }

    friend bool operator != ( const Quaternion<T>& a, const Quaternion<T>& b )
    {
        return( !kvs::Math::Equal( a[0], b[0] ) ||
                !kvs::Math::Equal( a[1], b[1] ) ||
                !kvs::Math::Equal( a[2], b[2] ) ||
                !kvs::Math::Equal( a[3], b[3] ) );
    }

    friend Quaternion<T> operator - ( const Quaternion<T>& a )
    {
        return( Quaternion<T>( -a[0], -a[1], -a[2], -a[3] ) );
    }

    friend Quaternion<T> operator + ( const Quaternion<T>& a, const Quaternion<T>& b )
    {
        Quaternion<T> ret( a ); ret += b;
        return( ret );
    }

    friend Quaternion<T> operator - ( const Quaternion<T>& a, const Quaternion<T>& b )
    {
        Quaternion<T> ret( a ); ret -= b;
        return( ret );
    }

    friend Quaternion<T> operator * ( const Quaternion<T>& q, T s )
    {
        return( Quaternion<T>( q[0] * s, q[1] * s, q[2] * s, q[3] * s ) );
    }

    friend Quaternion<T> operator * ( T s, const Quaternion<T>& q )
    {
        return( Quaternion<T>( s * q[0], s * q[1], s * q[2], s * q[3] ) );
    }

    friend Quaternion<T> operator / ( const Quaternion<T>& q, T s )
    {
        KVS_ASSERT( !kvs::Math::IsZero(s) );
        return( Quaternion<T>( q[0] / s, q[1] / s, q[2] / s, q[3] / s ) );
    }

    friend Quaternion<T> operator * ( const Quaternion<T>& a, const Quaternion<T>& b )
    {
        Quaternion<T> ret;
        ret.x() = a.w() * b.x() + a.x() * b.w() + a.y() * b.z() - a.z() * b.y();
        ret.y() = a.w() * b.y() - a.x() * b.z() + a.y() * b.w() + a.z() * b.x();
        ret.z() = a.w() * b.z() + a.x() * b.y() - a.y() * b.x() + a.z() * b.w();
        ret.w() = a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z();

        return( ret );
    }

public:

    kvs::Matrix33<T> toMatrix( void ) const;

    void toMatrix( kvs::Matrix33<T>& m ) const;

    void toMatrix( kvs::Matrix44<T>& m ) const;

    void toMatrix( T m[16] ) const;

public:

    friend std::ostream& operator << ( std::ostream& os, const Quaternion<T>& q )
    {
        const size_t width     = 8;
        const size_t precision = 3;

        const std::ios_base::fmtflags original_flags( os.flags() );

        os.setf( std::ios::fixed );
        os.setf( std::ios::showpoint );

        os << std::setw(width) << std::setprecision(precision) << q.w() << " + ";
        os << std::setw(width) << std::setprecision(precision) << q.x() << " i + ";
        os << std::setw(width) << std::setprecision(precision) << q.y() << " j + ";
        os << std::setw(width) << std::setprecision(precision) << q.z() << " k";

        os.flags( original_flags );

        return( os );
    }

public:

    Vector3<T> axis( void );

    T angle( void );

public:

    static Vector3<T> rotate( const Vector3<T>& pos, const Vector3<T>& axis, T rad );

    static Vector3<T> rotate( const Vector3<T>& pos, const Quaternion<T>& q );

    static Quaternion<T> rotationQuaternion( Vector3<T> v0, Vector3<T> v1 );

public:

    static Quaternion<T> linearInterpolation(
        const Quaternion<T>& q1,
        const Quaternion<T>& q2,
        double t,
        bool for_rotation );

    static Quaternion<T> sphericalLinearInterpolation(
        const Quaternion<T>& q1,
        const Quaternion<T>& q2,
        double t,
        bool invert,
        bool for_rotation );

    static Quaternion<T> sphericalCubicInterpolation(
        const Quaternion<T>& q1,
        const Quaternion<T>& q2,
        const Quaternion<T>& a,
        const Quaternion<T>& b,
        double t,
        bool for_rotation );

    static Quaternion<T> splineInterpolation(
        const Quaternion<T>& q1,
        const Quaternion<T>& q2,
        const Quaternion<T>& q3,
        const Quaternion<T>& q4,
        double t,
        bool for_rotation );

    static Quaternion<T> spline(
        const Quaternion<T>& qnm1,
        const Quaternion<T>& qn,
        const Quaternion<T>& qnp1 );
};

template<typename T>
Quaternion<T>::Quaternion( void )
{
    m_elements[0] = T(0);
    m_elements[1] = T(0);
    m_elements[2] = T(0);
    m_elements[3] = T(0);
}

template<typename T>
Quaternion<T>::Quaternion( T x, T y, T z, T w )
{
    m_elements[0] = x;
    m_elements[1] = y;
    m_elements[2] = z;
    m_elements[3] = w;
}

template<typename T>
Quaternion<T>::Quaternion( const Quaternion<T>& other )
{
    m_elements[0] = other.m_elements[0];
    m_elements[1] = other.m_elements[1];
    m_elements[2] = other.m_elements[2];
    m_elements[3] = other.m_elements[3];
}

template<typename T>
Quaternion<T>::Quaternion( const kvs::Vector3<T>& axis, T angle )
{
    T s = static_cast<T>( std::sin( angle * 0.5 ) );
    T w = static_cast<T>( std::cos( angle * 0.5 ) );

    kvs::Vector3<T> n( axis );
    n.normalize();

    kvs::Vector3<T> v( s * n );

    m_elements[0] = v.x();
    m_elements[1] = v.y();
    m_elements[2] = v.z();
    m_elements[3] = w;
}

template<typename T>
Quaternion<T>::Quaternion( const kvs::Matrix33<T>& m )
{
    double trace = double( m.trace() + 1.0 );

    T x, y, z, w;
    if( trace >= 1.0 )
    {
        double sqrt_trace = std::sqrt( trace );

        x = T( ( m[1][2] - m[2][1] ) * 0.5 / sqrt_trace );
        y = T( ( m[2][0] - m[0][2] ) * 0.5 / sqrt_trace );
        z = T( ( m[0][1] - m[1][0] ) * 0.5 / sqrt_trace );
        w = T( sqrt_trace * 0.5 );
    }
    else
    {
        if( m[0][0] > m[1][1] && m[0][0] > m[2][2] )
        {
            x = T( std::sqrt( double( m[0][0] - m[1][1] - m[2][2] ) + 1.0 ) * 0.5 );
            y = T( ( m[0][1] + m[1][0] ) * 0.25 / x );
            z = T( ( m[0][2] + m[2][0] ) * 0.25 / x );
            w = T( ( m[1][2] + m[2][1] ) * 0.25 / x );
        }
        else if( m[1][1] > m[2][2] )
        {
            y = T( std::sqrt( double( m[1][1] - m[2][2] - m[0][0] ) + 1.0 ) * 0.5 );
            z = T( ( m[1][2] + m[2][1] ) * 0.25 / y );
            x = T( ( m[1][0] + m[0][1] ) * 0.25 / y );
            w = T( ( m[2][0] + m[0][2] ) * 0.25 / y );
        }
        else
        {
            z = T( std::sqrt( double( m[2][2] - m[0][0] - m[1][1] ) + 1.0 ) * 0.5 );
            x = T( ( m[2][0] + m[0][2] ) * 0.25 / z );
            y = T( ( m[2][1] + m[1][2] ) * 0.25 / z );
            w = T( ( m[0][1] + m[1][0] ) * 0.25 / z );
        }
    }

    m_elements[0] = x;
    m_elements[1] = y;
    m_elements[2] = z;
    m_elements[3] = w;
}

template<typename T>
Quaternion<T>::Quaternion( const kvs::Matrix44<T>& m )
{
    double trace = double( m.trace() );

    T x, y, z, w;
    if( trace >= 1.0 )
    {
        double sqrt_trace = std::sqrt( trace );

        x = T( ( m[1][2] - m[2][1] ) * 0.5 / sqrt_trace );
        y = T( ( m[2][0] - m[0][2] ) * 0.5 / sqrt_trace );
        z = T( ( m[0][1] - m[1][0] ) * 0.5 / sqrt_trace );
        w = T( sqrt_trace * 0.5 );
    }
    else
    {
        if( m[0][0] > m[1][1] && m[0][0] > m[2][2] )
        {
            x = T( std::sqrt( double( m[0][0] - m[1][1] - m[2][2] ) + 1.0 ) * 0.5 );
            y = T( ( m[0][1] + m[1][0] ) * 0.25 / x );
            z = T( ( m[0][2] + m[2][0] ) * 0.25 / x );
            w = T( ( m[1][2] + m[2][1] ) * 0.25 / x );
        }
        else if( m[1][1] > m[2][2] )
        {
            y = T( std::sqrt( double( m[1][1] - m[2][2] - m[0][0] ) + 1.0 ) * 0.5 );
            z = T( ( m[1][2] + m[2][1] ) * 0.25 / y );
            x = T( ( m[1][0] + m[0][1] ) * 0.25 / y );
            w = T( ( m[2][0] + m[0][2] ) * 0.25 / y );
        }
        else
        {
            z = T( std::sqrt( double( m[2][2] - m[0][0] - m[1][1] ) + 1.0 ) * 0.5 );
            x = T( ( m[2][0] + m[0][2] ) * 0.25 / z );
            y = T( ( m[2][1] + m[1][2] ) * 0.25 / z );
            w = T( ( m[0][1] + m[1][0] ) * 0.25 / z );
        }
    }
}

template<typename T>
Quaternion<T>::Quaternion( const T elements[4] )
{
    m_elements[0] = elements[0];
    m_elements[1] = elements[1];
    m_elements[2] = elements[2];
    m_elements[3] = elements[3];
}

template<typename T>
void Quaternion<T>::set( T x, T y, T z, T w )
{
    m_elements[0] = x;
    m_elements[1] = y;
    m_elements[2] = z;
    m_elements[3] = w;
}

template<typename T>
T& Quaternion<T>::x( void )
{
    return( m_elements[0] );
}

template<typename T>
const T& Quaternion<T>::x( void ) const
{
    return( m_elements[0] );
}

template<typename T>
T& Quaternion<T>::y( void )
{
    return( m_elements[1] );
}

template<typename T>
const T& Quaternion<T>::y( void ) const
{
    return( m_elements[1] );
}

template<typename T>
T& Quaternion<T>::z( void )
{
    return( m_elements[2] );
}

template<typename T>
const T& Quaternion<T>::z( void ) const
{
    return( m_elements[2] );
}

template<typename T>
T& Quaternion<T>::w( void )
{
    return( m_elements[3] );
}

template<typename T>
const T& Quaternion<T>::w( void ) const
{
    return( m_elements[3] );
}

template<typename T>
Quaternion<T>& Quaternion<T>::zero( void )
{
    m_elements[0] = T(0);
    m_elements[1] = T(0);
    m_elements[2] = T(0);
    m_elements[3] = T(0);

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::zero( void ) const
{
    Quaternion<T> result;
    return( result.zero() );
}

template<typename T>
Quaternion<T>& Quaternion<T>::identity( void )
{
    m_elements[0] = T(0);
    m_elements[1] = T(0);
    m_elements[2] = T(0);
    m_elements[3] = T(1);

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::identity( void ) const
{
    Quaternion<T> result;
    return( result.identity() );
}

template<typename T>
Quaternion<T>& Quaternion<T>::conjunction( void )
{
    m_elements[0] *= T(-1);
    m_elements[1] *= T(-1);
    m_elements[2] *= T(-1);

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::conjunction( void ) const
{
    Quaternion<T> result( *this );
    return( result.conjunction() );
}

template<typename T>
Quaternion<T>& Quaternion<T>::normalize( void )
{
    T n = static_cast<T>( this->length() );
    n = n > T(0) ? T(1) / n : T(0);

    m_elements[0] *= n;
    m_elements[1] *= n;
    m_elements[2] *= n;
    m_elements[3] *= n;

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::normalize( void ) const
{
    Quaternion<T> result( *this );
    return( result.normalize() );
}

template<typename T>
Quaternion<T>& Quaternion<T>::inverse( void )
{
    T n = static_cast<T>( this->length2() );

    if( n > T(0) )
    {
        m_elements[0] /= -n;
        m_elements[1] /= -n;
        m_elements[2] /= -n;
        m_elements[3] /=  n;
    }

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::inverse( void ) const
{
    Quaternion<T> result( *this );
    return( result.inverse() );
}

template<typename T>
Quaternion<T>& Quaternion<T>::log( void )
{
    double theta     = std::acos( double( m_elements[3] ) );
    double sin_theta = std::sin( theta );

    m_elements[3] = T(0);

    if( sin_theta > 0 )
    {
        m_elements[0] = T( theta * m_elements[0] / sin_theta );
        m_elements[1] = T( theta * m_elements[1] / sin_theta );
        m_elements[2] = T( theta * m_elements[2] / sin_theta );
    }
    else
    {
        m_elements[0] = T( 0 );
        m_elements[1] = T( 0 );
        m_elements[2] = T( 0 );
    }

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::log( void ) const
{
    Quaternion<T> result( *this );
    return( result.log() );
}

template<typename T>
Quaternion<T>& Quaternion<T>::exp( void )
{
    double theta2 = 0.0;
    theta2 += m_elements[0] * m_elements[0];
    theta2 += m_elements[1] * m_elements[1];
    theta2 += m_elements[2] * m_elements[2];

    double theta     = std::sqrt( theta2 );
    double cos_theta = std::cos( theta );

    m_elements[3] = T( cos_theta );

    if( theta > 0 )
    {
        double sin_theta = std::sin( theta );
        m_elements[0] = T( sin_theta * m_elements[0] / theta );
        m_elements[1] = T( sin_theta * m_elements[1] / theta );
        m_elements[2] = T( sin_theta * m_elements[2] / theta );
    }
    else
    {
        m_elements[0] = T( 0 );
        m_elements[1] = T( 0 );
        m_elements[2] = T( 0 );
    }

    return( *this );
}

template<typename T>
const Quaternion<T> Quaternion<T>::exp( void ) const
{
    Quaternion<T> result( *this );
    return( result.exp() );
}

template<typename T>
const double Quaternion<T>::dot( const Quaternion<T>& q ) const
{
    double result = 0.0;
    result += this->x() * q.x();
    result += this->y() * q.y();
    result += this->z() * q.z();
    result += this->w() * q.w();

    return( result );
}

template<typename T>
const double Quaternion<T>::length( void ) const
{
    return( std::sqrt( this->length2() ) );
}

template<typename T>
const double Quaternion<T>::length2( void ) const
{
    double result = 0.0;
    result += m_elements[0] * m_elements[0];
    result += m_elements[1] * m_elements[1];
    result += m_elements[2] * m_elements[2];
    result += m_elements[3] * m_elements[3];

    return( result );
}

template<typename T>
T& Quaternion<T>::operator [] ( size_t index )
{
    KVS_ASSERT( index < 4 );

    return( m_elements[index] );
}

template<typename T>
const T Quaternion<T>::operator [] ( size_t index ) const
{
    KVS_ASSERT( index < 4 );

    return( m_elements[index] );
}

template<typename T>
Quaternion<T>& Quaternion<T>::operator = ( const Quaternion<T>& q )
{
    m_elements[0] = q.m_elements[0];
    m_elements[1] = q.m_elements[1];
    m_elements[2] = q.m_elements[2];
    m_elements[3] = q.m_elements[3];

    return( *this );
}

template<typename T>
Quaternion<T>& Quaternion<T>::operator += ( const Quaternion<T>& q )
{
    m_elements[0] += q.m_elements[0];
    m_elements[1] += q.m_elements[1];
    m_elements[2] += q.m_elements[2];
    m_elements[3] += q.m_elements[3];

    return( *this );
}

template<typename T>
Quaternion<T>& Quaternion<T>::operator -= ( const Quaternion<T>& q )
{
    m_elements[0] -= q.m_elements[0];
    m_elements[1] -= q.m_elements[1];
    m_elements[2] -= q.m_elements[2];
    m_elements[3] -= q.m_elements[3];

    return( *this );
}

template<typename T>
Quaternion<T>& Quaternion<T>::operator *= ( T a )
{
    m_elements[0] *= a;
    m_elements[1] *= a;
    m_elements[2] *= a;
    m_elements[3] *= a;
    return( *this );
}

template<typename T>
Quaternion<T>& Quaternion<T>::operator /= ( T a )
{
    KVS_ASSERT( !kvs::Math::IsZero(a) );

    m_elements[0] /= a;
    m_elements[1] /= a;
    m_elements[2] /= a;
    m_elements[3] /= a;

    return( *this );
};

template<typename T>
Quaternion<T>& Quaternion<T>::operator *= ( const Quaternion<T>& q )
{
    T x = this->x();
    T y = this->y();
    T z = this->z();
    T w = this->w();

    this->set( w * q.x() + x * q.w() + y * q.z() - z * q.y(),
               w * q.y() - x * q.z() + y * q.w() + z * q.x(),
               w * q.z() + x * q.y() - y * q.x() + z * q.w(),
               w * q.w() - x * q.x() - y * q.y() - z * q.z() );

    return( *this );
}

template<typename T>
kvs::Matrix33<T> Quaternion<T>::toMatrix( void ) const
{
    T length_2 = static_cast<T>( this->length2() );
    T s = ( length_2 > T(0) ) ? T(2) / length_2 : T(0);

    T xx = this->x() * this->x() * s;
    T yy = this->y() * this->y() * s;
    T zz = this->z() * this->z() * s;
    T xy = this->x() * this->y() * s;
    T xz = this->x() * this->z() * s;
    T yz = this->y() * this->z() * s;
    T wx = this->w() * this->x() * s;
    T wy = this->w() * this->y() * s;
    T wz = this->w() * this->z() * s;

    kvs::Matrix33<T> ret;
    ret[0][0] = T(1) - ( yy + zz );
    ret[1][0] = xy + wz;
    ret[2][0] = xz - wy;

    ret[0][1] = xy - wz;
    ret[1][1] = T(1) - ( xx + zz );
    ret[2][1] = yz + wx;

    ret[0][2] = xz + wy;
    ret[1][2] = yz - wx;
    ret[2][2] = T(1) - ( xx + yy );

    return( ret );
}

template<typename T>
void Quaternion<T>::toMatrix( kvs::Matrix33<T>& m ) const
{
    T length_2 = static_cast<T>( this->length2() );
    T s = ( length_2 > T(0) ) ? T(2) / length_2 : T(0);

    T xx = this->x() * this->x() * s;
    T yy = this->y() * this->y() * s;
    T zz = this->z() * this->z() * s;
    T xy = this->x() * this->y() * s;
    T xz = this->x() * this->z() * s;
    T yz = this->y() * this->z() * s;
    T wx = this->w() * this->x() * s;
    T wy = this->w() * this->y() * s;
    T wz = this->w() * this->z() * s;

    m[0][0] = T(1) - ( yy + zz );
    m[1][0] = xy + wz;
    m[2][0] = xz - wy;

    m[0][1] = xy - wz;
    m[1][1] = T(1) - ( xx + zz );
    m[2][1] = yz + wx;

    m[0][2] = xz + wy;
    m[1][2] = yz - wx;
    m[2][2] = T(1) - ( xx + yy );
}

template<typename T>
void Quaternion<T>::toMatrix( kvs::Matrix44<T>& m ) const
{
    T length_2 = static_cast<T>( this->length2() );
    T s = ( length_2 > T(0) ) ? T(2) / length_2 : T(0);

    T xx = this->x() * this->x() * s;
    T yy = this->y() * this->y() * s;
    T zz = this->z() * this->z() * s;
    T xy = this->x() * this->y() * s;
    T xz = this->x() * this->z() * s;
    T yz = this->y() * this->z() * s;
    T wx = this->w() * this->x() * s;
    T wy = this->w() * this->y() * s;
    T wz = this->w() * this->z() * s;

    m[0][0] = T(1) - ( yy + zz );
    m[1][0] = xy + wz;
    m[2][0] = xz - wy;
    m[3][0] = T(0);

    m[0][1] = xy - wz;
    m[1][1] = T(1) - ( xx + zz );
    m[2][1] = yz + wx;
    m[3][1] = T(0);

    m[0][2] = xz + wy;
    m[1][2] = yz - wx;
    m[2][2] = T(1) - ( xx + yy );
    m[3][2] = T(0);

    m[0][3] = T(0);
    m[1][3] = T(0);
    m[2][3] = T(0);
    m[3][3] = T(1);
}

template<typename T>
void Quaternion<T>::toMatrix( T m[16] ) const
{
    T length_2 = static_cast<T>( this->length2() );
    T s = ( length_2 > 0 ) ? T(2) / length_2 : T(0);

    T xx = this->x() * this->x() * s;
    T yy = this->y() * this->y() * s;
    T zz = this->z() * this->z() * s;
    T xy = this->x() * this->y() * s;
    T xz = this->x() * this->z() * s;
    T yz = this->y() * this->z() * s;
    T wx = this->w() * this->x() * s;
    T wy = this->w() * this->y() * s;
    T wz = this->w() * this->z() * s;

    m[0]  = T(1) - ( yy + zz );
    m[1]  = xy + wz;
    m[2]  = xz - wy;
    m[3]  = T(0);

    m[4]  = xy - wz;
    m[5]  = T(1) - ( xx + zz );
    m[6]  = yz + wx;
    m[7]  = T(0);

    m[8]  = xz + wy;
    m[9]  = yz - wx;
    m[10] = T(1) - ( xx + yy );
    m[11] = T(0);

    m[12] = T(0);
    m[13] = T(0);
    m[14] = T(0);
    m[15] = T(1);
}

template<typename T>
kvs::Vector3<T> Quaternion<T>::axis( void )
{
    T s = T( std::sin( double( this->angle() ) * 0.5 ) );

    kvs::Vector3<T> ret( this->x(), this->y(), this->z() );
    ret /= s;

    return( ret );
}

template<typename T>
T Quaternion<T>::angle( void )
{
    return( T( std::acos( double( this->w() ) ) * 2.0 ) );
}

template<typename T>
kvs::Vector3<T> Quaternion<T>::rotate( const kvs::Vector3<T>& pos, const kvs::Vector3<T>& axis, T rad )
{
    const Quaternion<T> p( pos.x(), pos.y(), pos.z(), T(0) );
    const Quaternion<T> rotate_quat( axis, rad );
    const Quaternion<T> rotate_conj = rotate_quat.conjunction();
    const Quaternion<T> rotate_pos = rotate_conj * p * rotate_quat;

    return( kvs::Vector3<T>( rotate_pos.x(), rotate_pos.y(), rotate_pos.z() ) );
}

template<typename T>
kvs::Vector3<T> Quaternion<T>::rotate( const kvs::Vector3<T>& pos, const kvs::Quaternion<T>& q )
{
    const Quaternion<T> p( pos.x(), pos.y(), pos.z(), T(0) );
    const Quaternion<T> rotate_conj = q.conjunction();
    const Quaternion<T> rotate_pos = rotate_conj * p * q;

    return( kvs::Vector3<T>( rotate_pos.x(), rotate_pos.y(), rotate_pos.z() ) );
}

template<typename T>
Quaternion<T> Quaternion<T>::rotationQuaternion( Vector3<T> v0, Vector3<T> v1 )
{
    Quaternion<T> q;

    v0.normalize();
    v1.normalize();

    kvs::Vector3<T> c = v0.cross( v1 );
    T               d = v0.x() * v1.x() + v0.y() * v1.y() + v0.z() * v1.z();
    double          s = std::sqrt( double( ( 1 + d ) * 2.0 ) );

    q.x() = T( c.x() / s );
    q.y() = T( c.y() / s );
    q.z() = T( c.z() / s );
    q.w() = T( s / 2.0 );

    return( q );
}

template<typename T>
Quaternion<T> Quaternion<T>::linearInterpolation(
    const Quaternion<T>& q1,
    const Quaternion<T>& q2,
    double               t,
    bool                 for_rotation )
{
    Quaternion<T> ret = q1 + T(t) * ( q2 - q1 );

    if ( for_rotation ) { ret.normalize(); }

    return( ret );
}

template<typename T>
Quaternion<T> Quaternion<T>::sphericalLinearInterpolation(
    const Quaternion<T>& q1,
    const Quaternion<T>& q2,
    double               t,
    bool                 invert,
    bool                 for_rotation )
{
    Quaternion<T> tmp1 = q1; tmp1.normalize();
    Quaternion<T> tmp2 = q2; tmp2.normalize();

    double dot = tmp1.dot( tmp2 );

    Quaternion<T> q3;

    // dot = cos( theta )
    // if (dot < 0), q1 and q2 are more than 90 degrees apart,
    // so we can invert one to reduce spining

    if( invert && dot < 0 )
    {
        dot = -dot;
        q3  = -q2;
    }
    else
    {
        q3 = q2;
    }

    if( (  invert && dot < 0.95 ) ||
        ( !invert && dot > -0.95 && dot < 0.95 ) )
    {
        double angle   = std::acos( dot );
        double sina    = std::sin( angle );
        double sinat   = std::sin( angle * t );
        double sinaomt = std::sin( angle * (1-t) );

        return( ( q1 * T( sinaomt ) + q3 * T( sinat ) ) / T( sina ) );
    }
    // if the angle is small, use linear interpolation
    else
    {
        return( linearInterpolation( q1, q3, t, for_rotation ) );
    }
}

template<typename T>
Quaternion<T> Quaternion<T>::sphericalCubicInterpolation(
    const Quaternion<T>& q1,
    const Quaternion<T>& q2,
    const Quaternion<T>& a,
    const Quaternion<T>& b,
    double               t,
    bool                 for_rotation )
{
    Quaternion<T> c = sphericalLinearInterpolation( q1, q2, t, false, for_rotation );
    Quaternion<T> d = sphericalLinearInterpolation(  a,  b, t, false, for_rotation );

    return( sphericalLinearInterpolation( c, d, 2.0 * t * (1-t), false, for_rotation ) );
}

template<typename T>
Quaternion<T> Quaternion<T>::splineInterpolation(
    const Quaternion<T>& q1,
    const Quaternion<T>& q2,
    const Quaternion<T>& q3,
    const Quaternion<T>& q4,
    double               t,
    bool                 for_rotation )
{
    Quaternion<T> a = spline( q1, q2, q3 );
    Quaternion<T> b = spline( q2, q3, q4 );

    return( sphericalCubicInterpolation( q2, q3, a, b, t, for_rotation ) );
}

template<typename T>
Quaternion<T> Quaternion<T>::spline(
    const Quaternion<T>& qnm1,
    const Quaternion<T>& qn,
    const Quaternion<T>& qnp1 )
{
    Quaternion<T> tmpm1 = qnm1; tmpm1.normalize();
    Quaternion<T> tmpp1 = qnp1; tmpp1.normalize();

    Quaternion<T> qni = qn.conjunction(); qni.normalize();

    return( qn * ( ( ( qni * tmpm1 ).log() + ( qni * tmpp1 ).log() ) / -4 ).exp() );
}

} // end of namespace kvs

#endif // KVS__QUATERNION_H_INCLUDE
