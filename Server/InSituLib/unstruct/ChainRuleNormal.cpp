#include "ChainRuleNormal.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../../VisModule/Visualization/Mapper/TransferFunctionSynthesizer.h"
#include "../../FunctionParser/ExpressionConverter.h"
#include "../../FunctionParser/ExpressionTokenizer.h"
#include "../../FunctionParser/ReversePolishNotation.h"
#include "../../FunctionParser/Token.h"

namespace
{

const float FiniteDifferenceScale = 1.0e-5f;

bool IsQVariable( const int variable_name, std::size_t* variable_index )
{
    if ( variable_name < Q1 || variable_name > Q23 ) return false;
    if ( ( variable_name - Q1 ) % 4 != 0 ) return false;

    *variable_index = static_cast<std::size_t>( ( variable_name - Q1 ) / 4 );
    return true;
}

vismodule::Vector3f SafeNormalFromGradient( const vismodule::Vector3f& grad_F )
{
    const float length = static_cast<float>( grad_F.length() );
    if ( !std::isfinite( length ) || length <= 0.0f ) return vismodule::Vector3f( 0.0f, 0.0f, 0.0f );
    return -( grad_F / length );
}

// Candidate integration points: replace calculation_glad() calls in kvs_wrapper.cpp
// around the existing particle-normal generation blocks after q_i gradients are available.

bool NearlyEqual( const vismodule::Vector3f& lhs, const vismodule::Vector3f& rhs, const float tolerance )
{
    return std::fabs( lhs.x() - rhs.x() ) <= tolerance &&
           std::fabs( lhs.y() - rhs.y() ) <= tolerance &&
           std::fabs( lhs.z() - rhs.z() ) <= tolerance;
}

} // namespace

