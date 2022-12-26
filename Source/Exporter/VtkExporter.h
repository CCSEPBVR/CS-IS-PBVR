/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
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
