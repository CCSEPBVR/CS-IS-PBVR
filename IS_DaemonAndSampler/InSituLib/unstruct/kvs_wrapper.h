#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include "../shared/thread_timer.h"
#include "TFS/VolumeObjectBase.h"

#ifdef DOUBLE_SCHEME
  typedef double Type;
#else
  typedef float Type;
#endif

typedef unsigned char Byte;
//typedef void* pbvr_ParticleWriteThread;

#ifdef __cplusplus
extern "C" {
#endif

    void kmath_initialization();
    void kmath_finalization();

    //    pbvr_ParticleWriteThread create_particle_write_thread();

    float GetRandomNumber();

#if 0
    typedef struct
    {
        int  npe,  npe_x,  npe_y,  npe_z, npe_xy;
        int  rank;
    } mpi_parameters;
#endif
#if 0
    typedef struct
    {
        int nx, ny, nz,
            mx, my, mz, m;

        int stm;

        float dx, dy, dz;

        int gnx, gny, gnz;

        int restart;
    } domain_parameters;
#else
    typedef struct
    {
        float x_global_min;
        float y_global_min;
        float z_global_min;

        float x_global_max;
        float y_global_max;
        float z_global_max;
    } domain_parameters;
#endif

#if 0
    typedef struct
    {
        Type initialize;
        Type sampling;
        Type writting;
        Type mpi_reduce;
        Type write_text;
        Type read_param;
        int nparticles;
        int iout;
    } time_parameters;
#else
    typedef struct
    {
        double initialize;
        double sampling;
        double writting;
        double mpi_reduce;
        double write_text;
        int nparticles;
    } time_parameters;
#endif

    void begin_wrapper_async_io();
    void   end_wrapper_async_io();

#if 0
    void generate_particles( const int time_step,
                             Type** values, int nvariables,
                             float* coords, int ncoords,
                             unsigned int* connections, int ncells,
                             domain_parameters* cdo,
                             mpi_parameters* mpi,
                             time_parameters* time );
#else
    void generate_particles( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype );
#endif

    void state_txt_writer( void );


#ifdef __cplusplus
}
#endif

#endif