namespace pbvr
{

ChainRuleNormalWorkspace::ChainRuleNormalWorkspace():
    m_expr( NULL )
{
    std::fill( m_variable_values, m_variable_values + 128, 0.0f );
}

void ChainRuleNormalWorkspace::setExpression( const EquationToken& expr, std::size_t nvariables )
{
    m_expr = &expr;
    m_active_variables.clear();

    for ( int i = 0; i < 128 && expr.exp_token[i] != END; ++i )
    {
        if ( expr.exp_token[i] != VARIABLE ) continue;

        std::size_t variable_index = 0;
        if ( !IsQVariable( expr.var_name[i], &variable_index ) ) continue;
        if ( variable_index >= nvariables ) continue;

        m_active_variables.push_back( variable_index );
    }

    std::sort( m_active_variables.begin(), m_active_variables.end() );
    m_active_variables.erase(
        std::unique( m_active_variables.begin(), m_active_variables.end() ),
        m_active_variables.end() );

    m_rpn.setExpToken( const_cast<int*>( &( expr.exp_token[0] ) ) );
    m_rpn.setVariableName( const_cast<int*>( &( expr.var_name[0] ) ) );
    m_rpn.setNumber( const_cast<float*>( &( expr.val_array[0] ) ) );
    m_rpn.setVariableValue( m_variable_values );
}

void ChainRuleNormalWorkspace::setVariableValue( std::size_t variable_name, float value )
{
    if ( variable_name >= 128 ) return;
    m_variable_values[variable_name] = value;
}

bool ChainRuleNormalWorkspace::computeGradient(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    vismodule::Vector3f* grad_F )
{
    if ( q_values.size() != grad_q.size() ) return false;
    if ( q_values.empty() ) return false;
    return this->computeGradient( expr, &q_values[0], &grad_q[0], q_values.size(), grad_F );
}

const std::vector<std::size_t>& ChainRuleNormalWorkspace::activeVariables() const
{
    return m_active_variables;
}

float ChainRuleNormalWorkspace::evalExpression()
{
    return m_rpn.eval();
}

bool ChainRuleNormalWorkspace::computeGradient(
    const float* q_values,
    const vismodule::Vector3f* grad_q,
    std::size_t nvariables,
    vismodule::Vector3f* grad_F )
{
    if ( m_expr == NULL || grad_F == NULL || q_values == NULL || grad_q == NULL ) return false;

    *grad_F = vismodule::Vector3f( 0.0f, 0.0f, 0.0f );

    for ( std::size_t i = 0; i < nvariables; ++i )
    {
        const std::size_t variable_name = Q1 + 4 * i;
        if ( variable_name >= 128 ) return false;
        m_variable_values[variable_name] = q_values[i];
    }

    for ( std::vector<std::size_t>::const_iterator it = m_active_variables.begin();
          it != m_active_variables.end(); ++it )
    {
        const std::size_t variable_index = *it;
        if ( variable_index >= nvariables ) return false;

        const std::size_t variable_name = Q1 + 4 * variable_index;
        const float base = q_values[variable_index];
        const float eps = FiniteDifferenceScale * std::max( 1.0f, std::fabs( base ) );

        m_variable_values[variable_name] = base + eps;
        const float plus = evalExpression();

        m_variable_values[variable_name] = base - eps;
        const float minus = evalExpression();

        m_variable_values[variable_name] = base;

        const float dF_dqi = ( plus - minus ) / ( 2.0f * eps );
        if ( !std::isfinite( dF_dqi ) ) continue;

        *grad_F += grad_q[variable_index] * dF_dqi;
    }

    return std::isfinite( grad_F->x() ) &&
           std::isfinite( grad_F->y() ) &&
           std::isfinite( grad_F->z() );
}

bool ChainRuleNormalWorkspace::computeGradient(
    const EquationToken& expr,
    const float* q_values,
    const vismodule::Vector3f* grad_q,
    std::size_t nvariables,
    vismodule::Vector3f* grad_F )
{
    this->setExpression( expr, nvariables );
    return this->computeGradient( q_values, grad_q, nvariables, grad_F );
}

bool ComputeChainRuleGradient(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    vismodule::Vector3f* grad_F )
{
    ChainRuleNormalWorkspace workspace;
    workspace.setExpression( expr, q_values.size() );
    if ( q_values.size() != grad_q.size() || q_values.empty() ) return false;
    return workspace.computeGradient( &q_values[0], &grad_q[0], q_values.size(), grad_F );
}

bool ComputeChainRuleNormal(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    vismodule::Vector3f* normal )
{
    if ( normal == NULL ) return false;

    vismodule::Vector3f grad_F;
    const bool ok = ComputeChainRuleGradient( expr, q_values, grad_q, &grad_F );
    *normal = ok ? SafeNormalFromGradient( grad_F ) : vismodule::Vector3f( 0.0f, 0.0f, 0.0f );
    return ok;
}

bool CompareChainRuleNormalWithFiniteDifference(
    const EquationToken& expr,
    const std::vector<float>& q_values,
    const std::vector<vismodule::Vector3f>& grad_q,
    const vismodule::Vector3f& finite_difference_grad,
    float* normal_dot )
{
    if ( normal_dot == NULL ) return false;

    vismodule::Vector3f chain_normal;
    const bool ok = ComputeChainRuleNormal( expr, q_values, grad_q, &chain_normal );
    const vismodule::Vector3f fd_normal = SafeNormalFromGradient( finite_difference_grad );
    *normal_dot = chain_normal.dot( fd_normal );
    return ok && std::isfinite( *normal_dot );
}

EquationToken MakeEquationTokenForChainRuleTest( const std::string& expression )
{
    FuncParser::ExpressionTokenizer tokenizer;
    FuncParser::ExpressionConverter converter;
    EquationToken token;

    tokenizer.tokenizeString( expression );
    converter.convertExpToken( tokenizer.m_exp_token );

    const int size = static_cast<int>( converter.token_array.size() );
    if ( size > 128 ) std::cerr << "Equation length too long" << std::endl;

    for ( int i = 0; i < 128; ++i )
    {
        if ( i < size )
        {
            token.exp_token[i] = converter.token_array[i];
            token.var_name[i] = converter.var_array[i];
            token.val_array[i] = converter.value_array[i];
        }
        else
        {
            token.exp_token[i] = 0;
            token.var_name[i] = 0;
            token.val_array[i] = 0.0f;
        }
    }

    return token;
}

bool RunChainRuleNormalSelfTest()
{
    const std::vector<vismodule::Vector3f> grad_q = {
        vismodule::Vector3f( 1.0f, 2.0f, 3.0f ),
        vismodule::Vector3f( -2.0f, 1.0f, 0.5f ),
        vismodule::Vector3f( 0.25f, -0.75f, 2.0f )
    };

    vismodule::Vector3f grad_F;

    if ( !ComputeChainRuleGradient(
             MakeEquationTokenForChainRuleTest( "q1" ),
             std::vector<float>{ 2.0f, 3.0f, 4.0f },
             grad_q,
             &grad_F ) || !NearlyEqual( grad_F, grad_q[0], 1.0e-2f ) )
    {
        std::cerr << "ChainRuleNormal self-test failed: F=q1, grad_F=" << grad_F << std::endl;
        return false;
    }

    if ( !ComputeChainRuleGradient(
             MakeEquationTokenForChainRuleTest( "q1+q2" ),
             std::vector<float>{ 2.0f, 3.0f, 4.0f },
             grad_q,
             &grad_F ) || !NearlyEqual( grad_F, grad_q[0] + grad_q[1], 1.0e-2f ) )
    {
        std::cerr << "ChainRuleNormal self-test failed: F=q1+q2, grad_F=" << grad_F << std::endl;
        return false;
    }

    if ( !ComputeChainRuleGradient(
             MakeEquationTokenForChainRuleTest( "q1*q2" ),
             std::vector<float>{ 2.0f, 3.0f, 4.0f },
             grad_q,
             &grad_F ) || !NearlyEqual( grad_F, grad_q[0] * 3.0f + grad_q[1] * 2.0f, 2.0e-2f ) )
    {
        std::cerr << "ChainRuleNormal self-test failed: F=q1*q2, grad_F=" << grad_F << std::endl;
        return false;
    }

    const std::vector<float> q_values = { 2.0f, 3.0f, 6.0f };
    const float F = std::sqrt( q_values[0] * q_values[0] + q_values[1] * q_values[1] + q_values[2] * q_values[2] );
    const vismodule::Vector3f expected =
        grad_q[0] * ( q_values[0] / F ) +
        grad_q[1] * ( q_values[1] / F ) +
        grad_q[2] * ( q_values[2] / F );

    if ( !ComputeChainRuleGradient(
             MakeEquationTokenForChainRuleTest( "sqrt(q1*q1+q2*q2+q3*q3)" ),
             q_values,
             grad_q,
             &grad_F ) || !NearlyEqual( grad_F, expected, 2.0e-2f ) )
    {
        std::cerr << "ChainRuleNormal self-test failed: F=sqrt(...), grad_F=" << grad_F
                  << ", expected=" << expected << std::endl;
        return false;
    }

    return true;
}

} // namespace pbvr
