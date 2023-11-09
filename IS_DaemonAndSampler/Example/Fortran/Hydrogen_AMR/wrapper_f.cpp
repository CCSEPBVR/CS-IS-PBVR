#include <string>
#include <iostream>
#include <vector>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Message>
#include "kvs_wrapper.h"
#include <mpi.h>

// Fortran ラッパー
extern "C" {
  int generate_particles_wrap( 
			      int time_step,
			      float global_min_coord_x, 
			      float global_min_coord_y, 
			      float global_min_coord_z, 
			      float global_max_coord_x, 
			      float global_max_coord_y, 
			      float global_max_coord_z, 
			      int *resolution_, 
			      float *cell_length_, 
			      float *leaf_min_coord_, 
			      int nvariables,
			      float *values_,
			      int nl,
			      int values_size)
  {
    int resolution[4];
    resolution[0]=resolution_[0];
    resolution[1]=resolution_[1];
    resolution[2]=resolution_[2];
    resolution[3]=resolution_[3];

    domain_parameters dom = {
        global_min_coord_x,
        global_min_coord_y,
        global_min_coord_z,
        global_max_coord_x,
        global_max_coord_y,
        global_max_coord_z,
        resolution
    };
    std::vector<float> cell_length;
    std::vector<float> leaf_min_coord;
    for (int l=0; l<nl; l++)
    {
      cell_length.push_back( cell_length_[l] );
      leaf_min_coord.push_back( leaf_min_coord_[0+l*3] );
      leaf_min_coord.push_back( leaf_min_coord_[1+l*3] );
      leaf_min_coord.push_back( leaf_min_coord_[2+l*3] );
    }

    float** values = new float*[nvariables];
    for( int i=0; i<nvariables; i++ )
    {
      values[i] = new float[ values_size ];
    }
    for( int cnt=0; cnt<values_size; cnt++ )
    {
      values[0][cnt] = values_[0+cnt*4];
      values[1][cnt] = values_[1+cnt*4];
      values[2][cnt] = values_[2+cnt*4];
      values[3][cnt] = values_[3+cnt*4];
    }
    generate_particles( time_step, dom,
                        cell_length, leaf_min_coord,
                        nvariables, (const float**)values );

    for( int i=0; i<nvariables; i++ )
    {
        delete [] values[i];
    }
    delete [] values;
    return 1;
  }
  int generate_particles_wrap_(
			       int *time_step,
			       float *global_min_coord_x, 
			       float *global_min_coord_y, 
			       float *global_min_coord_z, 
			       float *global_max_coord_x, 
			       float *global_max_coord_y, 
			       float *global_max_coord_z, 
			       int *resolution_, 
			       float *cell_length_, 
			       float *leaf_min_coord_, 
			       int *nvariables,
			       float *values_,
			       int *nl,
			       int *values_size)
  {
    
    generate_particles_wrap( 
			    time_step[0],
			    // -----
			    global_min_coord_x[0], 
			    global_min_coord_y[0], 
			    global_min_coord_z[0], 
			    global_max_coord_x[0], 
			    global_max_coord_y[0], 
			    global_max_coord_z[0], 
			    resolution_, 
			    // -----
			    cell_length_, 
			    leaf_min_coord_, 
			    nvariables[0],
			    values_,
			    nl[0],
			    values_size[0]);
    return 1;
  }
}
