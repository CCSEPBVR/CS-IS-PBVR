
#include <cstdio>
#include <cfloat>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <climits>

#include <vismodule/ValueArray>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/PointExporter>
#include <vismodule/RGBColor>
#include <vismodule/MersenneTwister>
#include <cstdint>
#include <vismodule/CellBase>
#include <vismodule/FrequencyTable>
#include <vismodule/TetrahedralCell>
#include <vismodule/HexahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/TrilinearInterpolator>  // 構造格子版アンサンブルの補間器
#ifdef ENABLE_ENSEMBLE_TIMER
#include <vismodule/Timer>
#endif

#ifndef CPU_VER
#include <mpi.h>
#endif

#include <vismodule/ParticleProperty>

#include <vismodule/ParameterFileReader>
#include <vismodule/ParameterFileWriter>

// Generate
#include <vismodule/GenerateParticle>
#include <vismodule/PointObjectGenerator>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/PlotOverLineGenerator>
#include <vismodule/PlotOverTimeGenerator>

#ifdef EXTEND_FILE_FORMAT
#include <vismodule/UnstructuredVolumeImporter>
#include <vtkSmartPointerBase.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/extendedfileformat/VtkUnstructuredFileFormat>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>
#include <kvs/extendedfileformat/VtkImporter>
#endif

#include "ChainRuleNormal.h"
#include "EnsembleCellHistogram.h"
#include "EnsembleParticleGenerator.h"

using namespace vismodule;
namespace Generator = vismodule::CellByCellParticleGenerator;

namespace
{


inline const size_t CalculateNumberOfParticlesV35(
    const float density,
    const float volume_of_cell,
    const float repetition,
    vismodule::MersenneTwister* mt
)
{
    const float n_particles = density * volume_of_cell * repetition;
    const float random = static_cast<float>( mt->rand() );

    size_t n = static_cast<size_t>( n_particles );
    if ( n_particles - n > random ) ++n;

    return n;
}

inline size_t ExpandedCapacity( const size_t current_capacity, const size_t required_capacity )
{
    if ( current_capacity >= required_capacity ) return current_capacity;
    size_t capacity = current_capacity > 0 ? current_capacity : 4096;
    while ( capacity < required_capacity ) capacity *= 2;
    return capacity;
}

void ReserveAdditionalUniformParticles(
    const size_t additional_particles,
    std::vector<vismodule::Real32>& coords,
    std::vector<vismodule::Real32>& scalars,
    std::vector<vismodule::Real32>& normals,
    std::vector<int>& cellids,
    std::vector<vismodule::Real32>& sq_scalars,
    std::vector<vismodule::Real32>& tmp_term )
{
    const size_t particle_capacity = ExpandedCapacity( scalars.capacity(), scalars.size() + additional_particles );
    scalars.reserve( particle_capacity );
    cellids.reserve( particle_capacity );
    sq_scalars.reserve( particle_capacity );

    const size_t vector_capacity = particle_capacity * 3;
    coords.reserve( vector_capacity );
    normals.reserve( vector_capacity );
    tmp_term.reserve( vector_capacity );
}

struct ChainRuleEvalContext
{
    ChainRuleEvalContext():
        valid( false )
    {
        std::fill( variable_values, variable_values + 128, 0.0f );
    }

    void initialize( const ::EquationToken equation_token, const int nvariables )
    {
        valid = false;
        expr = equation_token;
        // 現状の統計粒子生成は TF 数 1 個想定
        rpn.setExpToken( &( expr.exp_token[0] ) );
        rpn.setVariableName( &( expr.var_name[0] ) );
        rpn.setNumber( &( expr.val_array[0] ) );
        rpn.setVariableValue( variable_values );
        workspace.setVariableValueBuffer( variable_values );
        workspace.setExpression( expr, static_cast<std::size_t>( nvariables ) );
        valid = true;
    }

    bool valid;
    ::EquationToken expr;
    FuncParser::ReversePolishNotation rpn;
    vismodule::ChainRuleNormalWorkspace workspace;
    float variable_values[128];
};

void AppendRejectedStatisticParticle(
    const float scalar,
    const vismodule::Vector3f& coord,
    const vismodule::Vector3f& normal,
    vismodule::TransferFunction& tf,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density,
    vismodule::MersenneTwister* mt,
    std::vector<float>& coords,
    std::vector<Byte>& colors,
    std::vector<float>& normals
)
{
    const float opacity = tf.opacityMap().at( scalar );
    const float density = opacity < max_opacity ?
        -std::log( 1.0f - opacity ) * sampling_volume_inverse :
        max_density;

    if ( density <= max_density * static_cast<float>( mt->rand() ) ) return;

    const vismodule::RGBColor color = tf.colorMap().at( scalar );

    coords.push_back( coord.x() );
    coords.push_back( coord.y() );
    coords.push_back( coord.z() );
    colors.push_back( color.r() );
    colors.push_back( color.g() );
    colors.push_back( color.b() );
    normals.push_back( normal.x() );
    normals.push_back( normal.y() );
    normals.push_back( normal.z() );
}

bool EnsembleHistogramBin(
    const float value,
    const float min_value,
    const float max_value,
    size_t& bin
)
{
    if ( value < min_value || value > max_value ) return false;

    bin = 0;
    if ( vismodule::Math::Equal<float>( min_value, max_value ) ) return true;

    const float position = ( value - min_value ) / ( max_value - min_value ) * DEFAULT_NBINS;
    bin = static_cast<size_t>( position );
    if ( bin >= DEFAULT_NBINS ) bin = DEFAULT_NBINS - 1;
    return true;
}

// 統計量(平均・分散)の値を常用対数化する。変動係数は対象外(呼び出し前に算出済み)。
// 非正値は下限クランプ log10(max(値, delta)) で NaN(log10(負))/-Inf(log10(0)) を回避。
void ApplyLog10ToMeanVariance( std::vector<float>& mean, std::vector<float>& variance )
{
    const float floor_value = 1.0e-30f;
    for ( size_t i = 0; i < mean.size(); ++i )
        mean[i] = std::log10( std::max( mean[i], floor_value ) );
    for ( size_t i = 0; i < variance.size(); ++i )
        variance[i] = std::log10( std::max( variance[i], floor_value ) );
}

EnsembleStatisticRange MakeEnsembleStatisticMinMax(
    const std::vector<float>& values,
    const int tf_number
)
{
    EnsembleStatisticRange range;
    range.min_values.assign( tf_number * 2, FLT_MAX );
    range.max_values.assign( tf_number * 2, -FLT_MAX );
    range.o_bins.assign( tf_number * DEFAULT_NBINS, 0 );
    range.c_bins.assign( tf_number * DEFAULT_NBINS, 0 );

    if ( values.empty() ) return range;

    float min_value = FLT_MAX;
    float max_value = -FLT_MAX;
    for ( const float value : values )
    {
        min_value = vismodule::Math::Min( min_value, value );
        max_value = vismodule::Math::Max( max_value, value );
    }

    for ( int i = 0; i < tf_number; i++ )
    {
        range.min_values[2 * i    ] = min_value;
        range.max_values[2 * i    ] = max_value;
        range.min_values[2 * i + 1] = min_value;
        range.max_values[2 * i + 1] = max_value;
    }

    return range;
}

void AggregateEnsembleStatisticMinMax(
    EnsembleStatisticRange& range,
    const int tf_number,
    MPI_Comm comm = MPI_COMM_WORLD
)
{
#ifndef CPU_VER
    MPI_Allreduce( MPI_IN_PLACE, range.min_values.data(), tf_number * 2, MPI_FLOAT, MPI_MIN, comm );
    MPI_Allreduce( MPI_IN_PLACE, range.max_values.data(), tf_number * 2, MPI_FLOAT, MPI_MAX, comm );
#else
    (void)comm;
#endif

    for ( int i = 0; i < tf_number * 2; i++ )
    {
        if ( range.min_values[i] == FLT_MAX || range.max_values[i] == -FLT_MAX )
        {
            range.min_values[i] = 0.0f;
            range.max_values[i] = 0.0f;
        }
    }
}

void ApplyEnsembleStatisticMinMax(
    const EnsembleStatisticRange& range,
    std::vector<EnsembleTransferFunction>& transfunc_array,
    const int tf_number
)
{
    for ( int i = 0; i < tf_number; i++ )
    {
        const float min_value = range.min_values[2 * i + 1];
        const float max_value = range.max_values[2 * i + 1];
        transfunc_array[i].m_server_variable_min = min_value;
        transfunc_array[i].m_server_variable_max = max_value;

        if ( transfunc_array[i].m_server_range_mode == EnsembleTransferFunction::ServerRangeMode::ServerSide )
        {
            transfunc_array[i].setColorRange( min_value, max_value );
            transfunc_array[i].setOpacityRange( min_value, max_value );
        }
    }
}

void CalculateEnsembleStatisticHistogram(
    EnsembleStatisticRange& range,
    const std::vector<float>& values,
    const int tf_number,
    const std::vector<EnsembleTransferFunction>& transfunc_array
)
{
    range.o_bins.assign( tf_number * DEFAULT_NBINS, 0 );
    range.c_bins.assign( tf_number * DEFAULT_NBINS, 0 );

    if ( values.empty() ) return;

    const size_t max_histogram_samples = 100000;
    const size_t stride = std::max<size_t>( 1, ( values.size() + max_histogram_samples - 1 ) / max_histogram_samples );

    for ( size_t i = 0; i < values.size(); i += stride )
    {
        for ( int tf = 0; tf < tf_number; tf++ )
        {
            size_t o_bin = 0;
            size_t c_bin = 0;
            if ( EnsembleHistogramBin(
                values[i],
                transfunc_array[tf].opacityMap().minValue(),
                transfunc_array[tf].opacityMap().maxValue(),
                o_bin ) )
            {
                range.o_bins[o_bin + tf * DEFAULT_NBINS]++;
            }
            if ( EnsembleHistogramBin(
                values[i],
                transfunc_array[tf].colorMap().minValue(),
                transfunc_array[tf].colorMap().maxValue(),
                c_bin ) )
            {
                range.c_bins[c_bin + tf * DEFAULT_NBINS]++;
            }
        }
    }
}

struct ChainRuleTimingBreakdown
{
    double calc_scalar_grad;
    double q_grad_setup;
    double tf_scalar_eval;
    double chain_rule_dfdq;
    double normal_normalize;

