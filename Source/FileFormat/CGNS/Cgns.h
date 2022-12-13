/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__CGNS_H_INCLUDE
#define CVT__CGNS_H_INCLUDE
#include "kvs/FileFormatBase"
#include <vtkCGNSReader.h>
#include <vtkMultiBlockDataSet.h>

#include "FileFormat/VtkCompositeDataSetFileFormat.h"

namespace cvt
{

/**
 * A CGNS file IO.
 */
class Cgns : public kvs::FileFormatBase
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
    Cgns(): kvs::FileFormatBase() {}
    /**
     * Construct an IO.
     *
     * \param [in] filename A file name.
     */
    Cgns( const std::string& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    /**
     * Construct an IO.
     *
     * \param [in] filename A file name.
     */
    Cgns( std::string&& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    /**
     * Construct an IO.
     *
     * This takes the owner of the object.
     * So DO NOT free the object externally.
     *
     * \param [in] vtk_data A VTK data.
     */
    Cgns( vtkMultiBlockDataSet* vtk_data ): BaseClass(), vtk_data( vtk_data ) {}

public:
    bool read( const std::string& filename ) override
    {
        try
        {
            vtkNew<vtkCGNSReader> reader;
            reader->SetFileName( filename.c_str() );
            reader->UpdateInformation();
            reader->EnableAllBases();
            reader->EnableAllPointArrays();
            reader->EnableAllCellArrays();

            reader->Update();

            vtk_data = dynamic_cast<vtkMultiBlockDataSet*>( reader->GetOutput() );

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
        std::runtime_error( "This function has not been implemented yet" );
        return false;
    };
    /**
     * Get an interface to iterate each block.
     *
     * \return An interface to iterate.
     */
    cvt::detail::VtkMultiBlockContainer<vtkMultiBlockDataSet> eachBlock()
    {
        return cvt::detail::VtkMultiBlockContainer<vtkMultiBlockDataSet>( vtk_data );
    }
    /**
     * Get an interface to iterate each block.
     *
     * \return An interface to iterate.
     */
    cvt::detail::VtkMultiBlockContainer<vtkMultiBlockDataSet> eachBlock() const
    {
        return cvt::detail::VtkMultiBlockContainer<vtkMultiBlockDataSet>( vtk_data );
    }

private:
    vtkSmartPointer<vtkMultiBlockDataSet> vtk_data;
};
} // namespace cvt

#endif // CVT__CGNS_H_INCLUDE