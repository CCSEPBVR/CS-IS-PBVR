#include "PlotOverLineFile.h"
#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

void PlotOverLineFile::setFilePrefix( const std::string& prefix )
{
    m_file_prefix = prefix;
}

void PlotOverLineFile::setParameterFromFile()
{
    kvs::File file_prefix( m_file_prefix );
    // search final step
    kvs::Directory dir( file_prefix.pathName() );
    kvs::FileList pre_files = dir.fileList();
    kvs::FileList files( 0 );
    kvs::FileList::iterator fi;
    std::string prefix;
    kvs::UInt32 num_kvsml = 0;
    kvs::UInt32 step;
    kvs::UInt32 div = 0;
    kvs::UInt32 init_step = 0, fin_step = 0;
    std::string sstep, sdiv;
    std::stringstream ss;
    kvs::UInt32 step_length    = 5;
    kvs::UInt32 div_length     = 7;
    kvs::UInt32 div_num_length = 7;
    kvs::UInt32 suffix_length = static_cast<kvs::UInt32>( std::string("dat").length() );
    std::string f_prefix = file_prefix.fileName();

    kvs::Int32 file_length = f_prefix.length() + 1
                       + step_length + 1
                       + div_length + 1
                       + div_num_length + 1
                       + suffix_length;

    for ( fi = pre_files.begin(); fi != pre_files.end(); fi++ )
    {
        std::string bn = fi->baseName();
        std::string f_name = fi->fileName();
        kvs::UInt32 sep = bn.find(f_prefix, 0);
        if (sep == 0 && file_length == f_name.length())
        {
            std::string ext = fi->extension();
            if ( ext == "dat" )
            {
                files.push_back(*fi);
            }
        }
    }

    for ( fi = files.begin(); fi != files.end(); fi++ )
    {
        std::string bn = fi->baseName();
        std::string f_prefix = file_prefix.fileName();
        kvs::UInt32 sep0 = f_prefix.length();
        kvs::UInt32 sep1 = bn.find( "_", sep0 );
        kvs::UInt32 sep2 = bn.find( "_", sep1 + 1 );
        kvs::UInt32 sep3 = bn.find( "_", sep2 + 1 );
        if ( sep1 == std::string::npos || sep2 == std::string::npos || sep3 == std::string::npos )
            continue;

        prefix = bn.substr( 0, sep1 );
        sstep = bn.substr( sep1 + 1, step_length );
        sdiv = bn.substr( sep3 + 1, div_length );

        ss.clear();
        ss << sstep << ' ' << sdiv;
        ss >> step >> div;

        init_step = ( num_kvsml ) ? std::min( init_step, step ) : step;
        fin_step  = ( num_kvsml ) ? std::max( fin_step, step ) : step;

        num_kvsml++;
    }

    m_subvolume_number  = div;
    m_initial_step      = init_step;
    m_final_step        = fin_step;
    m_file_prefix       = dir.directoryPath( true ) + dir.Separator() + f_prefix;
    m_kvsml_file_number = num_kvsml;
}

void PlotOverLineFile::generatePOLObject( const int time_step, kvs::KVSMLObjectPlotOverLine* object )
{
    kvs::UInt32 subvolume_num = m_subvolume_number;
    std::string prefix = m_file_prefix;

    std::cout << "m_subvolume_number = " << m_subvolume_number << std::endl;
    std::cout << "m_file_prefix = " << m_file_prefix << std::endl;

    std::vector<bool> check_vol( subvolume_num, false );

    std::vector<float> xAxis;
    std::vector<bool> Mask;
    std::vector<float>  Values;
    bool read_success = false;
    bool resize_flag =true;
    while( !read_success )
    {
        for ( int m = 0; m < subvolume_num; m++ )
        {

            std::vector<float>  tmp_xAxis;
            std::vector<bool>   tmp_Mask;
            std::vector<float>  tmp_Values;
            if( !check_vol[m] )
            { 
                std::stringstream suffix;
                suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step
                    << '_' << std::setw( 7 ) << std::setfill( '0' ) << m + 1
                    << '_' << std::setw( 7 ) << std::setfill( '0' ) << subvolume_num;
                std::string filename = prefix + suffix.str() + ".dat";
                kvs::KVSMLObjectPlotOverLine tmpimp(filename);

                if( tmpimp.isSuccess() ) check_vol[m] = true;

                if ( check_vol[m] )
                {
                    int num = tmpimp.x_axis().size();
                    for (int i = 0; i < num; i ++  )
                    {
                        tmp_xAxis.push_back( tmpimp.x_axis()[ i ]); 
                        tmp_Mask.push_back( tmpimp.mask()[ i ]); 
                        tmp_Values.push_back(   tmpimp.values_on_line()[i]); 
                    }
                }

                if ( resize_flag)
                {
                   xAxis.resize(tmp_xAxis.size());
                   Mask.resize(tmp_Mask.size());
                   Values.resize(tmp_Values.size());
                   xAxis = tmp_xAxis;
                   resize_flag =false;
                }
                for(int i =0; i < tmp_Mask.size(); i++)
                { 
                    if (tmp_Mask[i]) 
                    {
                        Mask[i] = tmp_Mask[i];
                        Values[i] = tmp_Values[i];
                    } 
                }


            }
        }

        read_success = true;
        for( int m = 0; m < subvolume_num; m++ )
        {
            if( !check_vol[m] )
            {
                read_success = false;
                break;
            }
        }
    }

    kvs::ValueArray<float> x_axis(xAxis);
    kvs::ValueArray<float> valuesOnLine(Values);

    kvs::ValueArray<bool>  mask;
    mask.allocate(Mask.size());
    mask.fill(false);
    for(int i =0; i < Mask.size(); i++)
    { 
           mask[i] = Mask[i];
    }

    object-> setXAxis(x_axis);
    object-> setMask(mask);
    object-> setValuesOnLine(valuesOnLine);
                    std::cout << __FUNCTION__ <<__LINE__ <<std::endl; 
}



