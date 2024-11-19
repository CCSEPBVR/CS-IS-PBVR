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
#ifndef VIS_MODULE__FILTER_BASE_H_INCLUDE
#define VIS_MODULE__FILTER_BASE_H_INCLUDE

#include <vismodule/ObjectBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Filter base class.
 */
/*==========================================================================*/
class FilterBase
{
    visModuleClassName( vismodule::FilterBase );

    visModuleBase;

protected:

    bool m_is_success; ///< check flag for the filter process

public:

    FilterBase( void );

    virtual ~FilterBase( void );

public:

    virtual vismodule::ObjectBase* exec( const vismodule::ObjectBase* object ) = 0;

public:

    const bool isSuccess( void ) const;

    const bool isFailure( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__FILTER_BASE_H_INCLUDE
