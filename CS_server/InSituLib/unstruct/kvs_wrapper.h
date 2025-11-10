#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include "../shared/thread_timer.h"
#include <vismodule/TransferFunction>
#include <vismodule/VolumeObjectBase>
#include <vismodule/ParamInfo>
#include <vismodule/TransferFunction>
#include "../../Common/ParticleTransferProtocol.h"
#include <vismodule/CellByCellHistogram>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/Argument>
#include <vismodule/NameListFile>
#include <vismodule/CS_PointObjectGenerator>

#ifdef VTK
#include <vtkUnstructuredGrid.h>
#endif

#include <vismodule/UnstructuredVolumeObject>

//Glyph
#include <vismodule/GlyphSeed>
#include <vismodule/GlyphProperty>

// plot over line 
#include <vismodule/PlotOverLine>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/KVSMLObjectPlotOverLine>

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
    } domain_parameters_unstruct;
#else
//    typedef struct
//    {
//        float x_global_min;
//        float y_global_min;
//        float z_global_min;
//
//        float x_global_max;
//        float y_global_max;
//        float z_global_max;
//    } domain_parameters_unstruct;
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
    vismodule::Vector3f m_min_vec, m_max_vec;
    bool m_parameter_file_opened;
    float m_max_density;
    float m_particle_data_size_limit;
    float m_sampling_volume_inverse;
    float m_particle_density;
    int m_particle_limit;
    float m_max_opacity;
    int m_nvariables;

    //std::vector<pbvr::TransferFunction> m_tf ;
    std::vector<vismodule::TransferFunction> m_tf ;
    int m_tf_number;
    vismodule::ValueArray<float> m_O_min;//計算して得る最大最小値
    vismodule::ValueArray<float> m_O_max;
    vismodule::ValueArray<float> m_C_min;
    vismodule::ValueArray<float> m_C_max;
    vismodule::ValueArray<int> m_o_histogram;//不透明度ヒストグラムの配列
    vismodule::ValueArray<int> m_c_histogram;//色ヒストグラムの配列

    int con_log[8];

    //サンプリング手法
    char m_sampling_method;

    } pbvr_parameters;


    // グリフ用パラメータ
    typedef struct
    {
   std::vector<float> m_glyph_coords;
   std::vector<float> m_glyph_vectors;
   std::vector<float> m_glyph_sizes;
   std::vector<unsigned char>   m_glyph_colors;   

   // ファイルパス(デーモン→サーバー)
    std::string m_glyphParamPath;
    // ファイルパス(サーバー→デーモン)
    std::string m_glyphFilePath;

    } glyph_parameters;

    // plot over line用パラメータ
    typedef struct
    {
    // 集約用データ
    vismodule::ValueArray<float> m_values_on_line;
    vismodule::ValueArray<float> m_x_axis;
    vismodule::ValueArray<bool>  m_mask;
    bool m_plot_flag;

    // ファイルパス(デーモン→サーバー)
    std::string m_POLParamPath;
    // ファイルパス(サーバー→デーモン)
    std::string m_POLFilePath;

    } plot_over_line_parameters;

    void begin_wrapper_async_io();
    void   end_wrapper_async_io();

#if 0
    void generate_particles( const int time_step,
                             Type** values, int nvariables,
                             float* coords, int ncoords,
                             unsigned int* connections, int ncells,
                             domain_parameters_unstruct* cdo,
                             mpi_parameters* mpi,
                             time_parameters* time );
#else

    //void PbvrSampler_single( int time_step, domain_parameters_unstruct dom,
    bool generate_particles( int time_step, domain_parameters_unstruct dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  vismodule::VolumeObjectBase::CellType& celltypes );

#ifdef VTK
    bool generate_particles_vtk( int time_step,vtkUnstructuredGrid* ucd ); 
