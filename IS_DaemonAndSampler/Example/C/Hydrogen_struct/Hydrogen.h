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

#include <kvs/Vector2>
#include <kvs/Vector3>
#include <mpi.h>

/*===========================================================================*/
/**
 *  @brief  Hydrogen volume generator class.
 */
/*===========================================================================*/
class Hydrogen
{
public:

    kvs::Vector3i resolution;
    kvs::Vector2f  global_region[4];
    kvs::Vector3f  global_min_coord;
    kvs::Vector3f  global_max_coord;

    float          cell_length;
    int            mpi_rank;

    float**        values;//[nvariables][nnodes]
    int            nvariables;

public:

    Hydrogen( void );

    ~Hydrogen( void );

    int generate_volume( void );

    void show( void );
};

#endif // __HYDROGEN_H__
