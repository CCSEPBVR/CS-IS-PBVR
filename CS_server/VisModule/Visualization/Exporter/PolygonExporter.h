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
#ifndef VIS_MODULE__POLYGON_EXPORTER_H_INCLUDE
#define VIS_MODULE__POLYGON_EXPORTER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/PolygonObject>
#include <vismodule/KVSMLObjectPolygon>
#include <vismodule/Stl>
#include <vismodule/Ply>
#include "ExporterBase.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  Polygon exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class PolygonExporter : public vismodule::ExporterBase<FileFormatType>
{
    visModuleClassName( vismodule::PolygonExporter );

public:

    PolygonExporter( const vismodule::PolygonObject& object );

    FileFormatType* exec( const vismodule::ObjectBase& object );
};

template <typename FileFormatType>
PolygonExporter<FileFormatType>::PolygonExporter( const vismodule::PolygonObject& object )
{
    this->exec( object );
}

template <typename FileFormatType>
FileFormatType* PolygonExporter<FileFormatType>::exec( const vismodule::ObjectBase& object )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Polygon exporter class as KVSMLObjectPoint format.
 */
/*===========================================================================*/
template <>
class PolygonExporter<vismodule::KVSMLObjectPolygon> : public vismodule::ExporterBase<vismodule::KVSMLObjectPolygon>
{
public:

    PolygonExporter( const vismodule::PolygonObject& object );

    vismodule::KVSMLObjectPolygon* exec( const vismodule::ObjectBase& object );
};

/*===========================================================================*/
/**
 *  Polygon exporter class as STL format.
 */
/*===========================================================================*/
template <>
class PolygonExporter<vismodule::Stl> : public vismodule::ExporterBase<vismodule::Stl>
{
public:

    PolygonExporter( const vismodule::PolygonObject& object );

    vismodule::Stl* exec( const vismodule::ObjectBase& object );
};

/*===========================================================================*/
/**
 *  Polygon exporter class as PLY format.
 */
/*===========================================================================*/
template <>
class PolygonExporter<vismodule::Ply> : public vismodule::ExporterBase<vismodule::Ply>
{
public:

    PolygonExporter( const vismodule::PolygonObject& object );

    vismodule::Ply* exec( const vismodule::ObjectBase& object );
};

} // end of namespace vismodule

#endif // VIS_MODULE__POLYGON_EXPORTER_H_INCLUDE
