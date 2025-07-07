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
#ifndef EXTENDED_FILE_FORMAT__VTK_UNSTRUCTURED_FILE_FORMAT_H_INCLUDE
#define EXTENDED_FILE_FORMAT__VTK_UNSTRUCTURED_FILE_FORMAT_H_INCLUDE
#include <kvs/extendedfileformat/VtkFileFormat>

#include <unordered_set>

#include <vtkExtractCellsByType.h>

namespace kvs
{
namespace ExtendedFileFormat
{
namespace detail
{
template <typename BaseClass>
class CellTypeIterator
{
    using VtkDataPointerType = vtkSmartPointer<typename BaseClass::VtkDataType>;

private:
    using InternalIterator = std::unordered_set<int>::const_iterator;

public:
    CellTypeIterator( InternalIterator itr, VtkDataPointerType vtk_data ) noexcept:
        iterator( itr ), extractor( vtkSmartPointer<vtkExtractCellsByType>::New() )
    {
        data = vtk_data;
        extractor->SetInputData( data );
    }

public:
    BaseClass operator*()
    {
        extractor->RemoveAllCellTypes();
        extractor->AddCellType( *iterator );
        extractor->Update();

        return BaseClass(
            dynamic_cast<typename BaseClass::VtkDataType*>( extractor->GetOutput() ) );
    }
    CellTypeIterator<BaseClass> operator++()
    {
        ++iterator;
        return *this;
    }
    CellTypeIterator<BaseClass> operator++( int )
    {
        auto old = *this;
        ++iterator;
        return old;
    }
    bool operator==( const CellTypeIterator<BaseClass>& other )
    {
        return iterator == other.iterator;
    }
    bool operator==( const CellTypeIterator<BaseClass>& other ) const
    {
        return iterator == other.iterator;
    }
    bool operator!=( const CellTypeIterator<BaseClass>& other )
    {
        return iterator != other.iterator;
    }
    bool operator!=( const CellTypeIterator<BaseClass>& other ) const
    {
        return iterator != other.iterator;
    }

private:
    InternalIterator iterator;
    VtkDataPointerType data;
    vtkSmartPointer<vtkExtractCellsByType> extractor;
};

template <typename BaseClass>
class CellTypeContainer
{
public:
    using VtkDataPointerType = vtkSmartPointer<typename BaseClass::VtkDataType>;

public:
    CellTypeContainer( const std::unordered_set<int>& types, VtkDataPointerType d ):
        cell_types( types ), data( d )
    {
    }

public:
    kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass> begin()
    {
        return kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass>( cell_types.cbegin(), data );
    }
    kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass> begin() const
    {
        return kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass>( cell_types.begin(), data );
    }
    kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass> end()
    {
        return kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass>( cell_types.cend(), data );
    }
    kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass> end() const
    {
        return kvs::ExtendedFileFormat::detail::CellTypeIterator<BaseClass>( cell_types.end(), data );
    }

private:
    const std::unordered_set<int>& cell_types;
    VtkDataPointerType data;
};
} // namespace detail
} // namespace ExtendedFileFormat
} // namespace kvs

