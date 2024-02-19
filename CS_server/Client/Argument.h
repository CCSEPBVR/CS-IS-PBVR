#ifndef PBVR__KVS_VISCLIENT_ARGUMENT_H_INCLUDE
#define PBVR__KVS_VISCLIENT_ARGUMENT_H_INCLUDE

#include "kvs/CommandLine"
#include "kvs/TransferFunction"
#include <kvs/Directory>
#include <kvs/File>

namespace kvs
{
namespace visclient
{

class Argument : public kvs::CommandLine
{
private:
    bool m_viewer_flag;
    std::string m_pout_path_name; // APPEND Fj 2015.04.21

public:
    std::string m_header;
    kvs::TransferFunction m_transfer_function;
    size_t m_subpixel_level_abstract;
    size_t m_subpixel_level_detailed;
    size_t m_repeat_level_abstract;
    size_t m_repeat_level_detailed;
    std::string m_hostname;
    size_t m_port;
// APPEND START BY)M.Tanaka 2015.03.03
    size_t m_particle_limit;
    size_t m_particle_density;
    std::string m_parameter_file;
// APPEND START BY)M.Tanaka 2015.03.11
    std::string m_sampling_type;
    std::string m_detailed_particle_transfer_type;
    std::string m_shading_type;
    bool m_sampling_type_f;
    bool m_detailed_particle_transfer_type_flag;
    bool m_shading_type_flag;
// APPEND END   BY)M.Tanaka 2015.03.11
    std::string m_viewer;
// APPEND END   BY)M.Tanaka 2015.03.03
    std::string m_server_subvolume_directory;

// APPEND START BY)M.Tanaka 2015.03.05
    bool m_subpixel_level_detailed_flag;
    bool m_repeat_level_detailed_flag;
    bool m_particle_limit_flag;
    bool m_particle_density_flag;
    bool m_transfer_function_file_flag; // APPEND Fj 2015.03.06
    std::string m_transfer_function_file_name;
// APPEND END   BY)M.Tanaka 2015.03.05
    std::string m_image_base_name;
    std::string m_pin_option_directory[10];
    // add by @hira at 2016/12/01
    std::string m_filter_parameter_filename;
    bool m_hasfin;

