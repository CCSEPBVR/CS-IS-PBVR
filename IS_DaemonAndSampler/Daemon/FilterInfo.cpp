#include <iostream>
#include <fstream>
#include <algorithm>

#include "FilterInfo.h"
#include <kvs/File>
#include "endian2.h"

//--------------------------------------------------------------------------

int FilterInfo::LoadPFI( std::string& filename )
{
    std::ifstream fin( filename.c_str(), std::ios::in | std::ios::binary );
    if ( ! fin ) return -1;

    fin.read( ( char* )&numNodes, sizeof( int32_t ) );
    fin.read( ( char* )&numElements, sizeof( int32_t ) );
    fin.read( ( char* )&elemType, sizeof( int32_t ) );
    fin.read( ( char* )&fileType, sizeof( int32_t ) );
    fin.read( ( char* )&numFiles, sizeof( int32_t ) );
    fin.read( ( char* )&numIngredients, sizeof( int32_t ) );
    fin.read( ( char* )&staSteps, sizeof( int32_t ) );
    fin.read( ( char* )&endSteps, sizeof( int32_t ) );
    fin.read( ( char* )&numSubVolumes, sizeof( int32_t ) );

    endian2::ltoh( numNodes );
    endian2::ltoh( numElements );
    endian2::ltoh( elemType );
    endian2::ltoh( fileType );
    endian2::ltoh( numFiles );
    endian2::ltoh( numIngredients );
    endian2::ltoh( staSteps );
    endian2::ltoh( endSteps );
    endian2::ltoh( numSubVolumes );
    numSteps = endSteps - staSteps + 1;

    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    fin.read( ( char* )&x_min, sizeof( float ) );
    fin.read( ( char* )&y_min, sizeof( float ) );
    fin.read( ( char* )&z_min, sizeof( float ) );
    fin.read( ( char* )&x_max, sizeof( float ) );
    fin.read( ( char* )&y_max, sizeof( float ) );
    fin.read( ( char* )&z_max, sizeof( float ) );
    endian2::ltoh( x_min );
    endian2::ltoh( y_min );
    endian2::ltoh( z_min );
    endian2::ltoh( x_max );
    endian2::ltoh( y_max );
    endian2::ltoh( z_max );
    minObjectCoord.set( x_min, y_min, z_min );
    maxObjectCoord.set( x_max, y_max, z_max );

    int skipsize = 0;
    skipsize += ( numSubVolumes + numSubVolumes ) * sizeof( int );
    fin.seekg( skipsize, std::ios::cur );

    minSubVolumeCoord.resize( numSubVolumes );
    maxSubVolumeCoord.resize( numSubVolumes );
    for ( int32_t vl = 0; vl < numSubVolumes; vl++ )
    {
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        fin.read( ( char* )&sub_x_min, sizeof( float ) );
        fin.read( ( char* )&sub_y_min, sizeof( float ) );
        fin.read( ( char* )&sub_z_min, sizeof( float ) );
        fin.read( ( char* )&sub_x_max, sizeof( float ) );
        fin.read( ( char* )&sub_y_max, sizeof( float ) );
        fin.read( ( char* )&sub_z_max, sizeof( float ) );
        endian2::ltoh( sub_x_min );
        endian2::ltoh( sub_y_min );
        endian2::ltoh( sub_z_min );
        endian2::ltoh( sub_x_max );
        endian2::ltoh( sub_y_max );
        endian2::ltoh( sub_z_max );
        minSubVolumeCoord[vl].set( sub_x_min, sub_y_min, sub_z_min );
        maxSubVolumeCoord[vl].set( sub_x_max, sub_y_max, sub_z_max );
    }

    ingredStep.clear();
    for ( int32_t s = 0; s < numSteps; s++ )
    {
        ingredStep.push_back( ingredientsStep() );
        ingredStep[s].ingred.clear();
        for ( int32_t i = 0; i < numIngredients; i++ )
        {
            float min, max;
            fin.read( ( char* )&min, sizeof( float ) );
            fin.read( ( char* )&max, sizeof( float ) );
            endian2::ltoh( min );
            endian2::ltoh( max );

            ingredStep[s].ingred.push_back( ingredientsMinMax() );
            ingredStep[s].ingred[i].min = min;
            ingredStep[s].ingred[i].max = max;
        }
    }

    minValue = ingredStep[0].ingred[0].min;
    maxValue = ingredStep[0].ingred[0].max;
    for ( int32_t s = 0; s < numSteps; s++ )
    {
        for ( int32_t i = 0; i < numIngredients; i++ )
        {
            float min = ingredStep[s].ingred[i].min;
            float max = ingredStep[s].ingred[i].max;
            if ( min < minValue ) minValue = min;
            if ( max > maxValue ) maxValue = max;
        }
    }

    fin.close();

    /*
      std::cout << "numNodes:      " << numNodes       << std::endl;
      std::cout << "numElements:   " << numElements    << std::endl;
      std::cout << "elemType:      " << elemType       << std::endl;
      std::cout << "fileType:      " << fileType       << std::endl;
      std::cout << "numFiles:      " << numFiles       << std::endl;
      std::cout << "numIngredients:" << numIngredients << std::endl;
      std::cout << "numSteps:      " << numSteps       << std::endl;
      std::cout << "numSubVolumes: " << numSubVolumes  << std::endl;
    */

    filePath = filename;
    return 0;
}

