/*****************************************************************************/
/**
 *  @file   HydrogenVolumeData.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HydrogenVolumeData.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__HYDROGEN_VOLUME_DATA_H_INCLUDE
#define VIS_MODULE__HYDROGEN_VOLUME_DATA_H_INCLUDE

#include <vismodule/StructuredVolumeObject>
#include <vismodule/Vector3>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Hydrogen data generator class.
 */
/*===========================================================================*/
class HydrogenVolumeData : public vismodule::StructuredVolumeObject
{
    visModuleClassName( vismodule::HydrogenVolumeData );

public:

    typedef vismodule::StructuredVolumeObject SuperClass;

public:

    HydrogenVolumeData( void );

    HydrogenVolumeData( const vismodule::Vector3ui resolution );

    virtual ~HydrogenVolumeData( void );

public:

    SuperClass* exec( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__HYDROGEN_VOLUME_DATA_H_INCLUDE