    Argument( const int argc, char** argv ):
        kvs::CommandLine( argc, argv )
    {
        // Add help option (generate help message automatically).
        add_help_option();
//      add_value( "If \"s\" is specified, it starts in standalone mode.", false );
        add_option( "tf", "transfer_function", 1, false );
// MODIFIED START BY)M.Tanaka 2015.03.03
//		add_option( "sl","subpixel_level",1,false);
//		add_option( "rl","repeat_level",1, false);
// APPEND START BY)M.Tanaka 2015.03.11
        add_option( "S", "sampling type", 1, false );
        add_option( "tdata", "detailed particle transfer type", 1, false );
        add_option( "shading", "shading", 1, false );
// APPEND END   BY)M.Tanaka 2015.03.11
        add_option( "viewer", "resolution wxh", 1, false );
        add_option( "plimit", "particle_limit", 1, false );
        add_option( "pd", "particle_density", 1, false );
        add_option( "hn", "hostname", 1, false );
        add_option( "p", "port", 1, false );
        add_option( "pa", "parameter file", 1, false );
        add_option( "vin", "server subvolume directory", 1, false );
// MODIFIED END   BY)M.Tanaka 2015.03.03
        add_option( "pout", "particle file output directory.", 1, false ); // // APPEND By Fj 2015.04.21
        add_option( "iout", "image file output directory.", 1, false );
        add_option( "pin1", "particle directory No.1", 1, false );
        add_option( "pin2", "particle directory No.2", 1, false );
        add_option( "pin3", "particle directory No.3", 1, false );
        add_option( "pin4", "particle directory No.4", 1, false );
        add_option( "pin5", "particle directory No.5", 1, false );
        add_option( "pin6", "particle directory No.6", 1, false );
        add_option( "pin7", "particle directory No.7", 1, false );
        add_option( "pin8", "particle directory No.8", 1, false );
        add_option( "pin9", "particle directory No.9", 1, false );
        add_option( "pin10", "particle directory No.10", 1, false );
        // add by @hira at 2016/12/01
        add_option( "fin", "filter parameter filename", 1, false );

        if ( !this->parse() ) exit( EXIT_FAILURE );

        m_subpixel_level_abstract = 2;
        m_repeat_level_abstract = 1;
        m_subpixel_level_detailed = 2; /* 2 */
        m_repeat_level_detailed = 1; /* 16 */
        m_hostname = "localhost";
        m_port = 60000;
        m_server_subvolume_directory = "";
// APPEND START BY)M.Tanaka 2015.03.11
        m_sampling_type = "u";
        m_sampling_type_f = false;
        m_detailed_particle_transfer_type = "all";
        m_detailed_particle_transfer_type_flag = false;
        m_shading_type = "";
        m_shading_type_flag = false;
// APPEND END   BY)M.Tanaka 2015.03.11
        m_subpixel_level_detailed_flag = false;
        m_repeat_level_detailed_flag = false;
        m_particle_limit_flag = false;
        m_particle_density_flag = false;
        m_viewer_flag = false;
        m_transfer_function_file_flag = false;
        // add by @hira at 2016/12/01
        m_filter_parameter_filename  = "";
        m_hasfin = false;

        if ( this->hasOption( "tf" ) )
        {
            m_transfer_function_file_flag = true;
            m_transfer_function_file_name = this->optionValue<std::string>( "tf" );
        }
        else
        {
            m_transfer_function.create( 256 );//m_transfer_function.read( "defalt.kvsml" );
        }

// MODIFIED START BY)M.Tanaka 2015.03.03
        if ( this->hasOption( "sl" ) )
        {
            m_subpixel_level_detailed = this->optionValue<size_t>( "sl" );
            m_subpixel_level_detailed_flag = true;
        }
        if ( this->hasOption( "rl" ) )
        {
            m_repeat_level_detailed = this->optionValue<size_t>( "rl" );
            m_repeat_level_detailed_flag = true;
        }
// APPEND START BY)M.Tanaka 2015.03.11
        if ( this->hasOption( "S" ) )
        {
            m_sampling_type = this->optionValue<std::string>( "S" );
            m_sampling_type_f = true;
        }
        if ( this->hasOption( "tdata" ) )
        {
            m_detailed_particle_transfer_type = this->optionValue<std::string>( "tdata" );
            m_detailed_particle_transfer_type_flag = true;
        }
        if ( this->hasOption( "shading" ) )
        {
            m_shading_type = this->optionValue<std::string>( "shading" );
            m_shading_type_flag = true;
        }
// APPEND END   BY)M.Tanaka 2015.03.11
        if ( this->hasOption( "m_viewer" ) )
        {
            m_viewer = this->optionValue<std::string>( "m_viewer" );
            m_viewer_flag = true;
        }
        if ( this->hasOption( "plimit" ) )
        {
            m_particle_limit = this->optionValue<size_t>( "plimit" );
            m_particle_limit_flag = true;
        }
        if ( this->hasOption( "pd" ) )
        {
            m_particle_density = this->optionValue<size_t>( "pd" );
            m_particle_density_flag = true;
        }
        if ( this->hasOption( "hn" ) ) m_hostname = this->optionValue<std::string>( "hn" );
        if ( this->hasOption( "p" ) ) m_port = this->optionValue<size_t>( "p" );
        if ( this->hasOption( "pa" ) ) m_parameter_file = this->optionValue<std::string>( "pa" );
        if ( this->hasOption( "vin" ) )
        {
#if 0
            std::string tmpstring = this->optionValue<std::string>( "vin" );
            // erase ".pfi"
            if ( tmpstring.find( ".pfi" ) != std::string::npos )
            {
                printf( "find pfi" );
                tmpstring.erase( tmpstring.find( ".pfi" ) );
                m_server_subvolume_directory = tmpstring;
            }
            else
            {
                m_server_subvolume_directory = this->optionValue<std::string>( "vin" );
            }
#else
			m_server_subvolume_directory = this->optionValue<std::string>("vin");
#endif
            m_hasfin = false;
        }
// MODIFIED END   BY)M.Tanaka 2015.03.03

        // add by @hira at 2016/12/01
        if ( this->hasOption( "fin" ) ) {
            m_filter_parameter_filename = this->optionValue<std::string>( "fin" );
            m_hasfin = true;
        }

        if ( this->hasOption( "pout" ) )
        {
            m_pout_path_name = "PARTICLE_OUTDIR=";
            m_pout_path_name.append( this->optionValue<std::string>( "pout" ) );
        }
        else
        {
            m_pout_path_name = "PARTICLE_OUTDIR=";
            m_pout_path_name.append( "" );
        }
        if ( this->hasOption( "iout" ) )
        {
            m_image_base_name = this->optionValue<std::string>( "iout" );
        }
        else
        {
            m_image_base_name = "PBVR_image";
        }
#ifdef WIN32
        _putenv( m_pout_path_name.c_str() );
#else
        putenv( ( char* )m_pout_path_name.c_str() );
#endif
//      m_header = this->value<std::string>();
        int client_mode = 0;
        // mofify by @hira at 2016/12/01
        // if ( this->hasOption( "vin" ))
        if ( this->hasOption( "vin" ) || this->hasOption( "fin" ))
        {
            m_header = "";
            client_mode = 1;
        }
        else
        {
            m_header = "s";
        }

        // 初期化
        for ( int i = 0; i < 10; i++ )
        {
            m_pin_option_directory[i].clear();
        }
        // 統合粒子設定
        for ( int i = 0; i < 10; i++ )
        {
            char buf[64];
            sprintf( buf, "pin%d", i + 1 );
            if ( this->hasOption( buf ) )
            {
                kvs::File file_prefix( this->optionValue<std::string>( buf ) );
                kvs::Directory directory( file_prefix.pathName() );
                if ( !directory.isDirectory() )
                {
                    continue;
                }
                kvs::FileList::iterator bfile = directory.fileList().begin();
                kvs::FileList::iterator end = directory.fileList().end();
                int kvsmlFileNum = 0;

                while ( bfile != end )
                {
                    kvs::File f( bfile->fileName() );

                    if ( f.extension() == "kvsml" )
                    {
                        kvsmlFileNum++;
                    }
                    ++bfile;
                }

                if ( kvsmlFileNum == 0 )
                {
                    continue;
                }

                m_pin_option_directory[i].append( this->optionValue<std::string>( buf ) );
                client_mode = 2;
            }
        }
        /*
        if ( client_mode == 0 )
        {
            fprintf( stderr, "usage: %s -vin file_path [option]\n", argv[0] );
            fprintf( stderr, "         or\n" );
            fprintf( stderr, "       %s -pin* file_path [option]\n", argv[0] );
            exit( EXIT_FAILURE );
        }
        */
    }
};

}
}

#endif
