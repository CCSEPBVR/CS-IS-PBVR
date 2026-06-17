#ifndef PBVR_CHAIN_RULE_NORMAL_H
#define PBVR_CHAIN_RULE_NORMAL_H

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
    float m_variable_values[128];
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

} // namespace pbvr

#endif // PBVR_CHAIN_RULE_NORMAL_H