namespace kvs
{
namespace ExtendedFileFormat
{

/**
 * A file IO for vtkUnstructuredGrid.
 */
template <typename VtkDataType, typename VtkReaderType, typename VtkWriterType>
class VtkUnstructuredFileFormat
    : public kvs::ExtendedFileFormat::VtkFileFormat<VtkDataType, VtkReaderType, VtkWriterType>
{
public:
    /**
     * A base class type.
     */
    using BaseClass = kvs::ExtendedFileFormat::VtkFileFormat<VtkDataType, VtkReaderType, VtkWriterType>;

public:
    /**
     * Construct an empty IO object.
     *
     * This member is for exporters.
     */
    VtkUnstructuredFileFormat() noexcept: BaseClass() {}
    /**
     * Construct an IO object and read a file.
     *
     * \param[in] filename A file name.
     */
    VtkUnstructuredFileFormat( const std::string& filename ): BaseClass( filename )
    {
        initializeCellTypes();
    }
    /**
     * Construct an IO object and read a file.
     *
     * \param[in] filename A file name.
     * \param[in] reader_option_setter A function to configure a VTK reader additionally.
     */
    VtkUnstructuredFileFormat( const std::string& filename,
                               typename BaseClass::ReaderOptionSetter reader_option_setter ):
        BaseClass( filename, reader_option_setter )
    {
        initializeCellTypes();
    }
    /**
     * Construct an IO object.
     *
     * This takes the owner of the object.
     * So DO NOT free the object manually.
     *
     * \param[in] vtk_data A VTK data.
     */
    VtkUnstructuredFileFormat( VtkDataType* data ): BaseClass( data ) { initializeCellTypes(); }

public:
    /**
     * Get an interface to iterate by a cell type in a for loop.
     *
     * e.g.
     *
     * ```c++
     * SomeVtkUnstructuredFileFormat file_format;
     * for (SomeVtkUnstructuredFileFormat single_cell_type_file_format :
     *     file_format.eachCellType()) {
     *     // ...
     * }
     * ```
     *
     * \return An interface to iterate by a cell type.
     */
    kvs::ExtendedFileFormat::detail::CellTypeContainer<
        VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>
    eachCellType()
    {
        return kvs::ExtendedFileFormat::detail::CellTypeContainer<
            VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>(
            m_cell_types, BaseClass::get() );
    }
    /**
     * Get an interface to iterate by a cell type in a for loop.
     *
     * ```c++
     * SomeVtkUnstructuredFileFormat file_format;
     * for (SomeVtkUnstructuredFileFormat single_cell_type_file_format :
     *     file_format.eachCellType()) {
     *     // ...
     * }
     * ```
     *
     * \return An interface to iterate by a cell type.
     */
    kvs::ExtendedFileFormat::detail::CellTypeContainer<
        VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>
    eachCellType() const
    {
        return kvs::ExtendedFileFormat::detail::CellTypeContainer<
            VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>(
            m_cell_types, BaseClass::get() );
    }
    /**
     * Get all cell types contained in this data.
     *
     * \return A VTK cell type ID list.
     */
    const std::unordered_set<int>& cellTypes() { return m_cell_types; }
    /**
     * Get all cell types contained in this data.
     *
     * \return A VTK cell type ID list.
     */
    const std::unordered_set<int>& cellTypes() const { return m_cell_types; }
    /**
     * Check if this data could convert to a KVS PointObject.
     *
     * \return `true` if this is convertible, otherwise `false`.
     */
    bool isPointObjectConvertible() const
    {
        constexpr int CELL_TYPE_ID[] = { VTK_VERTEX, VTK_POLY_VERTEX };
        return isObjectConvertibleImpl( CELL_TYPE_ID, 2 );
    }
    /**
     * Check if this data could convert to a KVS LineObject.
     *
     * \return `true` if this is convertible, otherwise `false`.
     */
    bool isLineObjectConvertible() const
    {
        constexpr int CELL_TYPE_ID[] = { VTK_LINE, VTK_POLY_LINE };
        return isObjectConvertibleImpl( CELL_TYPE_ID, 2 );
    }
    /**
     * Check if this data could convert to a KVS LineObject.
     *
     * \return `true` if this is convertible, otherwise `false`.
     */
    bool isPolygonObjectConvertible() const
    {
        constexpr int CELL_TYPE_ID[] = { VTK_PIXEL, VTK_QUAD };
        return ( m_cell_types.size() == 1 && *m_cell_types.begin() == VTK_TRIANGLE ) ||
               isObjectConvertibleImpl( CELL_TYPE_ID, 2 );
    }

private:
    bool isObjectConvertibleImpl( const int* ids, int n ) const
    {
        bool x = true;
        for ( auto t : m_cell_types )
        {
            x &= std::find( ids, ids + n, t ) != ( ids + n );
        }
        return x;
    }

    void initializeCellTypes()
    {
        auto data = this->get();

        if ( data )
        {
            auto cell_types = data->GetDistinctCellTypesArray();
            m_cell_types.insert( cell_types->Begin(), cell_types->End() );
        }
    }

private:
    std::unordered_set<int> m_cell_types;
};
} // namespace ExtendedFileFormat
} // namespace kvs

#endif // CVT__VTK_UNSTRUCTURED_FILE_FORMAT_H_INCLUDE