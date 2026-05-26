#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include "../shared/thread_timer.h"
#include <kvs/TransferFunction>
#include "TFS/VolumeObjectBase.h"
#include "TFS/ParamInfo.h"
#include "TFS/TransferFunction.h"
#include "../../Common/ParticleTransferProtocol.h"

#ifdef VTK
#include <vtkUnstructuredGrid.h>
#endif

#include <kvs/UnstructuredVolumeObject>
#include "TFS/PlotOverLine.h"

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
    std::vector<Byte>   m_sample_colors;
    std::vector<float>  m_sample_normals;
    std::vector<float>  m_varience_coords;
    std::vector<Byte>   m_varience_colors;
    std::vector<float>  m_varience_normals;
    std::vector<float>  m_cov_coords;
    std::vector<Byte>   m_cov_colors;
    std::vector<float>  m_cov_normals;
    std::vector<float>  m_skewness_coords;
    std::vector<Byte>   m_skewness_colors;
    std::vector<float>  m_skewness_normals;
    int m_subpixel_level;
    std::string m_ptcFilePath;
    std::string m_stateFilePath;
    std::string m_tfFilename;
    std::string m_visParamDir;
    std::string m_varFilePath;
    std::string m_covFilePath;
    std::string m_skeFilePath;
    //static ParamInfo m_param;
    kvs::Vector3f m_min_vec, m_max_vec;
    bool m_parameter_file_opened;
    float m_max_density;
    float m_particle_data_size_limit;
    float m_sampling_volume_inverse;
    float m_particle_density;
    int m_particle_limit;
    float m_max_opacity;
    int m_nvariables;

    std::vector<pbvr::TransferFunction> m_tf ;
    std::vector<pbvr::TransferFunction> m_ave_tf ;
    std::vector<pbvr::TransferFunction> m_var_tf ;
    std::vector<pbvr::TransferFunction> m_cov_tf ;
    int m_tf_number;
    kvs::ValueArray<float> m_O_min;//計算して得る最大最小値
    kvs::ValueArray<float> m_O_max;
    kvs::ValueArray<float> m_C_min;
    kvs::ValueArray<float> m_C_max;
    kvs::ValueArray<int> m_o_histogram;//不透明度ヒストグラムの配列
    kvs::ValueArray<int> m_c_histogram;//色ヒストグラムの配列
    // 平均
    kvs::ValueArray<float> m_ave_O_min;//計算して得る最大最小値
    kvs::ValueArray<float> m_ave_O_max;
    kvs::ValueArray<float> m_ave_C_min;
    kvs::ValueArray<float> m_ave_C_max;
    kvs::ValueArray<int> m_ave_o_histogram;//不透明度ヒストグラムの配列
    kvs::ValueArray<int> m_ave_c_histogram;//色ヒストグラムの配列
     //分散
    kvs::ValueArray<float> m_var_O_min;//計算して得る最大最小値
    kvs::ValueArray<float> m_var_O_max;
    kvs::ValueArray<float> m_var_C_min;
    kvs::ValueArray<float> m_var_C_max;
    kvs::ValueArray<int> m_var_o_histogram;//不透明度ヒストグラムの配列
    kvs::ValueArray<int> m_var_c_histogram;//色ヒストグラムの配列
    //変動係数
    kvs::ValueArray<float> m_cov_O_min;//計算して得る最大最小値
    kvs::ValueArray<float> m_cov_O_max;
    kvs::ValueArray<float> m_cov_C_min;
    kvs::ValueArray<float> m_cov_C_max;
    kvs::ValueArray<int> m_cov_o_histogram;//不透明度ヒストグラムの配列
    kvs::ValueArray<int> m_cov_c_histogram;//色ヒストグラムの配列


    int con_log[8];

    } pbvr_parameters;

    typedef struct
    {
        std::vector<kvs::Real32>& vertex_coords;
        std::vector<kvs::Real32>& vertex_scalars;
        std::vector<kvs::Real32>& vertex_normals;
        // 法線ベクトルを各成分で分ける
        std::vector<kvs::Real32>& vertex_normals_x;
        std::vector<kvs::Real32>& vertex_normals_y;
        std::vector<kvs::Real32>& vertex_normals_z;
        std::vector<int>&         vertex_cellids;
//        std::vector<kvs::UInt8>  average_colors;
        std::vector<kvs::Real32> average_scalars;
        std::vector<kvs::Real32> average_normals; 
        // 法線ベクトルを各成分で分ける
        std::vector<kvs::Real32> average_normals_x; 
        std::vector<kvs::Real32> average_normals_y; 
        std::vector<kvs::Real32> average_normals_z; 
        std::vector<kvs::Real32> varience_scalars;
        std::vector<kvs::Real32> varience_normals; 
        // 法線ベクトルを各成分で分ける
        std::vector<kvs::Real32> varience_normals_x; 
        std::vector<kvs::Real32> varience_normals_y; 
        std::vector<kvs::Real32> varience_normals_z; 

        int ncells;
        int  MPIprocess_per_ensemble; 
        int  ensemble_number; 
        //int  repitetion; 
        float  repitetion; 
    } Ensembleparameters;



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

    void ensemble_generate_particles( int time_step, domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltypes );

    void ensemble_test( int time_step, domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltypes );


#ifdef VTK
    void generate_particles_vtk( int time_step,vtkUnstructuredGrid* ucd ); 
#endif

    void GenerateHistogram( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );
 
    void GenerateParticles( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );

    void EnsembleGenerateParticles( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );

    void EnsembleTest( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );



    void GenerateGlyphs( int time_step,
                             domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             //_const  pbvr::VolumeObjectBase::CellType& celltype ,pbvr_parameters& particleBase );
                             const  pbvr::VolumeObjectBase::CellType& celltype );
 void callPlotOverLine( int time_step,
                              domain_parameters dom, 
                              Type** values, int nvariables,
                              float* coordinates, int ncoords,
                              unsigned int* connections, int ncells,
                              const  pbvr::VolumeObjectBase::CellType& celltype , PlotOverLine* plot_over_line );
    void GeneratePlotOverLine(int time_step, const kvs::UnstructuredVolumeObject* volume, PlotOverLine* plot_over_line);
    //void GeneratePlotOverLine(int time_step, const kvs::UnstructuredVolumeObject* volume,plot_over_line_data& polData );

    void OutputParticles( int time_step, int nvariables, pbvr_parameters& particleBase,  ParamInfo *param_info, bool skip_flag);
    bool SetParameter(const domain_parameters dom, pbvr_parameters* particleBase, ParamInfo *param_info, const int time_step);

#endif

    void state_txt_writer( void );


#ifdef __cplusplus
}
#endif

#endif
