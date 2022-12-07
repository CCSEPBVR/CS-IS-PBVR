/*****************************************************************************/
/**
 *  @file   PointExporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointExporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__POINT_EXPORTER_H_INCLUDE
#define KVS__POINT_EXPORTER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/PointObject>
#include <kvs/KVSMLObjectPoint>
#include "ExporterBase.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  Point exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class PointExporter : public kvs::ExporterBase<FileFormatType>
{
    kvsClassName( kvs::PointExporter );

public:

    PointExporter( const kvs::PointObject* object );

    FileFormatType* exec( const kvs::ObjectBase* object );
};

template <typename FileFormatType>
PointExporter<FileFormatType>::PointExporter( const kvs::PointObject* object )
{
    this->exec( object );
}

template <typename FileFormatType>
FileFormatType* PointExporter<FileFormatType>::exec( const kvs::ObjectBase* object )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Point exporter class as KVSMLObjectPoint format.
 */
/*===========================================================================*/
template <>
class PointExporter<kvs::KVSMLObjectPoint> : public kvs::ExporterBase<kvs::KVSMLObjectPoint>
{
public:

    PointExporter( const kvs::PointObject* object );

    kvs::KVSMLObjectPoint* exec( const kvs::ObjectBase* object );
};

} // end of namespace kvs

#endif // KVS__POINT_EXPORTER_H_INCLUDE
