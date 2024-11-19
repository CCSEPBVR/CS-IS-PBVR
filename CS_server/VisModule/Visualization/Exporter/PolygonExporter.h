/*****************************************************************************/
/**
 *  @file   PolygonExporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonExporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__POLYGON_EXPORTER_H_INCLUDE
#define KVS__POLYGON_EXPORTER_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/PolygonObject>
#include <kvs/KVSMLObjectPolygon>
#include <kvs/Stl>
#include <kvs/Ply>
#include "ExporterBase.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  Polygon exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class PolygonExporter : public kvs::ExporterBase<FileFormatType>
{
    kvsClassName( kvs::PolygonExporter );

public:

    PolygonExporter( const kvs::PolygonObject* object );

    FileFormatType* exec( const kvs::ObjectBase* object );
};

template <typename FileFormatType>
PolygonExporter<FileFormatType>::PolygonExporter( const kvs::PolygonObject* object )
{
    this->exec( object );
}

template <typename FileFormatType>
FileFormatType* PolygonExporter<FileFormatType>::exec( const kvs::ObjectBase* object )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Polygon exporter class as KVSMLObjectPoint format.
 */
/*===========================================================================*/
template <>
class PolygonExporter<kvs::KVSMLObjectPolygon> : public kvs::ExporterBase<kvs::KVSMLObjectPolygon>
{
public:

    PolygonExporter( const kvs::PolygonObject* object );

    kvs::KVSMLObjectPolygon* exec( const kvs::ObjectBase* object );
};

/*===========================================================================*/
/**
 *  Polygon exporter class as STL format.
 */
/*===========================================================================*/
template <>
class PolygonExporter<kvs::Stl> : public kvs::ExporterBase<kvs::Stl>
{
public:

    PolygonExporter( const kvs::PolygonObject* object );

    kvs::Stl* exec( const kvs::ObjectBase* object );
};

/*===========================================================================*/
/**
 *  Polygon exporter class as PLY format.
 */
/*===========================================================================*/
template <>
class PolygonExporter<kvs::Ply> : public kvs::ExporterBase<kvs::Ply>
{
public:

    PolygonExporter( const kvs::PolygonObject* object );

    kvs::Ply* exec( const kvs::ObjectBase* object );
};

} // end of namespace kvs

#endif // KVS__POLYGON_EXPORTER_H_INCLUDE
