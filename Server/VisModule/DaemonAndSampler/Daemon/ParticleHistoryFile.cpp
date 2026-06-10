#include "ParticleHistoryFile.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../../../Shared/json.hpp"

ParticleHistoryFile::ParticleHistoryFile()
{
    this->set_name();
    m_has_ensemble_statistic_histogram = false;
    m_nvariables           = 0;
    m_particle_limit       = 0;
    m_extra_opacity_factor = 0;
}

ParticleHistoryFile::ParticleHistoryFile( const std::string& file ):
    m_file_name( file )
{
    this->set_name();
    m_has_ensemble_statistic_histogram = false;
    m_nvariables           = 0;
    m_particle_limit       = 0;
    m_extra_opacity_factor = 0;
}

void ParticleHistoryFile::set_name()
{
#if 0
    m_name.push_back( "MIN_O1" );
    m_name.push_back( "MAX_O1" );
    m_name.push_back( "MIN_C1" );
    m_name.push_back( "MAX_C1" );

    m_name.push_back( "MIN_O2" );
    m_name.push_back( "MAX_O2" );
    m_name.push_back( "MIN_C2" );
    m_name.push_back( "MAX_C2" );

    m_name.push_back( "MIN_O3" );
    m_name.push_back( "MAX_O3" );
    m_name.push_back( "MIN_C3" );
    m_name.push_back( "MAX_C3" );

    m_name.push_back( "MIN_O4" );
    m_name.push_back( "MAX_O4" );
    m_name.push_back( "MIN_C4" );
    m_name.push_back( "MAX_C4" );

    m_name.push_back( "MIN_O5" );
    m_name.push_back( "MAX_O5" );
    m_name.push_back( "MIN_C5" );
    m_name.push_back( "MAX_C5" );
   
    m_name.push_back( "HISTOGRAM_C1" );
    m_name.push_back( "HISTOGRAM_C2" );
    m_name.push_back( "HISTOGRAM_C3" );
    m_name.push_back( "HISTOGRAM_C4" );
    m_name.push_back( "HISTOGRAM_C5" );

    m_name.push_back( "HISTOGRAM_O1" );
    m_name.push_back( "HISTOGRAM_O2" );
    m_name.push_back( "HISTOGRAM_O3" );
    m_name.push_back( "HISTOGRAM_O4" );
    m_name.push_back( "HISTOGRAM_O5" );
#endif

    m_name.push_back( "TF_NUMBER" );
    for (int i = 0; i < 99; i++) {
        std::stringstream ss;
        ss << i + 1;
        m_name.push_back( "MIN_O" + ss.str() );
        m_name.push_back( "MIN_C" + ss.str() );
        m_name.push_back( "MAX_O" + ss.str() );
        m_name.push_back( "MAX_C" + ss.str() );
        m_name.push_back( "HISTOGRAM_C" + ss.str() );
        m_name.push_back( "HISTOGRAM_O" + ss.str() );

        const std::string prefixes[] = { "AVE_", "VAR_", "COV_" };
        for ( const std::string& prefix : prefixes )
        {
            m_name.push_back( prefix + "MIN_O" + ss.str() );
            m_name.push_back( prefix + "MIN_C" + ss.str() );
            m_name.push_back( prefix + "MAX_O" + ss.str() );
            m_name.push_back( prefix + "MAX_C" + ss.str() );
            m_name.push_back( prefix + "HISTOGRAM_C" + ss.str() );
            m_name.push_back( prefix + "HISTOGRAM_O" + ss.str() );
        }
    }
    m_name.push_back( "N_VARIABLES" );
    // m_name.push_back( "EXTRA_OPACITY_FACTOR" ); // 一時的にコメントアウト
    m_name.push_back( "PARTICLE_LIMIT" );
    m_name.push_back( "END_HISTORY_FILE" );

}

