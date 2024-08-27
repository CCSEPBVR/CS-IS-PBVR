/*****************************************************************************/
/**
 *  @file   StreamlineBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StreamlineBase.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Streamline.h"
#include <kvs/DebugNew>
#include <kvs/Type>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new streamline class.
 */
/*===========================================================================*/
StreamlineBase::StreamlineBase( void ):
    kvs::MapperBase(),
    m_seed_points( NULL ),
    m_integration_method( Streamline::RungeKutta2nd ),
    m_integration_direction( Streamline::ForwardDirection ),
    m_integration_interval( 0.35f ),
    m_vector_length_threshold( 0.000001f ),
    m_integration_times_threshold( 256 ),
    m_enable_boundary_condition( true ),
    m_enable_vector_length_condition( true ),
    m_enable_integration_times_condition( true )
{
}

/*===========================================================================*/
/**
 *  @brief  Destroys the streamline class.
 */
/*===========================================================================*/
StreamlineBase::~StreamlineBase( void )
{
    if ( m_seed_points ) delete m_seed_points;
}

/*===========================================================================*/
/**
 *  @brief  Set the seed points.
 *  @param  seed_points [in] pointer to the seed points
 */
/*===========================================================================*/
void StreamlineBase::setSeedPoints( const kvs::PointObject* seed_points )
{
    // Shallow copy.
    m_seed_points = new kvs::PointObject( seed_points->coords() );
    if ( !m_seed_points )
    {
        kvsMessageError( "Cannot allocate memory for the seed points." );
    }
}

/*===========================================================================*/
/**
 *  @brief  Set the integration method.
 *  @param  method [in] integral method
 */
/*===========================================================================*/
void StreamlineBase::setIntegrationMethod( const StreamlineBase::IntegrationMethod method )
{
    m_integration_method = method;
}

/*===========================================================================*/
/**
 *  @brief  Set the integration direction.
 *  @param  direction [in] integration direction
 */
/*===========================================================================*/
void StreamlineBase::setIntegrationDirection( const StreamlineBase::IntegrationDirection direction )
{
    m_integration_direction = direction;
}

/*===========================================================================*/
/**
 *  @brief  Set the integration interval.
 *  @param  interval [in] integration interval
 */
/*===========================================================================*/
void StreamlineBase::setIntegrationInterval( const float interval )
{
    m_integration_interval = interval;
}

/*===========================================================================*/
/**
 *  @brief  Set the threshold of the vector length.
 *  @param  length [in] vector length
 */
/*===========================================================================*/
void StreamlineBase::setVectorLengthThreshold( const float length )
{
    m_vector_length_threshold = length;
}

/*===========================================================================*/
/**
 *  @brief  Set the threshold of the integration times.
 *  @param  times [in] integration times
 */
/*===========================================================================*/
void StreamlineBase::setIntegrationTimesThreshold( const size_t times )
{
    m_integration_times_threshold = times;
}

/*===========================================================================*/
/**
 *  @brief  Set the flag for the boundary condition.
 *  @param  enabled [in] flag
 */
/*===========================================================================*/
void StreamlineBase::setEnableBoundaryCondition( const bool enabled )
{
    m_enable_boundary_condition = enabled;
}

/*===========================================================================*/
/**
 *  @brief  Set the flag for vector length condition.
 *  @param  enabled [in] flag
 */
/*===========================================================================*/
void StreamlineBase::setEnableVectorLengthCondition( const bool enabled )
{
    m_enable_vector_length_condition = enabled;
}

/*===========================================================================*/
/**
 *  @brief  Set the flag for the integration times condition.
 *  @param  enabled [in] flag
 */
/*===========================================================================*/
void StreamlineBase::setEnableIntegrationTimesCondition( const bool enabled )
{
    m_enable_integration_times_condition = enabled;
}

