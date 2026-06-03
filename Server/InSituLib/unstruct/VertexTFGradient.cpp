#include "VertexTFGradient.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <memory>

#include <vismodule/HexahedralCell>
#include <vismodule/PyramidalCell>
#include <vismodule/PrismaticCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/TetrahedralCell>

#include "ChainRuleNormal.h"
#include "../../FunctionParser/ReversePolishNotation.h"
#include "../../FunctionParser/Token.h"
#include "../../VisModule/Visualization/Mapper/TransferFunctionSynthesizer.h"

namespace
{

double SecondsNow()
{
    return static_cast<double>( std::clock() ) / static_cast<double>( CLOCKS_PER_SEC );
}

bool IsQVariable( const int variable_name, std::size_t* variable_index )
{
    if ( variable_name < Q1 || variable_name > Q23 ) return false;
    if ( ( variable_name - Q1 ) % 4 != 0 ) return false;

    *variable_index = static_cast<std::size_t>( ( variable_name - Q1 ) / 4 );
    return true;
}

std::vector<std::size_t> ActiveQVariables( const EquationToken& expr, const int nvariables )
{
    std::vector<std::size_t> active_variables;

    for ( int i = 0; i < 128 && expr.exp_token[i] != END; ++i )
    {
        if ( expr.exp_token[i] != VARIABLE ) continue;

        std::size_t variable_index = 0;
        if ( !IsQVariable( expr.var_name[i], &variable_index ) ) continue;
        if ( variable_index >= static_cast<std::size_t>( nvariables ) ) continue;

        active_variables.push_back( variable_index );
    }

    std::sort( active_variables.begin(), active_variables.end() );
    active_variables.erase(
        std::unique( active_variables.begin(), active_variables.end() ),
        active_variables.end() );
    return active_variables;
}

float EvalExpression( const EquationToken& expr, float variable_values[128] )
{
    FuncParser::ReversePolishNotation rpn;
    rpn.setExpToken( const_cast<int*>( &( expr.exp_token[0] ) ) );
    rpn.setVariableName( const_cast<int*>( &( expr.var_name[0] ) ) );
    rpn.setNumber( const_cast<float*>( &( expr.val_array[0] ) ) );
    rpn.setVariableValue( variable_values );
    return rpn.eval();
}

std::unique_ptr< vismodule::CellBase<Type> > CreateCell(
    Type* values,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype )
{
    switch ( celltype )
    {
    case vismodule::VolumeObjectBase::Tetrahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::TetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::Hexahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::HexahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::QuadraticTetrahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::QuadraticTetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::QuadraticHexahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::QuadraticHexahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::Prism:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::PrismaticCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::Pyramid:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::PyramidalCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    default:
        return std::unique_ptr< vismodule::CellBase<Type> >();
    }
}

bool IsFiniteVector( const vismodule::Vector3f& v )
{
    return std::isfinite( v.x() ) && std::isfinite( v.y() ) && std::isfinite( v.z() );
}

} // namespace

