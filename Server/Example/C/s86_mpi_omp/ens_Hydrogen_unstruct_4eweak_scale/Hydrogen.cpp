/*****************************************************************************/
/**
 *  @file   Hydrogen.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/vismodule/copyright/ for details.
 *
 *  $Id: Hydrogen.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Hydrogen.h"
#include <vismodule/AnyValueArray>
#include <vismodule/ValueArray>
#include <vismodule/Vector3>
#include <algorithm>
#include <cstdlib>
#include <limits>



const vismodule::Int32 num_dimentions = 3;
const vismodule::Int32 num_element_node = 8;

namespace
{
int weak_base_ensemble_from_env()
{
    const char* env = std::getenv( "PBVR_WEAK_BASE_ENS" );
    const int value = env ? std::atoi( env ) : 2;
    return value > 0 ? value : 2;
}

}

/*===========================================================================*/
/**
 *  @brief  Generates hydrogen volume.
 *  @param  resolution [in] volume resolution
 *  @return value array of the hydrogen volume
 */
/*===========================================================================*/
int Hydrogen::generate_volume( void )
{
    const vismodule::UInt64 dim1 = resolution.x();
    const vismodule::UInt64 dim2 = resolution.y();
    const vismodule::UInt64 dim3 = resolution.z();

    int ens_id = effective_ens_id;
    const vismodule::Real64 dim= 128.0;
    const vismodule::Real64 kr = 32.0 / dim;
//    const vismodule::Real64 kd = 6.0;
    const vismodule::Real64 kd = weak_kd;
    const vismodule::Real64 kr3 = 32.0 / dim * 0.1667 * kd;

    ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 );
    nnodes = dim1 *  dim2  * dim3;

    connections = new unsigned int[ ncells * num_element_node ];
    coords      = new float[ nnodes * num_dimentions ];
    values      = new float*[ nvariables ];

    for(int i=0; i<nvariables; i++)
    {
        values[i] = new float[nnodes];
    }

    vismodule::UInt64 index = 0;
    vismodule::UInt64 coords_index = 0;
    value_min = std::numeric_limits<float>::max();
    value_max = -std::numeric_limits<float>::max();

    for ( vismodule::UInt64 k = 0; k < dim3; ++k )
    {
        for ( vismodule::UInt64 j = 0; j < dim2; ++j )
        {
            for ( vismodule::UInt64 i = 0; i < dim1; ++i )
            {

                const float x = (float)i * cell_length + global_region[ens_id].x();
                //const float y = (float)j * cell_length + global_region[mpi_rank].y();
                //const float x = (float)i * cell_length ;
                const float y = (float)j * cell_length ;
                const float z = (float)k * cell_length;

//                if(k == 0) 
//                {
//                    std::cout << "x = " << x << std::endl;
//                }
                const vismodule::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                const vismodule::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                const vismodule::Real64 dz = kr3 * ( z - ( dim / 2.0 ) );
//
//
                const vismodule::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                const vismodule::Real64 cos_theta = dz / r;
                const vismodule::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                const vismodule::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);
//                values[0][index] = index + mpi_rank*0.1;
//                values[0][index] = mpi_rank*0.1;
//                values[1][index] = mpi_rank*0.1;
//                values[2][index] = mpi_rank*0.1;
//                values[3][index] = mpi_rank*0.1;
//                values[0][index] = x;
                const float value = static_cast<float>( c );
                values[0][index] = value;
                values[1][index] = value;
                value_min = std::min( value_min, value );
                value_max = std::max( value_max, value );
//                values[2][index] = static_cast<float>( c );
//                values[3][index] = static_cast<float>( c );
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z;
            }
        }
    }

    vismodule::UInt64 line_size  = static_cast<vismodule::UInt32>( dim1 );
    vismodule::UInt64 slice_size = static_cast<vismodule::UInt32>( dim1 * dim2 );
    vismodule::UInt64 vertex_index = 0;
    vismodule::UInt64 connection_index = 0;
    for ( size_t z = 0; z < dim3-1; ++z )
    {
        for ( size_t y = 0; y < dim2-1; ++y )
        {
            for ( size_t x = 0; x < dim1-1; ++x )
            {
                const vismodule::UInt64 local_vertex_index[8] =
                {
                    vertex_index,
                    vertex_index + 1,
                    vertex_index + line_size,
                    vertex_index + line_size + 1,
                    vertex_index + slice_size,
                    vertex_index + slice_size + 1,
                    vertex_index + slice_size + line_size,
                    vertex_index + slice_size + line_size + 1
                };

                // hexahedra-1
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
 
                vertex_index++;
            }
            vertex_index++;
        }
        vertex_index += line_size;
    }
    return( 1 );
}


