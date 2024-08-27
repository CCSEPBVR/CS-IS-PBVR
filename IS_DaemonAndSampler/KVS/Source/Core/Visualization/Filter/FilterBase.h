/****************************************************************************/
/**
 *  @file FilterBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FilterBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__FILTER_BASE_H_INCLUDE
#define KVS__FILTER_BASE_H_INCLUDE

#include <kvs/ObjectBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*==========================================================================*/
/**
 *  Filter base class.
 */
/*==========================================================================*/
class FilterBase
{
    kvsClassName( kvs::FilterBase );

    kvsModuleBase;

protected:

    bool m_is_success; ///< check flag for the filter process

public:

    FilterBase( void );

    virtual ~FilterBase( void );

public:

    virtual kvs::ObjectBase* exec( const kvs::ObjectBase* object ) = 0;

public:

    const bool isSuccess( void ) const;

    const bool isFailure( void ) const;
};

} // end of namespace kvs

#endif // KVS__FILTER_BASE_H_INCLUDE
