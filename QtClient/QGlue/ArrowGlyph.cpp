/*****************************************************************************/
/**
 *  @file   ArrowGlyph.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ArrowGlyph.cpp 1797 2014-08-04 01:36:37Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "ArrowGlyph.h"
#include <kvs/OpenGL>
#include <kvs/IgnoreUnusedVariable>

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

const kvs::Vec3 ConeTranslation = kvs::Vec3( 0.0f, 0.0f, 0.7f );
inline void DrawCone( const kvs::Vec3& t )
{
    const GLdouble base = 0.15;
    const GLdouble top = 0.0;
    const GLdouble height = 0.3;
    const GLint slices = 20;
    const GLint stacks = 5;
    kvs::OpenGL::Translate( t );
    kvs::OpenGL::DrawCylinder( base, top, height, slices, stacks );
}

const kvs::Vec3 CylinderTranslation = kvs::Vec3( 0.0f, 0.0f, 0.0f );
inline void DrawCylinder( const kvs::Vec3& t )
{
    const GLdouble base = 0.07;
    const GLdouble top = 0.07;
    const GLdouble height = 0.7;
    const GLint slices = 20;
    const GLint stacks = 2;
    kvs::OpenGL::Translate( t );
    kvs::OpenGL::DrawCylinder( base, top, height, slices, stacks );
}

}; // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ArrowGlyph class.
 */
