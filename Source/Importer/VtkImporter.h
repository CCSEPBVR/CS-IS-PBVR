/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT_VTK_IMPORTER_H_INCLUDE
#define CVT_VTK_IMPORTER_H_INCLUDE

#include "kvs/ImporterBase"

#include "FileFormat/VtkFileFormat.h"
#include "VtkImport.h"

namespace cvt
{

/**
 * A data importer from a VTK data.
 */
template <typename VtkFileFormat>
class VtkImporter : public kvs::ImporterBase,
                    public cvt::VtkFileFormatTraits<VtkFileFormat>::KvsObjectType
{
public:
    /**
     * Construct a data importer and convert from a VTK data.
     *
     * \param [in] file_format A file format or a reader.
     */
    VtkImporter( VtkFileFormat* file_format ):
        cvt::VtkFileFormatTraits<VtkFileFormat>::KvsObjectType()
    {
        this->exec( file_format );
    }
    virtual ~VtkImporter() {}

public:
    kvs::ObjectBase* exec( const kvs::FileFormatBase* file_format )
    {
        kvs::ImporterBase::setSuccess( false );
        cvt::Import(
            dynamic_cast<typename cvt::VtkFileFormatTraits<VtkFileFormat>::KvsObjectType*>( this ),
            dynamic_cast<const VtkFileFormat*>( file_format )->get() );
        kvs::ImporterBase::setSuccess( true );

        return this;
    }
};
} // namespace cvt

#endif // CVT_VTK_IMPORTER_H_INCLUDE