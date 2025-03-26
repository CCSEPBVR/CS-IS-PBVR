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
#ifndef VIS_MODULE__REFERENCE_COUNTER_H_INCLUDE
#define VIS_MODULE__REFERENCE_COUNTER_H_INCLUDE

#include <cstddef>
#include <vismodule/ClassName>
#if defined ( VIS_MODULE_ENABLE_THREAD_SAFE )
#include <vismodule/Mutex>
#endif


namespace vismodule
{

/*==========================================================================*/
/**
 *  Reference counter class.
 */
/*==========================================================================*/
class ReferenceCounter
{
    visModuleClassName_without_virtual( vismodule::ReferenceCounter );

private:

#if defined ( VIS_MODULE_ENABLE_THREAD_SAFE )
    mutable vismodule::Mutex m_key;   ///< lock key
#endif
    size_t             m_value; ///< counter

public:

    explicit ReferenceCounter( size_t value = 0 );

    ~ReferenceCounter( void );

public:

    void setValue( size_t value );

    const size_t value( void ) const;

#if defined ( VIS_MODULE_ENABLE_THREAD_SAFE )
    vismodule::Mutex& key( void ) const;
#endif

public:

    void increment( void );

    void decrement( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__REFERENCE_COUNTER_H_INCLUDE
