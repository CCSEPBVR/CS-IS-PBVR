/*****************************************************************************/
/**
 *  @file   LineExporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineExporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__LINE_EXPORTER_H_INCLUDE
#define VIS_MODULE__LINE_EXPORTER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/LineObject>
#include <vismodule/KVSMLObjectLine>
#include "ExporterBase.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  Line exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class LineExporter : public vismodule::ExporterBase<FileFormatType>
{
    visModuleClassName( vismodule::LineExporter );

public:

    LineExporter( const vismodule::LineObject& object );

    FileFormatType* exec( const vismodule::ObjectBase& object );
};

template <typename FileFormatType>
LineExporter<FileFormatType>::LineExporter( const vismodule::LineObject& object )
{
    this->exec( object );
}

template <typename FileFormatType>
FileFormatType* LineExporter<FileFormatType>::exec( const vismodule::ObjectBase& base )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Line exporter class as KVSMLObjectLine format.
 */
/*===========================================================================*/
template <>
class LineExporter<vismodule::KVSMLObjectLine> : public vismodule::ExporterBase<vismodule::KVSMLObjectLine>
{
public:

    LineExporter( const vismodule::LineObject& object );

    vismodule::KVSMLObjectLine* exec( const vismodule::ObjectBase& object );
};

} // end of namespace vismodule

#endif // VIS_MODULE__LINE_EXPORTER_H_INCLUDE
