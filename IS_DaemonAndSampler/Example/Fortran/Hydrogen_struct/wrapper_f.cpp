#include <iostream>
#include <string>
#include <kvs/Vector3>
#include <mpi.h>
#include "kvs_wrapper.h"

// Fortran ラッパー
extern "C" {
  int generate_particles_wrap(
			  int   time_step,
			  //
			  float global_min_coord_x,
			  float global_min_coord_y,
			  float global_min_coord_z,
			  float global_max_coord_x,
			  float global_max_coord_y,
			  float global_max_coord_z,
			  float global_region_x,
			  float global_region_y,
			  float global_region_z,
			  int   resolution_x,
			  int   resolution_y,
			  int   resolution_z,
			  float cell_length,
			  //
			  float *values_,
			  int   nvariables)
  {
    int dim1 = resolution_x;
    int dim2 = resolution_y;
    int dim3 = resolution_z;

    float** values;
    values = new float*[ nvariables ];
    for(int i=0; i<nvariables; i++)
    {
        values[i] = new float[dim1*dim2*dim3];
    }
    for(int j=0; j<nvariables; j++)
    {
      for(int i=0; i<dim1*dim2*dim3; i++)
      {
	values[j][i]=values_[j + nvariables * i];
      }
    }

    int resol[3] = 
      { resolution_x, resolution_y, resolution_z };
    domain_parameters dom = {
        global_min_coord_x,
        global_min_coord_y,
        global_min_coord_z,
        global_max_coord_x,
        global_max_coord_y,
        global_max_coord_z,

        global_region_x,
        global_region_y,
	global_region_z, // 0.0

        resol,
        cell_length
    };

    //    generate_particles( time_step, dom, hydro.values, hydro.nvariables );
    generate_particles( time_step, dom, values, nvariables );

    for(int i=0; i<nvariables; i++)
      {
	delete [] values[i];
      }
    delete [] values;    
    return 1;

  }

  int generate_particles_wrap_(
			  int   *time_step,
			  //
			  float *global_min_coord_x,
			  float *global_min_coord_y,
			  float *global_min_coord_z,
			  float *global_max_coord_x,
			  float *global_max_coord_y,
			  float *global_max_coord_z,
			  float *global_region_x,
			  float *global_region_y,
			  float *global_region_z,
			  int   *resolution_x,
			  int   *resolution_y,
			  int   *resolution_z,
			  float *cell_length,
			  //
			  float *values_,
			  int   *nvariables)
  {
    generate_particles_wrap(
			  time_step[0],
			  //
			  global_min_coord_x[0],
			  global_min_coord_y[0],
			  global_min_coord_z[0],
			  global_max_coord_x[0],
			  global_max_coord_y[0],
			  global_max_coord_z[0],
			  global_region_x[0],
			  global_region_y[0],
			  global_region_z[0],
			  resolution_x[0],
			  resolution_y[0],
			  resolution_z[0],
			  cell_length[0],
			  //
			  values_,
			  nvariables[0]);
    return 1;
  }

}
