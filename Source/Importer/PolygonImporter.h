/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__POLYGON_IMPORTER_H_INCLUDE
#define CVT__POLYGON_IMPORTER_H_INCLUDE

#include <string>

#include "Core/FileFormat/FileFormatBase.h"
#include "Core/Visualization/Importer/ImporterBase.h"
#include "FileFormat/STL/Stl.h"

#include "kvs/Module"
#include "kvs/PolygonObject"

namespace cvt
{

/**
 * A Polygon data importer.
 */
class PolygonImporter : public kvs::ImporterBase, public kvs::PolygonObject
{
    kvsModule( cvt::PolygonImporter, Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( kvs::PolygonObject );

public:
    /**
     * Construct a polygon data importer and preload data from a file.
     *
     * \param [in] file_format A file format or a reader.
     */
    PolygonImporter( const kvs::FileFormatBase* file_format ) { exec( file_format ); }
    virtual ~PolygonImporter() {}

    kvs::ObjectBase* exec( const kvs::FileFormatBase* file_format );

private:
    void import( const cvt::Stl& stl );
};
} // namespace cvt

#endif // CVT__POLYGON_IMPORTER_H_INCLUDE