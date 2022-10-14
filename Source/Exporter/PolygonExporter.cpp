/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "PolygonExporter.h"

#include <stdexcept>

#include "FileFormat/STL/Stl.h"
#include "kvs/ObjectBase"
#include "kvs/PolygonExporter"

cvt::Stl* kvs::PolygonExporter<cvt::Stl>::exec( const kvs::ObjectBase* object )
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