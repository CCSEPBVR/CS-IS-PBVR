/*****************************************************************************/
/**
 *  @file   SphereGlyph.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SphereGlyph.cpp 1808 2014-08-30 00:00:29Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "SphereGlyph.h"
#include <kvs/OpenGL>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph():
    kvs::GlyphBase(),
    m_nslices( 10 ),
    m_nstacks( 10 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph( const kvs::PointObject* point ):
    kvs::GlyphBase(),
    m_nslices( 10 ),
    m_nstacks( 10 )
{
    this->attach_point( point );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph( const kvs::VolumeObjectBase* volume ):
    kvs::GlyphBase(),
    m_nslices( 10 ),
    m_nstacks( 10 )
{
    this->attach_volume( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 *  @param  transfer_function [in] transfer function
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph(
    const kvs::VolumeObjectBase* volume,
    const kvs::TransferFunction& transfer_function ):
    kvs::GlyphBase(),
    m_nslices( 10 ),
    m_nstacks( 10 )
{
    BaseClass::setTransferFunction( transfer_function );
    this->attach_volume( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new SphereGlyph class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
SphereGlyph::SphereGlyph( const int nglyphs, const kvs::ValueArray<kvs::Real32>& position ):
    kvs::GlyphBase(),
    m_nslices( 10 ),
    m_nstacks( 10 ),
    m_opacity( 255),
    m_size(0.02f),
    m_glyph_num(nglyphs)
{
    kvs::RGBColor color(255, 255, 255 );
    this->setPosition( position );
    this->setRGBColor( color);
    this -> setNormalType(kvs::SphereGlyph::VertexNormal);
    this -> setpolygon();
}

///*===========================================================================*/
//void SphereGlyph::setPosition(const kvs::ValueArray<kvs::Real32>& position )
//{
//    m_position.allocate(position.size());
//    for(int i=0 ;i < position.size(); i++)
//    {
//        m_position.at(i) = position.at(i);
//    }
//}
//
///*===========================================================================*/
/**
 *  @brief  Executes rendering process.
 *  @param  object [in] pointer to the volume object
 *  @param  camera [in] pointer to the camera
 *  @param  light [in] pointer to the light
 */
/*===========================================================================*/
void SphereGlyph::exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light )
{
    kvs::IgnoreUnusedVariable( light );
    kvs::IgnoreUnusedVariable( camera );

    std::cout << "exec !!";
    if ( object->objectType() == kvs::ObjectBase::Geometry )
    {
        const kvs::GeometryObjectBase* geometry = kvs::GeometryObjectBase::DownCast( object );
        if ( geometry->geometryType() == kvs::GeometryObjectBase::Point )
        {
            const kvs::PointObject* point = kvs::PointObject::DownCast( geometry );
            if ( !point ) { kvsMessageError("Input object is not point dat."); return; }
            if ( m_point != point ) { this->attach_point( point ); }
        }
    }
    else if ( object->objectType() == kvs::ObjectBase::Volume )
    {
        const kvs::VolumeObjectBase* volume = kvs::VolumeObjectBase::DownCast( object );
        if ( !volume ) { kvsMessageError("Input object is not volume dat."); return; }
        if ( m_volume != volume ) { this->attach_volume( volume ); }
    }

    BaseClass::startTimer();
    kvs::OpenGL::WithPushedAttrib p( GL_ALL_ATTRIB_BITS );
    kvs::OpenGL::Enable( GL_DEPTH_TEST );
    this->initialize();
    this->draw();
    BaseClass::stopTimer();
}

/*===========================================================================*/
/**
 *  @brief  Draw the sphere glyph.
 */
