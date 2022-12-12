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
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace cvt
{

/**
 * A file IO using VTK.
 */
template <typename VtkDataType_, typename VtkReaderType, typename VtkWriterType>
class VtkFileFormat : public kvs::FileFormatBase
{
public:
    /**
     * A base class type.
     */
    using BaseClass = kvs::FileFormatBase;
    /**
     * An inner VTK data type.
     */
    using VtkDataType = VtkDataType_;

public:
    /**
     * Construct an empty IO object.
     */
    VtkFileFormat() noexcept: kvs::FileFormatBase() {}
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     */
    VtkFileFormat( const std::string& filename ): kvs::FileFormatBase() { read( filename ); }
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     */
    VtkFileFormat( std::string&& filename ): kvs::FileFormatBase() { read( filename ); }
    /**
     * Construct an IO object.
     *
     * \param [in] vtk_data A VTK data.
     */
    VtkFileFormat( VtkDataType* data ): kvs::FileFormatBase() { vtk_data = data; }
    virtual ~VtkFileFormat() {}

public:
    bool read( const std::string& filename )
    {
        BaseClass::setFilename( filename );
        BaseClass::setSuccess( false );

        try
        {
            vtkNew<VtkReaderType> reader;
            reader->SetFileName( filename.c_str() );
            reader->Update();

            vtk_data = dynamic_cast<VtkDataType*>( reader->GetOutput() );

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
            vtkNew<VtkWriterType> writer;
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
     * Get a VTK data.
     *
     * \return A VTK data.
     */
    vtkSmartPointer<VtkDataType> get() { return vtk_data; }
    /**
     * Get a VTK data.
     *
     * \return A VTK data.
     */
    vtkSmartPointer<VtkDataType> get() const { return vtk_data; }
    /**
     * Set a VTK data.
     *
     * \param [in] object A writing object.
     */
    void set( vtkSmartPointer<VtkDataType> data ) { vtk_data = data; }
    /**
     * Set a VTK data.
     *
     * \param [in] object A writing object.
     */
    void set( VtkDataType* object ) { vtk_data = object; };

private:
    vtkSmartPointer<VtkDataType> vtk_data;
};

/**
 * A type traits for FileFormat.
 */
template <typename FileFormat>
struct VtkFileFormatTraits
{
    /**
     * An inner VTK data type.
     */
    using VtkDataType = typename FileFormat::VtkDataType;
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
} // namespace cvt

#endif // CVT__VTK_FILE_FORMAT_H_INCLUDE
