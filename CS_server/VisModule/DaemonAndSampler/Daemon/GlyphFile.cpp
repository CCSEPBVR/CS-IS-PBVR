#include "GlyphFile.h"
#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

void GlyphFile::setFilePrefix( const std::string& prefix )
{
    m_file_prefix = prefix;
}

void GlyphFile::setParameterFromFile()
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

void GlyphFile::generateGlyphObject( const int time_step, vismodule::KVSMLObjectGlyph* object )
{
    vismodule::UInt32 subvolume_num = m_subvolume_number;
    std::string prefix = m_file_prefix;

    std::vector<bool> check_vol( subvolume_num, false );

    std::vector<vismodule::Real32> glyph_coord    ;
    std::vector<vismodule::Real32> glyph_direction;
    std::vector<vismodule::Real32> glyph_size     ;
    std::vector<vismodule::UInt8>  glyph_color    ;
    bool read_success = false;
    while( !read_success )
    {
        for ( int m = 0; m < subvolume_num; m++ )
        {
            if( !check_vol[m] )
            { 
                std::stringstream suffix;
                suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step
                    << '_' << std::setw( 7 ) << std::setfill( '0' ) << m + 1
                    << '_' << std::setw( 7 ) << std::setfill( '0' ) << subvolume_num;
                std::string filename = prefix + suffix.str() + ".dat";
                vismodule::KVSMLObjectGlyph tmpimp(filename);

                if( tmpimp.isSuccess() ) check_vol[m] = true;

                if ( check_vol[m] )
                {
                    int num = tmpimp.sizes().size();
                    for (int i = 0; i < num; i ++  )
                    {
                        glyph_coord.push_back( tmpimp.coords()[3*i  ]); 
                        glyph_coord.push_back( tmpimp.coords()[3*i+1]); 
                        glyph_coord.push_back( tmpimp.coords()[3*i+2]); 
                        glyph_direction.push_back( tmpimp.directions()[3*i  ]); 
                        glyph_direction.push_back( tmpimp.directions()[3*i+1]); 
                        glyph_direction.push_back( tmpimp.directions()[3*i+2]); 
                        glyph_size.push_back(   tmpimp.sizes()[i]); 
                        glyph_color.push_back(  tmpimp.colors()[3*i  ]); 
                        glyph_color.push_back(  tmpimp.colors()[3*i+1]); 
                        glyph_color.push_back(  tmpimp.colors()[3*i+2]); 
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

    vismodule::ValueArray<vismodule::Real32> coords(glyph_coord);
    vismodule::ValueArray<vismodule::Real32> directions(glyph_direction);
    vismodule::ValueArray<vismodule::Real32> sizes(glyph_size);
    vismodule::ValueArray<vismodule::UInt8>  colors(glyph_color);
    object-> setCoords(coords);
    object-> setDirections(directions);
    object-> setSizes(sizes);
    object-> setColors(colors);
}



