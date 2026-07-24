#ifndef PBVR_CHAIN_RULE_NORMAL_H
#define PBVR_CHAIN_RULE_NORMAL_H

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>
#include <vismodule/Vector3>

#include "ReversePolishNotation.h"

struct EquationToken;

namespace pbvr
{

class ChainRuleNormalWorkspace
{
public:
    ChainRuleNormalWorkspace();

    void setExpression( const EquationToken& expr, std::size_t nvariables );
    void setVariableValueBuffer( float* variable_values );
    void setVariableValue( std::size_t variable_name, float value );
    bool computeGradient(
        const float* q_values,
        const vismodule::Vector3f* grad_q,
        std::size_t nvariables,
        vismodule::Vector3f* grad_F );
    bool computeGradient(
        const EquationToken& expr,
        const float* q_values,
        const vismodule::Vector3f* grad_q,
        std::size_t nvariables,
        vismodule::Vector3f* grad_F );
    bool computeGradient(
        const EquationToken& expr,
        const std::vector<float>& q_values,
        const std::vector<vismodule::Vector3f>& grad_q,
        vismodule::Vector3f* grad_F );

    const std::vector<std::size_t>& activeVariables() const;

private:
    float evalExpression();

    const EquationToken* m_expr;
    std::vector<std::size_t> m_active_variables;
    float m_owned_variable_values[128];
    float* m_variable_values;
    FuncParser::ReversePolishNotation m_rpn;
};

bool ComputeChainRuleGradient(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    vismodule::Vector3f* grad_F );

bool ComputeChainRuleNormal(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    vismodule::Vector3f* normal );

bool CompareChainRuleNormalWithFiniteDifference(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    const vismodule::Vector3f& finite_difference_grad,
    float* normal_dot );

EquationToken MakeEquationTokenForChainRuleTest( const std::string& expression );
bool RunChainRuleNormalSelfTest();

// 変動係数 CoV = sqrt(Var)/|mu| のスカラー勾配の「方向」を構成する。
//
//   grad CoV = grad Var/(2*sigma*|mu|) - sigma*sign(mu)*grad mu/mu^2      (sigma = sqrt(Var))
//
// 両辺に正のスカラー 2*sigma*|mu| を掛けると平方根も sigma による除算も消える。
// 法線は正規化されて方向のみが意味を持つため、次形で十分である(比例係数は常に正)。
//
//   grad CoV  ∝  grad Var - (2*Var/mu) * grad mu
//
// 引数は「真の勾配」であること。呼び出し側が別の符号規約(例 -grad)で保持している
// 場合は、本関数へ渡す前に規約を揃えること(本関数は grad Var・grad mu の双方について
// 線形なので、両者が同一規約なら係数はそのままで成立する)。
// |mean| <= mean_threshold では CoV 値自体が定義されないため variance_grad を返す
// (従来動作へのフォールバック)。
inline vismodule::Vector3f ComputeCoVNormalDirection(
    const float mean,
    const vismodule::Vector3f& mean_grad,
    const float variance,
    const vismodule::Vector3f& variance_grad,
    const float mean_threshold = 1.0e-5f )
{
    // NaN が来た場合も条件が偽になりフォールバックする書き方にしている。
    if ( !( std::fabs( mean ) > mean_threshold ) ) return variance_grad;
    return variance_grad - mean_grad * ( 2.0f * variance / mean );
}

bool RunCoVNormalSelfTest();

} // namespace pbvr

#endif // PBVR_CHAIN_RULE_NORMAL_H
