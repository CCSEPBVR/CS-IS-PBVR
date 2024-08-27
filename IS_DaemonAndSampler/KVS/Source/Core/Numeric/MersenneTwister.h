/****************************************************************************/
/**
 *  @file MersenneTwister.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MersenneTwister.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MERSENNE_TWISTER_H_INCLUDE
#define KVS__MERSENNE_TWISTER_H_INCLUDE

#include <ctime>
#include <climits>

#include <kvs/ClassName>

#ifdef _MSC_VER
#pragma warning (disable : 4146)
#endif


namespace kvs
{

/*==========================================================================*/
/**
 *  MersenneTwister class.
 */
/*==========================================================================*/
class MersenneTwister
{
    kvsClassName_without_virtual( kvs::MersenneTwister );

private:

    enum { N = 624 };
    enum { M = 397 }; // period parameter

private:

    unsigned long  m_state[N]; ///< internal state
    unsigned long* m_next;     ///< next value to get from state
    int            m_left;     ///< number of values left before reload needed

public:

    MersenneTwister( void );

    explicit MersenneTwister( const unsigned long seed );

    explicit MersenneTwister(
        const unsigned long* const seeds,
        const unsigned long        length = N );

public:

    void setSeed( void );

    void setSeed( const unsigned long seed );

    void setSeed(
        const unsigned long* const seeds,
        const unsigned long        length = N );

public:

    const double rand( void );

    const double rand( const double n );

    const double rand53( void );

    const unsigned long randInteger( void );

    const unsigned long randInteger( const unsigned long n );

public:

    const double operator ()( void );

private:

    void initialize( const unsigned long seed );

    void reload( void );

    const unsigned long high_bit( const unsigned long u ) const;

    const unsigned long low_bit( const unsigned long u ) const;

    const unsigned long low_bits( const unsigned long u ) const;

    const unsigned long mix_bits( const unsigned long u, const unsigned long v ) const;

    const unsigned long twist(
        const unsigned long m,
        const unsigned long s0,
        const unsigned long s1 ) const;

    static const unsigned long hash( std::time_t t, std::clock_t c );
};

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline const double MersenneTwister::rand( void )
{
    return( double ( this->randInteger() ) * ( 1.0 / 4294967295.0 ) );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param n TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const double MersenneTwister::rand( const double n )
{
    return( this->rand() * n );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline const double MersenneTwister::rand53( void )
{
    // Generate a uniform random number (53 bits precision)
    // Based on code by Isaku Wasa.
    // http://www001.upp.so-net.ne.jp/isaku/zmtrand.c.html
    const unsigned long a = this->randInteger() >> 5;
    const unsigned long b = this->randInteger() >> 6;
    return( ( a * 67108864.0 + b ) * ( 1.0 / 9007199254740992.0 ) );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::randInteger( void )
{
    // Pull a 32-bit integer from the generator state
    // Every other access function simply transforms
    // the numbers extracted here

    if ( m_left == 0 ) { this->reload(); }
    --m_left;

    register unsigned long s1;
    s1  = *m_next++;
    s1 ^= ( s1 >> 11 );
    s1 ^= ( s1 <<  7 ) & 0x9d2c5680UL;
    s1 ^= ( s1 << 15 ) & 0xefc60000UL;

    return( s1 ^ ( s1 >> 18 ) );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param n TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::randInteger( const unsigned long n )
{
    // Find which bits are used in n
    // Optimized by Magnus Jonsson (magnus@smartelectronix.com)
    unsigned long used = n;
    used |= used >> 1;
    used |= used >> 2;
    used |= used >> 4;
    used |= used >> 8;
    used |= used >> 16;

    // Draw numbers until one is found in [0,n]
    unsigned long i;
    do
    {
        i = this->randInteger() & used;  // toss unused bits to shorten search
    }
    while ( i > n );

    return( i );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline const double MersenneTwister::operator ()( void )
{
    return( this->rand() );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @return TODO
 */
/*==========================================================================*/
inline void MersenneTwister::reload( void )
{
    // Generate N new values in state
    // Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
    register unsigned long* p = m_state;
    register int            i;

    for ( i = N - M; i--; ++p )
    {
        *p = twist( p[M], p[0], p[1] );
    }

    for ( i = M; --i; ++p )
    {
        *p = twist( p[M - N], p[0], p[1] );
    }
    *p = twist( p[M - N], p[0], m_state[0] );

    m_left = N, m_next = m_state;
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param u TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::high_bit( const unsigned long u ) const
{
    return( u & 0x80000000UL );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param u TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::low_bit( const unsigned long u ) const
{
    return( u & 0x00000001UL );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param u TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::low_bits( const unsigned long u ) const
{
    return( u & 0x7fffffffUL );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param u TODO
 *  @param v TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::mix_bits( const unsigned long u, const unsigned long v ) const
{
    return( high_bit( u ) | low_bits( v ) );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param m TODO
 *  @param s0 TODO
 *  @param s1 TODO
 *  @return TODO
 */
/*==========================================================================*/
inline const unsigned long MersenneTwister::twist(
    const unsigned long m,
    const unsigned long s0,
    const unsigned long s1 ) const
{
    return( m ^ ( mix_bits( s0, s1 ) >> 1 ) ^ ( -low_bit( s1 ) & 0x9908b0dfUL ) );
}

} // end of namespace kvs

#endif // KVS__MERSENNE_TWISTER_H_INCLUDE
