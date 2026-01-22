#include "Converter/ConverterTaskInput.h"

#include <sstream>
#include <string>

#include <vtkNew.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include "Converter/XmlTag.h"

std::string cvt::ConverterTaskInput::serialize() const
{
    vtkNew<vtkXMLDataElement> input_element;
    input_element->SetName( cvt::xml::INPUT_TAG );
    input_element->SetAttribute( cvt::xml::GRID_TYPE_NAME, source_grid_type.c_str() );

    for ( int i = 0; i < source_file_paths.size(); ++i )
    {
        auto p = source_file_paths[i];
        vtkNew<vtkXMLDataElement> src_element;
        src_element->SetName( cvt::xml::SINGLE_TAG );
        src_element->SetCharacterData( p.c_str(), p.length() + 1 );
        src_element->SetAttribute( "q", q[i].c_str() );
        src_element->SetAttribute( "f", f[i].c_str() );

        input_element->AddNestedElement( src_element );
    }

    vtkNew<vtkXMLDataElement> output_element;
    output_element->SetName( cvt::xml::OUTPUT_TAG );

    vtkNew<vtkXMLDataElement> directory_element;
    directory_element->SetName( cvt::xml::DIRECTORY_PATH_TAG );
    directory_element->SetCharacterData( destination_directory.c_str(),
                                         destination_directory.length() + 1 );
    output_element->AddNestedElement( directory_element );

    vtkNew<vtkXMLDataElement> prefix_element;
    prefix_element->SetName( cvt::xml::PREFIX_TAG );
    prefix_element->SetCharacterData( destination_prefix.c_str(), destination_prefix.length() + 1 );
    output_element->AddNestedElement( prefix_element );

    vtkNew<vtkXMLDataElement> root;
    root->SetName( cvt::xml::SUB_TARGET_TAG );
    root->AddNestedElement( input_element );
    root->AddNestedElement( output_element );
    root->SetIntAttribute( cvt::xml::TARGET_INDEX_TAG, target_index );
    root->SetIntAttribute( cvt::xml::TIME_STEP_ID_TAG, time_step );
    root->SetIntAttribute( cvt::xml::LAST_TIME_STEP_ID_TAG, last_time_step );
    root->SetIntAttribute( cvt::xml::CONVERT_PROFILE_OUTPUT_TYPE, output_profile );
    root->SetIntAttribute( cvt::xml::MESH_DEFORMATION_FLAG_TAG, has_mesh_deformation );
    root->SetIntAttribute( cvt::xml::BINARY_FILE_FLAG_TAG, is_binary );

    std::stringstream ss;

    root->PrintXML( ss, vtkIndent() );

    return ss.str();
}

void cvt::ConverterTaskInput::deserialize( const std::string& expression )
{
    auto parser = vtkNew<vtkXMLDataParser>();
    std::istringstream ss( expression );
    parser->SetStream( &ss );
    parser->Parse();

    auto target = parser->GetRootElement();
    target->GetScalarAttribute( cvt::xml::TARGET_INDEX_TAG, target_index );
    target->GetScalarAttribute( cvt::xml::TIME_STEP_ID_TAG, time_step );
    target->GetScalarAttribute( cvt::xml::LAST_TIME_STEP_ID_TAG, last_time_step );
    target->GetScalarAttribute( cvt::xml::CONVERT_PROFILE_OUTPUT_TYPE, output_profile );
    target->GetScalarAttribute( cvt::xml::MESH_DEFORMATION_FLAG_TAG, has_mesh_deformation );
    target->GetScalarAttribute( cvt::xml::BINARY_FILE_FLAG_TAG, is_binary );

    auto input_element = target->FindNestedElementWithName( cvt::xml::INPUT_TAG );
    source_file_paths.resize( input_element->GetNumberOfNestedElements() );
    q.resize( input_element->GetNumberOfNestedElements() );
    f.resize( input_element->GetNumberOfNestedElements() );
    for ( int i = 0; i < input_element->GetNumberOfNestedElements(); ++i )
    {
        auto s = input_element->GetNestedElement( i );
        std::string path = s->GetCharacterData();
        source_file_paths[i] = cvt::Trim( path );

        if ( s->GetAttribute( "q" ) )
        {
            q[i] = s->GetAttribute( "q" );
        }
        if ( s->GetAttribute( "f" ) )
        {
            f[i] = s->GetAttribute( "f" );
        }
    }

    if ( auto grid_type = input_element->GetAttribute( cvt::xml::GRID_TYPE_NAME ) )
    {
        source_grid_type = grid_type;
        cvt::Trim( source_grid_type );
    }
    else
    {
        source_grid_type = "";
    }

    auto output = target->FindNestedElementWithName( cvt::xml::OUTPUT_TAG );

    for ( int o = 0; o < output->GetNumberOfNestedElements(); ++o )
    {
        auto n = output->GetNestedElement( o );
        std::string value = n->GetCharacterData();
        cvt::Trim( value );

        if ( cvt::IsTheXmlTag( n->GetName(), cvt::xml::DIRECTORY_PATH_TAG ) )
        {
            destination_directory = value;
        }
        else if ( cvt::IsTheXmlTag( n->GetName(), cvt::xml::PREFIX_TAG ) )
        {
            destination_prefix = value;
        }
    }
}
