#ifndef __TRANSFER_FUNCTION_SYNTHESIZER_H__
#define __TRANSFER_FUNCTION_SYNTHESIZER_H__

#include <map>
#include <string>
#include <kvs/RGBColor>
//#include <kvs/TrilinearInterpolator>
#include "TrilinearInterpolator.h"
#include "TransferFunction.h"
#include "../kvs_wrapper.h"
#include "ReversePolishNotation.h"
#include "Token.h"

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

class TransferFunctionSynthesizer
{
protected:
    //2019 kawamura
    EquationToken m_opa_func;//ex) A1+A2*A3
    EquationToken m_col_func;//ex) C1/C2-C3
    std::vector<EquationToken> m_opa_var;//ex) q1+q2
    std::vector<EquationToken> m_col_var;//ex) q3*q4

    float** m_var_value;//各変数の値[128][SIMDW]
    std::vector<float> m_scalars; //calc. results of m_opa_var or m_col_var.
    FuncParser::ReversePolishNotation m_rpn;

public:

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

    ~TransferFunctionSynthesizer();

public:

    void SynthesizedOpacityScalars(
        std::vector< TFS::TrilinearInterpolator* > interp,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        float scalars[][SIMDW] );//resulting scalars[TF_COUNT][SIMDW]

    void SynthesizedColorScalars(
        std::vector< TFS::TrilinearInterpolator* > interp,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        float scalars[][SIMDW] );//resulting scalars[TF_COUNT][SIMDW]

    void CalculateOpacity(
        std::vector< TFS::TrilinearInterpolator* > interp,
        std::vector<pbvr::TransferFunction>& tf,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        float* opacity );//resulting opacity[SIMDW]

    void CalculateColor(
        std::vector< TFS::TrilinearInterpolator* > interp ,
        std::vector<pbvr::TransferFunction>& tf,
        float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
        float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
        kvs::UInt8* Red, kvs::UInt8* Green, kvs::UInt8* Blue );//resulting colors[SIMDW]
};

#endif // __TRANSFER_FUNCTION_SYNTHESIZER_H__

