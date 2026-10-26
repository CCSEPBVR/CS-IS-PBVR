#ifndef PLOT_OVER_TIME
#define PLOT_OVER_TIME

#include <vismodule/Vector3>
#include <vismodule/Vector4>
#include <vismodule/Matrix33>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/ValueArray>
#include <vismodule/VolumeObjectBase>
#include <vismodule/Type>
#include <vismodule/PlotOverTimeProperty>

namespace vismodule
{
    using Vec3 = Vector3<double>;
    using Vec4 = Vector4<double>;
    using Mat3 = Matrix33<double>;
}

namespace POT
{
class Polyhedron
{
public:
    std::size_t nvertices;
    std::size_t ncells;
    vismodule::ValueArray<vismodule::Real32> coords;
    vismodule::ValueArray<vismodule::Real32> scalars;
    vismodule::ValueArray<vismodule::UInt32> connections;
    vismodule::ValueArray<vismodule::UInt32> connection_offsets;
    vismodule::ValueArray<vismodule::UInt32> faces;
    vismodule::ValueArray<vismodule::UInt32> face_offsets;
};
}

class PlotOverTime
{
private:
    vismodule::ValueArray<float> m_values_on_time;
    bool  m_mask;
    const POT::Polyhedron* m_polyhedron;

    // 変数配列
    float **m_values;
    int m_nvariables;
    float* m_coordinates;
    int m_nnodes;
    unsigned int* m_connections;
    int m_ncells;
    vismodule::VolumeObjectBase::CellType m_cellType;

    domain_parameters_struct m_dom;

public:
    // struct
    PlotOverTime(
        domain_parameters_struct dom,
        float** values,
        int nvariables, 
        const PlotOverTimeProperty& pot_property
    );

    // unstruct
    PlotOverTime(
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype, 
        const PlotOverTimeProperty& pot_property
    );

    vismodule::ValueArray<float> values(){ return m_values_on_time; }
    bool mask(){ return m_mask; }
    int nvariables(){ return m_nvariables; }

    void for_structured_mesh( const vismodule::Vec3 P );
    void for_tetrahedral_mesh( const vismodule::Vec3 P );
    void for_hexahedral_mesh( const vismodule::Vec3 P );
    void for_pyramidal_mesh( const vismodule::Vec3 P );
    void for_prismic_mesh( const vismodule::Vec3 P );
    void for_polyhedral_mesh( const vismodule::Vec3 P );
    void sampling_in_tetrahedra( const vismodule::Vec3 P, const vismodule::Vec3* vertices, const vismodule::Vec4* scalars, const int nvariables );
    const bool is_point_in_boundingbox( const vismodule::Vec3 MinCoord, const vismodule::Vec3 MaxCoord, const vismodule::Vec3 P );
    const vismodule::Vec4 barycentric_coordinates( const vismodule::Vec3 X0, const vismodule::Vec3 X1, const vismodule::Vec3 X2, const vismodule::Vec3 X3, const vismodule::Vec3 P );
    const float tetrahedra_signed_volume_X6( const vismodule::Vec3 X0, const vismodule::Vec3 X1, const vismodule::Vec3 X2, const vismodule::Vec3 X3 );
};

#endif // PLOT_OVER_TIME