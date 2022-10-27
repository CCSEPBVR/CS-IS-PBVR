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

namespace cvt
{

template <typename T>
class PolygonExporter : public kvs::ExporterBase, public T
{
    using BaseClass = kvs::ExporterBase;

public:
    PolygonExporter( const kvs::PolygonObject* object ): kvs::ExporterBase(), T( "" )
    {
        this->exec( object );
    }

public:
    kvs::FileFormatBase* exec( const kvs::ObjectBase* object )
    {
        BaseClass::setSuccess( false );

        if ( !object )
        {
            throw std::runtime_error( "Input object is NULL" );
            return nullptr;
        }

        const kvs::PolygonObject* polygon = kvs::PolygonObject::DownCast( object );
        if ( !polygon )
        {
            throw std::runtime_error( "Input object is not polygon object" );
            return nullptr;
        }

        this->set( polygon );
        BaseClass::setSuccess( true );

        return this;
    }
};
} // namespace cvt
#endif // CVT__POLYGON_EXPORTER_H_INCLUDE