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

/*===========================================================================*/
/**
 *  @brief  Hydrogen volume generator class.
 */
/*===========================================================================*/
class Hydrogen
{
public:
    kvs::Vector3f global_min_coord;
    kvs::Vector3f global_max_coord;

    // num. of leafs in each MPI region
    int nleafs;

    // x resolution of each leaf, 4x4x4
    int resol_x;

    // Leaf topology at each process
    //  Lv.1; 8x8x8; resol_lv1
    //  Lv.2; 16x16x16; resol_lv2
    //  Lv.2 on Lv.1
    int resol_lv1;
    int resol_lv2;
    float leaf_length_lv1;
    float leaf_length_lv2;
    float cell_length_lv1;
    float cell_length_lv2;

    // global x-y coordinates of each MPI region
    kvs::Vector2f global_region[4];
    int mpi_rank;

public:
    Hydrogen( void )
        {
            global_min_coord = kvs::Vector3f( 0.0, 0.0, 0.0 );
            global_max_coord = kvs::Vector3f( 32.0, 32.0, 32.0 );
            resol_x = 4;
            resol_lv1 = 8;
            resol_lv2 = 16;
            leaf_length_lv1 = 2.0;
            leaf_length_lv2 = 1.0;
            cell_length_lv1 = leaf_length_lv1 / (resol_x-1);
            cell_length_lv2 = leaf_length_lv2 / (resol_x-1);

            nleafs = resol_lv1 * resol_lv1 * resol_lv1 + resol_lv2 * resol_lv2 * resol_lv2;

            global_region[ 0 ] = kvs::Vector2f(  0,  0 );
            global_region[ 1 ] = kvs::Vector2f( 16,  0 );
            global_region[ 2 ] = kvs::Vector2f( 16, 16 );
            global_region[ 3 ] = kvs::Vector2f(  0, 16 );

            MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
        }

    const float cellLength( const int leaf_index )
        {
            const int slice_lv1 = resol_lv1 * resol_lv1;
            const int volum_lv1 = slice_lv1 * resol_lv1;

            const int slice_lv2 = resol_lv2 * resol_lv2;
            const int volum_lv2 = slice_lv2 * resol_lv2;

            if( 0 <= leaf_index && leaf_index < volum_lv1 )
            {
                return cell_length_lv1;
            }
            else if( volum_lv1 <= leaf_index && leaf_index < (volum_lv1 + volum_lv2) )
            {
                return cell_length_lv2;
            }
            else
            {
                std::cout<<"cellLength error, leaf_index="<<leaf_index<<std::endl;
                return 0.0;
            }
        }

    const kvs::Vector3f leafMinCoord( const int leaf_index )
        {
            const int slice_lv1 = resol_lv1 * resol_lv1;
            const int volum_lv1 = slice_lv1 * resol_lv1;

            const int slice_lv2 = resol_lv2 * resol_lv2;
            const int volum_lv2 = slice_lv2 * resol_lv2;

            int i,j,k;
            float x,y,z;

            if( 0 <= leaf_index && leaf_index < volum_lv1 )
            {
                k = leaf_index / slice_lv1;
                j = (leaf_index - k * slice_lv1) / resol_lv1;
                i = leaf_index - j * resol_lv1 - k * slice_lv1;

                x = (float)i * leaf_length_lv1 + global_region[mpi_rank].x();
                y = (float)j * leaf_length_lv1 + global_region[mpi_rank].y();
                z = (float)k * leaf_length_lv1;
            }
            else if( volum_lv1 <= leaf_index && leaf_index < (volum_lv1 + volum_lv2) )
            {
                int l_index = leaf_index - volum_lv1;
                k = l_index / slice_lv2;
                j = (l_index - k * slice_lv2) / resol_lv2;
                i = l_index - j * resol_lv2 - k * slice_lv2;

                x = (float)i * leaf_length_lv2 + global_region[mpi_rank].x();
                y = (float)j * leaf_length_lv2 + global_region[mpi_rank].y();
                z = (float)k * leaf_length_lv2;
                z += (float)resol_lv1  * leaf_length_lv1;
            }
            else
            {
                std::cout<<"leafMinCoord error, leaf_index="<<leaf_index<<std::endl;
            }

            return kvs::Vector3f( x, y, z);
        }

    const kvs::Vector3f gridCoord( const int i, const int j, const int k, const int l )
        {
            const float dx = this->cellLength( l );

            const kvs::Vector3f lm = this->leafMinCoord( l );

            const float x = i * dx + lm.x();
            const float y = j * dx + lm.y();
            const float z = k * dx + lm.z();

            return kvs::Vector3f( x, y, z );
        }

    const float value( const kvs::Vector3f coord )
        {
            const float x = coord.x();
            const float y = coord.y();
            const float z = coord.z();

            const float dim = 32.0;
            const float kr = 32.0 / dim;
            const float kd  = 6.0;

            const float dx = kr * ( x - ( dim / 2.0 ));
            const float dy = kr * ( y - ( dim / 2.0 ));
            const float dz = kr * ( z - ( dim / 2.0 ));

            const float r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
            const float cos_theta = dz / r;
            const float phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
            const float  c = (phi * phi) > 255 ? 255 : (phi * phi);

            return c;
        }
};

#endif // __HYDROGEN_H__
