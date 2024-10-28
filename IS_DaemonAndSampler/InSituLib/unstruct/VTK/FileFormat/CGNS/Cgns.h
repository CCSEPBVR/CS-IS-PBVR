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
     * \param[in] filename A file name.
     */
    Cgns( const std::string& filename ): BaseClass()
    {
        kvs::FileFormatBase::setFilename( filename );
        this->read( filename );
    }
    /**
     * Construct an IO.
     *
     * \param[in] filename A file name.
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
     * \param[in] vtk_data A VTK data.
     */
    Cgns( vtkMultiBlockDataSet* vtk_data ): BaseClass(), vtk_data( vtk_data ) {}

public:
    bool read( const std::string& filename ) override
    {
        setSuccess( false );
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

            setSuccess( true );
            return true;
        }
        catch ( std::exception& e )
        {
            kvsMessageError( e.what() );
            return false;
        }
    }
    bool write( const std::string& filename ) override
    {
        setSuccess( false );
        kvsMessageError( "This function has not been implemented yet" );
        return false;
    };
    /**
     * Get an interface to iterate each block.
     *
     * e.g.
     *
     * ```c++
     * cvt::Cgns cgns;
     * for (auto file_format : cgns.eachBlock()) {
     *     // ..
     * }
     * ```
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
     * e.g.
     *
     * ```c++
     * cvt::Cgns cgns;
     * for (auto file_format : cgns.eachBlock()) {
     *     // ..
     * }
     * ```
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