#endif

    void OutputCoordMinMaxFile(
        const domain_parameters_unstruct& dom,
        const std::string& coordMinMaxFilePath
    );

    bool SetParameterFilePath(
        const int time_step,
        std::string& historyFilePath,
        std::string& stateFilePath,
        std::string& coordMinMaxFilePath,
        std::string& particleFilePrefix,
        std::string& glyphFilePrefix,
        std::string& plotOverLineFilePrefix,
        std::string& tfFilePath,
        std::string& tfFilePath_old,
        std::string& tfFilePath_step,
        std::string& glyphParameterPath,
        std::string& glyphParameterPath_old,
        std::string& plotOverLineParameterPath,
        std::string& plotOverLineParameterPath_old
    );

    bool SetParticleParameter( 
        const domain_parameters_unstruct& dom,
        const std::string& tfFilePath,
        const std::string& tfFilePath_old,
        Argument& param,
        MultiVolumePropertyList& mvpl,
        NameListFile& nameListFile
    );

    bool SetGlyphParameter(
        const std::string& glyphParameterPath,
        const std::string& glyphParameterPath_old,
        Argument& param
    );

    bool SetPlotOverLineParameter(
        const std::string& plotOverLineParameterPath,
        const std::string& plotOverLineParameterPath_old,
        Argument& param
    );

    void MakeParticle(
        const vismodule::PointObject* point_object,
        std::vector<float>& coords,
        std::vector<Byte>&  colors,
        std::vector<float>& normals
    );

    void MakeGlyph(
        const vismodule::KVSMLObjectGlyph* glyph_object,
        std::vector<float>& coords,
        std::vector<float>& vectors,
        std::vector<float>& sizes,
        std::vector<unsigned char>& colors
    );

    void OutputParticles(
        const Argument& param,
        const MultiVolumePropertyList& mvpl,
        const int time_step,
        const int tf_number,
        const int nvariables,
        const std::string& particleFilePrefix,
        const std::string& stateFilePath,
        const std::string& histryFilePath,
        const std::vector<float>& coords,
        const std::vector<Byte>& colors,
        const std::vector<float>& normals,
        const vismodule::UInt64* c_bins,
        const vismodule::UInt64* o_bins,
        const float* max_array,
        const float* min_array
    );

    void OutputGlyphs(
        const int time_step,
        const std::string& glyphFilePrefix,
        const std::vector<float>& coords,
        const std::vector<float>& vectors,
        const std::vector<float>& sizes,
        const std::vector<unsigned char>& colors
    );

    void OutputLine(
        const int time_step,
        const std::string& plotOverLineFilePrefix,
        const vismodule::ValueArray<float>& values_on_line,
        const vismodule::ValueArray<bool>& mask,
        const vismodule::ValueArray<float>& x_axis
    );

    /*
    typedef struct
    {
        double initialize;
        double sampling;
        double writting;
        double mpi_reduce;
        double write_text;
        int nparticles;
    } time_parameters;

    void SetPOLParameter(jpv::ParticleTransferClientMessage* clntMes  ,const int time_step, plot_over_line_parameters& pol_param);

    void SetGlyphParameter(jpv::ParticleTransferClientMessage* clntMes  ,const int time_step, glyph_parameters& glyph_param);

    void OutputParticles( int time_step, int nvariables, pbvr_parameters& particleBase,  ParamInfo *param_info, bool skip_flag);
    void OutputGlyphs( const int time_step, glyph_parameters& glyph_param);
    void OutputLine(   const int time_step, plot_over_line_parameters& pol_param);

    void GenerateHistogram( int time_step,
                             domain_parameters_unstruct dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  vismodule::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );
 
    void GenerateParticles( int time_step,
                             domain_parameters_unstruct dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  vismodule::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase );

    void GlyphObjectGenerator( int time_step,
                             domain_parameters_unstruct dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  vismodule::VolumeObjectBase::CellType& celltype,
                             const jpv::ParticleTransferClientMessage& clntMes, glyph_parameters& glyph_param );

    void PlotOverLineObjectGenerator( int time_step,
                             domain_parameters_unstruct dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  vismodule::VolumeObjectBase::CellType& celltype,
                             const jpv::ParticleTransferClientMessage& clntMes, plot_over_line_parameters& pol_param );

    void callPlotOverLine( int time_step,
                              domain_parameters_unstruct dom, 
                              Type** values, int nvariables,
                              float* coordinates, int ncoords,
                              unsigned int* connections, int ncells,
                              const  vismodule::VolumeObjectBase::CellType& celltype , PlotOverLine* plot_over_line );
    void GeneratePlotOverLine(int time_step, const vismodule::UnstructuredVolumeObject* volume, PlotOverLine* plot_over_line);
    void GeneratePlotOverLine(int time_step, const vismodule::UnstructuredVolumeObject* volume, PlotOverLine* plot_over_line);
    bool SetParameter(const domain_parameters_unstruct dom, pbvr_parameters* particleBase, ParamInfo *param_info, const int time_step);
    */

#endif

    void state_txt_writer( void );


#ifdef __cplusplus
}
#endif

#endif
