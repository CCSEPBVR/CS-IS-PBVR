/*****************************************************************************/
/**
 *  @file   streamline_base.cpp
 *  @brief  
 *
 *  @author Yukio YASUHARA
 *  @date   2010/08/09 23:22:15
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: naohisa $
 *  $Date: 2009/01/15 16:13:56 $
 *  $Source: /home/Repository/viz-server2/cvsroot/VIS_MODULE_RC1/Source/Core/Visualization/Mapper/Streamline.cpp,v $
 *  $Revision: 1.2 $
 */
/*****************************************************************************/
#include "Streamline.h"
#include <vismodule/Type>
#include <vismodule/IgnoreUnusedVariable>
#include <vismodule/Message>
#include <vismodule/RGBColor>
#include <vismodule/Vector3>
#include <vismodule/VolumeObjectBase>


namespace
{

template <typename T>
inline const vismodule::Vector3f GetInterpolatedVector( const std::size_t vertex_id[8], const float weight[8], const vismodule::VolumeObjectBase& volume )
{
    const T* values = reinterpret_cast<const T*>( volume.values().pointer() );

    vismodule::Vector3f ret( 0.0f );
    for ( std::size_t i = 0; i < 8; i++ )
    {
        const std::size_t index = 3 * vertex_id[i];
        const float w = weight[i];
        ret.x() += static_cast<float>( values[ index     ] * w );
        ret.y() += static_cast<float>( values[ index + 1 ] * w );
        ret.z() += static_cast<float>( values[ index + 2 ] * w );
    }

    return( ret );
}

} // end of namespace


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new streamline class.
 */
/*===========================================================================*/
Streamline::Streamline( void ):
    vismodule::StreamlineBase()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new streamline class and executes this class.
 *  @param  volume [in] pointer to the input volume object
 *  @param  seed_points [in] pointer to the seed points
 */
/*===========================================================================*/
Streamline::Streamline(
    const vismodule::StructuredVolumeObject& volume,
    const vismodule::PointObject* seed_points,
    const vismodule::TransferFunction& transfer_function ):
    vismodule::StreamlineBase()
{
    BaseClass::setTransferFunction( transfer_function );
    BaseClass::setSeedPoints( seed_points );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the streamline class.
 */
/*===========================================================================*/
Streamline::~Streamline( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the mapper process.
 *  @param  object [in] pointer to the volume object
 *  @return line object
 */
/*===========================================================================*/
Streamline::BaseClass::SuperClass* Streamline::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::VolumeObjectBase* volume = vismodule::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not volume dat.");
        return( NULL );
    }

    // Check whether the volume can be processed or not.
    if ( volume->veclen() != 3 )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input volume is not vector field data.");
        return( NULL );
    }

    // Attach the pointer to the volume object.
    BaseClass::attach_volume( *volume );
    BaseClass::set_range( *volume );
    BaseClass::set_min_max_coords( *volume, this );

    // set the min/max vector length.
    if ( !volume->hasMinMaxValues() )
    {
        volume->updateMinMaxValues();
    }

    BaseClass::mapping( *volume );

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the vertices are accepted or not.
 *  @param  vertices [in] vertices
 *  @return true if the vertices are accepted
 */
