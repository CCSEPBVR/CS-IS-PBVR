#ifndef __TRANSFER_FUNCTION_SYNTHESIZER_H__
#define __TRANSFER_FUNCTION_SYNTHESIZER_H__

#include <map>
#include <string>
#include <vector>
#include <vismodule/RGBColor>
#include <vismodule/HexahedralCell>
#include <vismodule/TrilinearInterpolator>
#include <vismodule/TransferFunction>
//#include "ReversePolishNotation.h"
#include "../../../FunctionParser/ReversePolishNotation.h"
#include "../../../FunctionParser/Token.h"
//add by shimomura  2022/12/20
//#include "../Common/VariableRange.h"
#include <vismodule/VariableRange>
#include "../../../FunctionParser/ExpressionTokenizer.h"
#include "../../../FunctionParser/ExpressionConverter.h"


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

public:
    class VolumeEquation
    {
    public:
        std::string m_name;
        std::string m_equation;
    };

protected:
    //2019 kawamura
    EquationToken m_opa_func;//ex) A1+A2*A3
    EquationToken m_col_func;//ex) C1/C2-C3
    std::vector<EquationToken> m_opa_var;//ex) q1+q2
    std::vector<EquationToken> m_col_var;//ex) q3*q4

    std::string m_opa_func_synthesis;//ex) A1+A2*A3
    std::string m_col_func_synthesis;//ex) C1/C2-C3
    std::vector<std::string> m_opa_var_synthesis;//ex) q1+q2
    std::vector<std::string> m_col_var_synthesis;//ex) q3*q4
    
    float m_var_value[128];
    float **m_var_value_struct;
    float *m_var_value_array[NUMVAR];
    //float** m_var_value;//各変数の値[128][SIMDW]
    std::vector<float> m_scalars; //calc. results of m_opa_var or m_col_var.
    FuncParser::ReversePolishNotation m_rpn;
    float m_max_opacity; 
    float m_max_density;
    float m_sampling_volume_inverse;
    float m_extra_opacity_factor; 
    float m_particle_data_size_limit; 
    // add by shimomura 2022/12/20
    VariableRange m_variable_range;

  
public:

    // expression 
    std::string m_color_transfer_function_synthesis;
    std::string m_opacity_transfer_function_synthesis;
    VolumeEquation m_volume_equation;
    // expression
 
//    std::string color_transfer_function_synthesis(){ return m_color_transfer_function_synthesis  ;}
//    std::string opacity_transfer_function_synthesis(){ return m_opacity_transfer_function_synthesis;}
//
//    VolumeEquation VolumeEquation(){return m_volume_equation;} 
//
//    void setColorTransferFunctionSynthesis(std::string color_transfer_function_synthesis ); 
//    void setOpacityTransferFunctionSynthesis(std::string color_transfer_function_synthesis );  
//    void setVolumeEquation(VolumeEquation volume_equation );

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
    void  setExtraOpacityFactor( float extra_opacity_factor );
    void  setParticleDataSizeLimit( float particle_data_size_limit ); 

    float getExtraOpacityFactor();
    float getParticleDataSizeLimit();
    VariableRange& variableRange();                                                                                                                                           
    const VariableRange& variableRange() const;
    void  mergeVarRange( const TransferFunctionSynthesizer& tfs );

    void create();

    //void convert_token(std::vector<VolumeEquation> voleq); 
    EquationToken convert_token(std::string expression); 

    // function 4 debug  add by shimomura 2022/12/28 
    void setStabToken();


    ~TransferFunctionSynthesizer();

public:
//-----structured---------

    void SynthesizedOpacityScalars(
        std::vector< vismodule::TrilinearInterpolator* > interp,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        //float scalars[][SIMDW] );//resulting scalars[TF_COUNT][SIMDW]
        float** scalars);//resulting scalars[TF_COUNT][SIMDW]

    void SynthesizedColorScalars(
        std::vector< vismodule::TrilinearInterpolator* > interp,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        //float scalars[][SIMDW] );//resulting scalars[TF_COUNT][SIMDW]
        float** scalars );//resulting scalars[TF_COUNT][SIMDW]

    void CalculateOpacity(
        std::vector< vismodule::TrilinearInterpolator* > interp,
        std::vector<vismodule::TransferFunction>& tf,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        float* opacity );//resulting opacity[SIMDW]

    void CalculateColor(
        std::vector< vismodule::TrilinearInterpolator* > interp ,
        std::vector<vismodule::TransferFunction>& tf,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        vismodule::UInt8* Red, vismodule::UInt8* Green, vismodule::UInt8* Blue );//resulting colors[SIMDW]

    void CalculateCoordArray(
        std::vector< vismodule::TrilinearInterpolator* > interp ,
        const int loop_cnt,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        //const vismodule::Vector3f *local_coord,
        //const vismodule::Vector3f *global_coord,
        std::vector<vismodule::TransferFunction>& tf,
        const vismodule::CoordSynthesizerStrings css,
        float* nx_g, float* ny_g, float* nz_g ); //global coord[SIMDW])
        //vismodule::Vector3f *new_coord_array);


//-----unstructured-------
    std::vector<float> SynthesizedOpacityScalars(
        std::vector< vismodule::CellBase<Type>* > interp ,
        vismodule::Vector3f local_coord, vismodule::Vector3f global_coord );

    std::vector<float> SynthesizedColorScalars(
        std::vector< vismodule::CellBase<Type>* > interp ,
        vismodule::Vector3f local_coord, vismodule::Vector3f global_coord );

    float CalculateOpacity(
        std::vector< vismodule::CellBase<Type>* > interp ,
        vismodule::Vector3f local_coord, vismodule::Vector3f global_coord,
        std::vector<vismodule::TransferFunction>& tf);

    vismodule::RGBColor CalculateColor(
        std::vector< vismodule::CellBase<Type>* > interp ,
        vismodule::Vector3f local_coord, vismodule::Vector3f global_coord,
        std::vector<vismodule::TransferFunction>& tf);

    void CalculateOpacityArrayAverage(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<vismodule::TransferFunction>& tf,
        float *opacity_array);

    void CalculateCoordArray(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<vismodule::TransferFunction>& tf,
        const vismodule::CoordSynthesizerStrings css,
        vismodule::Vector3f *new_coord_array);

   void CalculateOpacityArray(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<vismodule::TransferFunction>& tf,
        float *opacity_array);

    void CalculateColorArray(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<vismodule::TransferFunction>& tf,
        vismodule::RGBColor *color_seq);

    void CalculateScalarsArray(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<vismodule::TransferFunction>& tf,
        float *scalars_array);

    void SynthesizedOpacityScalarsArray(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<float> *o_scalars_array);

    void SynthesizedColorScalarsArray(
        std::vector< vismodule::CellBase<Type>* > interp ,
        const int loop_cnt,
        const vismodule::Vector3f *local_coord,
        const vismodule::Vector3f *global_coord,
        std::vector<float> *c_scalars_array);
    //Fj add end

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