/*===========================================================================*/
ArrowGlyph::ArrowGlyph():
    kvs::GlyphBase(),
    m_type( ArrowGlyph::LineArrow ),
    m_volume( NULL )
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
    m_type( ArrowGlyph::LineArrow ),
    m_volume( NULL )
{
    this->attach_volume( volume );
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
    m_type( ArrowGlyph::LineArrow )
{
    BaseClass::setTransferFunction( transfer_function );
    this->attach_volume( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] number of glyphs & positions
 */
/*===========================================================================*/
ArrowGlyph::ArrowGlyph( const int nglyphs, const kvs::ValueArray<kvs::Real32>& position ):
    kvs::GlyphBase(),
    m_size(1.0f),
    m_opacity( 255),
    m_glyph_num(nglyphs)
{
    kvs::RGBColor color(255, 255, 255 );
    this->setPosition( position );
    this->setRGBColor( color);
    this->setType( kvs::ArrowGlyph::TubeArrow );
    this->setNormalType( kvs::ArrowGlyph::VertexNormal );
    this->ConvertPolygon();
}


/*===========================================================================*/
/**
 *  @brief  Executes rendering process.
 *  @param  object [in] pointer to the volume object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void ArrowGlyph::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );
    kvs::IgnoreUnusedVariable( camera );

    const kvs::VolumeObjectBase* volume = kvs::VolumeObjectBase::DownCast( object );
    if ( !volume ) { kvsMessageError("Input object is not volume dat."); return; }
    if ( m_volume != volume ) { this->attach_volume( volume ); }

    BaseClass::startTimer();

    kvs::OpenGL::WithPushedAttrib attrib( GL_CURRENT_BIT | GL_ENABLE_BIT );
    kvs::OpenGL::Enable( GL_DEPTH_TEST );
    this->initialize();
    this->draw();

    BaseClass::stopTimer();
}

/*===========================================================================*/
/**
 *  @brief  Attaches a volume object.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void ArrowGlyph::attach_volume( const kvs::VolumeObjectBase* volume )
{
    m_volume = volume;

    BaseClass::calculateCoords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        BaseClass::calculateSizes<kvs::Int8>( volume );
        BaseClass::calculateDirections<kvs::Int8>( volume );
        BaseClass::calculateColors<kvs::Int8>( volume );
        BaseClass::calculateOpacities<kvs::Int8>( volume );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        BaseClass::calculateSizes<kvs::Int16>( volume );
        BaseClass::calculateDirections<kvs::Int16>( volume );
        BaseClass::calculateColors<kvs::Int16>( volume );
        BaseClass::calculateOpacities<kvs::Int16>( volume );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        BaseClass::calculateSizes<kvs::Int32>( volume );
        BaseClass::calculateDirections<kvs::Int32>( volume );
        BaseClass::calculateColors<kvs::Int32>( volume );
        BaseClass::calculateOpacities<kvs::Int32>( volume );
    }
    else if ( type == typeid( kvs::Int64 ) )
    {
        BaseClass::calculateSizes<kvs::Int64>( volume );
        BaseClass::calculateDirections<kvs::Int64>( volume );
        BaseClass::calculateColors<kvs::Int64>( volume );
        BaseClass::calculateOpacities<kvs::Int64>( volume );
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
        BaseClass::calculateSizes<kvs::UInt8>( volume );
        BaseClass::calculateDirections<kvs::UInt8>( volume );
        BaseClass::calculateColors<kvs::UInt8>( volume );
        BaseClass::calculateOpacities<kvs::UInt8>( volume );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        BaseClass::calculateSizes<kvs::UInt16>( volume );
        BaseClass::calculateDirections<kvs::UInt16>( volume );
        BaseClass::calculateColors<kvs::UInt16>( volume );
        BaseClass::calculateOpacities<kvs::UInt16>( volume );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        BaseClass::calculateSizes<kvs::UInt32>( volume );
        BaseClass::calculateDirections<kvs::UInt32>( volume );
        BaseClass::calculateColors<kvs::UInt32>( volume );
        BaseClass::calculateOpacities<kvs::UInt32>( volume );
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
        BaseClass::calculateSizes<kvs::UInt64>( volume );
        BaseClass::calculateDirections<kvs::UInt64>( volume );
        BaseClass::calculateColors<kvs::UInt64>( volume );
        BaseClass::calculateOpacities<kvs::UInt64>( volume );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        BaseClass::calculateSizes<kvs::Real32>( volume );
        BaseClass::calculateDirections<kvs::Real32>( volume );
        BaseClass::calculateColors<kvs::Real32>( volume );
        BaseClass::calculateOpacities<kvs::Real32>( volume );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        BaseClass::calculateSizes<kvs::Real64>( volume );
        BaseClass::calculateDirections<kvs::Real64>( volume );
        BaseClass::calculateColors<kvs::Real64>( volume );
        BaseClass::calculateOpacities<kvs::Real64>( volume );
    }
}

/*===========================================================================*/
/**
 *  @brief  Draw the arrow glyph.
 */
/*===========================================================================*/
void ArrowGlyph::draw()
{
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
void ArrowGlyph::draw_lines()
{
    const size_t npoints = BaseClass::coords().size() / 3;
    const kvs::ValueArray<kvs::Real32> coords = BaseClass::coords();
    const kvs::ValueArray<kvs::UInt8> colors = BaseClass::colors();
    const kvs::ValueArray<kvs::Real32> sizes = BaseClass::sizes();
    const kvs::ValueArray<kvs::UInt8> opacities = BaseClass::opacities();

    if ( BaseClass::directions().size() == 0 )
    {
        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( coords.data() + index );
            const kvs::Real32 size = sizes[i];
            const kvs::RGBColor color( colors.data() + index );
            const kvs::UInt8 opacity = opacities[i];
            kvs::OpenGL::PushMatrix();
            {
                BaseClass::transform( position, size );
                this->draw_line_element( color, opacity );
            }
            kvs::OpenGL::PopMatrix();
        }
    }
    else
    {
        for( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( coords.data() + index );
            const kvs::Vector3f direction( BaseClass::directions().data() + index );
            const kvs::Real32 size = sizes[i];
            const kvs::RGBColor color( colors.data() + index );
            const kvs::UInt8 opacity = opacities[i];
            if ( direction.length() > 0.0f )
            {
                kvs::OpenGL::PushMatrix();
                {
                    BaseClass::transform( position, direction, size );
                    this->draw_line_element( color, opacity );
                }
                kvs::OpenGL::PopMatrix();
            }
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Draw the arrow glyph as polygons.
 */
/*===========================================================================*/
void ArrowGlyph::draw_tubes()
{
    const size_t npoints = BaseClass::coords().size() / 3;
    const kvs::ValueArray<kvs::Real32> coords = BaseClass::coords();
    const kvs::ValueArray<kvs::UInt8> colors = BaseClass::colors();
    const kvs::ValueArray<kvs::Real32> sizes = BaseClass::sizes();
    const kvs::ValueArray<kvs::UInt8> opacities = BaseClass::opacities();

    if ( BaseClass::directions().size() == 0 )
    {
        //        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        //        {
        //            const kvs::Vector3f position( coords.data() + index );
        //            const kvs::Real32 size = sizes[i];
        //            const kvs::RGBColor color( colors.data() + index );
        //            const kvs::UInt8 opacity = opacities[i];
        //            kvs::OpenGL::PushMatrix();
        //            {
        //                BaseClass::transform( position, size );
        //                this->draw_tube_element( color, opacity );
        //            }
        //            kvs::OpenGL::PopMatrix();
        //        }
        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( coords.data() + index );
            const kvs::Real32 size = sizes[i];
            const kvs::RGBColor color( colors.data() + index );
            const kvs::UInt8 opacity = opacities[i];
            kvs::OpenGL::PushMatrix();
            {
                BaseClass::transform( position, size );
                this->draw_tube_element( color, opacity );
            }
            kvs::OpenGL::PopMatrix();
        }
    }
    else
    {
        for( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( coords.data() + index );
            const kvs::Vector3f direction( BaseClass::directions().data() + index );
            const kvs::Real32 size = sizes[i];
            const kvs::RGBColor color( colors.data() + index );
            const kvs::UInt8 opacity = opacities[i];
            if ( direction.length() > 0.0f )
            {
                kvs::OpenGL::PushMatrix();
                {
                    BaseClass::transform( position, direction, size );
                    this->draw_tube_element( color, opacity );
                }
                kvs::OpenGL::PopMatrix();
            }
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
    kvs::OpenGL::Begin( GL_LINES );
    kvs::OpenGL::Color( color.r(), color.g(), color.b(), opacity );
    for ( size_t i = 0; i < 6; i++ )
    {
        kvs::OpenGL::Vertex3( ::LineVertices + ::LineConnections[i] * 3 );
    }
    kvs::OpenGL::End();
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
    const kvs::Real32 R = -90.0f; // rotation angle
    const kvs::Vec3 V( 1.0f, 0.0f, 0.0f ); // rotation vector
    const kvs::Vec3 T0( 0.0f, 0.0f, 0.7f ); // translation vector (cone)
    const kvs::Vec3 T1( 0.0f, 0.0f, 0.0f ); // translation vector (cylinder)

    kvs::OpenGL::PushMatrix();
    kvs::OpenGL::Rotate( R, V );
    {
        kvs::OpenGL::Color( color.r(), color.g(), color.b(), opacity );

        // Cone.
        kvs::OpenGL::PushMatrix();
        ::DrawCone( T0 );
        kvs::OpenGL::PopMatrix();

        // Cylinder.
        kvs::OpenGL::PushMatrix();
        ::DrawCylinder( T1 );
        kvs::OpenGL::PopMatrix();
    }
    kvs::OpenGL::PopMatrix();
}


void  ArrowGlyph::ConvertPolygon()
{
    const GLdouble radius = static_cast<GLdouble>( m_size );
    const kvs::Real32 R = -90.0f; // rotation angle
    const kvs::Vec3 V( 1.0f, 0.0f, 0.0f ); // rotation vector
    const kvs::Vec3 T0( 0.0f, 0.0f, 0.7f ); // translation vector (cone)
    const kvs::Vec3 T1( 0.0f, 0.0f, 0.0f ); // translation vector (cylinder)

    int  n_coords     = 189; // =(20+1)*(5+1) + (20+1)*(2+1)
    int  n_cells      = 294; // =2*((20+1)*5+(20+1)*2)
    int  n_connection = 3 * n_cells;          //tri cell type
    kvs::ValueArray<kvs::Real32> local_coords;
    kvs::ValueArray<kvs::UInt32> local_connection;
    kvs::ValueArray<kvs::Real32> local_normal;
    local_coords.allocate(m_glyph_num*3*n_coords);
    local_normal.allocate(m_glyph_num*3*n_coords);
    local_connection.allocate(m_glyph_num*n_connection);

    int coord_id = 0;
    int connection_id = 0;

    //kvs::OpenGL::Rotate( R, V );
    for (int gly_id = 0; gly_id<m_glyph_num; gly_id++ )
    {

        GLfloat t_x = m_position.at(gly_id*3 + 0);
        GLfloat t_y = m_position.at(gly_id*3 + 1);
        GLfloat t_z = m_position.at(gly_id*3 + 2);
        // Cone.
        //        ::DrawCone( T0 );
        GLdouble base = 0.15;
        GLdouble top = 0.0;
        GLdouble height = 0.3;
        GLint slices = 20;
        GLint stacks = 5;

        //    kvs::OpenGL::Translate( t ); //use T0(( 0.0f, 0.0f, 0.7f ))
        //    kvs::OpenGL::DrawCylinder( base, top, height, slices, stacks );
        const int CacheSize = 240;
        const float Pi = 3.14159265358979323846;

        GLint i,j;
        GLfloat sinCache[CacheSize];
        GLfloat cosCache[CacheSize];
        GLfloat sinCache2[CacheSize];
        GLfloat cosCache2[CacheSize];
        GLfloat sinCache3[CacheSize];
        GLfloat cosCache3[CacheSize];
        GLfloat angle;
        GLfloat zLow, zHigh;
        GLfloat length;
        GLfloat deltaRadius;
        GLfloat zNormal;
        GLfloat xyNormalRatio;
        GLfloat radiusLow, radiusHigh;
        int needCache2, needCache3;

        if (slices >= CacheSize) slices = CacheSize-1;

        if (slices < 2 || stacks < 1 || base < 0.0 || top < 0.0 || height < 0.0)
        {
            kvsMessageError("Invalid value.");
            return;
        }

        /* Compute length (needed for normal calculations) */
        deltaRadius = base - top;
        length = std::sqrt(deltaRadius*deltaRadius + height*height);
        if ( length == 0.0 )
        {
            kvsMessageError("Invalid value.");
            return;
        }

        /* Cache is the vertex locations cache */
        /* Cache2 is the various normals at the vertices themselves */
        /* Cache3 is the various normals for the faces */
        needCache2 = 1;
        needCache3 = 0;

        zNormal = deltaRadius / length;
        xyNormalRatio = height / length;

        for (i = 0; i < slices; i++)
        {
            angle = 2 * Pi * i / slices;
            if (needCache2)
            {
                sinCache2[i] = xyNormalRatio * std::sin(angle);
                cosCache2[i] = xyNormalRatio * std::cos(angle);
            }
            sinCache[i] = std::sin(angle);
            cosCache[i] = std::cos(angle);
        }

        if (needCache3)
        {
            for (i = 0; i < slices; i++)
            {
                angle = 2 * Pi * (i-0.5) / slices;
                sinCache3[i] = xyNormalRatio * std::sin(angle);
                cosCache3[i] = xyNormalRatio * std::cos(angle);
            }
        }

        sinCache[slices] = sinCache[0];
        cosCache[slices] = cosCache[0];
        if (needCache2)
        {
            sinCache2[slices] = sinCache2[0];
            cosCache2[slices] = cosCache2[0];
        }
        if (needCache3)
        {
            sinCache3[slices] = sinCache3[0];
            cosCache3[slices] = cosCache3[0];
        }

        /* Note:
         ** An argument could be made for using a TRIANGLE_FAN for the end
         ** of the cylinder of either radii is 0.0 (a cone).  However, a
         ** TRIANGLE_FAN would not work in smooth shading mode (the common
         ** case) because the normal for the apex is different for every
         ** triangle (and TRIANGLE_FAN doesn't let me respecify that normal).
         ** Now, my choice is GL_TRIANGLES, or leave the GL_QUAD_STRIP and
         ** just let the GL trivially reject one of the two triangles of the
         ** QUAD.  GL_QUAD_STRIP is probably faster, so I will leave this code
         ** alone.
         */
        for (j = 0; j <= stacks; j++)
        {
            zLow = j * height / stacks;
            //        zHigh = (j + 1) * height / stacks;
            radiusLow = base - deltaRadius * ((float) j / stacks);
            //        radiusHigh = base - deltaRadius * ((float) (j + 1) / stacks);

            for (i = 0; i <= slices; i++)
            {
                //add by shimomura
                local_coords.at(coord_id   ) = t_x + T0.x() + radiusLow  * sinCache[i];
                local_coords.at(coord_id +1) = t_y + T0.y() + radiusLow  * cosCache[i];
                local_coords.at(coord_id +2) = t_z + T0.z() + zLow;

                local_normal.at(coord_id   ) = sinCache2[i] ;
                local_normal.at(coord_id +1) = cosCache2[i] ;
                local_normal.at(coord_id +2) = zNormal;
                coord_id      +=3;
            }
        }
//        std::cout  <<  __PRETTY_FUNCTION__ <<" : "<<__LINE__ << std::endl;
        int line_size = slices + 1;
        int vertex_index = 0;
        for (j = 0; j < stacks; j++)
        {
            for (i = 0; i <= slices; i++)
            {
                const int local_vertex_index[4] =
                    {
                        vertex_index,
                        vertex_index + 1,
                        vertex_index + line_size,
                        vertex_index + line_size + 1
                    };
                local_connection.at(connection_id  ) = gly_id*n_coords +(local_vertex_index[0]);
                local_connection.at(connection_id+1) = gly_id*n_coords +(local_vertex_index[2]);
                local_connection.at(connection_id+2) = gly_id*n_coords +(local_vertex_index[1]);

                local_connection.at(connection_id+3) = gly_id*n_coords +(local_vertex_index[2]);
                local_connection.at(connection_id+4) = gly_id*n_coords +(local_vertex_index[3]);
                local_connection.at(connection_id+5) = gly_id*n_coords +(local_vertex_index[1]);
                connection_id += 6;
                vertex_index++;
            }
        }

        // Cylinder.
        //::DrawCylinder( T1 ) // use T1( 0.0f, 0.0f, 0.0f );
        base = 0.07;
        top = 0.07;
        height = 0.7;
        slices = 20;
        stacks = 2;

        /* Compute length (needed for normal calculations) */
        deltaRadius = base - top;
        length = std::sqrt(deltaRadius*deltaRadius + height*height);
        if ( length == 0.0 )
        {
            kvsMessageError("Invalid value.");
            return;
        }

        /* Cache is the vertex locations cache */
        /* Cache2 is the various normals at the vertices themselves */
        /* Cache3 is the various normals for the faces */
        needCache2 = 1;
        needCache3 = 0;

        zNormal = deltaRadius / length;
        xyNormalRatio = height / length;

        for (i = 0; i < slices; i++)
        {
            angle = 2 * Pi * i / slices;
            if (needCache2)
            {
                sinCache2[i] = xyNormalRatio * std::sin(angle);
                cosCache2[i] = xyNormalRatio * std::cos(angle);
            }
            sinCache[i] = std::sin(angle);
            cosCache[i] = std::cos(angle);
        }

        if (needCache3)
        {
            for (i = 0; i < slices; i++)
            {
                angle = 2 * Pi * (i-0.5) / slices;
                sinCache3[i] = xyNormalRatio * std::sin(angle);
                cosCache3[i] = xyNormalRatio * std::cos(angle);
            }
        }

        sinCache[slices] = sinCache[0];
        cosCache[slices] = cosCache[0];
        if (needCache2)
        {
            sinCache2[slices] = sinCache2[0];
            cosCache2[slices] = cosCache2[0];
        }
        if (needCache3)
        {
            sinCache3[slices] = sinCache3[0];
            cosCache3[slices] = cosCache3[0];
        }

        /* Note:
         ** An argument could be made for using a TRIANGLE_FAN for the end
         ** of the cylinder of either radii is 0.0 (a cone).  However, a
         ** TRIANGLE_FAN would not work in smooth shading mode (the common
         ** case) because the normal for the apex is different for every
         ** triangle (and TRIANGLE_FAN doesn't let me respecify that normal).
         ** Now, my choice is GL_TRIANGLES, or leave the GL_QUAD_STRIP and
         ** just let the GL trivially reject one of the two triangles of the
         ** QUAD.  GL_QUAD_STRIP is probably faster, so I will leave this code
         ** alone.
         */
        for (j = 0; j <= stacks; j++)
        {
            zLow = j * height / stacks;
            //        zHigh = (j + 1) * height / stacks;
            radiusLow = base - deltaRadius * ((float) j / stacks);
            //        radiusHigh = base - deltaRadius * ((float) (j + 1) / stacks);

            for (i = 0; i <= slices; i++)
            {
                //add by shimomura
                local_coords.at(coord_id   ) = t_x + radiusLow  * sinCache[i];
                local_coords.at(coord_id +1) = t_y + radiusLow  * cosCache[i];
                local_coords.at(coord_id +2) = t_z + zLow;

                local_normal.at(coord_id   ) = sinCache2[i] ;
                local_normal.at(coord_id +1) = cosCache2[i] ;
                local_normal.at(coord_id +2) = zNormal;
                coord_id      +=3;
            }
        }
        //        line_size = slices + 1;
        vertex_index = 126;
        std::cout  << "vertex_index = " << vertex_index <<std::endl;
        for (j = 0; j < stacks; j++)
        {
            for (i = 0; i <= slices; i++)
            {
                const int local_vertex_index[4] =
                    {
                        vertex_index,
                        vertex_index + 1,
                        vertex_index + line_size,
                        vertex_index + line_size + 1
                    };
                local_connection.at(connection_id  ) = gly_id*n_coords +(local_vertex_index[0]);
                local_connection.at(connection_id+1) = gly_id*n_coords +(local_vertex_index[2]);
                local_connection.at(connection_id+2) = gly_id*n_coords +(local_vertex_index[1]);

                local_connection.at(connection_id+3) = gly_id*n_coords +(local_vertex_index[2]);
                local_connection.at(connection_id+4) = gly_id*n_coords +(local_vertex_index[3]);
                local_connection.at(connection_id+5) = gly_id*n_coords +(local_vertex_index[1]);
                connection_id += 6;
                vertex_index++;
            }
        }
#if 0
        std::cout << "connection_id = " << connection_id <<std::endl;
        std::cout << "coord_id = " << coord_id <<std::endl;
        std::cout << "object->coord()  = " << float(local_coords[0]) << ", " <<  float(local_coords[1]) << ", " << float(local_coords[2]) << ", "  <<
            float(local_coords[402]) << ", " <<  float(local_coords[403]) << ", " << float(local_coords[404]) << ", " <<
            float(local_coords[564]) << ", " <<  float(local_coords[565]) << ", " << float(local_coords[566]) << ", " << std::endl;
        std::cout << "connection[id]  = " << float(local_connection[0]) << ", " <<  float(local_connection[1]) << ", " << float(local_connection[2]) << ", "  <<
            float(local_connection[879]) << ", " <<  float(local_connection[880]) << ", " << float(local_connection[881]) << ", " << std::endl;
#endif
    }

    SuperClass::setCoords( local_coords  );
    SuperClass::setConnections( local_connection );
    SuperClass::setColor( m_color );
    SuperClass::setNormals( local_normal );
    SuperClass::setOpacity( int(m_opacity) );
    SuperClass::setPolygonType( kvs::PolygonObject::Triangle );
    SuperClass::setColorType( kvs::PolygonObject::PolygonColor );

}
/*===========================================================================*/
/**
 *  @brief  Initialize OpenGL properties for rendering arrow glyph.
 */
/*===========================================================================*/
void ArrowGlyph::initialize()
{
    kvs::OpenGL::SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    kvs::OpenGL::SetPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    kvs::OpenGL::SetShadeModel( GL_SMOOTH );
    kvs::OpenGL::SetColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    kvs::OpenGL::SetLightModel( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );

    kvs::OpenGL::Disable( GL_LINE_SMOOTH );
    kvs::OpenGL::Enable( GL_BLEND );
    kvs::OpenGL::Enable( GL_COLOR_MATERIAL );
    if ( m_type == ArrowGlyph::LineArrow )
    {
        kvs::OpenGL::Disable( GL_NORMALIZE );
        kvs::OpenGL::Disable( GL_LIGHTING );
    }
    else
    {
        if ( !BaseClass::isEnabledShading() )
        {
            kvs::OpenGL::Disable( GL_NORMALIZE );
            kvs::OpenGL::Disable( GL_LIGHTING );
        }
        else
        {
            kvs::OpenGL::Enable( GL_NORMALIZE );
            kvs::OpenGL::Enable( GL_LIGHTING );
        }
    }
}

} // end of namespace kvs
