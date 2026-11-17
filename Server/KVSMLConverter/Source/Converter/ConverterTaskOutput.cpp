#include "ConverterTaskOutput.h"

#include <sstream>
#include <string>

#include <vtkNew.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include "kvs/KVSMLStructuredVolumeObject"
#include "kvs/KVSMLUnstructuredVolumeObject"

#include "Converter/XmlTag.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

void cvt::ConverterTaskOutput::registerObject( kvs::KVSMLUnstructuredVolumeObject* object,
                                               std::string local_base, int sub_volume_id )
{
    node_counts[local_base][sub_volume_id - 1] = static_cast<int>( object->nnodes() );
    cell_counts[local_base][sub_volume_id - 1] = static_cast<int>( object->ncells() );

    type_of_cells[local_base] = cvt::detail::GetKvsCellTypeId( object->cellType() );
    cvt::detail::MinVec3( min_external_coords, object->minExternalCoord() );
    cvt::detail::MaxVec3( max_external_coords, object->maxExternalCoord() );

    min_object_coords[local_base][sub_volume_id - 1] = object->minObjectCoord();
    max_object_coords[local_base][sub_volume_id - 1] = object->maxObjectCoord();
    value_minmax[local_base] =
        cvt::detail::GetMinMaxValues( object->values(), object->veclen(), object->nnodes() );

    grid_type = "unstructured";
}

void cvt::ConverterTaskOutput::registerObject( kvs::KVSMLStructuredVolumeObject* object,
                                               std::string local_base, int sub_volume_id )
{
    resolution = object->resolution();

    auto number_of_nodes = static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
    node_counts[local_base][sub_volume_id - 1] = number_of_nodes;
    cell_counts[local_base][sub_volume_id - 1] =
        static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );

    type_of_cells[local_base] = 7;
    cvt::detail::MinVec3( min_external_coords, object->minExternalCoord() );
    cvt::detail::MaxVec3( max_external_coords, object->maxExternalCoord() );

    min_object_coords[local_base][sub_volume_id - 1] = object->minObjectCoord();
    max_object_coords[local_base][sub_volume_id - 1] = object->maxObjectCoord();
    value_minmax[local_base] =
        cvt::detail::GetMinMaxValues( object->values(), object->veclen(), number_of_nodes );

    grid_type = "structured";
}

std::string cvt::ConverterTaskOutput::serialize() const
{
    vtkNew<vtkXMLDataElement> root;
    root->SetName( "result" );

    root->SetIntAttribute( cvt::xml::VERSION_TAG, 0 );

    root->SetIntAttribute( cvt::xml::TARGET_INDEX_TAG, target_index );
    root->SetIntAttribute( cvt::xml::TIME_STEP_ID_TAG, time_step );
    root->SetIntAttribute( cvt::xml::LAST_TIME_STEP_ID_TAG, last_time_step );
    root->SetIntAttribute( cvt::xml::MESH_DEFORMATION_FLAG_TAG, has_mesh_deformation );
    root->SetIntAttribute( cvt::xml::GHOST_CELL_COUNT_TAG, ghost_cell_count );

    std::string str;

    for ( auto& e : node_counts )
    {
        auto b = e.first;
        vtkNew<vtkXMLDataElement> pfi;
        pfi->SetName( cvt::xml::PFI_TAG );
        pfi->SetAttribute( cvt::xml::LOCAL_PREFIX_NAME, b.c_str() );

        auto add_element = [&]( const std::string& str, const char* name ) {
            vtkNew<vtkXMLDataElement> element;
            element->SetName( name );
            element->SetCharacterData( str.c_str(), str.length() + 1 );
            pfi->AddNestedElement( element );

            return element;
        };

        auto count = sub_volume_counts.at( b );
        add_element( std::to_string( count ), cvt::xml::SUB_VOLUME_COUNT_TAG );

        add_element( cvt::Join( node_counts.at( b ).data(), node_counts.at( b ).data() + count ),
                     cvt::xml::NODE_COUNT_TAG );
        add_element( cvt::Join( cell_counts.at( b ).data(), cell_counts.at( b ).data() + count ),
                     cvt::xml::CELL_COUNT_TAG );
        add_element( std::to_string( type_of_cells.at( b ) ), cvt::xml::TYPE_OF_CELL_TAG );

        auto& aec = min_external_coords;
        add_element( cvt::Join( aec.data(), aec.data() + 3 ), cvt::xml::MIN_EXTERNAL_COORDS_TAG );
        auto& iec = max_external_coords;
        add_element( cvt::Join( iec.data(), iec.data() + 3 ), cvt::xml::MAX_EXTERNAL_COORDS_TAG );

        auto& aoc = min_object_coords.at( b );
        add_element( cvt::Join( aoc.begin(), aoc.begin() + count ),
                     cvt::xml::MIN_OBJECT_COORDS_TAG );
        auto& ioc = max_object_coords.at( b );
        add_element( cvt::Join( ioc.begin(), ioc.begin() + count ),
                     cvt::xml::MAX_OBJECT_COORDS_TAG );

        auto& mins = std::get<0>( value_minmax.at( b ) );
        add_element( std::to_string( mins.size() ), cvt::xml::VECLEN_TAG );
        add_element( cvt::Join( mins.begin(), mins.end() ), cvt::xml::MIN_VALUES_TAG );
        auto& maxes = std::get<1>( value_minmax.at( b ) );
        add_element( cvt::Join( maxes.begin(), maxes.end() ), cvt::xml::MAX_VALUES_TAG );

        add_element( "cell centered", cvt::xml::VALUE_DEFINITION_TYPE );
        add_element( grid_type, cvt::xml::GRID_TYPE_NAME );

        if ( grid_type == "structured" )
        {
            add_element( cvt::Join( resolution.data(), resolution.data() + 3 ), "resolution" );
        }

        root->AddNestedElement( pfi );
    }

    std::stringstream ss;

    root->PrintXML( ss, vtkIndent() );

    return ss.str();
}

