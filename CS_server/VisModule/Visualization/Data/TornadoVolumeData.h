/*****************************************************************************/
/**
 *  @file   TornadoVolumeData.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TornadoVolumeData.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__TORNADO_VOLUME_DATA_H_INCLUDE
#define VIS_MODULE__TORNADO_VOLUME_DATA_H_INCLUDE

#include <vismodule/StructuredVolumeObject>
#include <vismodule/Vector3>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Tornado data generator class.
 */
/*===========================================================================*/
class TornadoVolumeData : public vismodule::StructuredVolumeObject
{
    visModuleClassName( vismodule::TornadoVolumeData );

public:

    typedef vismodule::StructuredVolumeObject SuperClass;

protected:

    int m_time; ///< time value

public:

    TornadoVolumeData( void );

    TornadoVolumeData( const vismodule::Vector3ui resolution, const int time = 0 );

    virtual ~TornadoVolumeData( void );

public:

    const int time( void ) const;

    void setTime( const int time );

public:

    SuperClass* exec( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TORNADO_VOLUME_DATA_H_INCLUDE
