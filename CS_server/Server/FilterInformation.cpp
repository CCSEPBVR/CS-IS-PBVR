#include <iostream>
#include <fstream>
#include <algorithm>

#include "FilterInformation.h"
#include <kvs/File>
#include "endian2.h"

//--------------------------------------------------------------------------

int FilterInformationFile::loadPFI( const std::string& filename )
{
    std::ifstream fin( filename.c_str(), std::ios::in | std::ios::binary );
    if ( ! fin ) return -1;

    fin.read( ( char* )&m_number_nodes, sizeof( int32_t ) );
    fin.read( ( char* )&m_number_elements, sizeof( int32_t ) );
    fin.read( ( char* )&m_elem_type, sizeof( int32_t ) );
    fin.read( ( char* )&m_file_type, sizeof( int32_t ) );
    fin.read( ( char* )&m_number_files, sizeof( int32_t ) );
    fin.read( ( char* )&m_number_ingredients, sizeof( int32_t ) );
    fin.read( ( char* )&m_start_step, sizeof( int32_t ) );
    fin.read( ( char* )&m_end_steps, sizeof( int32_t ) );
    fin.read( ( char* )&m_number_subvolumes, sizeof( int32_t ) );

    endian2::LittleToHost( &m_number_nodes );
    endian2::LittleToHost( &m_number_elements );
    endian2::LittleToHost( &m_elem_type );
    endian2::LittleToHost( &m_file_type );
    endian2::LittleToHost( &m_number_files );
    endian2::LittleToHost( &m_number_ingredients );
    endian2::LittleToHost( &m_start_step );
    endian2::LittleToHost( &m_end_steps );
    endian2::LittleToHost( &m_number_subvolumes );
    m_number_steps = m_end_steps - m_start_step + 1;

    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    fin.read( ( char* )&x_min, sizeof( float ) );
    fin.read( ( char* )&y_min, sizeof( float ) );
    fin.read( ( char* )&z_min, sizeof( float ) );
    fin.read( ( char* )&x_max, sizeof( float ) );
    fin.read( ( char* )&y_max, sizeof( float ) );
    fin.read( ( char* )&z_max, sizeof( float ) );
    endian2::LittleToHost( &x_min );
    endian2::LittleToHost( &y_min );
    endian2::LittleToHost( &z_min );
    endian2::LittleToHost( &x_max );
    endian2::LittleToHost( &y_max );
    endian2::LittleToHost( &z_max );
    m_min_object_coord.set( x_min, y_min, z_min );
    m_max_object_coord.set( x_max, y_max, z_max );

    int skipsize = 0;
    skipsize += ( m_number_subvolumes + m_number_subvolumes ) * sizeof( int );
    fin.seekg( skipsize, std::ios::cur );

    m_min_subvolume_coord.resize( m_number_subvolumes );
    m_max_subvolume_coord.resize( m_number_subvolumes );
    for ( int32_t vl = 0; vl < m_number_subvolumes; vl++ )
    {
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        fin.read( ( char* )&sub_x_min, sizeof( float ) );
        fin.read( ( char* )&sub_y_min, sizeof( float ) );
        fin.read( ( char* )&sub_z_min, sizeof( float ) );
        fin.read( ( char* )&sub_x_max, sizeof( float ) );
        fin.read( ( char* )&sub_y_max, sizeof( float ) );
        fin.read( ( char* )&sub_z_max, sizeof( float ) );
        endian2::LittleToHost( &sub_x_min );
        endian2::LittleToHost( &sub_y_min );
        endian2::LittleToHost( &sub_z_min );
        endian2::LittleToHost( &sub_x_max );
        endian2::LittleToHost( &sub_y_max );
        endian2::LittleToHost( &sub_z_max );
        m_min_subvolume_coord[vl].set( sub_x_min, sub_y_min, sub_z_min );
        m_max_subvolume_coord[vl].set( sub_x_max, sub_y_max, sub_z_max );
    }

    m_ingredient_step.clear();
    for ( int32_t s = 0; s < m_number_steps; s++ )
    {
        m_ingredient_step.push_back( IngredientsStep() );
        m_ingredient_step[s].m_ingredient.clear();
        for ( int32_t i = 0; i < m_number_ingredients; i++ )
        {
            float min, max;
            fin.read( ( char* )&min, sizeof( float ) );
            fin.read( ( char* )&max, sizeof( float ) );
            endian2::LittleToHost( &min );
            endian2::LittleToHost( &max );

            m_ingredient_step[s].m_ingredient.push_back( IngredientsMinMax() );
            m_ingredient_step[s].m_ingredient[i].m_min = min;
            m_ingredient_step[s].m_ingredient[i].m_max = max;
        }
    }

    m_min_value = m_ingredient_step[0].m_ingredient[0].m_min;
    m_max_value = m_ingredient_step[0].m_ingredient[0].m_max;
    for ( int32_t s = 0; s < m_number_steps; s++ )
    {
        for ( int32_t i = 0; i < m_number_ingredients; i++ )
        {
            float min = m_ingredient_step[s].m_ingredient[i].m_min;
            float max = m_ingredient_step[s].m_ingredient[i].m_max;
            if ( min < m_min_value ) m_min_value = min;
            if ( max > m_max_value ) m_max_value = max;
        }
    }

    fin.close();

    /*
      std::cout << "numNodes:      " << numNodes       << std::endl;
      std::cout << "m_number_elements:   " << m_number_elements    << std::endl;
      std::cout << "m_elem_type:      " << m_elem_type       << std::endl;
      std::cout << "fileType:      " << fileType       << std::endl;
      std::cout << "numFiles:      " << numFiles       << std::endl;
      std::cout << "m_number_ingredients:" << m_number_ingredients << std::endl;
      std::cout << "numSteps:      " << numSteps       << std::endl;
      std::cout << "m_number_subvolumes: " << m_number_subvolumes  << std::endl;
    */

    m_file_path = filename;
    return 0;
}

