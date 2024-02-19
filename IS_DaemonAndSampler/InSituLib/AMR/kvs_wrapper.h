#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include "../shared/thread_timer.h"

#ifndef SIMDW
#define SIMDW 128
#endif

typedef float Type;

typedef unsigned char Byte;

//typedef void* pbvr_ParticleWriteThread;

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

        int*  resolution;
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

    void generate_particles( const int time_step,
                             const domain_parameters dom,
                             const std::vector<float>& leaf_length,
                             const std::vector<float>& leaf_min_coord,
                             const int nvariables,
                             const float** values );

    void state_txt_writer( void );
#ifdef __cplusplus
}
#endif

#endif
