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
const kvs::Int32 num_element_node = 4;

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

    const kvs::Real64 dim= 128.0;
    //const kvs::Real64 dim= dim1;
    const kvs::Real64 kr = 32.0 / dim;
    const kvs::Real64 kd = 1.0 + (float)mpi_rank;
    const kvs::Real64 kr3 = 32.0 / dim * 0.1667 * kd;
    //const kvs::Real64 kd = 6;

    //ncells = 1;
    //nnodes = 4;
    ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 ) * 5 ;
    nnodes = dim1 *  dim2  * dim3;
    //ncells = 2;
    //nnodes = 8;

    connections = new unsigned int[ ncells * num_element_node ]; // 5tetra cell per 1cube
    coords      = new float[ nnodes * num_dimentions ];
    values      = new float*[ nvariables ];

    for(int i=0; i<nvariables; i++)
    {
        values[i] = new float[nnodes];
    }

    kvs::UInt64 index = 0;
    kvs::UInt64 coords_index = 0;

#if 0
    ncells = 2;
    nnodes = 8;
    //values[0][0] = 0+0.1*mpi_rank;
    //values[0][1] = 1+0.1*mpi_rank;
    //values[0][2] = 2+0.1*mpi_rank;
    //values[0][3] = 3+0.1*mpi_rank;
    values[0][0] = 0;
    values[0][1] = 0;
    values[0][2] = 0;
    values[0][3] = 0;

    coords[0] = 0.f; coords[1] = 0.f; coords[2] = 0.f;     // p0
    coords[3] = 1.f; coords[4] = 0.f; coords[5] = 0.f;     // p1
    coords[6] = 0.f; coords[7] = 1.f; coords[8] = 0.f;     // p2
    coords[9] = 0.f; coords[10] = 0.f; coords[11] = 1.f;     // p3

    connections[ 0 ] = 1; 
    connections[ 1 ] = 2;
    connections[ 2 ] = 3;
    connections[ 3 ] = 0;

    values[0][4] = 4+0.1*mpi_rank;
    values[0][5] = 5+0.1*mpi_rank;
    values[0][6] = 6+0.1*mpi_rank;
    values[0][7] = 7+0.1*mpi_rank;

    coords[12] = 0.f; coords[13] = 0.f; coords[14] = 1.f;     // p0
    coords[15] = 1.f; coords[16] = 0.f; coords[17] = 1.f;     // p1
    coords[18] = 0.f; coords[19] = 1.f; coords[20] = 1.f;     // p2
    coords[21] = 0.f; coords[22] = 0.f; coords[23] = 2.f;     // p3

    connections[ 4 ] = 5; 
    connections[ 5 ] = 6;
    connections[ 6 ] = 7;
    connections[ 7 ] = 4;

#else
    for ( kvs::UInt64 k = 0; k < dim3; ++k )
    {
        for ( kvs::UInt64 j = 0; j < dim2; ++j )
        {
            for ( kvs::UInt64 i = 0; i < dim1; ++i )
            {
                //const float x = (float)i * cell_length + global_region[mpi_rank].x();
                //const float y = (float)j * cell_length + global_region[mpi_rank].y();
                const float x = (float)i * cell_length;
                const float y = (float)j * cell_length;
                const float z = (float)k * cell_length;

                const kvs::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                const kvs::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                const kvs::Real64 dz = kr3* ( z - ( dim / 2.0 ) );


                const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                const kvs::Real64 cos_theta = dz / r;
                const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                const kvs::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

                values[0][index] = static_cast<float>( c );
//                values[1][index] = static_cast<float>( c );
//                values[2][index] = static_cast<float>( c );
//                values[3][index] = static_cast<float>( c );
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z;
            }
        }
    }

  //tetra 
    kvs::UInt64 line_size  = static_cast<kvs::UInt32>( dim1 );
    kvs::UInt64 slice_size = static_cast<kvs::UInt32>( dim1 * dim2 );
    kvs::UInt64 vertex_index = 0;
    kvs::UInt64 connection_index = 0;
    for ( size_t z = 0; z < dim3-1; ++z )
    {
        for ( size_t y = 0; y < dim2-1; ++y )
        {
            for ( size_t x = 0; x < dim1-1; ++x )
            {
                //for( int i = 0; i< 5 ; i++ )
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

                if ( (x+y+z)%2 == 0 )
                {
                // hexahedra-1
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );

                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                }
                else if ( (x+y+z) %2 == 1 )
                {
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );

                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
 
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
                connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
                }
                vertex_index++;
            }
            vertex_index++;
        }
        vertex_index += line_size;
    }
#endif
    return( 1 );
}

void Hydrogen::calc_each_ens(int mpi_size)
{
    const kvs::UInt64 dim1 = resolution.x();
    const kvs::UInt64 dim2 = resolution.y();
    const kvs::UInt64 dim3 = resolution.z();

        const kvs::Real64 dim= 128.0;
        const kvs::Real64 kr = 32.0 / dim;
        //const kvs::Real64 kd = 1.5 + 0.5f*(float)mpi_size;
        const kvs::Real64 kd = 1.f +(float)mpi_size;
        //const kvs::Real64 kd = 6;
        const kvs::Real64 kr3 = 32.0 / dim * 0.1667 * kd;

        ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 ) * 5 ;
        nnodes = dim1 *  dim2  * dim3;

        kvs::UInt64 index = 0;
        kvs::UInt64 coords_index = 0;

