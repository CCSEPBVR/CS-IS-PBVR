#ifndef PBVR_VERTEX_TF_GRADIENT_H
#define PBVR_VERTEX_TF_GRADIENT_H

#include <iosfwd>
#include <vector>
#include <vismodule/Vector3>
#include <vismodule/VolumeObjectBase>

#ifdef DOUBLE_SCHEME
typedef double Type;
#else
typedef float Type;
#endif

struct EquationToken;

namespace pbvr
{

struct VertexTFGradientLog
{
    int ncoords;
    int ncells;
    int nvariables;
    std::size_t tf_values_bytes;
    double build_seconds;
    double gradient_seconds;
    double compare_seconds;

    VertexTFGradientLog();
};

bool BuildVertexTFValues(
    const EquationToken& expr,
    Type** values,
    int nvariables,
    int ncoords,
    std::vector<float>& tf_values,
    VertexTFGradientLog* log = NULL );

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
    VertexTFGradientLog* log = NULL );

bool ComputeNormalFromGradient(
    const vismodule::Vector3f& grad_F,
    vismodule::Vector3f* normal );

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
    VertexTFGradientLog* log = NULL );

void PrintVertexTFGradientLog(
    const VertexTFGradientLog& log,
    std::ostream& os );

bool RunVertexTFGradientSelfTest();

} // namespace pbvr

#endif // PBVR_VERTEX_TF_GRADIENT_H