//--------------------------------------------------------------------------

FilterInfoLst::FilterInfoLst()
    : total_numNodes( 0 ), total_numElements( 0 ), total_numFiles( 0 ),
      total_staSteps( 0 ), total_endSteps( 0 ), total_numSteps( 0 ),
      total_numSubVolumes( 0 ), total_minValue( 0.f ), total_maxValue( 0.f )
{
}

FilterInfoLst::~FilterInfoLst()
{
}

int FilterInfoLst::LoadPFL( std::string& filename )
{
    m_list.clear();
    total_minSubVolumeCoord.clear();
    total_maxSubVolumeCoord.clear();

    kvs::File pfl( filename );
    if ( filename.size() < 4 || ! pfl.isExisted() ) return -1;
    if ( filename.substr( filename.size() - 3 ) == "pfi" )
    {
        FilterInfo fi;
        if ( fi.LoadPFI( filename ) < 0 ) return -1;
        m_list.push_back( fi );

        total_numNodes = fi.numNodes;
        total_numElements = fi.numElements;
        total_numFiles = fi.numFiles;
        total_staSteps = fi.staSteps;
        total_endSteps = fi.endSteps;
        total_numSteps = fi.numSteps;
        total_numSubVolumes = fi.numSubVolumes;
        total_minObjectCoord = fi.minObjectCoord;
        total_maxObjectCoord = fi.maxObjectCoord;
        total_minSubVolumeCoord = fi.minSubVolumeCoord;
        total_maxSubVolumeCoord = fi.maxSubVolumeCoord;
        total_minValue = fi.minValue;
        total_maxValue = fi.maxValue;
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
        FilterInfo fi;
        fi.LoadPFI( fbuff );

        if ( m_list.empty() )
        {
            total_numNodes = fi.numNodes;
            total_numElements = fi.numElements;
            total_numFiles = fi.numFiles;
            total_staSteps = fi.staSteps;
            total_endSteps = fi.endSteps;
            total_numSteps = fi.numSteps;
            total_numSubVolumes = fi.numSubVolumes;
            total_minObjectCoord = fi.minObjectCoord;
            total_maxObjectCoord = fi.maxObjectCoord;
            total_minSubVolumeCoord = fi.minSubVolumeCoord;
            total_maxSubVolumeCoord = fi.maxSubVolumeCoord;
            total_minValue = fi.minValue;
            total_maxValue = fi.maxValue;
        }
        else
        {
            total_numNodes += fi.numNodes;
            total_numElements += fi.numElements;
            total_numFiles += fi.numFiles;
            total_staSteps = std::min( total_staSteps, fi.staSteps );
            total_endSteps = std::max( total_endSteps, fi.endSteps );
            total_numSteps = total_endSteps - total_staSteps + 1;
            total_numSubVolumes += fi.numSubVolumes;
            total_minObjectCoord[0]
                = std::min( total_minObjectCoord[0], fi.minObjectCoord[0] );
            total_minObjectCoord[1]
                = std::min( total_minObjectCoord[1], fi.minObjectCoord[1] );
            total_minObjectCoord[2]
                = std::min( total_minObjectCoord[2], fi.minObjectCoord[2] );
            total_maxObjectCoord[0]
                = std::max( total_maxObjectCoord[0], fi.maxObjectCoord[0] );
            total_maxObjectCoord[1]
                = std::max( total_maxObjectCoord[1], fi.maxObjectCoord[1] );
            total_maxObjectCoord[2]
                = std::max( total_maxObjectCoord[2], fi.maxObjectCoord[2] );
            std::copy( fi.minSubVolumeCoord.begin(), fi.minSubVolumeCoord.end(),
                       std::back_inserter( total_minSubVolumeCoord ) );
            std::copy( fi.maxSubVolumeCoord.begin(), fi.maxSubVolumeCoord.end(),
                       std::back_inserter( total_maxSubVolumeCoord ) );
            total_minValue = std::min( total_minValue, fi.minValue );
            total_maxValue = std::max( total_maxValue, fi.maxValue );
        }

        m_list.push_back( fi );
    } // end of while()
    fin.close();

    return m_list.size();
}

int FilterInfoLst::GetFileIdx( int vl, int& xvl ) const
{
    if ( m_list.empty() ) return -1;
    if ( vl >= total_numSubVolumes ) return -1;

    int idx, cvl = vl;
    for ( idx = 0; idx < m_list.size(); idx++ )
    {
        if ( cvl < m_list[idx].numSubVolumes ) break;
        cvl -= m_list[idx].numSubVolumes;
    }
    if ( idx >= m_list.size() ) return -1;
    xvl = cvl;
    return idx;
}

void FilterInfoLst::CropTimestep( int s, int e )
{
    if ( m_list.empty() ) return;
    if ( s > e ) return;

    if ( s != INT_MIN )
    {
        if ( s > total_staSteps )
            total_staSteps = s;
    }

    if ( e != INT_MAX )
    {
        if ( e < total_endSteps )
            total_endSteps = e;
    }

    total_numSteps = total_endSteps - total_staSteps + 1;
}
