/*****************************************************************************/
/**
 *  @file   CellByCellLayeredSampling.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellLayeredSampling.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__CELL_BY_CELL_LAYERED_SAMPLING_H_INCLUDE
#define VIS_MODULE__CELL_BY_CELL_LAYERED_SAMPLING_H_INCLUDE

#include <vismodule/MapperBase>
#include <vismodule/Camera>
#include <vismodule/PointObject>
#include <vismodule/VolumeObjectBase>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/TetrahedralCell>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Cell-by-cell particle generation class.
 */
/*===========================================================================*/
class CellByCellLayeredSampling : public vismodule::MapperBase, public vismodule::PointObject
{
    // Class name.
    visModuleClassName( vismodule::CellByCellLayeredSampling );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::PointObject );

protected:

    struct SelectedParticles
    {
        size_t nparticles; ///< number of selected particles
        vismodule::ValueArray<vismodule::UInt32> indices; ///< index array of the selected particles
    };

private:

    const vismodule::Camera*     m_camera;         ///< camera (reference)
    size_t                 m_subpixel_level; ///< subpixel level
    float                  m_sampling_step;  ///< sampling step in the object coordinate
    float                  m_object_depth;   ///< object depth
    vismodule::ValueArray<float> m_density_map;    ///< density map
    vismodule::PointObject*      m_pregenerated_particles; ///< pregenerated particles
    SelectedParticles m_selected_particles; ///< particles selected from the pregenerated particles
    vismodule::Real32            m_M_value;  ///< numerical integration value of density distribution
    vismodule::Matrix44f         m_L_matrix; ///< conversion matrix
    vismodule::Matrix44f         m_A_matrix; ///< normalization conversion matrix

public:

    CellByCellLayeredSampling( void );

    CellByCellLayeredSampling(
        const vismodule::VolumeObjectBase* volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        const float                  object_depth = 0.0f );

    CellByCellLayeredSampling(
        const vismodule::Camera*           camera,
        const vismodule::VolumeObjectBase* volume,
        const size_t                 subpixel_level,
        const float                  sampling_step,
        const vismodule::TransferFunction& transfer_function,
        const float                  object_depth = 0.0f );

    virtual ~CellByCellLayeredSampling( void );

public:

    SuperClass* exec( const vismodule::ObjectBase* object );

public:

    const size_t subpixelLevel( void ) const;

    const float samplingStep( void ) const;

    const float objectDepth( void ) const;

    void attachCamera( const vismodule::Camera* camera );

    void setSubpixelLevel( const size_t subpixel_level );

    void setSamplingStep( const float sampling_step );

    void setObjectDepth( const float object_depth );

private:

    void mapping( const vismodule::Camera* camera, const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    void generate_particles( const vismodule::UnstructuredVolumeObject* volume );

    void pregenerate_particles( const size_t nparticles );

    template <typename T>
    void uniform_sampling(
        const vismodule::TetrahedralCell<T>* cell,
        const vismodule::TransferFunction& tfunc,
        const size_t nparticles,
        std::vector<vismodule::Real32>* coords,
        std::vector<vismodule::UInt8>*  colors,
        std::vector<vismodule::Real32>* normals );

    template <typename T>
    void rejection_sampling(
        const vismodule::TetrahedralCell<T>* cell,
        const vismodule::TransferFunction& tfunc,
        const size_t nparticles,
        std::vector<vismodule::Real32>* coords,
        std::vector<vismodule::UInt8>*  colors,
        std::vector<vismodule::Real32>* normals );

    template <typename T>
    void roulette_selection(
        const vismodule::TetrahedralCell<T>* cell,
        const vismodule::TransferFunction& tfunc,
        const size_t nparticles,
        std::vector<vismodule::Real32>* coords,
        std::vector<vismodule::UInt8>*  colors,
        std::vector<vismodule::Real32>* normals );

    const float calculate_density( const float scalar );

    const float calculate_maximum_density( const float scalar0, const float scalar1 );

    template <typename T>
    const size_t calculate_number_of_particles( const float density, const vismodule::TetrahedralCell<T>* cell );

    const size_t calculate_number_of_particles( const size_t nparticles_in_cell );

    template <typename T>
    void calculate_particles_in_cell(
        const vismodule::TetrahedralCell<T>* cell );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CELL_BY_CELL_LAYERED_SAMPLING_H_INCLUDE
