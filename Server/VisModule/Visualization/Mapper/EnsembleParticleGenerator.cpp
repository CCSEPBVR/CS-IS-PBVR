
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
    alignas(64) float xa[SIMD_BLK_SIZE], ya[SIMD_BLK_SIZE], za[SIMD_BLK_SIZE];
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
    ctx.rpn.evalArraySIMD( scalar_out, n );          // F (unperturbed)
#ifdef ENABLE_ENSEMBLE_TIMER
    t_eval.stop(); if ( timing ) timing->tf_scalar_eval += t_eval.sec();
#endif

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
#if defined(PBVR_SHIFT_ALLOC_MEM) && !defined(CPU_VER)
// Option C: persistent grow-only MPI_Alloc_mem buffer for the ensemble shift exchange.
// MPI_Alloc_mem returns MPT-registered memory, so large messages take the correct (fast,
// non-corrupting) RDMA path instead of the pageable-host path that CUDA-aware MPT garbles.
// CUDA-independent. Grow-only + reused across iterations -> pinning cost amortized, and the
// per-iteration std::vector alloc/free is removed.
struct ShiftPinnedBuf {
    void*  ptr = nullptr;
    size_t cap = 0;
    void* get( size_t bytes ) {
        if ( bytes > cap ) {
            if ( ptr ) MPI_Free_mem( ptr );
            cap = bytes + bytes / 4;   // 25% headroom -> rare re-registration
            MPI_Alloc_mem( (MPI_Aint)cap, MPI_INFO_NULL, &ptr );
        }
        return ptr;
    }
};
#endif

#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
// Direct ring slot for the ensemble shift exchange (opt-in A/B variant, NOT the default).
// grow-only reuse across hops (no per-hop malloc/free, no staging copy); MPI transfers directly
// on these buffers. Enable with PBVR_SHIFT_ALLOC_MEM_DIRECT; add PBVR_SHIFT_PLAIN for plain
// malloc + static-persistent slots (no registered memory), else MPI_Alloc_mem (registered) +
// function-local per-timestep. The default churn-free fix is PBVR_SHIFT_STATIC_VEC (all-static
// std::vector + zero-copy swap), which is faster at low MPI; these paths are kept for comparison.
struct ShiftSlot {
    int*   cellids = nullptr; float* scalars = nullptr; float* coords = nullptr;
    float* normals = nullptr; float* sq      = nullptr; float* tmp    = nullptr;
    int    count = 0;  size_t cap = 0;
    static void* am( size_t b ) {
#ifdef PBVR_SHIFT_PLAIN
        return malloc( b );                          // plain(非ピン・登録なし): churn仮説の検証用
#else
        void* p = nullptr; MPI_Alloc_mem( (MPI_Aint)b, MPI_INFO_NULL, &p ); return p;
#endif
    }
    void free_all() {
        if ( !cellids ) return;
#ifdef PBVR_SHIFT_PLAIN
        free(cellids); free(scalars); free(coords); free(normals); free(sq); free(tmp);
#else
        MPI_Free_mem(cellids); MPI_Free_mem(scalars); MPI_Free_mem(coords);
        MPI_Free_mem(normals); MPI_Free_mem(sq);      MPI_Free_mem(tmp);
#endif
        cellids=nullptr; scalars=nullptr; coords=nullptr; normals=nullptr; sq=nullptr; tmp=nullptr; cap=0; count=0;
    }
    void ensure( int need ) {
        if ( (size_t)need <= cap ) return;
        free_all();
        cap = (size_t)need + need / 4;
        cellids = (int*)   am( cap * sizeof(int) );   scalars = (float*) am( cap * sizeof(float) );
        coords  = (float*) am( 3*cap*sizeof(float) ); normals = (float*) am( 3*cap*sizeof(float) );
        sq      = (float*) am( cap * sizeof(float) ); tmp     = (float*) am( 3*cap*sizeof(float) );
    }
    ShiftSlot() = default;
    ~ShiftSlot() { free_all(); }
    ShiftSlot( const ShiftSlot& ) = delete;
    ShiftSlot& operator=( const ShiftSlot& ) = delete;
};
#endif

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
// bit-identical. Build -DPBVR_SCALAR_STORE to restore the old scalar loop.
static inline void store_uniform_block(
    const int n, const size_t scalar_offset, const size_t vector_offset,
    std::vector<vismodule::Real32>& vs, std::vector<vismodule::Real32>& vc,
    std::vector<vismodule::Real32>& vn, std::vector<int>& vi,
    std::vector<vismodule::Real32>& vsq, std::vector<vismodule::Real32>& vt,
    const float* scalar_array, const vismodule::Vector3f* local_coord_array,
    const float* grad_array_x, const float* grad_array_y, const float* grad_array_z,
    const vismodule::UInt32* cell_index )
{
#ifndef PBVR_SCALAR_STORE
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
#else
    for ( int k = 0; k < n; k++ )
    {
        const size_t s = scalar_offset + static_cast<size_t>( k );
        const size_t v = vector_offset + 3 * static_cast<size_t>( k );
        vs[s] = scalar_array[k];
        vc[v] = local_coord_array[k].x();
        vc[v + 1] = local_coord_array[k].y();
        vc[v + 2] = local_coord_array[k].z();
        vi[s] = cell_index[k];
        vn[v] = -grad_array_x[k];
        vn[v + 1] = -grad_array_y[k];
        vn[v + 2] = -grad_array_z[k];
        vsq[s] = scalar_array[k] * scalar_array[k];
        vt[v] = scalar_array[k] * grad_array_x[k];
        vt[v + 1] = scalar_array[k] * grad_array_y[k];
        vt[v + 2] = scalar_array[k] * grad_array_z[k];
    }
#endif
}

