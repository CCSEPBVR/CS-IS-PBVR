/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__STRUCTURED_VOLUME_OBJECT_EXPORTER_H_INCLUDE
#define CVT__STRUCTURED_VOLUME_OBJECT_EXPORTER_H_INCLUDE
// clang-format off
#include "kvs/ObjectBase"
// clang-format on
#include "kvs/ExporterBase"
#include "kvs/Module"
#include "kvs/StructuredVolumeObject"

#include "FileFormat/KVSML/KvsmlStructuredVolumeObject.h"

namespace cvt
{

/**
 * An exporter for KVS structured volume object.
 */
class StructuredVolumeObjectExporter : public kvs::ExporterBase,
                                       public cvt::KvsmlStructuredVolumeObject
{
    kvsModule( cvt::StructuredVolumeObjectExporter, Exporter );
    kvsModuleBaseClass( kvs::ExporterBase );

public:
    /**
     * Construct an exporter.
     *
     * \param[in] object An KVS object.
     */
    StructuredVolumeObjectExporter( const kvs::StructuredVolumeObject* object )
    {
        this->exec( object );
    }

public:
    kvs::FileFormatBase* exec( const kvs::ObjectBase* object ) override;
};
} // namespace cvt
#endif // CVT__STRUCTURED_VOLUME_OBJECT_EXPORTER_H_INCLUDE
