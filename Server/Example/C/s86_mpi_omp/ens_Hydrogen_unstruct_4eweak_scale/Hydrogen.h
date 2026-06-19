/*****************************************************************************/
/**
 *  @file   Hydrogen.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/vismodule/copyright/ for details.
 *
 *  $Id: Hydrogen.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef __HYDROGEN_H__
#define __HYDROGEN_H__
#include <vismodule/Vector3>
#include <vismodule/Vector2>
#ifndef CPU_VER
#include <mpi.h>
#endif

/*===========================================================================*/
/**
 *  @brief  Hydrogen volume generator class.
 */
/*===========================================================================*/
class Hydrogen
{
public:

    vismodule::Vector3ui resolution;
    vismodule::Vector2f  global_region[4];
    vismodule::Vector3f  global_min_coord;
    vismodule::Vector3f  global_max_coord;
    float          cell_length;
    int            mpi_rank;
    int            weak_base_ens;
    int            effective_ens_id;
    float          weak_kd;
    float          value_min;
    float          value_max;

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

    void show( void );
};

#endif // __HYDROGEN_H__
