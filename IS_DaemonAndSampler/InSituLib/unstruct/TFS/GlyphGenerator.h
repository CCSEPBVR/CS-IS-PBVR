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
#include <kvs/KVSMLObjectPoint>
#include "particle_write_thread.h"

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
   std::vector<float> m_glyph_coords; 
   std::vector<float> m_glyph_vectors; 
   std::vector<float> m_glyph_sizes; 
   std::vector<float> m_glyph_colors_data; 
   std::vector<unsigned char>   m_glyph_colors; 
   //std::vector<pbvr::TransferFunction> m_tf;
   pbvr::TransferFunction m_tf;
   TransferFunctionSynthesizer* m_tfs;
   
   float**           m_values;
   float*       m_coords;
   kvs::UInt32* m_connections;
   int          m_ncoords;
   int          m_ncells;
   int          m_nvariable;
   int          m_stride; 
   jpv::DataDefines         m_size_DataDefines;
   jpv::DataDefines         m_color_DataDefines;

   std::vector<float> m_min;
   std::vector<float> m_max;

private:
   void  PointSampling( glyph_parameters &glyphParameter);
   void  PointSampling( 
           //Type** values, int nvariables,
           //float* coordinates, int ncoords,
           int stride);
   void  DistributionSampling(int number_of_sampling_point ,int seed, const pbvr::VolumeObjectBase::CellType& celltype);
   void  DistributionSampling(glyph_parameters &glyphParameter, const pbvr::VolumeObjectBase::CellType& celltype);

   const size_t calculate_number_of_particles(
           const float density,
           const float volume_of_cell,
           kvs::MersenneTwister* MT );

public:
   //void GlyphSampling();
//   void GlyphSampling(
//           Type** values, int nvariables,
//           float* coordinates, int ncoords,
//           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype );
   void GlyphSampling( const pbvr::VolumeObjectBase::CellType& celltype);


    GlyphGenerator();
//    GlyphGenerator(GlyphMode mode, jpv::DataDefines size_DataDefines, jpv::DataDefines color_DataDefines,
//            int stride, int seed, int number_of_sampling_point,
//           Type** values, int nvariables,
//           float* coordinates, int ncoords,
//           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype,
//           pbvr::TransferFunction& tf, TransferFunctionSynthesizer* tfs );

    GlyphGenerator(glyph_parameters &glyphParameter ,Type** values, int nvariables,
           float* coordinates, int ncoords,
           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype,
           TransferFunctionSynthesizer* tfs );

    void OutputGlyph( glyph_parameters &glyphParameter,const  pbvr_parameters& particleBase, const int time_step);
    void CalculateMinMax();
    void NormalizeValues();

    void show(glyph_parameters &glyphParameter);

};

#endif
