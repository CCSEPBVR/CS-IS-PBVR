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

#include <kvs/Vector3>
#include <kvs/Vector2>
#include <mpi.h>
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Vector3>

/*===========================================================================*/
/**
 *  @brief  Hydrogen volume generator class.
 */
/*===========================================================================*/
class Hydrogen
{
public:

    kvs::Vector3ui resolution;
    kvs::Vector2f  global_region[4];
    kvs::Vector3f  global_min_coord;
    kvs::Vector3f  global_max_coord;
    float          cell_length;
    int            mpi_rank;

    float**        values;//[nvariables][nnodes]
    unsigned int*  connections;
    float*         coords;
    int            ncells;
    int            nnodes;
    int            nvariables;
    const kvs::Real64 dim= 128.0;
    const kvs::Real64 kr = 32.0 / dim;
    const kvs::Real64 kd = 6.0;

public:

    Hydrogen( void );

    ~Hydrogen( void );

    int generate_volume( void );

    float calc(const float x, const float y, const float z);
    
    void show( void );
};

#endif // __HYDROGEN_H__
