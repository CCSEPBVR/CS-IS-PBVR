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
#include <kvs/extendedfileformat/VtkCompositeDataSetFileFormat>

#include <set>
#include <stdexcept>
#include <string>

#include "kvs/FileFormatBase"

#include <vtkAppendFilter.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkDataSet.h>
#include <vtkGeometryFilter.h>
#include <vtkImageData.h>
#include <vtkPCellDataToPointData.h>
#include <vtkPolyData.h>
#include <vtkRemoveGhosts.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLDataParser.h>

#include <kvs/extendedfileformat/Stl>
#include <kvs/extendedfileformat/VtkXmlImageData>
#include <kvs/extendedfileformat/VtkXmlPolyData>
#include <kvs/extendedfileformat/VtkXmlRectilinearGrid>
#include <kvs/extendedfileformat/VtkXmlStructuredGrid>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>

namespace
{
std::set<int> ReadMergeBlockIndices( const char* file_path )
{
    std::set<int> merge_block_indices;

    auto parser = vtkNew<vtkXMLDataParser>();
    parser->SetFileName( file_path );
    std::cout << parser->Parse() << std::endl;

    auto root = parser->GetRootElement();
    std::cout << root->GetName() << std::endl;

    for ( int i = 0; i < root->GetNumberOfNestedElements(); ++i )
    {
        auto child = root->GetNestedElement( i );

        const char* SERIAL_BLOCK_TAG_NAME = "serialBlocks";
        if ( std::strncmp( SERIAL_BLOCK_TAG_NAME, child->GetName(),
                           sizeof( *SERIAL_BLOCK_TAG_NAME ) ) == 0 )
        {
            int start = -1;
            int end = -1;
            auto start_value = child->GetAttribute( "start" );
            if ( start_value != 0 )
            {
                start = std::atoi( start_value );
            }
            auto end_value = child->GetAttribute( "end" );
            if ( end_value != 0 )
            {
                end = std::atoi( end_value );
            }

            if ( start >= 0 && end >= 0 )
            {
                for ( int j = start; j <= end; ++j )
                {
                    merge_block_indices.insert( j );
                }
            }
            else
            {
                kvsMessageError(
                    "start and end attribute in serial-blocks tag must be positive numbers" );
                return std::set<int>();
            }
        }
        const char* BLOCK_TAG_NAME = "block";
        if ( std::strncmp( BLOCK_TAG_NAME, child->GetName(), sizeof( *BLOCK_TAG_NAME ) ) == 0 )
        {
            auto block_value = child->GetCharacterData();
            int block = std::atoi( block_value );
            if ( block >= 0 )
            {
                merge_block_indices.insert( block );
            }
            else
            {
                kvsMessageError( "A value of block tag must be positive numbers" );
                return std::set<int>();
            }
        }
    }

    for ( auto i : merge_block_indices )
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    return merge_block_indices;
}

vtkSmartPointer<vtkUnstructuredGrid> MergeBlocks( vtkCompositeDataSet* dataset,
                                                  const char* file_path, bool enable_point_merge )
{
    auto merge_block_indices = ::ReadMergeBlockIndices( file_path );

    vtkNew<vtkDataObjectTreeIterator> itr;

    itr->SetDataSet( dataset );
    itr->SkipEmptyNodesOn();
    itr->VisitOnlyLeavesOn();
    itr->InitTraversal();

    vtkNew<vtkAppendFilter> append_filter;
    append_filter->SetMergePoints( enable_point_merge );

    int i = 0;
    for ( ; itr->IsDoneWithTraversal() != 1; itr->GoToNextItem() )
    {
        if ( merge_block_indices.count( i ) > 0 )
        {
            vtkSmartPointer<vtkDataSet> data_object =
                vtkDataSet::SafeDownCast( itr->GetCurrentDataObject() );
            append_filter->AddInputData( data_object );
        }
        ++i;
    }

    append_filter->Update();
    vtkSmartPointer<vtkUnstructuredGrid> o = append_filter->GetOutput();

    return o;
}
} // namespace

std::shared_ptr<kvs::FileFormatBase> kvs::ExtendedFileFormat::detail::VtkMultiBlockIterator::dereference() const
{
    vtkDataObject* x = itr->GetCurrentDataObject();
    std::string class_name = x->GetClassName();

    if ( class_name == "vtkImageData" )
    {
        auto f = std::make_shared<kvs::ExtendedFileFormat::VtkXmlImageData>( dynamic_cast<vtkImageData*>( x ) );

        return f;
    }
    else if ( class_name == "vtkPolyData" )
    {
        vtkNew<vtkPCellDataToPointData> point_data_sampler;
        point_data_sampler->SetInputData( x );
        vtkNew<vtkRemoveGhosts> ghost_remover;
        ghost_remover->SetInputConnection( point_data_sampler->GetOutputPort() );
        ghost_remover->Update();

        auto f = std::make_shared<kvs::ExtendedFileFormat::VtkXmlPolyData>(
            dynamic_cast<vtkPolyData*>( ghost_remover->GetOutput() ) );

        return f;
    }
    else if ( class_name == "vtkRectilinearGrid" )
    {
        auto f =
            std::make_shared<kvs::ExtendedFileFormat::VtkXmlRectilinearGrid>( dynamic_cast<vtkRectilinearGrid*>( x ) );

        return f;
    }
    else if ( class_name == "vtkStructuredGrid" )
    {
        auto f =
            std::make_shared<kvs::ExtendedFileFormat::VtkXmlStructuredGrid>( dynamic_cast<vtkStructuredGrid*>( x ) );

        return f;
    }
    else if ( class_name == "vtkUnstructuredGrid" )
    {

        vtkNew<vtkPCellDataToPointData> point_data_sampler;
        point_data_sampler->SetInputData( x );
        vtkNew<vtkRemoveGhosts> ghost_remover;
        ghost_remover->SetInputConnection( point_data_sampler->GetOutputPort() );
        ghost_remover->Update();

        auto f = std::make_shared<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid>(
            dynamic_cast<vtkUnstructuredGrid*>( ghost_remover->GetOutput() ) );

        return f;
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> kvs::ExtendedFileFormat::detail::MergeBlocks( vtkCompositeDataSet* dataset,
                                                                       const char* file_path,
                                                                       bool enable_point_merge )
{
    auto merged = ::MergeBlocks( dataset, file_path, enable_point_merge );

    return std::make_shared<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid>( merged );
}

std::shared_ptr<kvs::ExtendedFileFormat::VtkXmlPolyData> kvs::ExtendedFileFormat::detail::MergeBlocksAsPolyData(
    vtkCompositeDataSet* dataset, const char* file_path, bool enable_point_merge )
{
    auto merged = ::MergeBlocks( dataset, file_path, enable_point_merge );

    vtkNew<vtkGeometryFilter> geo;
    geo->SetInputData( merged );
    geo->Update();

    return std::make_shared<kvs::ExtendedFileFormat::VtkXmlPolyData>( geo->GetOutput() );
}