void cvt::ConverterTaskOutput::deserialize( const std::string& expression )
{
    auto parser = vtkNew<vtkXMLDataParser>();
    std::istringstream ss( expression );
    parser->SetStream( &ss );
    parser->Parse();

    auto root = parser->GetRootElement();
    // reuse memories
    std::string value;

    int version = 0;
    root->GetScalarAttribute( cvt::xml::VERSION_TAG, version );
    if ( version != 0 )
    {
        return;
    }

    root->GetScalarAttribute( cvt::xml::TARGET_INDEX_TAG, target_index );
    root->GetScalarAttribute( cvt::xml::TIME_STEP_ID_TAG, time_step );
    root->GetScalarAttribute( cvt::xml::LAST_TIME_STEP_ID_TAG, last_time_step );
    root->GetScalarAttribute( cvt::xml::MESH_DEFORMATION_FLAG_TAG, has_mesh_deformation );
    root->GetScalarAttribute( cvt::xml::GHOST_CELL_COUNT_TAG, ghost_cell_count );

    for ( int i = 0; i < root->GetNumberOfNestedElements(); ++i )
    {
        auto nested = root->GetNestedElement( i );

        if ( cvt::IsTheXmlTag( nested->GetName(), cvt::xml::PFI_TAG ) )
        {
            std::string local_prefix = nested->GetAttribute( cvt::xml::LOCAL_PREFIX_NAME );
            cvt::Trim( local_prefix );

            auto get_value = [&]( const char* name ) {
                auto o = nested->FindNestedElementWithName( name );
                value = o->GetCharacterData();
                cvt::Trim( value );
                return value;
            };

            auto get_int_value = [&]( const char* name ) {
                auto o = nested->FindNestedElementWithName( name );
                value = o->GetCharacterData();
                cvt::Trim( value );
                return std::stoi( value );
            };

            auto get_int_vec = [&]( const char* name, int* vec3, std::size_t  n ) {
                auto o = nested->FindNestedElementWithName( name );
                value = o->GetCharacterData();
                cvt::Trim( value );

                std::stringstream ss( value );

                for ( int j = 0; j < n; ++j )
                {
                    std::string token;
                    std::getline( ss, token, ',' );
                    vec3[j] = std::stoi( token );
                }
            };

            auto get_vec = [&]( const char* name, auto vec3, std::size_t  n = 3 ) {
                auto o = nested->FindNestedElementWithName( name );
                value = o->GetCharacterData();
                cvt::Trim( value );

                std::stringstream ss( value );

                for ( int j = 0; j < n; ++j )
                {
                    std::string token;
                    std::getline( ss, token, ',' );
                    vec3[j] = std::stof( token );
                }
            };
            auto get_vec3 = [&]( const char* name, std::vector<kvs::Vec3>& v ) {
                auto o = nested->FindNestedElementWithName( name );
                value = o->GetCharacterData();
                cvt::Trim( value );

                std::stringstream ss( value );
                std::string token;
                std::string e;

                for ( int j = 0; j < v.size(); ++j )
                {
                    std::getline( ss, token, ',' );
                    std::stringstream st( token );
                    for ( int k = 0; k < 3; ++k )
                    {
                        std::getline( st, e, ' ' );
                        v[j][k] = std::stof( e );
                    }
                }
            };

            type_of_cells[local_prefix] = get_int_value( cvt::xml::TYPE_OF_CELL_TAG );
            int count = get_int_value( cvt::xml::SUB_VOLUME_COUNT_TAG );
            sub_volume_counts[local_prefix] = count;
            node_counts[local_prefix].resize( count );
            get_int_vec( cvt::xml::NODE_COUNT_TAG, node_counts[local_prefix].data(), count );
            cell_counts[local_prefix].resize( count );
            get_int_vec( cvt::xml::CELL_COUNT_TAG, cell_counts[local_prefix].data(), count );

            get_vec( cvt::xml::MIN_EXTERNAL_COORDS_TAG, min_external_coords.data() );
            get_vec( cvt::xml::MAX_EXTERNAL_COORDS_TAG, max_external_coords.data() );

            int veclen = get_int_value( cvt::xml::VECLEN_TAG );
            min_object_coords[local_prefix].resize( veclen );
            max_object_coords[local_prefix].resize( veclen );
            get_vec3( cvt::xml::MIN_OBJECT_COORDS_TAG, min_object_coords[local_prefix] );
            get_vec3( cvt::xml::MAX_OBJECT_COORDS_TAG, max_object_coords[local_prefix] );

            value_minmax[local_prefix] =
                std::make_tuple( std::vector<float>( veclen ), std::vector<float>( veclen ) );

            get_vec( cvt::xml::MIN_VALUES_TAG, std::get<0>( value_minmax[local_prefix] ).data(),
                     veclen );
            get_vec( cvt::xml::MAX_VALUES_TAG, std::get<1>( value_minmax[local_prefix] ).data(),
                     veclen );

            grid_type = get_value( cvt::xml::GRID_TYPE_NAME );
            if ( grid_type == "structured" )
            {
                get_vec( "resolution", resolution.data(), 3 );
            }
        }
    }
}