//--------------------------------------------------------------------------

FilterInformationList::FilterInformationList():
    m_total_number_nodes( 0 ), m_total_number_elements( 0 ), m_total_number_files( 0 ),
    m_total_start_steps( 0 ), m_total_end_step( 0 ), m_total_number_steps( 0 ),
    m_total_number_subvolumes( 0 ), m_total_min_value( 0.f ), m_total_max_value( 0.f )
{
}

FilterInformationList::~FilterInformationList()
{
}

int FilterInformationList::loadPFL( const std::string& filename )
{
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    kvs::File pfl( filename );
    if ( filename.size() < 4 || ! pfl.isExisted() ) return -1;
    if ( filename.substr( filename.size() - 3 ) == "pfi" )
    {
        FilterInformationFile fi;
        if ( fi.loadPFI( filename ) < 0 ) return -1;
        m_list.push_back( fi );

        m_total_number_nodes = fi.m_number_nodes;
        m_total_number_elements = fi.m_number_elements;
        m_total_number_files = fi.m_number_files;
        m_total_start_steps = fi.m_start_step;
        m_total_end_step = fi.m_end_steps;
        m_total_number_steps = fi.m_number_steps;
        m_total_number_subvolumes = fi.m_number_subvolumes;
        m_total_min_object_coord = fi.m_min_object_coord;
        m_total_max_object_coord = fi.m_max_object_coord;
        m_total_min_subvolume_coord = fi.m_min_subvolume_coord;
        m_total_max_subvolume_coord = fi.m_max_subvolume_coord;
        m_total_min_value = fi.m_min_value;
        m_total_max_value = fi.m_max_value;
        m_total_number_ingredients = fi.m_number_ingredients;
        m_total_ingredient.resize( m_total_number_ingredients );
        for (int32_t i = 0; i < m_total_number_ingredients; i++)
        {
            m_total_ingredient[i].m_min = FLT_MAX;
            m_total_ingredient[i].m_max = -FLT_MAX;
        }
        calculate_ingredient_min_max( fi, &m_total_ingredient );
        return 1;
    }

    std::ifstream fin( filename.c_str(), std::ios::in );
    if ( ! fin ) return -1;
    std::string fbuff;
    if ( ! getline( fin, fbuff ) ) return -1;
    if ( fbuff != std::string( "#PBVR PFI FILES" ) ) return -1;
    while ( getline( fin, fbuff ) )
    {
        kvs::File pfi( fbuff );
        if ( fbuff.size() < 4 ) continue;
        if ( fbuff.substr( fbuff.size() - 3 ) != "pfi" ) continue;
        if ( ! pfi.isExisted() )
        {
            std::string xpath = pfl.pathName( true ) + pfl.Separator() + pfi.filePath( false );
            kvs::File xpfi( xpath );
            if ( ! xpfi.isExisted() ) continue;
            fbuff = xpath;
        }
        FilterInformationFile fi;
        fi.loadPFI( fbuff );

        if ( m_list.empty() )
        {
            m_total_number_nodes = fi.m_number_nodes;
            m_total_number_elements = fi.m_number_elements;
            m_total_number_files = fi.m_number_files;
            m_total_start_steps = fi.m_start_step;
            m_total_end_step = fi.m_end_steps;
            m_total_number_steps = fi.m_number_steps;
            m_total_number_subvolumes = fi.m_number_subvolumes;
            m_total_min_object_coord = fi.m_min_object_coord;
            m_total_max_object_coord = fi.m_max_object_coord;
            m_total_min_subvolume_coord = fi.m_min_subvolume_coord;
            m_total_max_subvolume_coord = fi.m_max_subvolume_coord;
            m_total_min_value = fi.m_min_value;
            m_total_max_value = fi.m_max_value;
            m_total_number_ingredients = fi.m_number_ingredients;
        }
        else
        {
            m_total_number_nodes += fi.m_number_nodes;
            m_total_number_elements += fi.m_number_elements;
            m_total_number_files += fi.m_number_files;
            m_total_start_steps = std::min( m_total_start_steps, fi.m_start_step );
            m_total_end_step = std::max( m_total_end_step, fi.m_end_steps );
            m_total_number_steps = m_total_end_step - m_total_start_steps + 1;
            m_total_number_subvolumes += fi.m_number_subvolumes;
            m_total_min_object_coord[0]
                = std::min( m_total_min_object_coord[0], fi.m_min_object_coord[0] );
            m_total_min_object_coord[1]
                = std::min( m_total_min_object_coord[1], fi.m_min_object_coord[1] );
            m_total_min_object_coord[2]
                = std::min( m_total_min_object_coord[2], fi.m_min_object_coord[2] );
            m_total_max_object_coord[0]
                = std::max( m_total_max_object_coord[0], fi.m_max_object_coord[0] );
            m_total_max_object_coord[1]
                = std::max( m_total_max_object_coord[1], fi.m_max_object_coord[1] );
            m_total_max_object_coord[2]
                = std::max( m_total_max_object_coord[2], fi.m_max_object_coord[2] );
            std::copy( fi.m_min_subvolume_coord.begin(), fi.m_min_subvolume_coord.end(),
                       std::back_inserter( m_total_min_subvolume_coord ) );
            std::copy( fi.m_max_subvolume_coord.begin(), fi.m_max_subvolume_coord.end(),
                       std::back_inserter( m_total_max_subvolume_coord ) );
            m_total_min_value = std::min( m_total_min_value, fi.m_min_value );
            m_total_max_value = std::max( m_total_max_value, fi.m_max_value );
            m_total_number_ingredients = std::max( m_total_number_ingredients, fi.m_number_ingredients );
        }

        m_list.push_back( fi );
    } // end of while()
    fin.close();
 
    m_total_ingredient.resize( m_total_number_ingredients );

    for ( int32_t idx = 0; idx < m_list.size(); idx++ )
    {
        calculate_ingredient_min_max( m_list[idx], &m_total_ingredient );
    }

    return m_list.size();
}

