/*****************************************************************************/
/**
 *  @file   Hydrogen.h
 *  @brief  Strong-scaling variant: single executable, fixed global problem (N).
 *
 *  The global grid (GX x GY nodes, GZ_CELLS cells in z) and the number of
 *  ensembles are FIXED. Only the number of ranks that split one ensemble
 *  (mpi_per_ens = mpi_size / num_ensembles) varies at runtime, so the union
 *  of all ranks always reconstructs the same N. This makes the cases a valid
 *  strong-scaling series (N fixed, p increases).
 */
/*****************************************************************************/
#ifndef __HYDROGEN_H__
#define __HYDROGEN_H__

#include <kvs/Vector3>
#include <kvs/Vector2>
#include <mpi.h>

/*===========================================================================*/
/**
 *  @brief  Hydrogen volume generator class (strong-scaling variant).
 */
/*===========================================================================*/
class Hydrogen
{
public:

    kvs::Vector3ui resolution;
    kvs::Vector3f  global_region[8];   // kept for legacy calc_* (unused by generate_volume)
    kvs::Vector3f  m_origin;           // this rank's slab origin in the global domain
    kvs::Vector3f  global_min_coord;
    kvs::Vector3f  global_max_coord;
    float          cell_length;
    int            mpi_rank;
    int            mpi_size;
    int            num_ensembles;      // FIXED number of ensembles (strong scaling)
    int            mpi_per_ens = 1;    // ranks splitting one ensemble (= mpi_size/num_ensembles)

    float**        values;//[nvariables][nnodes]
    unsigned int*  connections;
    float*         coords;
    int            ncells;
    int            nnodes;
    int            nvariables;

public:

    Hydrogen( void );

    ~Hydrogen( void );

    int generate_volume( void );

    void calc_average( int mpi_size);

    void calc_each_ens( int mpi_size);

    void show( void );
};

#endif // __HYDROGEN_H__
