/*****************************************************************************/
/**
 *  @file   ArrowGlyph.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ArrowGlyph.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ArrowGlyph.h"
#include <kvs/OpenGL>


namespace
{

const kvs::Real32 LineVertices[12] =
{
     0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 0.0f,
    -0.2f, 0.8f, 0.0f,
     0.2f, 0.8f, 0.0f
};
const kvs::UInt32 LineConnections[6] =
{
    0, 1,
    0, 2,
    0, 3
};

const kvs::Vector3f ConeTranslation = kvs::Vector3f( 0.0f, 0.0f, 0.7f );
const kvs::Real32 ConeHeight = 0.3f;
const kvs::Real32 ConeRadius = 0.15f;
const size_t ConeSlices = 20;
const size_t ConeStacks = 5;

const kvs::Vector3f CylinderTranslation = kvs::Vector3f( 0.0f, 0.0f, 0.0f );
const kvs::Real32 CylinderHeight = 0.7f;
const kvs::Real32 CylinderRadius = 0.07f;
const size_t CylinderSlices = 20;
const size_t CylinderStacks = 2;

}; // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ArrowGlyph class.
 */
/*===========================================================================*/
ArrowGlyph::ArrowGlyph( void ):
    kvs::GlyphBase(),
    m_cylinder(NULL),
    m_type( ArrowGlyph::LineArrow )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ArrowGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
