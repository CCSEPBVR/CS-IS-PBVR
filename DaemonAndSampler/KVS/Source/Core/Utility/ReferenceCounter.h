/****************************************************************************/
/**
 *  @file ReferenceCounter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ReferenceCounter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__REFERENCE_COUNTER_H_INCLUDE
#define KVS__REFERENCE_COUNTER_H_INCLUDE

#include <cstddef>
#include <kvs/ClassName>
#if defined ( KVS_ENABLE_THREAD_SAFE )
#include <kvs/Mutex>
#endif


namespace kvs
{

/*==========================================================================*/
/**
 *  Reference counter class.
 */
/*==========================================================================*/
class ReferenceCounter
{
    kvsClassName_without_virtual( kvs::ReferenceCounter );

private:

#if defined ( KVS_ENABLE_THREAD_SAFE )
    mutable kvs::Mutex m_key;   ///< lock key
#endif
    size_t             m_value; ///< counter

public:

    explicit ReferenceCounter( size_t value = 0 );

    ~ReferenceCounter( void );

public:

    void setValue( size_t value );

    const size_t value( void ) const;

#if defined ( KVS_ENABLE_THREAD_SAFE )
    kvs::Mutex& key( void ) const;
#endif

public:

    void increment( void );

    void decrement( void );
};

} // end of namespace kvs

#endif // KVS__REFERENCE_COUNTER_H_INCLUDE
