#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include "../shared/thread_timer.h"
#include "TFS/VolumeObjectBase.h"
#include "TFS/ParamInfo.h"
#include "TFS/TransferFunction.h"

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

    typedef struct
    {
    std::vector<float>  m_sample_coords;
    std::vector<Byte>  m_sample_colors;
    std::vector<float>  m_sample_normals;
    int m_subpixel_level;
    std::string m_ptcFilePath;
    std::string m_stateFilePath;
    std::string m_tfFilename;
    std::string m_visParamDir;
    //static ParamInfo m_param;
    kvs::Vector3f m_min_vec, m_max_vec;
    bool m_parameter_file_opened;
    float m_max_density;
    float m_particle_data_size_limit;
    float m_sampling_volume_inverse;
    float m_particle_density;
    float m_max_opacity;

    std::vector<pbvr::TransferFunction> m_tf ;
    int m_tf_number;
    kvs::ValueArray<float> m_O_min;//計算して得る最大最小値
    kvs::ValueArray<float> m_O_max;
    kvs::ValueArray<float> m_C_min;
    kvs::ValueArray<float> m_C_max;
    kvs::ValueArray<int> m_o_histogram;//不透明度ヒストグラムの配列
    kvs::ValueArray<int> m_c_histogram;//色ヒストグラムの配列

    } pbvr_parameters;



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

    //void PbvrSampler_single( int time_step, domain_parameters dom,
    void generate_particles( int time_step, domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltypes );

    void GenerateHistogram( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );
 
    void GenerateParticles( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );
    void OutputParticles( int time_step, int nvariables, pbvr_parameters& particleBase,  ParamInfo *param_info, bool skip_flag);
    bool SetParameter(const domain_parameters dom, pbvr_parameters* particleBase, ParamInfo *param_info, const int time_step);

#endif

    void state_txt_writer( void );


#ifdef __cplusplus
}
#endif

#endif
