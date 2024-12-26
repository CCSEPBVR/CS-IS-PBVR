#ifndef GLYPH_GENERATOR
#define GLYPH_GENERATOR

#include <cmath>
#include "TransferFunctionSynthesizer.h"
#include "ParamInfo.h"
#include "float.h"
#include "UnstructuredVolumeObject.h"
#include <mpi.h>
#include "CellBase.h"
#include "CellBase_hex.h"
#include "TetrahedralCell.h"
#include "QuadraticTetrahedralCell.h"
#include "HexahedralCell.h"
#include "QuadraticHexahedralCell.h"
#include "PrismaticCell.h"
#include "PyramidalCell.h"
#include "../kvs_wrapper.h"
#include <kvs/PointObject>
#include <kvs/GlyphObject>
#include <kvs/KVSMLObjectPoint>
#include "particle_write_thread.h"
//#include "KVSMLObjectGlyph.h"

#include "GlyphGenerator.h"
#include "GlyphProperty.h"

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

   enum class DataDefines : int32_t {
       Constant = 0,  // 値の設定
       SingleVariable = 1,
       VariablesArray = 2
   };

   enum class GlyphMode : int32_t {
       All_points = 0,  // 値の設定
       Every_points = 1,
       Uniform_distribution = 2
   };


class GlyphGenerator 
{
//    kvsClassName( pbvr::CellBase );
protected:
   pbvr::TransferFunction m_tf;
   TransferFunctionSynthesizer* m_tfs;
   
   float**           m_values;
   float*       m_coords;
   kvs::UInt32* m_connections;
   int          m_ncoords;
   int          m_ncells;
   int          m_nvariable;

//  glyph parameter 
   // ファイルパス(デーモン→サーバー)
   std::string m_glyphParamPath;
    // ファイルパス(サーバー→デーモン)
   std::string m_glyphFilePath;
    //出力用パラメータ(サーバー→デーモン→クライアント)
   std::vector<float> m_glyph_coords; 
   std::vector<float> m_glyph_vectors; 
   std::vector<float> m_glyph_sizes; 
   std::vector<float> m_glyph_colors_data; 
   std::vector<unsigned char>   m_glyph_colors; 

    //入力パラメータ(デーモン→サーバー)
    std::vector<int>  m_direction_variables;
    jpv::DataDefines m_size_sampling_method;
    std::vector<int> m_size_variables;
    jpv::GlyphMode m_distribution_modes;
    int m_stride;
    int m_seed;
    int m_number_of_sample_points;
    kvs::ColorMap m_color_map;
    jpv::DataDefines m_color_sampling_method;
    std::vector<int> m_color_data_variables;
// glyph paramter end

   
   //ポイントデータ用minmax
   std::vector<float> m_color_min;
   std::vector<float> m_color_max;
   std::vector<float> m_size_min;
   std::vector<float> m_size_max;

private:
   //void  PointSampling( glyph_parameters &glyphParameter);
   void  PointSampling( );
   void  PointSampling( 
           //Type** values, int nvariables,
           //float* coordinates, int ncoords,
           int stride);
   void  DistributionSampling(int number_of_sampling_point ,int seed, const pbvr::VolumeObjectBase::CellType& celltype);
   //void  DistributionSampling(glyph_parameters &glyphParameter, const pbvr::VolumeObjectBase::CellType& celltype);
   void  DistributionSampling(const pbvr::VolumeObjectBase::CellType& celltype);

   const size_t calculate_number_of_particles(
           const float density,
           const float volume_of_cell,
           kvs::MersenneTwister* MT );
    bool SetGlyphParameter(pbvr_parameters& particleBase,const int time_step);

public:
   void GlyphSampling( const pbvr::VolumeObjectBase::CellType& celltype);


    GlyphGenerator();
    GlyphGenerator(pbvr_parameters& particleBase, const int time_step, Type** values, int nvariables,
           float* coordinates, int ncoords,
           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype);


    void OutputGlyph( const  pbvr_parameters& particleBase, const int time_step);
    void show();

};

#endif
