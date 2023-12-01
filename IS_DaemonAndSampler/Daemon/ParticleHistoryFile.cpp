#include "ParticleHistoryFile.h"

ParticleHistoryFile::ParticleHistoryFile()
{
    this->set_name();
}

ParticleHistoryFile::ParticleHistoryFile( const std::string& file ):
    m_file_name( file )
{
    this->set_name();
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
    }
    m_name.push_back( "END_HISTORY_FILE" );

}

void ParticleHistoryFile::assign_name_list( const NameListFile& name_list_file )
{
    NameListFile nml = name_list_file;

#if 0
    m_variable_range.clear();
    m_variable_range.setValue( "t1_var_o", nml.getValue<kvs::Real32>( "MIN_O1" ) );
    m_variable_range.setValue( "t1_var_o", nml.getValue<kvs::Real32>( "MAX_O1" ) );
    m_variable_range.setValue( "t1_var_c", nml.getValue<kvs::Real32>( "MIN_C1" ) );
    m_variable_range.setValue( "t1_var_c", nml.getValue<kvs::Real32>( "MAX_C1" ) );
                                                                    
    m_variable_range.setValue( "t2_var_o", nml.getValue<kvs::Real32>( "MIN_O2" ) );
    m_variable_range.setValue( "t2_var_o", nml.getValue<kvs::Real32>( "MAX_O2" ) );
    m_variable_range.setValue( "t2_var_c", nml.getValue<kvs::Real32>( "MIN_C2" ) );
    m_variable_range.setValue( "t2_var_c", nml.getValue<kvs::Real32>( "MAX_C2" ) );
                                                                    
    m_variable_range.setValue( "t3_var_o", nml.getValue<kvs::Real32>( "MIN_O3" ) );
    m_variable_range.setValue( "t3_var_o", nml.getValue<kvs::Real32>( "MAX_O3" ) );
    m_variable_range.setValue( "t3_var_c", nml.getValue<kvs::Real32>( "MIN_C3" ) );
    m_variable_range.setValue( "t3_var_c", nml.getValue<kvs::Real32>( "MAX_C3" ) );
                                                                    
    m_variable_range.setValue( "t4_var_o", nml.getValue<kvs::Real32>( "MIN_O4" ) );
    m_variable_range.setValue( "t4_var_o", nml.getValue<kvs::Real32>( "MAX_O4" ) );
    m_variable_range.setValue( "t4_var_c", nml.getValue<kvs::Real32>( "MIN_C4" ) );
    m_variable_range.setValue( "t4_var_c", nml.getValue<kvs::Real32>( "MAX_C4" ) );
                                                                    
    m_variable_range.setValue( "t5_var_o", nml.getValue<kvs::Real32>( "MIN_O5" ) );
    m_variable_range.setValue( "t5_var_o", nml.getValue<kvs::Real32>( "MAX_O5" ) );
    m_variable_range.setValue( "t5_var_c", nml.getValue<kvs::Real32>( "MIN_C5" ) );
    m_variable_range.setValue( "t5_var_c", nml.getValue<kvs::Real32>( "MAX_C5" ) );

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
    int cur_tf_number = 0;
    const bool exist = nml.getCount( "TF_NUMBER" );
    if (exist) {
        cur_tf_number = nml.getValue<int>( "TF_NUMBER" );
    }
    else {
        cur_tf_number = 0;
    }

    for (int i = 0; i < cur_tf_number; i++) {
        std::stringstream ss;
        ss << (i + 1);
        const std::string idxbuf = ss.str();
        m_variable_range.setValue( "t" + idxbuf + "_var_o", nml.getValue<kvs::Real32>( "MIN_O" + idxbuf ) );
        m_variable_range.setValue( "t" + idxbuf + "_var_o", nml.getValue<kvs::Real32>( "MAX_O" + idxbuf ) );
        m_variable_range.setValue( "t" + idxbuf + "_var_c", nml.getValue<kvs::Real32>( "MIN_C" + idxbuf ) );
        m_variable_range.setValue( "t" + idxbuf + "_var_c", nml.getValue<kvs::Real32>( "MAX_C" + idxbuf ) );
        m_color_histogram_array.push_back( this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_C" + idxbuf ) ) );
        m_opacity_histogram_array.push_back( this->split_csv<int>( nml.getValue<std::string>( "HISTOGRAM_O" + idxbuf ) ) );
    }

}

void ParticleHistoryFile::read_name_list_file()
{
    NameListFile nlfile( m_file_name );

    for( NameArray::iterator i = m_name.begin(); i != m_name.end(); i++ )
    {
        nlfile.setName( *i );
    }

    if( nlfile.read() ) {
        while (!nlfile.getCount("END_HISTORY_FILE") || nlfile.getValue<std::string>("END_HISTORY_FILE") != "SUCCESS") {
            nlfile.read();
        }
        this->assign_name_list( nlfile );
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
