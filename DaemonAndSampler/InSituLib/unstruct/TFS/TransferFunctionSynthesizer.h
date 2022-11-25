#ifndef __TRANSFER_FUNCTION_SYNTHESIZER_H__
#define __TRANSFER_FUNCTION_SYNTHESIZER_H__

#include <map>
#include <string>
#include <vector>
#include "TransferFunction.h"
#include <kvs/RGBColor>
#include "HexahedralCell.h"
#include "../kvs_wrapper.h"
#include "ReversePolishNotation.h"
#include "Token.h"

#define TF_COUNT 5
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

    float m_var_value[128];
    float *m_var_value_array[NUMVAR];
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

    std::vector<float> SynthesizedOpacityScalars(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord );

    std::vector<float> SynthesizedColorScalars(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord );

    float CalculateOpacity(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord,
        std::vector<pbvr::TransferFunction>& tf);

    kvs::RGBColor CalculateColor(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        kvs::Vector3f local_coord, kvs::Vector3f global_coord,
        std::vector<pbvr::TransferFunction>& tf);

    //Fj add

    void CalculateOpacityArray(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<pbvr::TransferFunction>& tf,
        float *opacity_array);

    void CalculateColorArray(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<pbvr::TransferFunction>& tf,
        kvs::RGBColor *color_seq);

    void SynthesizedOpacityScalarsArray(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<float> *o_scalars_array);

    void SynthesizedColorScalarsArray(
        std::vector< pbvr::HexahedralCell<Type>* > interp ,
        const int loop_cnt,
        const kvs::Vector3f *local_coord,
        const kvs::Vector3f *global_coord,
        std::vector<float> *c_scalars_array);
    //Fj add end

};

#endif // __TRANSFER_FUNCTION_SYNTHESIZER_H__

