#include "ParticleFile.h"
#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

void ParticleFile::setFilePrefix( const std::string& prefix )
{
    m_file_prefix = prefix;
}

void ParticleFile::setParameterFromFile()
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
    vismodule::UInt32 suffix_length = static_cast<vismodule::UInt32>( std::string("kvsml").length() );
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
            if ( ext == "kvsml" )
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

void ParticleFile::generatePointObject( const int time_step, vismodule::PointObject* object )
{
    vismodule::UInt32 subvolume_num = m_subvolume_number;
    std::string prefix = m_file_prefix;

    std::vector<bool> check_vol( subvolume_num, false );
    bool read_success = false;
#if _OPENMP
    int max_threads = omp_get_max_threads();
    int thid        = omp_get_thread_num();
#else
    int max_threads = 1;
    int thid        = 0;
#endif
    vismodule::PointObject** tmp_obj = new vismodule::PointObject*[ max_threads];

    for ( int i = 0; i < max_threads; i++ )
    {
        tmp_obj[i] = new vismodule::PointObject();
    }

    while( !read_success )
    {
        #pragma omp parallel
        {
            #if _OPENMP
            int thid        = omp_get_thread_num();
            #else
            int thid        = 0;
            #endif
            #pragma omp for
            for ( int m = 0; m < subvolume_num; m++ )
            {
                if( !check_vol[m] )
                { 
                    std::stringstream suffix;
                    suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step
                           << '_' << std::setw( 7 ) << std::setfill( '0' ) << m + 1
                           << '_' << std::setw( 7 ) << std::setfill( '0' ) << subvolume_num;
                    std::string filename = prefix + suffix.str() + ".kvsml";
                    const vismodule::File file( filename );
                    vismodule::PointImporter* tmpimp = new vismodule::PointImporter( filename );

                    if( tmpimp->isSuccess() ) check_vol[m] = true;

                    if ( check_vol[m] )
                    {
                        vismodule::PointObject* impobj = tmpimp;

                        vismodule::UInt32 nmemb = impobj->coords().size();
    
                        if ( nmemb != 0 )
                        {
                            *( tmp_obj[thid] ) += *impobj;
                        }
                    } 
                    delete tmpimp;
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

    object -> setSize(1); 
    for ( int i = 0; i < max_threads; i++ )
    {
        vismodule::UInt32 nmemb = tmp_obj[i]->coords().size();
        if ( nmemb != 0 )
        {
            ( *object ) += *( tmp_obj[i] );
        }
        delete tmp_obj[i];
    }
    delete[] tmp_obj;
}


