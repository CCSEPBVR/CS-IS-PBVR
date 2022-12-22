/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
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