// Philox-4x32-10 counter-based RNG (clean-room from Salmon et al. 2011). Stateless: each
// output is a pure function of (key, counter), so a batch of particles vectorizes. Only the
// integer core is used (no __uint128_t / intrinsics) -> portable to gcc and icpc.
// Enabled by PBVR_PHILOX_RNG; default keeps MersenneTwister. Key = mpi_rank (thread-independent
// -> reproducible even at OMP>1), counter = (cell_id, within-cell index).
#define PBVR_PHILOX_ROUND \
        const uint64_t p0 = (uint64_t)0xD2511F53u * c0; \
        const uint64_t p1 = (uint64_t)0xCD9E8D57u * c2; \
        const uint32_t a0 = (uint32_t)( p1 >> 32 ) ^ c1 ^ kk0; \
        const uint32_t a1 = (uint32_t)p1; \
        const uint32_t a2 = (uint32_t)( p0 >> 32 ) ^ c3 ^ kk1; \
        const uint32_t a3 = (uint32_t)p0; \
        c0 = a0; c1 = a1; c2 = a2; c3 = a3; \
        kk0 += 0x9E3779B9u; kk1 += 0xBB67AE85u
static inline void philox_coords( const int n, vismodule::Vector3f* out,
        const uint32_t k0, const uint32_t k1, const uint32_t cell_id, const uint32_t base_j )
{
    const float INV24 = 1.0f / 16777216.0f;
    #pragma omp simd
    for ( int t = 0; t < n; t++ )
    {
        uint32_t c0 = cell_id, c1 = base_j + (uint32_t)t, c2 = 0u, c3 = 0u;
        uint32_t kk0 = k0, kk1 = k1;
        for ( int r = 0; r < 10; r++ ) { PBVR_PHILOX_ROUND; }
        out[t] = vismodule::Vector3f( (float)( c0 >> 8 ) * INV24,
                                      (float)( c1 >> 8 ) * INV24,
                                      (float)( c2 >> 8 ) * INV24 );
    }
}
static inline float philox_count_rand( const uint32_t k0, const uint32_t k1, const uint32_t cell_id )
{
    uint32_t c0 = cell_id, c1 = 0xFFFFFFFFu, c2 = 0u, c3 = 0u;
    uint32_t kk0 = k0, kk1 = k1;
    for ( int r = 0; r < 10; r++ ) { PBVR_PHILOX_ROUND; }
    return (float)( c0 >> 8 ) * ( 1.0f / 16777216.0f );
}
static inline size_t CalculateNumberOfParticlesV35_philox(
        const float density, const float volume_of_cell, const float repetition,
        const uint32_t k0, const uint32_t k1, const uint32_t cell_id )
{
    const float n_particles = density * volume_of_cell * repetition;
    const float random = philox_count_rand( k0, k1, cell_id );
    size_t n = static_cast<size_t>( n_particles );
    if ( n_particles - n > random ) ++n;
    return n;
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
#ifdef PBVR_SHIFT_STATIC_VEC   // サンプリング出力(vertex_*)も static 永続化し、MPIが触る全バッファを churn-free に
    static std::vector<vismodule::Real32> vertex_coords;
    static std::vector<vismodule::Real32> vertex_scalars;
    static std::vector<vismodule::Real32> vertex_normals;
    static std::vector<int> vertex_cellids;
    static std::vector<vismodule::Real32> sq_scalars;
    static std::vector<vismodule::Real32> tmp_term;
    // static ゆえ前 timestep のデータが残る。merge は size() 起点で追記するため毎回空へリセット
    // (clear は capacity を保持 = realloc 無 = grow-only)。
    vertex_coords.clear(); vertex_scalars.clear(); vertex_normals.clear();
    vertex_cellids.clear(); sq_scalars.clear(); tmp_term.clear();
#else
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::Real32> vertex_scalars;
    std::vector<vismodule::Real32> vertex_normals;
    std::vector<int> vertex_cellids;
    std::vector<vismodule::Real32> sq_scalars;
    std::vector<vismodule::Real32> tmp_term;
#endif



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
#ifndef PBVR_SERIAL_MERGE
    std::vector<size_t> merge_off( max_threads + 1, 0 );
    size_t merge_base_s = 0, merge_base_v = 0;
#endif
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
        const uint32_t philox_key0 = static_cast<uint32_t>( mpi_rank );
        const uint32_t philox_key1 = 0xD2B79A53u;
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
#if defined( ENABLE_HEX_TET_VOLUME )
            // Candidate 1: the cells were already loaded by bindCellArray() above, so
            // reuse m_vertices_array and compute every volume with one vectorizable call
            // instead of a per-cell virtual bindCell() (which blocked vectorization,
            // #15333) plus a redundant gather. Bit-identical (same 6-tet decomposition
            // and vertex order).
            static_cast<vismodule::HexahedralCell<Type>*>( cell[thid][0] )
                ->volumeArrayByTetraDecomposition( remain, volume_array );
#else
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell[thid][0]->bindCell( cell_index[cell_BLK] );
#if defined( ENABLE_HEX_TET_VOLUME )
                    volume_array[cell_BLK] =
                        static_cast<vismodule::HexahedralCell<Type>*>( cell[thid][0] )->volumeByTetraDecomposition();
#else
                volume_array[cell_BLK] = cell[thid][0]->volume();
#endif
            }
