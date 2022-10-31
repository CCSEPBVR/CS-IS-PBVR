/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_MULTI_BLOCK_ITERATOR_H_INCLUDE
#define CVT__VTK_MULTI_BLOCK_ITERATOR_H_INCLUDE

#include <memory>

#include "kvs/FileFormatBase"
#include <vtkCompositeDataSet.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkSmartPointer.h>

#include "FileFormat/VtkFileFormat.h"

namespace cvt
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
    VtkMultiBlockIterator( vtkSmartPointer<vtkCompositeDataSet> dataset ):
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
} // namespace cvt
#endif // CVT__VTK_MULTI_BLOCK_ITERATOR_H_INCLUDE