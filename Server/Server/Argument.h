#ifndef PBVR__ARGUMENT_H_INCLUDE
#define PBVR__ARGUMENT_H_INCLUDE

#include <climits>
#include <kvs/CommandLine>
#include "CropRegion.h"

#include "TransferFunctionSynthesizer.h"

class Argument : public kvs::CommandLine
{
private:
    bool m_dump;
    std::string m_hexahedra_directory;
    std::string m_quadratic_tetrahedra_directory;
    bool m_test_volume;
    float m_density_factor;

public:
    std::string m_input_data;
    std::string m_input_data_base;
    std::string m_filter_parameter_filename;         // add by @hira at 2016/12/01
    pbvr::TransferFunction m_transfer_function;
    TransferFunctionSynthesizer* m_transfunc_synthesizer;
    size_t m_subpixel_level;
    size_t m_repeat_level;
    int m_port;
    float m_sampling_step;
    char m_sampling_method;
    bool m_pyramid;
    size_t m_component_Id;
    bool m_gt5d;
    bool m_gt5d_full;
    bool m_httr;
    bool m_batch;
    bool m_batch_spec_time_step;
    int  m_batch_time_step[2];
    bool m_batch_join_flag;
    CropRegion m_crop;
    double m_latency_threshold;
    std::string m_parameter_file;
    int m_job_id_pack_size;
    int m_particle_limit, m_particle_limit_pre;
    float m_particle_density;
    std::string m_output_data_base;
    size_t m_window_width;
    size_t m_window_height;
    std::string m_x_synthesis;
    std::string m_y_synthesis;
    std::string m_z_synthesis;
    size_t m_normal_ingredient;
    int m_rank;

public:
    Argument( const int argc, char** argv ):
        kvs::CommandLine( argc, argv )
    {
        // Add help option (generate help message automatically).
        add_help_option();
#ifdef LEGACY_OPTION
        //Add legacy option
        add_option( "t", "transfer_function", 1, false );
        add_option( "d", "DEBUG:dump result", 0, false );
        add_option( "H", "DEBUG:read Hexahedra data dir", 1, false );
        add_option( "qt", "DEBUG:read Quadratic Tetrahedra data dir", 1, false );
        add_option( "p", "DEBUG:generate pyramid data", 0, false );
        add_option( "V", "DEBUG:test volume calculation", 0, false );
        add_option( "GT5D", "GT5D data", 0, false );
        add_option( "gt5d_full", "GT5D full torus data", 0, false );
        add_option( "HTTR", "HTTR data", 0, false );
        add_option( "ts", "number of time steps", 1, false );
        add_option( "dv", "number of sub-volume division", 1, false );
        add_option( "normal_ingredient", "index of ingred for normal vector.", 1, false );
        add_option( "c", "component_Id", 1, false );
        add_option( "step", "mampling_step", 1, false );
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
        add_option( "Bd", "dont join subvolumes in batch mode", 0, false );
        add_option( "fin", "input pbvr filter parameter file name.", 1, false );

        if ( !this->parse() ) exit( EXIT_FAILURE );

        m_subpixel_level = 2;
        m_repeat_level = 1;
        m_port = 60000;
        m_sampling_step = 80; // 0.5f;
        m_component_Id = 0;
        m_dump = false;
        m_sampling_method = 'u';
        m_hexahedra_directory = "";
        m_quadratic_tetrahedra_directory = "";
        m_pyramid = false;
        m_test_volume = false;
        m_gt5d = false;
        m_gt5d_full = false;
        m_httr = false;
        m_batch = false;
        m_batch_spec_time_step = false;
        m_batch_time_step[0] = INT_MIN;
        m_batch_time_step[1] = INT_MAX;
        m_batch_join_flag = true;
        m_latency_threshold = 0.0;
        m_parameter_file = "";
        m_job_id_pack_size = 1;
        m_normal_ingredient = 0;
        m_particle_limit = 10000000;
        m_particle_limit_pre = -1;
        m_density_factor = 1.0;
        m_particle_density = 1.0;
        m_output_data_base = "./";
        m_window_width  = 620;
        m_window_height = 620;
        m_rank = 0;
        m_input_data_base.clear();
        m_filter_parameter_filename.clear();

        if ( this->hasOption( "t" ) ) m_transfer_function.read( this->optionValue<std::string>( "t" ) );
        //if( this->hasOption("t") ) ; /* 131017 removed */
        else m_transfer_function.create( 256 );
        if ( this->hasOption( "sl" ) ) m_subpixel_level = this->optionValue<size_t>( "sl" );
        if ( this->hasOption( "rl" ) ) m_repeat_level = this->optionValue<size_t>( "rl" );
        if ( this->hasOption( "step" ) ) m_sampling_step = this->optionValue<float>( "step" );
        if ( this->hasOption( "c" ) ) m_component_Id = this->optionValue<size_t>( "compomentId" );
        if ( this->hasOption( "d" ) ) m_dump = true;
        if ( this->hasOption( "S" ) ) m_sampling_method = this->optionValue<char>( "S" );
        if ( this->hasOption( "H" ) ) m_hexahedra_directory = this->optionValue<std::string>( "H" );
        if ( this->hasOption( "qt" ) ) m_quadratic_tetrahedra_directory = this->optionValue<std::string>( "qt" );
#ifdef LEGACY_OPTION
        if ( this->hasOption( "p" ) ) pyramid = true;
#endif
        if ( this->hasOption( "V" ) ) m_test_volume = true;
        if ( this->hasOption( "GT5D" ) ) m_gt5d = true;
        if ( this->hasOption( "m_gt5d_full" ) ) m_gt5d_full = true;
        if ( this->hasOption( "HTTR" ) ) m_httr = true;
        if ( this->hasOption( "B" ) ) m_batch = true;
        if ( this->hasOption( "p" ) ) m_port = this->optionValue<size_t>( "p" );
        if ( this->hasOption( "L" ) ) m_latency_threshold = this->optionValue<double>( "L" );
        if ( this->hasOption( "pa" ) ) m_parameter_file = this->optionValue<std::string>( "pa" );
        if ( this->hasOption( "jn" ) ) m_job_id_pack_size = this->optionValue<int>( "jn" );
        if ( this->hasOption( "m_normal_ingredient" ) ) m_normal_ingredient = this->optionValue<size_t>( "m_normal_ingredient" );
        if ( this->hasOption( "plimit" ) ) m_particle_limit = this->optionValue<int>( "plimit" );
        if ( this->hasOption( "pd" ) ) m_particle_density = this->optionValue<int>( "pd" );
        // add by @hira at 2016/12/01
        if ( this->hasOption( "fin" ) )
        {
            std::string param = this->optionValue<std::string>( "fin" );
            m_filter_parameter_filename = param;
        }
        else if ( this->hasOption( "vin" ) )
        {
            size_t pos;
            std::string input = this->optionValue<std::string>( "vin" );

#if 0
            if ( ( pos = input.find( ".pfi" ) ) != std::string::npos )
            {
                m_input_data_base = input.substr( 0, pos );
            }
            else
            {
                m_input_data_base = input;
            }
#else
			m_input_data_base = input;
#endif
        }
        if ( this->hasOption( "pout" ) ) m_output_data_base = this->optionValue<std::string>( "pout" );
        if ( this->hasOption( "viewer" ) )
        {
            int pos;

            std::string line = this->optionValue<std::string>( "viewer" );

            if ( ( pos = line.find_first_of( "x" ) ) != std::string::npos )
            {
                m_window_width  = static_cast<size_t>( std::atoi( line.substr( 0, pos ).c_str() ) );
                m_window_height = static_cast<size_t>( std::atoi( line.substr( pos + 1 ).c_str() ) );
            }
        }
        if ( this->hasOption( "Bs" ) ) m_batch_time_step[0] = this->optionValue<int>( "Bs" );
        if ( this->hasOption( "Be" ) ) m_batch_time_step[1] = this->optionValue<int>( "Be" );
        if ( this->hasOption( "Bd" ) ) m_batch_join_flag = false;
        if ( m_batch_time_step[0] > INT_MIN || m_batch_time_step[1] < INT_MAX ) m_batch_spec_time_step = true;

        if ( m_particle_limit < 0 ) m_particle_limit = 10;
        m_transfunc_synthesizer = NULL;
    }
};

#endif  // PBVR__ARGUMENT_H_INCLUDE

