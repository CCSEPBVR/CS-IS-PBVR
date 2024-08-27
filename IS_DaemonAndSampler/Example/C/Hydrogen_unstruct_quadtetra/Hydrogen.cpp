/*****************************************************************************/
/**
 *  @file   Hydrogen.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Hydrogen.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Hydrogen.h"
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Vector3>



const kvs::Int32 num_dimentions = 3;
const kvs::Int32 num_element_node = 10;
const kvs::Int32 num_cell_per_cube = 5 ; //num of cell per 1cube
/*===========================================================================*/
/**
 *  @brief  Generates hydrogen volume.
 *  @param  resolution [in] volume resolution
 *  @return value array of the hydrogen volume
 */
/*===========================================================================*/
int Hydrogen::generate_volume( void )
{
    const kvs::UInt64 dim1 = resolution.x();
    const kvs::UInt64 dim2 = resolution.y();
    const kvs::UInt64 dim3 = resolution.z();


    ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 ) * num_cell_per_cube;
    nnodes = dim1 *  dim2  * dim3 + ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 ) * 18;

    connections = new unsigned int[ ncells * num_element_node  ];
    coords      = new float[ nnodes * num_dimentions ];
    values      = new float*[ nvariables ];

    for(int i=0; i<nvariables; i++)
    {
        values[i] = new float[nnodes];
    }

    kvs::UInt64 index = 0;
    kvs::UInt64 coords_index = 0;

    for ( kvs::UInt64 k = 0; k < resolution.z(); ++k )
    {
        for ( kvs::UInt64 j = 0; j < resolution.y(); ++j )
        {
            for ( kvs::UInt64 i = 0; i < resolution.x(); ++i )
            {
                const float x = (float)i * cell_length + global_region[mpi_rank].x();
                const float y = (float)j * cell_length + global_region[mpi_rank].y();
                const float z = (float)k * cell_length;

                values[0][index] = calc(x,y,z);
                values[1][index] = calc(x,y,z);
                values[2][index] = calc(x,y,z);
                values[3][index] = calc(x,y,z);
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z;
 
            }
        }
    }

    //insert  half point
    for ( kvs::UInt64 k = 0; k < resolution.z()-1; ++k )
    {
        for ( kvs::UInt64 j = 0; j < resolution.y()-1; ++j )
        {
            for ( kvs::UInt64 i = 0; i < resolution.x()-1; ++i )
            {
                const float x   = (float)i * cell_length + global_region[mpi_rank].x();
                const float y   = (float)j * cell_length + global_region[mpi_rank].y();
                const float z   = (float)k * cell_length;
                const float xp1 = (float)(i+1) * cell_length + global_region[mpi_rank].x();
                const float yp1 = (float)(j+1) * cell_length + global_region[mpi_rank].y();
                const float zp1 = (float)(k+1) * cell_length;
                const float x_half = (float)(i+0.5) * cell_length + global_region[mpi_rank].x();
                const float y_half = (float)(j+0.5) * cell_length + global_region[mpi_rank].y();
                const float z_half = (float)(k+0.5) * cell_length;

                //(x+0.5,y,z)
                values[0][index] = calc(x_half,y,z);
                values[1][index] = calc(x_half,y,z);
                values[2][index] = calc(x_half,y,z);
                values[3][index] = calc(x_half,y,z);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z;

                //(x+1,y+0.5,z)
                values[0][index] = calc(xp1,y_half,z);
                values[1][index] = calc(xp1,y_half,z);
                values[2][index] = calc(xp1,y_half,z);
                values[3][index] = calc(xp1,y_half,z);
                index++;
                coords[ coords_index++ ] = xp1;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = z;

                //(x+0.5,y+1,z)
                values[0][index] = calc(x_half,yp1,z);
                values[1][index] = calc(x_half,yp1,z);
                values[2][index] = calc(x_half,yp1,z);
                values[3][index] = calc(x_half,yp1,z);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = yp1;
                coords[ coords_index++ ] = z;
                
                //(x,y+0.5,z)
                values[0][index] = calc(x,y_half,z);
                values[1][index] = calc(x,y_half,z);
                values[2][index] = calc(x,y_half,z);
                values[3][index] = calc(x,y_half,z);
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = z;

                //(x+0.5,y+0.5,z)
                values[0][index] = calc(x_half,y_half,z);
                values[1][index] = calc(x_half,y_half,z);
                values[2][index] = calc(x_half,y_half,z);
                values[3][index] = calc(x_half,y_half,z);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = z;
                
                //(x+0.5,y,z+1)
                values[0][index] = calc(x_half,y,zp1);
                values[1][index] = calc(x_half,y,zp1);
                values[2][index] = calc(x_half,y,zp1);
                values[3][index] = calc(x_half,y,zp1);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = zp1;
                
                //(x+1,y+0.5,z+1)
                values[0][index] = calc(xp1,y_half,zp1);
                values[1][index] = calc(xp1,y_half,zp1);
                values[2][index] = calc(xp1,y_half,zp1);
                values[3][index] = calc(xp1,y_half,zp1);
                index++;
                coords[ coords_index++ ] = xp1;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = zp1;
                
                //(x+0.5,y+1,z+1)
                values[0][index] = calc(x_half,yp1,zp1);
                values[1][index] = calc(x_half,yp1,zp1);
                values[2][index] = calc(x_half,yp1,zp1);
                values[3][index] = calc(x_half,yp1,zp1);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = yp1;
                coords[ coords_index++ ] = zp1;
                
                //(x,y+0.5,z+1)
                values[0][index] = calc(x,y_half,zp1);
                values[1][index] = calc(x,y_half,zp1);
                values[2][index] = calc(x,y_half,zp1);
                values[3][index] = calc(x,y_half,zp1);
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = zp1;
                
                //(x+0.5,y+0.5,z+1)
                values[0][index] = calc(x_half,y_half,zp1);
                values[1][index] = calc(x_half,y_half,zp1);
                values[2][index] = calc(x_half,y_half,zp1);
                values[3][index] = calc(x_half,y_half,zp1);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = zp1;

                //(x,y,z+0.5)
                values[0][index] = calc(x,y,z_half);
                values[1][index] = calc(x,y,z_half);
                values[2][index] = calc(x,y,z_half);
                values[3][index] = calc(x,y,z_half);
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z_half;
                
                //(x+1,y,z+0.5)
                values[0][index] = calc(xp1,y,z_half);
                values[1][index] = calc(xp1,y,z_half);
                values[2][index] = calc(xp1,y,z_half);
                values[3][index] = calc(xp1,y,z_half);
                index++;
                coords[ coords_index++ ] = xp1;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z_half;
                
                //(x+1,y+1,z+0.5)
                values[0][index] = calc(xp1,yp1,z_half);
                values[1][index] = calc(xp1,yp1,z_half);
                values[2][index] = calc(xp1,yp1,z_half);
                values[3][index] = calc(xp1,yp1,z_half);
                index++;
                coords[ coords_index++ ] = xp1;
                coords[ coords_index++ ] = yp1;
                coords[ coords_index++ ] = z_half;
                
                //(x,y+1,z+0.5)
                values[0][index] = calc(x,yp1,z_half);
                values[1][index] = calc(x,yp1,z_half);
                values[2][index] = calc(x,yp1,z_half);
                values[3][index] = calc(x,yp1,z_half);
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = yp1;
                coords[ coords_index++ ] = z_half;

                //(x+0.5,y,z+0.5)
                values[0][index] = calc(x_half,y,z_half);
                values[1][index] = calc(x_half,y,z_half);
                values[2][index] = calc(x_half,y,z_half);
                values[3][index] = calc(x_half,y,z_half);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z_half;
                
                //(x+1,y+0.5,z+0.5)
                values[0][index] = calc(xp1,y_half,z_half);
                values[1][index] = calc(xp1,y_half,z_half);
                values[2][index] = calc(xp1,y_half,z_half);
                values[3][index] = calc(xp1,y_half,z_half);
                index++;
                coords[ coords_index++ ] = xp1;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = z_half;
                
                //(x+0.5,y+1,z+0.5)
                values[0][index] = calc(x_half,yp1,z_half);
                values[1][index] = calc(x_half,yp1,z_half);
                values[2][index] = calc(x_half,yp1,z_half);
                values[3][index] = calc(x_half,yp1,z_half);
                index++;
                coords[ coords_index++ ] = x_half;
                coords[ coords_index++ ] = yp1;
                coords[ coords_index++ ] = z_half;
                
                //(x,y+0.5,z+0.5)
                values[0][index] = calc(x,y_half,z_half);
                values[1][index] = calc(x,y_half,z_half);
                values[2][index] = calc(x,y_half,z_half);
                values[3][index] = calc(x,y_half,z_half);
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y_half;
                coords[ coords_index++ ] = z_half;
            }
        }
    }

    std::cout << "index = " << index << std::endl;

    kvs::UInt64 line_size  = static_cast<kvs::UInt32>( dim1 );
    kvs::UInt64 slice_size = static_cast<kvs::UInt32>( dim1 * dim2 );
    kvs::UInt64 vertex_index = 0;
    kvs::UInt64 half_vertex_index = 0;
    kvs::UInt64 connection_index = 0;
    for ( size_t z = 0; z < dim3-1; ++z )
    {
        for ( size_t y = 0; y < dim2-1; ++y )
        {
            for ( size_t x = 0; x < dim1-1; ++x )
            {
                const kvs::UInt64 local_vertex_index[8] =
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

                const kvs::UInt64 local_halfpoint_vertex_index[18] =
                {
                    half_vertex_index*18 + (dim1 *  dim2  * dim3)    ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +1 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +2 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +3 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +4 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +5 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +6 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +7 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +8 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +9 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +10 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +11 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +12 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +13 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +14 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +15 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +16 ,
                    half_vertex_index*18 + (dim1 *  dim2  * dim3) +17 ,
                };
                //quad tetra vtk 
//#ifdef VTK 
//                if ( (x+y+z)%2 == 0 )
//                {
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 11] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
//
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 12] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
// 
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 8 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 10] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
// 
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 13] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 2] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
// 
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
//                }
//                else if ( (x+y+z) %2 == 1 )
//                {
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 5 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 11] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
//
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 0 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 10] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
// 
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 2 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 12] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
// 
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 8 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 13] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
// 
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
//                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
//                }
//#else
                // quad tetra kvs
                if ( (x+y+z)%2 == 0 )
                {
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 11] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );

                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 12] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 8 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 10] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 13] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
                }
                else if ( (x+y+z) %2 == 1 )
                {
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 11] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );

                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 0] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 10] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 12] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 8] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 13] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 14] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 9 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 17] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 15] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 16] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_halfpoint_vertex_index[ 4 ] );
                }
