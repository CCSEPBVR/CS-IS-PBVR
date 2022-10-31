/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__VTK_MULTI_BLOCK_H_INCLUDE
#define CVT__VTK_MULTI_BLOCK_H_INCLUDE
#include <memory>
#include <stdexcept>
#include <string>

#include "kvs/FileFormatBase"
#include <vtkCompositeDataSet.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkSmartPointer.h>

#include "FileFormat/CompositeDataSetFormatBase.h"
#include "VtkMultiBlockIterator.h"

namespace cvt
{

/**
 * A VTK Multi Block IO class.
 */
class VtkMultiBlock : public cvt::CompositeDataSetFormatBase<cvt::VtkMultiBlockIterator>
{
public:
    /**
     * A base class type.
     */
    typedef cvt::CompositeDataSetFormatBase<cvt::VtkMultiBlockIterator> BaseClass;
    typedef cvt::VtkMultiBlockIterator iterator;
    typedef cvt::VtkMultiBlockIterator const_iterator;

public:
    /**
     * Construct an IO.
     *
     * \param [in] filename A file name.
     */
    VtkMultiBlock( const std::string& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    /**
     * Construct an IO.
     *
     * \param [in] filename A file name.
     */
    VtkMultiBlock( std::string&& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    ~VtkMultiBlock() {}

public:
    bool read( const std::string& filename );
    bool write( const std::string& filename );
    iterator begin() { return cvt::VtkMultiBlockIterator( dataset ); };
    iterator end() { return cvt::VtkMultiBlockIterator( dataset ); };
    const_iterator cbegin() const { return cvt::VtkMultiBlockIterator( dataset ); };
    const_iterator cend() const { return cvt::VtkMultiBlockIterator( dataset ); };

private:
    vtkSmartPointer<vtkCompositeDataSet> dataset;
};
} // namespace cvt
#endif // CVT__VTK_MULTI_BLOCK_H_INCLUDE