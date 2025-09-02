#include <string>
#include <iostream>
#include "kvs_wrapper.h"
#include <mpi.h>

// Fortran ラッパー
extern "C" {
  int generate_particles_wrap( 
			      int time_step, 
			      // ----
			      float global_min_coord_x,
			      float global_min_coord_y,
			      float global_min_coord_z,
			      float global_max_coord_x,
			      float global_max_coord_y,
			      float global_max_coord_z,
			      // ----
			      float *values_, int nvariables,
			      float *coords , int nnodes,
			      int   *connections_, int ncells ,
			      int num_element_node
			      )
  {
    float** values;
    unsigned int*  connections;
    values = new float*[ nvariables ];
    for(int i=0; i<nvariables; i++)
    {
      values[i] = new float[nnodes];
    }
    for(int j=0; j<nvariables; j++)
    {
      for(int i=0; i<nnodes; i++)
      {
	values[j][i] = values_[j + nvariables * i];
      }
    }
    // ----
    connections = new unsigned int[ ncells * num_element_node ];
    for(int i=0; i<ncells * num_element_node; i++)
    {
      connections[i]=static_cast<unsigned int>(connections_[i]);
    }

    //全体の座標
    domain_parameters dom = {
        global_min_coord_x,
        global_min_coord_y,
        global_min_coord_z,
        global_max_coord_x,
        global_max_coord_y,
        global_max_coord_z
    };

    generate_particles( 
		       time_step, dom,
		       values, nvariables,
		       coords, nnodes,
		       connections, ncells );

    for(int i=0; i<nvariables; i++)
    {
        delete [] values[i];
    }
    delete [] values;
    delete [] connections;

    return 1;
  }
  int generate_particles_wrap_(
			       int *time_step, 
			       // ----
			       float *global_min_coord_x,
			       float *global_min_coord_y,
			       float *global_min_coord_z,
			       float *global_max_coord_x,
			       float *global_max_coord_y,
			       float *global_max_coord_z,
			       // ----
			       float *values_ , int *nvariables,
			       float *coords  , int *nnodes,
			       int *connections_, int *ncells,
			       int *num_element_node)
  {
    generate_particles_wrap( 
			    time_step[0], 
			    // ----
			    global_min_coord_x[0],
			    global_min_coord_y[0],
			    global_min_coord_z[0],
			    global_max_coord_x[0],
			    global_max_coord_y[0],
			    global_max_coord_z[0],
			    // ----
			    values_, nvariables[0],
			    coords , nnodes[0],
			    connections_, ncells[0] ,
			    num_element_node[0]
			     );
    return 1;
  }
}