/*===========================================================================*/
/**
 *  @brief  Calculates the streamlines.
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void StreamlineBase::mapping( const kvs::VolumeObjectBase* volume )
{
    if ( volume->volumeType() == kvs::VolumeObjectBase::Structured )
    {
        const kvs::StructuredVolumeObject* structured_volume =
            reinterpret_cast<const kvs::StructuredVolumeObject*>( volume );

        this->extract_lines( structured_volume );
    }
    else // volume->volumeType() == kvs::VolumeObjectBase::Unstructured
    {
        kvsMessageError("Unstructured volume object is not supported in the current system.");
    }
}

/*===========================================================================*/
/**
 *  @brief  Extracts the line segments.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void StreamlineBase::extract_lines(
    const kvs::StructuredVolumeObject* volume )
{
    kvs::IgnoreUnusedVariable( volume );

    // Calculated data arrays.
    std::vector<kvs::Real32> coords;
    std::vector<kvs::UInt32> connections;
    std::vector<kvs::UInt8>  colors;

    // Calculate streamline for each seed point.
    const size_t npoints = m_seed_points->nvertices();
    for ( size_t index = 0; index < npoints; index++ )
    {
        std::vector<kvs::Real32> line_coords;
        std::vector<kvs::UInt8> line_colors;

        if ( this->calculate_line( &line_coords, &line_colors, index ) )
        {
            if ( !this->check_for_acceptance( line_coords ) ) continue;

            // Set the first vertex ID to the connections.
            const size_t dimension = 3;
            const size_t start_id = coords.size() / dimension;
            connections.push_back( static_cast<kvs::UInt32>( start_id ) );

            // Set the line coordinate and color value array to the coords and colors, respectively.
            const size_t ncoords = line_coords.size();
            for( size_t i = 0; i < ncoords; i++ )
            {
                coords.push_back( line_coords[i] );
                colors.push_back( line_colors[i] );
            }

            // Set the last vertex ID to the connections.
            const size_t last_id = coords.size() / dimension - 1;
            connections.push_back( static_cast<kvs::UInt32>( last_id ) );
        }
    }

    SuperClass::setLineType( kvs::LineObject::Polyline );
    SuperClass::setColorType( kvs::LineObject::VertexColor );
    SuperClass::setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    SuperClass::setConnections( kvs::ValueArray<kvs::UInt32>( connections ) );
    SuperClass::setColors( kvs::ValueArray<kvs::UInt8>( colors ) );
    SuperClass::setSize( 1.0f );
}

/*===========================================================================*/
/**
 *  @brief  Calculate a streamline from the starting point that specified by the index.
 *  @param  coords [out] pointer to the coordinates value array
 *  @param  colors [out] pointer to the color value array
 *  @param  index [in] index of the starting point
 *  @return 
 */
