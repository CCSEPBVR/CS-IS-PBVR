/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "FileFormat/VTKMultiBlock/VtkMultiBlock.h"

#include <vtkCompositeDataSet.h>
#include <vtkNew.h>
#include <vtkXMLMultiBlockDataReader.h>

bool cvt::VtkMultiBlock::read( const std::string& filename )
{
    try
    {
        vtkNew<vtkXMLMultiBlockDataReader> reader;
        reader->SetFileName( filename.c_str() );
        reader->Update();

        dataset = dynamic_cast<vtkCompositeDataSet*>( reader->GetOutput() );

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

bool cvt::VtkMultiBlock::write( const std::string& filename )
{
    std::runtime_error( "not implemented yet" );
    return false;
}