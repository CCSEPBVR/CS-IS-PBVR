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
#ifndef CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#define CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#include <memory>

#include "kvs/FileFormatBase"
#include <vtkCompositeDataSet.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkSmartPointer.h>
#include <vtkXMLMultiBlockDataWriter.h>

#include "VTK/VtkXmlPolyData.h"
#include "VTK/VtkXmlUnstructuredGrid.h"
#include "VtkFileFormat.h"

namespace cvt
{
namespace detail
{
/**
 * A forward-iterator like wrapper class to traverse a vtkCompositeDataSet object.
 *
 * This wrapper class must be used only in for-loop.
 * DO NOT use like a normal forward iterator, especially an iterator comparison.
 */
class VtkMultiBlockIterator
{
public:
    VtkMultiBlockIterator( vtkCompositeDataSet* dataset ):
        itr( vtkSmartPointer<vtkDataObjectTreeIterator>::New() )
    {
        itr->SetDataSet( dataset );
        itr->SkipEmptyNodesOn();
        itr->VisitOnlyLeavesOn();
        itr->InitTraversal();
    }

public:
    std::shared_ptr<kvs::FileFormatBase> operator*() { return dereference(); }
    std::shared_ptr<kvs::FileFormatBase> operator*() const { return dereference(); }
    VtkMultiBlockIterator& operator++()
    {
        itr->GoToNextItem();
        return *this;
    }
    VtkMultiBlockIterator operator++( int )
    {
        std::runtime_error( "Prefix operator has not implemented" );
        return *this;
    }
    bool operator==( VtkMultiBlockIterator& other ) { return itr->IsDoneWithTraversal() == 1; }
    bool operator==( VtkMultiBlockIterator& other ) const
    {
        return itr->IsDoneWithTraversal() == 1;
    }
    /**
     * Check only if this has finished a traversal.
     *
     * \param [in] other Don't be used.
     */
    bool operator==( const VtkMultiBlockIterator& other ) const
    {
        return itr->IsDoneWithTraversal() == 1;
    }
    bool operator!=( VtkMultiBlockIterator& other ) { return !( *this == other ); }
    bool operator!=( VtkMultiBlockIterator& other ) const { return !( *this == other ); }
    /**
     * Check only if this has not finished a traversal.
     *
     * \param [in] other Don't be used.
     */
    bool operator!=( const VtkMultiBlockIterator& other ) const { return !( *this == other ); }

public:
    std::shared_ptr<kvs::FileFormatBase> dereference() const;

private:
    vtkSmartPointer<vtkDataObjectTreeIterator> itr;
};

template <typename VtkCompositeDataSet>
class VtkMultiBlockContainer
{
public:
    VtkMultiBlockContainer( vtkSmartPointer<VtkCompositeDataSet> vtk_data ): vtk_data( vtk_data ) {}

public:
    /**
     * Return an iterator to the beginning of a composite data set leaf.
     *
     * \return an iterator.
     */
    cvt::detail::VtkMultiBlockIterator begin()
    {
        return cvt::detail::VtkMultiBlockIterator( vtk_data );
    }
    /**
     * Return an iterator to the end of a composite data set leaf.
     *
     * \return an iterator.
     */
    cvt::detail::VtkMultiBlockIterator end()
    {
        return cvt::detail::VtkMultiBlockIterator( vtk_data );
    }
    /**
     * Return an iterator to the beginning of a composite data set leaf.
     *
     * \return an iterator.
     */
    cvt::detail::VtkMultiBlockIterator begin() const
    {
        return cvt::detail::VtkMultiBlockIterator( vtk_data );
    }
    /**
     * Return an iterator to the end of a composite data set leaf.
     *
     * \return an iterator.
     */
    cvt::detail::VtkMultiBlockIterator end() const
    {
        return cvt::detail::VtkMultiBlockIterator( vtk_data );
    }

private:
    vtkSmartPointer<VtkCompositeDataSet> vtk_data;
};
} // namespace detail
} // namespace cvt

