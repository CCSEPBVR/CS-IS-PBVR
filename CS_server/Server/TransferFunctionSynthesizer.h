#ifndef __TRANSFER_FUNCTION_SYNTHESIZER_H__
#define __TRANSFER_FUNCTION_SYNTHESIZER_H__

#include <map>
#include <string>
#include <vector>
#include <kvs/RGBColor>
//#include <kvs/TrilinearInterpolator>
#include "HexahedralCell.h"
#include "TrilinearInterpolator.h"
#include "TransferFunction.h"
//#include "../kvs_wrapper.h"
#include "ReversePolishNotation.h"
#include "Token.h"
//add by shimomura  2022/12/20
#include "../Common/VariableRange.h"

#ifdef DOUBLE_SCHEME  
  typedef double Type;
#else
  typedef float Type;
#endif

#ifndef SIMDW 
#define SIMDW 128
#endif


#define TF_COUNT 5 //伝達関数の数
#define VAR_OFFSET_A 112 //Token.h->VarName-A1へのオフセット
#define VAR_OFFSET_C 96 //Token.h->VarName->C1へのオフセット

//2019 kawamura

struct EquationToken
{
    int   exp_token[128];//数式のトークン配列
    int   var_name[128];//数式の変数配列
    float val_array[128];//数式の値の配列
};

typedef std::map<std::string, size_t> QuantityMap;

class TransferFunctionSynthesizer
{
protected:
    //2019 kawamura
    EquationToken m_opa_func;//ex) A1+A2*A3
    EquationToken m_col_func;//ex) C1/C2-C3
    std::vector<EquationToken> m_opa_var;//ex) q1+q2
    std::vector<EquationToken> m_col_var;//ex) q3*q4

    float m_var_value[128];
    float *m_var_value_array[NUMVAR];
    //float** m_var_value;//各変数の値[128][SIMDW]
    std::vector<float> m_scalars; //calc. results of m_opa_var or m_col_var.
    FuncParser::ReversePolishNotation m_rpn;
    float m_max_opacity; 
    float m_max_density;
    float m_sampling_volume_inverse;
    float m_particle_density; 
    float m_particle_data_size_limit; 
    // add by shimomura 2022/12/20
    VariableRange m_variable_range;
    

public:

    std::vector<float> m_o_min; 
    std::vector<float> m_o_max; 
    std::vector<float> m_c_min; 
    std::vector<float> m_c_max; 


    TransferFunctionSynthesizer();

    TransferFunctionSynthesizer( TransferFunctionSynthesizer& tfs);

    EquationToken opacityFunction( void );

    EquationToken colorFunction( void );

    std::vector<EquationToken> opacityVariable( void );

    std::vector<EquationToken> colorVariable( void );

    void setOpacityFunction( EquationToken opa_func );

    void setColorFunction( EquationToken col_func );

    void setOpacityVariable( std::vector<EquationToken> opa_var );

    void setColorVariable( std::vector<EquationToken> col_var );

    void setMaxOpacity( const float opacity );
    void setMaxDensity( const float density );
    void setSamplingVolumeInverse( const float sampling_volume_inverse );
    std::vector<float> scalars() ;

    float getMaxOpacity() const;
    float getMaxDensity() const;
    float getSamplingVolumeInverse() const;

    //add by shimomura 2022/12/19
    void  setParticleDensity( float particle_density);
    void  setParticleDataSizeLimit( float particle_data_size_limit ); 

    float getParticleDensity();
    float getParticleDataSizeLimit();
    VariableRange& variableRange();                                                                                                                                           
    const VariableRange& variableRange() const;
    void  mergeVarRange( const TransferFunctionSynthesizer& tfs );

    void create();
    // function 4 debug  add by shimomura 2022/12/28 
    void setStabToken();


    ~TransferFunctionSynthesizer();

public:

    std::vector<float> SynthesizedOpacityScalars(
        //std::vector< pbvr::HexahedralCell<Type>* > interp ,
        std::vector< pbvr::CellBase<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord );

    std::vector<float> SynthesizedColorScalars(
        //std::vector< pbvr::HexahedralCell<Type>* > interp ,
        std::vector< pbvr::CellBase<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord );

    float CalculateOpacity(
        //std::vector< pbvr::HexahedralCell<Type>* > interp ,
        std::vector< pbvr::CellBase<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord,
        std::vector<pbvr::TransferFunction>& tf);

    kvs::RGBColor CalculateColor(
        //std::vector< pbvr::HexahedralCell<Type>* > interp ,
        std::vector< pbvr::CellBase<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord,
        std::vector<pbvr::TransferFunction>& tf);

    void CalculateOpacityArrayAverage(
        std::vector< pbvr::CellBase<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<pbvr::TransferFunction>& tf,
        float *opacity_array);

    void CalculateCoordArray(
        std::vector< pbvr::CellBase<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<pbvr::TransferFunction>& tf,
        const pbvr::CoordSynthesizerTokens cst,
//        float *new_coord_array);
        kvs::Vector3f *new_coord_array);

   void CalculateOpacityArray(
        std::vector< pbvr::CellBase<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<pbvr::TransferFunction>& tf,
        float *opacity_array);

    void CalculateColorArray(
        std::vector< pbvr::CellBase<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<pbvr::TransferFunction>& tf,
        kvs::RGBColor *color_seq);

    void SynthesizedOpacityScalarsArray(
        std::vector< pbvr::CellBase<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<float> *o_scalars_array);

    void SynthesizedColorScalarsArray(
        std::vector< pbvr::CellBase<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<float> *c_scalars_array);
    //Fj add end

//
//    void SynthesizedOpacityScalars(
//        std::vector< TFS::TrilinearInterpolator* > interp,
//        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//        float scalars[][SIMDW] );//resulting scalars[TF_COUNT][SIMDW]
//
//    void SynthesizedColorScalars(
//        std::vector< TFS::TrilinearInterpolator* > interp,
//        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//        float scalars[][SIMDW] );//resulting scalars[TF_COUNT][SIMDW]
//
//    void CalculateOpacity(
//        std::vector< TFS::TrilinearInterpolator* > interp,
//        std::vector<pbvr::TransferFunction>& tf,
//        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//        float* opacity );//resulting opacity[SIMDW]
//
//    void CalculateColor(
//        std::vector< TFS::TrilinearInterpolator* > interp ,
//        std::vector<pbvr::TransferFunction>& tf,
//        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//        kvs::UInt8* Red, kvs::UInt8* Green, kvs::UInt8* Blue );//resulting colors[SIMDW]
public:
    class NumericException : public std::runtime_error
    {
    public:
        NumericException( const std::string& str ):
            std::runtime_error( str )
        {
        }
        virtual ~NumericException() throw()
        {
        }
    };

    static void AssertValid( const float& v, const char* file, const int line );
};

#endif // __TRANSFER_FUNCTION_SYNTHESIZER_H__