/*===========================================================================*/
const bool Streamline::check_for_acceptance( const std::vector<vismodule::Real32>& vertices )
{
    vismodule::IgnoreUnusedVariable( vertices );
    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Test whether the streamline is futher integrated or terminated.
 *  @param  current_vertex [in] position of the current vertex
 *  @param  direction [in] direction vector
 *  @param  integration_times [in] integration times
 *  @param  next_vertex [in] position of the next vertex
 *  @return true, if the stremaline is integrated.
 */
/*===========================================================================*/
const bool Streamline::check_for_termination(
    const vismodule::Vector3f& current_vertex,
    const vismodule::Vector3f& direction,
    const std::size_t         integration_times,
    const vismodule::Vector3f& next_vertex )
{
    vismodule::IgnoreUnusedVariable( current_vertex );

    if ( m_enable_boundary_condition )
    {
        if ( !BaseClass::check_for_inside_volume( next_vertex ) ) return( true );
    }

    if ( m_enable_vector_length_condition )
    {
        return( BaseClass::check_for_vector_length( direction ) );
    }

    if ( m_enable_integration_times_condition )
    {
        return( BaseClass::check_for_integration_times( integration_times ) );
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Calculate the interpolated vector on the seed point.
 *  @param  point [in] seed point
 *  @return interpolated vector
 */
/*===========================================================================*/
const vismodule::Vector3f Streamline::calculate_vector( const vismodule::Vector3f& point )
{
    const vismodule::Vector3f origin( 0.0f, 0.0f, 0.0f );
    return( this->interpolate_vector( point, origin ) );
}

/*===========================================================================*/
/**
 *  @brief  Calculate the color.
 *  @param  direction [in] direction vector
 *  @return color
 */
/*===========================================================================*/
const vismodule::RGBColor Streamline::calculate_color( const vismodule::Vector3f& direction )
{
    const vismodule::Real64 min_length = BaseClass::volume()->minValue();
    const vismodule::Real64 max_length = BaseClass::volume()->maxValue();
    const vismodule::Real64 diff = direction.length() - min_length;
    const vismodule::Real64 interval = max_length - min_length;
    const vismodule::UInt8 level = vismodule::UInt8( 255.0 * diff / interval );

    return( BaseClass::transferFunction().colorMap()[level] );
}

/*===========================================================================*/
/**
 *  @brief  Interpolates the vertex.
 *  @param  vertex [in] vertex
 *  @param  previous_vector [in] previous vector
 *  @return interpolated vector
 */
/*===========================================================================*/
const vismodule::Vector3f Streamline::interpolate_vector(
    const vismodule::Vector3f& vertex,
    const vismodule::Vector3f& previous_vector )
{
    vismodule::IgnoreUnusedVariable( previous_vector );

    const std::size_t cell_x = static_cast<size_t>( vertex.x() );
    const std::size_t cell_y = static_cast<size_t>( vertex.y() );
    const std::size_t cell_z = static_cast<size_t>( vertex.z() );

    const vismodule::StructuredVolumeObject* volume = vismodule::StructuredVolumeObject::DownCast( *BaseClass::volume() );
    const std::size_t resolution_x = static_cast<size_t>( volume->resolution().x() );
    const std::size_t resolution_y = static_cast<size_t>( volume->resolution().y() );
//    const std::size_t resolution_z = static_cast<size_t>( volume->resolution().z() );

    std::size_t vertex_id[8];
    vertex_id[0] = cell_z * resolution_x * resolution_y + cell_y * resolution_x + cell_x;
    vertex_id[1] = vertex_id[0] + 1;
    vertex_id[2] = vertex_id[1] + resolution_x;
    vertex_id[3] = vertex_id[2] - 1;
    vertex_id[4] = vertex_id[0] + resolution_x * resolution_y;
    vertex_id[5] = vertex_id[4] + 1;
    vertex_id[6] = vertex_id[5] + resolution_x;
    vertex_id[7] = vertex_id[6] - 1;

    // Weight.
    const vismodule::Vector3f local_coord(
        2.0f * ( vertex.x() - cell_x ) - 1.0f,
        2.0f * ( vertex.y() - cell_y ) - 1.0f,
        2.0f * ( vertex.z() - cell_z ) - 1.0f );

    const float x_min = local_coord.x() - 1.0f;
    const float x_max = local_coord.x() + 1.0f;
    const float y_min = local_coord.y() - 1.0f;
    const float y_max = local_coord.y() + 1.0f;
    const float z_min = local_coord.z() - 1.0f;
    const float z_max = local_coord.z() + 1.0f;

    const float weight[8] = {
        -x_min * y_min * z_min * 0.125f,
        x_max  * y_min * z_min * 0.125f,
        -x_max * y_max * z_min * 0.125f,
        x_min  * y_max * z_min * 0.125f,
        x_min  * y_min * z_max * 0.125f,
        -x_max * y_min * z_max * 0.125f,
        x_max  * y_max * z_max * 0.125f,
        -x_min * y_max * z_max * 0.125f };

    // Interpolate.
    const std::type_info& type = BaseClass::volume()->values().typeInfo()->type();
    if (      type == typeid( vismodule::Int8   ) ) return( ::GetInterpolatedVector<vismodule::Int8>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::Int16  ) ) return( ::GetInterpolatedVector<vismodule::Int16>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::Int32  ) ) return( ::GetInterpolatedVector<vismodule::Int32>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::Int64  ) ) return( ::GetInterpolatedVector<vismodule::Int64>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::UInt8  ) ) return( ::GetInterpolatedVector<vismodule::UInt8>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::UInt16 ) ) return( ::GetInterpolatedVector<vismodule::UInt16>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::UInt32 ) ) return( ::GetInterpolatedVector<vismodule::UInt32>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::UInt64 ) ) return( ::GetInterpolatedVector<vismodule::UInt64>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::Real32 ) ) return( ::GetInterpolatedVector<vismodule::Real32>( vertex_id, weight, *BaseClass::volume() ) );
    else if ( type == typeid( vismodule::Real64 ) ) return( ::GetInterpolatedVector<vismodule::Real64>( vertex_id, weight, *BaseClass::volume() ) );

    return( vismodule::Vector3f( 0.0f, 0.0f, 0.0f ) );
}

} // end of namespace vismodule
