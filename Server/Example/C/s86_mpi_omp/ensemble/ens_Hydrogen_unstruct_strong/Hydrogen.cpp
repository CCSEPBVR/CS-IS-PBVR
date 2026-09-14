/*****************************************************************************/
/**
 *  @file   Hydrogen.cpp
 *  @brief  Strong-scaling variant of the Hydrogen volume generator.
 *
 *  Design (single executable, N fixed):
 *    - GX x GY nodes in x,y and GZ_CELLS cells in z are FIXED constants.
 *    - num_ensembles is FIXED (default 4, override with env PBVR_STRONG_ENS).
 *    - mpi_per_ens = mpi_size / num_ensembles is derived at runtime.
 *    - One ensemble's z extent is partitioned into mpi_per_ens contiguous slabs
 *      (z-axis 1D decomposition). Each rank builds resolution (GX,GY,chunk+1),
 *      where chunk = GZ_CELLS / mpi_per_ens, offset by origin_z.
 *    - The union of all ranks in an ensemble is always GX x GY x GZ_CELLS,
 *      so the total problem size N does not change with the rank count.
 *    - The scalar field is evaluated in GLOBAL coordinates, so values (and the
 *      resulting particle count) are independent of how the grid is split.
 */
/*****************************************************************************/
#include "Hydrogen.h"
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Vector3>
#include <cmath>
#include <cstdlib>
#include <iostream>



const kvs::Int32 num_dimentions = 3;
const kvs::Int32 num_element_node = 8;

// ---- Fixed GLOBAL problem definition (do not depend on rank count) ----
static const int   GX_NODES   = 256;   // x nodes (fixed)
static const int   GY_NODES   = 256;   // y nodes (fixed)
static const int   GZ_CELLS   = 256;   // global z cells (fixed; divisible by 1,2,4,8,16)
static const float CELL_LENGTH = 0.5f; // fixed physical spacing

/*===========================================================================*/
/**
 *  @brief  Generates one rank's z-slab of the fixed global hydrogen volume.
 */