    ChainRuleTimingBreakdown():
        calc_scalar_grad( 0.0 ),
        q_grad_setup( 0.0 ),
        tf_scalar_eval( 0.0 ),
        chain_rule_dfdq( 0.0 ),
        normal_normalize( 0.0 )
    {
    }
};

// SIMD (across-particle) block chain rule: replaces the per-particle tf_scalar_eval
// + chain_rule_dfdq loop. Evaluates F and its finite-difference gradient for the
// whole particle block via ReversePolishNotation::evalArraySIMD. Bit-identical to
// the scalar path (same math / order); just vectorized across particles and with
// far fewer eval() calls (per-block instead of per-particle).
/*===========================================================================*/
/**
 *  @brief  法線(∇F)の計算方式。
 *
 *  ChainRule            : 変数ごとに ∂F/∂q を求めて合算する（現行）。
 *                         数式が微分量(dq)だけで構成されると活性変数が空になり
 *                         ∇F=0 になる制約がある。
 *  CoordinateDifference : 局所座標で ±ε ずらして F を直接差分する（方式A、段2）。
 *  NodeFField           : 節点で F を評価して場を作り、その勾配を取る（方式C、段4）。
 */
/*===========================================================================*/
enum class NormalMethod
{
    ChainRule,
    CoordinateDifference,
    NodeFField
};

/*===========================================================================*/
/**
 *  @brief  変数値の並び(varr)を組み立てて、数式 F の値をブロック一括で求める。
 *
 *  chainRuleBlock() の前半を切り出したもの。方式A(座標差分法)は局所座標を
 *  ずらしながらこの関数を繰り返し呼ぶ。
 *
 *  @param  xa,ya,za [in,out] global座標のスクラッチ。**呼び出し側で確保すること**。
 *      varr[X..Z] としてポインタが ctx.rpn に登録され、
 *      チェーンルールの再評価(evalArraySIMD)でも読み続けられるため、
 *      この関数のローカル変数にすると関数を抜けた時点で宙に浮く。
 */
/*===========================================================================*/
inline void eval_F_block(
    ChainRuleEvalContext& ctx,
    const int n,
    const int nvariables,
    float (*scalar_array)[SIMD_BLK_SIZE],
    float (*grad_qx)[SIMD_BLK_SIZE],
    float (*grad_qy)[SIMD_BLK_SIZE],
    float (*grad_qz)[SIMD_BLK_SIZE],
    const vismodule::Vector3f* coord,
    float* xa,
    float* ya,
    float* za,
    float* F_out,
    ChainRuleTimingBreakdown* timing )
{
    for ( int p = 0; p < n; ++p ) { xa[p] = coord[p].x(); ya[p] = coord[p].y(); za[p] = coord[p].z(); }

    float* varr[NUMVAR];
    for ( int i = 0; i < NUMVAR; ++i ) varr[i] = 0;
    varr[X] = xa; varr[Y] = ya; varr[Z] = za;
    for ( int v = 0; v < nvariables; ++v )
    {
        varr[Q1 + 4 * v]     = scalar_array[v];
        varr[Q1 + 4 * v + 1] = grad_qx[v];
        varr[Q1 + 4 * v + 2] = grad_qy[v];
        varr[Q1 + 4 * v + 3] = grad_qz[v];
    }
    ctx.rpn.setVariableValueArray( varr );

#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer t_eval; t_eval.start();
#endif
    ctx.rpn.evalArraySIMD( F_out, n );
#ifdef ENABLE_ENSEMBLE_TIMER
    t_eval.stop(); if ( timing ) timing->tf_scalar_eval += t_eval.sec();
#endif
}

inline void chainRuleBlock(
    ChainRuleEvalContext& ctx,
    const int n,
    const int nvariables,
    float (*scalar_array)[SIMD_BLK_SIZE],
    float (*grad_qx)[SIMD_BLK_SIZE],
    float (*grad_qy)[SIMD_BLK_SIZE],
    float (*grad_qz)[SIMD_BLK_SIZE],
    const vismodule::Vector3f* coord,
    float* scalar_out,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    const float FD = 1.0e-5f;   // == ChainRuleNormal FiniteDifferenceScale
    // xa/ya/za は varr 経由で ctx.rpn に登録され、以降のチェーンルール再評価でも
    // 参照される。したがってこの関数のスコープで保持し、eval_F_block へ渡す。
    alignas(64) float xa[SIMD_BLK_SIZE], ya[SIMD_BLK_SIZE], za[SIMD_BLK_SIZE];
    eval_F_block( ctx, n, nvariables, scalar_array, grad_qx, grad_qy, grad_qz,
                  coord, xa, ya, za, scalar_out, timing );   // F (unperturbed)

    for ( int p = 0; p < n; ++p ) { grad_array_x[p] = 0.0f; grad_array_y[p] = 0.0f; grad_array_z[p] = 0.0f; }

#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer t_cr; t_cr.start();
#endif
    alignas(64) float Fp[SIMD_BLK_SIZE], Fm[SIMD_BLK_SIZE], qs[SIMD_BLK_SIZE], eps[SIMD_BLK_SIZE];
    const std::vector<std::size_t>& act = ctx.workspace.activeVariables();
    for ( std::size_t k = 0; k < act.size(); ++k )
    {
        const int i = static_cast<int>( act[k] );
        float* qi = scalar_array[i];                 // varr[Q1+4i] aliases this
        for ( int p = 0; p < n; ++p ) { qs[p] = qi[p]; eps[p] = FD * std::max( 1.0f, std::fabs( qi[p] ) ); }
        for ( int p = 0; p < n; ++p ) qi[p] = qs[p] + eps[p];
        ctx.rpn.evalArraySIMD( Fp, n );
        for ( int p = 0; p < n; ++p ) qi[p] = qs[p] - eps[p];
        ctx.rpn.evalArraySIMD( Fm, n );
        for ( int p = 0; p < n; ++p ) qi[p] = qs[p];
        for ( int p = 0; p < n; ++p )
        {
            const float dF = ( Fp[p] - Fm[p] ) / ( 2.0f * eps[p] );
            if ( std::isfinite( dF ) )
            {
                grad_array_x[p] += grad_qx[i][p] * dF;
                grad_array_y[p] += grad_qy[i][p] * dF;
                grad_array_z[p] += grad_qz[i][p] * dF;
            }
        }
    }
    for ( int p = 0; p < n; ++p )
    {
        if ( !( std::isfinite( grad_array_x[p] ) && std::isfinite( grad_array_y[p] ) && std::isfinite( grad_array_z[p] ) ) )
        { grad_array_x[p] = 0.0f; grad_array_y[p] = 0.0f; grad_array_z[p] = 0.0f; }
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    t_cr.stop(); if ( timing ) timing->chain_rule_dfdq += t_cr.sec();
#endif
}

/*===========================================================================*/
/**
 *  @brief  指定した局所座標で、各変数の値 q と勾配 ∇q をブロック一括で求める。
 *
 *  calculate_scalar_and_chain_rule_grad() の前半を切り出したもの。
 *  方式A(座標差分法)は局所座標をずらしながらこの関数を繰り返し呼ぶ。
 */
/*===========================================================================*/
static void gather_variable_values(
    const int nparticles_count,
    const int nvariables,
    const std::vector< vismodule::CellBase<Type>* >& interp,
    const vismodule::Vector3f* local_coord_array,
    float (*scalar_array)[SIMD_BLK_SIZE],
    float (*grad_qx)[SIMD_BLK_SIZE],
    float (*grad_qy)[SIMD_BLK_SIZE],
    float (*grad_qz)[SIMD_BLK_SIZE],
    ChainRuleTimingBreakdown* timing )
{
#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer calc_scalar_grad_timer;
    calc_scalar_grad_timer.start();
#endif
    if ( nvariables > 1 && interp[0]->supportsJacobianReuse() )
    {
        // Candidate A: the inverse Jacobian is geometry-only (vertices + shape-fn
        // derivatives) and identical across variables sharing coordinates/connections.
        // Compute it once on interp[0], reuse it for every variable's gradient.
        double cof[9][SIMD_BLK_SIZE];
        double det_inverse[SIMD_BLK_SIZE];
        double scale_factor[SIMD_BLK_SIZE];
        double determinant[SIMD_BLK_SIZE];
        interp[0]->setLocalPointArray( nparticles_count, local_coord_array );
        interp[0]->computeScaledInvJacobianArray(
            nparticles_count, cof, det_inverse, scale_factor, determinant );
        for ( int j = 0; j < nvariables; ++j )
        {
            if ( j != 0 ) interp[j]->setLocalPointArray( nparticles_count, local_coord_array );
            interp[j]->scalar_ary( scalar_array[j], nparticles_count );
            interp[j]->gradFromScaledInvJacobianArray(
                nparticles_count, cof, det_inverse, scale_factor, determinant,
                grad_qx[j], grad_qy[j], grad_qz[j] );
        }
    }
    else
    {
        for ( int j = 0; j < nvariables; ++j )
        {
            interp[j]->setLocalPointArray( nparticles_count, local_coord_array );
            interp[j]->CalcScalarGrad(
                nparticles_count,
                scalar_array[j],
                grad_qx[j],
                grad_qy[j],
                grad_qz[j] );
        }
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    calc_scalar_grad_timer.stop();
    if ( timing ) timing->calc_scalar_grad += calc_scalar_grad_timer.sec();
#endif
}

void calculate_scalar_and_chain_rule_grad(
    const int nparticles_count,
    const int nvariables,
    ChainRuleEvalContext& chain_context,
    const std::vector< vismodule::CellBase<Type>* >& interp,
    const vismodule::Vector3f* local_coord_array,
    const vismodule::Vector3f* global_coord_array,
    const vismodule::UInt32* cell_index,
    float* scalar_result,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    float scalar_array[nvariables][SIMD_BLK_SIZE];
    float grad_qx[nvariables][SIMD_BLK_SIZE];
    float grad_qy[nvariables][SIMD_BLK_SIZE];
    float grad_qz[nvariables][SIMD_BLK_SIZE];

    gather_variable_values( nparticles_count, nvariables, interp, local_coord_array,
                            scalar_array, grad_qx, grad_qy, grad_qz, timing );

    if ( !chain_context.valid )
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer normal_timer;
        normal_timer.start();
#endif
        for ( int p = 0; p < nparticles_count; ++p )
        {
            scalar_result[p] = 0.0f;
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        normal_timer.stop();
        if ( timing ) timing->normal_normalize += normal_timer.sec();
#endif
        return;
    }

#ifdef PBVR_SIMD_CHAINRULE
    chainRuleBlock( chain_context, nparticles_count, nvariables,
                    scalar_array, grad_qx, grad_qy, grad_qz,
                    global_coord_array,
                    scalar_result, grad_array_x, grad_array_y, grad_array_z,
                    timing );
    return;
#endif

    float q_values[128];
    vismodule::Vector3f grad_q[128];

    for ( int p = 0; p < nparticles_count; ++p )
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer q_grad_setup_timer;
        q_grad_setup_timer.start();
#endif
        chain_context.variable_values[X] = global_coord_array[p].x();
        chain_context.variable_values[Y] = global_coord_array[p].y();
        chain_context.variable_values[Z] = global_coord_array[p].z();

        for ( int v = 0; v < nvariables; ++v )
        {
            const std::size_t q = static_cast<std::size_t>( 4 * ( v + 1 ) );
            q_values[v] = scalar_array[v][p];
            chain_context.variable_values[q] = scalar_array[v][p];
            chain_context.variable_values[q + 1] = grad_qx[v][p];
            chain_context.variable_values[q + 2] = grad_qy[v][p];
            chain_context.variable_values[q + 3] = grad_qz[v][p];
            grad_q[v] = vismodule::Vector3f(
                grad_qx[v][p],
                grad_qy[v][p],
                grad_qz[v][p] );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        q_grad_setup_timer.stop();
        if ( timing ) timing->q_grad_setup += q_grad_setup_timer.sec();
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer tf_scalar_eval_timer;
        tf_scalar_eval_timer.start();
#endif
        scalar_result[p] = chain_context.rpn.eval();
#ifdef ENABLE_ENSEMBLE_TIMER
        tf_scalar_eval_timer.stop();
        if ( timing ) timing->tf_scalar_eval += tf_scalar_eval_timer.sec();
#endif

        vismodule::Vector3f grad_F;
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer chain_rule_timer;
        chain_rule_timer.start();
#endif
        const bool ok = chain_context.workspace.computeGradient(
            q_values,
            grad_q,
            nvariables,
            &grad_F );
#ifdef ENABLE_ENSEMBLE_TIMER
        chain_rule_timer.stop();
        if ( timing ) timing->chain_rule_dfdq += chain_rule_timer.sec();
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer normal_timer;
        normal_timer.start();
#endif
        if ( !ok )
        {
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
#ifdef ENABLE_ENSEMBLE_TIMER
            normal_timer.stop();
            if ( timing ) timing->normal_normalize += normal_timer.sec();
#endif
            continue;
        }

        grad_array_x[p] = grad_F.x();
        grad_array_y[p] = grad_F.y();
        grad_array_z[p] = grad_F.z();
#ifdef ENABLE_ENSEMBLE_TIMER
        normal_timer.stop();
        if ( timing ) timing->normal_normalize += normal_timer.sec();
#endif
    }
}

// 構造格子版の値・勾配取得 + chain rule 法線。
/*===========================================================================*/
/**
 *  @brief  方式A（座標差分法）: 局所座標で ±ε ずらして数式 F を直接差分する。
 *
 *  チェーンルールを使わないので、数式が微分量(dq)だけで構成されていても動作する。
 *  現行方式が破綻する「活性変数が空になる」段階がそもそも存在しない。
 *
 *  ε はセルの外へ出てもよい。形状関数は局所座標の多項式なので、はみ出しても
 *  同じ多項式を外挿するだけであり、「そのセルの補間関数を微分する」という目的に
 *  対しては正しい値が得られる。したがって隣接セルの探索は不要である。
 *
 *  差分で得られるのは局所座標での勾配なので、通常PBVR の方式A
 *  （CellByCellUniformSampling.cpp の grad_array 算出部）と同じく J^-1 を掛けて
 *  物理座標系へ変換する。実体は CellBase::applyInvJacobianToGradArray() で、
 *  チェーンルール経路の grad_ary() と同じ式・同じ数値の扱いを使う。
 *
 *  この変換は省けない。節点の並びが形状関数の規約と食い違うと局所軸の向きが
 *  反転し、変換なしでは法線の該当成分が逆を向くからである。局所座標での節点の
 *  並びは形状関数の規約で決まる一方、入力メッシュがどの順で節点を並べるかは
 *  シミュレーション側の都合で決まるため、両者が一致する保証がない。実例として
 *  HexahedralCell は節点0〜3を zeta=1(上面) と定義しているが、
 *  ens_Hydrogen_unstruct と CityLBM はどちらも節点0〜3に下面を割り当てており、
 *  この並びでは J = diag(h,h,-h) となる。J^-1 を通せばこの符号は自動的に処理され、
 *  さらに異方セル(dx != dy != dz)や三角柱のような非直交セルにも対応できる。
 *
 *  制約: 一次四面体では微分量がセル内で定数のため、微分量だけの数式に対して
 *  差分が厳密にゼロになる。セル種による分岐は行わない（通常PBVR と同じ扱い）。
 */
/*===========================================================================*/
void calculate_scalar_and_grad_coorddiff(
    const int nparticles_count,
    const int nvariables,
    ChainRuleEvalContext& chain_context,
    const std::vector< vismodule::CellBase<Type>* >& interp,
    const vismodule::Vector3f* local_coord_array,
    const vismodule::Vector3f* global_coord_array,
    float* scalar_result,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    float scalar_array[nvariables][SIMD_BLK_SIZE];
    float grad_qx[nvariables][SIMD_BLK_SIZE];
    float grad_qy[nvariables][SIMD_BLK_SIZE];
    float grad_qz[nvariables][SIMD_BLK_SIZE];
    // xa/ya/za は eval_F_block が varr[X..Z] として ctx.rpn に登録する。
    // この関数のスコープで保持する(理由は eval_F_block のコメント参照)。
    alignas(64) float xa[SIMD_BLK_SIZE], ya[SIMD_BLK_SIZE], za[SIMD_BLK_SIZE];

    // --- 中心点: 描画に使う F の値を求める ---
    gather_variable_values( nparticles_count, nvariables, interp, local_coord_array,
                            scalar_array, grad_qx, grad_qy, grad_qz, timing );

    if ( !chain_context.valid )
    {
        for ( int p = 0; p < nparticles_count; ++p )
        {
            scalar_result[p] = 0.0f;
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
        }
        return;
    }

    eval_F_block( chain_context, nparticles_count, nvariables,
                  scalar_array, grad_qx, grad_qy, grad_qz,
                  global_coord_array, xa, ya, za, scalar_result, timing );

    // --- 3方向 x +- の6点で中央差分 ---
    const float EPS = 0.1f;                      // 局所座標。通常PBVR と同一
    const float INV = 1.0f / ( 2.0f * EPS );
    vismodule::Vector3f off[SIMD_BLK_SIZE];
    vismodule::Vector3f goff[SIMD_BLK_SIZE];
    alignas(64) float Fp[SIMD_BLK_SIZE], Fm[SIMD_BLK_SIZE];
    float* out[3] = { grad_array_x, grad_array_y, grad_array_z };

    for ( int k = 0; k < 3; ++k )
    {
        for ( int s = 0; s < 2; ++s )
        {
            const float d = ( s == 0 ) ? EPS : -EPS;
            for ( int p = 0; p < nparticles_count; ++p )
            {
                off[p] = local_coord_array[p];
                off[p][k] += d;
            }
            gather_variable_values( nparticles_count, nvariables, interp, off,
                                    scalar_array, grad_qx, grad_qy, grad_qz, timing );
            // 差分点の global 座標。数式が X,Y,Z を参照する場合に備えて毎回求める。
            // transformLocalToGlobalArray は setLocalPointArray が設定した内挿関数を
            // 使うため、gather_variable_values の後に呼ぶ必要がある。
            interp[0]->transformLocalToGlobalArray( nparticles_count, off, goff );
            eval_F_block( chain_context, nparticles_count, nvariables,
                          scalar_array, grad_qx, grad_qy, grad_qz,
                          goff, xa, ya, za, ( s == 0 ) ? Fp : Fm, timing );
        }
        for ( int p = 0; p < nparticles_count; ++p )
        {
            out[k][p] = ( Fp[p] - Fm[p] ) * INV;
        }
    }

    // 局所座標の勾配を物理座標系へ。J は中心点のものを使うので、差分点で
    // 上書きされた形状関数の微分を中心点に戻してから変換する。
    interp[0]->setLocalPointArray( nparticles_count, local_coord_array );
    interp[0]->applyInvJacobianToGradArray(
        nparticles_count, grad_array_x, grad_array_y, grad_array_z );

    for ( int p = 0; p < nparticles_count; ++p )
    {
        if ( !( std::isfinite( grad_array_x[p] ) &&
                std::isfinite( grad_array_y[p] ) &&
                std::isfinite( grad_array_z[p] ) ) )
        {
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  利用者が要求した法線計算方式。auto は実行時に選ぶ。
 */
/*===========================================================================*/
enum class NormalMethodRequest
{
    Auto,
    ChainRule,
    CoordinateDifference,
    NodeFField
};

/*===========================================================================*/
/**
 *  @brief  環境変数 PBVR_NORMAL_METHOD から要求を読む。既定は現行方式。
 *      chainrule (既定) / coorddiff / nodefield / auto
 */
/*===========================================================================*/
inline NormalMethodRequest resolve_normal_method_request()
{
    const char* e = std::getenv( "PBVR_NORMAL_METHOD" );
    if ( e == NULL || e[0] == '\0' ) return NormalMethodRequest::ChainRule;
    if ( std::strcmp( e, "chainrule" ) == 0 ) return NormalMethodRequest::ChainRule;
    if ( std::strcmp( e, "coorddiff" ) == 0 ) return NormalMethodRequest::CoordinateDifference;
    if ( std::strcmp( e, "nodefield" ) == 0 ) return NormalMethodRequest::NodeFField;
    if ( std::strcmp( e, "auto" ) == 0 )      return NormalMethodRequest::Auto;
    std::cerr << "PBVR_NORMAL_METHOD: unknown value '" << e
              << "' (expected chainrule|coorddiff|nodefield|auto). Using chainrule."
              << std::endl;
    return NormalMethodRequest::ChainRule;
}

/*===========================================================================*/
/**
 *  @brief  auto で方式A と方式C を分ける、粒子数/セル数の閾値。
 *
 *  方式C の前処理（節点微分量の復元）は「セル数 x 節点数 x 微分量を使う変数の数」に
 *  比例する固定費で、粒子数には依らない。一方 方式A は1粒子につき ±eps の6点で
 *  数式を評価するため粒子数に比例する。したがって粒子数が多いほど方式C が有利になり、
 *  逆転点は節点数と変数の数に比例して上がる。
 *
 *  校正は実測1点のみ。六面体（節点8）・微分量を使う変数1個・4MPI×2OMP・256^3 格子で、
 *  逆転点が「1ランクあたりの粒子数/セル数」= 約 0.39 だった。その1点を基準に、
 *  固定費が何倍になるか（節点数の比 x 変数の数の比）で割り増しする。
 *
 *  注意: 節点数と変数の数に比例するという部分は実測していない。方式C の前処理が
 *  「セル数 x 節点数 x 変数の数」の勾配評価であることからの推定で、六面体1条件からの
 *  外挿である。四面体（節点4）や二次六面体（節点20）では確かめていない。
 *
 *  比はランクあたりで取ること。ensemble_timer_summary.csv の粒子数は MPI_SUM で
 *  全ランクを合計した値なので、そのまま使うとランク数ぶん過大になる。
 *
 *  条件が合わない場合は環境変数 PBVR_NORMAL_AUTO_THRESHOLD で閾値そのものを
 *  直接指定できる。
 */
/*===========================================================================*/
inline double normal_auto_threshold( const int nnodes, const int ndq_variables )
{
    const char* e = std::getenv( "PBVR_NORMAL_AUTO_THRESHOLD" );
    if ( e != NULL && e[0] != '\0' ) return std::atof( e );

    // 校正点（実測した条件と、そのときの逆転点）
    const double CALIB_RATIO  = 0.39;   // そのときの 粒子数/セル数
    const int    CALIB_NNODES = 8;      // そのときのセルの節点数（六面体）
    const int    CALIB_NDQVAR = 1;      // そのときの微分量を使う変数の数

    const int v = ( ndq_variables > 0 ) ? ndq_variables : 1;
    return CALIB_RATIO
         * ( static_cast<double>( nnodes ) / static_cast<double>( CALIB_NNODES ) )
         * ( static_cast<double>( v )      / static_cast<double>( CALIB_NDQVAR ) );
}

inline const char* celltype_name( const vismodule::VolumeObjectBase::CellType& c )
{
    switch ( c )
    {
    case vismodule::VolumeObjectBase::Tetrahedra:          return "四面体";
    case vismodule::VolumeObjectBase::Hexahedra:           return "六面体";
    case vismodule::VolumeObjectBase::QuadraticTetrahedra: return "二次四面体";
    case vismodule::VolumeObjectBase::QuadraticHexahedra:  return "二次六面体";
    case vismodule::VolumeObjectBase::Prism:               return "三角柱";
    case vismodule::VolumeObjectBase::Pyramid:             return "四角錐";
    default:                                               return "不明";
    }
}

inline const char* normal_method_name( const NormalMethod m )
{
    switch ( m )
    {
    case NormalMethod::CoordinateDifference: return "座標差分法(coorddiff)";
    case NormalMethod::NodeFField:           return "節点F場法(nodefield)";
    default:                                 return "チェーンルール(chainrule)";
    }
}

/*===========================================================================*/
/**
 *  @brief  全セルの体積の総和。auto での粒子数の見積もりに使う。
 *
 *  一様サンプリングの粒子数は CalculateNumberOfParticlesV35( max_density, セル体積,
 *  repetitions, ... ) で決まり、密度は定数なので、期待値は
 *      粒子数 = max_density * 総体積 * repetitions
 *  で前もって求まる。ここは総体積を1回走査して求める（auto のときだけ呼ぶ）。
 */
/*===========================================================================*/
static double total_cell_volume(
    std::vector< std::vector<vismodule::CellBase<Type>*> >& cell, const int ncells )
{
    double total = 0.0;
#pragma omp parallel reduction(+:total)
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
#else
        const int thid = 0;
#endif
        vismodule::UInt32 cid[SIMD_BLK_SIZE];
        vismodule::Real32 vol[SIMD_BLK_SIZE];
#pragma omp for schedule( static )
        for ( int base = 0; base < ncells; base += SIMD_BLK_SIZE )
        {
            const int m = ( ncells - base > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncells - base;
            for ( int p = 0; p < m; ++p ) cid[p] = static_cast<vismodule::UInt32>( base + p );
            cell[thid][0]->bindCellArray( m, cid );
            cell[thid][0]->volumeArray( m, cid, vol );
            for ( int p = 0; p < m; ++p )
            {
                if ( std::isfinite( vol[p] ) && vol[p] > 0.0f ) total += vol[p];
            }
        }
    }
    return total;
}

/*===========================================================================*/
/**
 *  @brief  法線計算方式を決める。
 *
 *  最初に「その方式では法線が出ない」組み合わせを潰す。ここは利用者の要求より
 *  優先する。黙って法線がゼロの出力を出すより、方式を変えて警告する方がよい。
 *
 *    ・数式が微分量(dq)を含む + チェーンルール
 *        合算対象を選ぶ判定が dq を弾くため、法線が全粒子ゼロになる。
 *    ・数式が微分量を含む + 座標差分 + 一次四面体
 *        一次四面体は grad q がセル内で定数なので、セル内で F を差分すると
 *        厳密にゼロになる。節点F場法は節点ごとに違う微分量が入るので影響を受けない。
 *    ・数式が微分量を含む + 座標差分 + 四角錐
 *        PyramidalCell は局所座標を x=2x/(1-z) に潰してから形状関数を評価するため、
 *        格納された局所座標をずらす座標差分とはパラメータ化が食い違い、向きが
 *        約18度ずれる。法線は出るので警告のみとし、方式は変えない。
 *
 *  そのうえで auto なら速度で選ぶ。
 */
/*===========================================================================*/
static NormalMethod select_normal_method(
    const NormalMethodRequest request,
    const bool uses_dq,
    const int ndq_variables,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const int nnodes,
    const double np_over_ncell,
    const bool np_known,
    const int mpi_rank,
    std::string* reason )
{
    const bool is_tetra   = ( celltype == vismodule::VolumeObjectBase::Tetrahedra );
    const bool is_pyramid = ( celltype == vismodule::VolumeObjectBase::Pyramid );
    std::ostringstream why;

    NormalMethod m = NormalMethod::ChainRule;

    if ( request == NormalMethodRequest::Auto )
    {
        if ( !uses_dq )
        {
            m = NormalMethod::ChainRule;
            why << "数式が微分量を含まないため現行方式で足りる";
        }
        else if ( is_tetra )
        {
            m = NormalMethod::NodeFField;
            why << "一次四面体では座標差分の法線が厳密にゼロになるため";
        }
//        else if ( is_pyramid )
//        {
//            m = NormalMethod::NodeFField;
//            why << "四角錐は局所座標の潰しにより座標差分の向きがずれるため";
//        }
        else
        {
            const double th = normal_auto_threshold( nnodes, ndq_variables );
            if ( !np_known )
            {
                m = NormalMethod::CoordinateDifference;
                why << "粒子数を見積もれないため座標差分を選択";
            }
            else if ( np_over_ncell >= th )
            {
                m = NormalMethod::NodeFField;
                why << "粒子数/セル数=" << np_over_ncell << " が閾値 " << th << " 以上";
            }
            else
            {
                m = NormalMethod::CoordinateDifference;
                why << "粒子数/セル数=" << np_over_ncell << " が閾値 " << th << " 未満";
            }
        }
    }
    else
    {
        switch ( request )
        {
        case NormalMethodRequest::CoordinateDifference: m = NormalMethod::CoordinateDifference; break;
        case NormalMethodRequest::NodeFField:           m = NormalMethod::NodeFField;           break;
        default:                                        m = NormalMethod::ChainRule;            break;
        }
        why << "利用者の指定";

        // --- 正しさの門番。要求を上書きする ---
        if ( uses_dq && m == NormalMethod::ChainRule )
        {
            m = ( is_tetra || is_pyramid ) ? NormalMethod::NodeFField
                                           : NormalMethod::CoordinateDifference;
            why.str( "" );
            why << "数式が微分量を含みチェーンルールでは法線が全粒子ゼロになるため切り替え";
            if ( mpi_rank == 0 )
            {
                std::cerr << "[PBVR] 警告: 数式が微分量(dq)を含むため、チェーンルールでは"
                          << "法線が全粒子ゼロになります。" << normal_method_name( m )
                          << " に切り替えます。" << std::endl;
            }
        }
        else if ( uses_dq && m == NormalMethod::CoordinateDifference && is_tetra )
        {
            m = NormalMethod::NodeFField;
            why.str( "" );
            why << "一次四面体では座標差分の法線が厳密にゼロになるため切り替え";
            if ( mpi_rank == 0 )
            {
                std::cerr << "[PBVR] 警告: 一次四面体では座標差分の法線が厳密にゼロになります。"
                          << "節点F場法に切り替えます。" << std::endl;
            }
        }
        else if ( uses_dq && m == NormalMethod::CoordinateDifference && is_pyramid )
        {
            if ( mpi_rank == 0 )
            {
                std::cerr << "[PBVR] 警告: 四角錐は局所座標の潰しにより座標差分の法線の向きが"
                          << "約18度ずれます。節点F場法を推奨します。" << std::endl;
            }
        }

        // --- 速度の助言（方式は変えない） ---
        if ( uses_dq && np_known && !is_tetra && !is_pyramid && mpi_rank == 0 )
        {
            const double th = normal_auto_threshold( nnodes, ndq_variables );
            if ( m == NormalMethod::CoordinateDifference && np_over_ncell >= th )
            {
                std::cerr << "[PBVR] 助言: 粒子数/セル数=" << np_over_ncell
                          << " は閾値 " << th << " 以上です。節点F場法の方が速い見込みです。"
                          << std::endl;
            }
            else if ( m == NormalMethod::NodeFField && np_over_ncell < th )
            {
                std::cerr << "[PBVR] 助言: 粒子数/セル数=" << np_over_ncell
                          << " は閾値 " << th << " 未満です。座標差分の方が速い見込みです。"
                          << std::endl;
            }
        }
    }

    if ( reason != NULL ) *reason = why.str();
    return m;
}

/*===========================================================================*/
/**
 *  @brief  節点微分量を復元するときの grad q の評価点。
 *
 *  NodePosition : 各節点位置で評価する。セルあたり節点数だけ評価が要るが、
 *                 節点での値をそのまま使うので素直。
 *  CellCenter   : セル重心で1回だけ評価し、そのセルの全節点へ同じ値を散布する。
 *                 評価点数が 1/節点数 になる（六面体なら 1/8）。
 *                 grad q がセル内で一次式なら、節点を囲むセルの重心での値を平均すると
 *                 対称性から内部節点では節点位置評価と一致する。境界節点では偏る。
 */
/*===========================================================================*/
enum class NodeDqMode
{
    NodePosition,
    CellCenter
};

/*===========================================================================*/
/**
 *  @brief  環境変数 PBVR_NODE_DQ_MODE から評価点を決める。既定は各節点位置。
 *      node (既定) / center
 */
/*===========================================================================*/
inline NodeDqMode resolve_node_dq_mode()
{
    const char* e = std::getenv( "PBVR_NODE_DQ_MODE" );
    if ( e == NULL || e[0] == '\0' ) return NodeDqMode::NodePosition;
    if ( std::strcmp( e, "node" ) == 0 )   return NodeDqMode::NodePosition;
    if ( std::strcmp( e, "center" ) == 0 ) return NodeDqMode::CellCenter;
    std::cerr << "PBVR_NODE_DQ_MODE: unknown value '" << e
              << "' (expected node|center). Using node." << std::endl;
    return NodeDqMode::NodePosition;
}

/*===========================================================================*/
/**
 *  @brief  数式が参照している微分量(dq)の一覧。
 *
 *  slot は (変数, 成分) ごとの節点微分量の格納位置。参照されていなければ -1。
 *  var_index は復元に必要な変数の番号を昇順に並べたもの。
 *
 *  成分単位で持つのは散布と格納を減らすため。grad_ary() は3成分を同時に作るので
 *  計算そのものは成分単位では省けず、変数単位でしか飛ばせない。
 */
/*===========================================================================*/
struct DqUsage
{
    std::vector<int> slot;        // [v*3+c] -> 格納位置。未参照は -1
    std::vector<int> var_index;   // 復元が要る変数の番号（昇順）
    int nslots;

    DqUsage() : nslots( 0 ) {}
    bool any() const { return nslots > 0; }
};

/*===========================================================================*/
/**
 *  @brief  数式のトークンを走査して、参照されている微分量を集める。
 *
 *  トークンは Q1=4, DQ1X=5, DQ1Y=6, DQ1Z=7, Q2=8, ... と並ぶので、
 *  Q1 からの差を 4 で割れば変数番号、余りが 1/2/3 なら x/y/z 成分になる。
 *  余り 0 は q 本体で、微分量ではない。
 */
/*===========================================================================*/
static DqUsage collect_dq_usage( const ::EquationToken& expr, const int nvariables )
{
    DqUsage u;
    u.slot.assign( static_cast<size_t>( nvariables ) * 3, -1 );

    std::vector<bool> used( static_cast<size_t>( nvariables ) * 3, false );
    for ( int i = 0; i < 128 && expr.exp_token[i] != END; ++i )
    {
        if ( expr.exp_token[i] != VARIABLE ) continue;
        const int name = expr.var_name[i];
        if ( name < Q1 || name > Q23 ) continue;
        const int d = name - Q1;
        const int c = d % 4;
        if ( c == 0 ) continue;                  // q 本体
        const int v = d / 4;
        if ( v < 0 || v >= nvariables ) continue;
        used[ static_cast<size_t>( v ) * 3 + ( c - 1 ) ] = true;
    }

    for ( int v = 0; v < nvariables; ++v )
    {
        bool any_component = false;
        for ( int c = 0; c < 3; ++c )
        {
            if ( !used[ static_cast<size_t>( v ) * 3 + c ] ) continue;
            u.slot[ static_cast<size_t>( v ) * 3 + c ] = u.nslots++;
            any_component = true;
        }
        if ( any_component ) u.var_index.push_back( v );
    }
    return u;
}

/*===========================================================================*/
/**
 *  @brief  セル種に応じた補間器を1つ生成する。
 *
 *  節点F場用の補間器を作るために追加した。既存のセル生成 switch は変更していない
 *  （そちらは変数ごとに nvariables 個作るので、まとめ方が違う）。
 */
/*===========================================================================*/
static vismodule::CellBase<Type>* create_cell_for_celltype(
    const vismodule::VolumeObjectBase::CellType& celltype,
    Type* values, float* coordinates, const int ncoords,
    unsigned int* connections, const int ncells )
{
    switch ( celltype )
    {
    case vismodule::VolumeObjectBase::Tetrahedra:
        return new vismodule::TetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case vismodule::VolumeObjectBase::Hexahedra:
        return new vismodule::HexahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case vismodule::VolumeObjectBase::QuadraticTetrahedra:
        return new vismodule::QuadraticTetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case vismodule::VolumeObjectBase::QuadraticHexahedra:
        return new vismodule::QuadraticHexahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case vismodule::VolumeObjectBase::Prism:
        return new vismodule::PrismaticCell<Type>( values, coordinates, ncoords, connections, ncells );
    case vismodule::VolumeObjectBase::Pyramid:
        return new vismodule::PyramidalCell<Type>( values, coordinates, ncoords, connections, ncells );
    default:
        return NULL;
    }
}

/*===========================================================================*/
/**
 *  @brief  局所座標が全粒子共通のときの gather_variable_values()。
 *
 *  形状関数の再評価を1点ぶんに減らす以外は gather_variable_values() と同じで、
 *  返る値も同一である。節点微分量の復元のように、ブロック内の全セルを同じ局所座標で
 *  評価する場面で使う。既存の gather_variable_values() は変更していない
 *  （方式A などの経路は局所座標が粒子ごとに異なるため、そちらでは使えない）。
 */
/*===========================================================================*/
static void gather_variable_values_uniform(
    const int nparticles_count,
    const int nvariables,
    const std::vector< vismodule::CellBase<Type>* >& interp,
    const vismodule::Vector3f& local_coord,
    float (*scalar_array)[SIMD_BLK_SIZE],
    float (*grad_qx)[SIMD_BLK_SIZE],
    float (*grad_qy)[SIMD_BLK_SIZE],
    float (*grad_qz)[SIMD_BLK_SIZE] )
{
    if ( nvariables > 1 && interp[0]->supportsJacobianReuse() )
    {
        double cof[9][SIMD_BLK_SIZE];
        double det_inverse[SIMD_BLK_SIZE];
        double scale_factor[SIMD_BLK_SIZE];
        double determinant[SIMD_BLK_SIZE];
        interp[0]->setLocalPointUniformArray( nparticles_count, local_coord );
        interp[0]->computeScaledInvJacobianArray(
            nparticles_count, cof, det_inverse, scale_factor, determinant );
        for ( int j = 0; j < nvariables; ++j )
        {
            if ( j != 0 ) interp[j]->setLocalPointUniformArray( nparticles_count, local_coord );
            interp[j]->scalar_ary( scalar_array[j], nparticles_count );
            interp[j]->gradFromScaledInvJacobianArray(
                nparticles_count, cof, det_inverse, scale_factor, determinant,
                grad_qx[j], grad_qy[j], grad_qz[j] );
        }
    }
    else
    {
        for ( int j = 0; j < nvariables; ++j )
        {
            interp[j]->setLocalPointUniformArray( nparticles_count, local_coord );
            interp[j]->CalcScalarGrad( nparticles_count, scalar_array[j],
                                       grad_qx[j], grad_qy[j], grad_qz[j] );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  節点の局所座標表(CellBase::localNodeCoord)の自己検査。
 *
 *  節点 i の局所座標で局所→物理変換をすると、その節点の座標そのものが出るはずである
 *  （形状関数が N_i(xi_j) = delta_ij を満たすため）。表の写し間違いはここで捕まる。
 *  代表としてセル0で全節点を確認する。許容誤差はセルの代表長に対する相対で見る。
 */
/*===========================================================================*/
static bool verify_local_node_coords(
    vismodule::CellBase<Type>* cell,
    const float* coordinates,
    const unsigned int* connections,
    const int nnodes )
{
    if ( cell == NULL || coordinates == NULL || connections == NULL ) return false;
    if ( nnodes <= 0 || nnodes > SIMD_BLK_SIZE ) return false;

    vismodule::UInt32 cid[SIMD_BLK_SIZE];
    vismodule::Vector3f lc[SIMD_BLK_SIZE];
    vismodule::Vector3f gc[SIMD_BLK_SIZE];
    for ( int i = 0; i < nnodes; ++i )
    {
        cid[i] = 0;
        if ( !cell->localNodeCoord( i, &lc[i] ) )
        {
            std::cerr << "localNodeCoord: セル種が節点局所座標を提供していない (node "
                      << i << ")" << std::endl;
            return false;
        }
    }
    cell->bindCellArray( nnodes, cid );
    cell->setLocalPointArray( nnodes, lc );
    cell->transformLocalToGlobalArray( nnodes, lc, gc );

    // セルの代表長（節点0からの最大距離）。許容誤差の基準にする。
    float extent = 0.0f;
    const unsigned int n0 = connections[0];
    for ( int i = 1; i < nnodes; ++i )
    {
        const unsigned int ni = connections[i];
        const float dx = coordinates[3 * ni]     - coordinates[3 * n0];
        const float dy = coordinates[3 * ni + 1] - coordinates[3 * n0 + 1];
        const float dz = coordinates[3 * ni + 2] - coordinates[3 * n0 + 2];
        const float d = std::sqrt( dx * dx + dy * dy + dz * dz );
        if ( d > extent ) extent = d;
    }
    if ( extent <= 0.0f ) extent = 1.0f;
    const float tol = 1.0e-4f * extent;

    bool ok = true;
    for ( int i = 0; i < nnodes; ++i )
    {
        const unsigned int ni = connections[i];
        const float ex = coordinates[3 * ni];
        const float ey = coordinates[3 * ni + 1];
        const float ez = coordinates[3 * ni + 2];
        const float dx = gc[i].x() - ex;
        const float dy = gc[i].y() - ey;
        const float dz = gc[i].z() - ez;
        const float err = std::sqrt( dx * dx + dy * dy + dz * dz );
        if ( !( err <= tol ) )
        {
            ok = false;
            std::cerr << "localNodeCoord 自己検査 NG: node " << i
                      << " local(" << lc[i].x() << "," << lc[i].y() << "," << lc[i].z() << ")"
                      << " -> global(" << gc[i].x() << "," << gc[i].y() << "," << gc[i].z() << ")"
                      << " expected(" << ex << "," << ey << "," << ez << ")"
                      << " err=" << err << " tol=" << tol << std::endl;
        }
    }
    return ok;
}

/*===========================================================================*/
/**
 *  @brief  節点の微分量 grad q を復元する（方式C の前段、毎ステップ1回）。
 *
 *  各セルで grad q を評価し、節点に散布して平均する。評価点は mode で選ぶ
 *  （各節点位置 / セル重心。NodeDqMode の説明を参照）。
 *  節点は複数のセルに共有され、補間関数がセルごとに独立なので、そこでの微分量は
 *  セルごとに違う値になる（本来「多価」）。この平均で一意な代表値を決める。
 *
 *  混ぜ方は単純平均にしてある。grad_ary() が J^-1 を適用した時点で勾配は
 *  物理座標系の量になっており、セルの大小には依存しないので、体積は勾配そのものには
 *  効かない。体積重みにする選択肢もあるが（節点まわりの体積平均という解釈になる）、
 *  一様メッシュでは単純平均と一致し、非一様でも単純平均は同様に標準的な選び方である。
 *
 *  節点の局所座標は CellBase::localNodeCoord() から取る。使う前に
 *  verify_local_node_coords() で表を検査する。
 *
 *  復元するのは数式が実際に参照している (変数, 成分) だけである。参照の無い変数は
 *  評価そのものを飛ばし、参照の無い成分は散布と格納を飛ばす。grad_ary() は3成分を
 *  同時に作るので、計算は変数単位でしか省けない（未使用成分は副産物として出るが捨てる）。
 *
 *  コストはセル数 x 節点数 x 変数の数の勾配評価。粒子数には依らないので、
 *  粒子数がセル数より十分多い場合に方式C が有利になる。
 *
 *  節点への加算は複数のスレッドが同じ節点を叩くため排他制御が要る。実測ではこれが
 *  復元処理の 6 割を占めたので、スレッドごとに配列を持って加算し、最後に合算する
 *  形にしてある。メモリはスレッド数に比例するので、上限を超える場合だけ従来どおり
 *  排他制御に落とす（上限は環境変数 PBVR_NODE_DQ_TLS_MB、既定 4096 MB）。
 *
 *  制約: MPI 領域境界の節点は自ランクのセルからの寄与しか集まらないため、
 *  そこだけ片側平均になる。境界の合算は未実装（法線＝陰影付け用途のため許容する）。
 *
 *  @param  node_dq [out] 添字は (v*3+c)*ncoords + node。c は 0=x,1=y,2=z。
 */
/*===========================================================================*/
// 定義は後方にあるので前方宣言する。
static inline void bind_variables_scalars_opt(
    std::vector< vismodule::CellBase<Type>* >& cells,
    const int nvariables, const int n, const vismodule::UInt32* cell_index );

static bool build_node_dq_field(
    std::vector< std::vector<vismodule::CellBase<Type>*> >& cell,
    const int nvariables,
    const DqUsage& usage,
    const NodeDqMode mode,
    const float* coordinates, const int ncoords,
    const unsigned int* connections, const int ncells,
    std::vector<float>& node_dq )
{
    if ( cell.empty() || cell[0].empty() || nvariables <= 0 ) return false;
    if ( !usage.any() ) return false;
    if ( coordinates == NULL || connections == NULL || ncoords <= 0 || ncells <= 0 ) return false;

    const int nnodes = static_cast<int>( cell[0][0]->numberOfNodes() );
    if ( !verify_local_node_coords( cell[0][0], coordinates, connections, nnodes ) ) return false;

    vismodule::Vector3f node_local[SIMD_BLK_SIZE];
    for ( int k = 0; k < nnodes; ++k )
    {
        if ( !cell[0][0]->localNodeCoord( k, &node_local[k] ) ) return false;
    }

    // 重心は節点局所座標の平均として求める。6セル種すべてでセルクラスの
    // setLocalGravityPoint() と同じ点になり、しかも定数を書き写さずに済む
    // （PrismaticCell の同関数は Vector3f( 1 / 3, 1 / 3, 0.5 ) と整数除算になっており
    //   重心が (0, 0, 0.5) にずれている。平均で求めればこれを踏まない）。
    vismodule::Vector3f center_local( 0.0f, 0.0f, 0.0f );
    for ( int k = 0; k < nnodes; ++k ) center_local += node_local[k];
    center_local /= static_cast<float>( nnodes );

    const int nused = static_cast<int>( usage.var_index.size() );
    node_dq.assign( static_cast<size_t>( usage.nslots ) * ncoords, 0.0f );
    std::vector<float> weight( static_cast<size_t>( ncoords ), 0.0f );

#if _OPENMP
    const int nthreads = omp_get_max_threads();
#else
    const int nthreads = 1;
#endif
    // スレッド別に配列を持てば排他制御が要らない。メモリはスレッド数に比例するので、
    // 上限を超える場合は従来どおり共有配列＋排他制御に落とす。
    const size_t tls_slab = static_cast<size_t>( usage.nslots ) * ncoords;
    const size_t tls_bytes =
        static_cast<size_t>( nthreads ) * ( tls_slab + ncoords ) * sizeof( float );
    size_t tls_limit = static_cast<size_t>( 4096 ) * 1024 * 1024;
    {
        const char* e = std::getenv( "PBVR_NODE_DQ_TLS_MB" );
        if ( e != NULL && e[0] != '\0' )
        {
            const double mb = std::atof( e );
            tls_limit = ( mb > 0.0 ) ? static_cast<size_t>( mb * 1024.0 * 1024.0 ) : 0;
        }
    }
    const bool use_tls = ( nthreads > 1 ) && ( tls_bytes <= tls_limit );
    std::vector<float> tls_dq, tls_w;
    if ( use_tls )
    {
        tls_dq.assign( static_cast<size_t>( nthreads ) * tls_slab, 0.0f );
        tls_w.assign( static_cast<size_t>( nthreads ) * ncoords, 0.0f );
    }
    else if ( nthreads > 1 )
    {
        std::cerr << "節点微分量の復元: スレッド別配列に "
                  << ( tls_bytes / ( 1024 * 1024 ) ) << " MB 必要で上限を超えるため、"
                  << "排他制御による加算に切り替える。" << std::endl;
    }

#pragma omp parallel
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
#else
        const int thid = 0;
#endif
        // 参照されている変数の補間器だけを集める。既存の bind/gather はそのまま使える。
        std::vector<vismodule::CellBase<Type>*> uc( nused );
        for ( int j = 0; j < nused; ++j ) uc[j] = cell[thid][ usage.var_index[j] ];

        float sa[nused][SIMD_BLK_SIZE];
        float gx[nused][SIMD_BLK_SIZE];
        float gy[nused][SIMD_BLK_SIZE];
        float gz[nused][SIMD_BLK_SIZE];
        vismodule::UInt32 cid[SIMD_BLK_SIZE];

        // 自スレッドの加算先。NULL なら共有配列＋排他制御。
        float* const acc_dq = use_tls ? &tls_dq[ static_cast<size_t>( thid ) * tls_slab ] : NULL;
        float* const acc_w  = use_tls ? &tls_w [ static_cast<size_t>( thid ) * ncoords ]  : NULL;

#pragma omp for schedule( static )
        for ( int base = 0; base < ncells; base += SIMD_BLK_SIZE )
        {
            const int m = ( ncells - base > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncells - base;
            for ( int p = 0; p < m; ++p ) cid[p] = static_cast<vismodule::UInt32>( base + p );

            bind_variables_scalars_opt( uc, nused, m, cid );

            // 重心評価なら 1回だけ評価して全節点へ散布する。節点評価なら節点ごとに評価する。
            const int neval = ( mode == NodeDqMode::CellCenter ) ? 1 : nnodes;
            for ( int e = 0; e < neval; ++e )
            {
                const vismodule::Vector3f& ec =
                    ( mode == NodeDqMode::CellCenter ) ? center_local : node_local[e];
                // ブロック内の全セルを同じ局所座標で評価するので、形状関数は1回で足りる。
                gather_variable_values_uniform( m, nused, uc, ec, sa, gx, gy, gz );

                const int k0 = ( mode == NodeDqMode::CellCenter ) ? 0      : e;
                const int k1 = ( mode == NodeDqMode::CellCenter ) ? nnodes : e + 1;
                for ( int k = k0; k < k1; ++k )
                {
                    if ( acc_dq != NULL )
                    {
                        // スレッド別配列。他スレッドと衝突しないので排他制御は要らない。
                        for ( int p = 0; p < m; ++p )
                        {
                            const size_t node =
                                connections[ static_cast<size_t>( nnodes ) * ( base + p ) + k ];
                            acc_w[node] += 1.0f;   // 寄与したセルの数を数える
                            for ( int j = 0; j < nused; ++j )
                            {
                                const int v = usage.var_index[j];
                                const float* const a[3] = { gx[j], gy[j], gz[j] };
                                for ( int c = 0; c < 3; ++c )
                                {
                                    const int s = usage.slot[ static_cast<size_t>( v ) * 3 + c ];
                                    if ( s < 0 ) continue;
                                    const float av = std::isfinite( a[c][p] ) ? a[c][p] : 0.0f;
                                    acc_dq[ static_cast<size_t>( s ) * ncoords + node ] += av;
                                }
                            }
                        }
                    }
                    else
                    {
                        for ( int p = 0; p < m; ++p )
                        {
                            const size_t node =
                                connections[ static_cast<size_t>( nnodes ) * ( base + p ) + k ];
#pragma omp atomic
                            weight[node] += 1.0f;
                            for ( int j = 0; j < nused; ++j )
                            {
                                const int v = usage.var_index[j];
                                const float* const a[3] = { gx[j], gy[j], gz[j] };
                                for ( int c = 0; c < 3; ++c )
                                {
                                    const int s = usage.slot[ static_cast<size_t>( v ) * 3 + c ];
                                    if ( s < 0 ) continue;
                                    const float av = std::isfinite( a[c][p] ) ? a[c][p] : 0.0f;
#pragma omp atomic
                                    node_dq[ static_cast<size_t>( s ) * ncoords + node ] += av;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ( use_tls )
    {
        // スレッド別の加算結果を合算する。スレッドを外側にすると連続アクセスになる。
        for ( int t = 0; t < nthreads; ++t )
        {
            const float* const sdq = &tls_dq[ static_cast<size_t>( t ) * tls_slab ];
            const float* const sw  = &tls_w [ static_cast<size_t>( t ) * ncoords ];
#pragma omp parallel for schedule( static )
            for ( long i = 0; i < static_cast<long>( tls_slab ); ++i ) node_dq[i] += sdq[i];
#pragma omp parallel for schedule( static )
            for ( int n = 0; n < ncoords; ++n ) weight[n] += sw[n];
        }
        std::vector<float>().swap( tls_dq );
        std::vector<float>().swap( tls_w );
    }

    // 寄与したセルの数で割って平均にする。どのセルからも寄与が無かった節点は 0 のまま。
#pragma omp parallel for schedule( static )
    for ( int n = 0; n < ncoords; ++n )
    {
        const float w = weight[n];
        if ( w <= 0.0f ) continue;
        const float inv = 1.0f / w;
        for ( int s = 0; s < usage.nslots; ++s )
        {
            node_dq[ static_cast<size_t>( s ) * ncoords + n ] *= inv;
        }
    }
    return true;
}

/*===========================================================================*/
/**
 *  @brief  節点で数式 F を評価して節点F場を作る（方式C の前段、毎ステップ1回）。
 *
 *  節点の q はデータそのものなので補間は要らない。X,Y,Z は節点座標を渡す。
 *  微分量(dq)は node_dq から取る。NULL を渡すと 0 として扱うので、
 *  dq を含まない数式では復元を省略できる（その方が安い）。
 *
 *  評価は粒子側と同じ eval_F_block() を使うので、同じ q に対して同じ F が出る。
 *  節点は互いに独立なのでスレッドで分割できる。
 *
 *  @param  node_F [out] 節点F場（長さ ncoords）。補間器がこの配列を参照し続けるので、
 *      呼び出し側は補間器より長く生存させること。
 */
/*===========================================================================*/
static bool build_node_f_field(
    const ::EquationToken& equation_token,
    Type** values, const int nvariables,
    const float* coordinates, const int ncoords,
    const std::vector<float>* node_dq, const DqUsage& usage,
    std::vector<Type>& node_F )
{
    if ( values == NULL || coordinates == NULL || nvariables <= 0 || ncoords <= 0 ) return false;

    node_F.assign( static_cast<size_t>( ncoords ), static_cast<Type>( 0 ) );
    bool ok = true;

#pragma omp parallel
    {
        ChainRuleEvalContext ctx;
        ctx.initialize( equation_token, nvariables );
        if ( !ctx.valid )
        {
#pragma omp critical(node_f_field)
            ok = false;
        }
        else
        {
            float sa[nvariables][SIMD_BLK_SIZE];
            float gx[nvariables][SIMD_BLK_SIZE];
            float gy[nvariables][SIMD_BLK_SIZE];
            float gz[nvariables][SIMD_BLK_SIZE];
            for ( int v = 0; v < nvariables; ++v )
            {
                for ( int p = 0; p < SIMD_BLK_SIZE; ++p )
                {
                    gx[v][p] = 0.0f; gy[v][p] = 0.0f; gz[v][p] = 0.0f;   // node_dq が無い場合
                }
            }
            vismodule::Vector3f coord[SIMD_BLK_SIZE];
            // xa/ya/za は varr[X..Z] として ctx.rpn に登録されるので、
            // eval_F_block を抜けた後も生きている必要がある(eval_F_block のコメント参照)。
            alignas(64) float xa[SIMD_BLK_SIZE], ya[SIMD_BLK_SIZE], za[SIMD_BLK_SIZE];
            alignas(64) float Fb[SIMD_BLK_SIZE];

#pragma omp for schedule( static )
            for ( int base = 0; base < ncoords; base += SIMD_BLK_SIZE )
            {
                const int m = ( ncoords - base > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncoords - base;
                for ( int p = 0; p < m; ++p )
                {
                    const int n = base + p;
                    coord[p] = vismodule::Vector3f(
                        coordinates[3 * n], coordinates[3 * n + 1], coordinates[3 * n + 2] );
                    for ( int v = 0; v < nvariables; ++v )
                    {
                        sa[v][p] = static_cast<float>( values[v][n] );
                    }
                    if ( node_dq != NULL )
                    {
                        for ( int v = 0; v < nvariables; ++v )
                        {
                            float* const a[3] = { &gx[v][p], &gy[v][p], &gz[v][p] };
                            for ( int c = 0; c < 3; ++c )
                            {
                                const int s = usage.slot[ static_cast<size_t>( v ) * 3 + c ];
                                if ( s < 0 ) continue;   // 参照されていない成分は 0 のまま
                                *a[c] = ( *node_dq )[ static_cast<size_t>( s ) * ncoords + n ];
                            }
                        }
                    }
                }
                eval_F_block( ctx, m, nvariables, sa, gx, gy, gz, coord, xa, ya, za, Fb, 0 );
                for ( int p = 0; p < m; ++p )
                {
                    node_F[base + p] = static_cast<Type>( std::isfinite( Fb[p] ) ? Fb[p] : 0.0f );
                }
            }
        }
    }
    return ok;
}

/*===========================================================================*/
/**
 *  @brief  方式C（節点F場法）: 節点F場を1回だけ補間して F と grad F を得る。
 *
 *  数式の評価も変数ごとの補間もチェーンルールも要らない。粒子1個あたりの仕事は
 *  「補間1回 + 勾配1回」で、変数の数にも数式の複雑さにも依らない。
 *  grad_ary() が J^-1 を適用するので、勾配は物理座標系で返る。
 *
 *  ただし節点で一度 F にしてから補間するため、セル内部では F を節点値の
 *  形状関数補間で近似することになる。数式が非線形なほど元の F からずれる。
 */
/*===========================================================================*/
void calculate_scalar_and_grad_nodefield(
    const int nparticles_count,
    vismodule::CellBase<Type>* interp_F,
    const vismodule::Vector3f* local_coord_array,
    const vismodule::UInt32* cell_index,
    float* scalar_result,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z )
{
    interp_F->bindCellArray( nparticles_count, cell_index );
    interp_F->setLocalPointArray( nparticles_count, local_coord_array );
    interp_F->scalar_ary( scalar_result, nparticles_count );
    interp_F->grad_ary( grad_array_x, grad_array_y, grad_array_z, nparticles_count );
}

/*===========================================================================*/
/**
 *  @brief  法線計算の振り分け。方式によらず「F の値」と「∇F」を返す。
 *
 *  @param  method   [in] 計算方式
 *  @param  interp_F [in] 節点F場の補間器。方式C でのみ使用し、他方式では未使用。
 */
/*===========================================================================*/
void calculate_scalar_and_normal(
    const NormalMethod method,
    const int nparticles_count,
    const int nvariables,
    ChainRuleEvalContext& chain_context,
    const std::vector< vismodule::CellBase<Type>* >& interp,
    const vismodule::Vector3f* local_coord_array,
    const vismodule::Vector3f* global_coord_array,
    const vismodule::UInt32* cell_index,
    vismodule::CellBase<Type>* interp_F,
    float* scalar_result,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    switch ( method )
    {
    case NormalMethod::CoordinateDifference:
        ( void )interp_F;
        ( void )cell_index;
        calculate_scalar_and_grad_coorddiff(
            nparticles_count, nvariables, chain_context, interp,
            local_coord_array, global_coord_array,
            scalar_result, grad_array_x, grad_array_y, grad_array_z, timing );
        return;

    case NormalMethod::NodeFField:
        if ( interp_F != NULL )
        {
            ( void )global_coord_array;
            calculate_scalar_and_grad_nodefield(
                nparticles_count, interp_F, local_coord_array, cell_index,
                scalar_result, grad_array_x, grad_array_y, grad_array_z );
            return;
        }
        {
            // 節点F場の構築に失敗した場合はここに来る。現行方式へ退避する。
            static bool warned = false;
            if ( !warned )
            {
                warned = true;
                std::cerr << "PBVR_NORMAL_METHOD=nodefield: node F field is unavailable."
                          << " Falling back to chainrule." << std::endl;
            }
        }
        break;
    case NormalMethod::ChainRule:
    default:
        break;
    }

    ( void )interp_F;
    calculate_scalar_and_chain_rule_grad(
        nparticles_count, nvariables, chain_context, interp,
        local_coord_array, global_coord_array, cell_index,
        scalar_result, grad_array_x, grad_array_y, grad_array_z, timing );
}

// 非構造版 calculate_scalar_and_chain_rule_grad との違いは前半のみ:
//   CellBase(setLocalPointArray+CalcScalarGrad) を TrilinearInterpolator(attachPoint+scalar/gradient)
//   に置換(局所=格子単位座標)。後半(chain rule)は完全に同一ロジック。
void calculate_scalar_and_chain_rule_grad_struct(
    const int nparticles_count,
    const int nvariables,
    ChainRuleEvalContext& chain_context,
    const std::vector< vismodule::TrilinearInterpolator* >& interp,
    const vismodule::Vector3f* local_coord_array,
    const vismodule::Vector3f* global_coord_array,
    float* scalar_result,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    float scalar_array[nvariables][SIMD_BLK_SIZE];
    float grad_qx[nvariables][SIMD_BLK_SIZE];
    float grad_qy[nvariables][SIMD_BLK_SIZE];
    float grad_qz[nvariables][SIMD_BLK_SIZE];

#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer calc_scalar_grad_timer;
    calc_scalar_grad_timer.start();
#endif
    // 格子単位座標(local)を SIMD 配列へ展開し、末尾を最後の有効要素でパディング(通常struct と同じ)
    float px[SIMD_BLK_SIZE];
    float py[SIMD_BLK_SIZE];
    float pz[SIMD_BLK_SIZE];
    const int last = nparticles_count > 0 ? nparticles_count - 1 : 0;
    for ( int p = 0; p < SIMD_BLK_SIZE; ++p )
    {
        const int s = p < nparticles_count ? p : last;
        px[p] = local_coord_array[s].x();
        py[p] = local_coord_array[s].y();
        pz[p] = local_coord_array[s].z();
    }
    // 各変数を三次元線形補間(格子単位座標→値・勾配)
    for ( int j = 0; j < nvariables; ++j )
    {
        interp[j]->attachPoint( px, py, pz );
        interp[j]->scalar( scalar_array[j] );
        interp[j]->gradient( grad_qx[j], grad_qy[j], grad_qz[j] );
        // TrilinearInterpolator::gradient() は -∇q(負の勾配)を返すが、非構造版 CellBase::grad_ary は
        // +∇q(正の勾配)。法線符号を非構造版に揃えるため反転する。分散/変動係数の"値"はスカラー g,g²
        // (R_sq,R_scal)のみに依存し不変で、反転は R_norm/R_tmp 経由の法線の符号のみを是正する。
        for ( int p = 0; p < SIMD_BLK_SIZE; ++p )
        {
            grad_qx[j][p] = -grad_qx[j][p];
            grad_qy[j][p] = -grad_qy[j][p];
            grad_qz[j][p] = -grad_qz[j][p];
        }
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    calc_scalar_grad_timer.stop();
    if ( timing ) timing->calc_scalar_grad += calc_scalar_grad_timer.sec();
#endif

    // --- 以下 chain rule は非構造版 calculate_scalar_and_chain_rule_grad と同一 ---
    if ( !chain_context.valid )
    {
        for ( int p = 0; p < nparticles_count; ++p )
        {
            scalar_result[p] = 0.0f;
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
        }
        return;
    }

#ifdef PBVR_SIMD_CHAINRULE
    chainRuleBlock( chain_context, nparticles_count, nvariables,
                    scalar_array, grad_qx, grad_qy, grad_qz,
                    global_coord_array,
                    scalar_result, grad_array_x, grad_array_y, grad_array_z,
                    timing );
    return;
#endif

    float q_values[128];
    vismodule::Vector3f grad_q[128];
    for ( int p = 0; p < nparticles_count; ++p )
    {
        chain_context.variable_values[X] = global_coord_array[p].x();
        chain_context.variable_values[Y] = global_coord_array[p].y();
        chain_context.variable_values[Z] = global_coord_array[p].z();
        for ( int v = 0; v < nvariables; ++v )
        {
            const std::size_t q = static_cast<std::size_t>( 4 * ( v + 1 ) );
            q_values[v] = scalar_array[v][p];
            chain_context.variable_values[q] = scalar_array[v][p];
            chain_context.variable_values[q + 1] = grad_qx[v][p];
            chain_context.variable_values[q + 2] = grad_qy[v][p];
            chain_context.variable_values[q + 3] = grad_qz[v][p];
            grad_q[v] = vismodule::Vector3f( grad_qx[v][p], grad_qy[v][p], grad_qz[v][p] );
        }
        scalar_result[p] = chain_context.rpn.eval();
        vismodule::Vector3f grad_F;
        const bool ok = chain_context.workspace.computeGradient( q_values, grad_q, nvariables, &grad_F );
        if ( !ok )
        {
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
            continue;
        }
        grad_array_x[p] = grad_F.x();
        grad_array_y[p] = grad_F.y();
        grad_array_z[p] = grad_F.z();
    }
}

// store_uniform_block の構造格子版: cell_id(vi) を持たない
// (struct は格子座標 vc だけで再補間できるため cell 番号は不要)。他は非構造版と同一。
static inline void store_uniform_block_struct(
    const int n, const size_t scalar_offset, const size_t vector_offset,
    std::vector<vismodule::Real32>& vs, std::vector<vismodule::Real32>& vc,
    std::vector<vismodule::Real32>& vn, std::vector<vismodule::Real32>& vsq,
    std::vector<vismodule::Real32>& vt,
    const float* scalar_array, const vismodule::Vector3f* local_coord_array,
    const float* grad_array_x, const float* grad_array_y, const float* grad_array_z )
{
    vismodule::Real32* __restrict os = vs.data()  + scalar_offset;
    vismodule::Real32* __restrict oq = vsq.data() + scalar_offset;
    vismodule::Real32* __restrict oc = vc.data()  + vector_offset;
    vismodule::Real32* __restrict on = vn.data()  + vector_offset;
    vismodule::Real32* __restrict ot = vt.data()  + vector_offset;
    #pragma omp simd
    for ( int k = 0; k < n; k++ )
    {
        os[k] = scalar_array[k];
        oq[k] = scalar_array[k] * scalar_array[k];
        oc[3*k]     = local_coord_array[k].x();
        oc[3*k + 1] = local_coord_array[k].y();
        oc[3*k + 2] = local_coord_array[k].z();
        on[3*k]     = -grad_array_x[k];
        on[3*k + 1] = -grad_array_y[k];
        on[3*k + 2] = -grad_array_z[k];
        ot[3*k]     = scalar_array[k] * grad_array_x[k];
        ot[3*k + 1] = scalar_array[k] * grad_array_y[k];
        ot[3*k + 2] = scalar_array[k] * grad_array_z[k];
    }
}

void calculation_glad(const int nparticles_count, const int nvariables,
        TransferFunctionSynthesizer* th_tfs,
        std::vector<vismodule::TransferFunction>& th_tf,
        const std::vector< vismodule::CellBase<Type>* > interp,
        const vismodule::Vector3f* local_coord_array,
        const vismodule::UInt32* cell_index,
        float* grad_array_x,
        float* grad_array_y,
        float* grad_array_z)
{

    vismodule::Vector3f l_plus_coord[ SIMD_BLK_SIZE ];
    vismodule::Vector3f l_minus_coord[ SIMD_BLK_SIZE ];
    vismodule::Vector3f g_plus_coord[ SIMD_BLK_SIZE ];
    vismodule::Vector3f g_minus_coord[ SIMD_BLK_SIZE ];
    float S_plus_opacity[ SIMD_BLK_SIZE ];
    float S_minus_opacity[ SIMD_BLK_SIZE ];


                    for( int j = 0; j < nvariables; j++ )
                    {
                        interp[j]->bindCellArray( nparticles_count, cell_index );
                    }


                    // dsdx ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0.1,0,0);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(-0.1,0,0);

                    }

                    interp[0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );


                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf,
                                                         S_plus_opacity );
                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf,
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        grad_array_x[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------

                    // dsdy ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0.1,0);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,-0.1,0);
                    }

                    interp[0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );

                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf,
                                                         S_plus_opacity );
                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf,
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        grad_array_y[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------
                    // dsdz ----------------------------------------
                    // dsdz ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0,0.1);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0,-0.1);
                    }

                    interp[0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                            l_plus_coord,
                                                            g_plus_coord );

                    interp[0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );

                    th_tfs->CalculateOpacityArray( interp,
                                                   nparticles_count,
                                                   l_plus_coord,
                                                   g_plus_coord,
                                                   th_tf,
                                                   S_plus_opacity );
                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf,
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        grad_array_z[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
}


// Bind all variables for a cell block. The vertex gather in bindCellArray is identical
// across variables; when the Jacobian-reuse path is active (Hex, nvariables>1) the
// variables k>0 never read their own m_vertices_array (geometry goes through cell[*][0]).
// So bind full scalars+vertices for variable 0 and scalars only for the rest, skipping
// the redundant vertex gather.


static inline void bind_variables_scalars_opt(
    std::vector< vismodule::CellBase<Type>* >& cells,
    const int nvariables, const int n, const vismodule::UInt32* cell_index )
{
    cells[0]->bindCellArray( n, cell_index );
    const bool scalars_only = cells[0]->supportsJacobianReuse();
    for ( int k = 1; k < nvariables; ++k )
    {
        if ( scalars_only ) cells[k]->bindScalarsArray( n, cell_index );
        else                cells[k]->bindCellArray( n, cell_index );
    }
}

// store_uniform_block: append n particles' data to the per-thread output vectors. __restrict
// on the freshly-grown output regions tells the compiler they do not alias the (stack) input
// arrays, clearing the false #15346 dependence that blocked vectorization. Same writes ->
// bit-identical.
static inline void store_uniform_block(
    const int n, const size_t scalar_offset, const size_t vector_offset,
    std::vector<vismodule::Real32>& vs, std::vector<vismodule::Real32>& vc,
    std::vector<vismodule::Real32>& vn, std::vector<int>& vi,
    std::vector<vismodule::Real32>& vsq, std::vector<vismodule::Real32>& vt,
    const float* scalar_array, const vismodule::Vector3f* local_coord_array,
    const float* grad_array_x, const float* grad_array_y, const float* grad_array_z,
    const vismodule::UInt32* cell_index )
{
    vismodule::Real32* __restrict os = vs.data()  + scalar_offset;
    vismodule::Real32* __restrict oq = vsq.data() + scalar_offset;
    int*               __restrict oi = vi.data()  + scalar_offset;
    vismodule::Real32* __restrict oc = vc.data()  + vector_offset;
    vismodule::Real32* __restrict on = vn.data()  + vector_offset;
    vismodule::Real32* __restrict ot = vt.data()  + vector_offset;
    const vismodule::Vector3f* __restrict lc = local_coord_array;
    const float* __restrict sa = scalar_array;
    const float* __restrict gx = grad_array_x;
    const float* __restrict gy = grad_array_y;
    const float* __restrict gz = grad_array_z;
    const vismodule::UInt32* __restrict ci = cell_index;
    #pragma omp simd
    for ( int k = 0; k < n; k++ )
    {
        os[k] = sa[k];
        oq[k] = sa[k] * sa[k];
        oi[k] = static_cast<int>( ci[k] );
        oc[3*k]     = lc[k].x();
        oc[3*k + 1] = lc[k].y();
        oc[3*k + 2] = lc[k].z();
        on[3*k]     = -gx[k];
        on[3*k + 1] = -gy[k];
        on[3*k + 2] = -gz[k];
        ot[3*k]     = sa[k] * gx[k];
        ot[3*k + 1] = sa[k] * gy[k];
        ot[3*k + 2] = sa[k] * gz[k];
    }
}


} // anonymous namespace

namespace vismodule
{

bool GenerateEnsembleParticles(
    const int num_ensemble,
    ParticleProperty& particle_property,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    EnsembleParticleArrays& average,
    EnsembleParticleArrays& variance,
    EnsembleParticleArrays& coefficient,
    EnsembleStatisticRange& average_range,
    EnsembleStatisticRange& variance_range,
    EnsembleStatisticRange& co_variation_range
#ifdef ENABLE_ENSEMBLE_TIMER
    , EnsembleTimerCollector* timer
#endif
)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif
    int tf_number = particle_property.m_transfunc_array.size();
#ifdef ENABLE_ENSEMBLE_TIMER
    EnsembleTimerCollector& ensemble_timer = *timer;
#endif

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
//    std::vector< std::vector<vismodule::TransferFunction> > th_tf;

    //std::vector<vismodule::TransferFunction> transfer_functions( tf_number );
    std::vector<std::vector<vismodule::TransferFunction>> transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>>           mean_transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>>       variance_transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>> coef_variation_transfer_functions( max_threads );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerInitTransferFunctions );
#endif
        for ( int n = 0; n < max_threads; n++ )
        {
            th_tfs[n] = new TransferFunctionSynthesizer( *particle_property.m_transfunc_synthesizer );
        }

        for ( int i = 0; i < max_threads; i++ )
        {
            transfer_functions[ i ].resize( tf_number );
                      mean_transfer_functions[ i ].resize( tf_number );
                  variance_transfer_functions[ i ].resize( tf_number );
            coef_variation_transfer_functions[ i ].resize( tf_number );
            for ( int j = 0; j < tf_number; j++ )
            {
                transfer_functions[i][j] = particle_property.m_transfunc_array[j];
                          mean_transfer_functions[i][j] = particle_property.m_mean_transfer_function_array[j];
                      variance_transfer_functions[i][j] = particle_property.m_variance_transfer_function_array[j];
                coef_variation_transfer_functions[i][j] = particle_property.m_coefficient_of_variation_transfer_function_array[j];
            }
        }
    }

    // アンサンブル用伝達関数のEquationTokenを取得 
    std::string expression = particle_property.m_mean_transfer_function_array[0].m_variable;
    const ::EquationToken equation_token = EnsembleTransferFunction::convert_token( expression );

//    std::cout << "particle_property.mean_max = " << particle_property.m_mean_transfer_function_array[0].colorMap().maxValue() << std::endl;
//    std::cout << "particle_property.var_max = " << particle_property.m_variance_transfer_function_array[0].colorMap().maxValue() << std::endl;
//    std::cout << "particle_property.cov_max = " << particle_property.m_coefficient_of_variation_transfer_function_array[0].colorMap().maxValue() << std::endl;



    std::vector<float> average_coords;
    std::vector<Byte> average_colors;
    std::vector<float> average_normals;
    std::vector<float> variance_coords;
    std::vector<Byte> variance_colors;
    std::vector<float> variance_normals;
    std::vector<float> coefficient_coords;
    std::vector<Byte> coefficient_colors;
    std::vector<float> coefficient_normals;

    std::vector<vismodule::UInt64> tmp_c_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<vismodule::UInt64> tmp_o_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<float> tmp_max( tf_number * 2, FLT_MIN );
    std::vector<float> tmp_min( tf_number * 2, FLT_MAX );

#ifndef CPU_VER
    if ( mpi_size <= 1 )
    {
        std::cout << "ensemble_generate_particles requires MPI ensemble ranks." << std::endl;
        return false;
    }

    // 法線計算の方式。粒子生成の全体で1回だけ決め、以降のブロックで切り替えない。
    // 実際の決定はサンプリング準備の後（粒子数の見積もりが要るため）。
    const NormalMethodRequest normal_method_request = resolve_normal_method_request();
    NormalMethod normal_method = NormalMethod::ChainRule;

    std::vector<std::vector<vismodule::CellBase<Type>*> > cell( max_threads );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCreateCells );
#endif
        for ( int thread = 0; thread < max_threads; thread++ )
        {
            cell[thread].resize( nvariables, nullptr );
            for ( int variable = 0; variable < nvariables; variable++ )
            {
                switch ( celltype )
                {
                case vismodule::VolumeObjectBase::Tetrahedra:
                    cell[thread][variable] = new vismodule::TetrahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::Hexahedra:
                    cell[thread][variable] = new vismodule::HexahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::QuadraticTetrahedra:
                    cell[thread][variable] = new vismodule::QuadraticTetrahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::QuadraticHexahedra:
                    cell[thread][variable] = new vismodule::QuadraticHexahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::Prism:
                    cell[thread][variable] = new vismodule::PrismaticCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::Pyramid:
                    cell[thread][variable] = new vismodule::PyramidalCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                default:
                    std::cout << "Unsupported cell type." << std::endl;
                    for ( int i = 0; i <= thread; i++ )
                    {
                        for ( int j = 0; j < nvariables; j++ ) delete cell[i][j];
                    }
                    return false;
                }
            }
        }
    }

    float sampling_volume_inverse = 0.0f;
    float max_opacity = 0.0f;
    float max_density = 0.0f;
    float repetitions = particle_property.m_repeat_level;  //
    const float particle_density = 1.0f;
    const int MPIprocess_per_ensemble = mpi_size/num_ensemble;
    const int ens_number = num_ensemble;
    {  // 区間計測用の{}
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerSamplingPrepare );
#endif
        sampling_volume_inverse = particle_property.m_transfunc_synthesizer->getSamplingVolumeInverse();
        max_opacity = particle_property.m_transfunc_synthesizer->getMaxOpacity();
        max_density = particle_property.m_transfunc_synthesizer->getMaxDensity();
        if ( mpi_size % MPIprocess_per_ensemble != 0 )
        {
            std::cerr << "error !! need  ens_number % MPIprocess_per_ensemble = 0!!  " << std::endl;
            return false;
        } 
        repetitions /= static_cast<float>( ens_number );
    }

    // --- 法線計算方式の決定 ---
    // 粒子数の期待値は max_density * 総体積 * repetitions で前もって求まる。
    // 総体積の走査は auto のときだけ行う（全セルを1回なめるので無料ではない）。
    {
        const DqUsage usage_for_select = collect_dq_usage( equation_token, nvariables );
        const int nnodes_of_cell = static_cast<int>( cell[0][0]->numberOfNodes() );
        double np_over_ncell = 0.0;
        bool   np_known = false;
        if ( normal_method_request == NormalMethodRequest::Auto || usage_for_select.any() )
        {
            const double v_total = total_cell_volume( cell, ncells );
            if ( v_total > 0.0 && ncells > 0 )
            {
                const double np = static_cast<double>( max_density ) * v_total
                                * static_cast<double>( repetitions );
                np_over_ncell = np / static_cast<double>( ncells );
                np_known = true;
            }
        }
        std::string reason;
        normal_method = select_normal_method(
            normal_method_request, usage_for_select.any(),
            static_cast<int>( usage_for_select.var_index.size() ),
            celltype, nnodes_of_cell, np_over_ncell, np_known, mpi_rank, &reason );
        if ( mpi_rank == 0 )
        {
            std::cerr << "[PBVR] 法線計算: " << normal_method_name( normal_method )
                      << "  判断根拠: " << reason << std::endl
                      << "[PBVR]   セル種=" << celltype_name( celltype )
                      << "(節点" << nnodes_of_cell << ") セル数=" << ncells;
            if ( np_known ) std::cerr << " 粒子数/セル数=" << np_over_ncell;
            std::cerr << " 微分量を使う変数=" << usage_for_select.var_index.size() << std::endl;
        }
    }

    // 方式C: 節点F場を1回だけ作り、スレッドごとに補間器を用意する。
    // node_F は補間器が参照し続けるので、補間器より長く生きるスコープに置く。
    std::vector<Type> node_F;
    std::vector<vismodule::CellBase<Type>*> cell_F( max_threads, NULL );
    if ( normal_method == NormalMethod::NodeFField )
    {
        // dq を含む数式のときだけ節点微分量を復元する（復元はセル数に比例して重い）。
        // 参照されている (変数, 成分) だけに絞る。
        const DqUsage dq_usage = collect_dq_usage( equation_token, nvariables );
        const bool need_dq = dq_usage.any();
        std::vector<float> node_dq;
        bool built = true;
        const NodeDqMode node_dq_mode = resolve_node_dq_mode();
        if ( need_dq && !build_node_dq_field( cell, nvariables, dq_usage, node_dq_mode,
                                              coordinates, ncoords,
                                              connections, ncells, node_dq ) )
        {
            std::cerr << "PBVR_NORMAL_METHOD=nodefield: 節点微分量の復元に失敗した。"
                      << "chainrule へ退避する。" << std::endl;
            built = false;
        }
        if ( built && !build_node_f_field( equation_token, values, nvariables,
                                           coordinates, ncoords,
                                           need_dq ? &node_dq : NULL, dq_usage, node_F ) )
        {
            std::cerr << "PBVR_NORMAL_METHOD=nodefield: 節点F場の構築に失敗した。"
                      << "chainrule へ退避する。" << std::endl;
            built = false;
        }
        // 節点F場が出来た時点で微分量は不要。ピークメモリを抑えるため解放する。
        std::vector<float>().swap( node_dq );
        if ( !built )
        {
            normal_method = NormalMethod::ChainRule;
        }
        else
        {
            for ( int thread = 0; thread < max_threads; thread++ )
            {
                cell_F[thread] = create_cell_for_celltype(
                    celltype, &node_F[0], coordinates, ncoords, connections, ncells );
            }
        }
    }

    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::Real32> vertex_scalars;
    std::vector<vismodule::Real32> vertex_normals;
    std::vector<int> vertex_cellids;
    std::vector<vismodule::Real32> sq_scalars;
    std::vector<vismodule::Real32> tmp_term;



#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> uniform_thread_times( max_threads, 0.0 );
    std::vector<double> uniform_setup_times( max_threads, 0.0 );
    std::vector<double> uniform_cell_index_times( max_threads, 0.0 );
    std::vector<double> uniform_bind_times( max_threads, 0.0 );
    std::vector<double> uniform_volume_times( max_threads, 0.0 );
    std::vector<double> uniform_particle_count_times( max_threads, 0.0 );
    std::vector<double> uniform_sampling_loop_times( max_threads, 0.0 );
    std::vector<double> uniform_local_coord_times( max_threads, 0.0 );
    std::vector<double> uniform_flush_prepare_times( max_threads, 0.0 );
    std::vector<double> uniform_scalar_times( max_threads, 0.0 );
    std::vector<double> uniform_calc_scalar_grad_times( max_threads, 0.0 );
    std::vector<double> uniform_q_grad_setup_times( max_threads, 0.0 );
    std::vector<double> uniform_tf_scalar_eval_times( max_threads, 0.0 );
    std::vector<double> uniform_chain_rule_dfdq_times( max_threads, 0.0 );
    std::vector<double> uniform_normal_normalize_times( max_threads, 0.0 );
    std::vector<double> uniform_store_times( max_threads, 0.0 );
    std::vector<double> uniform_merge_times( max_threads, 0.0 );
    vismodule::Timer uniform_timer;
    uniform_timer.start();
#endif
    std::vector<size_t> merge_off( max_threads + 1, 0 );
    size_t merge_base_s = 0, merge_base_v = 0;
#pragma omp parallel
    {
#if _OPENMP
        const int nthreads = omp_get_num_threads();
        const int thid = omp_get_thread_num();
#else
        const int nthreads = 1;
        const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer thread_timer;
        thread_timer.start();
        vismodule::Timer setup_timer;
        setup_timer.start();
        double th_uniform_setup_time = 0.0;
        double th_uniform_cell_index_time = 0.0;
        double th_uniform_bind_time = 0.0;
        double th_uniform_volume_time = 0.0;
        double th_uniform_particle_count_time = 0.0;
        double th_uniform_sampling_loop_time = 0.0;
        double th_uniform_local_coord_time = 0.0;
        double th_uniform_flush_prepare_time = 0.0;
        double th_uniform_scalar_time = 0.0;
        double th_uniform_calc_scalar_grad_time = 0.0;
        double th_uniform_q_grad_setup_time = 0.0;
        double th_uniform_tf_scalar_eval_time = 0.0;
        double th_uniform_chain_rule_dfdq_time = 0.0;
        double th_uniform_normal_normalize_time = 0.0;
        double th_uniform_store_time = 0.0;
        double th_uniform_merge_time = 0.0;
#endif
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
        float volume_array[SIMD_BLK_SIZE];
        int nparticles_array[SIMD_BLK_SIZE];
        float scalar_array[SIMD_BLK_SIZE];
        float grad_scalar[SIMD_BLK_SIZE];
        float grad_array_x[SIMD_BLK_SIZE];
        float grad_array_y[SIMD_BLK_SIZE];
        float grad_array_z[SIMD_BLK_SIZE];
        std::vector<float> o_scalars_array[SIMD_BLK_SIZE];
        for ( int i = 0; i < SIMD_BLK_SIZE; i++ ) o_scalars_array[i].resize( tf_number );
        ChainRuleEvalContext chain_context;
        chain_context.initialize( equation_token, nvariables );
        std::vector<vismodule::Real32> th_vertex_coords;
        std::vector<vismodule::Real32> th_vertex_scalars;
        std::vector<vismodule::Real32> th_vertex_normals;
        std::vector<int> th_vertex_cellids;
        std::vector<vismodule::Real32> th_sq_scalars;
        std::vector<vismodule::Real32> th_tmp_term;
#ifdef PBVR_UNIFORM_FISSION2
        // Geometry buffers for the deferred formula pass (generation writes here;
        // the post-loop pass reads them back to evaluate scalar + chain-rule normal).
        std::vector<vismodule::Vector3f> th_geo_local;
        std::vector<vismodule::UInt32>   th_geo_cellid;
#endif
        vismodule::MersenneTwister mt( thid + mpi_rank * nthreads );
#ifdef ENABLE_ENSEMBLE_TIMER
        setup_timer.stop();
        th_uniform_setup_time += setup_timer.sec();
#endif

#pragma omp for schedule( dynamic ) nowait
        for ( size_t index = 0; index < static_cast<size_t>( ncells ); index += SIMD_BLK_SIZE )
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer cell_index_timer;
            cell_index_timer.start();
#endif
            const int remain = ( ncells - index > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncells - index;
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = static_cast<vismodule::UInt32>( index + cell_BLK );
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            cell_index_timer.stop();
            th_uniform_cell_index_time += cell_index_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer bind_timer;
            bind_timer.start();
#endif
            bind_variables_scalars_opt( cell[thid], nvariables, remain, cell_index );
#ifdef ENABLE_ENSEMBLE_TIMER
            bind_timer.stop();
            th_uniform_bind_time += bind_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer volume_timer;
            volume_timer.start();
#endif
            // 一様サンプリング用のセル体積を計算する。セル種ごとの最速経路は
            // volumeArray() の override が持つ: Hex/Tet は bindCellArray() 済みの
            // m_vertices_array を読む一括版(再 gather なし・ベクトル化, #15333)、
            // それ以外は CellBase の既定実装(per-cell の bindCell+volume())。
            // ここでは celltype で分岐しない。
            cell[thid][0]->volumeArray( remain, cell_index, volume_array );
#ifdef ENABLE_ENSEMBLE_TIMER
            volume_timer.stop();
            th_uniform_volume_time += volume_timer.sec();
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer particle_count_timer;
            particle_count_timer.start();
#endif
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                nparticles_array[cell_BLK] = static_cast<int>(
                    CalculateNumberOfParticlesV35( max_density, volume_array[cell_BLK], repetitions, &mt )
//                    5
                );
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            particle_count_timer.stop();
            th_uniform_particle_count_time += particle_count_timer.sec();
#endif

            int p_id = 0;
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer sampling_loop_timer;
            sampling_loop_timer.start();
#endif
#ifndef PBVR_UNIFORM_FISSION2
            for ( int cell_BLK = 0; cell_BLK < remain + 1; cell_BLK++ )
            {
                const int nparticles_in_cell = cell_BLK < remain ? nparticles_array[cell_BLK] : 1;
                for ( int i = 0; i < nparticles_in_cell; i += SIMD_BLK_SIZE )
                {
                    const int remain_BLK = ( nparticles_in_cell - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : nparticles_in_cell - i;
                    if ( cell_BLK < remain )
                    {
#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer local_coord_timer;
                        local_coord_timer.start();
#endif
                        int j = 0;
                        while ( j < remain_BLK )
                        {
                            const int take = ( remain_BLK - j < SIMD_BLK_SIZE - p_id )
                                             ? ( remain_BLK - j ) : ( SIMD_BLK_SIZE - p_id );
                            for ( int t = 0; t < take; t++ )
                                cell_index[p_id + t] = static_cast<vismodule::UInt32>( index + cell_BLK );
                            for ( int t = 0; t < take; t++ )
                                local_coord_array[p_id + t] = cell[thid][0]->randomSampling_MT( &mt );
                            p_id += take;
                            j += take;
                            if ( p_id == SIMD_BLK_SIZE )
                            {
#ifdef ENABLE_ENSEMBLE_TIMER
                                local_coord_timer.stop();
                                th_uniform_local_coord_time += local_coord_timer.sec();
                                vismodule::Timer flush_prepare_timer;
                                flush_prepare_timer.start();
#endif
                                bind_variables_scalars_opt( cell[thid], nvariables, p_id, cell_index );
                                cell[thid][0]->setLocalPointArray( p_id, local_coord_array );
                                cell[thid][0]->transformLocalToGlobalArray( p_id, local_coord_array, global_coord_array );
#ifdef ENABLE_ENSEMBLE_TIMER
                                flush_prepare_timer.stop();
                                th_uniform_flush_prepare_time += flush_prepare_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
                                vismodule::Timer scalar_timer;
                                scalar_timer.start();
                                ChainRuleTimingBreakdown chain_rule_timing;
#endif
	                                calculate_scalar_and_normal(
	                                    normal_method,
	                                    p_id,
	                                    nvariables,
	                                    chain_context,
	                                    cell[thid],
	                                    local_coord_array,
	                                    global_coord_array,
	                                    cell_index,
	                                    cell_F[thid],   // interp_F: 方式C
	                                    scalar_array,
	                                    grad_array_x,
	                                    grad_array_y,
	                                    grad_array_z,
#ifdef ENABLE_ENSEMBLE_TIMER
	                                    &chain_rule_timing
#else
	                                    0
#endif
	                                     );
#ifdef ENABLE_ENSEMBLE_TIMER
                                scalar_timer.stop();
                                th_uniform_scalar_time += scalar_timer.sec();
                                th_uniform_calc_scalar_grad_time += chain_rule_timing.calc_scalar_grad;
                                th_uniform_q_grad_setup_time += chain_rule_timing.q_grad_setup;
                                th_uniform_tf_scalar_eval_time += chain_rule_timing.tf_scalar_eval;
                                th_uniform_chain_rule_dfdq_time += chain_rule_timing.chain_rule_dfdq;
                                th_uniform_normal_normalize_time += chain_rule_timing.normal_normalize;
#endif
//                                calculation_glad(p_id, nvariables, th_tfs[thid], transfer_functions[thid], cell[thid], local_coord_array, cell_index, grad_array_x, grad_array_y, grad_array_z);

#ifdef ENABLE_ENSEMBLE_TIMER
                                vismodule::Timer store_timer;
                                store_timer.start();
#endif
                                ReserveAdditionalUniformParticles(
                                    static_cast<size_t>( p_id ),
                                    th_vertex_coords,
	                                    th_vertex_scalars,
	                                    th_vertex_normals,
	                                    th_vertex_cellids,
	                                    th_sq_scalars,
	                                    th_tmp_term );
	                                const size_t scalar_offset = th_vertex_scalars.size();
	                                const size_t vector_offset = th_vertex_coords.size();
	                                th_vertex_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                                th_vertex_cellids.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                                th_sq_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                                th_vertex_coords.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                                th_vertex_normals.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                                th_tmp_term.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                                store_uniform_block( p_id, scalar_offset, vector_offset,
	                                    th_vertex_scalars, th_vertex_coords, th_vertex_normals,
	                                    th_vertex_cellids, th_sq_scalars, th_tmp_term,
	                                    scalar_array, local_coord_array,
	                                    grad_array_x, grad_array_y, grad_array_z, cell_index );
#ifdef ENABLE_ENSEMBLE_TIMER
                                store_timer.stop();
                                th_uniform_store_time += store_timer.sec();
#endif
                                p_id = 0;
#ifdef ENABLE_ENSEMBLE_TIMER
                                local_coord_timer.start();
#endif
                            }
                        }
#ifdef ENABLE_ENSEMBLE_TIMER
                        local_coord_timer.stop();
                        th_uniform_local_coord_time += local_coord_timer.sec();
#endif
                    }
                    else if ( p_id > 0 )
                    {
#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer flush_prepare_timer;
                        flush_prepare_timer.start();
#endif
                        bind_variables_scalars_opt( cell[thid], nvariables, p_id, cell_index );
                        cell[thid][0]->setLocalPointArray( p_id, local_coord_array );
                        cell[thid][0]->transformLocalToGlobalArray( p_id, local_coord_array, global_coord_array );
#ifdef ENABLE_ENSEMBLE_TIMER
                        flush_prepare_timer.stop();
                        th_uniform_flush_prepare_time += flush_prepare_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer scalar_timer;
                        scalar_timer.start();
                        ChainRuleTimingBreakdown chain_rule_timing;
#endif
	                        calculate_scalar_and_normal(
	                            normal_method,
	                            p_id,
	                            nvariables,
	                            chain_context,
	                            cell[thid],
	                            local_coord_array,
	                            global_coord_array,
	                            cell_index,
	                            cell_F[thid],   // interp_F: 方式C
	                            scalar_array,
	                            grad_array_x,
	                            grad_array_y,
	                            grad_array_z,
#ifdef ENABLE_ENSEMBLE_TIMER
	                            &chain_rule_timing
#else
	                            0
#endif
	                             );
#ifdef ENABLE_ENSEMBLE_TIMER
                        scalar_timer.stop();
                        th_uniform_scalar_time += scalar_timer.sec();
                        th_uniform_calc_scalar_grad_time += chain_rule_timing.calc_scalar_grad;
                        th_uniform_q_grad_setup_time += chain_rule_timing.q_grad_setup;
                        th_uniform_tf_scalar_eval_time += chain_rule_timing.tf_scalar_eval;
                        th_uniform_chain_rule_dfdq_time += chain_rule_timing.chain_rule_dfdq;
                        th_uniform_normal_normalize_time += chain_rule_timing.normal_normalize;
#endif
//                        calculation_glad(p_id, nvariables, th_tfs[thid], transfer_functions[thid], cell[thid], local_coord_array, cell_index, grad_array_x, grad_array_y, grad_array_z);


#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer store_timer;
                        store_timer.start();
#endif
                        ReserveAdditionalUniformParticles(
                            static_cast<size_t>( p_id ),
                            th_vertex_coords,
	                            th_vertex_scalars,
	                            th_vertex_normals,
	                            th_vertex_cellids,
	                            th_sq_scalars,
	                            th_tmp_term );
	                        const size_t scalar_offset = th_vertex_scalars.size();
	                        const size_t vector_offset = th_vertex_coords.size();
	                        th_vertex_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                        th_vertex_cellids.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                        th_sq_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                        th_vertex_coords.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                        th_vertex_normals.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                        th_tmp_term.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                        store_uniform_block( p_id, scalar_offset, vector_offset,
	                            th_vertex_scalars, th_vertex_coords, th_vertex_normals,
	                            th_vertex_cellids, th_sq_scalars, th_tmp_term,
	                            scalar_array, local_coord_array,
	                            grad_array_x, grad_array_y, grad_array_z, cell_index );
#ifdef ENABLE_ENSEMBLE_TIMER
                        store_timer.stop();
                        th_uniform_store_time += store_timer.sec();
#endif
                        p_id = 0;
                    }
                }
            }
#else
            // PBVR_UNIFORM_FISSION2: no packing. Append n local coords + cellid per cell.
            // bind / setLocal / transform / formula all move to the dense pass below
            // (single bind, no global buffer). mt draw order = cell order = fused order.
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                const int npc = nparticles_array[cell_BLK];
                if ( npc <= 0 ) continue;
                const size_t g0 = th_geo_cellid.size();
                th_geo_cellid.resize( g0 + static_cast<size_t>( npc ) );
                th_geo_local.resize(  g0 + static_cast<size_t>( npc ) );
                const vismodule::UInt32 cid = static_cast<vismodule::UInt32>( index + cell_BLK );
                for ( int t = 0; t < npc; t++ )
                {
                    th_geo_local[g0 + t]  = cell[thid][0]->randomSampling_MT( &mt );
                    th_geo_cellid[g0 + t] = cid;
                }
            }
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            sampling_loop_timer.stop();
            th_uniform_sampling_loop_time += sampling_loop_timer.sec();
#endif
        }
#ifdef PBVR_UNIFORM_FISSION2
        // Dense formula pass (loop fission) over all generated particles: re-bind per BLK,
        // transform to global, evaluate scalar + chain-rule normal, and store. Single bind,
        // no global buffer. Invariant-preserving (formula consumes no RNG; bit-identical at
        // 1 thread/rank).
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer fission_timer;
            double th_fission_scalar = 0.0, th_fission_store = 0.0;
#endif
            const size_t P = th_geo_cellid.size();
            for ( size_t fp = 0; fp < P; fp += SIMD_BLK_SIZE )
            {
                const int m = ( P - fp > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : static_cast<int>( P - fp );
                vismodule::Vector3f* fl = &th_geo_local[fp];
                vismodule::Vector3f* fg = global_coord_array;
                const vismodule::UInt32*   fc = &th_geo_cellid[fp];
#ifdef ENABLE_ENSEMBLE_TIMER
                fission_timer.start();
#endif
                bind_variables_scalars_opt( cell[thid], nvariables, m, fc );
                cell[thid][0]->setLocalPointArray( m, fl );
                cell[thid][0]->transformLocalToGlobalArray( m, fl, fg );
                calculate_scalar_and_normal( normal_method, m, nvariables, chain_context, cell[thid],
                    fl, fg, fc, cell_F[thid], scalar_array, grad_array_x, grad_array_y, grad_array_z, 0 );
#ifdef ENABLE_ENSEMBLE_TIMER
                fission_timer.stop();  th_fission_scalar += fission_timer.sec();  fission_timer.start();
#endif
                ReserveAdditionalUniformParticles( static_cast<size_t>( m ),
                    th_vertex_coords, th_vertex_scalars, th_vertex_normals,
                    th_vertex_cellids, th_sq_scalars, th_tmp_term );
                const size_t scalar_offset = th_vertex_scalars.size();
                const size_t vector_offset = th_vertex_coords.size();
                th_vertex_scalars.resize( scalar_offset + static_cast<size_t>( m ) );
                th_vertex_cellids.resize( scalar_offset + static_cast<size_t>( m ) );
                th_sq_scalars.resize( scalar_offset + static_cast<size_t>( m ) );
                th_vertex_coords.resize( vector_offset + 3 * static_cast<size_t>( m ) );
                th_vertex_normals.resize( vector_offset + 3 * static_cast<size_t>( m ) );
                th_tmp_term.resize( vector_offset + 3 * static_cast<size_t>( m ) );
                store_uniform_block( m, scalar_offset, vector_offset,
                    th_vertex_scalars, th_vertex_coords, th_vertex_normals,
                    th_vertex_cellids, th_sq_scalars, th_tmp_term,
                    scalar_array, fl, grad_array_x, grad_array_y, grad_array_z, fc );
#ifdef ENABLE_ENSEMBLE_TIMER
                fission_timer.stop();  th_fission_store += fission_timer.sec();
#endif
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            th_uniform_scalar_time += th_fission_scalar;
            th_uniform_store_time  += th_fission_store;
#endif
        }
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer merge_timer;
        merge_timer.start();
#endif
        // Prefix-sum parallel merge: each thread records its particle count, one thread
        // computes the offsets and resizes the global arrays once, then every thread copies
        // its buffer to its own disjoint offset in parallel (no omp critical -> no
        // serialization). Particle order becomes thread-id order (was non-deterministic
        // critical order); invariants are preserved.
        merge_off[thid + 1] = th_vertex_scalars.size();
        #pragma omp barrier
        #pragma omp single
        {
            merge_base_s = vertex_scalars.size();
            merge_base_v = vertex_coords.size();
            for ( int t = 0; t < max_threads; ++t ) merge_off[t + 1] += merge_off[t];
            const size_t total = merge_off[max_threads];
            vertex_scalars.resize( merge_base_s + total );
            vertex_cellids.resize( merge_base_s + total );
            sq_scalars.resize( merge_base_s + total );
            vertex_coords.resize( merge_base_v + 3 * total );
            vertex_normals.resize( merge_base_v + 3 * total );
            tmp_term.resize( merge_base_v + 3 * total );
        }
        {
            const size_t so = merge_base_s + merge_off[thid];
            const size_t vo = merge_base_v + 3 * merge_off[thid];
            std::copy( th_vertex_scalars.begin(), th_vertex_scalars.end(), vertex_scalars.begin() + so );
            std::copy( th_vertex_cellids.begin(), th_vertex_cellids.end(), vertex_cellids.begin() + so );
            std::copy( th_sq_scalars.begin(), th_sq_scalars.end(), sq_scalars.begin() + so );
            std::copy( th_vertex_coords.begin(), th_vertex_coords.end(), vertex_coords.begin() + vo );
            std::copy( th_vertex_normals.begin(), th_vertex_normals.end(), vertex_normals.begin() + vo );
            std::copy( th_tmp_term.begin(), th_tmp_term.end(), tmp_term.begin() + vo );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        merge_timer.stop();
        th_uniform_merge_time += merge_timer.sec();
        thread_timer.stop();
        uniform_thread_times[thid] += thread_timer.sec();
        uniform_setup_times[thid] += th_uniform_setup_time;
        uniform_cell_index_times[thid] += th_uniform_cell_index_time;
        uniform_bind_times[thid] += th_uniform_bind_time;
        uniform_volume_times[thid] += th_uniform_volume_time;
        uniform_particle_count_times[thid] += th_uniform_particle_count_time;
        uniform_sampling_loop_times[thid] += th_uniform_sampling_loop_time;
        uniform_local_coord_times[thid] += th_uniform_local_coord_time;
        uniform_flush_prepare_times[thid] += th_uniform_flush_prepare_time;
        uniform_scalar_times[thid] += th_uniform_scalar_time;
        uniform_calc_scalar_grad_times[thid] += th_uniform_calc_scalar_grad_time;
        uniform_q_grad_setup_times[thid] += th_uniform_q_grad_setup_time;
        uniform_tf_scalar_eval_times[thid] += th_uniform_tf_scalar_eval_time;
        uniform_chain_rule_dfdq_times[thid] += th_uniform_chain_rule_dfdq_time;
        uniform_normal_normalize_times[thid] += th_uniform_normal_normalize_time;
        uniform_store_times[thid] += th_uniform_store_time;
        uniform_merge_times[thid] += th_uniform_merge_time;
#endif
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    uniform_timer.stop();
    ensemble_timer.add( EnsembleTimerOmpUniformSampling, uniform_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpUniformSampling, t, uniform_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformThreadSetup, t, uniform_setup_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformCellIndexSetup, t, uniform_cell_index_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformBindCellArray, t, uniform_bind_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformVolumeCalculation, t, uniform_volume_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformParticleCountCalculation, t, uniform_particle_count_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformParticleSamplingLoop, t, uniform_sampling_loop_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformLocalCoordGeneration, t, uniform_local_coord_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformFlushPrepare, t, uniform_flush_prepare_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformCalculateScalars, t, uniform_scalar_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformCalcScalarGrad, t, uniform_calc_scalar_grad_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformQGradSetup, t, uniform_q_grad_setup_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformTfScalarEval, t, uniform_tf_scalar_eval_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformChainRuleDfdq, t, uniform_chain_rule_dfdq_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformNormalNormalize, t, uniform_normal_normalize_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformStoreParticleData, t, uniform_store_times[t] );
        ensemble_timer.addThread( EnsembleTimerThreadParticleMerge, t, uniform_merge_times[t] );
    }
    ensemble_timer.setUniformParticleCount(
        static_cast<unsigned long long>( vertex_coords.size() / 3 ) );
#endif

    std::vector<std::vector<float> > v_scalars( 2 );
    std::vector<std::vector<float> > v_coords( 2 );
    std::vector<std::vector<float> > v_normals( 2 );
    std::vector<std::vector<int> > v_cellids( 2 );
    std::vector<std::vector<float> > v_sq( 2 );
    std::vector<std::vector<float> > v_tmp( 2 );
    v_scalars[0].swap( vertex_scalars );
    v_coords[0].swap( vertex_coords );
    v_normals[0].swap( vertex_normals );
    v_cellids[0].swap( vertex_cellids );
    v_sq[0].swap( sq_scalars );
    v_tmp[0].swap( tmp_term );

    int cur = 0;
    int nxt = 1;
#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> shift_interp_thread_times( max_threads, 0.0 );
    std::vector<double> shift_scalar_thread_times( max_threads, 0.0 );
    std::vector<double> shift_calc_scalar_grad_times( max_threads, 0.0 );
    std::vector<double> shift_q_grad_setup_times( max_threads, 0.0 );
    std::vector<double> shift_tf_scalar_eval_times( max_threads, 0.0 );
    std::vector<double> shift_chain_rule_dfdq_times( max_threads, 0.0 );
    std::vector<double> shift_normal_normalize_times( max_threads, 0.0 );
    std::vector<double> shift_recover_times( max_threads, 0.0 );
    std::vector<double> shift_flush_prepare_times( max_threads, 0.0 );
    std::vector<double> shift_store_times( max_threads, 0.0 );
    vismodule::Timer mpi_shift_timer;
    mpi_shift_timer.start();
#endif
    for ( int shift = 1; shift < ens_number; shift++ )
    {
        const int send_to = ( mpi_rank + MPIprocess_per_ensemble ) % mpi_size;
        const int recv_from = ( mpi_rank - MPIprocess_per_ensemble + mpi_size ) % mpi_size;
        const size_t send_count = v_scalars[cur].size();
        if ( send_count > static_cast<size_t>( INT_MAX ) ||
             send_count > static_cast<size_t>( INT_MAX / 3 ) ||
             v_coords[cur].size() != 3 * send_count ||
             v_normals[cur].size() != 3 * send_count ||
             v_cellids[cur].size() != send_count ||
             v_sq[cur].size() != send_count ||
             v_tmp[cur].size() != 3 * send_count )
        {
            std::cerr << "Invalid ensemble exchange send buffer size at rank " << mpi_rank
                      << ": scalars=" << v_scalars[cur].size()
                      << ", coords=" << v_coords[cur].size()
                      << ", normals=" << v_normals[cur].size()
                      << ", cellids=" << v_cellids[cur].size()
                      << ", sq=" << v_sq[cur].size()
                      << ", tmp=" << v_tmp[cur].size()
                      << std::endl;
            return false;
        }
        const int send_size = static_cast<int>( send_count );
        int recv_size = 0;
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer mpi_size_timer;
        mpi_size_timer.start();
#endif
        {
            MPI_Request reqs[2];
            MPI_Isend( &send_size, 1, MPI_INT, send_to, 0, MPI_COMM_WORLD, &reqs[0] );
            MPI_Irecv( &recv_size, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, &reqs[1] );
            MPI_Waitall( 2, reqs, MPI_STATUSES_IGNORE );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        mpi_size_timer.stop();
        ensemble_timer.add( EnsembleTimerMpiShiftSizeExchange, mpi_size_timer.sec() );
#endif
        if ( recv_size < 0 || recv_size > INT_MAX / 3 )
        {
            std::cerr << "Invalid ensemble exchange receive size at rank " << mpi_rank
                      << ": recv_size=" << recv_size << std::endl;
            return false;
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer alloc_timer;
        alloc_timer.start();
#endif
        std::vector<float> recv_scalars( recv_size );
        std::vector<float> recv_coords( 3 * recv_size );
        std::vector<float> recv_normals( 3 * recv_size );
        std::vector<int> recv_cellids( recv_size );
        std::vector<float> recv_sq_scalars( recv_size );
        std::vector<float> recv_tmp_term( 3 * recv_size );
#ifdef ENABLE_ENSEMBLE_TIMER
        alloc_timer.stop();
        ensemble_timer.add( EnsembleTimerMpiShiftAllocRecvBuffer, alloc_timer.sec() );
#endif
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer payload_all_timer;
            payload_all_timer.start();
#endif
            MPI_Request req_recv[6];
            MPI_Request req_send[6];
            MPI_Irecv( recv_cellids.data(), recv_size, MPI_INT, recv_from, 12, MPI_COMM_WORLD, &req_recv[0] );
            MPI_Irecv( recv_scalars.data(), recv_size, MPI_FLOAT, recv_from, 10, MPI_COMM_WORLD, &req_recv[1] );
            MPI_Irecv( recv_coords.data(), 3 * recv_size, MPI_FLOAT, recv_from, 11, MPI_COMM_WORLD, &req_recv[2] );
            MPI_Irecv( recv_normals.data(), 3 * recv_size, MPI_FLOAT, recv_from, 13, MPI_COMM_WORLD, &req_recv[3] );
            MPI_Irecv( recv_sq_scalars.data(), recv_size, MPI_FLOAT, recv_from, 14, MPI_COMM_WORLD, &req_recv[4] );
            MPI_Irecv( recv_tmp_term.data(), 3 * recv_size, MPI_FLOAT, recv_from, 15, MPI_COMM_WORLD, &req_recv[5] );
            MPI_Isend( v_cellids[cur].data(), send_size, MPI_INT, send_to, 12, MPI_COMM_WORLD, &req_send[0] );
            MPI_Isend( v_scalars[cur].data(), send_size, MPI_FLOAT, send_to, 10, MPI_COMM_WORLD, &req_send[1] );
            MPI_Isend( v_coords[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 11, MPI_COMM_WORLD, &req_send[2] );
            MPI_Isend( v_normals[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 13, MPI_COMM_WORLD, &req_send[3] );
            MPI_Isend( v_sq[cur].data(), send_size, MPI_FLOAT, send_to, 14, MPI_COMM_WORLD, &req_send[4] );
            MPI_Isend( v_tmp[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 15, MPI_COMM_WORLD, &req_send[5] );
            MPI_Waitall( 6, req_recv, MPI_STATUSES_IGNORE );
            MPI_Waitall( 6, req_send, MPI_STATUSES_IGNORE );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_all_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadAll, payload_all_timer.sec() );
#endif
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer shift_interp_timer;
        shift_interp_timer.start();
#endif
        const int rn = recv_size;
        int* R_cell = recv_cellids.data(); float* R_coor = recv_coords.data(); float* R_scal = recv_scalars.data();
        float* R_norm = recv_normals.data(); float* R_sq = recv_sq_scalars.data(); float* R_tmp = recv_tmp_term.data();
#pragma omp parallel
        {
#if _OPENMP
            const int thid = omp_get_thread_num();
#else
            const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer thread_timer;
            thread_timer.start();
            double th_shift_scalar_time = 0.0;
            double th_shift_calc_scalar_grad_time = 0.0;
            double th_shift_q_grad_setup_time = 0.0;
            double th_shift_tf_scalar_eval_time = 0.0;
            double th_shift_chain_rule_dfdq_time = 0.0;
            double th_shift_normal_normalize_time = 0.0;
            double th_shift_recover_time = 0.0;
            double th_shift_flush_prepare_time = 0.0;
            double th_shift_store_time = 0.0;
#endif
            vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
            vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
            vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
            std::vector<float> o_scalars_array[SIMD_BLK_SIZE];
            float scalar_array[SIMD_BLK_SIZE];
            float grad_scalar[SIMD_BLK_SIZE];
            float grad_array_x[SIMD_BLK_SIZE];
            float grad_array_y[SIMD_BLK_SIZE];
            float grad_array_z[SIMD_BLK_SIZE];
            for ( int i = 0; i < SIMD_BLK_SIZE; i++ ) o_scalars_array[i].resize( tf_number );
            ChainRuleEvalContext chain_context;
            chain_context.initialize( equation_token, nvariables );

//#pragma omp for schedule( dynamic )
#pragma omp for 
            for ( int i = 0; i < rn; i += SIMD_BLK_SIZE )
            {
                const int remain_BLK = ( rn - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : rn - i;
#ifdef ENABLE_ENSEMBLE_TIMER
                vismodule::Timer shift_recover_timer;
                shift_recover_timer.start();
#endif
                for ( int j = 0; j < remain_BLK; j++ )
                {
                    cell_index[j] = static_cast<vismodule::UInt32>( R_cell[i + j] );
                    local_coord_array[j] = vismodule::Vector3f(
                        R_coor[3 * ( i + j )],
                        R_coor[3 * ( i + j ) + 1],
                        R_coor[3 * ( i + j ) + 2]
                    );
                }
#ifdef ENABLE_ENSEMBLE_TIMER
                shift_recover_timer.stop();
                th_shift_recover_time += shift_recover_timer.sec();
                vismodule::Timer shift_flush_timer;
                shift_flush_timer.start();
#endif
                bind_variables_scalars_opt( cell[thid], nvariables, remain_BLK, cell_index );
                cell[thid][0]->setLocalPointArray( remain_BLK, local_coord_array );
                cell[thid][0]->transformLocalToGlobalArray( remain_BLK, local_coord_array, global_coord_array );
#ifdef ENABLE_ENSEMBLE_TIMER
                shift_flush_timer.stop();
                th_shift_flush_prepare_time += shift_flush_timer.sec();
                vismodule::Timer scalar_timer;
                scalar_timer.start();
                ChainRuleTimingBreakdown chain_rule_timing;
#endif
	                calculate_scalar_and_normal(
	                    normal_method,
	                    remain_BLK,
	                    nvariables,
	                    chain_context,
	                    cell[thid],
	                    local_coord_array,
	                    global_coord_array,
	                    cell_index,
	                    cell_F[thid],   // interp_F: 方式C
	                    scalar_array,
	                    grad_array_x,
	                    grad_array_y,
	                    grad_array_z,
#ifdef ENABLE_ENSEMBLE_TIMER
	                    &chain_rule_timing
#else
	                    0
#endif
	                     );
#ifdef ENABLE_ENSEMBLE_TIMER
                scalar_timer.stop();
                th_shift_scalar_time += scalar_timer.sec();
                th_shift_calc_scalar_grad_time += chain_rule_timing.calc_scalar_grad;
                th_shift_q_grad_setup_time += chain_rule_timing.q_grad_setup;
                th_shift_tf_scalar_eval_time += chain_rule_timing.tf_scalar_eval;
                th_shift_chain_rule_dfdq_time += chain_rule_timing.chain_rule_dfdq;
                th_shift_normal_normalize_time += chain_rule_timing.normal_normalize;
#endif
//                calculation_glad(remain_BLK, nvariables, th_tfs[thid], transfer_functions[thid], cell[thid], local_coord_array, cell_index, grad_array_x, grad_array_y, grad_array_z);


#ifdef ENABLE_ENSEMBLE_TIMER
                vismodule::Timer shift_store_timer;
                shift_store_timer.start();
#endif
                for ( int j = 0; j < remain_BLK; j++ )
                {
                    const float scalar = scalar_array[j];
                    R_scal[i + j] += scalar;
                    R_norm[3 * ( i + j )]     += -grad_array_x[j];
                    R_norm[3 * ( i + j ) + 1] += -grad_array_y[j];
                    R_norm[3 * ( i + j ) + 2] += -grad_array_z[j];
                    R_sq[i + j] += scalar * scalar;
                    R_tmp[3 * ( i + j )] += scalar * grad_array_x[j];
                    R_tmp[3 * ( i + j ) + 1] += scalar * grad_array_y[j];
                    R_tmp[3 * ( i + j ) + 2] += scalar * grad_array_z[j];
                }
#ifdef ENABLE_ENSEMBLE_TIMER
                shift_store_timer.stop();
                th_shift_store_time += shift_store_timer.sec();
#endif
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            thread_timer.stop();
            shift_interp_thread_times[thid] += thread_timer.sec();
            shift_scalar_thread_times[thid] += th_shift_scalar_time;
            shift_calc_scalar_grad_times[thid] += th_shift_calc_scalar_grad_time;
            shift_q_grad_setup_times[thid] += th_shift_q_grad_setup_time;
            shift_tf_scalar_eval_times[thid] += th_shift_tf_scalar_eval_time;
            shift_chain_rule_dfdq_times[thid] += th_shift_chain_rule_dfdq_time;
            shift_normal_normalize_times[thid] += th_shift_normal_normalize_time;
            shift_recover_times[thid] += th_shift_recover_time;
            shift_flush_prepare_times[thid] += th_shift_flush_prepare_time;
            shift_store_times[thid] += th_shift_store_time;
#endif
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        shift_interp_timer.stop();
        ensemble_timer.add( EnsembleTimerOmpShiftInterpolation, shift_interp_timer.sec() );
#endif

        v_scalars[nxt].swap( recv_scalars );
        v_coords[nxt].swap( recv_coords );
        v_normals[nxt].swap( recv_normals );
        v_cellids[nxt].swap( recv_cellids );
        v_sq[nxt].swap( recv_sq_scalars );
        v_tmp[nxt].swap( recv_tmp_term );
        std::swap( cur, nxt );
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    mpi_shift_timer.stop();
    ensemble_timer.add( EnsembleTimerMpiShiftExchange, mpi_shift_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpShiftInterpolation, t, shift_interp_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftCalculateScalars, t, shift_scalar_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftCalcScalarGrad, t, shift_calc_scalar_grad_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftQGradSetup, t, shift_q_grad_setup_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftTfScalarEval, t, shift_tf_scalar_eval_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftChainRuleDfdq, t, shift_chain_rule_dfdq_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftNormalNormalize, t, shift_normal_normalize_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftRecoverRecv, t, shift_recover_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftFlushPrepare, t, shift_flush_prepare_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftStoreAccumulate, t, shift_store_times[t] );
    }
#endif

    vertex_scalars.swap( v_scalars[cur] );
    vertex_coords.swap( v_coords[cur] );
    vertex_normals.swap( v_normals[cur] );
    vertex_cellids.swap( v_cellids[cur] );
    sq_scalars.swap( v_sq[cur] );
    tmp_term.swap( v_tmp[cur] );

    std::vector<float> tmp_varience( vertex_scalars.size() );
    std::vector<float> tmp_varience_normals( 3 * vertex_scalars.size() );
    std::vector<float> co_varietion( vertex_scalars.size() );
    std::vector<float> co_varietion_normals( 3 * vertex_scalars.size() );
    size_t co_varietion_normal_fallback_count = 0;
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatAverageVariance );
#endif
    const float invert_num = 1.0f / static_cast<float>( ens_number );
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        vertex_scalars[i] *= invert_num;
        sq_scalars[i] *= invert_num;
    }
    for ( size_t i = 0; i < tmp_term.size(); i++ )
    {
        tmp_term[i] = -2.0f * invert_num * tmp_term[i];
        vertex_normals[i] *= -invert_num;
    }
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        tmp_varience[i] = sq_scalars[i] - vertex_scalars[i] * vertex_scalars[i];
        if ( tmp_varience[i] < 0.0f ) tmp_varience[i] = 0.0f;
        tmp_varience_normals[3 * i] = tmp_term[3 * i] - ( -2.0f  * vertex_scalars[i] * vertex_normals[3 * i] );
        tmp_varience_normals[3 * i + 1] = tmp_term[3 * i + 1] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 1] );
        tmp_varience_normals[3 * i + 2] = tmp_term[3 * i + 2] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 2] );
    }

    const float delta = 1.0e-30f;
    const float eps = 1.0e-5f;
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        co_varietion[i] = std::fabs(vertex_scalars[i]) > eps ? std::sqrt( tmp_varience[i] ) /std::fabs( vertex_scalars[i]) : delta;
    }

    // 法線の構成は co_varietion の計算ループとは別ループにする。
    // 同一ループへ追記するとベクトル化/FP縮約の判断が変わり co_varietion が最下位ビットで
    // 変動しうる(実測: 色マップの量子化境界で 1 粒子の色が 1 階調ずれた)。分離すれば
    // co_varietion の生成コードが元のままとなり、粒子の色は完全に一致する。
    // 符号規約(実測): vertex_normals=+grad mu / tmp_varience_normals=-grad Var（両者は規約が逆）。
    // 展開: co_varietion_normals[j] = tmp_varience_normals[j] + vertex_normals[j] * s_i
    //   s_i = 2*Var/mu = 2*co_varietion[i]^2*mu_i （|mu|>eps。co_varietion=sqrt(Var)/|mu| より除算を乗算化=A-1）
    //       = 0 （|mu|<=eps: CoV 未定義 → 分散法線を流用する従来動作。s_i=0 で varn+vn*0=varn と厳密一致）
    // フォールバックは s_i=0 でブランチレス化(A-2)。法線は正規化され方向のみ有意のため OpenMP+SIMD/FMA で
    // 構成する(A-3/B-1)。co_varietion の"値"は本ループでは読むだけで不変(別ループ)ゆえ色・座標は従来と一致。
    // ComputeCoVNormalDirection(ChainRuleNormal.h)と代数的に等価: 2*co_varietion^2*mu = 2*Var/mu。
    {
        const size_t            nvert = vertex_scalars.size();
        const float* __restrict mu    = vertex_scalars.data();       // 平均 mu
        const float* __restrict cov   = co_varietion.data();         // 算出済み CoV 値(直前ループ)
        const float* __restrict vn    = vertex_normals.data();       // +grad mu
        const float* __restrict varn  = tmp_varience_normals.data(); // -grad Var
        float*       __restrict covn  = co_varietion_normals.data();
        size_t fb = 0;
        #pragma omp parallel for simd reduction(+:fb) schedule(static)
        for ( size_t i = 0; i < nvert; i++ )
        {
            const bool   ok = std::fabs( mu[i] ) > eps;              // NaN も偽=フォールバック
            const float  si = ok ? ( 2.0f * cov[i] * cov[i] * mu[i] ) : 0.0f;
            const size_t b  = 3 * i;
//            covn[b    ] = varn[b    ] + vn[b    ] * si;
//            covn[b + 1] = varn[b + 1] + vn[b + 1] * si;
//            covn[b + 2] = varn[b + 2] + vn[b + 2] * si;
            covn[b    ] = varn[b    ] - vn[b    ] * si;
            covn[b + 1] = varn[b + 1] - vn[b + 1] * si;
            covn[b + 2] = varn[b + 2] - vn[b + 2] * si;
            fb += ok ? 0u : 1u;
        }
        co_varietion_normal_fallback_count = fb;
    }
    }

    fprintf( stdout, "[cov_normal] rank=%d fallback=%llu / particles=%llu\n",
             mpi_rank,
             static_cast<unsigned long long>( co_varietion_normal_fallback_count ),
             static_cast<unsigned long long>( vertex_scalars.size() ) );

    // 統計量の対数化: フラグ ON なら 平均(vertex_scalars)・分散(tmp_varience)を log10。
    // 変動係数(co_varietion)・変動係数法線は上で算出済み＝対数化前。法線/最適化ループは不変。
    if ( particle_property.m_log_scale_statistics )
        ApplyLog10ToMeanVariance( vertex_scalars, tmp_varience );

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatHistogram );
#endif
        average_range = MakeEnsembleStatisticMinMax( vertex_scalars, tf_number );
        variance_range = MakeEnsembleStatisticMinMax( tmp_varience, tf_number );
        co_variation_range = MakeEnsembleStatisticMinMax( co_varietion, tf_number );

        AggregateEnsembleStatisticMinMax( average_range, tf_number, MPI_COMM_WORLD );
        AggregateEnsembleStatisticMinMax( variance_range, tf_number, MPI_COMM_WORLD );
        AggregateEnsembleStatisticMinMax( co_variation_range, tf_number, MPI_COMM_WORLD );

        ApplyEnsembleStatisticMinMax(
            average_range, particle_property.m_mean_transfer_function_array, tf_number );
        ApplyEnsembleStatisticMinMax(
            variance_range, particle_property.m_variance_transfer_function_array, tf_number );
        ApplyEnsembleStatisticMinMax(
            co_variation_range,
            particle_property.m_coefficient_of_variation_transfer_function_array,
            tf_number );

        for ( int thread = 0; thread < max_threads; thread++ )
        {
            for ( int i = 0; i < tf_number; i++ )
            {
                mean_transfer_functions[thread][i] = particle_property.m_mean_transfer_function_array[i];
                variance_transfer_functions[thread][i] = particle_property.m_variance_transfer_function_array[i];
                coef_variation_transfer_functions[thread][i] =
                    particle_property.m_coefficient_of_variation_transfer_function_array[i];
            }
        }

        CalculateEnsembleStatisticHistogram(
            average_range,
            vertex_scalars,
            tf_number,
            particle_property.m_mean_transfer_function_array );
        CalculateEnsembleStatisticHistogram(
            variance_range,
            tmp_varience,
            tf_number,
            particle_property.m_variance_transfer_function_array );
        CalculateEnsembleStatisticHistogram(
            co_variation_range,
            co_varietion,
            tf_number,
            particle_property.m_coefficient_of_variation_transfer_function_array );

        particle_property.m_transfunc_synthesizer->m_o_min.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_o_max.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_c_min.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_c_max.resize( tf_number );
        for ( int i = 0; i < tf_number; i++ )
        {
            particle_property.m_transfunc_synthesizer->m_o_min[i] = average_range.min_values[2 * i    ];
            particle_property.m_transfunc_synthesizer->m_o_max[i] = average_range.max_values[2 * i    ];
            particle_property.m_transfunc_synthesizer->m_c_min[i] = average_range.min_values[2 * i + 1];
            particle_property.m_transfunc_synthesizer->m_c_max[i] = average_range.max_values[2 * i + 1];
        }
    }

#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> rejection_thread_times( max_threads, 0.0 );
    std::vector<double> rejection_merge_times( max_threads, 0.0 );
    vismodule::Timer rejection_timer;
    rejection_timer.start();
#endif
    std::vector<size_t> rej_avg_off( max_threads + 1, 0 );
    std::vector<size_t> rej_var_off( max_threads + 1, 0 );
    std::vector<size_t> rej_coef_off( max_threads + 1, 0 );
    size_t rej_avg_base = 0, rej_var_base = 0, rej_coef_base = 0;
#pragma omp parallel
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
#else
        const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer thread_timer;
        thread_timer.start();
        double th_rejection_merge_time = 0.0;
#endif
        std::vector<float> th_average_coords;
        std::vector<Byte> th_average_colors;
        std::vector<float> th_average_normals;
        std::vector<float> th_variance_coords;
        std::vector<Byte> th_variance_colors;
        std::vector<float> th_variance_normals;
        std::vector<float> th_coefficient_coords;
        std::vector<Byte> th_coefficient_colors;
        std::vector<float> th_coefficient_normals;
        vismodule::MersenneTwister mt( 10 + mpi_rank + thid );
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];

#pragma omp for schedule( dynamic )
        for ( size_t i = 0; i < vertex_scalars.size(); i += SIMD_BLK_SIZE )
        {
            const int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : vertex_scalars.size() - i;
            for ( int j = 0; j < remain_BLK; j++ )
            {
                cell_index[j] = static_cast<vismodule::UInt32>( vertex_cellids[i + j] );
                local_coord_array[j] = vismodule::Vector3f(
                    vertex_coords[3 * ( i + j )],
                    vertex_coords[3 * ( i + j ) + 1],
                    vertex_coords[3 * ( i + j ) + 2]
                );
            }
            bind_variables_scalars_opt( cell[thid], nvariables, remain_BLK, cell_index );
            cell[thid][0]->setLocalPointArray( remain_BLK, local_coord_array );
            cell[thid][0]->transformLocalToGlobalArray( remain_BLK, local_coord_array, global_coord_array );

            for ( int j = 0; j < remain_BLK; j++ )
            {
                const size_t idx = i + j;
                const vismodule::Vector3f average_normal(
                    -vertex_normals[3 * idx],
                    -vertex_normals[3 * idx + 1],
                    -vertex_normals[3 * idx + 2]
                );
                const vismodule::Vector3f variance_normal(
                    tmp_varience_normals[3 * idx],
                    tmp_varience_normals[3 * idx + 1],
                    tmp_varience_normals[3 * idx + 2]
                );
                const vismodule::Vector3f coefficient_normal(
                    co_varietion_normals[3 * idx],
                    co_varietion_normals[3 * idx + 1],
                    co_varietion_normals[3 * idx + 2]
                );

//                std::cout << "tmp_varience_normals = " << variance_normal <<std::endl;
                AppendRejectedStatisticParticle(
                    vertex_scalars[idx], global_coord_array[j], average_normal, mean_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_average_coords, th_average_colors, th_average_normals
                );
                AppendRejectedStatisticParticle(
                    tmp_varience[idx], global_coord_array[j], variance_normal, variance_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_variance_coords, th_variance_colors, th_variance_normals
                );
                AppendRejectedStatisticParticle(
                    co_varietion[idx], global_coord_array[j], coefficient_normal, coef_variation_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_coefficient_coords, th_coefficient_colors, th_coefficient_normals
                );
            }
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer merge_timer;
        merge_timer.start();
#endif
        // Prefix-sum parallel merge per statistic type (average/variance/coefficient have
        // independent accepted counts; within a type coords/colors/normals share the count).
        rej_avg_off[thid + 1]  = th_average_coords.size();
        rej_var_off[thid + 1]  = th_variance_coords.size();
        rej_coef_off[thid + 1] = th_coefficient_coords.size();
        #pragma omp barrier
        #pragma omp single
        {
            rej_avg_base  = average_coords.size();
            rej_var_base  = variance_coords.size();
            rej_coef_base = coefficient_coords.size();
            for ( int t = 0; t < max_threads; ++t ) {
                rej_avg_off[t + 1]  += rej_avg_off[t];
                rej_var_off[t + 1]  += rej_var_off[t];
                rej_coef_off[t + 1] += rej_coef_off[t];
            }
            average_coords.resize( rej_avg_base + rej_avg_off[max_threads] );
            average_colors.resize( rej_avg_base + rej_avg_off[max_threads] );
            average_normals.resize( rej_avg_base + rej_avg_off[max_threads] );
            variance_coords.resize( rej_var_base + rej_var_off[max_threads] );
            variance_colors.resize( rej_var_base + rej_var_off[max_threads] );
            variance_normals.resize( rej_var_base + rej_var_off[max_threads] );
            coefficient_coords.resize( rej_coef_base + rej_coef_off[max_threads] );
            coefficient_colors.resize( rej_coef_base + rej_coef_off[max_threads] );
            coefficient_normals.resize( rej_coef_base + rej_coef_off[max_threads] );
        }
        {
            const size_t ao = rej_avg_base  + rej_avg_off[thid];
            const size_t vo = rej_var_base  + rej_var_off[thid];
            const size_t co = rej_coef_base + rej_coef_off[thid];
            std::copy( th_average_coords.begin(),  th_average_coords.end(),  average_coords.begin()  + ao );
            std::copy( th_average_colors.begin(),  th_average_colors.end(),  average_colors.begin()  + ao );
            std::copy( th_average_normals.begin(), th_average_normals.end(), average_normals.begin() + ao );
            std::copy( th_variance_coords.begin(),  th_variance_coords.end(),  variance_coords.begin()  + vo );
            std::copy( th_variance_colors.begin(),  th_variance_colors.end(),  variance_colors.begin()  + vo );
            std::copy( th_variance_normals.begin(), th_variance_normals.end(), variance_normals.begin() + vo );
            std::copy( th_coefficient_coords.begin(),  th_coefficient_coords.end(),  coefficient_coords.begin()  + co );
            std::copy( th_coefficient_colors.begin(),  th_coefficient_colors.end(),  coefficient_colors.begin()  + co );
            std::copy( th_coefficient_normals.begin(), th_coefficient_normals.end(), coefficient_normals.begin() + co );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        merge_timer.stop();
        th_rejection_merge_time += merge_timer.sec();
        thread_timer.stop();
        rejection_thread_times[thid] += thread_timer.sec();
        rejection_merge_times[thid] += th_rejection_merge_time;
#endif
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    rejection_timer.stop();
    ensemble_timer.add( EnsembleTimerOmpRejection, rejection_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpRejection, t, rejection_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerRejectionThreadMerge, t, rejection_merge_times[t] );
    }
    ensemble_timer.setStatisticParticleCounts(
        static_cast<unsigned long long>( average_coords.size() / 3 ),
        static_cast<unsigned long long>( variance_coords.size() / 3 ),
        static_cast<unsigned long long>( coefficient_coords.size() / 3 ) );
#endif
#else
    std::cout << "ensemble_generate_particles requires MPI; CPU_VER path is disabled." << std::endl;
    return false;
#endif


    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCleanupTfs );
#endif
    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;
    }

#ifndef CPU_VER
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCleanupCells );
#endif
    for ( int thread = 0; thread < max_threads; thread++ )
    {
        for ( int variable = 0; variable < nvariables; variable++ )
        {
            delete cell[thread][variable];
        }
        // 方式C の節点F場用補間器。使わなかった場合は NULL のまま。
        if ( cell_F[thread] ) { delete cell_F[thread]; cell_F[thread] = NULL; }
    }
    }
    average.coords.swap( average_coords );
    average.colors.swap( average_colors );
    average.normals.swap( average_normals );
    variance.coords.swap( variance_coords );
    variance.colors.swap( variance_colors );
    variance.normals.swap( variance_normals );
    coefficient.coords.swap( coefficient_coords );
    coefficient.colors.swap( coefficient_colors );
    coefficient.normals.swap( coefficient_normals );
#endif
    return true;
}


// ===== 構造格子版(v1: 非構造版複製ベース。格子固有箇所を順次置換) =====
bool GenerateEnsembleParticlesStruct(
    const int num_ensemble,
    ParticleProperty& particle_property,
    const domain_parameters_struct& dom,
    Type** values,
    int nvariables,
    EnsembleParticleArrays& average,
    EnsembleParticleArrays& variance,
    EnsembleParticleArrays& coefficient,
    EnsembleStatisticRange& average_range,
    EnsembleStatisticRange& variance_range,
    EnsembleStatisticRange& co_variation_range
#ifdef ENABLE_ENSEMBLE_TIMER
    , EnsembleTimerCollector* timer
#endif
)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif
    int tf_number = particle_property.m_transfunc_array.size();
#ifdef ENABLE_ENSEMBLE_TIMER
    EnsembleTimerCollector& ensemble_timer = *timer;
#endif

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
//    std::vector< std::vector<vismodule::TransferFunction> > th_tf;

    //std::vector<vismodule::TransferFunction> transfer_functions( tf_number );
    std::vector<std::vector<vismodule::TransferFunction>> transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>>           mean_transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>>       variance_transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>> coef_variation_transfer_functions( max_threads );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerInitTransferFunctions );
#endif
        for ( int n = 0; n < max_threads; n++ )
        {
            th_tfs[n] = new TransferFunctionSynthesizer( *particle_property.m_transfunc_synthesizer );
        }

        for ( int i = 0; i < max_threads; i++ )
        {
            transfer_functions[ i ].resize( tf_number );
                      mean_transfer_functions[ i ].resize( tf_number );
                  variance_transfer_functions[ i ].resize( tf_number );
            coef_variation_transfer_functions[ i ].resize( tf_number );
            for ( int j = 0; j < tf_number; j++ )
            {
                transfer_functions[i][j] = particle_property.m_transfunc_array[j];
                          mean_transfer_functions[i][j] = particle_property.m_mean_transfer_function_array[j];
                      variance_transfer_functions[i][j] = particle_property.m_variance_transfer_function_array[j];
                coef_variation_transfer_functions[i][j] = particle_property.m_coefficient_of_variation_transfer_function_array[j];
            }
        }
    }

    // アンサンブル用伝達関数のEquationTokenを取得 
    std::string expression = particle_property.m_mean_transfer_function_array[0].m_variable;
    const ::EquationToken equation_token = EnsembleTransferFunction::convert_token( expression );

//    std::cout << "particle_property.mean_max = " << particle_property.m_mean_transfer_function_array[0].colorMap().maxValue() << std::endl;
//    std::cout << "particle_property.var_max = " << particle_property.m_variance_transfer_function_array[0].colorMap().maxValue() << std::endl;
//    std::cout << "particle_property.cov_max = " << particle_property.m_coefficient_of_variation_transfer_function_array[0].colorMap().maxValue() << std::endl;



    std::vector<float> average_coords;
    std::vector<Byte> average_colors;
    std::vector<float> average_normals;
    std::vector<float> variance_coords;
    std::vector<Byte> variance_colors;
    std::vector<float> variance_normals;
    std::vector<float> coefficient_coords;
    std::vector<Byte> coefficient_colors;
    std::vector<float> coefficient_normals;

    std::vector<vismodule::UInt64> tmp_c_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<vismodule::UInt64> tmp_o_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<float> tmp_max( tf_number * 2, FLT_MIN );
    std::vector<float> tmp_min( tf_number * 2, FLT_MAX );

#ifndef CPU_VER
    if ( mpi_size <= 1 )
    {
        std::cout << "ensemble_generate_particles requires MPI ensemble ranks." << std::endl;
        return false;
    }

    // 構造格子: TrilinearInterpolator を各スレッド・各変数に構築(非構造版 cell 構築の置換)。
    // 格子単位座標で補間するため setCellLength(1)。物理座標は global=local*cell_length+min で別途算出。
    const vismodule::Vector3ui resolution(
        static_cast<unsigned int>( dom.resolution[0] ),
        static_cast<unsigned int>( dom.resolution[1] ),
        static_cast<unsigned int>( dom.resolution[2] ) );
    std::vector<std::vector<vismodule::TrilinearInterpolator*> > cell( max_threads );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCreateCells );
#endif
        for ( int thread = 0; thread < max_threads; thread++ )
        {
            cell[thread].resize( nvariables, nullptr );
            for ( int variable = 0; variable < nvariables; variable++ )
            {
                cell[thread][variable] = new vismodule::TrilinearInterpolator( values[variable], resolution );
                cell[thread][variable]->setCellLength( 1 );
            }
        }
    }