/*===========================================================================*/
const bool StreamlineBase::calculate_line(
    std::vector<kvs::Real32>* coords,
    std::vector<kvs::UInt8>* colors,
    const size_t index )
{
    const kvs::Vector3f seed_point = m_seed_points->coord( index );
    if ( !this->check_for_inside_volume( seed_point ) ) return( false );

    const kvs::Vector3f seed_vector = this->calculate_vector( seed_point );
    if ( m_integration_direction == StreamlineBase::ForwardDirection )
    {
        // Forward direction.
        return( this->calculate_one_side(
                    &(*coords),
                    &(*colors),
                    seed_point,
                    seed_vector ) );
    }
    else if ( m_integration_direction == StreamlineBase::BackwardDirection )
    {
        // backward direction.
        return( this->calculate_one_side(
                    &(*coords),
                    &(*colors),
                    seed_point,
                    -seed_vector ) );
    }
    else // m_direction == Streamline::BothDirections
    {
        // Forward direction.
        std::vector<kvs::Real32> tmp_coords1;
        std::vector<kvs::UInt8> tmp_colors1;
        if ( !this->calculate_one_side(
                 &tmp_coords1,
                 &tmp_colors1,
                 seed_point,
                 seed_vector ) )
        {
            return( false );
        }

        // backward direction.
        std::vector<kvs::Real32> tmp_coords2;
        std::vector<kvs::UInt8> tmp_colors2;
        if( !this->calculate_one_side(
                &tmp_coords2,
                &tmp_colors2,
                seed_point,
                -seed_vector ) )
        {
            return( false );
        }

        const size_t nvertices1 = tmp_coords1.size() / 3;
        for( size_t i = 0; i < nvertices1; i++ )
        {
            const size_t id  = nvertices1 - i - 1;
            const size_t id3 = 3 * id;

            coords->push_back( tmp_coords1[id3  ] );
            coords->push_back( tmp_coords1[id3+1] );
            coords->push_back( tmp_coords1[id3+2] );
            colors->push_back( tmp_colors1[id3]   );
            colors->push_back( tmp_colors1[id3+1] );
            colors->push_back( tmp_colors1[id3+2] );
        }

        const size_t nvertices2 = tmp_coords2.size() / 3;
        for( size_t i = 1; i < nvertices2; i++ )
        {
            const size_t id3 = 3 * i;

            coords->push_back( tmp_coords2[id3  ] );
            coords->push_back( tmp_coords2[id3+1] );
            coords->push_back( tmp_coords2[id3+2] );
            colors->push_back( tmp_colors2[id3]   );
            colors->push_back( tmp_colors2[id3+1] );
            colors->push_back( tmp_colors2[id3+2] );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Calculate a streamline from the seed point along the seed vector.
 *  @param  coords [out] pointer to the coordinate data array
 *  @param  colors [out] pointer to the color data array
 *  @param  seed_point [in] seed point
 *  @param  seed_vector [in] seed vector
 *  @return 
 */
/*===========================================================================*/
const bool StreamlineBase::calculate_one_side(
    std::vector<kvs::Real32>* coords,
    std::vector<kvs::UInt8>* colors,
    const kvs::Vector3f& seed_point,
    const kvs::Vector3f& seed_vector )
{
    // Register the seed point.
    kvs::Vector3f current_vertex = seed_point;
    kvs::Vector3f next_vertex = seed_point;

    coords->push_back( seed_point.x() );
    coords->push_back( seed_point.y() );
    coords->push_back( seed_point.z() );

    // Register the vector on the seed point.
    kvs::Vector3f current_vector = seed_vector;
    kvs::Vector3f previous_vector = seed_vector;

    // Set the color of seed point.
    kvs::RGBColor col = this->calculate_color( current_vector );

    colors->push_back( col.r() );
    colors->push_back( col.g() );
    colors->push_back( col.b() );

    size_t integral_times = 0;
    for ( ; ; )
    {
        // Calculate the next vertex.
        if ( !this->calculate_next_vertex(
                 current_vertex,
                 current_vector,
                 &next_vertex ) )
        {
            return( true );
        }

        // Check the termination.
        if ( this->check_for_termination(
                 current_vertex,
                 current_vector,
                 integral_times,
                 next_vertex ) )
        {
            return( true );
        }

        // Update the vertex and vector.
        current_vertex = next_vertex;
        previous_vector = current_vector;

        coords->push_back( current_vertex.x() );
        coords->push_back( current_vertex.y() );
        coords->push_back( current_vertex.z() );

        // Interpolate vector from vertex of cell.
        current_vector = this->interpolate_vector( current_vertex, previous_vector );

        // Set color of vertex.
        kvs::RGBColor col = this->calculate_color( current_vector );

        colors->push_back( col.r() );
        colors->push_back( col.g() );
        colors->push_back( col.b() );

        integral_times++;
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculate a next vertex.
 *  @param  current_vertex [in] current vertex
 *  @param  current_direction [in] current direction vector
 *  @param  next_vertex [in] next vertex
 *  @return 
 */
/*===========================================================================*/
const bool StreamlineBase::calculate_next_vertex(
    const kvs::Vector3f& current_vertex,
    const kvs::Vector3f& current_direction,
    kvs::Vector3f* next_vertex )
{
    switch( m_integration_method )
    {
    case StreamlineBase::Euler:
        return( this->integrate_by_euler(
                    current_vertex,
                    current_direction,
                    &(*next_vertex) ) );
    case StreamlineBase::RungeKutta2nd:
        return( this->integrate_by_runge_kutta_2nd(
                    current_vertex,
                    current_direction,
                    &(*next_vertex) ) );
    case StreamlineBase::RungeKutta4th:
        return( this->integrate_by_runge_kutta_4th(
                    current_vertex,
                    current_direction,
                    &(*next_vertex) ) );
    default: break;
    }

    kvsMessageError( "Specified integral method is not defined." );
    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Integrate by Eular.
 *  @param  current_vertex [in] current vertex
 *  @param  current_direction [in] current direction vector
 *  @param  next_vertex [in] next vertex
 *  @return 
 */
/*===========================================================================*/
const bool StreamlineBase::integrate_by_euler(
    const kvs::Vector3f& current_vertex,
    const kvs::Vector3f& current_direction,
    kvs::Vector3f* next_vertex )
{
    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( current_vertex ) ) return( false );
    }

    const kvs::Vector3f k1 = m_integration_interval * current_direction;
    *next_vertex = current_vertex + k1;

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Integrate by Runge-Kutta 2nd.
 *  @param  current_vertex [in] current vertex
 *  @param  current_direction [in] current direction vector
 *  @param  next_vertex [in] next vertex
 *  @return 
 */
/*===========================================================================*/
const bool StreamlineBase::integrate_by_runge_kutta_2nd(
    const kvs::Vector3f& current_vertex,
    const kvs::Vector3f& current_direction,
    kvs::Vector3f* next_vertex )
{
    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( current_vertex ) ) return( false );
    }

    const kvs::Vector3f k1 = m_integration_interval * current_direction;
    // Interpolate vector from vertex of cell.
    const kvs::Vector3f vertex = current_vertex + 0.5f * k1;

    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( vertex ) ) return( false );
    }

    const kvs::Vector3f direction = this->interpolate_vector( vertex, current_direction );
    const kvs::Vector3f k2 = m_integration_interval * direction;
    *next_vertex = vertex + k2;

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Integrate by Runge-Kutta 4th.
 *  @param  current_vertex [in] current vertex
 *  @param  current_direction [in] current direction vector
 *  @param  next_vertex [in] next vertex
 *  @return 
 */
/*===========================================================================*/
const bool StreamlineBase::integrate_by_runge_kutta_4th(
    const kvs::Vector3f& current_vertex,
    const kvs::Vector3f& current_direction,
    kvs::Vector3f* next_vertex )
{
    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( current_vertex ) ) return( false );
    }

    // Calculate integration interval.
    const float interval = m_integration_interval / static_cast<float>(current_direction.length());

    const kvs::Vector3f k1 = interval * current_direction;

    // Interpolate vector from vertex of cell.
    const kvs::Vector3f vertex2 = current_vertex + 0.5f * k1;

    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( vertex2 ) ) return( false );
    }

    const kvs::Vector3f direction2 = this->interpolate_vector( vertex2, current_direction );
    const kvs::Vector3f k2 = interval * direction2;

    // Interpolate vector from vertex of cell.
    const kvs::Vector3f vertex3 = current_vertex + 0.5f * k2;

    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( vertex3 ) ) return( false );
    }

    const kvs::Vector3f direction3 = this->interpolate_vector( vertex3, current_direction );
    const kvs::Vector3f k3 = interval * direction3;

    // Interpolate vector from vertex of cell.
    const kvs::Vector3f vertex4 = current_vertex + k3;

    if ( m_enable_boundary_condition )
    {
        if ( !this->check_for_inside_volume( vertex4 ) ) return( false );
    }

    const kvs::Vector3f direction4 = this->interpolate_vector( vertex4, current_direction );
    const kvs::Vector3f k4 = interval * direction4;

    *next_vertex = current_vertex + ( k1 + 2.0f * ( k2 + k3 ) + k4 ) / 6.0f;

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the seed point is included in the volume.
 *  @param  point [in] seed point
 *  @return true, if the seed point is inside the volume
 */
