/****************************************************************************/
/**
 *  @file Xorshift128.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Xorshift128.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Xorshift128.h"
#include <ctime>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructs a new Xorshift128.
 */
/*==========================================================================*/
Xorshift128::Xorshift128( void )
    : m_x( 0 )
    , m_y( 0 )
    , m_z( 0 )
    , m_w( 0 )
{
    const kvs::UInt32 seed = static_cast<kvs::UInt32>( time( 0 ) );

    this->setSeed( seed );
}

/*==========================================================================*/
/**
 *  .
 *
 *  @param seed TODO
 *  @return TODO
 */
/*==========================================================================*/
void Xorshift128::setSeed( kvs::UInt32 seed )
{
    m_x = seed = 1812433253UL * ( seed ^ ( seed >> 30 ) ) + 1;
    m_y = seed = 1812433253UL * ( seed ^ ( seed >> 30 ) ) + 2;
    m_z = seed = 1812433253UL * ( seed ^ ( seed >> 30 ) ) + 3;
    m_w = seed = 1812433253UL * ( seed ^ ( seed >> 30 ) ) + 4;
}

} // end of namespace kvs
