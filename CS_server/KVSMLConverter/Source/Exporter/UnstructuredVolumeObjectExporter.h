/*
 * Copyright (c) 2022 Japan Atomic Energy Agency
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
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
