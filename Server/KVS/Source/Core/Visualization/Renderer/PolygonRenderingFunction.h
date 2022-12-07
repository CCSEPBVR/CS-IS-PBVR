/****************************************************************************/
/**
 *  @file PolygonRenderingFunction.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonRenderingFunction.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS_CORE_POLYGON_RENDERING_FUNCTION_H_INCLUDE
#define KVS_CORE_POLYGON_RENDERING_FUNCTION_H_INCLUDE

#include <kvs/PolygonObject>
#include <kvs/RGBColor>
#include <kvs/Type>

/* Function name: Rendering_xxx
 *
 * polygon type
 *   Tri: triangle
 *   Quad: quadrangle
 *
 * color type
 *   VCs: vertex color array
 *   PCs: polygon color array
 *   SC: vertex or polygon color (single color)
 *
 * opacity
 *   Os: opacity array
 *   O: opacity (single value)
 *
 * connection
 *   Cs: connection array
 *
 * normal type
 *   VN: vertex normal
 *   PN: polygon normal
 */

namespace
{

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with VCs and O.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_VCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt8*  colors  = polygon->colors().pointer();
        const kvs::Real32* coords  = polygon->coords().pointer();
        const kvs::UInt8   opacity = polygon->opacity();

        const size_t nvertices = polygon->nvertices();
        for ( size_t i = 0, index = 0; i < nvertices; i++, index += 3 )
        {
            glColor4ub( *( colors + index ), *( colors + index + 1 ), *( colors + index + 2 ), opacity );
            glVertex3fv( coords + index );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with VCs, O and Cs.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_VCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt8*  colors      = polygon->colors().pointer();
        const kvs::Real32* coords      = polygon->coords().pointer();
        const kvs::UInt32* connections = polygon->connections().pointer();
        const kvs::UInt8   opacity     = polygon->opacity();

        const size_t nconnections = polygon->nconnections();
        for ( size_t i = 0, index = 0; i < nconnections; i++, index += 3 )
        {
            const size_t id0 = *( connections + index ) * 3;
            const size_t id1 = *( connections + index + 1 ) * 3;
            const size_t id2 = *( connections + index + 2 ) * 3;

            glColor4ub( *( colors + id0 ), *( colors + id0 + 1 ), *( colors + id0 + 2 ), opacity );
            glVertex3f( *( coords + id0 ), *( coords + id0 + 1 ), *( coords + id0 + 2 ) );

            glColor4ub( *( colors + id1 ), *( colors + id1 + 1 ), *( colors + id1 + 2 ), opacity );
            glVertex3f( *( coords + id1 ), *( coords + id1 + 1 ), *( coords + id1 + 2 ) );

            glColor4ub( *( colors + id2 ), *( colors + id2 + 1 ), *( colors + id2 + 2 ), opacity );
            glVertex3f( *( coords + id2 ), *( coords + id2 + 1 ), *( coords + id2 + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with VCs and Os.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_VCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt8*  colors    = polygon->colors().pointer();
        const kvs::Real32* coords    = polygon->coords().pointer();
        const kvs::UInt8*  opacities = polygon->opacities().pointer();

        const size_t nvertices = polygon->nvertices();
        for ( size_t i = 0, index = 0; i < nvertices; i++, index += 3 )
        {
            glColor4ub( *( colors + index ), *( colors + index + 1 ), *( colors + index + 2 ), opacities[i] );
            glVertex3f( *( coords + index ), *( coords + index + 1 ), *( coords + index + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with VCs, Os and Cs.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_VCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt8*  colors      = polygon->colors().pointer();
        const kvs::Real32* coords      = polygon->coords().pointer();
        const kvs::UInt32* connections = polygon->connections().pointer();
        const kvs::UInt8*  opacities   = polygon->opacities().pointer();

        const size_t nconnections = polygon->nconnections();
        for ( size_t i = 0, index = 0; i < nconnections; i++, index += 3 )
        {
            const size_t id0 = *( connections + index ) * 3;
            const size_t id1 = *( connections + index + 1 ) * 3;
            const size_t id2 = *( connections + index + 2 ) * 3;

            glColor4ub( *( colors + id0 ), *( colors + id0 + 1 ), *( colors + id0 + 2 ), opacities[id0/3] );
            glVertex3f( *( coords + id0 ), *( coords + id0 + 1 ), *( coords + id0 + 2 ) );

            glColor4ub( *( colors + id1 ), *( colors + id1 + 1 ), *( colors + id1 + 2 ), opacities[id1/3] );
            glVertex3f( *( coords + id1 ), *( coords + id1 + 1 ), *( coords + id1 + 2 ) );

            glColor4ub( *( colors + id2 ), *( colors + id2 + 1 ), *( colors + id2 + 2 ), opacities[id2/3] );
            glVertex3f( *( coords + id2 ), *( coords + id2 + 1 ), *( coords + id2 + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with SC and O.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_SC_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::Real32*  coords  = polygon->coords().pointer();
        const kvs::RGBColor color   = polygon->color(0);
        const kvs::UInt8    opacity = polygon->opacity(0);

        glColor4ub( color.r(), color.g(), color.b(), opacity );

        const size_t nvertices = polygon->nvertices();
        for ( size_t i = 0, index = 0; i < nvertices; i++, index += 3 )
        {
            glVertex3f( *( coords + index ), *( coords + index + 1 ), *( coords + index + 2 ) );
        }

    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with SC, O and Cs.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_SC_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::Real32*  coords      = polygon->coords().pointer();
        const kvs::UInt32*  connections = polygon->connections().pointer();
        const kvs::RGBColor color       = polygon->color(0);
        const kvs::UInt8    opacity     = polygon->opacity(0);

        glColor4ub( color.r(), color.g(), color.b(), opacity );

        const size_t nconnections = polygon->nconnections();
        for ( size_t i = 0, index = 0; i < nconnections; i++, index += 3 )
        {
            const size_t id0 = *( connections + index ) * 3;
            const size_t id1 = *( connections + index + 1 ) * 3;
            const size_t id2 = *( connections + index + 2 ) * 3;

            glVertex3f( *( coords + id0 ), *( coords + id0 + 1 ), *( coords + id0 + 2 ) );
            glVertex3f( *( coords + id1 ), *( coords + id1 + 1 ), *( coords + id1 + 2 ) );
            glVertex3f( *( coords + id2 ), *( coords + id2 + 1 ), *( coords + id2 + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with SC and Os.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_SC_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor color     = polygon->color(0);
        const kvs::UInt8*   opacities = polygon->opacities().pointer();
        const kvs::Real32*  coords    = polygon->coords().pointer();

        const size_t nopacities = polygon->nopacities();
        for ( size_t i = 0, index = 0; i < nopacities; i++, index += 9 )
        {
            glColor4ub( color.r(), color.g(), color.b(), opacities[i] );

            glVertex3f( *( coords + index ),     *( coords + index + 1 ), *( coords + index + 2 ) );
            glVertex3f( *( coords + index + 3 ), *( coords + index + 4 ), *( coords + index + 5 ) );
            glVertex3f( *( coords + index + 6 ), *( coords + index + 7 ), *( coords + index + 8 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with SC, Os and Cs.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_SC_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor color       = polygon->color(0);
        const kvs::UInt32*  connections = polygon->connections().pointer();
        const kvs::Real32*  coords      = polygon->coords().pointer();
        const kvs::UInt8*   opacities   = polygon->opacities().pointer();

        const size_t nconnections = polygon->nconnections();
        for ( size_t i = 0, index = 0; i < nconnections; i++, index += 3 )
        {
            const size_t id0 = *( connections + index ) * 3;
            const size_t id1 = *( connections + index + 1 ) * 3;
            const size_t id2 = *( connections + index + 2 ) * 3;

            glColor4ub( color.r(), color.g(), color.b(), opacities[i] );
            glVertex3f( *( coords + id0 ), *( coords + id0 + 1 ), *( coords + id0 + 2 ) );
            glVertex3f( *( coords + id1 ), *( coords + id1 + 1 ), *( coords + id1 + 2 ) );
            glVertex3f( *( coords + id2 ), *( coords + id2 + 1 ), *( coords + id2 + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with PCs and O.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_PCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt8   opacity = polygon->opacity(0);
        const kvs::UInt8*  colors  = polygon->colors().pointer();
        const kvs::Real32* coords  = polygon->coords().pointer();

        const size_t ncolors = polygon->ncolors();
        for ( size_t i = 0, index3 = 0, index9 = 0; i < ncolors; i++, index3 += 3, index9 += 9 )
        {
            glColor4ub( *( colors + index3 ), *( colors + index3 + 1 ), *( colors + index3 + 2 ), opacity );

            glVertex3f( *( coords + index9 ),     *( coords + index9 + 1 ), *( coords + index9 + 2 ) );
            glVertex3f( *( coords + index9 + 3 ), *( coords + index9 + 4 ), *( coords + index9 + 5 ) );
            glVertex3f( *( coords + index9 + 6 ), *( coords + index9 + 7 ), *( coords + index9 + 8 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with PCs, O and Cs.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_PCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt32* connections = polygon->connections().pointer();
        const kvs::UInt8*  colors      = polygon->colors().pointer();
        const kvs::UInt8   opacity     = polygon->opacity(0);
        const kvs::Real32* coords      = polygon->coords().pointer();

        const size_t nconnections = polygon->nconnections();
        for ( size_t i = 0, index = 0; i < nconnections; i++, index += 3 )
        {
            const size_t id0 = *( connections + index ) * 3;
            const size_t id1 = *( connections + index + 1 ) * 3;
            const size_t id2 = *( connections + index + 2 ) * 3;

            glColor4ub( *( colors + index ), *( colors + index + 1 ), *( colors + index + 2 ), opacity );
            glVertex3f( *( coords + id0 ), *( coords + id0 + 1 ), *( coords + id0 + 2 ) );
            glVertex3f( *( coords + id1 ), *( coords + id1 + 1 ), *( coords + id1 + 2 ) );
            glVertex3f( *( coords + id2 ), *( coords + id2 + 1 ), *( coords + id2 + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with PCs and Os.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_PCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt8*  colors    = polygon->colors().pointer();
        const kvs::UInt8*  opacities = polygon->opacities().pointer();
        const kvs::Real32* coords    = polygon->coords().pointer();

        const size_t ncolors = polygon->ncolors();
        for ( size_t i = 0, index3 = 0, index9 = 0; i < ncolors; i++, index3 += 3, index9 += 9 )
        {
            glColor4ub( *( colors + index3 ), *( colors + index3 + 1 ), *( colors + index3 + 2 ), opacities[i] );

            glVertex3f( *( coords + index9 ),     *( coords + index9 + 1 ), *( coords + index9 + 2 ) );
            glVertex3f( *( coords + index9 + 3 ), *( coords + index9 + 4 ), *( coords + index9 + 5 ) );
            glVertex3f( *( coords + index9 + 6 ), *( coords + index9 + 7 ), *( coords + index9 + 8 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for triangles with PCs, Os and Cs.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Tri_PCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::UInt32* connections = polygon->connections().pointer();
        const kvs::UInt8*  colors      = polygon->colors().pointer();
        const kvs::UInt8*  opacities   = polygon->opacities().pointer();
        const kvs::Real32* coords      = polygon->coords().pointer();

        const size_t nconnections = polygon->nconnections();
        for ( size_t i = 0, index = 0; i < nconnections; i++, index += 3 )
        {
            const size_t id0 = *( connections + index ) * 3;
            const size_t id1 = *( connections + index + 1 ) * 3;
            const size_t id2 = *( connections + index + 2 ) * 3;

            glColor4ub( *( colors + index ), *( colors + index + 1 ), *( colors + index + 2 ), opacities[i] );
            glVertex3f( *( coords + id0 ), *( coords + id0 + 1 ), *( coords + id0 + 2 ) );
            glVertex3f( *( coords + id1 ), *( coords + id1 + 1 ), *( coords + id1 + 2 ) );
            glVertex3f( *( coords + id2 ), *( coords + id2 + 1 ), *( coords + id2 + 2 ) );
        }
    }
    glEnd();
}

/*===========================================================================*/
/**
 *  @brief  Rendering function for quadragles with VCs and O.
 *  @param  polygon [in] pointer to the polygon object
 */
/*===========================================================================*/
void Rendering_Quad_VCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::UInt8*  colors  = polygon->colors().pointer();
        const kvs::UInt8   opacity = polygon->opacity(0);
        const kvs::Real32* coords  = polygon->coords().pointer();

        const size_t nvertices = polygon->nvertices();
        for ( size_t i = 0, index = 0; i < nvertices; i++, index += 3 )
        {
            glColor4ub( *( colors + index ), *( colors + index + 1 ), *( colors + index + 2 ), opacity );
            glVertex3f( *( coords + index ), *( coords + index + 1 ), *( coords + index + 2 ) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glColor4ub( *(polygon->colors().pointer()   + con3),
                        *(polygon->colors().pointer()   + con3+1),
                        *(polygon->colors().pointer()   + con3+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer()   + index),
                        *(polygon->colors().pointer()   + index+1),
                        *(polygon->colors().pointer()   + index+2),
                        polygon->opacity( i ) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity(con0/3) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity(con1/3) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity(con2/3) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glColor4ub( *(polygon->colors().pointer()   + con3),
                        *(polygon->colors().pointer()   + con3+1),
                        *(polygon->colors().pointer()   + con3+2),
                        polygon->opacity(con3/3) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_SC_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_SC_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_SC_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nopacities(); i++ )
        {
            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( i ) );
            size_t index = 12 * i;

            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_SC_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity(i) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );

            index = 12 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );

            index = 12 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_VCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer() + index),
                        *(polygon->normals().pointer() + index+1),
                        *(polygon->normals().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_VCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( *(polygon->colors().pointer() + con0),
                        *(polygon->colors().pointer() + con0+1),
                        *(polygon->colors().pointer() + con0+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_VCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer()   + index),
                        *(polygon->colors().pointer()   + index+1),
                        *(polygon->colors().pointer()   + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_VCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity(con0/3) );
            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity(con1/3) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity(con2/3) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_SC_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_SC_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_SC_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nopacities(); i++ )
        {
            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( i ) );
            size_t index = 9 * i;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glNormal3f( *(polygon->normals().pointer()    + index+3),
                        *(polygon->normals().pointer()    + index+4),
                        *(polygon->normals().pointer()    + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glNormal3f( *(polygon->normals().pointer()    + index+6),
                        *(polygon->normals().pointer()    + index+7),
                        *(polygon->normals().pointer()    + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_SC_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity(i) );
            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_PCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );

            index = 9 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glNormal3f( *(polygon->normals().pointer()    + index+3),
                        *(polygon->normals().pointer()    + index+4),
                        *(polygon->normals().pointer()    + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glNormal3f( *(polygon->normals().pointer()    + index+6),
                        *(polygon->normals().pointer()    + index+7),
                        *(polygon->normals().pointer()    + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_PCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_PCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );

            index = 9 * i;
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glNormal3f( *(polygon->normals().pointer()  + index+3),
                        *(polygon->normals().pointer()  + index+4),
                        *(polygon->normals().pointer()  + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glNormal3f( *(polygon->normals().pointer()  + index+6),
                        *(polygon->normals().pointer()  + index+7),
                        *(polygon->normals().pointer()  + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_VN_PCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer()  + con0),
                        *(polygon->normals().pointer()  + con0+1),
                        *(polygon->normals().pointer()  + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()  + con1),
                        *(polygon->normals().pointer()  + con1+1),
                        *(polygon->normals().pointer()  + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()  + con2),
                        *(polygon->normals().pointer()  + con2+1),
                        *(polygon->normals().pointer()  + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_VCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_VCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( *(polygon->colors().pointer() + con0),
                        *(polygon->colors().pointer() + con0+1),
                        *(polygon->colors().pointer() + con0+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + con0),
                        *(polygon->normals().pointer()  + con0+1),
                        *(polygon->normals().pointer()  + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + con1),
                        *(polygon->normals().pointer()  + con1+1),
                        *(polygon->normals().pointer()  + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + con2),
                        *(polygon->normals().pointer()  + con2+1),
                        *(polygon->normals().pointer()  + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glColor4ub( *(polygon->colors().pointer()   + con3),
                        *(polygon->colors().pointer()   + con3+1),
                        *(polygon->colors().pointer()   + con3+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + con3),
                        *(polygon->normals().pointer()  + con3+1),
                        *(polygon->normals().pointer()  + con3+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_VCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer()   + index),
                        *(polygon->colors().pointer()   + index+1),
                        *(polygon->colors().pointer()   + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_VCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity(con0/3) );
            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity(con1/3) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity(con2/3) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glColor4ub( *(polygon->colors().pointer()   + con3),
                        *(polygon->colors().pointer()   + con3+1),
                        *(polygon->colors().pointer()   + con3+2),
                        polygon->opacity(con3/3) );
            glNormal3f( *(polygon->normals().pointer()    + con3),
                        *(polygon->normals().pointer()    + con3+1),
                        *(polygon->normals().pointer()    + con3+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_SC_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nvertices(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_SC_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glNormal3f( *(polygon->normals().pointer()    + con3),
                        *(polygon->normals().pointer()    + con3+1),
                        *(polygon->normals().pointer()    + con3+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_SC_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nopacities(); i++ )
        {
            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( i ) );
            size_t index = 12 * i;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glNormal3f( *(polygon->normals().pointer()    + index+3),
                        *(polygon->normals().pointer()    + index+4),
                        *(polygon->normals().pointer()    + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glNormal3f( *(polygon->normals().pointer()    + index+6),
                        *(polygon->normals().pointer()    + index+7),
                        *(polygon->normals().pointer()    + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glNormal3f( *(polygon->normals().pointer()    + index+9),
                        *(polygon->normals().pointer()    + index+10),
                        *(polygon->normals().pointer()    + index+11) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_SC_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity(i) );
            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glNormal3f( *(polygon->normals().pointer()    + con3),
                        *(polygon->normals().pointer()    + con3+1),
                        *(polygon->normals().pointer()    + con3+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_PCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );

            index = 12 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glNormal3f( *(polygon->normals().pointer()    + index+3),
                        *(polygon->normals().pointer()    + index+4),
                        *(polygon->normals().pointer()    + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glNormal3f( *(polygon->normals().pointer()    + index+6),
                        *(polygon->normals().pointer()    + index+7),
                        *(polygon->normals().pointer()    + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glNormal3f( *(polygon->normals().pointer()    + index+9),
                        *(polygon->normals().pointer()    + index+10),
                        *(polygon->normals().pointer()    + index+11) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_PCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glNormal3f( *(polygon->normals().pointer()    + con3),
                        *(polygon->normals().pointer()    + con3+1),
                        *(polygon->normals().pointer()    + con3+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_PCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );

            index = 12 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glNormal3f( *(polygon->normals().pointer()    + index+3),
                        *(polygon->normals().pointer()    + index+4),
                        *(polygon->normals().pointer()    + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glNormal3f( *(polygon->normals().pointer()    + index+6),
                        *(polygon->normals().pointer()    + index+7),
                        *(polygon->normals().pointer()    + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glNormal3f( *(polygon->normals().pointer()    + index+9),
                        *(polygon->normals().pointer()    + index+10),
                        *(polygon->normals().pointer()    + index+11) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_VN_PCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glNormal3f( *(polygon->normals().pointer()    + con0),
                        *(polygon->normals().pointer()    + con0+1),
                        *(polygon->normals().pointer()    + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glNormal3f( *(polygon->normals().pointer()    + con1),
                        *(polygon->normals().pointer()    + con1+1),
                        *(polygon->normals().pointer()    + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glNormal3f( *(polygon->normals().pointer()    + con2),
                        *(polygon->normals().pointer()    + con2+1),
                        *(polygon->normals().pointer()    + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glNormal3f( *(polygon->normals().pointer()    + con3),
                        *(polygon->normals().pointer()    + con3+1),
                        *(polygon->normals().pointer()    + con3+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_VCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nnormals(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 9 * i;
            glColor4ub( *(polygon->colors().pointer()   + index),
                        *(polygon->colors().pointer()   + index+1),
                        *(polygon->colors().pointer()   + index+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glColor4ub( *(polygon->colors().pointer()   + index+3),
                        *(polygon->colors().pointer()   + index+4),
                        *(polygon->colors().pointer()   + index+5),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glColor4ub( *(polygon->colors().pointer()   + index+6),
                        *(polygon->colors().pointer()   + index+7),
                        *(polygon->colors().pointer()   + index+8),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_VCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_VCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nnormals(); i++ )
        {
            size_t nindex = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + nindex),
                        *(polygon->normals().pointer()    + nindex+1),
                        *(polygon->normals().pointer()    + nindex+2) );

            size_t vindex = 9 * i;
            glColor4ub( *(polygon->colors().pointer()   + vindex),
                        *(polygon->colors().pointer()   + vindex+1),
                        *(polygon->colors().pointer()   + vindex+2),
                        polygon->opacity( nindex ) );
            glVertex3f( *(polygon->coords().pointer() + vindex),
                        *(polygon->coords().pointer() + vindex+1),
                        *(polygon->coords().pointer() + vindex+2) );
            glColor4ub( *(polygon->colors().pointer()   + vindex+3),
                        *(polygon->colors().pointer()   + vindex+4),
                        *(polygon->colors().pointer()   + vindex+5),
                        polygon->opacity( nindex+1 ) );
            glVertex3f( *(polygon->coords().pointer() + vindex+3),
                        *(polygon->coords().pointer() + vindex+4),
                        *(polygon->coords().pointer() + vindex+5) );
            glColor4ub( *(polygon->colors().pointer()   + vindex+6),
                        *(polygon->colors().pointer()   + vindex+7),
                        *(polygon->colors().pointer()   + vindex+8),
                        polygon->opacity( nindex+2 ) );
            glVertex3f( *(polygon->coords().pointer() + vindex+6),
                        *(polygon->coords().pointer() + vindex+7),
                        *(polygon->coords().pointer() + vindex+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_VCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity(con0/3) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity(con1/3) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity(con2/3) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_SC_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nnormals(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer() + index),
                        *(polygon->normals().pointer() + index+1),
                        *(polygon->normals().pointer() + index+2) );

            index = 9 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_SC_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_SC_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nopacities(); i++ )
        {
            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( i ) );

            size_t index = 3 * i;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 9 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_SC_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity(i) );
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_PCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );

            index = 9 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_PCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_PCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );

            index = 9 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Tri_PN_PCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_TRIANGLES );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;

            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_VCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const size_t nnormals = polygon->nnormals();
        for( size_t i = 0; i < nnormals; i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 12 * i;
            glColor4ub( *(polygon->colors().pointer()   + index),
                        *(polygon->colors().pointer()   + index+1),
                        *(polygon->colors().pointer()   + index+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glColor4ub( *(polygon->colors().pointer()   + index+3),
                        *(polygon->colors().pointer()   + index+4),
                        *(polygon->colors().pointer()   + index+5),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glColor4ub( *(polygon->colors().pointer()   + index+6),
                        *(polygon->colors().pointer()   + index+7),
                        *(polygon->colors().pointer()   + index+8),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glColor4ub( *(polygon->colors().pointer()   + index+9),
                        *(polygon->colors().pointer()   + index+10),
                        *(polygon->colors().pointer()   + index+11),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_VCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glColor4ub( *(polygon->colors().pointer()   + con3),
                        *(polygon->colors().pointer()   + con3+1),
                        *(polygon->colors().pointer()   + con3+2),
                        polygon->opacity( 0 ) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_VCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nnormals(); i++ )
        {
            size_t nindex = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + nindex),
                        *(polygon->normals().pointer()    + nindex+1),
                        *(polygon->normals().pointer()    + nindex+2) );

            size_t vindex = 12 * i;
            glColor4ub( *(polygon->colors().pointer()   + vindex),
                        *(polygon->colors().pointer()   + vindex+1),
                        *(polygon->colors().pointer()   + vindex+2),
                        polygon->opacity( nindex ) );
            glVertex3f( *(polygon->coords().pointer() + vindex),
                        *(polygon->coords().pointer() + vindex+1),
                        *(polygon->coords().pointer() + vindex+2) );
            glColor4ub( *(polygon->colors().pointer()   + vindex+3),
                        *(polygon->colors().pointer()   + vindex+4),
                        *(polygon->colors().pointer()   + vindex+5),
                        polygon->opacity( nindex+1 ) );
            glVertex3f( *(polygon->coords().pointer() + vindex+3),
                        *(polygon->coords().pointer() + vindex+4),
                        *(polygon->coords().pointer() + vindex+5) );
            glColor4ub( *(polygon->colors().pointer()   + vindex+6),
                        *(polygon->colors().pointer()   + vindex+7),
                        *(polygon->colors().pointer()   + vindex+8),
                        polygon->opacity( nindex+2 ) );
            glVertex3f( *(polygon->coords().pointer() + vindex+6),
                        *(polygon->coords().pointer() + vindex+7),
                        *(polygon->coords().pointer() + vindex+8) );
            glColor4ub( *(polygon->colors().pointer()   + vindex+9),
                        *(polygon->colors().pointer()   + vindex+10),
                        *(polygon->colors().pointer()   + vindex+11),
                        polygon->opacity( nindex+3 ) );
            glVertex3f( *(polygon->coords().pointer() + vindex+9),
                        *(polygon->coords().pointer() + vindex+10),
                        *(polygon->coords().pointer() + vindex+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_VCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glColor4ub( *(polygon->colors().pointer()   + con0),
                        *(polygon->colors().pointer()   + con0+1),
                        *(polygon->colors().pointer()   + con0+2),
                        polygon->opacity(con0/3) );
            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glColor4ub( *(polygon->colors().pointer()   + con1),
                        *(polygon->colors().pointer()   + con1+1),
                        *(polygon->colors().pointer()   + con1+2),
                        polygon->opacity(con1/3) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glColor4ub( *(polygon->colors().pointer()   + con2),
                        *(polygon->colors().pointer()   + con2+1),
                        *(polygon->colors().pointer()   + con2+2),
                        polygon->opacity(con2/3) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glColor4ub( *(polygon->colors().pointer()   + con3),
                        *(polygon->colors().pointer()   + con3+1),
                        *(polygon->colors().pointer()   + con3+2),
                        polygon->opacity(con3/3) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_SC_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nnormals(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 12 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_SC_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( 0 ) );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_SC_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nopacities(); i++ )
        {
            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity( i ) );

            size_t index = 3 * i;

            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 12 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_SC_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        const kvs::RGBColor& col = polygon->color( 0 );

        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            glColor4ub( col.r(), col.g(), col.b(), polygon->opacity(i) );

            size_t index = 3 * i;
            glNormal3f( *(polygon->normals().pointer()    + index),
                        *(polygon->normals().pointer()    + index+1),
                        *(polygon->normals().pointer()    + index+2) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_PCs_O( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer()  + index),
                        *(polygon->normals().pointer()  + index+1),
                        *(polygon->normals().pointer()  + index+2) );

            index = 12 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_PCs_O_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( 0 ) );
            glNormal3f( *(polygon->normals().pointer() + index),
                        *(polygon->normals().pointer() + index+1),
                        *(polygon->normals().pointer() + index+2) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_PCs_Os( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->ncolors(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer() + index),
                        *(polygon->normals().pointer() + index+1),
                        *(polygon->normals().pointer() + index+2) );

            index = 12 * i;
            glVertex3f( *(polygon->coords().pointer() + index),
                        *(polygon->coords().pointer() + index+1),
                        *(polygon->coords().pointer() + index+2) );
            glVertex3f( *(polygon->coords().pointer() + index+3),
                        *(polygon->coords().pointer() + index+4),
                        *(polygon->coords().pointer() + index+5) );
            glVertex3f( *(polygon->coords().pointer() + index+6),
                        *(polygon->coords().pointer() + index+7),
                        *(polygon->coords().pointer() + index+8) );
            glVertex3f( *(polygon->coords().pointer() + index+9),
                        *(polygon->coords().pointer() + index+10),
                        *(polygon->coords().pointer() + index+11) );
        }
    }
    glEnd();
}

/*==========================================================================*/
/**
 *  
 *  @param  polygon     
 */
/*==========================================================================*/
void Rendering_Quad_PN_PCs_Os_Cs( const kvs::PolygonObject* polygon )
{
    glBegin( GL_QUADS );
    {
        for( size_t i = 0; i < polygon->nconnections(); i++ )
        {
            size_t index = 3 * i;
            glColor4ub( *(polygon->colors().pointer() + index),
                        *(polygon->colors().pointer() + index+1),
                        *(polygon->colors().pointer() + index+2),
                        polygon->opacity( i ) );
            glNormal3f( *(polygon->normals().pointer() + index),
                        *(polygon->normals().pointer() + index+1),
                        *(polygon->normals().pointer() + index+2) );

            index = 4 * i;
            size_t con0 = *(polygon->connections().pointer() + index)   * 3;
            size_t con1 = *(polygon->connections().pointer() + index+1) * 3;
            size_t con2 = *(polygon->connections().pointer() + index+2) * 3;
            size_t con3 = *(polygon->connections().pointer() + index+3) * 3;

            glVertex3f( *(polygon->coords().pointer() + con0),
                        *(polygon->coords().pointer() + con0+1),
                        *(polygon->coords().pointer() + con0+2) );
            glVertex3f( *(polygon->coords().pointer() + con1),
                        *(polygon->coords().pointer() + con1+1),
                        *(polygon->coords().pointer() + con1+2) );
            glVertex3f( *(polygon->coords().pointer() + con2),
                        *(polygon->coords().pointer() + con2+1),
                        *(polygon->coords().pointer() + con2+2) );
            glVertex3f( *(polygon->coords().pointer() + con3),
                        *(polygon->coords().pointer() + con3+1),
                        *(polygon->coords().pointer() + con3+2) );
        }
    }
    glEnd();
}

typedef void (*PolygonRenderingFunctionType)( const kvs::PolygonObject* polygon );

enum PolygonRenderingType
{
    Type_Tri_VCs_O = 0,
    Type_Tri_VCs_O_Cs,
    Type_Tri_VCs_Os,
    Type_Tri_VCs_Os_Cs,
    Type_Tri_SC_O,
    Type_Tri_SC_O_Cs,
    Type_Tri_SC_Os,
    Type_Tri_SC_Os_Cs,
    Type_Tri_PCs_O,
    Type_Tri_PCs_O_Cs,
    Type_Tri_PCs_Os,
    Type_Tri_PCs_Os_Cs,

    Type_Quad_VCs_O,
    Type_Quad_VCs_O_Cs,
    Type_Quad_VCs_Os,
    Type_Quad_VCs_Os_Cs,
    Type_Quad_SC_O,
    Type_Quad_SC_O_Cs,
    Type_Quad_SC_Os,
    Type_Quad_SC_Os_Cs,
    Type_Quad_PCs_O,
    Type_Quad_PCs_O_Cs,
    Type_Quad_PCs_Os,
    Type_Quad_PCs_Os_Cs,

    Type_Tri_VN_VCs_O,
    Type_Tri_VN_VCs_O_Cs,
    Type_Tri_VN_VCs_Os,
    Type_Tri_VN_VCs_Os_Cs,
    Type_Tri_VN_SC_O,
    Type_Tri_VN_SC_O_Cs,
    Type_Tri_VN_SC_Os,
    Type_Tri_VN_SC_Os_Cs,
    Type_Tri_VN_PCs_O,
    Type_Tri_VN_PCs_O_Cs,
    Type_Tri_VN_PCs_Os,
    Type_Tri_VN_PCs_Os_Cs,

    Type_Quad_VN_VCs_O,
    Type_Quad_VN_VCs_O_Cs,
    Type_Quad_VN_VCs_Os,
    Type_Quad_VN_VCs_Os_Cs,
    Type_Quad_VN_SC_O,
    Type_Quad_VN_SC_O_Cs,
    Type_Quad_VN_SC_Os,
    Type_Quad_VN_SC_Os_Cs,
    Type_Quad_VN_PCs_O,
    Type_Quad_VN_PCs_O_Cs,
    Type_Quad_VN_PCs_Os,
    Type_Quad_VN_PCs_Os_Cs,

    Type_Tri_PN_VCs_O,
    Type_Tri_PN_VCs_O_Cs,
    Type_Tri_PN_VCs_Os,
    Type_Tri_PN_VCs_Os_Cs,
    Type_Tri_PN_SC_O,
    Type_Tri_PN_SC_O_Cs,
    Type_Tri_PN_SC_Os,
    Type_Tri_PN_SC_Os_Cs,
    Type_Tri_PN_PCs_O,
    Type_Tri_PN_PCs_O_Cs,
    Type_Tri_PN_PCs_Os,
    Type_Tri_PN_PCs_Os_Cs,

    Type_Quad_PN_VCs_O,
    Type_Quad_PN_VCs_O_Cs,
    Type_Quad_PN_VCs_Os,
    Type_Quad_PN_VCs_Os_Cs,
    Type_Quad_PN_SC_O,
    Type_Quad_PN_SC_O_Cs,
    Type_Quad_PN_SC_Os,
    Type_Quad_PN_SC_Os_Cs,
    Type_Quad_PN_PCs_O,
    Type_Quad_PN_PCs_O_Cs,
    Type_Quad_PN_PCs_Os,
    Type_Quad_PN_PCs_Os_Cs,

    NumberOfRenderingTypes
};

PolygonRenderingFunctionType Rendering[NumberOfRenderingTypes] =
{
    &Rendering_Tri_VCs_O,
    &Rendering_Tri_VCs_O_Cs,
    &Rendering_Tri_VCs_Os,
    &Rendering_Tri_VCs_Os_Cs,
    &Rendering_Tri_SC_O,
    &Rendering_Tri_SC_O_Cs,
    &Rendering_Tri_SC_Os,
    &Rendering_Tri_SC_Os_Cs,
    &Rendering_Tri_PCs_O,
    &Rendering_Tri_PCs_O_Cs,
    &Rendering_Tri_PCs_Os,
    &Rendering_Tri_PCs_Os_Cs,

    &Rendering_Quad_VCs_O,
    &Rendering_Quad_VCs_O_Cs,
    &Rendering_Quad_VCs_Os,
    &Rendering_Quad_VCs_Os_Cs,
    &Rendering_Quad_SC_O,
    &Rendering_Quad_SC_O_Cs,
    &Rendering_Quad_SC_Os,
    &Rendering_Quad_SC_Os_Cs,
    &Rendering_Quad_PCs_O,
    &Rendering_Quad_PCs_O_Cs,
    &Rendering_Quad_PCs_Os,
    &Rendering_Quad_PCs_Os_Cs,

    &Rendering_Tri_VN_VCs_O,
    &Rendering_Tri_VN_VCs_O_Cs,
    &Rendering_Tri_VN_VCs_Os,
    &Rendering_Tri_VN_VCs_Os_Cs,
    &Rendering_Tri_VN_SC_O,
    &Rendering_Tri_VN_SC_O_Cs,
    &Rendering_Tri_VN_SC_Os,
    &Rendering_Tri_VN_SC_Os_Cs,
    &Rendering_Tri_VN_PCs_O,
    &Rendering_Tri_VN_PCs_O_Cs,
    &Rendering_Tri_VN_PCs_Os,
    &Rendering_Tri_VN_PCs_Os_Cs,

    &Rendering_Quad_VN_VCs_O,
    &Rendering_Quad_VN_VCs_O_Cs,
    &Rendering_Quad_VN_VCs_Os,
    &Rendering_Quad_VN_VCs_Os_Cs,
    &Rendering_Quad_VN_SC_O,
    &Rendering_Quad_VN_SC_O_Cs,
    &Rendering_Quad_VN_SC_Os,
    &Rendering_Quad_VN_SC_Os_Cs,
    &Rendering_Quad_VN_PCs_O,
    &Rendering_Quad_VN_PCs_O_Cs,
    &Rendering_Quad_VN_PCs_Os,
    &Rendering_Quad_VN_PCs_Os_Cs,

    &Rendering_Tri_PN_VCs_O,
    &Rendering_Tri_PN_VCs_O_Cs,
    &Rendering_Tri_PN_VCs_Os,
    &Rendering_Tri_PN_VCs_Os_Cs,
    &Rendering_Tri_PN_SC_O,
    &Rendering_Tri_PN_SC_O_Cs,
    &Rendering_Tri_PN_SC_Os,
    &Rendering_Tri_PN_SC_Os_Cs,
    &Rendering_Tri_PN_PCs_O,
    &Rendering_Tri_PN_PCs_O_Cs,
    &Rendering_Tri_PN_PCs_Os,
    &Rendering_Tri_PN_PCs_Os_Cs,

    &Rendering_Quad_PN_VCs_O,
    &Rendering_Quad_PN_VCs_O_Cs,
    &Rendering_Quad_PN_VCs_Os,
    &Rendering_Quad_PN_VCs_Os_Cs,
    &Rendering_Quad_PN_SC_O,
    &Rendering_Quad_PN_SC_O_Cs,
    &Rendering_Quad_PN_SC_Os,
    &Rendering_Quad_PN_SC_Os_Cs,
    &Rendering_Quad_PN_PCs_O,
    &Rendering_Quad_PN_PCs_O_Cs,
    &Rendering_Quad_PN_PCs_Os,
    &Rendering_Quad_PN_PCs_Os_Cs,
};

PolygonRenderingType GetPolygonRenderingType( const kvs::PolygonObject* polygon )
{
    const size_t nopacities = polygon->nopacities();
    const size_t ncolors    = polygon->ncolors();
    const size_t nconnects  = polygon->nconnections();
    const kvs::PolygonObject::PolygonType polygon_type = polygon->polygonType();
    const kvs::PolygonObject::NormalType  normal_type  = polygon->normalType();
    const kvs::PolygonObject::ColorType   color_type   = polygon->colorType();

    if( polygon->normals().size() == 0 )
    {
        if( polygon_type == kvs::PolygonObject::Triangle )
        {
            if( color_type == kvs::PolygonObject::VertexColor )
            {
                if( ncolors == 1 )
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Tri_SC_O : Type_Tri_SC_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Tri_SC_Os : Type_Tri_SC_Os_Cs );
                }
                else
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Tri_VCs_O : Type_Tri_VCs_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Tri_VCs_Os : Type_Tri_VCs_Os_Cs );
                }
            }
            else if( color_type == kvs::PolygonObject::PolygonColor )
            {
                if( ncolors == 1 )
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Tri_SC_O : Type_Tri_SC_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Tri_SC_Os : Type_Tri_SC_Os_Cs );
                }
                else
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Tri_PCs_O : Type_Tri_PCs_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Tri_PCs_Os : Type_Tri_PCs_Os_Cs );
                }
            }
        }
        else if( polygon_type == kvs::PolygonObject::Quadrangle )
        {
            if( color_type == kvs::PolygonObject::VertexColor )
            {
                if( ncolors == 1 )
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Quad_SC_O : Type_Quad_SC_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Quad_SC_Os : Type_Quad_SC_Os_Cs );
                }
                else
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Quad_VCs_O : Type_Quad_VCs_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Quad_VCs_Os : Type_Quad_VCs_Os_Cs );
                }
            }
            else if( color_type == kvs::PolygonObject::PolygonColor )
            {
                if( ncolors == 1 )
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Quad_SC_O : Type_Quad_SC_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Quad_SC_Os : Type_Quad_SC_Os_Cs );
                }
                else
                {
                    if( nopacities == 1 )
                        return( ( nconnects == 0 ) ? Type_Quad_PCs_O : Type_Quad_PCs_O_Cs );
                    else
                        return( ( nconnects == 0 ) ? Type_Quad_PCs_Os : Type_Quad_PCs_Os_Cs );
                }
            }
        }
    }

    else
    {
        if( normal_type == kvs::PolygonObject::VertexNormal )
        {
            if( polygon_type == kvs::PolygonObject::Triangle )
            {
                if( color_type == kvs::PolygonObject::VertexColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_VN_SC_O : Type_Tri_VN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_VN_SC_Os : Type_Tri_VN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_VN_VCs_O : Type_Tri_VN_VCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_VN_VCs_Os : Type_Tri_VN_VCs_Os_Cs );
                    }
                }
                else if( color_type == kvs::PolygonObject::PolygonColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_VN_SC_O : Type_Tri_VN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_VN_SC_Os : Type_Tri_VN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_VN_PCs_O : Type_Tri_VN_PCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_VN_PCs_Os : Type_Tri_VN_PCs_Os_Cs );
                    }
                }
            }
            else if( polygon_type == kvs::PolygonObject::Quadrangle )
            {
                if( color_type == kvs::PolygonObject::VertexColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_VN_SC_O : Type_Quad_VN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_VN_SC_Os : Type_Quad_VN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_VN_VCs_O : Type_Quad_VN_VCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_VN_VCs_Os : Type_Quad_VN_VCs_Os_Cs );
                    }
                }
                else if( color_type == kvs::PolygonObject::PolygonColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_VN_SC_O : Type_Quad_VN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_VN_SC_Os : Type_Quad_VN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_VN_PCs_O : Type_Quad_VN_PCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_VN_PCs_Os : Type_Quad_VN_PCs_Os_Cs );
                    }
                }
            }
        }

        else if( normal_type == kvs::PolygonObject::PolygonNormal )
        {
            if( polygon_type == kvs::PolygonObject::Triangle )
            {
                if( color_type == kvs::PolygonObject::VertexColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_PN_SC_O : Type_Tri_PN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_PN_SC_Os : Type_Tri_PN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_PN_VCs_O : Type_Tri_PN_VCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_PN_VCs_Os : Type_Tri_PN_VCs_Os_Cs );
                    }
                }
                else if( color_type == kvs::PolygonObject::PolygonColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_PN_SC_O : Type_Tri_PN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_PN_SC_Os : Type_Tri_PN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Tri_PN_PCs_O : Type_Tri_PN_PCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Tri_PN_PCs_Os : Type_Tri_PN_PCs_Os_Cs );
                    }
                }
            }

            else if( polygon_type == kvs::PolygonObject::Quadrangle )
            {
                if( color_type == kvs::PolygonObject::VertexColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_PN_SC_O : Type_Quad_PN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_PN_SC_Os : Type_Quad_PN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_PN_VCs_O : Type_Quad_PN_VCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_PN_VCs_Os : Type_Quad_PN_VCs_Os_Cs );
                    }
                }
                else if( color_type == kvs::PolygonObject::PolygonColor )
                {
                    if( ncolors == 1 )
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_PN_SC_O : Type_Quad_PN_SC_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_PN_SC_Os : Type_Quad_PN_SC_Os_Cs );
                    }
                    else
                    {
                        if( nopacities == 1 )
                            return( ( nconnects == 0 ) ? Type_Quad_PN_PCs_O : Type_Quad_PN_PCs_O_Cs );
                        else
                            return( ( nconnects == 0 ) ? Type_Quad_PN_PCs_Os : Type_Quad_PN_PCs_Os_Cs );
                    }
                }
            }
        }
    }

    return( Type_Tri_VCs_O );
};

void PolygonRenderingFunction( const kvs::PolygonObject* polygon )
{
    if( polygon->nvertices() > 0 )
    {
        PolygonRenderingType type = GetPolygonRenderingType( polygon );
        Rendering[type]( polygon );
    }
};

} // end of namespace

#endif // KVS_CORE_POLYGON_RENDERING_FUNCTION_H_INCLUDE
