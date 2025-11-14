#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include <vismodule/VolumeObjectBase>
#include <vismodule/Argument>
#include <vismodule/NameListFile>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/CellByCellParticleGenerator>

#ifdef EXTEND_FILE_FORMAT
#include <vtkUnstructuredGrid.h>
#endif

#ifdef DOUBLE_SCHEME
    typedef double Type;
#else
    typedef float Type;
#endif

typedef unsigned char Byte;

#ifdef __cplusplus
extern "C" {
#endif

void OutputCoordMinMaxFile(
    const domain_parameters_unstruct& dom,
    const std::string& coordMinMaxFilePath
);

bool SetParticleParameter( 
    const domain_parameters_unstruct& dom,
    const std::string& tfFilePath,
    const std::string& tfFilePath_old,
    Argument& param,
    MultiVolumePropertyList& mvpl,
    NameListFile& nameListFile
);

bool generate_particles(
    int time_step,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltypes
);

#ifdef EXTEND_FILE_FORMAT
bool generate_particles_vtk(
    int time_step,
    vtkUnstructuredGrid* ucd
);

void SetDomain(
    vtkUnstructuredGrid* ucd,
    domain_parameters_unstruct* dom
);
#endif

    /*
    void kmath_initialization();
    void kmath_finalization();
    float GetRandomNumber();

    typedef struct
    {
        int  npe,  npe_x,  npe_y,  npe_z, npe_xy;
        int  rank;
    } mpi_parameters;

    typedef struct
    {
        int nx, ny, nz,
            mx, my, mz, m;

        int stm;

        float dx, dy, dz;

        int gnx, gny, gnz;

        int restart;
    } domain_parameters_unstruct;

    typedef struct
    {
        float x_global_min;
        float y_global_min;
        float z_global_min;

        float x_global_max;
        float y_global_max;
        float z_global_max;
    } domain_parameters_unstruct;

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

    void generate_particles( const int time_step,
                             Type** values, int nvariables,
                             float* coords, int ncoords,
                             unsigned int* connections, int ncells,
                             domain_parameters_unstruct* cdo,
                             mpi_parameters* mpi,
                             time_parameters* time );

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

    void state_txt_writer( void );
    */

#ifdef __cplusplus
}
#endif

#endif
