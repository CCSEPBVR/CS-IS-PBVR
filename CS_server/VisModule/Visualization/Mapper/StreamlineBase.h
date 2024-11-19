/*****************************************************************************/
/**
 *  @file   StreamlineBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StreamlineBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__STREAMLINE_BASE_H_INCLUDE
#define VIS_MODULE__STREAMLINE_BASE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/MapperBase>
#include <vismodule/LineObject>
#include <vismodule/PointObject>
#include <vismodule/StructuredVolumeObject>


namespace vismodule
{

/*===========================================================================*/
/**
 *  Streamline class.
 */
/*===========================================================================*/
class StreamlineBase : public vismodule::MapperBase, public vismodule::LineObject
{
    // Class name.
    visModuleClassName( vismodule::StreamlineBase );

    // Module information.
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::LineObject );

public:

    enum IntegrationMethod
    {
        Euler = 0,
        RungeKutta2nd = 1,
        RungeKutta4th = 2
    };

    enum IntegrationDirection
    {
        ForwardDirection = 0,
        BackwardDirection = 1,
        BothDirections = 2
    };

protected:

    vismodule::PointObject* m_seed_points; ///< seed points
    IntegrationMethod m_integration_method; ///< integtration method
    IntegrationDirection m_integration_direction; ///< integration direction
    float m_integration_interval; ///< integration interval in the object coordinate
    float m_vector_length_threshold; ///< threshold of the vector length
    size_t m_integration_times_threshold; ///< threshold of the integration times
    bool m_enable_boundary_condition; ///< flag for the boundray condition
    bool m_enable_vector_length_condition; ///< flag for the vector length condition
    bool m_enable_integration_times_condition; ///< flag for the integration times

public:

    StreamlineBase( void );

    virtual ~StreamlineBase( void );

public:

    void setSeedPoints( const vismodule::PointObject* seed_points );

    void setIntegrationMethod( const StreamlineBase::IntegrationMethod method );

    void setIntegrationDirection( const StreamlineBase::IntegrationDirection direction );

    void setIntegrationInterval( const float interval );

    void setVectorLengthThreshold( const float length );

    void setIntegrationTimesThreshold( const size_t times );

    void setEnableBoundaryCondition( const bool enabled );

    void setEnableVectorLengthCondition( const bool enabled );

    void setEnableIntegrationTimesCondition( const bool enabled );

public:

    virtual vismodule::ObjectBase* exec( const vismodule::ObjectBase* object ) = 0;

protected:

    virtual const bool check_for_acceptance( const std::vector<vismodule::Real32>& vertices ) = 0;

    virtual const bool check_for_termination(
        const vismodule::Vector3f& current_vertex,
        const vismodule::Vector3f& direction,
        const size_t integration_times,
        const vismodule::Vector3f& next_vertex ) = 0;

    virtual const vismodule::Vector3f interpolate_vector( const vismodule::Vector3f& vertex, const vismodule::Vector3f& direction ) = 0;

    virtual const vismodule::Vector3f calculate_vector( const vismodule::Vector3f& vertex ) = 0;

    virtual const vismodule::RGBColor calculate_color( const vismodule::Vector3f& direction ) = 0;

protected:

    void mapping( const vismodule::VolumeObjectBase* volume );

    void extract_lines(
        const vismodule::StructuredVolumeObject* volume );

    const bool calculate_line(
        std::vector<vismodule::Real32>* vertices,
        std::vector<vismodule::UInt8>* colors,
        const size_t index );

    const bool calculate_one_side(
        std::vector<vismodule::Real32>* coords,
        std::vector<vismodule::UInt8>* colors,
        const vismodule::Vector3f& seed_point,
        const vismodule::Vector3f& seed_vector );

    const bool calculate_next_vertex(
        const vismodule::Vector3f& current_vertex,
        const vismodule::Vector3f& current_direction,
        vismodule::Vector3f* next_vertex );

    const bool integrate_by_euler(
        const vismodule::Vector3f& current_vertex,
        const vismodule::Vector3f& current_direction,
        vismodule::Vector3f* next_vertex );

    const bool integrate_by_runge_kutta_2nd(
        const vismodule::Vector3f& current_vertex,
        const vismodule::Vector3f& current_direction,
        vismodule::Vector3f* next_vertex );

    const bool integrate_by_runge_kutta_4th(
        const vismodule::Vector3f& current_vertex,
        const vismodule::Vector3f& current_direction,
        vismodule::Vector3f* next_vertex );

protected:

    const bool check_for_inside_volume( const vismodule::Vector3f& seed );

    const bool check_for_vector_length( const vismodule::Vector3f& direction );

    const bool check_for_integration_times( const size_t times );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STREAMLINE_BASE_H_INCLUDE
