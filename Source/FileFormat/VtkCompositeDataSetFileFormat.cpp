/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "VtkCompositeDataSetFileFormat.h"

#include <stdexcept>
#include <string>

#include "kvs/FileFormatBase"
#include <vtkDataSet.h>
#include <vtkPolyData.h>

#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"

std::shared_ptr<kvs::FileFormatBase> cvt::detail::VtkMultiBlockIterator::dereference() const
{
    vtkDataObject* o = itr->GetCurrentDataObject();
    std::string class_name = o->GetClassName();

    if ( class_name == "vtkPolyData" )
    {
        auto f = std::make_shared<cvt::VtkXmlPolyData>();

        f->set( dynamic_cast<vtkPolyData*>( o ) );

        return f;
    }
    else if ( class_name == "vtkRectilinearGrid" )
    {
        auto f = std::make_shared<cvt::VtkXmlRectilinearGrid>();

        f->set( dynamic_cast<vtkRectilinearGrid*>( o ) );

        return f;
    }
    else if ( class_name == "vtkUnstructuredGrid" )
    {
        auto f = std::make_shared<cvt::VtkXmlUnstructuredGrid>();

        f->set( dynamic_cast<vtkUnstructuredGrid*>( o ) );

        return f;
    }
    else
    {
        return nullptr;
    }
}