    float sampling_volume_inverse = 0.0f;
    float max_opacity = 0.0f;
    float max_density = 0.0f;
    float repetitions = particle_property.m_repeat_level;  //
    const float particle_density = 1.0f;
    const int MPIprocess_per_ensemble = mpi_size/num_ensemble;
    const int ens_number = num_ensemble;
    {  // 区間計測用の{}
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerSamplingPrepare );
#endif
        sampling_volume_inverse = particle_property.m_transfunc_synthesizer->getSamplingVolumeInverse();
        max_opacity = particle_property.m_transfunc_synthesizer->getMaxOpacity();
        max_density = particle_property.m_transfunc_synthesizer->getMaxDensity();
        if ( mpi_size % MPIprocess_per_ensemble != 0 )
        {
            std::cerr << "error !! need  ens_number % MPIprocess_per_ensemble = 0!!  " << std::endl;
            return false;
        } 
        repetitions /= static_cast<float>( ens_number );
    }
    // === 構造格子 サンプリング出力バッファ(cell_id は持たない) ===
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::Real32> vertex_scalars;
    std::vector<vismodule::Real32> vertex_normals;
    std::vector<vismodule::Real32> sq_scalars;
    std::vector<vismodule::Real32> tmp_term;

    // === 構造格子 一様サンプリング(非構造版 cellループ+リングの置換。v1: リングなし=各メンバ単独) ===
    // 各格子セルで max_density*cell_volume*repetitions 個の候補点を生成し、
    // TrilinearInterpolator で値・勾配を補間、chain rule 法線を求めて統計初期値(g, g^2, -grad, g*grad)を格納。
    const int nx_1 = static_cast<int>( dom.resolution[0] ) - 1;
    const int ny_1 = static_cast<int>( dom.resolution[1] ) - 1;
    const int nz_1 = static_cast<int>( dom.resolution[2] ) - 1;
    const long nxy_1 = static_cast<long>( nx_1 ) * ny_1;
    const size_t ncells_struct = static_cast<size_t>( nx_1 ) * ny_1 * nz_1;
    const float cell_length_f = dom.cell_length;
    const float cell_volume = cell_length_f * cell_length_f * cell_length_f;
    const vismodule::Vector3f min_vec( dom.x_global_min, dom.y_global_min, dom.z_global_min );
