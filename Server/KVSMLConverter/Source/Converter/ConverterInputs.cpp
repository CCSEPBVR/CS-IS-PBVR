#include "Converter/ConverterInputs.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>

#include <vtkGlobFileNames.h>
#include <vtkSmartPointer.h>
#include <vtkSortFileNames.h>
#include <vtkStringArray.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

#include <kvs/Message>

#include "Converter/XmlTag.h"
#include "Filesystem.h"
#include "TimeSeriesFiles/SeriesFileResolver.h"

namespace
{

std::string GetGridType( vtkSmartPointer<vtkXMLDataElement> src )
{
    auto t = src->GetAttribute( cvt::xml::GRID_TYPE_NAME );

    if ( t )
    {
        std::string type( t );
        cvt::Trim( type );
        return type;
    }
    else
    {
        return "";
    }
}

void GetStepIds( cvt::ConverterTaskInput& file, vtkXMLDataElement* n )
{
    if ( n->GetAttribute( cvt::xml::TIME_STEP_ID_TAG ) )
    {
        n->GetScalarAttribute( cvt::xml::TIME_STEP_ID_TAG, file.time_step );
    }
    else
    {
        file.time_step = 0;
    }
    if ( n->GetAttribute( cvt::xml::LAST_TIME_STEP_ID_TAG ) )
    {
        n->GetScalarAttribute( cvt::xml::LAST_TIME_STEP_ID_TAG, file.last_time_step );
    }
    else
    {
        file.last_time_step = 0;
    }
}

void GetPlot3dAttributes( cvt::ConverterTaskInput& file, vtkXMLDataElement* n )
{
    if ( n->GetAttribute( "q" ) )
    {
        for ( int i = 0; i < file.q.size(); ++i )
        {
            file.q[i] = n->GetAttribute( "q" );
        }
    }
    if ( n->GetAttribute( "f" ) )
    {
        for ( int i = 0; i < file.f.size(); ++i )
        {
            file.f[i] = n->GetAttribute( "f" );
        }
    }
}

bool AddInputFiles( std::list<cvt::ConverterTaskInput>& sub, int target_index,
                    vtkSmartPointer<vtkXMLDataElement> inputs, int has_mesh_deformation,
                    int is_binary )
{
    bool has_input = false;

    for ( int i = 0; i < inputs->GetNumberOfNestedElements(); ++i )
    {
        auto n = inputs->GetNestedElement( i );

        if ( cvt::IsTheXmlTag( n->GetName(), cvt::xml::SINGLE_TAG ) )
        {
            std::string s = n->GetCharacterData();
            cvt::filesystem::path path( cvt::Trim( s ) );

            if ( cvt::filesystem::exists( path ) )
            {
                cvt::ConverterTaskInput file;

                file.target_index = target_index;
                file.source_file_paths.reserve( 1 );
                file.source_file_paths.push_back( path.u8string() );
                file.q.resize( 1 );
                file.f.resize( 1 );
                ::GetPlot3dAttributes( file, n );
                file.source_grid_type = ::GetGridType( n );
                file.destination_directory = path.parent_path().string();
                file.destination_prefix = path.stem().string();
                ::GetStepIds( file, n );
                file.has_mesh_deformation = has_mesh_deformation;
                file.is_binary = is_binary;

                sub.push_back( file );

                has_input = true;
            }
        }
        else if ( cvt::IsTheXmlTag( n->GetName(), cvt::xml::DISTRIBUTED_TAG ) )
        {
            auto grid_type = ::GetGridType( n );

            cvt::filesystem::path src_dir = ".";
            std::string prefix = "";
            std::string extension = "*";
            std::string file_pattern = "";

            if ( auto m = n->FindNestedElementWithName( cvt::xml::DIRECTORY_PATH_TAG ) )
            {
                std::string s = m->GetCharacterData();
                src_dir = cvt::Trim( s );
            }
            if ( auto m = n->FindNestedElementWithName( cvt::xml::EXTENSION_TAG ) )
            {
                extension = m->GetCharacterData();
                cvt::Trim( extension );
            }
            if ( auto m = n->FindNestedElementWithName( cvt::xml::PREFIX_TAG ) )
            {
                prefix = m->GetCharacterData();
                cvt::Trim( prefix );
            }
            if ( auto m = n->FindNestedElementWithName( cvt::xml::WILDCARD_TAG ) )
            {
                file_pattern = m->GetCharacterData();
                cvt::Trim( file_pattern );

                if ( prefix == "" )
                {
                    auto pos = file_pattern.find_first_of( '*' );
                    if ( pos != std::string::npos )
                    {
                        prefix = file_pattern.substr( 0, pos );
                    }
                }
            }

            // Update a file pattern
            if ( file_pattern == "" )
            {
                file_pattern = prefix + "*." + extension;
            }
            cvt::filesystem::path pattern = src_dir;
            pattern /= file_pattern;
            pattern.make_preferred();
            auto p = pattern.u8string();
            std::replace( p.begin(), p.end(), '\\', '/' );

            vtkNew<vtkGlobFileNames> glob;
            glob->RecurseOff();
            glob->AddFileNames( p.c_str() );

            // Search files
            auto f = glob->GetFileNames();

            vtkNew<vtkSortFileNames> sorter;
            sorter->GroupingOff();
            sorter->NumericSortOn();
            sorter->IgnoreCaseOff();
            sorter->SkipDirectoriesOn();
            sorter->SetInputFileNames( f );

            auto target_files = sorter->GetFileNames();

            if ( target_files->GetNumberOfValues() > 0 )
            {
                cvt::ConverterTaskInput file;

                file.target_index = target_index;
                file.source_file_paths.reserve( target_files->GetNumberOfValues() );
                for ( int t = 0; t < target_files->GetNumberOfValues(); ++t )
                {
                    file.source_file_paths.push_back( target_files->GetValue( t ) );
                }
                file.q.resize( target_files->GetNumberOfValues() );
                file.f.resize( target_files->GetNumberOfValues() );
                ::GetPlot3dAttributes( file, n );
                file.source_grid_type = grid_type;
                file.destination_directory = src_dir.string();
                if ( prefix == "" )
                {
                    cvt::filesystem::path p = std::string( target_files->GetValue( 0 ) );
                    file.destination_prefix = p.stem().string();
                    kvsMessageWarning( ( std::string( "The output prefix is set to " ) +
                                         file.destination_prefix +
                                         ". Sometimes, this will generate unexpected files." )
                                           .c_str() );
                }
                else
                {
                    file.destination_prefix = prefix;
                }
                ::GetStepIds( file, n );
                file.has_mesh_deformation = has_mesh_deformation;
                file.is_binary = is_binary;

                sub.push_back( file );

                has_input = true;
            }
        }
        else if ( cvt::IsTheXmlTag( n->GetName(), "serial" ) )
        {
            auto grid_type = ::GetGridType( n );

            cvt::filesystem::path src_dir = ".";
            std::string prefix = "";
            std::string extension = "*";
            std::string file_pattern = "";

            if ( auto m = n->FindNestedElementWithName( cvt::xml::DIRECTORY_PATH_TAG ) )
            {
                std::string s = m->GetCharacterData();
                src_dir = cvt::Trim( s );
            }
            if ( auto m = n->FindNestedElementWithName( cvt::xml::EXTENSION_TAG ) )
            {
                extension = m->GetCharacterData();
                cvt::Trim( extension );
            }
            if ( auto m = n->FindNestedElementWithName( cvt::xml::PREFIX_TAG ) )
            {
                prefix = m->GetCharacterData();
                cvt::Trim( prefix );
            }
            if ( auto m = n->FindNestedElementWithName( cvt::xml::WILDCARD_TAG ) )
            {
                file_pattern = m->GetCharacterData();
                cvt::Trim( file_pattern );
            }

            // Update a file pattern
            if ( file_pattern == "" )
            {
                file_pattern = prefix + "*." + extension;
            }
            cvt::filesystem::path pattern = src_dir;
            pattern /= file_pattern;
            pattern.make_preferred();
            auto p = pattern.u8string();
            std::replace( p.begin(), p.end(), '\\', '/' );

            cvt::ResolvedSeries series;
            std::string series_error;
            if ( !cvt::ResolveSeries( p, series, series_error ) )
            {
                kvsMessageError( "%s", series_error.c_str() );
            }
            else
            {
                if ( prefix.empty() ) prefix = series.output_base;
                const auto& target_files = series.file_paths;
                for ( std::size_t t = 0; t < target_files.size(); ++t )
                {
                    cvt::ConverterTaskInput file;

                    file.target_index = target_index;
                    file.source_file_paths.reserve( 1 );
                    file.source_file_paths.push_back( target_files[t] );
                    file.q.resize( 1 );
                    file.f.resize( 1 );
                    ::GetPlot3dAttributes( file, n );
                    file.source_grid_type = grid_type;
                    file.destination_directory = src_dir.string();
                    file.destination_prefix = prefix;
                    file.time_step = static_cast<int>( t );
                    file.last_time_step = static_cast<int>( target_files.size() ) - 1;
                    file.has_mesh_deformation = has_mesh_deformation;
                    file.is_binary = is_binary;

                    sub.push_back( file );

                    has_input = true;
                }
            }
        }
    }

    return has_input;
}

void OverwriteOutputConfiguration( std::list<cvt::ConverterTaskInput>& sub,
                                   vtkSmartPointer<vtkXMLDataElement> output )
{
    for ( int i = 0; i < output->GetNumberOfNestedElements(); ++i )
    {
        auto n = output->GetNestedElement( i );

        if ( cvt::IsTheXmlTag( n->GetName(), cvt::xml::DIRECTORY_PATH_TAG ) )
        {
            std::string path = n->GetCharacterData();
            cvt::Trim( path );

            for ( auto& s : sub )
            {
                s.destination_directory = path;
            }
        }
        else if ( cvt::IsTheXmlTag( n->GetName(), cvt::xml::PREFIX_TAG ) )
        {
            std::string prefix = n->GetCharacterData();
            cvt::Trim( prefix );

            for ( auto& s : sub )
            {
                s.destination_prefix = prefix;
            }
        }
    }
}

int GetVersion( vtkSmartPointer<vtkXMLDataElement> root )
{
    int version = -1;
    if ( root->GetAttribute( cvt::xml::VERSION_TAG ) )
    {
        root->GetScalarAttribute( cvt::xml::VERSION_TAG, version );
    }

    return version;
}
} // namespace

