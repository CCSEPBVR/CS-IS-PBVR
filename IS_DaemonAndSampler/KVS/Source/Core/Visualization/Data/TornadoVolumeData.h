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
#ifndef KVS__TORNADO_VOLUME_DATA_H_INCLUDE
#define KVS__TORNADO_VOLUME_DATA_H_INCLUDE

#include <kvs/StructuredVolumeObject>
#include <kvs/Vector3>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Tornado data generator class.
 */
/*===========================================================================*/
class TornadoVolumeData : public kvs::StructuredVolumeObject
{
    kvsClassName( kvs::TornadoVolumeData );

public:

    typedef kvs::StructuredVolumeObject SuperClass;

protected:

    int m_time; ///< time value

public:

    TornadoVolumeData( void );

    TornadoVolumeData( const kvs::Vector3ui resolution, const int time = 0 );

    virtual ~TornadoVolumeData( void );

public:

    const int time( void ) const;

    void setTime( const int time );

public:

    SuperClass* exec( void );
};

} // end of namespace kvs

#endif // KVS__TORNADO_VOLUME_DATA_H_INCLUDE
