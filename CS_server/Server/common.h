#ifndef VIS_MODULE__COMMON_H_INCLUDE
#define VIS_MODULE__COMMON_H_INCLUDE
#include <vismodule/Compiler>

#ifdef VIS_MODULE_COMPILER_VC
#include <time.h>
#else
#include <sys/time.h>
#endif

#define VIS_MODULE_TIME_TEST__

#define VIS_MODULE_TINYMT_FILE "tinymt32dc.0.65536.txt"
#define VIS_MODULE_TINYMT_NUM_PARAM 65536

#define VIS_MODULE_VIS_MODULE_CUDA_SAFE_CALL_NO_SYNC(call) {                                       \
    cudaError err = call;                                                    \
    if( cudaSuccess != err) {                                                \
        fprintf(stderr, "Cuda error in file '%s' in line %i : %s.\n",        \
                __FILE__, __LINE__, cudaGetErrorString( err) );              \
        exit(EXIT_FAILURE);                                                  \
    } }

#define VIS_MODULE_CUDA_SAFE_CALL( call)     VIS_MODULE_VIS_MODULE_CUDA_SAFE_CALL_NO_SYNC(call);

#define VIS_MODULE_MATH_TINY_VALUE (1.e-6f)

inline double GetTime()
{
#ifdef VIS_MODULE_COMPILER_VC
    clock_t time;
    time = clock();
    // return msec
    return static_cast<double>( time * 1000 / CLOCKS_PER_SEC );
#else
    struct timeval tv;
    gettimeofday( &tv, NULL );
    // return msec
    return ( double )( tv.tv_sec ) * 1000 + ( double )( tv.tv_usec ) * 1e-3;
#endif
}

#endif