/*===========================================================================*/
void SphereGlyph::draw()
{
    const size_t npoints = BaseClass::coords().size() / 3;
    if ( BaseClass::directions().size() == 0 )
    {
        //        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        //        {
        //            const kvs::Vector3f position( BaseClass::coords().data() + index );
        //            const kvs::Real32 size = BaseClass::sizes()[i];
        ////            const kvs::RGBColor color( BaseClass::colors().data() + index );
        ////            const kvs::UInt8 opacity = BaseClass::opacities()[i];
        //            const kvs::RGBColor color( 255, 255, 255 );
        ////            const kvs::RGBColor color( 0, 0, 0 );
        //            const kvs::UInt8 opacity = 255; //range 0-255?
        //            kvs::OpenGL::PushMatrix();
        //            BaseClass::transform( position, size );
        //            this->draw_element( color, opacity );
        //            kvs::OpenGL::PopMatrix();
        //        }
        //        for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        //        {
        //            const kvs::Vector3f position( 5,5,0 );
        //            const kvs::Real32 size = 1.0f;
        //            const kvs::RGBColor color( BaseClass::colors().data() + index );
        //            const kvs::UInt8 opacity = BaseClass::opacities()[i];
        //            const kvs::RGBColor color( 255, 255, 255 );
        //            const kvs::RGBColor color( 0, 0, 0 );
        //            const kvs::UInt8 opacity = 255; //range 0-255?
        //kvs::OpenGL::PushMatrix();
        //BaseClass::transform( m_position, m_size );
        //this->draw_element( m_color, m_opacity );
        //kvs::OpenGL::PopMatrix();
        //        }

    }
    else
    {
        for( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
        {
            const kvs::Vector3f position( BaseClass::coords().data() + index );
            const kvs::Vector3f direction( BaseClass::directions().data() + index );
            const kvs::Real32 size = BaseClass::sizes()[i];
            //            const kvs::RGBColor color( BaseClass::colors().data() + index );
            const kvs::UInt8 opacity = BaseClass::opacities()[i];
            const kvs::RGBColor color( 255, 255, 255 );
            //            const kvs::UInt8 opacity = 1;
            kvs::OpenGL::PushMatrix();
            BaseClass::transform( position, direction, size );
            this->draw_element( color, opacity );
            kvs::OpenGL::PopMatrix();
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Attaches a point object.
 *  @param  point [in] pointer to the point object
 */
/*===========================================================================*/
void SphereGlyph::attach_point( const kvs::PointObject* point )
{
    m_point = point;

    const size_t nvertices = point->numberOfVertices();

    BaseClass::setCoords( point->coords() );

    if ( BaseClass::directionMode() == BaseClass::DirectionByNormal )
    {
        if ( point->numberOfNormals() != 0 )
        {
            BaseClass::setDirections( point->normals() );
        }
    }

    if ( point->numberOfSizes() == 1 )
    {
        const kvs::Real32 size = point->size();
        kvs::ValueArray<kvs::Real32> sizes( nvertices );
        for ( size_t i = 0; i < nvertices; i++ ) sizes[i] = size;
        BaseClass::setSizes( sizes );
    }
    else
    {
        BaseClass::setSizes( point->sizes() );
    }

    if ( point->numberOfColors() == 1 )
    {
        const kvs::RGBColor color = point->color();
        kvs::ValueArray<kvs::UInt8> colors( nvertices * 3 );
        for ( size_t i = 0, j = 0; i < nvertices; i++, j += 3 )
        {
            colors[j]   = color.r();
            colors[j+1] = color.g();
            colors[j+2] = color.b();
        }
        BaseClass::setColors( colors );
    }
    else
    {
        BaseClass::setColors( point->colors() );
    }

    const kvs::UInt8 opacity = static_cast<kvs::UInt8>( 255 );
    kvs::ValueArray<kvs::UInt8> opacities( nvertices );
    for ( size_t i = 0; i < nvertices; i++ ) opacities[i] = opacity;
    BaseClass::setOpacities( opacities );
}

/*===========================================================================*/
/**
 *  @brief  Attaches a volume object.
 *  @param  volume [in] pointer to the volume object
 *  @return glyph object
 */
/*===========================================================================*/
void SphereGlyph::attach_volume( const kvs::VolumeObjectBase* volume )
{
    m_volume = volume;

    BaseClass::calculateCoords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        BaseClass::calculateSizes<kvs::Int8>( volume );
        BaseClass::calculateColors<kvs::Int8>( volume );
        BaseClass::calculateOpacities<kvs::Int8>( volume );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        BaseClass::calculateSizes<kvs::Int16>( volume );
        BaseClass::calculateColors<kvs::Int16>( volume );
        BaseClass::calculateOpacities<kvs::Int16>( volume );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        BaseClass::calculateSizes<kvs::Int32>( volume );
        BaseClass::calculateColors<kvs::Int32>( volume );
        BaseClass::calculateOpacities<kvs::Int32>( volume );
    }
    else if ( type == typeid( kvs::Int64 ) )
    {
        BaseClass::calculateSizes<kvs::Int64>( volume );
        BaseClass::calculateColors<kvs::Int64>( volume );
        BaseClass::calculateOpacities<kvs::Int64>( volume );
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
        BaseClass::calculateSizes<kvs::UInt8>( volume );
        BaseClass::calculateColors<kvs::UInt8>( volume );
        BaseClass::calculateOpacities<kvs::UInt8>( volume );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        BaseClass::calculateSizes<kvs::UInt16>( volume );
        BaseClass::calculateColors<kvs::UInt16>( volume );
        BaseClass::calculateOpacities<kvs::UInt16>( volume );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        BaseClass::calculateSizes<kvs::UInt32>( volume );
        BaseClass::calculateColors<kvs::UInt32>( volume );
        BaseClass::calculateOpacities<kvs::UInt32>( volume );
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
        BaseClass::calculateSizes<kvs::UInt64>( volume );
        BaseClass::calculateColors<kvs::UInt64>( volume );
        BaseClass::calculateOpacities<kvs::UInt64>( volume );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        BaseClass::calculateSizes<kvs::Real32>( volume );
        BaseClass::calculateColors<kvs::Real32>( volume );
        BaseClass::calculateOpacities<kvs::Real32>( volume );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        BaseClass::calculateSizes<kvs::Real64>( volume );
        BaseClass::calculateColors<kvs::Real64>( volume );
        BaseClass::calculateOpacities<kvs::Real64>( volume );
    }
}

//------------polygon object-------------
//----------output polygon format---------
//----------------------------------------
//# if 0
void SphereGlyph::setpolygon()
{
    //const GLdouble radius = 0.5;
    const GLdouble radius = static_cast<GLdouble>( m_size );
    GLint slices = static_cast<GLint>( m_nslices );
    GLint stacks = static_cast<GLint>( m_nstacks );

    const int CacheSize = 240;
    const float Pi = 3.14159265358979323846;


    GLint i,j;
    GLfloat sinCache1a[CacheSize];
    GLfloat cosCache1a[CacheSize];
    GLfloat sinCache2a[CacheSize];
    GLfloat cosCache2a[CacheSize];
    GLfloat sinCache3a[CacheSize];
    GLfloat cosCache3a[CacheSize];
    GLfloat sinCache1b[CacheSize];
    GLfloat cosCache1b[CacheSize];
    GLfloat sinCache2b[CacheSize];
    GLfloat cosCache2b[CacheSize];
    //    GLfloat sinCache3b[CacheSize];
    //    GLfloat cosCache3b[CacheSize];
    GLfloat angle;
    GLfloat zLow, zHigh;
    GLfloat sintemp1 = 0.0, sintemp2 = 0.0, sintemp3 = 0.0, sintemp4 = 0.0;
    GLfloat costemp3 = 0.0, costemp4 = 0.0;
    GLboolean needCache2, needCache3;
    GLint start, finish;

    if (slices >= CacheSize) slices = CacheSize-1;
    if (stacks >= CacheSize) stacks = CacheSize-1;
    if (slices < 2 || stacks < 1 || radius < 0.0)
    {
        kvsMessageError("Invalid value.");
        return;
    }
    /* Cache is the vertex locations cache */
    /* Cache2 is the various normals at the vertices themselves */
    /* Cache3 is the various normals for the faces */
    needCache2 = GL_TRUE;
    needCache3 = GL_FALSE;

    for (i = 0; i < slices; i++)
    {
        angle = 2 * Pi * i / slices;
        sinCache1a[i] = std::sin(angle);
        cosCache1a[i] = std::cos(angle);
        if (needCache2)
        {
            sinCache2a[i] = sinCache1a[i];
            cosCache2a[i] = cosCache1a[i];
        }
    }

    for (j = 0; j <= stacks; j++)
    {
        angle = Pi * j / stacks;
        if (needCache2)
        {
            sinCache2b[j] = std::sin(angle);
            cosCache2b[j] = std::cos(angle);
        }
        sinCache1b[j] = radius * std::sin(angle);
        cosCache1b[j] = radius * std::cos(angle);
    }
    /* Make sure it comes to a point */
    sinCache1b[0] = 0;
    sinCache1b[stacks] = 0;

    if (needCache3) {
        for (i = 0; i < slices; i++)
        {
            angle = 2 * Pi * (i-0.5) / slices;
            sinCache3a[i] = std::sin(angle);
            cosCache3a[i] = std::cos(angle);
        }
        //        for (j = 0; j <= stacks; j++)
        //        {
        //            angle = Pi * (j - 0.5) / stacks;
        //            sinCache3b[j] = std::sin(angle);
        //            cosCache3b[j] = std::cos(angle);
        //        }
    }

    sinCache1a[slices] = sinCache1a[0];
    cosCache1a[slices] = cosCache1a[0];
    if (needCache2)
    {
        sinCache2a[slices] = sinCache2a[0];
        cosCache2a[slices] = cosCache2a[0];
    }
    if (needCache3)
    {
        sinCache3a[slices] = sinCache3a[0];
        cosCache3a[slices] = cosCache3a[0];
    }

    /* Do ends of sphere as TRIANGLE_FAN's (if not texturing)
    ** We don't do it when texturing because we need to respecify the
    ** texture coordinates of the apex for every adjacent vertex (because
    ** it isn't a constant for that point)
    */

    /* Low end first (j == 0 iteration) */

    // calc. coord.
    //add by shimomura
    int n_coords = 2*(slices+2) +(slices+1)* (stacks);
    int  n_cells  = 2* slices + 2*(slices+1)*(stacks-1);
    int  n_connection = 3 * n_cells; //tri cell type
    kvs::ValueArray<kvs::Real32> local_coords;
    kvs::ValueArray<kvs::UInt32> local_connection;
    kvs::ValueArray<kvs::Real32> local_normal;
    local_coords.allocate(m_glyph_num*3*n_coords);
    local_normal.allocate(m_glyph_num*3*n_coords);
    local_connection.allocate(m_glyph_num*n_connection);

    int coord_id = 0;
    int connection_id = 0;
    for (int gly_id = 0; gly_id<m_glyph_num; gly_id++ )
    {
        start = 1;
        finish = stacks - 1;
        sintemp2 = sinCache1b[1];
        zHigh = cosCache1b[1];
        sintemp3 = sinCache2b[1];
        costemp3 = cosCache2b[1];

        GLfloat t_x = m_position.at(gly_id*3 + 0);
        GLfloat t_y = m_position.at(gly_id*3 + 1);
        GLfloat t_z = m_position.at(gly_id*3 + 2);
        //#if 0
        local_coords.at(coord_id   ) = t_x + 0.0;
        local_coords.at(coord_id +1) = t_y + 0.0;
        local_coords.at(coord_id +2) = t_z + radius;
        local_normal.at(coord_id   ) = sinCache2a[0] * sinCache2b[0];
        local_normal.at(coord_id +1) = cosCache2a[0] * sinCache2b[0];
        local_normal.at(coord_id +2) = cosCache2b[0];
        coord_id      +=3;

        for (i = slices; i >= 0; i--)
        {
            //add by shimomura
            local_coords.at(coord_id   ) = t_x + sintemp2 * sinCache1a[i];
            local_coords.at(coord_id +1) = t_y + sintemp2 * cosCache1a[i];
            local_coords.at(coord_id +2) = t_z + zHigh;
            local_normal.at(coord_id   ) = sinCache2a[i] * sintemp3;
            local_normal.at(coord_id +1) = cosCache2a[i] * sintemp3;
            local_normal.at(coord_id +2) = costemp3;
            coord_id      +=3;
        }

        for (i = 0; i < slices; i++)
        {
            local_connection.at(connection_id)   = gly_id*n_coords + 0;
            local_connection.at(connection_id+1) = gly_id*n_coords + i+1;
            local_connection.at(connection_id+2) = gly_id*n_coords + i+2;
            connection_id += 3;
        }
        //#endif

        for (j = start; j <= finish+1; j++)
        {
            zLow = cosCache1b[j];
            sintemp1 = sinCache1b[j];
            sintemp4 = sinCache2b[j];
            costemp4 = cosCache2b[j];

            for (i = 0; i <= slices; i++)
            {
                //add by shimomura
                local_coords.at(coord_id)   = t_x + sintemp1 * sinCache1a[i];
                local_coords.at(coord_id+1) = t_y + sintemp1 * cosCache1a[i];
                local_coords.at(coord_id+2) = t_z + zLow;
                local_normal.at(coord_id) = sinCache2a[i] * sintemp4;
                local_normal.at(coord_id+1) = cosCache2a[i] * sintemp4;
                local_normal.at(coord_id+2) = costemp4;
                coord_id      +=3;
            }
        }

        int line_size = slices + 1;
        int vertex_index = 0;
        for (j = start; j <= finish; j++)
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
                local_connection.at(connection_id+1) = gly_id*n_coords +(local_vertex_index[1]);
                local_connection.at(connection_id+2) = gly_id*n_coords +(local_vertex_index[2]);

                local_connection.at(connection_id+3) = gly_id*n_coords +(local_vertex_index[2]);
                local_connection.at(connection_id+4) = gly_id*n_coords +(local_vertex_index[1]);
                local_connection.at(connection_id+5) = gly_id*n_coords +(local_vertex_index[3]);
                connection_id += 6;
                vertex_index++;
            }
        }

        //#if 0
        /* High end next (j == stacks-1 iteration) */
        sintemp2 = sinCache1b[stacks-1];
        zHigh = cosCache1b[stacks-1];
        sintemp3 = sinCache2b[stacks-1];
        costemp3 = cosCache2b[stacks-1];

        for (i = 0; i <= slices; i++)
        {
            //add by shimomura
            local_coords.at(coord_id   ) = t_x + sintemp2 * sinCache1a[i];
            local_coords.at(coord_id +1) = t_y + sintemp2 * cosCache1a[i];
            local_coords.at(coord_id +2) = t_z + zHigh;
            local_normal.at(coord_id   ) = sinCache2a[i] * sintemp3;
            local_normal.at(coord_id +1) = cosCache2a[i] * sintemp3;
            local_normal.at(coord_id +2) = costemp3;
            coord_id      +=3;
        }

        //add by shimomura
        local_coords.at(coord_id   ) = t_x + 0.0;
        local_coords.at(coord_id +1) = t_y + 0.0;
        local_coords.at(coord_id +2) = t_z + (-radius);
        local_normal.at(coord_id   ) = sinCache2a[stacks] * sinCache2b[stacks];
        local_normal.at(coord_id +1) = cosCache2a[stacks] * sinCache2b[stacks];
        local_normal.at(coord_id +2) = cosCache2b[stacks];
        coord_id      +=3;
        // size =  1 + slice + 1

        for (i = 0; i < slices; i++)
        {
            local_connection.at(connection_id)   = gly_id*n_coords +n_coords - 1;
            local_connection.at(connection_id+1) = gly_id*n_coords +n_coords - (slices + 2) + i;
            local_connection.at(connection_id+2) = gly_id*n_coords +n_coords - (slices + 2) + i + 1;
            connection_id += 3;
        }
    }

    SuperClass::setCoords( local_coords  );
    SuperClass::setConnections( local_connection );
    SuperClass::setColor( m_color );
    SuperClass::setNormals( local_normal );
    SuperClass::setOpacity( int(m_opacity) );
    SuperClass::setPolygonType( kvs::PolygonObject::Triangle );
    SuperClass::setColorType( kvs::PolygonObject::PolygonColor );

    //     std::cout  <<  __PRETTY_FUNCTION__ <<" : "<<__LINE__ << std::endl;


}

//#endif
/*===========================================================================*/
/**
 *  @brief  Draw the glyph element.
 *  @param  color [in] color value
 *  @param  opacity [in] opacity value
 */
/*===========================================================================*/
void SphereGlyph::draw_element( const kvs::RGBColor& color, const kvs::UInt8 opacity )
{
    const GLdouble radius = 0.5;
    //const GLint slices = static_cast<GLint>( m_nslices );
    //const GLint stacks = static_cast<GLint>( m_nstacks );
    GLint slices = static_cast<GLint>( m_nslices );
    GLint stacks = static_cast<GLint>( m_nstacks );

    //    kvs::OpenGL::Color( color.r(), color.g(), color.b(), opacity );
    //    kvs::OpenGL::DrawSphere( radius, slices, stacks );
    //
    //void DrawSphere( GLdouble radius, GLint slices, GLint stacks )
    //{

    const int CacheSize = 240;
    const float Pi = 3.14159265358979323846;

    GLint i,j;
    GLfloat sinCache1a[CacheSize];
    GLfloat cosCache1a[CacheSize];
    GLfloat sinCache2a[CacheSize];
    GLfloat cosCache2a[CacheSize];
    GLfloat sinCache3a[CacheSize];
    GLfloat cosCache3a[CacheSize];
    GLfloat sinCache1b[CacheSize];
    GLfloat cosCache1b[CacheSize];
    GLfloat sinCache2b[CacheSize];
    GLfloat cosCache2b[CacheSize];
    //    GLfloat sinCache3b[CacheSize];
    //    GLfloat cosCache3b[CacheSize];
    GLfloat angle;
    GLfloat zLow, zHigh;
    GLfloat sintemp1 = 0.0, sintemp2 = 0.0, sintemp3 = 0.0, sintemp4 = 0.0;
    GLfloat costemp3 = 0.0, costemp4 = 0.0;
    GLboolean needCache2, needCache3;
    GLint start, finish;

    if (slices >= CacheSize) slices = CacheSize-1;
    if (stacks >= CacheSize) stacks = CacheSize-1;
    if (slices < 2 || stacks < 1 || radius < 0.0)
    {
        kvsMessageError("Invalid value.");
        return;
    }

    /* Cache is the vertex locations cache */
    /* Cache2 is the various normals at the vertices themselves */
    /* Cache3 is the various normals for the faces */
    needCache2 = GL_TRUE;
    needCache3 = GL_FALSE;

    for (i = 0; i < slices; i++)
    {
        angle = 2 * Pi * i / slices;
        sinCache1a[i] = std::sin(angle);
        cosCache1a[i] = std::cos(angle);
        if (needCache2)
        {
            sinCache2a[i] = sinCache1a[i];
            cosCache2a[i] = cosCache1a[i];
        }
    }

    for (j = 0; j <= stacks; j++)
    {
        angle = Pi * j / stacks;
        if (needCache2)
        {
            sinCache2b[j] = std::sin(angle);
            cosCache2b[j] = std::cos(angle);
        }
        sinCache1b[j] = radius * std::sin(angle);
        cosCache1b[j] = radius * std::cos(angle);
    }
    /* Make sure it comes to a point */
    sinCache1b[0] = 0;
    sinCache1b[stacks] = 0;

    if (needCache3) {
        for (i = 0; i < slices; i++)
        {
            angle = 2 * Pi * (i-0.5) / slices;
            sinCache3a[i] = std::sin(angle);
            cosCache3a[i] = std::cos(angle);
        }
        //        for (j = 0; j <= stacks; j++)
        //        {
        //            angle = Pi * (j - 0.5) / stacks;
        //            sinCache3b[j] = std::sin(angle);
        //            cosCache3b[j] = std::cos(angle);
        //        }
    }

    sinCache1a[slices] = sinCache1a[0];
    cosCache1a[slices] = cosCache1a[0];
    if (needCache2)
    {
        sinCache2a[slices] = sinCache2a[0];
        cosCache2a[slices] = cosCache2a[0];
    }
    if (needCache3)
    {
        sinCache3a[slices] = sinCache3a[0];
        cosCache3a[slices] = cosCache3a[0];
    }

    /* Do ends of sphere as TRIANGLE_FAN's (if not texturing)
    ** We don't do it when texturing because we need to respecify the
    ** texture coordinates of the apex for every adjacent vertex (because
    ** it isn't a constant for that point)
    */
    start = 1;
    finish = stacks - 1;

    /* Low end first (j == 0 iteration) */
    sintemp2 = sinCache1b[1];
    zHigh = cosCache1b[1];
    sintemp3 = sinCache2b[1];
    costemp3 = cosCache2b[1];

    //add by shimomura
    int n_coords = (2*(slices+2) + (slices+1)*(stacks-1));
    //int n_coords = 2 + (slices * stacks);
    int  n_connection = 3*2*slices*stacks; // tet cell type
    int  n_cells =2 *slices*stacks ;
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::UInt32> connection;
    kvs::ValueArray<kvs::Real32> normal;
    coords.allocate(3*n_coords);
    normal.allocate(3*n_coords);
    //セル数 = 2*sl*st
    connection.allocate(n_connection);

    int coord_id = 0;
    int connection_id = 0;
    coords.at(coord_id   ) = 0.0;
    coords.at(coord_id +1) = 0.0;
    coords.at(coord_id +2) = radius;
    normal.at(coord_id   ) = sinCache2a[0] * sinCache2b[0];
    normal.at(coord_id +1) = cosCache2a[0] * sinCache2b[0];
    normal.at(coord_id +2) = cosCache2b[0];
    coord_id      +=3;

    kvs::OpenGL::Begin(GL_TRIANGLE_FAN);
    kvs::OpenGL::Normal(sinCache2a[0] * sinCache2b[0], cosCache2a[0] * sinCache2b[0], cosCache2b[0]);
    kvs::OpenGL::Vertex(0.0, 0.0, radius);

    for (i = slices; i >= 0; i--)
    {
        kvs::OpenGL::Normal(sinCache2a[i] * sintemp3, cosCache2a[i] * sintemp3, costemp3);
        kvs::OpenGL::Vertex(sintemp2 * sinCache1a[i], sintemp2 * cosCache1a[i], zHigh);
        //add by shimomura
        coords.at(coord_id   ) = sintemp2 * sinCache1a[i];
        coords.at(coord_id +1) = sintemp2 * cosCache1a[i];
        coords.at(coord_id +2) = zHigh;
        normal.at(coord_id   ) = sinCache2a[i] * sintemp3;
        normal.at(coord_id +1) = cosCache2a[i] * sintemp3;
        normal.at(coord_id +2) = costemp3;
        coord_id      +=3;
        //        connection_id +=3;
    }
    // size =  1 + slice + 1
    kvs::OpenGL::End();

    for (i = 0; i < slices; i++)
    {
        connection.at(connection_id)   = 0;
        connection.at(connection_id+1) = i+1;
        connection.at(connection_id+2) = i+2;
        connection_id += 3;
    }


    for (j = start; j < finish; j++)
    {
        zLow = cosCache1b[j];
        zHigh = cosCache1b[j+1];
        sintemp1 = sinCache1b[j];
        sintemp2 = sinCache1b[j+1];
        sintemp3 = sinCache2b[j+1];
        costemp3 = cosCache2b[j+1];
        sintemp4 = sinCache2b[j];
        costemp4 = cosCache2b[j];

        kvs::OpenGL::Begin(GL_QUAD_STRIP);
        for (i = 0; i <= slices; i++)
        {
            kvs::OpenGL::Normal(sinCache2a[i] * sintemp3, cosCache2a[i] * sintemp3, costemp3);
            kvs::OpenGL::Vertex(sintemp2 * sinCache1a[i], sintemp2 * cosCache1a[i], zHigh);

            kvs::OpenGL::Normal(sinCache2a[i] * sintemp4, cosCache2a[i] * sintemp4, costemp4);
            kvs::OpenGL::Vertex(sintemp1 * sinCache1a[i], sintemp1 * cosCache1a[i], zLow);

            //add by shimomura
            coords.at(coord_id   ) = sintemp2 * sinCache1a[i];
            coords.at(coord_id +1) = sintemp2 * cosCache1a[i];
            coords.at(coord_id +2) = zHigh;
            normal.at(coord_id   ) = sinCache2a[i] * sintemp3;
            normal.at(coord_id +1) = cosCache2a[i] * sintemp3;
            normal.at(coord_id +2) = costemp3;

            coords.at(coord_id   ) = sintemp1 * sinCache1a[i];
            coords.at(coord_id +1) = sintemp1 * cosCache1a[i];
            coords.at(coord_id +2) = zLow;
            normal.at(coord_id   ) = sinCache2a[i] * sintemp4;
            normal.at(coord_id +1) = cosCache2a[i] * sintemp4;
            normal.at(coord_id +2) = costemp4;
            coord_id      +=6;
            //connection_id +=6;
        }
        // size =  (slice+1)* (stacks-1)
        kvs::OpenGL::End();
    }

    int line_size = slices;
    int vertex_index = slices+2;
    for (j = start; j < finish; j++)
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
            connection.at(connection_id) = (local_vertex_index[0]);
            connection.at(connection_id) = (local_vertex_index[1]);
            connection.at(connection_id) = (local_vertex_index[2]);

            connection.at(connection_id) = (local_vertex_index[2]);
            connection.at(connection_id) = (local_vertex_index[1]);
            connection.at(connection_id) = (local_vertex_index[3]);

            connection_id +=6;
        }
        // size =  (slice+1)* (stacks-1)
    }

    /* High end next (j == stacks-1 iteration) */
    sintemp2 = sinCache1b[stacks-1];
    zHigh = cosCache1b[stacks-1];
    sintemp3 = sinCache2b[stacks-1];
    costemp3 = cosCache2b[stacks-1];

    kvs::OpenGL::Begin(GL_TRIANGLE_FAN);
    kvs::OpenGL::Normal(sinCache2a[stacks] * sinCache2b[stacks], cosCache2a[stacks] * sinCache2b[stacks], cosCache2b[stacks]);
    kvs::OpenGL::Vertex(0.0, 0.0, -radius);
    for (i = 0; i <= slices; i++)
    {
        kvs::OpenGL::Normal(sinCache2a[i] * sintemp3, cosCache2a[i] * sintemp3, costemp3);
        kvs::OpenGL::Vertex(sintemp2 * sinCache1a[i], sintemp2 * cosCache1a[i], zHigh);

        //add by shimomura
        coords.at(coord_id   ) = sintemp2 * sinCache1a[i];
        coords.at(coord_id +1) = sintemp2 * cosCache1a[i];
        coords.at(coord_id +2) = zHigh;
        normal.at(coord_id   ) = sinCache2a[i] * sintemp3;
        normal.at(coord_id +1) = cosCache2a[i] * sintemp3;
        normal.at(coord_id +2) = costemp3;
        coord_id      +=3;
        //connection_id +=3;
    }


    //add by shimomura
    coords.at(coord_id   ) = 0.0;
    coords.at(coord_id +1) = 0.0;
    coords.at(coord_id +2) = -radius;
    normal.at(coord_id   ) = sinCache2a[stacks] * sinCache2b[stacks];
    normal.at(coord_id +1) = cosCache2a[stacks] * sinCache2b[stacks];
    normal.at(coord_id +2) = cosCache2b[stacks];
    coord_id      +=3;
    // size =  1 + slice + 1
    kvs::OpenGL::End();

    for (i = 0; i < slices; i++)
    {
        connection.at(connection_id)   = n_coords - 1;
        connection.at(connection_id+1) = n_coords - (slices + 1) + i;
        connection.at(connection_id+2) = n_coords - (slices + 1) + i + 1;
        connection_id += 3;
    }

    std::cout  << "debug 1 "<< std::endl;

    SuperClass::setCoords( kvs::ValueArray<kvs::Real32>( coords ) );
    SuperClass::setConnections( kvs::ValueArray<kvs::UInt32>( connection ) );
    SuperClass::setColor( color );
    SuperClass::setNormals( kvs::ValueArray<kvs::Real32>( normal ) );
    SuperClass::setOpacity( 255 );
    SuperClass::setPolygonType( kvs::PolygonObject::Triangle );
    SuperClass::setColorType( kvs::PolygonObject::PolygonColor );
    std::cout  << "debug 2 "<< std::endl;



    //}

}
////////////
kvs::UInt8 SphereGlyph::opacity()
{
    kvs::UInt8 opacity =  SuperClass::opacity();
    return opacity;
}


/*===========================================================================*/
/**
 *  @brief  Initialize the modelview matrix.
 */
/*===========================================================================*/
void SphereGlyph::initialize()
{
    kvs::OpenGL::SetBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    kvs::OpenGL::SetPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    kvs::OpenGL::SetShadeModel( GL_SMOOTH );
    kvs::OpenGL::SetColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

    kvs::OpenGL::Disable( GL_LINE_SMOOTH );
    kvs::OpenGL::Enable( GL_BLEND );
    kvs::OpenGL::Enable( GL_COLOR_MATERIAL );
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

} // end of namespace kvs
