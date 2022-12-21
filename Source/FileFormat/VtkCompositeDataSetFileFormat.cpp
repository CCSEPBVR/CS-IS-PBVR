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
#include <vtkImageData.h>
#include <vtkPCellDataToPointData.h>
#include <vtkPolyData.h>
#include <vtkRemoveGhosts.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include "FileFormat/VTK/VtkXmlImageData.h"
#include "FileFormat/VTK/VtkXmlPolyData.h"
#include "FileFormat/VTK/VtkXmlRectilinearGrid.h"
#include "FileFormat/VTK/VtkXmlStructuredGrid.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"

std::shared_ptr<kvs::FileFormatBase> cvt::detail::VtkMultiBlockIterator::dereference() const
{
    vtkDataObject* x = itr->GetCurrentDataObject();
    std::string class_name = x->GetClassName();

    if ( class_name == "vtkImageData" )
    {
        auto f = std::make_shared<cvt::VtkXmlImageData>( dynamic_cast<vtkImageData*>( x ) );

        return f;
    }
    else if ( class_name == "vtkPolyData" )
    {
        vtkNew<vtkRemoveGhosts> ghost_remover;
        ghost_remover->SetInputData( x );

        vtkNew<vtkPCellDataToPointData> point_data_sampler;
        point_data_sampler->SetInputConnection( ghost_remover->GetOutputPort() );

        point_data_sampler->Update();

        auto o = point_data_sampler->GetOutput();

        auto f = std::make_shared<cvt::VtkXmlPolyData>( dynamic_cast<vtkPolyData*>( o ) );

        return f;
    }
    else if ( class_name == "vtkRectilinearGrid" )
    {
        auto f =
            std::make_shared<cvt::VtkXmlRectilinearGrid>( dynamic_cast<vtkRectilinearGrid*>( x ) );

        return f;
    }
    else if ( class_name == "vtkStructuredGrid" )
    {
        auto f =
            std::make_shared<cvt::VtkXmlStructuredGrid>( dynamic_cast<vtkStructuredGrid*>( x ) );

        return f;
    }
    else if ( class_name == "vtkUnstructuredGrid" )
    {
        vtkNew<vtkRemoveGhosts> ghost_remover;
        ghost_remover->SetInputData( x );

        vtkNew<vtkPCellDataToPointData> point_data_sampler;
        point_data_sampler->SetInputConnection( ghost_remover->GetOutputPort() );

        point_data_sampler->Update();

        auto o = point_data_sampler->GetOutput();
        auto f = std::make_shared<cvt::VtkXmlUnstructuredGrid>(
            dynamic_cast<vtkUnstructuredGrid*>( o ) );

        return f;
    }
    else
    {
        return nullptr;
    }
}