void ParticleHistoryFile::assign_name_list( const NameListFile& name_list_file )
{
    NameListFile nml = name_list_file;

#if 0
    m_variable_range.clear();
    m_variable_range.setValue( "t1_var_o", nml.getValue<vismodule::Real32>( "MIN_O1" ) );
    m_variable_range.setValue( "t1_var_o", nml.getValue<vismodule::Real32>( "MAX_O1" ) );
    m_variable_range.setValue( "t1_var_c", nml.getValue<vismodule::Real32>( "MIN_C1" ) );
    m_variable_range.setValue( "t1_var_c", nml.getValue<vismodule::Real32>( "MAX_C1" ) );
                                                                    
    m_variable_range.setValue( "t2_var_o", nml.getValue<vismodule::Real32>( "MIN_O2" ) );
    m_variable_range.setValue( "t2_var_o", nml.getValue<vismodule::Real32>( "MAX_O2" ) );
    m_variable_range.setValue( "t2_var_c", nml.getValue<vismodule::Real32>( "MIN_C2" ) );
    m_variable_range.setValue( "t2_var_c", nml.getValue<vismodule::Real32>( "MAX_C2" ) );
                                                                    
    m_variable_range.setValue( "t3_var_o", nml.getValue<vismodule::Real32>( "MIN_O3" ) );
    m_variable_range.setValue( "t3_var_o", nml.getValue<vismodule::Real32>( "MAX_O3" ) );
    m_variable_range.setValue( "t3_var_c", nml.getValue<vismodule::Real32>( "MIN_C3" ) );
    m_variable_range.setValue( "t3_var_c", nml.getValue<vismodule::Real32>( "MAX_C3" ) );
                                                                    
    m_variable_range.setValue( "t4_var_o", nml.getValue<vismodule::Real32>( "MIN_O4" ) );
    m_variable_range.setValue( "t4_var_o", nml.getValue<vismodule::Real32>( "MAX_O4" ) );
    m_variable_range.setValue( "t4_var_c", nml.getValue<vismodule::Real32>( "MIN_C4" ) );
    m_variable_range.setValue( "t4_var_c", nml.getValue<vismodule::Real32>( "MAX_C4" ) );
                                                                    
    m_variable_range.setValue( "t5_var_o", nml.getValue<vismodule::Real32>( "MIN_O5" ) );
    m_variable_range.setValue( "t5_var_o", nml.getValue<vismodule::Real32>( "MAX_O5" ) );
    m_variable_range.setValue( "t5_var_c", nml.getValue<vismodule::Real32>( "MIN_C5" ) );
    m_variable_range.setValue( "t5_var_c", nml.getValue<vismodule::Real32>( "MAX_C5" ) );

    m_color_histogram_array.resize( 5 );    
    m_color_histogram_array[0] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C1" ) );
    m_color_histogram_array[1] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C2" ) );
    m_color_histogram_array[2] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C3" ) );
    m_color_histogram_array[3] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C4" ) );
    m_color_histogram_array[4] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C5" ) );
    
    m_opacity_histogram_array.resize( 5 );
    m_opacity_histogram_array[0] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O1" ) );
    m_opacity_histogram_array[1] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O2" ) );
    m_opacity_histogram_array[2] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O3" ) );
    m_opacity_histogram_array[3] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O4" ) );
    m_opacity_histogram_array[4] =  this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O5" ) );
#endif

    m_variable_range.clear();
    m_color_histogram_array.clear();
    m_opacity_histogram_array.clear();
    m_average_variable_range.clear();
    m_variance_variable_range.clear();
    m_coefficient_of_variation_variable_range.clear();
    m_average_color_histogram_array.clear();
    m_average_opacity_histogram_array.clear();
    m_variance_color_histogram_array.clear();
    m_variance_opacity_histogram_array.clear();
    m_coefficient_of_variation_color_histogram_array.clear();
    m_coefficient_of_variation_opacity_histogram_array.clear();
    m_has_ensemble_statistic_histogram = false;
    int cur_tf_number = 0;
    const bool exist = nml.getCount( "TF_NUMBER" );
    if (exist) {
        cur_tf_number = nml.getValue<int>( "TF_NUMBER" );
    }
    else {
        cur_tf_number = 0;
    }
    m_nvariables           = nml.getValue<int>( "N_VARIABLES" );
    m_particle_limit       = nml.getValue<int>( "PARTICLE_LIMIT" );
    // m_extra_opacity_factor = nml.getValue<float>( "EXTRA_OPACITY_FACTOR" ); // 一時的にコメントアウト

    for (int i = 0; i < cur_tf_number; i++) {
        std::stringstream ss;
        ss << (i + 1);
        const std::string idxbuf = ss.str();
        m_variable_range.setValue( "t" + idxbuf + "_var_o", nml.getValue<vismodule::Real32>( "MIN_O" + idxbuf ) );
        m_variable_range.setValue( "t" + idxbuf + "_var_o", nml.getValue<vismodule::Real32>( "MAX_O" + idxbuf ) );
        m_variable_range.setValue( "t" + idxbuf + "_var_c", nml.getValue<vismodule::Real32>( "MIN_C" + idxbuf ) );
        m_variable_range.setValue( "t" + idxbuf + "_var_c", nml.getValue<vismodule::Real32>( "MAX_C" + idxbuf ) );
        m_color_histogram_array.push_back( this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C" + idxbuf ) ) );
        m_opacity_histogram_array.push_back( this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O" + idxbuf ) ) );
    }

    auto readStatistic = [&]( const std::string& prefix,
                              VariableRange& variable_range,
                              HistogramArray& color_histogram_array,
                              HistogramArray& opacity_histogram_array )
    {
        bool has_statistic = false;
        for ( int i = 0; i < cur_tf_number; i++ )
        {
            std::stringstream ss;
            ss << ( i + 1 );
            const std::string idxbuf = ss.str();
            const std::string min_o_key = prefix + "MIN_O" + idxbuf;
            const std::string max_o_key = prefix + "MAX_O" + idxbuf;
            const std::string min_c_key = prefix + "MIN_C" + idxbuf;
            const std::string max_c_key = prefix + "MAX_C" + idxbuf;
            const std::string histogram_o_key = prefix + "HISTOGRAM_O" + idxbuf;
            const std::string histogram_c_key = prefix + "HISTOGRAM_C" + idxbuf;

            const bool has_required_opacity =
                !nml.getValue<std::string>( min_o_key ).empty() &&
                !nml.getValue<std::string>( max_o_key ).empty() &&
                !nml.getValue<std::string>( histogram_o_key ).empty();

            if ( !has_required_opacity )
            {
                continue;
            }

            variable_range.setValue( "t" + idxbuf + "_var_o", nml.getValue<vismodule::Real32>( min_o_key ) );
            variable_range.setValue( "t" + idxbuf + "_var_o", nml.getValue<vismodule::Real32>( max_o_key ) );
            if ( !nml.getValue<std::string>( min_c_key ).empty() &&
                 !nml.getValue<std::string>( max_c_key ).empty() )
            {
                variable_range.setValue( "t" + idxbuf + "_var_c", nml.getValue<vismodule::Real32>( min_c_key ) );
                variable_range.setValue( "t" + idxbuf + "_var_c", nml.getValue<vismodule::Real32>( max_c_key ) );
            }

            std::vector<int> opacity_histogram = this->split_csv<int>( nml.getValue<std::string>( histogram_o_key ) );
            if ( opacity_histogram.empty() )
            {
                continue;
            }

            std::vector<int> color_histogram;
            if ( !nml.getValue<std::string>( histogram_c_key ).empty() )
            {
                color_histogram = this->split_csv<int>( nml.getValue<std::string>( histogram_c_key ) );
            }

            opacity_histogram_array.push_back( opacity_histogram );
            color_histogram_array.push_back( color_histogram );
            has_statistic = true;
        }
        return has_statistic;
    };

    const bool has_average = readStatistic(
        "AVE_",
        m_average_variable_range,
        m_average_color_histogram_array,
        m_average_opacity_histogram_array );
    const bool has_variance = readStatistic(
        "VAR_",
        m_variance_variable_range,
        m_variance_color_histogram_array,
        m_variance_opacity_histogram_array );
    const bool has_coefficient_of_variation = readStatistic(
        "COV_",
        m_coefficient_of_variation_variable_range,
        m_coefficient_of_variation_color_histogram_array,
        m_coefficient_of_variation_opacity_histogram_array );

    m_has_ensemble_statistic_histogram = has_average || has_variance || has_coefficient_of_variation;
    std::cout << "[Server][ParticleHistoryFile] ensemble statistic history"
              << " has=" << m_has_ensemble_statistic_histogram
              << " average=" << m_average_opacity_histogram_array.size()
              << " variance=" << m_variance_opacity_histogram_array.size()
              << " cv=" << m_coefficient_of_variation_opacity_histogram_array.size()
              << std::endl;

}

void ParticleHistoryFile::read_name_list_file()
{
    std::ifstream input( m_file_name.c_str() );
    if ( !input )
    {
        return;
    }

    try
    {
        nlohmann::json root;
        input >> root;

        m_variable_range.clear();
        m_color_histogram_array.clear();
        m_opacity_histogram_array.clear();

        const int tf_number = root.at( "tf_number" ).get<int>();
        m_nvariables = root.at( "nvariables" ).get<int>();
        m_particle_limit = root.at( "particle_limit" ).get<int>();

        const nlohmann::json& transfer_functions = root.at( "transfer_functions" );
        for ( int i = 1; i <= tf_number; i++ )
        {
            std::stringstream ss;
            ss << i;
            const std::string idxbuf = ss.str();

            std::stringstream opacity_key;
            opacity_key << "O" << i;
            const nlohmann::json& opacity = transfer_functions.at( opacity_key.str() );

            std::stringstream color_key;
            color_key << "C" << i;
            const nlohmann::json& color = transfer_functions.at( color_key.str() );

            m_variable_range.setValue(
                "t" + idxbuf + "_var_o",
                opacity.at( "min" ).get<vismodule::Real32>() );
            m_variable_range.setValue(
                "t" + idxbuf + "_var_o",
                opacity.at( "max" ).get<vismodule::Real32>() );
            m_variable_range.setValue(
                "t" + idxbuf + "_var_c",
                color.at( "min" ).get<vismodule::Real32>() );
            m_variable_range.setValue(
                "t" + idxbuf + "_var_c",
                color.at( "max" ).get<vismodule::Real32>() );

            const int opacity_resolution = opacity.at( "resolution" ).get<int>();
            const std::vector<int> opacity_histogram = opacity.at( "histogram" ).get<std::vector<int> >();
            if ( static_cast<int>( opacity_histogram.size() ) != opacity_resolution )
            {
                std::stringstream message;
                message << "Invalid history json: histogram size is " << opacity_histogram.size()
                        << " but resolution is " << opacity_resolution;
                throw std::runtime_error( message.str() );
            }

            const int color_resolution = color.at( "resolution" ).get<int>();
            const std::vector<int> color_histogram = color.at( "histogram" ).get<std::vector<int> >();
            if ( static_cast<int>( color_histogram.size() ) != color_resolution )
            {
                std::stringstream message;
                message << "Invalid history json: histogram size is " << color_histogram.size()
                        << " but resolution is " << color_resolution;
                throw std::runtime_error( message.str() );
            }

            m_opacity_histogram_array.push_back( opacity_histogram );
            m_color_histogram_array.push_back( color_histogram );
        }
    }
    catch ( const std::exception& )
    {
        m_variable_range.clear();
        m_color_histogram_array.clear();
        m_opacity_histogram_array.clear();
        m_nvariables = 0;
        m_particle_limit = 0;
    }
}

void ParticleHistoryFile::setFileName( const std::string& file )
{
    m_file_name = file;
}

void ParticleHistoryFile::read()
{
    this->read_name_list_file();
}



VariableRange& ParticleHistoryFile::variableRange()
{
   return m_variable_range;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::colorHistogramArray()
{
   return m_color_histogram_array;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::opacityHistogramArray()
{
   return m_opacity_histogram_array;
}

VariableRange& ParticleHistoryFile::averageVariableRange()
{
   return m_average_variable_range;
}

VariableRange& ParticleHistoryFile::varianceVariableRange()
{
   return m_variance_variable_range;
}

VariableRange& ParticleHistoryFile::coefficientOfVariationVariableRange()
{
   return m_coefficient_of_variation_variable_range;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::averageColorHistogramArray()
{
   return m_average_color_histogram_array;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::averageOpacityHistogramArray()
{
   return m_average_opacity_histogram_array;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::varianceColorHistogramArray()
{
   return m_variance_color_histogram_array;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::varianceOpacityHistogramArray()
{
   return m_variance_opacity_histogram_array;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::coefficientOfVariationColorHistogramArray()
{
   return m_coefficient_of_variation_color_histogram_array;
}

ParticleHistoryFile::HistogramArray& ParticleHistoryFile::coefficientOfVariationOpacityHistogramArray()
{
   return m_coefficient_of_variation_opacity_histogram_array;
}