/*===========================================================================*/
int Hydrogen::generate_volume( void )
{
    const int local_rank = mpi_rank % mpi_per_ens;
    const int ens_id     = mpi_rank / mpi_per_ens;

    // z-axis 1D partition of the FIXED global grid.
    int chunk = GZ_CELLS / mpi_per_ens;          // z cells owned by this rank
    if ( chunk < 1 ) chunk = 1;
    const int local_dim3 = chunk + 1;            // z nodes (shared boundary with neighbor)
    resolution = kvs::Vector3ui( (unsigned)GX_NODES, (unsigned)GY_NODES, (unsigned)local_dim3 );

    const kvs::UInt64 dim1 = resolution.x();
    const kvs::UInt64 dim2 = resolution.y();
    const kvs::UInt64 dim3 = resolution.z();

    // Origin of this rank's slab in the global domain (x,y full; z offset).
    const float origin_x = 0.0f;
    const float origin_y = 0.0f;
    const float origin_z = (float)( local_rank * chunk ) * cell_length;
    m_origin = kvs::Vector3f( origin_x, origin_y, origin_z );

    const kvs::Real64 dim = 128.0;
    const kvs::Real64 kr  = 32.0 / dim;
    const kvs::Real64 kd  = 1.0 + (double)( ens_id );
    const kvs::Real64 kr3 = 32.0 / dim * 0.1667 * kd;

    ncells = ( dim1 - 1 ) * ( dim2 - 1 ) * ( dim3 - 1 );
    nnodes = dim1 * dim2 * dim3;

    connections = new unsigned int[ ncells * num_element_node ];
    coords      = new float[ nnodes * num_dimentions ];
    values      = new float*[ nvariables ];
    for(int i=0; i<nvariables; i++)
    {
        values[i] = new float[nnodes];
    }

    kvs::UInt64 index = 0;
    kvs::UInt64 coords_index = 0;

    for ( kvs::UInt64 k = 0; k < dim3; ++k )
    {
        for ( kvs::UInt64 j = 0; j < dim2; ++j )
        {
            for ( kvs::UInt64 i = 0; i < dim1; ++i )
            {
                const float x = (float)i * cell_length + origin_x;
                const float y = (float)j * cell_length + origin_y;
                const float z = (float)k * cell_length + origin_z;

                const kvs::Real64 dx = kr * ( x - ( dim / 2.0 ) );
                const kvs::Real64 dy = kr * ( y - ( dim / 2.0 ) );
                const kvs::Real64 dz = kr3* ( z - ( dim / 2.0 ) );

                const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
                const kvs::Real64 cos_theta = dz / r;
                const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
                const kvs::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

                values[0][index] = static_cast<float>( c );
                values[1][index] = static_cast<float>( c );
                index++;
                coords[ coords_index++ ] = x;
                coords[ coords_index++ ] = y;
                coords[ coords_index++ ] = z;
            }
        }
    }

    // Hexahedra connections.
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

// NOTE: calc_each_ens / calc_average are legacy helpers kept for API
// compatibility. They are NOT called in the strong-scaling driver.
void Hydrogen::calc_each_ens(int /*mpi_size*/) {}
void Hydrogen::calc_average(int /*mpi_size*/) {}

/*===========================================================================*/
/**
 *  @brief  Constructs the strong-scaling hydrogen generator.
 */
/*===========================================================================*/
Hydrogen::Hydrogen( void )
{
    nvariables = 2;

    // Fixed global grid / spacing (per-rank z resolution is set in generate_volume).
    resolution  = kvs::Vector3ui( (unsigned)GX_NODES, (unsigned)GY_NODES, (unsigned)GZ_CELLS );
    cell_length = CELL_LENGTH;

    global_min_coord = kvs::Vector3f( 0, 0, 0 );
    global_max_coord = kvs::Vector3f( (float)((GX_NODES-1)*CELL_LENGTH),
                                      (float)((GY_NODES-1)*CELL_LENGTH),
                                      (float)( GZ_CELLS    *CELL_LENGTH) );

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    // Number of ensembles is FIXED for strong scaling (default 4).
    num_ensembles = 4;
    const char* env_ens = std::getenv( "PBVR_STRONG_ENS" );
    if ( env_ens != NULL )
    {
        const int e = std::atoi( env_ens );
        if ( e >= 1 ) num_ensembles = e;
    }

    // ranks splitting one ensemble.
    mpi_per_ens = mpi_size / num_ensembles;
    if ( mpi_per_ens < 1 ) mpi_per_ens = 1;

    if ( mpi_rank == 0 )
    {
        std::cout << "[strong] mpi_size=" << mpi_size
                  << " num_ensembles=" << num_ensembles
                  << " mpi_per_ens=" << mpi_per_ens
                  << " (GX=" << GX_NODES << ",GY=" << GY_NODES
                  << ",GZ_CELLS=" << GZ_CELLS << ")" << std::endl;
        if ( mpi_size % num_ensembles != 0 )
            std::cout << "[strong][WARN] mpi_size not divisible by num_ensembles" << std::endl;
        if ( GZ_CELLS % mpi_per_ens != 0 )
            std::cout << "[strong][WARN] GZ_CELLS not divisible by mpi_per_ens" << std::endl;
    }

    this->generate_volume();
}

/*===========================================================================*/
/**
 *  @brief  Prints a short summary of this rank's slab.
 */
/*===========================================================================*/
void Hydrogen::show( void )
{
    std::cout<<"-------show volume data------"<<std::endl;
    std::cout<<"mpi_rank="<<mpi_rank<<std::endl;
    std::cout<<"mpi_per_ens="<<mpi_per_ens<<std::endl;
    std::cout<<"ens_id="<<(mpi_rank/mpi_per_ens)<<std::endl;
    std::cout<<"resolution="<<resolution<<std::endl;
    std::cout<<"nvariables="<<nvariables<<std::endl;
    std::cout<<"origin="<<m_origin<<std::endl;
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
