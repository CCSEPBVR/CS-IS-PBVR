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

#include "FileFormat/VtkFileFormat.h"

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
class VtkCompositeDataSetFileFormat : public kvs::FileFormatBase
{
public:
    /**
     * A base class type.
     */
    typedef kvs::FileFormatBase BaseClass;

public:
    /**
     * Construct an IO.
     */
    VtkCompositeDataSetFileFormat(): kvs::FileFormatBase() {}
    /**
     * Construct an IO.
     *
     * \param [in] filename A file name.
     */
    VtkCompositeDataSetFileFormat( const std::string& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    /**
     * Construct an IO.
     *
     * \param [in] filename A file name.
     */
    VtkCompositeDataSetFileFormat( std::string&& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    VtkCompositeDataSetFileFormat( VtkDataType* vtk_data ): BaseClass(), vtk_data( vtk_data ) {}

public:
    bool read( const std::string& filename ) override
    {
        try
        {
            vtkNew<VtkDataReader> reader;
            reader->SetFileName( filename.c_str() );
            reader->Update();

            vtk_data = dynamic_cast<VtkDataType*>( reader->GetOutput() );

            return true;
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
    bool write( const std::string& filename ) override
    {
        std::runtime_error( "This function has not implemented yet" );
        return false;
    };

    cvt::detail::VtkMultiBlockContainer<VtkDataType> eachBlocks()
    {
        return cvt::detail::VtkMultiBlockContainer<VtkDataType>( vtk_data );
    }

private:
    vtkSmartPointer<VtkDataType> vtk_data;
};
} // namespace cvt

#endif // CVT__COMPOSITE_DATA_FORMAT_BASE_H_INCLUDE