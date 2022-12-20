/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#define CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE
#include <memory>

#include "kvs/FileFormatBase"
#include <vtkCompositeDataSet.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkSmartPointer.h>
#include <vtkXMLMultiBlockDataWriter.h>

#include "FileFormat/VtkFileFormat.h"
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
        std::runtime_error( "prefix operator was not implemented" );
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

/**
 * A base class for a composite data set IO.
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
     * \return An interface to iterate.
     */
    cvt::detail::VtkMultiBlockContainer<VtkDataType> eachBlock()
    {
        return cvt::detail::VtkMultiBlockContainer<VtkDataType>( BaseClass::get() );
    }
    /**
     * Get an interface to iterate each block.
     *
     * \return An interface to iterate.
     */
    cvt::detail::VtkMultiBlockContainer<VtkDataType> eachBlock() const
    {
        return cvt::detail::VtkMultiBlockContainer<VtkDataType>( BaseClass::get() );
    }
};
} // namespace cvt

#endif // CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE