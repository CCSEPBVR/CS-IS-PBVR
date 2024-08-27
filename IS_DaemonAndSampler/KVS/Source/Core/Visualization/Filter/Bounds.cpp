/****************************************************************************/
/**
 *  @file Bounds.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Bounds.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Bounds.h"
#include <kvs/PointObject>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Bounds class.
 */
/*===========================================================================*/
Bounds::Bounds( void ):
    m_type( Bounds::Box ),
    m_corner_scale( 0.2f ),
    m_division( 50.0f )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new Bounds class.
 *  @param  object [in] pointer to the object
 *  @param  type [in] bounds type
 */
/*==========================================================================*/
Bounds::Bounds( const kvs::ObjectBase* object, const Bounds::Type type ):
    m_type( type ),
    m_corner_scale( 0.2f ),
    m_division( 50.0f )
{
    this->exec( object );
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new Bounds class.
 *  @param  min_coord [in] min. coordinate
 *  @param  max_coord [in] max. coordinate
 *  @param  type [in] bounds type
 */
/*==========================================================================*/
Bounds::Bounds( const kvs::Vector3f& min_coord, const kvs::Vector3f& max_coord, const Bounds::Type type ):
    m_type( type ),
    m_corner_scale( 0.2f ),
    m_division( 50.0f )
{
    kvs::PointObject object;
    object.setMinMaxExternalCoords( min_coord, max_coord );
    object.setMinMaxObjectCoords( min_coord, max_coord );

    this->exec( &object );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Bounds class.
 */
/*===========================================================================*/
Bounds::~Bounds( void )
{
}

/*==========================================================================*/
/**
 *  Create a bounding box as line object.
 *  @param object [in] pointer to the object
 */
/*==========================================================================*/
Bounds::SuperClass* Bounds::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

/*
    if ( object->objectType() != kvs::ObjectBase::Geometry ||
         object->objectType() != kvs::ObjectBase::Volume )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is not supported.");
        return( NULL );
    }
*/

    if ( object->hasMinMaxExternalCoords() )
    {
        const kvs::Vector3f min_coord( object->minExternalCoord() );
        const kvs::Vector3f max_coord( object->maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( object->hasMinMaxObjectCoords() )
    {
        const kvs::Vector3f min_coord( object->minObjectCoord() );
        const kvs::Vector3f max_coord( object->maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    switch( m_type )
    {
    case Bounds::Box:    this->create_box_bounds();    break;
    case Bounds::Corner: this->create_corner_bounds(); break;
    case Bounds::Circle: this->create_circle_bounds(); break;
    default: break;
    }

    SuperClass::disableCollision();

    return( this );
}

/*==========================================================================*/
/**
 *  @breif  Set the bounds type.
 *  @param  type [in] bounds type
 */
/*==========================================================================*/
void Bounds::setType( const Bounds::Type type )
{
    m_type = type;
}

/*==========================================================================*/
/**
 *  @breif  Set the corner scale size.
 *  @param  corner_scale [in] corner scale size
 */
/*==========================================================================*/
void Bounds::setCornerScale( const float corner_scale )
{
    m_corner_scale = corner_scale;
}

/*==========================================================================*/
/**
 *  @breif  Set the division size.
 *  @param  division [in] division size
 */
/*==========================================================================*/
void Bounds::setCircleDivision( const float division )
{
    m_division = division;
}

/*==========================================================================*/
/**
 *  @breif  Initialize the parameters.
 */
/*==========================================================================*/
void Bounds::initialize( void )
{
    m_type         = Bounds::Box;
    m_corner_scale = 0.2f;
    m_division     = 20.0f;
}

/*==========================================================================*/
/**
 *  @breif  Create a box type bounds.
 */
/*==========================================================================*/
void Bounds::create_box_bounds( void )
{
    std::vector<kvs::Real32> coords;
    std::vector<kvs::UInt32> connects;

    const kvs::Real32 min_x( SuperClass::minObjectCoord().x() );
    const kvs::Real32 min_y( SuperClass::minObjectCoord().y() );
    const kvs::Real32 min_z( SuperClass::minObjectCoord().z() );
    const kvs::Real32 max_x( SuperClass::maxObjectCoord().x() );
    const kvs::Real32 max_y( SuperClass::maxObjectCoord().y() );
    const kvs::Real32 max_z( SuperClass::maxObjectCoord().z() );

    coords.push_back( min_x ); coords.push_back( min_y ); coords.push_back( max_z ); //0
    coords.push_back( max_x ); coords.push_back( min_y ); coords.push_back( max_z ); //1
    coords.push_back( max_x ); coords.push_back( min_y ); coords.push_back( min_z ); //2
    coords.push_back( min_x ); coords.push_back( min_y ); coords.push_back( min_z ); //3
    coords.push_back( min_x ); coords.push_back( max_y ); coords.push_back( max_z ); //4
    coords.push_back( max_x ); coords.push_back( max_y ); coords.push_back( max_z ); //5
    coords.push_back( max_x ); coords.push_back( max_y ); coords.push_back( min_z ); //6
    coords.push_back( min_x ); coords.push_back( max_y ); coords.push_back( min_z ); //7

    // x_line
    connects.push_back( 0 ); connects.push_back( 1 );
    connects.push_back( 3 ); connects.push_back( 2 );
    connects.push_back( 4 ); connects.push_back( 5 );
    connects.push_back( 7 ); connects.push_back( 6 );

    // y_line
    connects.push_back( 4 ); connects.push_back( 0 );
    connects.push_back( 5 ); connects.push_back( 1 );
    connects.push_back( 7 ); connects.push_back( 3 );
    connects.push_back( 6 ); connects.push_back( 2 );

    // z_line
    connects.push_back( 0 ); connects.push_back( 3 );
    connects.push_back( 1 ); connects.push_back( 2 );
    connects.push_back( 4 ); connects.push_back( 7 );
    connects.push_back( 5 ); connects.push_back( 6 );

    SuperClass::setLineType( kvs::LineObject::Segment );
    SuperClass::setColorType( kvs::LineObject::LineColor );
    SuperClass::setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    SuperClass::setConnections( kvs::ValueArray<kvs::UInt32>( connects ) );
    SuperClass::setColor( kvs::RGBColor( 0, 0, 0 ) );
    SuperClass::setSize( 1.0f );
}

/*==========================================================================*/
/**
 *  @breif  Create a corner type bounds.
 */
/*==========================================================================*/
void Bounds::create_corner_bounds( void )
{
    std::vector<kvs::Real32> coords;
    std::vector<kvs::UInt32> connects;

    const kvs::Real32 min_x( SuperClass::minObjectCoord().x() );
    const kvs::Real32 min_y( SuperClass::minObjectCoord().y() );
    const kvs::Real32 min_z( SuperClass::minObjectCoord().z() );
    const kvs::Real32 max_x( SuperClass::maxObjectCoord().x() );
    const kvs::Real32 max_y( SuperClass::maxObjectCoord().y() );
    const kvs::Real32 max_z( SuperClass::maxObjectCoord().z() );

    const kvs::Real32 corner =
        kvs::Math::Min( max_x-min_x, max_y-min_y, max_z-min_z ) * m_corner_scale;

    const kvs::Real32 max_ext_x( max_x - corner );
    const kvs::Real32 max_ext_y( max_y - corner );
    const kvs::Real32 max_ext_z( max_z - corner );
    const kvs::Real32 min_ext_x( min_x + corner );
    const kvs::Real32 min_ext_y( min_y + corner );
    const kvs::Real32 min_ext_z( min_z + corner );

    this->set_corner( kvs::Vector3f( min_x,     min_y,     min_z ),
                      kvs::Vector3f( min_ext_x, min_ext_y, min_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( max_x,     min_y,     min_z ),
                      kvs::Vector3f( max_ext_x, min_ext_y, min_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( max_x,     min_y,     max_z ),
                      kvs::Vector3f( max_ext_x, min_ext_y, max_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( min_x,     min_y,     max_z ),
                      kvs::Vector3f( min_ext_x, min_ext_y, max_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( min_x,     max_y,     min_z ),
                      kvs::Vector3f( min_ext_x, max_ext_y, min_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( max_x,     max_y,     min_z ),
                      kvs::Vector3f( max_ext_x, max_ext_y, min_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( max_x,     max_y,     max_z ),
                      kvs::Vector3f( max_ext_x, max_ext_y, max_ext_z ),
                      &coords, &connects );
    this->set_corner( kvs::Vector3f( min_x,     max_y,     max_z ),
                      kvs::Vector3f( min_ext_x, max_ext_y, max_ext_z ),
                      &coords, &connects );

    SuperClass::setLineType( kvs::LineObject::Segment );
    SuperClass::setColorType( kvs::LineObject::LineColor );
    SuperClass::setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    SuperClass::setConnections( kvs::ValueArray<kvs::UInt32>( connects ) );
    SuperClass::setColor( kvs::RGBColor( 0, 0, 0 ) );
    SuperClass::setSize( 1.0f );
}

/*==========================================================================*/
/**
 *  @breif  Create a circel type bounds.
 */
/*==========================================================================*/
void Bounds::create_circle_bounds( void )
{
    std::vector<kvs::Real32> coords;
    std::vector<kvs::UInt32> connects;

    const kvs::Real32 min_x( SuperClass::minObjectCoord().x() );
    const kvs::Real32 min_y( SuperClass::minObjectCoord().y() );
    const kvs::Real32 min_z( SuperClass::minObjectCoord().z() );
    const kvs::Real32 max_x( SuperClass::maxObjectCoord().x() );
    const kvs::Real32 max_y( SuperClass::maxObjectCoord().y() );
    const kvs::Real32 max_z( SuperClass::maxObjectCoord().z() );

    const kvs::Vector3f interval = kvs::Vector3f( max_x-min_x, max_y-min_y, max_z-min_z );
    const kvs::Vector3f center = kvs::Vector3f( max_x+min_x, max_y+min_y, max_z+min_z ) * 0.5f;

    const float pi = static_cast<float>( kvs::Math::pi );
    const float diff_angle = ( 2.0f * pi ) / m_division;

    const float x1 = interval.x() * 0.5f;
    const float y1 = interval.y() * 0.5f;
    const float z1 = interval.z() * 0.5f;

    const float sqrt2 = static_cast<float>( kvs::Math::sqrt2 );
    const float a = sqrt2 * x1;  const float a2 = a * a;
    const float b = sqrt2 * y1;  const float b2 = b * b;
    const float c = sqrt2 * z1;  const float c2 = c * c;

    size_t vertex_id = 0;
    const size_t division = static_cast<size_t>( m_division );

    // Circle 1 (X-Y axis)
    const float a2b2 = a2 * b2;
    for( size_t i = 0; i <= division; i++ )
    {
        const float angle = diff_angle * i;

        const float sin  = std::sin( angle );
        const float cos  = std::cos( angle );
        const float sin2 = sin * sin;
        const float cos2 = cos * cos;

        const float r = std::sqrt( a2b2 / ( a2 * sin2 + b2 * cos2 ) );

        coords.push_back( r * cos + center.x() );
        coords.push_back( r * sin + center.y() );
        coords.push_back( center.z() );
    }

    connects.push_back( vertex_id );
    vertex_id = coords.size() / 3 - 1;
    connects.push_back( vertex_id );
    vertex_id++;

    // Circle 2 (Y-Z axis)
    const float b2c2 = b2 * c2;
    for( size_t i = 0; i <= division; i++ )
    {
        const float angle = diff_angle * i;

        const float sin  = std::sin( angle );
        const float cos  = std::cos( angle );
        const float sin2 = sin * sin;
        const float cos2 = cos * cos;

        const float r = std::sqrt( b2c2 / ( b2 * sin2 + c2 * cos2 ) );

        coords.push_back( center.x() );
        coords.push_back( r * cos + center.y() );
        coords.push_back( r * sin + center.z() );
    }

    connects.push_back( vertex_id );
    vertex_id = coords.size() / 3 - 1;
    connects.push_back( vertex_id );
    vertex_id++;

    // Circle 3 (Z-X axis)
    const float c2a2 = c2 * a2;
    for( size_t i = 0; i <= division; i++ )
    {
        const float angle = diff_angle * i;

        const float sin  = std::sin( angle );
        const float cos  = std::cos( angle );
        const float sin2 = sin * sin;
        const float cos2 = cos * cos;

        const float r = std::sqrt( c2a2 / ( c2 * sin2 + a2 * cos2 ) );

        coords.push_back( r * sin + center.x() );
        coords.push_back( center.y() );
        coords.push_back( r * cos + center.z() );
    }

    connects.push_back( vertex_id );
    vertex_id = coords.size() / 3 - 1;
    connects.push_back( vertex_id );

    SuperClass::setLineType( kvs::LineObject::Polyline );
    SuperClass::setColorType( kvs::LineObject::LineColor );
    SuperClass::setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    SuperClass::setConnections( kvs::ValueArray<kvs::UInt32>( connects ) );
    SuperClass::setColor( kvs::RGBColor( 0, 0, 0 ) );
    SuperClass::setSize( 1.0f );
}

/*==========================================================================*/
/**
 *  @breif  Set cornet lines.
 *  @param  v1 [in] vertex #1
 *  @param  v2 [in] vertex #2
 *  @param  coords [out] pointer to the coordinate array
 *  @param  connects [out] pointer to the connection array
 */
/*==========================================================================*/
void Bounds::set_corner(
    const kvs::Vector3f&      v1,
    const kvs::Vector3f&      v2,
    std::vector<kvs::Real32>* coords,
    std::vector<kvs::UInt32>* connects )
{
    coords->push_back( v1.x() );
    coords->push_back( v1.y() );
    coords->push_back( v1.z() );

    coords->push_back( v2.x() );
    coords->push_back( v1.y() );
    coords->push_back( v1.z() );

    coords->push_back( v1.x() );
    coords->push_back( v2.y() );
    coords->push_back( v1.z() );

    coords->push_back( v1.x() );
    coords->push_back( v1.y() );
    coords->push_back( v2.z() );

    const size_t dimension = 3;
    const size_t nvertices = coords->size() / dimension;

    connects->push_back( nvertices - 4 );
    connects->push_back( nvertices - 3 );
    connects->push_back( nvertices - 4 );
    connects->push_back( nvertices - 2 );
    connects->push_back( nvertices - 4 );
    connects->push_back( nvertices - 1 );
}

} // end of namespace kvs