//#endif
                vertex_index++;
                half_vertex_index++;
            }
            vertex_index++;
        }
        vertex_index += line_size;
    }
    return( 1 );
}

/*===========================================================================*/
/**
 *  @brief  calculate hydrogen electric density.
 *  @param  resolution [in] volume resolution
 *  @return value float 
 */
/*===========================================================================*/
float Hydrogen::calc(const float x, const float y, const float z)
{
                const kvs::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                const kvs::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                const kvs::Real64 dz = kr * ( z - ( dim / 2.0 ) );

                const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                const kvs::Real64 cos_theta = dz / r;
                const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                const kvs::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

                return static_cast<float>(c);
                //return c;
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
    resolution = kvs::Vector3ui( 64, 64, 128 );
    //resolution = kvs::Vector3ui( 32, 32, 64 );
    //resolution = kvs::Vector3ui( 63, 63, 127 );
    //resolution = kvs::Vector3ui( 8, 8, 16 );
    //resolution = kvs::Vector3ui( 4, 4, 8 );
    //resolution = kvs::Vector3ui( 2, 2, 4 );

    cell_length = 1.0;
    //cell_length = 2.0;
    //cell_length = 8.0;
    //cell_length = 16.0;
    //cell_length = 32.0;

    //global_region[ 0 ] = kvs::Vector2f(  0,  0 );
    //global_region[ 1 ] = kvs::Vector2f( 62,  0 );
    //global_region[ 2 ] = kvs::Vector2f( 62, 62 );
    //global_region[ 3 ] = kvs::Vector2f(  0, 62 );

    //global_min_coord = kvs::Vector3f( 0, 0, 0 );
    //global_max_coord = kvs::Vector3f( 124, 124, 126 );
    
    global_region[ 0 ] = kvs::Vector2f(  0,  0 );
    global_region[ 1 ] = kvs::Vector2f( 63,  0 );
    global_region[ 2 ] = kvs::Vector2f( 63, 63 );
    global_region[ 3 ] = kvs::Vector2f(  0, 63 );

    global_min_coord = kvs::Vector3f( 0, 0, 0 );
    global_max_coord = kvs::Vector3f( 126, 126, 127 );

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

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
    std::cout<<"resolution="<<resolution<<std::endl;
    std::cout<<"nvariables="<<nvariables<<std::endl;
    std::cout<<"global_region="<<global_region[mpi_rank]<<std::endl;
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

    float value_min = 17;
    float value_max = 17;
    for(int i=0; i< nnodes; i++)
    {
        value_min = value_min < values[0][i] ? value_min : values[0][i];
        value_max = value_max > values[0][i] ? value_max : values[0][i];
    }
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