namespace pbvr
{

VertexTFGradientLog::VertexTFGradientLog() :
    ncoords( 0 ),
    ncells( 0 ),
    nvariables( 0 ),
    tf_values_bytes( 0 ),
    build_seconds( 0.0 ),
    gradient_seconds( 0.0 ),
    compare_seconds( 0.0 )
{
}

bool BuildVertexTFValues(
    const EquationToken& expr,
    Type** values,
    int nvariables,
    int ncoords,
    std::vector<float>& tf_values,
    VertexTFGradientLog* log )
{
    if ( values == NULL || nvariables <= 0 || ncoords <= 0 ) return false;

    const double start = SecondsNow();
    const std::vector<std::size_t> active_variables = ActiveQVariables( expr, nvariables );
    if ( active_variables.empty() ) return false;

    tf_values.resize( static_cast<std::size_t>( ncoords ) );

    // Thread note: this loop is node-independent. If OpenMP is enabled later,
    // make variable_values thread-local to avoid races.
    float variable_values[128];
    std::fill( variable_values, variable_values + 128, 0.0f );

    for ( int node_id = 0; node_id < ncoords; ++node_id )
    {
        for ( std::vector<std::size_t>::const_iterator it = active_variables.begin();
              it != active_variables.end(); ++it )
        {
            const std::size_t variable_index = *it;
            variable_values[Q1 + 4 * variable_index] = static_cast<float>( values[variable_index][node_id] );
        }

        const float F = EvalExpression( expr, variable_values );
        tf_values[node_id] = std::isfinite( F ) ? F : 0.0f;
    }

    if ( log != NULL )
    {
        log->ncoords = ncoords;
        log->nvariables = nvariables;
        log->tf_values_bytes = tf_values.size() * sizeof( float );
        log->build_seconds += SecondsNow() - start;
    }

    return true;
}

bool ComputeVertexTFGradientAtPoint(
    const std::vector<float>& tf_values,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    int cell_id,
    const vismodule::Vector3f& local_coord,
    vismodule::Vector3f* grad_F,
    VertexTFGradientLog* log )
{
    if ( grad_F == NULL ) return false;
    if ( tf_values.empty() || coordinates == NULL || connections == NULL ) return false;
    if ( cell_id < 0 || cell_id >= ncells ) return false;

    const double start = SecondsNow();

    // CellBase stores references to values/coordinates/connections in its constructor.
    // It copies only the bound cell's vertex scalars/coordinates into m_scalars/m_vertices
    // during bindCell()/bindCellArray(), so tf_values must outlive this cell object.
    Type* cell_values = NULL;
#ifdef DOUBLE_SCHEME
    std::vector<Type> tf_values_for_cell( tf_values.begin(), tf_values.end() );
    cell_values = &tf_values_for_cell[0];
#else
    cell_values = const_cast<Type*>( tf_values.data() );
#endif

    std::unique_ptr< vismodule::CellBase<Type> > cell = CreateCell(
        cell_values, coordinates, ncoords, connections, ncells, celltype );

    if ( !cell ) return false;

    cell->bindCell( static_cast<vismodule::UInt32>( cell_id ) );
    cell->setLocalPoint( local_coord );
    cell->differentialFunctions( local_coord );
    *grad_F = cell->gradient();

    if ( !IsFiniteVector( *grad_F ) ) *grad_F = vismodule::Vector3f( 0.0f, 0.0f, 0.0f );

    if ( log != NULL )
    {
        log->ncoords = ncoords;
        log->ncells = ncells;
        log->tf_values_bytes = tf_values.size() * sizeof( float );
        log->gradient_seconds += SecondsNow() - start;
    }

    return true;
}

bool ComputeNormalFromGradient( const vismodule::Vector3f& grad_F, vismodule::Vector3f* normal )
{
    if ( normal == NULL ) return false;

    const float norm = static_cast<float>( grad_F.length() );
    if ( !std::isfinite( norm ) || norm <= 0.0f )
    {
        *normal = vismodule::Vector3f( 0.0f, 0.0f, 0.0f );
        return false;
    }

    *normal = -( grad_F / norm );
    return true;
}

bool CompareVertexTFGradientWithChainRule(
    const EquationToken& expr,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    int cell_id,
    const vismodule::Vector3f& local_coord,
    float* normal_dot,
    VertexTFGradientLog* log )
{
    if ( normal_dot == NULL ) return false;

    const double start = SecondsNow();

    std::vector<float> tf_values;
    if ( !BuildVertexTFValues( expr, values, nvariables, ncoords, tf_values, log ) ) return false;

    vismodule::Vector3f vertex_grad;
    if ( !ComputeVertexTFGradientAtPoint(
             tf_values,
             coordinates,
             ncoords,
             connections,
             ncells,
             celltype,
             cell_id,
             local_coord,
             &vertex_grad,
             log ) ) return false;

    std::vector<float> q_values( static_cast<std::size_t>( nvariables ) );
    std::vector<vismodule::Vector3f> grad_q( static_cast<std::size_t>( nvariables ) );

    for ( int v = 0; v < nvariables; ++v )
    {
        std::unique_ptr< vismodule::CellBase<Type> > q_cell = CreateCell(
            values[v],
            coordinates,
            ncoords,
            connections,
            ncells,
            celltype );
        if ( !q_cell ) return false;

        q_cell->bindCell( static_cast<vismodule::UInt32>( cell_id ) );
        q_cell->setLocalPoint( local_coord );
        q_cell->interpolationFunctions( local_coord );
        q_cell->differentialFunctions( local_coord );
        q_values[v] = q_cell->scalar();
        grad_q[v] = q_cell->gradient();
    }

    vismodule::Vector3f chain_grad;
    if ( !ComputeChainRuleGradient( expr, q_values, grad_q, &chain_grad ) ) return false;

    vismodule::Vector3f vertex_normal;
    vismodule::Vector3f chain_normal;
    ComputeNormalFromGradient( vertex_grad, &vertex_normal );
    ComputeNormalFromGradient( chain_grad, &chain_normal );

    *normal_dot = vertex_normal.dot( chain_normal );

    if ( log != NULL )
    {
        log->ncells = ncells;
        log->ncoords = ncoords;
        log->nvariables = nvariables;
        log->compare_seconds += SecondsNow() - start;
    }

    return std::isfinite( *normal_dot );
}

void PrintVertexTFGradientLog( const VertexTFGradientLog& log, std::ostream& os )
{
    os << "VertexTFGradient: ncoords=" << log.ncoords
       << ", ncells=" << log.ncells
       << ", nvariables=" << log.nvariables
       << ", tf_values_bytes=" << log.tf_values_bytes
       << ", build_seconds=" << log.build_seconds
       << ", gradient_seconds=" << log.gradient_seconds
       << ", compare_seconds=" << log.compare_seconds
       << std::endl;
}

bool RunVertexTFGradientSelfTest()
{
    float coordinates[12] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    unsigned int connections[4] = { 0, 1, 2, 3 };

    Type q1[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
    Type q2[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
    Type* values[2] = { q1, q2 };

    const EquationToken expr_sum = MakeEquationTokenForChainRuleTest( "q1+q2" );
    std::vector<float> tf_values;
    if ( !BuildVertexTFValues( expr_sum, values, 2, 4, tf_values, NULL ) ) return false;

    for ( int i = 0; i < 4; ++i )
    {
        if ( std::fabs( tf_values[i] - static_cast<float>( q1[i] + q2[i] ) ) > 1.0e-5f ) return false;
    }

    const EquationToken expr_q1 = MakeEquationTokenForChainRuleTest( "q1" );
    std::vector<float> tf_q1;
    if ( !BuildVertexTFValues( expr_q1, values, 2, 4, tf_q1, NULL ) ) return false;

    vismodule::Vector3f grad_q1;
    if ( !ComputeVertexTFGradientAtPoint(
             tf_q1,
             coordinates,
             4,
             connections,
             1,
             vismodule::VolumeObjectBase::Tetrahedra,
             0,
             vismodule::Vector3f( 0.25f, 0.25f, 0.25f ),
             &grad_q1,
             NULL ) ) return false;

    if ( std::fabs( grad_q1.x() - 1.0f ) > 1.0e-5f ||
         std::fabs( grad_q1.y() ) > 1.0e-5f ||
         std::fabs( grad_q1.z() ) > 1.0e-5f ) return false;

    float normal_dot = 0.0f;
    if ( !CompareVertexTFGradientWithChainRule(
             expr_sum,
             values,
             2,
             coordinates,
             4,
             connections,
             1,
             vismodule::VolumeObjectBase::Tetrahedra,
             0,
             vismodule::Vector3f( 0.25f, 0.25f, 0.25f ),
             &normal_dot,
             NULL ) ) return false;
    if ( std::fabs( normal_dot - 1.0f ) > 1.0e-5f ) return false;

    vismodule::Vector3f normal;
    if ( !ComputeNormalFromGradient( vismodule::Vector3f( 0.0f, 0.0f, 2.0f ), &normal ) ) return false;
    return std::fabs( normal.z() + 1.0f ) <= 1.0e-5f;
}

} // namespace pbvr
