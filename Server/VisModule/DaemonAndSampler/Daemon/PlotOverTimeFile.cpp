#include "PlotOverTimeFile.h"

#include <sstream>
#include <iomanip>
#include <filesystem>
#include <vismodule/File>
#include <vismodule/Directory>

void PlotOverTimeFile::setFilePrefix( const std::string& prefix )
{
    m_file_prefix = prefix;
}

void PlotOverTimeFile::setParameterFromFile()
{
    vismodule::File file_prefix( m_file_prefix );
    // search final step
    vismodule::Directory dir( file_prefix.pathName() );
    vismodule::FileList pre_files = dir.fileList();
    vismodule::FileList files( 0 );
    vismodule::FileList::iterator fi;
    std::string prefix;
    vismodule::UInt32 num_kvsml = 0;
    vismodule::UInt32 step;
    vismodule::UInt32 div = 0;
    vismodule::UInt32 init_step = 0, fin_step = 0;
    std::string sstep, sdiv;
    std::stringstream ss;
    vismodule::UInt32 step_length    = 5;
    vismodule::UInt32 div_length     = 7;
    vismodule::UInt32 div_num_length = 7;
    vismodule::UInt32 suffix_length = static_cast<vismodule::UInt32>( std::string("dat").length() );
    std::string f_prefix = file_prefix.fileName();

    vismodule::Int32 file_length = f_prefix.length() + 1
                       + step_length + 1
                       + div_length + 1
                       + div_num_length + 1
                       + suffix_length;

    for ( fi = pre_files.begin(); fi != pre_files.end(); fi++ )
    {
        std::string bn = fi->baseName();
        std::string f_name = fi->fileName();
        vismodule::UInt32 sep = bn.find(f_prefix, 0);
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
        vismodule::UInt32 sep0 = f_prefix.length();
        vismodule::UInt32 sep1 = bn.find( "_", sep0 );
        vismodule::UInt32 sep2 = bn.find( "_", sep1 + 1 );
        vismodule::UInt32 sep3 = bn.find( "_", sep2 + 1 );
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

bool PlotOverTimeFile::generatePOTObject( const int time_step, vismodule::KVSMLObjectPlotOverTime* object )
{
    vismodule::UInt32 subvolume_num = m_subvolume_number;
    std::string prefix = m_file_prefix;

    std::vector<bool>  check_vol( subvolume_num, false );
    bool Mask = false;
    std::vector<float> Values;

    for ( int m = 0; m < subvolume_num; m++ )
    {
        if( !check_vol[m] )
        { 
            std::stringstream suffix;
            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << m + 1
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << subvolume_num;
            std::string filename = prefix + suffix.str() + ".dat";
            namespace fs = std::filesystem;
            fs::path filepath( filename );

            // ファイルが存在しない場合、オブジェクト生成失敗を返す
            if ( !fs::exists( filepath ) )
            {
                return false;
            }

            vismodule::KVSMLObjectPlotOverTime tmpimp( filename );

            if( tmpimp.isSuccess() )
            {
                check_vol[m] = true;

                if ( tmpimp.mask() )
                {
                    Mask = true;

                    size_t num = tmpimp.values_on_time().size();
                    for (size_t i = 0; i < num; i++ )
                    {
                        Values.push_back( tmpimp.values_on_time()[i]);
                    }

                    // 値を取得出来たら他のサブボリュームファイルは確認しない
                    break;
                }
            }
        }
    }

    vismodule::ValueArray<float> valuesOnTime( Values );
    object->setMask( Mask );
    object->setValuesOnTime( valuesOnTime );

    return true;
}