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
