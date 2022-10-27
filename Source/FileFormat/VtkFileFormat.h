/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_FILE_FORMAT_H_INCLUDE
#define CVT__VTK_FILE_FORMAT_H_INCLUDE
#include <algorithm>
#include <iterator>
#include <string>

#include "kvs/File"
#include "kvs/FileFormatBase"
#include "kvs/PolygonObject"
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include "FileFormatDetail.h"

namespace cvt
{

/**
 * A type information about an file format
 */
template <typename FormatTag>
struct VtkFileFormatTraits;

/**
 * A file IO and converter using VTK.
 */
template <typename FormatTag>
class VtkFileFormat : public kvs::FileFormatBase
{
public:
    /**
     * A base class type.
     */
    using BaseClass = kvs::FileFormatBase;
    using Traits = cvt::VtkFileFormatTraits<FormatTag>;

public:
    /**
     * Construct an empty IO object.
     *
     * \param filename A file name.
     */
    VtkFileFormat() noexcept: kvs::FileFormatBase() {}
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     */
    VtkFileFormat( const std::string& filename ): kvs::FileFormatBase() { setFilename( filename ); }
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     */
    VtkFileFormat( std::string&& filename ): kvs::FileFormatBase() { setFilename( filename ); }
    virtual ~VtkFileFormat() {}

public:
    bool read( const std::string& filename )
    {
        BaseClass::setFilename( filename );
        BaseClass::setSuccess( false );

        try
        {
            vtkNew<typename Traits::VtkReaderType> reader;
            reader->SetFileName( filename.c_str() );
            reader->Update();

            vtk_data = reader->GetOutput();

            bool is_success = vtk_data->GetNumberOfCells() > 0;
            BaseClass::setSuccess( is_success );

            return is_success;
        }
        catch ( std::exception& e )
        {
            throw e;
        }
        catch ( ... )
        {
            return false;
        }
    }
    bool write( const std::string& filename )
    {
        BaseClass::setFilename( filename );
        BaseClass::setSuccess( false );

        try
        {
            vtkNew<typename Traits::VtkWriterType> writer;
            writer->SetInputData( vtk_data );
            writer->SetFileName( filename.c_str() );

            bool is_success = writer->Write() == 1;
            BaseClass::setSuccess( is_success );

            return is_success;
        }
        catch ( std::exception& e )
        {
            throw e;
        }
        catch ( ... )
        {
            return false;
        }
    }

public:
    /**
     * Convert and set to a KVS object.
     *
     * \param [out] object a KVS object.
     */
    void get( typename Traits::KvsDataType* object ) { cvt::detail::Convert( object, vtk_data ); }
    /**
     * Convert and set to a KVS object.
     *
     * \param [out] object a KVS object.
     */
    void get( typename Traits::KvsDataType* object ) const
    {
        cvt::detail::Convert( object, vtk_data );
    }
    /**
     * Reserve a writing object.
     *
     * \param [in] object A writing object.
     */
    void set( const typename Traits::KvsDataType* object )
    {
        cvt::detail::Convert<vtkSmartPointer<typename Traits::VtkDataType>&>( vtk_data, object );
    }
    /**
     * Reserve a writing object.
     *
     * \param [in] object A writing object.
     */
    void set( const typename Traits::KvsDataType& object ) { set( object ); }
    /**
     * Reserve a writing object.
     *
     * \param [in] object A writing object.
     */
    void set( typename Traits::KvsDataType&& object ) { set( object ); };

private:
    vtkSmartPointer<typename Traits::VtkDataType> vtk_data;
};
} // namespace cvt

namespace cvt
{

namespace detail
{

inline bool CheckExtensionImpl( const std::string& filename, const char* const file_extensions[],
                                std::size_t n )
{
    const kvs::File file( filename );
    auto extension = file.extension();

    std::transform( extension.cbegin(), extension.cend(), extension.begin(), tolower );
    return std::any_of( file_extensions, file_extensions + n,
                        [&extension]( const char* const e ) { return extension == e; } );
}
} // namespace detail
/**
 * Check if a file extention is for STL.
 *
 * \param filename A file name.
 */
template <typename T>
bool CheckExtension( const std::string& filename );
} // namespace cvt

#endif // CVT__VTK_FILE_FORMAT_H_INCLUDE