/*===========================================================================*/
const bool StreamlineBase::check_for_inside_volume( const kvs::Vector3f& point )
{
    const kvs::StructuredVolumeObject* structured_volume =
        reinterpret_cast<const kvs::StructuredVolumeObject*>( BaseClass::volume() );
    const float dimx = static_cast<float>( structured_volume->resolution().x() - 1 );
    const float dimy = static_cast<float>( structured_volume->resolution().y() - 1 );
    const float dimz = static_cast<float>( structured_volume->resolution().z() - 1 );

    if ( point.x() < 0.0f || dimx < point.x() ) return( false );
    if ( point.y() < 0.0f || dimy < point.y() ) return( false );
    if ( point.z() < 0.0f || dimz < point.z() ) return( false );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Check the vector length.
 *  @param  direction [in] direction vector
 *  @return true if the vector length is smaller than the threshold
 */
/*===========================================================================*/
const bool StreamlineBase::check_for_vector_length( const kvs::Vector3f& direction )
{
    return( direction.length() < m_vector_length_threshold );
}

/*===========================================================================*/
/**
 *  @brief  Check the number of integrations.
 *  @param  times [in] number of integrations
 *  @return true if the given number is bigger than the threshold
 */
/*===========================================================================*/
const bool StreamlineBase::check_for_integration_times( const size_t times )
{
    return( times >= m_integration_times_threshold );
}

} // end of namespace kvs
