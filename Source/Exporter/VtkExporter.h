/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__POLYGON_EXPORTER_H_INCLUDE
#define CVT__POLYGON_EXPORTER_H_INCLUDE

// clang-format off
#include "kvs/ObjectBase"
#include "kvs/ExporterBase"
// clang-format on
#include <vtkSmartPointer.h>

#include "FileFormat/VtkFileFormat.h"
#include "VtkExport.h"

namespace cvt
{

template <typename VtkFileFormat>
class VtkExporter : public kvs::ExporterBase, public VtkFileFormat
{
    using BaseClass = kvs::ExporterBase;
    using KvsObjectType = typename cvt::VtkFileFormatTraits<VtkFileFormat>::KvsObjectType;
    using KvsObjectTag = typename cvt::VtkFileFormatTraits<VtkFileFormat>::KvsObjectTag;
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
            throw std::runtime_error( "Input object is not polygon object" );
            return nullptr;
        }

        vtkSmartPointer<VtkDataType> vtk_data;
        cvt::Export<std::reference_wrapper<vtkSmartPointer<VtkDataType>>, const KvsObjectType*,
                    KvsObjectTag>( std::ref( vtk_data ), kvs_object );
        VtkFileFormat::set( vtk_data );

        BaseClass::setSuccess( true );

        return this;
    }
};
} // namespace cvt
#endif // CVT__POLYGON_EXPORTER_H_INCLUDE