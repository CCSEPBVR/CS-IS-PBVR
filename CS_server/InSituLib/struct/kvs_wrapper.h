#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED


#include "../shared/thread_timer.h"
#include <vismodule/PointObject>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/CellByCellHistogram>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/ParamInfo>
#include "../../Common/ParticleTransferProtocol.h"

#include <vismodule/KVSMLObjectPlotOverLine>

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
                             domain_parameters_struct dom,
                             Type** volume_data, 
                             int nvariables );
    
    void GenerateParticles( int time_step,
                             domain_parameters_struct dom,
                             Type** volume_data, 
                             int nvariables,  pbvr_parameters& particleBase  );

    void GenerateGlyphs_PlotOverLine( int time_step,
                             domain_parameters_struct dom,
                             Type** values, int nvariables);

    void CallPlotOverLine( int time_step,
                             domain_parameters_struct dom,
                             Type** values, int nvariables);

    void GlyphObjectGenerator( int time_step,
                             domain_parameters_struct dom,
                             Type** values, int nvariables,
                             const jpv::ParticleTransferClientMessage& clntMes, glyph_parameters& glyph_param );

    void PlotOverLineObjectGenerator( int time_step,
                             domain_parameters_struct dom,
                             Type** values, int nvariables,
                             const jpv::ParticleTransferClientMessage& clntMes, plot_over_line_parameters& pol_param );

    //bool SetParameter(const domain_parameters_struct dom, pbvr_parameters* particleBase, ParamInfo *param_info, const int time_step);
    bool SetParameter(const domain_parameters_struct dom, pbvr_parameters* particleBase, ParamInfo *param_info, const int time_step);
    void SetPOLParameter(jpv::ParticleTransferClientMessage* clntMes  ,const int time_step, plot_over_line_parameters& pol_param);
    void SetGlyphParameter(jpv::ParticleTransferClientMessage* clntMes  ,const int time_step, glyph_parameters& glyph_param);


    void OutputParticles( int time_step, int nvariables, pbvr_parameters& particleBase,  ParamInfo *param_info, bool skip_flag);
    void OutputGlyphs( const int time_step, glyph_parameters& glyph_param);
    void OutputLine(   const int time_step, plot_over_line_parameters& pol_param);

    void state_txt_writer( void );



#ifdef __cplusplus
}
#endif

#endif
