#include <tuple>
#include <unordered_map>

#include "Converter/ConverterInputs.h"
#include "Converter/ConverterTask.h"
#include "Converter/ConverterTaskInput.h"
#include "Converter/ConverterTaskOutput.h"
#include "PBVRFileInformation/Pfl.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

#include "Filesystem.h"

namespace
{

int MainImpl( std::filesystem::path input_xml_path )
{
    auto targets = cvt::ListInputFilesFromConfigFile( input_xml_path.u8string().c_str() );
    std::unordered_map<int, std::list<cvt::ConverterTaskOutput>> outputs;
    std::unordered_map<int, cvt::ConverterTaskInput> input_table;

    for ( auto& target : targets )
    {
        input_table.emplace( target.target_index, target );
        kvsMessage( ( std::string( "The main process " ) + " starts a convert task including " +
                      target.source_file_paths[0] )
                        .c_str() );
        auto output = cvt::Convert( target );
        if ( output )
        {
            outputs[target.target_index].push_back( *output );
        }
        kvsMessage( std::string( "The main process ends a convert task" ).c_str() );
    }

    for ( auto& o : outputs )
    {
        auto& l = o.second;
        if ( l.size() != 0 )
        {
            l.sort( []( auto& o0, auto& o1 ) { return o0.time_step < o1.time_step; } );
            std::unordered_map<std::string, cvt::UnstructuredPfi> pfi_map;
            int last_time_step = l.size() - 1;
            int number_of_components =
                std::get<0>( l.begin()->value_minmax.begin()->second ).size();
            for ( auto& k : l.begin()->sub_volume_counts )
            {
                pfi_map.emplace( k.first, cvt::UnstructuredPfi( number_of_components,
                                                                last_time_step, k.second ) );
            }
            for ( auto& o : l )
            {
                for ( auto& k : o.sub_volume_counts )
                {
                    pfi_map.at( k.first ).registerObject( &o, k.first );
                }
            }

            cvt::Pfl pfl;
            for ( auto& pfi : pfi_map )
            {
                pfi.second.write( input_table.at( o.first ).destination_directory, pfi.first );
                pfl.registerPfi( input_table.at( o.first ).destination_directory, pfi.first );
            }
            pfl.write( input_table.at( o.first ).destination_directory,
                       input_table.at( o.first ).destination_prefix );
            kvsMessage( ( std::string( "The main process outputs PFI and PFL to " ) +
                          input_table.at( o.first ).destination_directory )
                            .c_str() );
        }
    }

    return EXIT_SUCCESS;
}
} // namespace

int main( int argc, char* argv[] )
{
    std::optional<std::filesystem::path> input;

    if ( argc >= 2 && ( input = cvt::CheckConfigFile( argv[1] ) ) )
    {
        return ::MainImpl( *input );
    }
    else
    {
        std::cout << "A file converter to a KVSML file" << std::endl;
        std::cout << "USAGE" << std::endl;
        std::cout << "  mpirun -np N kvsml-converter-mpi <XML_PATH>" << std::endl;
        return EXIT_FAILURE;
    }
}