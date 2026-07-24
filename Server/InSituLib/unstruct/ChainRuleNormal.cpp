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
    m_expr( NULL ),
    m_variable_values( m_owned_variable_values )
{
    std::fill( m_owned_variable_values, m_owned_variable_values + 128, 0.0f );
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

void ChainRuleNormalWorkspace::setVariableValueBuffer( float* variable_values )
{
    m_variable_values = variable_values != NULL ? variable_values : m_owned_variable_values;
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
    for ( std::size_t i = 0; i < nvariables; ++i )
    {
        const std::size_t variable_name = Q1 + 4 * i;
        if ( variable_name >= 128 ) return false;
        m_variable_values[variable_name] = q_values[i];
    }
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

// ---------------------------------------------------------------------------
// 変動係数(CoV)勾配の単体テスト
// ---------------------------------------------------------------------------

// 単位ベクトル化した内積で「方向のみ」を比較する。
static bool SameDirection(
    const vismodule::Vector3f& lhs,
    const vismodule::Vector3f& rhs,
    const float tolerance )
{
    const double llen = lhs.length();
    const double rlen = rhs.length();
    if ( !( llen > 0.0 ) || !( rlen > 0.0 ) ) return false;

    const vismodule::Vector3f a = lhs / static_cast<float>( llen );
    const vismodule::Vector3f b = rhs / static_cast<float>( rlen );
    return a.dot( b ) >= 1.0f - tolerance;
}

// CoV = sqrt(Var)/|mu| の場を直接数値微分し、解析式の導出そのものを検算する。
// mu, Var は各軸方向に線形に変化する場として与える。
static vismodule::Vector3f NumericalCoVGradient(
    const double mu0,
    const vismodule::Vector3f& grad_mu,
    const double var0,
    const vismodule::Vector3f& grad_var,
    const double h )
{
    const double dmu[3]  = { grad_mu.x(),  grad_mu.y(),  grad_mu.z()  };
    const double dvar[3] = { grad_var.x(), grad_var.y(), grad_var.z() };

    float out[3];
    for ( int axis = 0; axis < 3; ++axis )
    {
        const double cov_p = std::sqrt( var0 + dvar[axis] * h ) / std::fabs( mu0 + dmu[axis] * h );
        const double cov_m = std::sqrt( var0 - dvar[axis] * h ) / std::fabs( mu0 - dmu[axis] * h );
        out[axis] = static_cast<float>( ( cov_p - cov_m ) / ( 2.0 * h ) );
    }
    return vismodule::Vector3f( out[0], out[1], out[2] );
}

bool RunCoVNormalSelfTest()
{
    const vismodule::Vector3f ex( 1.0f, 0.0f, 0.0f );
    const vismodule::Vector3f ey( 0.0f, 1.0f, 0.0f );
    const vismodule::Vector3f zero( 0.0f, 0.0f, 0.0f );
    const float tol = 1.0e-6f;
    bool ok = true;

    // T1: Var 一定、mu = 1 + x (grad mu = ex)。 grad CoV ∝ -(2*Var/mu)*ex
    {
        const float mu = 1.0f, var = 3.0f;
        const vismodule::Vector3f got = ComputeCoVNormalDirection( mu, ex, var, zero );
        const vismodule::Vector3f expected = ex * ( -2.0f * var / mu );
        if ( !SameDirection( got, expected, tol ) )
        {
            std::cerr << "CoVNormal self-test failed: T1 got=" << got
                      << ", expected=" << expected << std::endl;
            ok = false;
        }
    }

    // T2: mu 一定 = 2、Var = 1 + y (grad Var = ey)。 grad CoV ∝ ey
    {
        const float mu = 2.0f, var = 1.0f;
        const vismodule::Vector3f got = ComputeCoVNormalDirection( mu, zero, var, ey );
        if ( !SameDirection( got, ey, tol ) )
        {
            std::cerr << "CoVNormal self-test failed: T2 got=" << got
                      << ", expected=" << ey << std::endl;
            ok = false;
        }
    }

    // T3: mu = 1 + x、Var = 1 + y を (x,y)=(0.5,0.25) で評価。
    //     grad CoV ∝ ey - (2*Var/mu)*ex
    {
        const float mu = 1.5f, var = 1.25f;
        const vismodule::Vector3f got = ComputeCoVNormalDirection( mu, ex, var, ey );
        const vismodule::Vector3f expected = ey - ex * ( 2.0f * var / mu );
        if ( !SameDirection( got, expected, tol ) )
        {
            std::cerr << "CoVNormal self-test failed: T3 got=" << got
                      << ", expected=" << expected << std::endl;
            ok = false;
        }

        // 検算: CoV 場の中央差分と方向が一致すること(解析式の導出そのものの確認)。
        const vismodule::Vector3f numeric = NumericalCoVGradient( mu, ex, var, ey, 1.0e-3 );
        if ( !SameDirection( got, numeric, 1.0e-5f ) )
        {
            std::cerr << "CoVNormal self-test failed: T3(numeric) got=" << got
                      << ", numeric=" << numeric << std::endl;
            ok = false;
        }
    }

    // T4: mu < 0 (符号の確認)。 mu=-2, Var=1 -> ey - (2*1/-2)*ex = ey + ex
    {
        const float mu = -2.0f, var = 1.0f;
        const vismodule::Vector3f got = ComputeCoVNormalDirection( mu, ex, var, ey );
        const vismodule::Vector3f expected = ey - ex * ( 2.0f * var / mu );
        if ( !SameDirection( got, expected, tol ) )
        {
            std::cerr << "CoVNormal self-test failed: T4 got=" << got
                      << ", expected=" << expected << std::endl;
            ok = false;
        }

        const vismodule::Vector3f numeric = NumericalCoVGradient( mu, ex, var, ey, 1.0e-3 );
        if ( !SameDirection( got, numeric, 1.0e-5f ) )
        {
            std::cerr << "CoVNormal self-test failed: T4(numeric) got=" << got
                      << ", numeric=" << numeric << std::endl;
            ok = false;
        }
    }

    // T5: |mu| <= 1e-5 はフォールバック(variance_grad をそのまま返す)。
    {
        const float mu = 1.0e-6f, var = 1.0f;
        const vismodule::Vector3f got = ComputeCoVNormalDirection( mu, ex, var, ey );
        if ( !( got.x() == ey.x() && got.y() == ey.y() && got.z() == ey.z() ) )
        {
            std::cerr << "CoVNormal self-test failed: T5 got=" << got
                      << ", expected(fallback)=" << ey << std::endl;
            ok = false;
        }
    }

    // T6: kvs_wrapper.cpp 本体と同じ積算・スケーリング・符号規約を再現し、
    //     tmp_varience_normals が -grad Var に、呼び出し側の渡し方が -grad CoV に
    //     なることを検算する(呼び出し側の符号規約そのものの試験)。
    {
        const int n = 3;
        const float g[3] = { 1.0f, 2.0f, 4.0f };
        const vismodule::Vector3f gg[3] = {
            vismodule::Vector3f(  0.5f, -0.25f,  0.125f ),
            vismodule::Vector3f( -1.0f,  0.75f,  0.5f   ),
            vismodule::Vector3f(  0.25f, 1.5f,  -0.75f  )
        };

        // --- store_uniform_block / リング積算と同じ符号で貯める ---
        float scal = 0.0f, sq = 0.0f;
        vismodule::Vector3f norm_acc( 0.0f, 0.0f, 0.0f ); // -grad g を積算
        vismodule::Vector3f tmp_acc( 0.0f, 0.0f, 0.0f );  // +g*grad g を積算
        for ( int m = 0; m < n; ++m )
        {
            scal += g[m];
            sq   += g[m] * g[m];
            norm_acc = norm_acc - gg[m];
            tmp_acc  = tmp_acc + gg[m] * g[m];
        }

        // --- 統計ブロックと同じスケーリング ---
        const float inv = 1.0f / static_cast<float>( n );
        const float mu  = scal * inv;
        const vismodule::Vector3f tmp_term = tmp_acc * ( -2.0f * inv ); // = -2 E[g grad g]
        const vismodule::Vector3f vnorm    = norm_acc * ( -inv );       // = +grad mu
        float var = sq * inv - mu * mu;
        if ( var < 0.0f ) var = 0.0f;
        const vismodule::Vector3f n_var = tmp_term - vnorm * ( -2.0f * mu ); // = -grad Var

        // --- 真値 ---
        vismodule::Vector3f grad_mu( 0.0f, 0.0f, 0.0f );
        vismodule::Vector3f e_g_grad_g( 0.0f, 0.0f, 0.0f );
        for ( int m = 0; m < n; ++m )
        {
            grad_mu    = grad_mu + gg[m] * inv;
            e_g_grad_g = e_g_grad_g + gg[m] * ( g[m] * inv );
        }
        const vismodule::Vector3f grad_var = ( e_g_grad_g - grad_mu * mu ) * 2.0f;

        if ( !SameDirection( n_var, grad_var * -1.0f, 1.0e-5f ) )
        {
            std::cerr << "CoVNormal self-test failed: T6(-grad Var) n_var=" << n_var
                      << ", expected=" << ( grad_var * -1.0f ) << std::endl;
            ok = false;
        }

        // 呼び出し側と同じ渡し方: grad mu 側の符号を反転して渡す。
        const vismodule::Vector3f cov_n = ComputeCoVNormalDirection( mu, vnorm * -1.0f, var, n_var );
        const vismodule::Vector3f grad_cov = grad_var - grad_mu * ( 2.0f * var / mu );
        if ( !SameDirection( cov_n, grad_cov * -1.0f, 1.0e-5f ) )
        {
            std::cerr << "CoVNormal self-test failed: T6(-grad CoV) got=" << cov_n
                      << ", expected=" << ( grad_cov * -1.0f ) << std::endl;
            ok = false;
        }

        // 誤って符号を揃えずに渡した場合は方向が一致しないこと(試験自体の感度確認)。
        const vismodule::Vector3f cov_wrong = ComputeCoVNormalDirection( mu, vnorm, var, n_var );
        if ( SameDirection( cov_wrong, grad_cov * -1.0f, 1.0e-5f ) )
        {
            std::cerr << "CoVNormal self-test failed: T6(sensitivity) "
                         "wrong-sign variant unexpectedly matched" << std::endl;
            ok = false;
        }
    }

    if ( ok ) std::cout << "CoVNormal self-test: all cases passed (T1-T6)" << std::endl;
    return ok;
}

} // namespace pbvr