#if 0
#else
        for ( kvs::UInt64 k = 0; k < dim3; ++k )
        {
            for ( kvs::UInt64 j = 0; j < dim2; ++j )
            {
                for ( kvs::UInt64 i = 0; i < dim1; ++i )
                {
                    //const float x = (float)i * cell_length + global_region[mpi_rank].x();
                    //const float y = (float)j * cell_length + global_region[mpi_rank].y();
                    const float x = (float)i * cell_length;
                    const float y = (float)j * cell_length;
                    const float z = (float)k * cell_length;

                    const kvs::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                    const kvs::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                    const kvs::Real64 dz = kr3* ( z - ( dim / 2.0 ) );



                    const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                    const kvs::Real64 cos_theta = dz / r;
                    const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                    const kvs::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

                    values[0][index] = static_cast<float>( c );
                    //values[0][index] += static_cast<float>( c )/mpi_size;
                    //                std::cout << "values[0][index] = " << values[0][index] << std::endl;
                    //                values[1][index] = static_cast<float>( c );
                    //                values[2][index] = static_cast<float>( c );
                    //                values[3][index] = static_cast<float>( c );
                    index++;
                    //                coords[ coords_index++ ] = x;
                    //                coords[ coords_index++ ] = y;
                    //                coords[ coords_index++ ] = z;
                }
            }
        }
#endif


}
void Hydrogen::calc_average(int mpi_size)
{
    const kvs::UInt64 dim1 = resolution.x();
    const kvs::UInt64 dim2 = resolution.y();
    const kvs::UInt64 dim3 = resolution.z();

    for (int step =0; step< mpi_size; step++)
    {
        const kvs::Real64 dim= 128.0;
        const kvs::Real64 kr = 32.0 / dim;
        //const kvs::Real64 kd = 1.5 + 0.5f*(float)mpi_size;
        const kvs::Real64 kd = 1.f +(float)step;
        //const kvs::Real64 kd = 6;
        const kvs::Real64 kr3 = 32.0 / dim * 0.1667 * kd;

        //ncells = 1;
        //nnodes = 4;
        ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 ) * 5 ;
        nnodes = dim1 *  dim2  * dim3;
        //ncells = 2;
        //nnodes = 8;

        //    connections = new unsigned int[ ncells * num_element_node ]; // 5tetra cell per 1cube
        //    coords      = new float[ nnodes * num_dimentions ];
        //    values      = new float*[ nvariables ];
        //
        //    for(int i=0; i<nvariables; i++)
        //    {
        //        values[i] = new float[nnodes];
        //    }

        kvs::UInt64 index = 0;
        kvs::UInt64 coords_index = 0;

#if 0
#else
        for ( kvs::UInt64 k = 0; k < dim3; ++k )
        {
            for ( kvs::UInt64 j = 0; j < dim2; ++j )
            {
                for ( kvs::UInt64 i = 0; i < dim1; ++i )
                {
                    //const float x = (float)i * cell_length + global_region[mpi_rank].x();
                    //const float y = (float)j * cell_length + global_region[mpi_rank].y();
                    const float x = (float)i * cell_length;
                    const float y = (float)j * cell_length;
                    const float z = (float)k * cell_length;

                    const kvs::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                    const kvs::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                    const kvs::Real64 dz = kr3* ( z - ( dim / 2.0 ) );



                    const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                    const kvs::Real64 cos_theta = dz / r;
                    const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                    const kvs::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

                    values[0][index] += static_cast<float>( c )/mpi_size;
                    //                std::cout << "values[0][index] = " << values[0][index] << std::endl;
                    //                values[1][index] = static_cast<float>( c );
                    //                values[2][index] = static_cast<float>( c );
                    //                values[3][index] = static_cast<float>( c );
                    index++;
                    //                coords[ coords_index++ ] = x;
                    //                coords[ coords_index++ ] = y;
                    //                coords[ coords_index++ ] = z;
                }
            }
        }
#endif
    }


}

/*===========================================================================*/
/**
 *  @brief  Constructs a new hydrogen class.
 *  @param  resolution [in] volume resolution
 */
/*===========================================================================*/
Hydrogen::Hydrogen( void )
{
    nvariables = 1;

    // Including halo region in x-y boundary
    //resolution = kvs::Vector3ui( 2, 2, 2 );
    //resolution = kvs::Vector3ui( 4, 4, 8 );
    resolution = kvs::Vector3ui( 32, 32, 32 );
    //resolution = kvs::Vector3ui( 64, 64, 64 );

    //cell_length = 16.0;
    //cell_length = 1.0;
    //cell_length = 2.0;
    cell_length = 4.0;

//    global_region[ 0 ] = kvs::Vector2f(  0,  0 );
//    global_region[ 1 ] = kvs::Vector2f( 63,  0 );
//    global_region[ 2 ] = kvs::Vector2f( 63, 63 );
//    global_region[ 3 ] = kvs::Vector2f(  0, 63 );

    global_min_coord = kvs::Vector3f( 0, 0, 0 );
    
    //global_max_coord = kvs::Vector3f( 31, 31, 31 );
    global_max_coord = kvs::Vector3f( 127, 127, 127 );
    //global_max_coord = kvs::Vector3f( 1, 1, 1 );
    //global_max_coord = kvs::Vector3f( 1, 1, 2 );

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

    unsigned int con_min = 0;
    unsigned int con_max = 0;
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
