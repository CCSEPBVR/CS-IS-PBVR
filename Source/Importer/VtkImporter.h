/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT_VTK_IMPORTER_H_INCLUDE
#define CVT_VTK_IMPORTER_H_INCLUDE

#include "kvs/ImporterBase"

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
     * \param [in] mode A reader mode.
     */
    VtkImporter( VtkFileFormat* file_format, int mode = 0 ): KvsObjectType(), m_mode( mode )
    {
        this->exec( file_format );
    }
    virtual ~VtkImporter() {}

public:
    kvs::ObjectBase* exec( const kvs::FileFormatBase* file_format )
    {
        kvs::ImporterBase::setSuccess( false );
        cvt::Import<KvsObjectType*,
                    vtkSmartPointer<typename cvt::VtkFileFormatTraits<VtkFileFormat>::VtkDataType>>(
            dynamic_cast<KvsObjectType*>( this ),
            dynamic_cast<const VtkFileFormat*>( file_format )->get(), m_mode );
        kvs::ImporterBase::setSuccess( true );

        return this;
    }

private:
    int m_mode;
};
} // namespace cvt

#endif // CVT_VTK_IMPORTER_H_INCLUDE