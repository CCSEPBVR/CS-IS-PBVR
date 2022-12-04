/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_UNSTRUCTURED_FILE_FORMAT_H_INCLUDE
#define CVT__VTK_UNSTRUCTURED_FILE_FORMAT_H_INCLUDE
#include "FileFormat/VtkFileFormat.h"

#include <unordered_set>

#include <vtkExtractCellsByType.h>

namespace cvt
{

template <typename BaseClass>
class CellTypeIterator
{
    using VtkDataPointerType = vtkSmartPointer<typename BaseClass::VtkDataType>;

private:
    using InternalIterator = std::unordered_set<int>::iterator;

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
} // namespace cvt

namespace cvt
{

/**
 * A file IO for vtkUnstructuredGrid.
 */
template <typename VtkDataType, typename VtkReaderType, typename VtkWriterType>
class VtkUnstructuredFileFormat
    : public cvt::VtkFileFormat<VtkDataType, VtkReaderType, VtkWriterType>
{
public:
    /**
     * A base class type.
     */
    using BaseClass = cvt::VtkFileFormat<VtkDataType, VtkReaderType, VtkWriterType>;

public:
    /**
     * Construct an empty IO object.
     */
    VtkUnstructuredFileFormat() noexcept: BaseClass() {}
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     */
    VtkUnstructuredFileFormat( const std::string& filename ): BaseClass( filename )
    {
        InitializeCellTypes();
    }
    /**
     * Construct an IO object.
     *
     * \param filename A file name.
     */
    VtkUnstructuredFileFormat( std::string&& filename ): BaseClass( filename )
    {
        InitializeCellTypes();
    }
    /**
     * Construct an IO object.
     *
     * \param [in] vtk_data A VTK data.
     */
    VtkUnstructuredFileFormat( VtkDataType* data ): BaseClass( data ) { InitializeCellTypes(); }
    virtual ~VtkUnstructuredFileFormat() {}

public:
    cvt::CellTypeIterator<VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>
    begin()
    {
        return cvt::CellTypeIterator<
            VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>(
            cell_types.begin(), BaseClass::get() );
    }
    cvt::CellTypeIterator<VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>
    end()
    {
        return cvt::CellTypeIterator<
            VtkUnstructuredFileFormat<VtkDataType, VtkReaderType, VtkWriterType>>(
            cell_types.end(), BaseClass::get() );
    }

private:
    void InitializeCellTypes()
    {
        auto data = this->get();

        if ( data->GetNumberOfPoints() > 0 )
        {
            for ( vtkIdType i = 0; i < data->GetNumberOfCells(); ++i )
            {
                auto cell = data->GetCell( i );
                cell_types.insert( cell->GetCellType() );
            }
        }
    }

private:
    std::unordered_set<int> cell_types;
};
} // namespace cvt

#endif // CVT__VTK_UNSTRUCTURED_FILE_FORMAT_H_INCLUDE