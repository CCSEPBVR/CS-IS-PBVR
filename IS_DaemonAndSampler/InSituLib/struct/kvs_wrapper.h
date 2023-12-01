#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED


#include "../shared/thread_timer.h"

#ifndef SIMDW
#define SIMDW 128
#endif

#ifdef DOUBLE_SCHEME
  typedef double Type;
#else
  typedef float Type;
#endif

//typedef void* pbvr_ParticleWriteThread;

typedef unsigned char Byte;

#ifdef __cplusplus
extern "C" {
#endif

    //    pbvr_ParticleWriteThread create_particle_write_thread();

    float GetRandomNumber();

    typedef struct
    {
        float x_global_min;
        float y_global_min;
        float z_global_min;

        float x_global_max;
        float y_global_max;
        float z_global_max;

        float x_min;
        float y_min;
        float z_min;

        int*  resolution;
        float cell_length;
    } domain_parameters;

    typedef struct
    {
        double initialize;
        double sampling;
        double writting;
        double mpi_reduce;
        double write_text;
        int nparticles;
    } time_parameters;

    void begin_wrapper_async_io();
    void   end_wrapper_async_io();

    void generate_particles( int time_step,
                             domain_parameters dom,
                             Type** volume_data, 
                             int num_volume_data );

    void state_txt_writer( void );



#ifdef __cplusplus
}
#endif

#endif