/*===========================================================================*/
/**
 *  @brief  Constructs a new hydrogen class.
 *  @param  resolution [in] volume resolution
 */
/*===========================================================================*/
Hydrogen::Hydrogen( void )
{
    nvariables = 2;

    // Including halo region in x-y boundary
    //resolution = vismodule::Vector3ui( 64, 64, 128 );
//    resolution = vismodule::Vector3ui( 4, 4, 4 );
//    resolution = vismodule::Vector3ui( 128, 128, 128 );
    resolution = vismodule::Vector3ui( 128, 256, 128 );
//    resolution = vismodule::Vector3ui( 256, 256, 256 );

    //cell_length = 1.0;
    cell_length = 127.f/(resolution.x()-1);

    global_region[ 0 ] = vismodule::Vector2f(  0,  0 );
    global_region[ 1 ] = vismodule::Vector2f( 63,  0 );
//    global_region[ 2 ] = vismodule::Vector2f( 63, 63 );
//    global_region[ 3 ] = vismodule::Vector2f(  0, 63 );

    global_min_coord = vismodule::Vector3f( 0, 0, 0 );
    global_max_coord = vismodule::Vector3f( 127, 127, 127 );
//    global_max_coord = vismodule::Vector3f( 1, 1, 1 );

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    weak_base_ens = weak_base_ensemble_from_env();
    effective_ens_id = mpi_rank % weak_base_ens;
    weak_kd = 1.0f + static_cast<float>( effective_ens_id );

    this->generate_volume();
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new hydrogen class.
 *  @param  resolution [in] volume resolution
 */
/*===========================================================================*/
void Hydrogen::show( void )
{
    std::cout<<"-------show volume data------"<<std::endl;
    std::cout<<"mpi_rank="<<mpi_rank<<std::endl;
    std::cout<<"weak_base_ens="<<weak_base_ens<<std::endl;
    std::cout<<"effective_ens_id="<<effective_ens_id<<std::endl;
    std::cout<<"kd="<<weak_kd<<std::endl;
    std::cout<<"resolution="<<resolution<<std::endl;
    std::cout<<"nvariables="<<nvariables<<std::endl;
    if ( mpi_rank < 4 )
    {
        std::cout<<"global_region="<<global_region[mpi_rank]<<std::endl;
    }
    std::cout<<"ncells="<<ncells<<std::endl;
    std::cout<<"nnodes="<<nnodes<<std::endl;

    unsigned int con_min = 17;
    unsigned int con_max = 17;
    for(int i=0; i<ncells * num_element_node; i++)
    {
        con_min = con_min < connections[i] ? con_min : connections[i];
        con_max = con_max > connections[i] ? con_max : connections[i];
    }
    std::cout<<"connection min = "<<con_min<<std::endl;
    std::cout<<"connection max = "<<con_max<<std::endl;

    float coord_min = 17;
    float coord_max = 17;
    for(int i=0; i<nnodes * num_dimentions; i++)
    {
        coord_min = coord_min < coords[i] ? coord_min : coords[i];
        coord_max = coord_max > coords[i] ? coord_max : coords[i];
    }
    std::cout<<"coord min = "<<coord_min<<std::endl;
    std::cout<<"coord max = "<<coord_max<<std::endl;

    std::cout<<"value min = "<<value_min<<std::endl;
    std::cout<<"value max = "<<value_max<<std::endl;
    std::cout<<"-------end show volume data------"<<std::endl;
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Hydrogen class.
 */
/*===========================================================================*/
Hydrogen::~Hydrogen( void )
{
    delete [] values;
    delete [] connections;
    delete [] coords;
}
