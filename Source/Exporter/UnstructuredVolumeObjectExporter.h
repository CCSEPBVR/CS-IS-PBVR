/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__UNSTRUCTURED_VOLUME_OBJECT_EXPORTER_H_INCLUDE
#define CVT__UNSTRUCTURED_VOLUME_OBJECT_EXPORTER_H_INCLUDE
// clang-format off
#include "kvs/ObjectBase"
// clang-format on
#include "kvs/ExporterBase"
#include "kvs/Module"
#include "kvs/UnstructuredVolumeObject"

#include "FileFormat/KVSML/KvsmlUnstructuredVolumeObject.h"

namespace cvt
{

/**
 * An exporter for KVS unstructured volume object.
 */
class UnstructuredVolumeObjectExporter : public kvs::ExporterBase,
                                         public cvt::KvsmlUnstructuredVolumeObject
{
    kvsModule( cvt::UnstructuredVolumeObjectExporter, Exporter );
    kvsModuleBaseClass( kvs::ExporterBase );

public:
    /**
     * Construct an exporter.
     *
     * \param[in] object An KVS object.
     */
    UnstructuredVolumeObjectExporter( const kvs::UnstructuredVolumeObject* object )
    {
        this->exec( object );
    }

public:
    kvs::FileFormatBase* exec( const kvs::ObjectBase* object ) override;
};
} // namespace cvt
#endif // CVT__UNSTRUCTURED_VOLUME_OBJECT_EXPORTER_H_INCLUDE
