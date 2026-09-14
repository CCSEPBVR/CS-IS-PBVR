/*****************************************************************************/
/**
 *  @file   Hydrogen.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Hydrogen.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef __HYDROGEN_H__
#define __HYDROGEN_H__

#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/Vector2>
#include <mpi.h>

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

    float**        values;//[nvariables][nnodes]
    unsigned int*  connections;
    float*         coords;
    int            ncells;
    int            nnodes;
    int            nvariables;
    const vismodule::Real64 dim= 128.0;
    const vismodule::Real64 kr = 32.0 / dim;
    vismodule::Real64 kd;    // アンサンブルメンバ依存: 1.0 + ens_id
    vismodule::Real64 kr3;   // z方向の係数(ens版と同じ kr*0.1667*kd)

public:

    Hydrogen( void );

    ~Hydrogen( void );

    int generate_volume( void );

    float calc(const float x, const float y, const float z);
    
    void show( void );
};

#endif // __HYDROGEN_H__
