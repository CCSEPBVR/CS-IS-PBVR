#ifndef GLYPH_GENERATOR
#define GLYPH_GENERATOR

#include <cmath>
#include "TransferFunctionSynthesizer.h"
//#include "ParamInfo.h"
#include "float.h"
#include <vismodule/UnstructuredVolumeObject>
#include "CellBase.h"
#include <vismodule/TrilinearInterpolator>
#include "TetrahedralCell.h"
#include "QuadraticTetrahedralCell.h"
#include "HexahedralCell.h"
#include "QuadraticHexahedralCell.h"
#include "PrismaticCell.h"
#include "PyramidalCell.h"
#include <vismodule/PointObject>
#include <vismodule/GlyphObject>
#include <vismodule/KVSMLObjectPoint>

#include <vismodule/KVSMLObjectGlyph>
#include <vismodule/GlyphProperty>
#include "../Common/ParticleTransferProtocol.h"

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP
#ifndef CPU_VER
#include <mpi.h>
#endif
/*
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
*/

typedef unsigned char Byte;

class GlyphGenerator 
{
//    visModuleClassName( vismodule::CellBase );
protected:
   vismodule::TransferFunction m_tf;
   TransferFunctionSynthesizer* m_tfs;
   
   float**           m_values;
   float*       m_coords;
   //vismodule::UInt32* m_connections;
   unsigned int* m_connections;
   int          m_ncoords;
   int          m_ncells;
   int          m_nvariable;
    // 生成判定フラグ
   bool m_g_flag;

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
    vismodule::ColorMap m_color_map;
    jpv::DataDefines m_color_sampling_method;
    std::vector<int> m_color_data_variables;
// glyph paramter end

   
   //ポイントデータ用minmax
   float m_color_min;
   float m_color_max;
   float m_size_min;
   float m_size_max;

private:
   void  PointSampling( );
   void  PointSampling( int stride);
   void  PointSampling_struct(const vismodule::StructuredVolumeObject* object );
   void  PointSampling_unstruct( );
   void  DistributionSampling(int number_of_sampling_point ,int seed, const vismodule::VolumeObjectBase::CellType& celltype);
   void  DistributionSampling_struct(const vismodule::StructuredVolumeObject* object);
   void  DistributionSampling_unstruct(const vismodule::VolumeObjectBase::CellType& celltype);

   const size_t calculate_number_of_particles(
           const float density,
           const float volume_of_cell,
           vismodule::MersenneTwister* MT );
    bool SetGlyphParameter();
    bool InputParameter(const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide);

public:
   void GlyphSampling( const vismodule::VolumeObjectBase::CellType& celltype);


    GlyphGenerator();
    GlyphGenerator( Type** values, int nvariables,
           float* coordinates, int ncoords,
           unsigned int* connections, int ncells, const  vismodule::VolumeObjectBase::CellType& celltype); //ISPBVR

    GlyphGenerator(const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide, Type** values, int nvariables,
           float* coordinates, int ncoords,
           unsigned int* connections, int ncells,
           const  vismodule::VolumeObjectBase::CellType& celltype ); // CSPBVR
    GlyphGenerator(const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide,const vismodule::StructuredVolumeObject& object ); 
    ~GlyphGenerator()
    {
    }

   std::vector<float> glyph_coords(){return m_glyph_coords;} 
   std::vector<float> glyph_directions(){return m_glyph_vectors;} 
   std::vector<float> glyph_sizes(){return m_glyph_sizes;} 
   std::vector<unsigned char>  glyph_colors(){return m_glyph_colors;}

    void getGlyphData(vismodule::KVSMLObjectGlyph* other);
    void OutputGlyph( const int time_step);
    void show();

};

#endif
