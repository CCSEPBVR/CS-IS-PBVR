/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "VtkMultiBlockIterator.h"

#include <stdexcept>
#include <string>

#include "kvs/FileFormatBase"
#include <vtkDataSet.h>
#include <vtkPolyData.h>

#include "FileFormat/VTK/VtkXmlPolyData.h"

std::shared_ptr<kvs::FileFormatBase> cvt::VtkMultiBlockIterator::dereference() const
{
    vtkDataObject* o = itr->GetCurrentDataObject();
    std::string class_name = o->GetClassName();

    if ( class_name == "vtkPolyData" )
    {
        auto f = std::make_shared<cvt::VtkXmlPolyData>();

        f->set( dynamic_cast<vtkPolyData*>( o ) );

        return f;
    }
    else
    {
        std::runtime_error( "Unsupported vtk data type." );
        return nullptr;
    }
}