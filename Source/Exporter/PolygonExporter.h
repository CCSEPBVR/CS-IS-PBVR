/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__POLYGON_EXPORTER_H_INCLUDE
#define CVT__POLYGON_EXPORTER_H_INCLUDE

#include "FileFormat/STL/Stl.h"
#include "kvs/ObjectBase"
#include "kvs/PolygonExporter"

namespace kvs
{

template <> class PolygonExporter<cvt::Stl> : public kvs::ExporterBase, public cvt::Stl
{
public:
    PolygonExporter( const kvs::PolygonObject* object ): kvs::ExporterBase(), cvt::Stl( "" )
    {
        this->exec( object );
    }

public:
    cvt::Stl* exec( const kvs::ObjectBase* object );
};

} // namespace kvs
#endif // CVT__POLYGON_EXPORTER_H_INCLUDE