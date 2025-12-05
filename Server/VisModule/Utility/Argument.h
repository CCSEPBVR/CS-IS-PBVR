#ifndef VIS_MODULE__ARGUMENT_H_INCLUDE
#define VIS_MODULE__ARGUMENT_H_INCLUDE

#include <climits>
#include <vismodule/CommandLine>
//#include <vismodule/CropRegion>

#include <vismodule/TransferFunctionSynthesizer>
//#include <vismodule/ExtendedTransferFunction>
#include <vismodule/ExtendedTransferFunctionParameter>
#include <vismodule/TransferFunction>

class Argument : public vismodule::CommandLine
{
private:
    bool m_dump;
    float m_density_factor;

public:
    std::string m_input_data;
    std::string m_input_data_base;
    std::string m_color_transfer_function_synthesis;
    std::string m_opacity_transfer_function_synthesis;
    vismodule::TransferFunction m_transfer_function;
    std::vector<NamedTransferFunctionParameter> m_named_transfunc_array; // use server message
    std::vector<vismodule::TransferFunction> m_transfunc_array; // use CellByCellxxxSampling
    TransferFunctionSynthesizer* m_transfunc_synthesizer;
    int m_time_step;
    size_t m_subpixel_level;
    size_t m_repeat_level;
    int m_level_index;
    int m_port;
    float m_sampling_step;
    char m_sampling_method;
    double m_latency_threshold;
    std::string m_parameter_file;
    int m_job_id_pack_size;
    int m_particle_limit, m_particle_limit_pre;
    float m_particle_density;
    float m_particle_data_size_limit;
    std::string m_output_data_base;
    vismodule::Camera* m_camera;
    size_t m_window_width;
    size_t m_window_height;
    std::string m_x_synthesis;
    std::string m_y_synthesis;
    std::string m_z_synthesis;

    int m_subvolume_id;

    class VolumeEquation
    {
    public:
        std::string m_name;
        std::string m_equation;
    };

    std::vector<VolumeEquation>        m_voleqn;
    size_t m_normal_ingredient;

    VariableRange m_server_side_variable_range;

    // glyph paramter
    bool m_glyph_flag;
    int m_stride;
    int m_seed;
    float m_number_of_sampling_point;
    float m_glyph_color_min;
    float m_glyph_color_max;
    float m_glyph_size_min;
    float m_glyph_size_max;
    std::vector<int32_t> m_glyph_color_map_table;
    vismodule::ColorMap m_color_map;
    std::string m_direction_variable[3];
    // jpv::DataDefines m_size_sampling_method; // 多地点対応のため一旦コメントアウト
    std::vector<std::string> m_size_variable;
    // jpv::GlyphMode m_distribution_mode; // 多地点対応のため一旦コメントアウト
    // jpv::DataDefines m_color_data_sampling_method; // 多地点対応のため一旦コメントアウト
    std::vector<std::string> m_color_data_variable;
    // glyph parameter

    // plot over line parameter
    bool m_plot_flag;
    std::string m_plot_variable;
    float m_start_point[3];
    float m_end_point[3];    
    int32_t m_sampling_size; // resolution
    // plot over line parameter

public:
    Argument( const int argc, char** argv ):
        vismodule::CommandLine( argc, argv )
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

        if ( !this->parse() ) exit( EXIT_FAILURE );

        m_time_step = 0;
        m_subpixel_level = 2;
        m_repeat_level = 1;
        m_level_index = 0;
        m_port = 60000;
        m_sampling_step = 80; // 0.5f;
        m_dump = false;
        m_sampling_method = 'u';
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
        m_subvolume_id = 1;

        if ( this->hasOption( "t" ) ) m_transfer_function.read( this->optionValue<std::string>( "t" ) );
        //if( this->hasOption("t") ) ; /* 131017 removed */
        else m_transfer_function.create( 256 );
        if ( this->hasOption( "sl" ) ) m_subpixel_level = this->optionValue<size_t>( "sl" );
        if ( this->hasOption( "rl" ) ) m_repeat_level = this->optionValue<size_t>( "rl" );
        if ( this->hasOption( "step" ) ) m_sampling_step = this->optionValue<float>( "step" );
        if ( this->hasOption( "d" ) ) m_dump = true;
        if ( this->hasOption( "S" ) ) m_sampling_method = this->optionValue<char>( "S" );
        if ( this->hasOption( "p" ) ) m_port = this->optionValue<size_t>( "p" );
        if ( this->hasOption( "L" ) ) m_latency_threshold = this->optionValue<double>( "L" );
        if ( this->hasOption( "pa" ) ) m_parameter_file = this->optionValue<std::string>( "pa" );
        if ( this->hasOption( "jn" ) ) m_job_id_pack_size = this->optionValue<int>( "jn" );
        if ( this->hasOption( "m_normal_ingredient" ) ) m_normal_ingredient = this->optionValue<size_t>( "m_normal_ingredient" );
        if ( this->hasOption( "plimit" ) ) m_particle_limit = this->optionValue<int>( "plimit" );
        if ( this->hasOption( "pd" ) ) m_particle_density = this->optionValue<int>( "pd" );
        if ( this->hasOption( "vin" ) )
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
        if ( m_particle_limit < 0 ) m_particle_limit = 10;
        m_transfunc_synthesizer = NULL;
    }
};

#endif