std::optional<cvt::filesystem::path> cvt::CheckConfigFile( const char* arg ) noexcept
{
    try
    {
        cvt::filesystem::path f = arg;
        f.make_preferred();

        if ( cvt::filesystem::exists( f ) && f.extension() == ".xml" )
        {
            return f;
        }
        else
        {
            return std::nullopt;
        }
    }
    catch ( ... )
    {
        return std::nullopt;
    }
}

std::list<cvt::ConverterTaskInput> cvt::ListInputFilesFromConfigFile( const char* xml_file_path )
{
    std::list<cvt::ConverterTaskInput> file_list;

    auto parser = vtkNew<vtkXMLDataParser>();
    parser->SetFileName( xml_file_path );
    parser->Parse();

    auto root = parser->GetRootElement();

    // Check version
    if ( ::GetVersion( root ) != 0 )
    {
        return file_list;
    }

    for ( int g = 0; g < root->GetNumberOfNestedElements(); ++g )
    {
        auto target = root->GetNestedElement( g );

        if ( cvt::IsTheXmlTag( target->GetName(), cvt::xml::TARGET_TAG ) )
        {
            std::list<cvt::ConverterTaskInput> sub_file_list;
            int input_index = 0;
            int has_mesh_deformation = 1;
            int is_binary = 1;

            auto input = target->FindNestedElementWithName( cvt::xml::INPUT_TAG );
            if ( input )
            {
                if ( auto f = input->GetAttribute( cvt::xml::MESH_DEFORMATION_FLAG_TAG ) )
                {
                    has_mesh_deformation = atoi( f );
                }
                if ( auto f = input->GetAttribute( cvt::xml::BINARY_FILE_FLAG_TAG ) )
                {
                    is_binary = atoi( f );
                }
            }
            if ( input &&
                 ::AddInputFiles( sub_file_list, g, input, has_mesh_deformation, is_binary ) )
            {
                ++input_index;
            }

            if ( input_index > 0 )
            {
                if ( auto output = target->FindNestedElementWithName( cvt::xml::OUTPUT_TAG ) )
                {
                    ::OverwriteOutputConfiguration( sub_file_list, output );
                }

                if ( auto config = target->FindNestedElementWithName( cvt::xml::CONFIG_TYPE ) )
                {
                    if ( auto output_flag_element = config->FindNestedElementWithName(
                             cvt::xml::CONVERT_PROFILE_OUTPUT_TYPE ) )
                    {
                        std::string x = output_flag_element->GetCharacterData();
                        try
                        {
                            for ( auto& i : sub_file_list )
                            {
                                i.output_profile = std::atoi( x.c_str() ) > 0;
                            }
                        }
                        catch ( ... )
                        {
                        }
                    }
                }

                file_list.splice( file_list.end(), sub_file_list );
            }
        }
    }

    return file_list;
}