#pragma omp parallel
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
        const int nthreads = omp_get_num_threads();
#else
        const int thid = 0;
        const int nthreads = 1;
#endif
        ChainRuleEvalContext chain_context;
        chain_context.initialize( equation_token, nvariables );
        ChainRuleTimingBreakdown chain_rule_timing;
        vismodule::MersenneTwister mt( thid + mpi_rank * nthreads );

        std::vector<vismodule::Real32> th_vertex_coords;
        std::vector<vismodule::Real32> th_vertex_scalars;
        std::vector<vismodule::Real32> th_vertex_normals;
        std::vector<vismodule::Real32> th_sq_scalars;
        std::vector<vismodule::Real32> th_tmp_term;

        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
        float scalar_array[SIMD_BLK_SIZE];
        float grad_array_x[SIMD_BLK_SIZE];
        float grad_array_y[SIMD_BLK_SIZE];
        float grad_array_z[SIMD_BLK_SIZE];
        int nparticles_array[SIMD_BLK_SIZE];

#pragma omp for schedule( dynamic ) nowait
        for ( size_t index = 0; index < ncells_struct; index += SIMD_BLK_SIZE )
        {
            const int remain = ( ncells_struct - index > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : static_cast<int>( ncells_struct - index );
            for ( int cb = 0; cb < remain; cb++ )
            {
                nparticles_array[cb] = static_cast<int>(
                    CalculateNumberOfParticlesV35( max_density, cell_volume, repetitions, &mt ) );
            }
            int p_id = 0;
            for ( int cb = 0; cb < remain; cb++ )
            {
                const size_t cid = index + cb;
                const int k = static_cast<int>( cid / nxy_1 );
                const int j = static_cast<int>( ( cid - static_cast<size_t>( k ) * nxy_1 ) / nx_1 );
                const int i = static_cast<int>( cid - static_cast<size_t>( k ) * nxy_1 - static_cast<size_t>( j ) * nx_1 );
                for ( int p = 0; p < nparticles_array[cb]; p++ )
                {
                    const float rx = static_cast<float>( mt.rand() );
                    const float ry = static_cast<float>( mt.rand() );
                    const float rz = static_cast<float>( mt.rand() );
                    local_coord_array[p_id] = vismodule::Vector3f( i + rx, j + ry, k + rz );
                    global_coord_array[p_id] = vismodule::Vector3f(
                        ( i + rx ) * cell_length_f + min_vec.x(),
                        ( j + ry ) * cell_length_f + min_vec.y(),
                        ( k + rz ) * cell_length_f + min_vec.z() );
                    p_id++;
                    if ( p_id == SIMD_BLK_SIZE )
                    {
                        calculate_scalar_and_chain_rule_grad_struct(
                            p_id, nvariables, chain_context, cell[thid],
                            local_coord_array, global_coord_array,
                            scalar_array, grad_array_x, grad_array_y, grad_array_z, &chain_rule_timing );
                        const size_t so = th_vertex_scalars.size();
                        const size_t vo = th_vertex_coords.size();
                        th_vertex_scalars.resize( so + p_id );  th_sq_scalars.resize( so + p_id );
                        th_vertex_coords.resize( vo + 3 * p_id );  th_vertex_normals.resize( vo + 3 * p_id );  th_tmp_term.resize( vo + 3 * p_id );
                        store_uniform_block_struct( p_id, so, vo,
                            th_vertex_scalars, th_vertex_coords, th_vertex_normals, th_sq_scalars, th_tmp_term,
                            scalar_array, local_coord_array, grad_array_x, grad_array_y, grad_array_z );
                        p_id = 0;
                    }
                }
            }
            if ( p_id > 0 )
            {
                calculate_scalar_and_chain_rule_grad_struct(
                    p_id, nvariables, chain_context, cell[thid],
                    local_coord_array, global_coord_array,
                    scalar_array, grad_array_x, grad_array_y, grad_array_z, &chain_rule_timing );
                const size_t so = th_vertex_scalars.size();
                const size_t vo = th_vertex_coords.size();
                th_vertex_scalars.resize( so + p_id );  th_sq_scalars.resize( so + p_id );
                th_vertex_coords.resize( vo + 3 * p_id );  th_vertex_normals.resize( vo + 3 * p_id );  th_tmp_term.resize( vo + 3 * p_id );
                store_uniform_block_struct( p_id, so, vo,
                    th_vertex_scalars, th_vertex_coords, th_vertex_normals, th_sq_scalars, th_tmp_term,
                    scalar_array, local_coord_array, grad_array_x, grad_array_y, grad_array_z );
            }
        }
#pragma omp critical
        {
            vertex_scalars.insert( vertex_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end() );
            sq_scalars.insert( sq_scalars.end(), th_sq_scalars.begin(), th_sq_scalars.end() );
            vertex_coords.insert( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
            tmp_term.insert( tmp_term.end(), th_tmp_term.begin(), th_tmp_term.end() );
        }
    }

    // === v2: リング交換で全メンバ統計を集約(非構造版リングの struct 版) ===
    // 自分の候補点を v_*[0] に移し、shift ループで隣へ順送り。受信した候補点(格子座標)で
    // 自分のメンバの値を TrilinearInterpolator で再補間し統計に累積する。cell_id は不要。
    {
        std::vector< std::vector<vismodule::Real32> > v_scalars(2), v_coords(2), v_normals(2), v_sq(2), v_tmp(2);
        v_scalars[0].swap( vertex_scalars );
        v_coords[0].swap( vertex_coords );
        v_normals[0].swap( vertex_normals );
        v_sq[0].swap( sq_scalars );
        v_tmp[0].swap( tmp_term );
        int cur = 0, nxt = 1;
        for ( int shift = 1; shift < ens_number; shift++ )
        {
            const int send_to = ( mpi_rank + MPIprocess_per_ensemble ) % mpi_size;
            const int recv_from = ( mpi_rank - MPIprocess_per_ensemble + mpi_size ) % mpi_size;
            int send_size = static_cast<int>( v_scalars[cur].size() );
            int recv_size = 0;
            MPI_Sendrecv( &send_size, 1, MPI_INT, send_to, 0,
                          &recv_size, 1, MPI_INT, recv_from, 0,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE );
            std::vector<vismodule::Real32> recv_scalars( recv_size ), recv_coords( 3 * recv_size ),
                recv_normals( 3 * recv_size ), recv_sq_scalars( recv_size ), recv_tmp_term( 3 * recv_size );
            MPI_Request req[10];
            MPI_Irecv( recv_scalars.data(),    recv_size,     MPI_FLOAT, recv_from, 10, MPI_COMM_WORLD, &req[0] );
            MPI_Irecv( recv_coords.data(),     3 * recv_size, MPI_FLOAT, recv_from, 11, MPI_COMM_WORLD, &req[1] );
            MPI_Irecv( recv_normals.data(),    3 * recv_size, MPI_FLOAT, recv_from, 13, MPI_COMM_WORLD, &req[2] );
            MPI_Irecv( recv_sq_scalars.data(), recv_size,     MPI_FLOAT, recv_from, 14, MPI_COMM_WORLD, &req[3] );
            MPI_Irecv( recv_tmp_term.data(),   3 * recv_size, MPI_FLOAT, recv_from, 15, MPI_COMM_WORLD, &req[4] );
            MPI_Isend( v_scalars[cur].data(),  send_size,     MPI_FLOAT, send_to, 10, MPI_COMM_WORLD, &req[5] );
            MPI_Isend( v_coords[cur].data(),   3 * send_size, MPI_FLOAT, send_to, 11, MPI_COMM_WORLD, &req[6] );
            MPI_Isend( v_normals[cur].data(),  3 * send_size, MPI_FLOAT, send_to, 13, MPI_COMM_WORLD, &req[7] );
            MPI_Isend( v_sq[cur].data(),       send_size,     MPI_FLOAT, send_to, 14, MPI_COMM_WORLD, &req[8] );
            MPI_Isend( v_tmp[cur].data(),      3 * send_size, MPI_FLOAT, send_to, 15, MPI_COMM_WORLD, &req[9] );
            MPI_Waitall( 10, req, MPI_STATUSES_IGNORE );
            const int rn = recv_size;
            float* R_scal = recv_scalars.data();
            float* R_norm = recv_normals.data();
            float* R_sq = recv_sq_scalars.data();
            float* R_tmp = recv_tmp_term.data();
            const float* R_coor = recv_coords.data();
#pragma omp parallel
            {
#if _OPENMP
                const int thid = omp_get_thread_num();
#else
                const int thid = 0;
#endif
                ChainRuleEvalContext chain_context;
                chain_context.initialize( equation_token, nvariables );
                ChainRuleTimingBreakdown chain_rule_timing;
                vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
                vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
                float scalar_array[SIMD_BLK_SIZE];
                float grad_array_x[SIMD_BLK_SIZE];
                float grad_array_y[SIMD_BLK_SIZE];
                float grad_array_z[SIMD_BLK_SIZE];
#pragma omp for
                for ( int i = 0; i < rn; i += SIMD_BLK_SIZE )
                {
                    const int remain_BLK = ( rn - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : rn - i;
                    for ( int j = 0; j < remain_BLK; j++ )
                    {
                        const vismodule::Vector3f lc( R_coor[3*(i+j)], R_coor[3*(i+j)+1], R_coor[3*(i+j)+2] );
                        local_coord_array[j] = lc;
                        global_coord_array[j] = vismodule::Vector3f(
                            lc.x() * cell_length_f + min_vec.x(),
                            lc.y() * cell_length_f + min_vec.y(),
                            lc.z() * cell_length_f + min_vec.z() );
                    }
                    calculate_scalar_and_chain_rule_grad_struct(
                        remain_BLK, nvariables, chain_context, cell[thid],
                        local_coord_array, global_coord_array,
                        scalar_array, grad_array_x, grad_array_y, grad_array_z, &chain_rule_timing );
                    for ( int j = 0; j < remain_BLK; j++ )
                    {
                        const float scalar = scalar_array[j];
                        R_scal[i + j] += scalar;
                        R_norm[3*(i+j)]     += -grad_array_x[j];
                        R_norm[3*(i+j) + 1] += -grad_array_y[j];
                        R_norm[3*(i+j) + 2] += -grad_array_z[j];
                        R_sq[i + j] += scalar * scalar;
                        R_tmp[3*(i+j)]     += scalar * grad_array_x[j];
                        R_tmp[3*(i+j) + 1] += scalar * grad_array_y[j];
                        R_tmp[3*(i+j) + 2] += scalar * grad_array_z[j];
                    }
                }
            }
            v_scalars[nxt].swap( recv_scalars );
            v_coords[nxt].swap( recv_coords );
            v_normals[nxt].swap( recv_normals );
            v_sq[nxt].swap( recv_sq_scalars );
            v_tmp[nxt].swap( recv_tmp_term );
            std::swap( cur, nxt );
        }
        vertex_scalars.swap( v_scalars[cur] );
        vertex_coords.swap( v_coords[cur] );
        vertex_normals.swap( v_normals[cur] );
        sq_scalars.swap( v_sq[cur] );
        tmp_term.swap( v_tmp[cur] );
    }

    // 統計計算用の中間バッファ(関数スコープ: 統計ブロック後の histogram/棄却でも使うため)
    std::vector<vismodule::Real32> tmp_varience( vertex_scalars.size() );
    std::vector<vismodule::Real32> tmp_varience_normals( vertex_normals.size() );
    std::vector<vismodule::Real32> co_varietion( vertex_scalars.size() );
    std::vector<vismodule::Real32> co_varietion_normals( vertex_normals.size() );
    size_t co_varietion_normal_fallback_count = 0;
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatAverageVariance );
#endif
    const float invert_num = 1.0f / static_cast<float>( ens_number );
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        vertex_scalars[i] *= invert_num;
        sq_scalars[i] *= invert_num;
    }
    for ( size_t i = 0; i < tmp_term.size(); i++ )
    {
        tmp_term[i] = -2.0f * invert_num * tmp_term[i];
        vertex_normals[i] *= -invert_num;
    }
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        tmp_varience[i] = sq_scalars[i] - vertex_scalars[i] * vertex_scalars[i];
        if ( tmp_varience[i] < 0.0f ) tmp_varience[i] = 0.0f;
        tmp_varience_normals[3 * i] = tmp_term[3 * i] - ( -2.0f  * vertex_scalars[i] * vertex_normals[3 * i] );
        tmp_varience_normals[3 * i + 1] = tmp_term[3 * i + 1] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 1] );
        tmp_varience_normals[3 * i + 2] = tmp_term[3 * i + 2] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 2] );
    }

    const float delta = 1.0e-30f;
    const float eps = 1.0e-5f;
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        co_varietion[i] = std::fabs(vertex_scalars[i]) > eps ? std::sqrt( tmp_varience[i] ) /std::fabs( vertex_scalars[i]) : delta;
    }

    // 法線の構成は co_varietion の計算ループとは別ループにする。
    // 同一ループへ追記するとベクトル化/FP縮約の判断が変わり co_varietion が最下位ビットで
    // 変動しうる(実測: 色マップの量子化境界で 1 粒子の色が 1 階調ずれた)。分離すれば
    // co_varietion の生成コードが元のままとなり、粒子の色は完全に一致する。
    // 符号規約(実測): vertex_normals=+grad mu / tmp_varience_normals=-grad Var（両者は規約が逆）。
    // 展開: co_varietion_normals[j] = tmp_varience_normals[j] + vertex_normals[j] * s_i
    //   s_i = 2*Var/mu = 2*co_varietion[i]^2*mu_i （|mu|>eps。co_varietion=sqrt(Var)/|mu| より除算を乗算化=A-1）
    //       = 0 （|mu|<=eps: CoV 未定義 → 分散法線を流用する従来動作。s_i=0 で varn+vn*0=varn と厳密一致）
    // フォールバックは s_i=0 でブランチレス化(A-2)。法線は正規化され方向のみ有意のため OpenMP+SIMD/FMA で
    // 構成する(A-3/B-1)。co_varietion の"値"は本ループでは読むだけで不変(別ループ)ゆえ色・座標は従来と一致。
    // ComputeCoVNormalDirection(ChainRuleNormal.h)と代数的に等価: 2*co_varietion^2*mu = 2*Var/mu。
    {
        const size_t            nvert = vertex_scalars.size();
        const float* __restrict mu    = vertex_scalars.data();       // 平均 mu
        const float* __restrict cov   = co_varietion.data();         // 算出済み CoV 値(直前ループ)
        const float* __restrict vn    = vertex_normals.data();       // +grad mu
        const float* __restrict varn  = tmp_varience_normals.data(); // -grad Var
        float*       __restrict covn  = co_varietion_normals.data();
        size_t fb = 0;
        #pragma omp parallel for simd reduction(+:fb) schedule(static)
        for ( size_t i = 0; i < nvert; i++ )
        {
            const bool   ok = std::fabs( mu[i] ) > eps;              // NaN も偽=フォールバック
            const float  si = ok ? ( 2.0f * cov[i] * cov[i] * mu[i] ) : 0.0f;
            const size_t b  = 3 * i;
//            covn[b    ] = varn[b    ] + vn[b    ] * si;
//            covn[b + 1] = varn[b + 1] + vn[b + 1] * si;
//            covn[b + 2] = varn[b + 2] + vn[b + 2] * si;
            covn[b    ] = varn[b    ] - vn[b    ] * si;
            covn[b + 1] = varn[b + 1] - vn[b + 1] * si;
            covn[b + 2] = varn[b + 2] - vn[b + 2] * si;
            fb += ok ? 0u : 1u;
        }
        co_varietion_normal_fallback_count = fb;
    }
    }

    fprintf( stdout, "[cov_normal] rank=%d fallback=%llu / particles=%llu\n",
             mpi_rank,
             static_cast<unsigned long long>( co_varietion_normal_fallback_count ),
             static_cast<unsigned long long>( vertex_scalars.size() ) );

    // 統計量の対数化: フラグ ON なら 平均(vertex_scalars)・分散(tmp_varience)を log10。
    // 変動係数(co_varietion)・変動係数法線は上で算出済み＝対数化前。法線/最適化ループは不変。
    if ( particle_property.m_log_scale_statistics )
        ApplyLog10ToMeanVariance( vertex_scalars, tmp_varience );

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatHistogram );
#endif
        average_range = MakeEnsembleStatisticMinMax( vertex_scalars, tf_number );
        variance_range = MakeEnsembleStatisticMinMax( tmp_varience, tf_number );
        co_variation_range = MakeEnsembleStatisticMinMax( co_varietion, tf_number );

        AggregateEnsembleStatisticMinMax( average_range, tf_number, MPI_COMM_WORLD );
        AggregateEnsembleStatisticMinMax( variance_range, tf_number, MPI_COMM_WORLD );
        AggregateEnsembleStatisticMinMax( co_variation_range, tf_number, MPI_COMM_WORLD );

        ApplyEnsembleStatisticMinMax(
            average_range, particle_property.m_mean_transfer_function_array, tf_number );
        ApplyEnsembleStatisticMinMax(
            variance_range, particle_property.m_variance_transfer_function_array, tf_number );
        ApplyEnsembleStatisticMinMax(
            co_variation_range,
            particle_property.m_coefficient_of_variation_transfer_function_array,
            tf_number );

        for ( int thread = 0; thread < max_threads; thread++ )
        {
            for ( int i = 0; i < tf_number; i++ )
            {
                mean_transfer_functions[thread][i] = particle_property.m_mean_transfer_function_array[i];
                variance_transfer_functions[thread][i] = particle_property.m_variance_transfer_function_array[i];
                coef_variation_transfer_functions[thread][i] =
                    particle_property.m_coefficient_of_variation_transfer_function_array[i];
            }
        }

        CalculateEnsembleStatisticHistogram(
            average_range,
            vertex_scalars,
            tf_number,
            particle_property.m_mean_transfer_function_array );
        CalculateEnsembleStatisticHistogram(
            variance_range,
            tmp_varience,
            tf_number,
            particle_property.m_variance_transfer_function_array );
        CalculateEnsembleStatisticHistogram(
            co_variation_range,
            co_varietion,
            tf_number,
            particle_property.m_coefficient_of_variation_transfer_function_array );

        particle_property.m_transfunc_synthesizer->m_o_min.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_o_max.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_c_min.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_c_max.resize( tf_number );
        for ( int i = 0; i < tf_number; i++ )
        {
            particle_property.m_transfunc_synthesizer->m_o_min[i] = average_range.min_values[2 * i    ];
            particle_property.m_transfunc_synthesizer->m_o_max[i] = average_range.max_values[2 * i    ];
            particle_property.m_transfunc_synthesizer->m_c_min[i] = average_range.min_values[2 * i + 1];
            particle_property.m_transfunc_synthesizer->m_c_max[i] = average_range.max_values[2 * i + 1];
        }
    }