ArrowGlyph::ArrowGlyph( const kvs::VolumeObjectBase* volume ):
    kvs::GlyphBase(),
    m_cylinder(NULL),
    m_type( ArrowGlyph::LineArrow )
{
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ArrowGlyph class.
 *  @param  volume [in] pointer to the Volume object
 *  @param  transfer_function [in] transfer function
 */
/*===========================================================================*/
ArrowGlyph::ArrowGlyph(
    const kvs::VolumeObjectBase* volume,
    const kvs::TransferFunction& transfer_function ):
    kvs::GlyphBase(),
    m_cylinder(NULL),
    m_type( ArrowGlyph::LineArrow )
{
    BaseClass::setTransferFunction( transfer_function );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the Arrow glyph class.
 */
/*===========================================================================*/
ArrowGlyph::~ArrowGlyph( void )
{
    gluDeleteQuadric( m_cylinder );
}

/*===========================================================================*/
/**
 *  @brief  Returs an arrow type.
 *  @return arrow type
 */
/*===========================================================================*/
const ArrowGlyph::ArrowType ArrowGlyph::type( void ) const
{
    return( m_type );
}

/*===========================================================================*/
/**
 *  @brief  Sets an arrow type.
 *  @param  type [in] arrow type
 */
/*===========================================================================*/
void ArrowGlyph::setType( const ArrowType type )
{
    m_type = type;
}

/*===========================================================================*/
/**
 *  @brief  Creates an arrow glyph object.
 *  @param  object [in] pointer to the input object (volume object)
 *  @return pointer to the created glyph object
 */
/*===========================================================================*/
ArrowGlyph::BaseClass::SuperClass* ArrowGlyph::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    const kvs::VolumeObjectBase* volume = kvs::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is not volume dat.");
        return( NULL );
    }

    BaseClass::attach_volume( volume );
    BaseClass::set_min_max_coords( volume, this );
    BaseClass::calculate_coords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        BaseClass::calculate_sizes<kvs::Int8>( volume );
        BaseClass::calculate_directions<kvs::Int8>( volume );
        BaseClass::calculate_colors<kvs::Int8>( volume );
        BaseClass::calculate_opacities<kvs::Int8>( volume );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        BaseClass::calculate_sizes<kvs::Int16>( volume );
        BaseClass::calculate_directions<kvs::Int16>( volume );
        BaseClass::calculate_colors<kvs::Int16>( volume );
        BaseClass::calculate_opacities<kvs::Int16>( volume );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        BaseClass::calculate_sizes<kvs::Int32>( volume );
        BaseClass::calculate_directions<kvs::Int32>( volume );
        BaseClass::calculate_colors<kvs::Int32>( volume );
        BaseClass::calculate_opacities<kvs::Int32>( volume );
    }
    else if ( type == typeid( kvs::Int64 ) )
    {
        BaseClass::calculate_sizes<kvs::Int64>( volume );
        BaseClass::calculate_directions<kvs::Int64>( volume );
        BaseClass::calculate_colors<kvs::Int64>( volume );
        BaseClass::calculate_opacities<kvs::Int64>( volume );
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
        BaseClass::calculate_sizes<kvs::UInt8>( volume );
        BaseClass::calculate_directions<kvs::UInt8>( volume );
        BaseClass::calculate_colors<kvs::UInt8>( volume );
        BaseClass::calculate_opacities<kvs::UInt8>( volume );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        BaseClass::calculate_sizes<kvs::UInt16>( volume );
        BaseClass::calculate_directions<kvs::UInt16>( volume );
        BaseClass::calculate_colors<kvs::UInt16>( volume );
        BaseClass::calculate_opacities<kvs::UInt16>( volume );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        BaseClass::calculate_sizes<kvs::UInt32>( volume );
        BaseClass::calculate_directions<kvs::UInt32>( volume );
        BaseClass::calculate_colors<kvs::UInt32>( volume );
        BaseClass::calculate_opacities<kvs::UInt32>( volume );
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
        BaseClass::calculate_sizes<kvs::UInt64>( volume );
        BaseClass::calculate_directions<kvs::UInt64>( volume );
        BaseClass::calculate_colors<kvs::UInt64>( volume );
        BaseClass::calculate_opacities<kvs::UInt64>( volume );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        BaseClass::calculate_sizes<kvs::Real32>( volume );
        BaseClass::calculate_directions<kvs::Real32>( volume );
        BaseClass::calculate_colors<kvs::Real32>( volume );
        BaseClass::calculate_opacities<kvs::Real32>( volume );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        BaseClass::calculate_sizes<kvs::Real64>( volume );
        BaseClass::calculate_directions<kvs::Real64>( volume );
        BaseClass::calculate_colors<kvs::Real64>( volume );
        BaseClass::calculate_opacities<kvs::Real64>( volume );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Draw the arrow glyph.
 */
/*===========================================================================*/
void ArrowGlyph::draw( void )
{
    if ( !m_cylinder )
    {
        m_cylinder = gluNewQuadric();
        gluQuadricDrawStyle( m_cylinder, GLU_FILL );
        gluQuadricNormals( m_cylinder, GLU_SMOOTH );
    }

    this->initialize();
    BaseClass::applyMaterial();

    switch ( m_type )
    {
    case LineArrow: this->draw_lines(); break;
    case TubeArrow: this->draw_tubes(); break;
    default: break;
    }
}

/*===========================================================================*/
/**
 *  @brief  Draw the arrow glyph as lines.
 */
/*===========================================================================*/
void ArrowGlyph::draw_lines( void )
{
    const size_t npoints = BaseClass::coords().size() / 3;

    if ( BaseClass::directions().size() == 0 )
    {
        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( BaseClass::coords().pointer() + index );
            const kvs::Real32 size = BaseClass::sizes().at(i);
            const kvs::RGBColor color( BaseClass::colors().pointer() + index );
            const kvs::UInt8 opacity = BaseClass::opacities().at(i);
            glPushMatrix();
            {
                BaseClass::transform( position, size );
                this->draw_line_element( color, opacity );
            }
            glPopMatrix();
        }
    }
    else
    {
        for( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( BaseClass::m_coords.pointer() + index );
            const kvs::Vector3f direction( BaseClass::m_directions.pointer() + index );
            const kvs::Real32 size = BaseClass::m_sizes[i];
            const kvs::RGBColor color( BaseClass::m_colors.pointer() + index );
            const kvs::UInt8 opacity = BaseClass::m_opacities[i];
            glPushMatrix();
            {
                BaseClass::transform( position, direction, size );
                this->draw_line_element( color, opacity );
            }
            glPopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Draw the arrow glyph as polygons.
 */
/*===========================================================================*/
void ArrowGlyph::draw_tubes( void )
{
    const size_t npoints = BaseClass::coords().size() / 3;

    if ( BaseClass::directions().size() == 0 )
    {
        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( BaseClass::coords().pointer() + index );
            const kvs::Real32 size = BaseClass::sizes().at(i);
            const kvs::RGBColor color( BaseClass::colors().pointer() + index );
            const kvs::UInt8 opacity = BaseClass::opacities().at(i);
            glPushMatrix();
            {
                BaseClass::transform( position, size );
                this->draw_tube_element( color, opacity );
            }
            glPopMatrix();
        }
    }
    else
    {
        for( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( BaseClass::m_coords.pointer() + index );
            const kvs::Vector3f direction( BaseClass::m_directions.pointer() + index );
            const kvs::Real32 size = BaseClass::m_sizes[i];
            const kvs::RGBColor color( BaseClass::m_colors.pointer() + index );
            const kvs::UInt8 opacity = BaseClass::m_opacities[i];
            glPushMatrix();
            {
                BaseClass::transform( position, direction, size );
                this->draw_tube_element( color, opacity );
            }
            glPopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Draw the line element.
 *  @param  color [in] color value
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void ArrowGlyph::draw_line_element( const kvs::RGBColor& color, const kvs::UInt8 opacity )
{
    glColor4ub( color.r(), color.g(), color.b(), opacity );

    glBegin( GL_LINES );
    for ( size_t i = 0; i < 6; i++ )
    {
        const kvs::Real32* vertex = ::LineVertices + ::LineConnections[i] * 3;
        glVertex3fv( vertex );
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Draw the tube element.
 *  @param  color [in] color value
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void ArrowGlyph::draw_tube_element( const kvs::RGBColor& color, const kvs::UInt8 opacity )
{
    glColor4ub( color.r(), color.g(), color.b(), opacity );

    glPushMatrix();
    {
        glRotated( -90.0, 1.0, 0.0, 0.0 );

        // Cone.
        glPushMatrix();
        glTranslatef(
            ::ConeTranslation.x(),
            ::ConeTranslation.y(),
            ::ConeTranslation.z() );
        gluCylinder(
            m_cylinder,
            ::ConeRadius,
            0.0,
            ::ConeHeight,
            ::ConeSlices,
            ::ConeStacks );
        glPopMatrix();

        // Cylinder.
        glPushMatrix();
        glTranslatef(
            ::CylinderTranslation.x(),
            ::CylinderTranslation.y(),
            ::CylinderTranslation.z() );
        gluCylinder(
            m_cylinder,
            ::CylinderRadius,
            ::CylinderRadius,
            ::CylinderHeight,
            ::CylinderSlices,
            ::CylinderStacks );
        glPopMatrix();
    }
    glPopMatrix();
}

/*===========================================================================*/
/**
 *  @brief  Initialize the modelview matrix.
 */
/*===========================================================================*/
void ArrowGlyph::initialize( void )
{
    glDisable( GL_LINE_SMOOTH );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glShadeModel( GL_SMOOTH );

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    if ( m_type == ArrowGlyph::LineArrow )
    {
        glDisable( GL_NORMALIZE );
        glDisable( GL_LIGHTING );
    }

    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
}

} // end of namespace kvs