#endif
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
#ifdef PBVR_PHILOX_RNG
                nparticles_array[cell_BLK] = static_cast<int>(
                    CalculateNumberOfParticlesV35_philox( max_density, volume_array[cell_BLK], repetitions,
                        philox_key0, philox_key1, static_cast<uint32_t>( index + cell_BLK ) ) );
#else
                nparticles_array[cell_BLK] = static_cast<int>(
                    CalculateNumberOfParticlesV35( max_density, volume_array[cell_BLK], repetitions, &mt )
//                    5
                );
#endif
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
#ifdef PBVR_PHILOX_RNG
                            philox_coords( take, &local_coord_array[p_id], philox_key0, philox_key1,
                                           static_cast<uint32_t>( index + cell_BLK ), static_cast<uint32_t>( j ) );
#else
                            for ( int t = 0; t < take; t++ )
                                local_coord_array[p_id + t] = cell[thid][0]->randomSampling_MT( &mt );
#endif
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
	                                calculate_scalar_and_chain_rule_grad(
	                                    p_id,
	                                    nvariables,
	                                    chain_context,
	                                    cell[thid],
	                                    local_coord_array,
	                                    global_coord_array,
	                                    cell_index,
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
	                        calculate_scalar_and_chain_rule_grad(
	                            p_id,
	                            nvariables,
	                            chain_context,
	                            cell[thid],
	                            local_coord_array,
	                            global_coord_array,
	                            cell_index,
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
                calculate_scalar_and_chain_rule_grad( m, nvariables, chain_context, cell[thid],
                    fl, fg, fc, scalar_array, grad_array_x, grad_array_y, grad_array_z, 0 );
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
#ifdef PBVR_SERIAL_MERGE
#pragma omp critical
        {
            vertex_coords.reserve( vertex_coords.size() + th_vertex_coords.size() );
            vertex_scalars.reserve( vertex_scalars.size() + th_vertex_scalars.size() );
            vertex_normals.reserve( vertex_normals.size() + th_vertex_normals.size() );
            vertex_cellids.reserve( vertex_cellids.size() + th_vertex_cellids.size() );
            sq_scalars.reserve( sq_scalars.size() + th_sq_scalars.size() );
            tmp_term.reserve( tmp_term.size() + th_tmp_term.size() );
            vertex_coords.insert( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_scalars.insert( vertex_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
            vertex_cellids.insert( vertex_cellids.end(), th_vertex_cellids.begin(), th_vertex_cellids.end() );
            sq_scalars.insert( sq_scalars.end(), th_sq_scalars.begin(), th_sq_scalars.end() );
            tmp_term.insert( tmp_term.end(), th_tmp_term.begin(), th_tmp_term.end() );
        }
#else
        // Prefix-sum parallel merge: each thread records its particle count, one thread
        // computes the offsets and resizes the global arrays once, then every thread copies
        // its buffer to its own disjoint offset in parallel (no omp critical -> no
        // serialization). Particle order becomes thread-id order (was non-deterministic
        // critical order); invariants are preserved. PBVR_SERIAL_MERGE restores the old path.
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
#endif
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

#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
#ifdef PBVR_SHIFT_PLAIN
    static ShiftSlot slot0, slot1;   // static永続=churn完全撤廃(plainなのでleak可・MPI非依存)
#else
    ShiftSlot slot0, slot1;
#endif
    ShiftSlot* cur = &slot0;
    ShiftSlot* nxt = &slot1;
    { const int n = (int)vertex_scalars.size(); cur->ensure( n ); cur->count = n;
      std::memcpy( cur->cellids, vertex_cellids.data(), sizeof(int)   * n );
      std::memcpy( cur->scalars, vertex_scalars.data(), sizeof(float) * n );
      std::memcpy( cur->coords,  vertex_coords.data(),  sizeof(float) * 3 * n );
      std::memcpy( cur->normals, vertex_normals.data(), sizeof(float) * 3 * n );
      std::memcpy( cur->sq,      sq_scalars.data(),     sizeof(float) * n );
      std::memcpy( cur->tmp,     tmp_term.data(),       sizeof(float) * 3 * n ); }
#else
#ifdef PBVR_SHIFT_STATIC_VEC   // shift ring バッファ(v_*/recv_*)を static 永続+grow-only 化: swap 回転そのまま=コピー無/churn無
    static std::vector<std::vector<float> > v_scalars( 2 );
    static std::vector<std::vector<float> > v_coords( 2 );
    static std::vector<std::vector<float> > v_normals( 2 );
    static std::vector<std::vector<int> > v_cellids( 2 );
    static std::vector<std::vector<float> > v_sq( 2 );
    static std::vector<std::vector<float> > v_tmp( 2 );
#else
    std::vector<std::vector<float> > v_scalars( 2 );
    std::vector<std::vector<float> > v_coords( 2 );
    std::vector<std::vector<float> > v_normals( 2 );
    std::vector<std::vector<int> > v_cellids( 2 );
    std::vector<std::vector<float> > v_sq( 2 );
    std::vector<std::vector<float> > v_tmp( 2 );
#endif
    v_scalars[0].swap( vertex_scalars );
    v_coords[0].swap( vertex_coords );
    v_normals[0].swap( vertex_normals );
    v_cellids[0].swap( vertex_cellids );
    v_sq[0].swap( sq_scalars );
    v_tmp[0].swap( tmp_term );

    int cur = 0;
    int nxt = 1;
#endif
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
#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
        const int send_size = cur->count;
#else
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
#endif
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
#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
        nxt->ensure( recv_size ); nxt->count = recv_size;
#else
#ifdef PBVR_SHIFT_STATIC_VEC   // 永続+grow-only: 毎hopの確保/解放を撤廃(resize は capacity 以下なら realloc 無)
        static std::vector<float> recv_scalars;    recv_scalars.resize( recv_size );
        static std::vector<float> recv_coords;     recv_coords.resize( 3 * recv_size );
        static std::vector<float> recv_normals;    recv_normals.resize( 3 * recv_size );
        static std::vector<int>   recv_cellids;    recv_cellids.resize( recv_size );
        static std::vector<float> recv_sq_scalars; recv_sq_scalars.resize( recv_size );
        static std::vector<float> recv_tmp_term;   recv_tmp_term.resize( 3 * recv_size );
#else
        std::vector<float> recv_scalars( recv_size );
        std::vector<float> recv_coords( 3 * recv_size );
        std::vector<float> recv_normals( 3 * recv_size );
        std::vector<int> recv_cellids( recv_size );
        std::vector<float> recv_sq_scalars( recv_size );
        std::vector<float> recv_tmp_term( 3 * recv_size );
#endif
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
        alloc_timer.stop();
        ensemble_timer.add( EnsembleTimerMpiShiftAllocRecvBuffer, alloc_timer.sec() );
#endif
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer payload_all_timer;
            payload_all_timer.start();
#endif
#if !defined(PBVR_SHIFT_ALLOC_MEM) || defined(CPU_VER)
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
#else  // ---- Option C: MPI_Alloc_mem (registered) staging path; existing path kept above ----
            static ShiftPinnedBuf sp[12];   // 0..5 = send cellids/scalars/coords/normals/sq/tmp, 6..11 = recv
            int*   ps_cell = (int*)   sp[0].get( sizeof(int)   *     (size_t)send_size );
            float* ps_scal = (float*) sp[1].get( sizeof(float) *     (size_t)send_size );
            float* ps_coor = (float*) sp[2].get( sizeof(float) * 3 * (size_t)send_size );
            float* ps_norm = (float*) sp[3].get( sizeof(float) * 3 * (size_t)send_size );
            float* ps_sq   = (float*) sp[4].get( sizeof(float) *     (size_t)send_size );
            float* ps_tmp  = (float*) sp[5].get( sizeof(float) * 3 * (size_t)send_size );
            std::memcpy( ps_cell, v_cellids[cur].data(), sizeof(int)   *     (size_t)send_size );
            std::memcpy( ps_scal, v_scalars[cur].data(), sizeof(float) *     (size_t)send_size );
            std::memcpy( ps_coor, v_coords[cur].data(),  sizeof(float) * 3 * (size_t)send_size );
            std::memcpy( ps_norm, v_normals[cur].data(), sizeof(float) * 3 * (size_t)send_size );
            std::memcpy( ps_sq,   v_sq[cur].data(),      sizeof(float) *     (size_t)send_size );
            std::memcpy( ps_tmp,  v_tmp[cur].data(),     sizeof(float) * 3 * (size_t)send_size );
            int*   pr_cell = (int*)   sp[6].get(  sizeof(int)   *     (size_t)recv_size );
            float* pr_scal = (float*) sp[7].get(  sizeof(float) *     (size_t)recv_size );
            float* pr_coor = (float*) sp[8].get(  sizeof(float) * 3 * (size_t)recv_size );
            float* pr_norm = (float*) sp[9].get(  sizeof(float) * 3 * (size_t)recv_size );
            float* pr_sq   = (float*) sp[10].get( sizeof(float) *     (size_t)recv_size );
            float* pr_tmp  = (float*) sp[11].get( sizeof(float) * 3 * (size_t)recv_size );
            MPI_Request rq[12];
            MPI_Irecv( pr_cell, recv_size,     MPI_INT,   recv_from, 12, MPI_COMM_WORLD, &rq[0] );
            MPI_Irecv( pr_scal, recv_size,     MPI_FLOAT, recv_from, 10, MPI_COMM_WORLD, &rq[1] );
            MPI_Irecv( pr_coor, 3 * recv_size, MPI_FLOAT, recv_from, 11, MPI_COMM_WORLD, &rq[2] );
            MPI_Irecv( pr_norm, 3 * recv_size, MPI_FLOAT, recv_from, 13, MPI_COMM_WORLD, &rq[3] );
            MPI_Irecv( pr_sq,   recv_size,     MPI_FLOAT, recv_from, 14, MPI_COMM_WORLD, &rq[4] );
            MPI_Irecv( pr_tmp,  3 * recv_size, MPI_FLOAT, recv_from, 15, MPI_COMM_WORLD, &rq[5] );
            MPI_Isend( ps_cell, send_size,     MPI_INT,   send_to, 12, MPI_COMM_WORLD, &rq[6] );
            MPI_Isend( ps_scal, send_size,     MPI_FLOAT, send_to, 10, MPI_COMM_WORLD, &rq[7] );
            MPI_Isend( ps_coor, 3 * send_size, MPI_FLOAT, send_to, 11, MPI_COMM_WORLD, &rq[8] );
            MPI_Isend( ps_norm, 3 * send_size, MPI_FLOAT, send_to, 13, MPI_COMM_WORLD, &rq[9] );
            MPI_Isend( ps_sq,   send_size,     MPI_FLOAT, send_to, 14, MPI_COMM_WORLD, &rq[10] );
            MPI_Isend( ps_tmp,  3 * send_size, MPI_FLOAT, send_to, 15, MPI_COMM_WORLD, &rq[11] );
            MPI_Waitall( 12, rq, MPI_STATUSES_IGNORE );
            std::memcpy( recv_cellids.data(),    pr_cell, sizeof(int)   *     (size_t)recv_size );
            std::memcpy( recv_scalars.data(),    pr_scal, sizeof(float) *     (size_t)recv_size );
            std::memcpy( recv_coords.data(),     pr_coor, sizeof(float) * 3 * (size_t)recv_size );
            std::memcpy( recv_normals.data(),    pr_norm, sizeof(float) * 3 * (size_t)recv_size );
            std::memcpy( recv_sq_scalars.data(), pr_sq,   sizeof(float) *     (size_t)recv_size );
            std::memcpy( recv_tmp_term.data(),   pr_tmp,  sizeof(float) * 3 * (size_t)recv_size );
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_all_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadAll, payload_all_timer.sec() );
#endif
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer shift_interp_timer;
        shift_interp_timer.start();
#endif
#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
        const int rn = nxt->count;
        int* R_cell = nxt->cellids; float* R_coor = nxt->coords; float* R_scal = nxt->scalars;
        float* R_norm = nxt->normals; float* R_sq = nxt->sq; float* R_tmp = nxt->tmp;
#else
        const int rn = recv_size;
        int* R_cell = recv_cellids.data(); float* R_coor = recv_coords.data(); float* R_scal = recv_scalars.data();
        float* R_norm = recv_normals.data(); float* R_sq = recv_sq_scalars.data(); float* R_tmp = recv_tmp_term.data();
#endif
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
	                calculate_scalar_and_chain_rule_grad(
	                    remain_BLK,
	                    nvariables,
	                    chain_context,
	                    cell[thid],
	                    local_coord_array,
	                    global_coord_array,
	                    cell_index,
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

#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
        { ShiftSlot* t = cur; cur = nxt; nxt = t; }
#else
        v_scalars[nxt].swap( recv_scalars );
        v_coords[nxt].swap( recv_coords );
        v_normals[nxt].swap( recv_normals );
        v_cellids[nxt].swap( recv_cellids );
        v_sq[nxt].swap( recv_sq_scalars );
        v_tmp[nxt].swap( recv_tmp_term );
        std::swap( cur, nxt );
#endif
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

#ifdef PBVR_SHIFT_ALLOC_MEM_DIRECT
    { const int n = cur->count;
      vertex_cellids.resize(n); vertex_scalars.resize(n); sq_scalars.resize(n);
      vertex_coords.resize(3*n); vertex_normals.resize(3*n); tmp_term.resize(3*n);
      std::memcpy(vertex_cellids.data(), cur->cellids, sizeof(int)   * n);
      std::memcpy(vertex_scalars.data(), cur->scalars, sizeof(float) * n);
      std::memcpy(vertex_coords.data(),  cur->coords,  sizeof(float) * 3 * n);
      std::memcpy(vertex_normals.data(), cur->normals, sizeof(float) * 3 * n);
      std::memcpy(sq_scalars.data(),     cur->sq,      sizeof(float) * n);
      std::memcpy(tmp_term.data(),       cur->tmp,     sizeof(float) * 3 * n); }
#else
    vertex_scalars.swap( v_scalars[cur] );
    vertex_coords.swap( v_coords[cur] );
    vertex_normals.swap( v_normals[cur] );
    vertex_cellids.swap( v_cellids[cur] );
    sq_scalars.swap( v_sq[cur] );
    tmp_term.swap( v_tmp[cur] );
#endif

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
#ifndef PBVR_SERIAL_MERGE
    std::vector<size_t> rej_avg_off( max_threads + 1, 0 );
    std::vector<size_t> rej_var_off( max_threads + 1, 0 );
    std::vector<size_t> rej_coef_off( max_threads + 1, 0 );
    size_t rej_avg_base = 0, rej_var_base = 0, rej_coef_base = 0;
#endif
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
#ifdef PBVR_SERIAL_MERGE
#pragma omp critical
        {
            average_coords.insert( average_coords.end(), th_average_coords.begin(), th_average_coords.end() );
            average_colors.insert( average_colors.end(), th_average_colors.begin(), th_average_colors.end() );
            average_normals.insert( average_normals.end(), th_average_normals.begin(), th_average_normals.end() );
            variance_coords.insert( variance_coords.end(), th_variance_coords.begin(), th_variance_coords.end() );
            variance_colors.insert( variance_colors.end(), th_variance_colors.begin(), th_variance_colors.end() );
            variance_normals.insert( variance_normals.end(), th_variance_normals.begin(), th_variance_normals.end() );
            coefficient_coords.insert( coefficient_coords.end(), th_coefficient_coords.begin(), th_coefficient_coords.end() );
            coefficient_colors.insert( coefficient_colors.end(), th_coefficient_colors.begin(), th_coefficient_colors.end() );
            coefficient_normals.insert( coefficient_normals.end(), th_coefficient_normals.begin(), th_coefficient_normals.end() );
        }
#else
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
#endif
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
