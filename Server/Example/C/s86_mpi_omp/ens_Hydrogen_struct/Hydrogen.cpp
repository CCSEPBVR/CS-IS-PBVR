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
#include <vismodule/Vector3>

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

    const vismodule::Real64 dim= 128.0;
    const vismodule::Real64 kr = 32.0 / dim;
    // アンサンブルメンバ(mpi_rank)ごとに波動関数の分布を変える(非構造版 ens_Hydrogen_unstruct と同じ)
    int ens_id = mpi_rank;
    const vismodule::Real64 kd = 1.0 + (float)(ens_id);
    const vismodule::Real64 kr3 = 32.0 / dim * 0.1667 * kd;

    values = new float*[ nvariables ];

    for(int i=0; i<nvariables; i++)
    {
        values[i] = new float[dim1*dim2*dim3];
    }

    vismodule::UInt64 index = 0;

    for ( vismodule::UInt64 k = 0; k < dim3; ++k )
    {
        for ( vismodule::UInt64 j = 0; j < dim2; ++j )
        {
            for ( vismodule::UInt64 i = 0; i < dim1; ++i )
            {
//                const float x = (float)i * cell_length + global_region[mpi_rank].x();
//                const float y = (float)j * cell_length + global_region[mpi_rank].y();
                const float x = (float)i * cell_length;
                const float y = (float)j * cell_length;
                const float z = (float)k * cell_length;

                const vismodule::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                const vismodule::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                const vismodule::Real64 dz = kr3 * ( z - ( dim / 2.0 ) );


                const vismodule::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                const vismodule::Real64 cos_theta = dz / r;
                const vismodule::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                const vismodule::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

                values[0][index] = static_cast<float>( c );
                values[1][index] = static_cast<float>( c );
                values[2][index] = static_cast<float>( c );
                values[3][index] = static_cast<float>( c );
                index++;
            }
        }
    }

    return 1;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new hydrogen class.
 *  @param  resolution [in] volume resolution
 */
/*===========================================================================*/
Hydrogen::Hydrogen( void )
{
    nvariables = 4;

    // Including halo region in x-y boundary
    //resolution = vismodule::Vector3i( 64, 64, 128 );
    resolution = vismodule::Vector3i( 128, 128, 128 );

    cell_length = 1.0;

    global_region[ 0 ] = vismodule::Vector2f(  0,  0 );
    global_region[ 1 ] = vismodule::Vector2f( 63,  0 );
    global_region[ 2 ] = vismodule::Vector2f( 63, 63 );
    global_region[ 3 ] = vismodule::Vector2f(  0, 63 );

    global_min_coord = vismodule::Vector3f( 0, 0, 0 );
    global_max_coord = vismodule::Vector3f( 127, 127, 127 );

#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );
#else
    mpi_rank = 0;
#endif

    this->generate_volume();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Hydrogen class.
 */
/*===========================================================================*/
Hydrogen::~Hydrogen( void )
{
    for(int i=0; i<nvariables; i++)
    {
        delete [] values[i];
    }

    delete [] values;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Hydrogen class.
 */
/*===========================================================================*/
void Hydrogen::show( void )
{
    std::cout<<"-------show volume data------"<<std::endl;
    std::cout<<"mpi_rank="<<mpi_rank<<std::endl;
    std::cout<<"resolution="<<resolution<<std::endl;
    std::cout<<"nvariables="<<nvariables<<std::endl;
    std::cout<<"global_region="<<global_region[mpi_rank]<<std::endl;

    float value_min = 17;
    float value_max = 17;
    for(int i=0; i< resolution.x()*resolution.y()*resolution.z(); i++)
    {
        value_min = value_min < values[0][i] ? value_min : values[0][i];
        value_max = value_max > values[0][i] ? value_max : values[0][i];
    }
    std::cout<<"value min = "<<value_min<<std::endl;
    std::cout<<"value max = "<<value_max<<std::endl;
    std::cout<<"-------end show volume data------"<<std::endl;
}