#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> rejection_thread_times( max_threads, 0.0 );
    std::vector<double> rejection_merge_times( max_threads, 0.0 );
    vismodule::Timer rejection_timer;
    rejection_timer.start();
#endif
    std::vector<size_t> rej_avg_off( max_threads + 1, 0 );
    std::vector<size_t> rej_var_off( max_threads + 1, 0 );
    std::vector<size_t> rej_coef_off( max_threads + 1, 0 );
    size_t rej_avg_base = 0, rej_var_base = 0, rej_coef_base = 0;
#pragma omp parallel
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
#else
        const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer thread_timer;
        thread_timer.start();
        double th_rejection_merge_time = 0.0;
#endif
        std::vector<float> th_average_coords;
        std::vector<Byte> th_average_colors;
        std::vector<float> th_average_normals;
        std::vector<float> th_variance_coords;
        std::vector<Byte> th_variance_colors;
        std::vector<float> th_variance_normals;
        std::vector<float> th_coefficient_coords;
        std::vector<Byte> th_coefficient_colors;
        std::vector<float> th_coefficient_normals;
        vismodule::MersenneTwister mt( 10 + mpi_rank + thid );
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];

#pragma omp for schedule( dynamic )
        for ( size_t i = 0; i < vertex_scalars.size(); i += SIMD_BLK_SIZE )
        {
            const int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : vertex_scalars.size() - i;
            for ( int j = 0; j < remain_BLK; j++ )
            {
                const vismodule::Vector3f lc(
                    vertex_coords[3 * ( i + j )],
                    vertex_coords[3 * ( i + j ) + 1],
                    vertex_coords[3 * ( i + j ) + 2] );
                local_coord_array[j] = lc;
                // 構造格子: 大域座標 = 格子座標 * cell_length + 原点(transformLocalToGlobal の置換)
                global_coord_array[j] = vismodule::Vector3f(
                    lc.x() * cell_length_f + min_vec.x(),
                    lc.y() * cell_length_f + min_vec.y(),
                    lc.z() * cell_length_f + min_vec.z() );
            }

            for ( int j = 0; j < remain_BLK; j++ )
            {
                const size_t idx = i + j;
                const vismodule::Vector3f average_normal(
                    -vertex_normals[3 * idx],
                    -vertex_normals[3 * idx + 1],
                    -vertex_normals[3 * idx + 2]
                );
                const vismodule::Vector3f variance_normal(
                    tmp_varience_normals[3 * idx],
                    tmp_varience_normals[3 * idx + 1],
                    tmp_varience_normals[3 * idx + 2]
                );
                const vismodule::Vector3f coefficient_normal(
                    co_varietion_normals[3 * idx],
                    co_varietion_normals[3 * idx + 1],
                    co_varietion_normals[3 * idx + 2]
                );

//                std::cout << "tmp_varience_normals = " << variance_normal <<std::endl;
                AppendRejectedStatisticParticle(
                    vertex_scalars[idx], global_coord_array[j], average_normal, mean_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_average_coords, th_average_colors, th_average_normals
                );
                AppendRejectedStatisticParticle(
                    tmp_varience[idx], global_coord_array[j], variance_normal, variance_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_variance_coords, th_variance_colors, th_variance_normals
                );
                AppendRejectedStatisticParticle(
                    co_varietion[idx], global_coord_array[j], coefficient_normal, coef_variation_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_coefficient_coords, th_coefficient_colors, th_coefficient_normals
                );
            }
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer merge_timer;
        merge_timer.start();
#endif
        // Prefix-sum parallel merge per statistic type (average/variance/coefficient have
        // independent accepted counts; within a type coords/colors/normals share the count).
        rej_avg_off[thid + 1]  = th_average_coords.size();
        rej_var_off[thid + 1]  = th_variance_coords.size();
        rej_coef_off[thid + 1] = th_coefficient_coords.size();
        #pragma omp barrier
        #pragma omp single
        {
            rej_avg_base  = average_coords.size();
            rej_var_base  = variance_coords.size();
            rej_coef_base = coefficient_coords.size();
            for ( int t = 0; t < max_threads; ++t ) {
                rej_avg_off[t + 1]  += rej_avg_off[t];
                rej_var_off[t + 1]  += rej_var_off[t];
                rej_coef_off[t + 1] += rej_coef_off[t];
            }
            average_coords.resize( rej_avg_base + rej_avg_off[max_threads] );
            average_colors.resize( rej_avg_base + rej_avg_off[max_threads] );
            average_normals.resize( rej_avg_base + rej_avg_off[max_threads] );
            variance_coords.resize( rej_var_base + rej_var_off[max_threads] );
            variance_colors.resize( rej_var_base + rej_var_off[max_threads] );
            variance_normals.resize( rej_var_base + rej_var_off[max_threads] );
            coefficient_coords.resize( rej_coef_base + rej_coef_off[max_threads] );
            coefficient_colors.resize( rej_coef_base + rej_coef_off[max_threads] );
            coefficient_normals.resize( rej_coef_base + rej_coef_off[max_threads] );
        }
        {
            const size_t ao = rej_avg_base  + rej_avg_off[thid];
            const size_t vo = rej_var_base  + rej_var_off[thid];
            const size_t co = rej_coef_base + rej_coef_off[thid];
            std::copy( th_average_coords.begin(),  th_average_coords.end(),  average_coords.begin()  + ao );
            std::copy( th_average_colors.begin(),  th_average_colors.end(),  average_colors.begin()  + ao );
            std::copy( th_average_normals.begin(), th_average_normals.end(), average_normals.begin() + ao );
            std::copy( th_variance_coords.begin(),  th_variance_coords.end(),  variance_coords.begin()  + vo );
            std::copy( th_variance_colors.begin(),  th_variance_colors.end(),  variance_colors.begin()  + vo );
            std::copy( th_variance_normals.begin(), th_variance_normals.end(), variance_normals.begin() + vo );
            std::copy( th_coefficient_coords.begin(),  th_coefficient_coords.end(),  coefficient_coords.begin()  + co );
            std::copy( th_coefficient_colors.begin(),  th_coefficient_colors.end(),  coefficient_colors.begin()  + co );
            std::copy( th_coefficient_normals.begin(), th_coefficient_normals.end(), coefficient_normals.begin() + co );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        merge_timer.stop();
        th_rejection_merge_time += merge_timer.sec();
        thread_timer.stop();
        rejection_thread_times[thid] += thread_timer.sec();
        rejection_merge_times[thid] += th_rejection_merge_time;
#endif
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    rejection_timer.stop();
    ensemble_timer.add( EnsembleTimerOmpRejection, rejection_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpRejection, t, rejection_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerRejectionThreadMerge, t, rejection_merge_times[t] );
    }
    ensemble_timer.setStatisticParticleCounts(
        static_cast<unsigned long long>( average_coords.size() / 3 ),
        static_cast<unsigned long long>( variance_coords.size() / 3 ),
        static_cast<unsigned long long>( coefficient_coords.size() / 3 ) );
#endif
#else
    std::cout << "ensemble_generate_particles requires MPI; CPU_VER path is disabled." << std::endl;
    return false;
#endif


    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCleanupTfs );
#endif
    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;
    }

#ifndef CPU_VER
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCleanupCells );
#endif
    for ( int thread = 0; thread < max_threads; thread++ )
    {
        for ( int variable = 0; variable < nvariables; variable++ )
        {
            delete cell[thread][variable];
        }
    }
    }
    average.coords.swap( average_coords );
    average.colors.swap( average_colors );
    average.normals.swap( average_normals );
    variance.coords.swap( variance_coords );
    variance.colors.swap( variance_colors );
    variance.normals.swap( variance_normals );
    coefficient.coords.swap( coefficient_coords );
    coefficient.colors.swap( coefficient_colors );
    coefficient.normals.swap( coefficient_normals );
#endif
    return true;
}


} // namespace vismodule