void FilterInformationList::calculate_ingredient_min_max( const FilterInformationFile &fi, 
                                                         std::vector<FilterInformationFile::IngredientsMinMax> *total_ingredient )
{
    for ( int32_t i = 0; i < fi.m_number_ingredients; i++ )
    {
        float min = fi.m_ingredient_step[0].m_ingredient[i].m_min;
        float max = fi.m_ingredient_step[0].m_ingredient[i].m_max;
        (*total_ingredient)[i].m_min = std::min(min, (*total_ingredient)[i].m_min);
        (*total_ingredient)[i].m_max = std::max(max, (*total_ingredient)[i].m_max);
    }
}

int FilterInformationList::getFileIndex( const int vl, int* xvl ) const
{
    if ( m_list.empty() ) return -1;
    if ( vl >= m_total_number_subvolumes ) return -1;

    int idx, cvl = vl;
    for ( idx = 0; idx < m_list.size(); idx++ )
    {
        if ( cvl < m_list[idx].m_number_subvolumes ) break;
        cvl -= m_list[idx].m_number_subvolumes;
    }
    if ( idx >= m_list.size() ) return -1;
    *xvl = cvl;
    return idx;
}

void FilterInformationList::cropTimeStep( const int s, const int e )
{
    if ( m_list.empty() ) return;
    if ( s > e ) return;

    if ( s != INT_MIN )
    {
        if ( s > m_total_start_steps )
            m_total_start_steps = s;
    }

    if ( e != INT_MAX )
    {
        if ( e < m_total_end_step )
            m_total_end_step = e;
    }

    m_total_number_steps = m_total_end_step - m_total_start_steps + 1;
}
