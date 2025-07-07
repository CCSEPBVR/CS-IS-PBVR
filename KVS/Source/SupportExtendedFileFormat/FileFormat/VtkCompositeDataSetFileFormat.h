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
#ifndef EXTENDED_FILE_FORMAT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#define EXTENDED_FILE_FORMAT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#include <memory>

#include "kvs/FileFormatBase"
#include <vtkCompositeDataSet.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkSmartPointer.h>
#include <vtkXMLMultiBlockDataWriter.h>

#include <kvs/extendedfileformat/VtkXmlPolyData>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>
#include <kvs/extendedfileformat/VtkFileFormat>

namespace kvs
{
namespace ExtendedFileFormat
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
    kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator begin()
    {
        return kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator( vtk_data );
    }
    /**
     * Return an iterator to the end of a composite data set leaf.
     *
     * \return an iterator.
     */
    kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator end()
    {
        return kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator( vtk_data );
    }
    /**
     * Return an iterator to the beginning of a composite data set leaf.
     *
     * \return an iterator.
     */
    kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator begin() const
    {
        return kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator( vtk_data );
    }
    /**
     * Return an iterator to the end of a composite data set leaf.
     *
     * \return an iterator.
     */
    kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator end() const
    {
        return kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator( vtk_data );
    }

private:
    vtkSmartPointer<VtkCompositeDataSet> vtk_data;
};
} // namespace detail
} // namespace ExtendedFileFormat
} // namespace kvs

namespace kvs
{
namespace ExtendedFileFormat
{
namespace detail
{
/**
 * \private
 */
std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> MergeBlocks( vtkCompositeDataSet* dataset,
                                                          const char* file_path,
                                                          bool enable_point_merge );
/**
 * \private
 */
std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlPolyData> MergeBlocksAsPolyData( vtkCompositeDataSet* dataset,
                                                            const char* file_path,
                                                            bool enable_point_merge );
} // namespace detail
} // namespace ExtendedFileFormat
} // namespace kvs

namespace kvs
{
namespace ExtendedFileFormat
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
     * kvs::ExtendedFileFormat::VtkCompositeDataSetFileFormat<MultiBlockType> multi_block;
     *
     * for (auto file_format : multi_block.eachBlock()) {
     *   //..
     * }
     * ```
     *
     * \return An interface to iterate each block.
     */
    kvs::ExtendedFileFormat::detail::VtkMultiBlockContainer<VtkDataType> eachBlock()
    {
        return kvs::ExtendedFileFormat::detail::VtkMultiBlockContainer<VtkDataType>( BaseClass::get() );
    }
    /**
     * Get an interface to iterate each block.
     *
     * e.g.
     *
     * ```c++
     * kvs::ExtendedFileFormat::VtkCompositeDataSetFileFormat<MultiBlockType> multi_block;
     *
     * for (auto file_format : multi_block.eachBlock()) {
     *   //..
     * }
     * ```
     *
     * \return An interface to iterate each block.
     */
    kvs::ExtendedFileFormat::detail::VtkMultiBlockContainer<VtkDataType> eachBlock() const
    {
        return kvs::ExtendedFileFormat::detail::VtkMultiBlockContainer<VtkDataType>( BaseClass::get() );
    }

public:
    /**
     * Merge some blocks.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged block unstructured grid.
     */
    std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> mergeBlocks( const char* file_path,
                                                              bool enable_point_merge = true )
    {
        return kvs::ExtendedFileFormat::detail::MergeBlocks( BaseClass::get(), file_path, enable_point_merge );
    }
    /**
     * Merge some blocks.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged block unstructured grid.
     */
    std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> mergeBlocks( const std::string& file_path,
                                                              bool enable_point_merge = true )
    {
        return kvs::ExtendedFileFormat::detail::MergeBlocks( BaseClass::get(), file_path.c_str(), enable_point_merge );
    }
    /**
     * Extract some block surfaces.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged surface.
     */
    std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlPolyData> mergeBlocksAsPolygon( const char* file_path,
                                                               bool enable_point_merge )
    {
        return kvs::ExtendedFileFormat::detail::MergeBlocksAsPolyData( BaseClass::get(), file_path,
                                                   enable_point_merge );
    }
    /**
     * Extract some block surfaces.
     *
     * \param[in] file_path A XML file path to configure block merge.
     * \param[in] enable_point_merge Merge same coordinate points if this is true.
     * \return A merged surface.
     */
    std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlPolyData> mergeBlocksAsPolygon( const std::string& file_path,
                                                               bool enable_point_merge = true )
    {
        return kvs::ExtendedFileFormat::detail::MergeBlocksAsPolyData( BaseClass::get(), file_path.c_str(),
                                                   enable_point_merge );
    }
};
} // namespace ExtendedFileFormat
} // namespace kvs

#endif // CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE