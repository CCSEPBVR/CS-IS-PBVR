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
#ifndef CVT_VTK_IMPORTER_H_INCLUDE
#define CVT_VTK_IMPORTER_H_INCLUDE

#include "kvs/ImporterBase"
#include "kvs/Message"

#include "CvtTypeTraits.h"
#include "FileFormat/VtkFileFormat.h"
#include "VtkImport.h"

namespace cvt
{

/**
 * A data importer from a VTK data.
 */
template <typename VtkFileFormat, typename KvsObjectType_ = typename cvt::ConvertibleTypeTraits<
                                      typename VtkFileFormat::VtkDataType>::DestinationType>
class VtkImporter : public kvs::ImporterBase, public KvsObjectType_
{
public:
    using KvsObjectType = KvsObjectType_;

public:
    /**
     * Construct a data importer and convert from a VTK data.
     *
     * \param [in] file_format A file format or a reader.
     */
    VtkImporter( VtkFileFormat* file_format ): KvsObjectType() { this->exec( file_format ); }
    virtual ~VtkImporter() {}

public:
    kvs::ObjectBase* exec( const kvs::FileFormatBase* file_format )
    {
        kvs::ImporterBase::setSuccess( false );

        try
        {
            cvt::Import<
                KvsObjectType*,
                vtkSmartPointer<typename cvt::VtkFileFormatTraits<VtkFileFormat>::VtkDataType>>(
                dynamic_cast<KvsObjectType*>( this ),
                dynamic_cast<const VtkFileFormat*>( file_format )->get() );

            kvs::ImporterBase::setSuccess( true );
            return this;
        }
        catch ( std::exception& e )
        {
            kvsMessageError( e.what() );
            return nullptr;
        }
    }
};
} // namespace cvt

#endif // CVT_VTK_IMPORTER_H_INCLUDE