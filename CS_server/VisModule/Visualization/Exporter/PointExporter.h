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
#ifndef VIS_MODULE__POINT_EXPORTER_H_INCLUDE
#define VIS_MODULE__POINT_EXPORTER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectPoint>
#include "ExporterBase.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  Point exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class PointExporter : public vismodule::ExporterBase<FileFormatType>
{
    visModuleClassName( vismodule::PointExporter );

public:

    PointExporter( const vismodule::PointObject* object );

    FileFormatType* exec( const vismodule::ObjectBase* object );
};

template <typename FileFormatType>
PointExporter<FileFormatType>::PointExporter( const vismodule::PointObject* object )
{
    this->exec( object );
}

template <typename FileFormatType>
FileFormatType* PointExporter<FileFormatType>::exec( const vismodule::ObjectBase* object )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Point exporter class as KVSMLObjectPoint format.
 */
/*===========================================================================*/
template <>
class PointExporter<vismodule::KVSMLObjectPoint> : public vismodule::ExporterBase<vismodule::KVSMLObjectPoint>
{
public:

    PointExporter( const vismodule::PointObject* object );

    vismodule::KVSMLObjectPoint* exec( const vismodule::ObjectBase* object );
};

} // end of namespace vismodule

#endif // VIS_MODULE__POINT_EXPORTER_H_INCLUDE