namespace cvt
{
namespace detail
{
/**
 * \private
 */
std::shared_ptr<cvt::VtkXmlUnstructuredGrid> MergeBlocks( vtkCompositeDataSet* dataset,
                                                          const char* file_path,
                                                          bool enable_point_merge );
/**
 * \private
 */
std::shared_ptr<cvt::VtkXmlPolyData> MergeBlocksAsPolyData( vtkCompositeDataSet* dataset,
                                                            const char* file_path,
                                                            bool enable_point_merge );
} // namespace detail
} // namespace cvt

namespace cvt
{

/**
 * A VTK composite data set IO.
 *
 * This file format must not pass exporters.
 */
template <typename VtkDataType, typename VtkDataReader>
class VtkCompositeDataSetFileFormat
    : public VtkFileFormat<VtkDataType, VtkDataReader, vtkXMLMultiBlockDataWriter>
{
public:
    /**
     * A base class type.
     */
    using BaseClass = VtkFileFormat<VtkDataType, VtkDataReader, vtkXMLMultiBlockDataWriter>;

public:
    using BaseClass::BaseClass;
    ~VtkCompositeDataSetFileFormat() {}

public:
    /**
     * Get an interface to iterate each block.
     *
     * e.g.
     *
     * ```c++
     * cvt::VtkCompositeDataSetFileFormat<MultiBlockType> multi_block;
     *
     * for (auto file_format : multi_block.eachBlock()) {
     *   //..
     * }
     * ```
     *
     * \return An interface to iterate each block.
     */
    cvt::detail::VtkMultiBlockContainer<VtkDataType> eachBlock()
    {
        return cvt::detail::VtkMultiBlockContainer<VtkDataType>( BaseClass::get() );
    }
    /**
     * Get an interface to iterate each block.
     *
     * e.g.
     *
     * ```c++
     * cvt::VtkCompositeDataSetFileFormat<MultiBlockType> multi_block;
     *
     * for (auto file_format : multi_block.eachBlock()) {
     *   //..
     * }
     * ```
     *
     * \return An interface to iterate each block.
     */
    cvt::detail::VtkMultiBlockContainer<VtkDataType> eachBlock() const
    {
        return cvt::detail::VtkMultiBlockContainer<VtkDataType>( BaseClass::get() );
    }

public:
    /**
     * Merge some blocks.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged block unstructured grid.
     */
    std::shared_ptr<cvt::VtkXmlUnstructuredGrid> mergeBlocks( const char* file_path,
                                                              bool enable_point_merge = true )
    {
        return cvt::detail::MergeBlocks( BaseClass::get(), file_path, enable_point_merge );
    }
    /**
     * Merge some blocks.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged block unstructured grid.
     */
    std::shared_ptr<cvt::VtkXmlUnstructuredGrid> mergeBlocks( const std::string& file_path,
                                                              bool enable_point_merge = true )
    {
        return cvt::detail::MergeBlocks( BaseClass::get(), file_path.c_str(), enable_point_merge );
    }
    /**
     * Extract some block surfaces.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged surface.
     */
    std::shared_ptr<cvt::VtkXmlPolyData> mergeBlocksAsPolygon( const char* file_path,
                                                               bool enable_point_merge )
    {
        return cvt::detail::MergeBlocksAsPolyData( BaseClass::get(), file_path,
                                                   enable_point_merge );
    }
    /**
     * Extract some block surfaces.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged surface.
     */
    std::shared_ptr<cvt::VtkXmlPolyData> mergeBlocksAsPolygon( const std::string& file_path,
                                                               bool enable_point_merge = true )
    {
        return cvt::detail::MergeBlocksAsPolyData( BaseClass::get(), file_path.c_str(),
                                                   enable_point_merge );
    }
};
} // namespace cvt

#endif // CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE