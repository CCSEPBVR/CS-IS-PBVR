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
#ifndef CVT__VTK_EXPORTER_H_INCLUDE
#define CVT__VTK_EXPORTER_H_INCLUDE

// clang-format off
#include "kvs/ObjectBase"
#include "kvs/ExporterBase"
// clang-format on
#include <vtkSmartPointer.h>

#include "CvtTypeTraits.h"
#include "FileFormat/VtkFileFormat.h"
#include "VtkExport.h"

namespace cvt
{

/**
 * \private
 */
template <typename VtkFileFormat>
class VtkExporter : public kvs::ExporterBase, public VtkFileFormat
{
    using BaseClass = kvs::ExporterBase;
    using KvsObjectType =
        typename cvt::ConvertibleTypeTraits<typename VtkFileFormat::VtkDataType>::DestinationType;
    using VtkDataType = typename cvt::VtkFileFormatTraits<VtkFileFormat>::VtkDataType;

public:
    VtkExporter( const KvsObjectType* object ): kvs::ExporterBase(), VtkFileFormat()
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

        auto kvs_object = KvsObjectType::DownCast( object );
        if ( !kvs_object )
        {
            throw std::runtime_error( "Input object is not a convertible object" );
            return nullptr;
        }

        vtkSmartPointer<VtkDataType> vtk_data;
        cvt::Export<std::reference_wrapper<vtkSmartPointer<VtkDataType>>, const KvsObjectType*>(
            std::ref( vtk_data ), kvs_object );
        VtkFileFormat::set( vtk_data );

        BaseClass::setSuccess( true );

        return this;
    }
};
} // namespace cvt
#endif // CVT__VTK_EXPORTER_H_INCLUDE
