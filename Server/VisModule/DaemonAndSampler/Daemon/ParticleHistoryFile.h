#ifndef __JUPITER_PARTICLE_HISTORY_FILE_H__
#define __JUPITER_PARTICLE_HISTORY_FILE_H__

#include <iostream>
#include <string>
#include <vector>
#include <vismodule/Type>
#include <vismodule/NameListFile>
#include <vismodule/VariableRange>

class ParticleHistoryFile
{
private:
    typedef std::vector<std::string> NameArray;
    typedef std::vector< std::vector<int> > HistogramArray;
   
    std::string       m_file_name;
    NameArray         m_name;
    VariableRange     m_variable_range;
    HistogramArray    m_color_histogram_array;
    HistogramArray    m_opacity_histogram_array;
    VariableRange     m_average_variable_range;
    VariableRange     m_variance_variable_range;
    VariableRange     m_coefficient_of_variation_variable_range;
    HistogramArray    m_average_color_histogram_array;
    HistogramArray    m_average_opacity_histogram_array;
    HistogramArray    m_variance_color_histogram_array;
    HistogramArray    m_variance_opacity_histogram_array;
    HistogramArray    m_coefficient_of_variation_color_histogram_array;
    HistogramArray    m_coefficient_of_variation_opacity_histogram_array;
    bool              m_has_ensemble_statistic_histogram;
    int               m_nvariables;
    int               m_particle_limit;
    float             m_extra_opacity_factor;

private:

    template<typename T>
    inline T from_string( const std::string& name );

    template<typename T>
    inline std::template vector<T> split( const std::string& line, const char delim  );

    template<typename T>
    inline std::template vector<T> split_csv( const std::string& line );

    void set_name();
    void assign_name_list( const NameListFile& name_list_file );
    void read_name_list_file();

public:
    ParticleHistoryFile();
    ParticleHistoryFile( const std::string& file );
    void setFileName( const std::string& file );
    void read();
    VariableRange& variableRange();
    HistogramArray& colorHistogramArray();
    HistogramArray& opacityHistogramArray();
    bool hasEnsembleStatisticHistogram() const { return m_has_ensemble_statistic_histogram; };
    VariableRange& averageVariableRange();
    VariableRange& varianceVariableRange();
    VariableRange& coefficientOfVariationVariableRange();
    HistogramArray& averageColorHistogramArray();
    HistogramArray& averageOpacityHistogramArray();
    HistogramArray& varianceColorHistogramArray();
    HistogramArray& varianceOpacityHistogramArray();
    HistogramArray& coefficientOfVariationColorHistogramArray();
    HistogramArray& coefficientOfVariationOpacityHistogramArray();
    int nVariables(){return m_nvariables; };
    int ParticleLimit(){return m_particle_limit; };
    float ExtraOpacityFactor(){return m_extra_opacity_factor; };
};

template<typename T>
inline T ParticleHistoryFile::from_string( const std::string& name )
{
    T value;
    std::istringstream is( name );
    is >> value;
    return value;
}

template<typename T>
inline std::template vector<T> ParticleHistoryFile::split( const std::string& line, const char delim  )
{
    std::template vector<T> array;
    std::stringstream ss_line( line );
    std::string item;

    while ( std::getline( ss_line, item, delim ) )
    {
        if ( item.empty() ) continue;
        array.push_back( this->from_string<T>( item ) );
    }

    return array;
}

template<typename T>
inline std::template vector<T> ParticleHistoryFile::split_csv( const std::string& line )
{
    return split<T>( line, ',' );
}
#endif
