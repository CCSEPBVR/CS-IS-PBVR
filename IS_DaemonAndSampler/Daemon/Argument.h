#ifndef __ARGUMENT_H__
#define __ARGUMENT_H__

#include <climits>
#include <kvs/CommandLine>
#include "CropRegion.h"

class Argument : public kvs::CommandLine
{
public:

    std::string input_data;
    std::string input_data_base;
    //pbvr::TransferFunction transfer_function;
    size_t subpixel_level;
    size_t repeat_level;
    int port;
    float sampling_step;
    bool dump;
    char sampling_method;
    std::string hexahedra_dir;
    std::string quadratic_tetrahedra_dir;
    bool pyramid;
    bool test_volume;
    size_t component_Id;
    bool GT5D;
    bool GT5D_full;
    bool HTTR;
    bool batch;
    bool batch_spec_ts;
    int  batch_ts[2];
    bool batch_fjoin;
    CropRegion crop;
    double latency_threshold;
    std::string param_file;
    int jid_pack_size;
    int particle_limit, particle_limit_pre;
    float density_factor;
    float particle_density;
    std::string output_data_base;
    size_t windowWidth;
    size_t windowHeight;

    size_t normal_ingred;

Argument( int argc, char** argv ):
    kvs::CommandLine( argc, argv )
    {
        // Add help option (generate help message automatically).
        add_help_option();
#ifdef LEGACY_OPTION
        //Add legacy option
        //add_option( "t", "transfer_function", 1, false );
        add_option( "d", "DEBUG:dump result", 0, false );
        add_option( "H", "DEBUG:read Hexahedra data dir", 1, false );
        add_option( "qt", "DEBUG:read Quadratic Tetrahedra data dir", 1, false );
        add_option( "p", "DEBUG:generate pyramid data", 0, false );
        add_option( "V", "DEBUG:test volume calculation", 0, false );
        add_option( "GT5D", "GT5D data", 0, false );
        add_option( "GT5D_full", "GT5D full torus data", 0, false );
        add_option( "HTTR", "HTTR data", 0, false );
        add_option( "ts", "number of time steps", 1, false );
        add_option( "dv", "number of sub-volume division", 1, false );
        add_option( "normal_ingred", "index of ingred for normal vector.", 1, false );
        add_option( "c", "component_Id", 1, false );
        add_option( "step", "sampling_step", 1, false );
        add_option( "sl", "subpixel_level", 1, false );
#endif
        // Add input value.
        add_option( "pa", "parameter file", 1, false );
//      add_option( "rl","repeat_level",1,false );
        add_option( "plimit", "number of particle limit.", 1, false );
        add_option( "vin", "input file name.", 1, false );
        add_option( "pout", "output directory.", 1, false );
//      add_option( "tf","parameter file",1,false );
        add_option( "p", "port number", 1, false );
        add_option( "S", "sampling method", 1, false );
        add_option( "B", "batch mode", 0, false );
        add_option( "L", "latency threshold", 1, false );
        add_option( "jn", "number of jobs a communication", 1, false );
        add_option( "viewer", "Viewer resolution", 1, false );
        add_option( "pd", "particle density", 1, false );

        add_option( "Bs", "start timestep in batch mode", 1, false );
        add_option( "Be", "end timestep in batch mode", 1, false );
        add_option( "Bj", "join subvolumes in batch mode", 0, false );

        if ( !this->parse() ) exit( EXIT_FAILURE );

        subpixel_level = 2;
        repeat_level = 1;
        port = 60000;
        sampling_step = 80; // 0.5f;
        component_Id = 0;
        dump = false;
        sampling_method = 'u';
        hexahedra_dir = "";
        quadratic_tetrahedra_dir = "";
        pyramid = false;
        test_volume = false;
        GT5D = false;
        GT5D_full = false;
        HTTR = false;
        batch = false;
        batch_spec_ts = false;
        batch_ts[0] = INT_MIN;
        batch_ts[1] = INT_MAX;
        batch_fjoin = false;
        latency_threshold = 0.0;
        param_file = "";
        jid_pack_size = 1;
        normal_ingred = 0;
        particle_limit = 10000000;
        particle_limit_pre = -1;
        density_factor = 1.0;
        particle_density = 1.0;
        output_data_base = "./";
        windowWidth  = 620;
        windowHeight = 620;

        //if ( this->hasOption( "t" ) ) transfer_function.read( this->optionValue<std::string>( "t" ) );
        //if( this->hasOption("t") ) ; /* 131017 removed */
        //else transfer_function.create( 256 );
        if ( this->hasOption( "sl" ) ) subpixel_level = this->optionValue<size_t>( "sl" );
        if ( this->hasOption( "rl" ) ) repeat_level = this->optionValue<size_t>( "rl" );
        if ( this->hasOption( "step" ) ) sampling_step = this->optionValue<float>( "step" );
        if ( this->hasOption( "c" ) ) component_Id = this->optionValue<size_t>( "compomentId" );
        if ( this->hasOption( "d" ) ) dump = true;
        if ( this->hasOption( "S" ) ) sampling_method = this->optionValue<char>( "S" );
        if ( this->hasOption( "H" ) ) hexahedra_dir = this->optionValue<std::string>( "H" );
        if ( this->hasOption( "qt" ) ) quadratic_tetrahedra_dir = this->optionValue<std::string>( "qt" );
#ifdef LEGACY_OPTION
        if ( this->hasOption( "p" ) ) pyramid = true;
#endif
        if ( this->hasOption( "V" ) ) test_volume = true;
        if ( this->hasOption( "GT5D" ) ) GT5D = true;
        if ( this->hasOption( "GT5D_full" ) ) GT5D_full = true;
        if ( this->hasOption( "HTTR" ) ) HTTR = true;
        if ( this->hasOption( "B" ) ) batch = true;
        if ( this->hasOption( "p" ) ) port = this->optionValue<size_t>( "p" );
        if ( this->hasOption( "L" ) ) latency_threshold = this->optionValue<double>( "L" );
        if ( this->hasOption( "pa" ) ) param_file = this->optionValue<std::string>( "pa" );
        if ( this->hasOption( "jn" ) ) jid_pack_size = this->optionValue<int>( "jn" );
        if ( this->hasOption( "normal_ingred" ) ) normal_ingred = this->optionValue<size_t>( "normal_ingred" );
        if ( this->hasOption( "plimit" ) ) particle_limit = this->optionValue<int>( "plimit" );
        if ( this->hasOption( "pd" ) ) particle_density = this->optionValue<int>( "pd" );
        if ( this->hasOption( "vin" ) )
        {
            size_t pos;
            std::string input = this->optionValue<std::string>( "vin" );

            if ( ( pos = input.find( ".pfi" ) ) != std::string::npos )
            {
                input_data_base = input.substr( 0, pos );
            }
            else
            {
                input_data_base = input;
            }
        }
        if ( this->hasOption( "pout" ) ) output_data_base = this->optionValue<std::string>( "pout" );
        if ( this->hasOption( "viewer" ) )
        {
            int pos;

            std::string line = this->optionValue<std::string>( "viewer" );

            if ( ( pos = line.find_first_of( "x" ) ) != std::string::npos )
            {
                windowWidth  = static_cast<size_t>( std::atoi( line.substr( 0, pos ).c_str() ) );
                windowHeight = static_cast<size_t>( std::atoi( line.substr( pos + 1 ).c_str() ) );
            }
        }
        if ( this->hasOption( "Bs" ) ) batch_ts[0] = this->optionValue<int>( "Bs" );
        if ( this->hasOption( "Be" ) ) batch_ts[1] = this->optionValue<int>( "Be" );
        if ( this->hasOption( "Bj" ) ) batch_fjoin = true;
        if ( batch_ts[0] > INT_MIN || batch_ts[1] < INT_MAX ) batch_spec_ts = true;

        if ( particle_limit < 0 ) particle_limit = 10;
    }
};

#endif
