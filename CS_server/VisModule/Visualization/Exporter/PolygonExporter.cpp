/*****************************************************************************/
/**
 *  @file   PolygonExporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonExporter.cpp 657 2011-01-10 09:26:13Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PolygonExporter.h"
#include <vismodule/Message>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new PolygonExporter class for VISMODULE.LObjectPolygon format.
 *  @param  object [in] pointer to the input polygon object
 */
/*===========================================================================*/
PolygonExporter<vismodule::KVSMLObjectPolygon>::PolygonExporter( const vismodule::PolygonObject& object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Executes the export process.
 *  @param  object [in] pointer to the input object
 *  @return pointer to the KVSMLObjectPolygon format
 */
/*===========================================================================*/
vismodule::KVSMLObjectPolygon* PolygonExporter<vismodule::KVSMLObjectPolygon>::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::PolygonObject* polygon = vismodule::PolygonObject::DownCast( object );
    if ( !polygon )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not polygon object.");
        return( NULL );
    }

    switch ( polygon->polygonType() )
    {
    case vismodule::PolygonObject::Triangle: this->setPolygonType( "triangle" ); break;
    case vismodule::PolygonObject::Quadrangle: this->setPolygonType( "quadrangle" ); break;
    default: break;
    }

    switch ( polygon->colorType() )
    {
    case vismodule::PolygonObject::VertexColor: this->setColorType( "vertex" ); break;
    case vismodule::PolygonObject::PolygonColor: this->setColorType( "polygon" ); break;
    default: break;
    }

    switch ( polygon->normalType() )
    {
    case vismodule::PolygonObject::VertexNormal: this->setNormalType( "vertex" ); break;
    case vismodule::PolygonObject::PolygonNormal: this->setNormalType( "polygon" ); break;
    default: break;
    }

    this->setCoords( polygon->coords() );
    this->setColors( polygon->colors() );
    this->setConnections( polygon->connections() );
    this->setNormals( polygon->normals() );
    this->setOpacities( polygon->opacities() );

    return( this );
}

PolygonExporter<vismodule::Stl>::PolygonExporter( const vismodule::PolygonObject& object )
{
    this->exec( object );
}

vismodule::Stl* PolygonExporter<vismodule::Stl>::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::PolygonObject* polygon = vismodule::PolygonObject::DownCast( object );
    if ( !polygon )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not polygon object.");
        return( NULL );
    }

    if ( polygon->polygonType() != vismodule::PolygonObject::Triangle )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not triangle polygon.");
        return( NULL );
    }

    this->setCoords( polygon->coords() );

    if ( polygon->normalType() == vismodule::PolygonObject::VertexNormal )
    {
        // Convert to vismodule::PolygonObject::PolygonNormal type.
        const size_t npolygons = polygon->connections().size() / 3;
        const vismodule::UInt32* pconnections = polygon->connections().pointer();
        const vismodule::Real32* pnormals = polygon->normals().pointer();
        vismodule::ValueArray<vismodule::Real32> normals( npolygons * 3 );
        for ( size_t i = 0; i < npolygons; i++ )
        {
            const vismodule::UInt32 index0 = *(pconnections++);
            const vismodule::UInt32 index1 = *(pconnections++);
            const vismodule::UInt32 index2 = *(pconnections++);

            const vismodule::Vector3f normal0( pnormals + 3 * index0 );
            const vismodule::Vector3f normal1( pnormals + 3 * index1 );
            const vismodule::Vector3f normal2( pnormals + 3 * index2 );
            const vismodule::Vector3f normal = ( normal0 + normal1 + normal2 ) / 3;

            normals[ 3 * i + 0 ] = normal.x();
            normals[ 3 * i + 1 ] = normal.y();
            normals[ 3 * i + 2 ] = normal.z();
        }
        this->setNormals( normals );
    }
    else if ( polygon->normalType() == vismodule::PolygonObject::PolygonNormal )
    {
        this->setNormals( polygon->normals() );
    }

    return( this );
}

PolygonExporter<vismodule::Ply>::PolygonExporter( const vismodule::PolygonObject& object )
{
    this->exec( object );
}

vismodule::Ply* PolygonExporter<vismodule::Ply>::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::PolygonObject* polygon = vismodule::PolygonObject::DownCast( object );
    if ( !polygon )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not polygon object.");
        return( NULL );
    }

    if ( polygon->polygonType() != vismodule::PolygonObject::Triangle )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not triangle polygon.");
        return( NULL );
    }

    this->setCoords( polygon->coords() );

    const size_t nvertices = polygon->coords().size() / 3;

    if ( polygon->colors().size() == 3 )
    {
        vismodule::ValueArray<vismodule::UInt8> colors( nvertices * 3 );

        const vismodule::RGBColor color = polygon->color();
        for ( size_t i = 0; i < nvertices; i++ )
        {
            colors[ 3 * i + 0 ] = color.r();
            colors[ 3 * i + 1 ] = color.g();
            colors[ 3 * i + 2 ] = color.b();
        }
        this->setColors( colors );
    }
    else if ( polygon->colors().size() > 3 )
    {
        if ( polygon->colorType() == vismodule::PolygonObject::PolygonColor )
        {
            vismodule::ValueArray<vismodule::UInt32> temp( nvertices * 3 ); temp.fill( 0x00 );
            vismodule::ValueArray<vismodule::UInt32> counter( nvertices ); counter.fill( 0x00 );
            if ( polygon->nconnections() == 0 )
            {
                const size_t npolygons = polygon->coords().size() / 3;
                for ( size_t i = 0; i < npolygons; i++ )
                {
                    const vismodule::UInt32 index0 = 3 * i + 0;
                    const vismodule::UInt32 index1 = 3 * i + 1;
                    const vismodule::UInt32 index2 = 3 * i + 2;

                    const vismodule::RGBColor color = polygon->color(i);
                    temp[ 3 * index0 + 0 ] += color.r();
                    temp[ 3 * index0 + 1 ] += color.g();
                    temp[ 3 * index0 + 2 ] += color.b();
                    counter[ index0 ] += 1;

                    temp[ 3 * index1 + 0 ] += color.r();
                    temp[ 3 * index1 + 1 ] += color.g();
                    temp[ 3 * index1 + 2 ] += color.b();
                    counter[ index1 ] += 1;

                    temp[ 3 * index2 + 0 ] += color.r();
                    temp[ 3 * index2 + 1 ] += color.g();
                    temp[ 3 * index2 + 2 ] += color.b();
                    counter[ index2 ] += 1;
                }

                vismodule::ValueArray<vismodule::UInt8> colors( nvertices * 3 );
                for ( size_t i = 0; i < nvertices; i++ )
                {
                    if ( counter[i] == 0 ) continue;
                    colors[ 3 * i + 0 ] = vismodule::UInt8( temp[ 3 * i + 0 ] / counter[i] );
                    colors[ 3 * i + 0 ] = vismodule::UInt8( temp[ 3 * i + 0 ] / counter[i] );
                    colors[ 3 * i + 0 ] = vismodule::UInt8( temp[ 3 * i + 0 ] / counter[i] );
                }

                this->setColors( colors );
            }
            else // polygon->nconnections() > 0
            {
                const size_t npolygons = polygon->connections().size() / 3;
                const vismodule::UInt32* pconnections = polygon->connections().pointer();
                for ( size_t i = 0; i < npolygons; i++ )
                {
                    const vismodule::UInt32 index0 = *(pconnections++);
                    const vismodule::UInt32 index1 = *(pconnections++);
                    const vismodule::UInt32 index2 = *(pconnections++);

                    const vismodule::RGBColor color = polygon->color(i);
                    temp[ 3 * index0 + 0 ] += color.r();
                    temp[ 3 * index0 + 1 ] += color.g();
                    temp[ 3 * index0 + 2 ] += color.b();
                    counter[ index0 ] += 1;

                    temp[ 3 * index1 + 0 ] += color.r();
                    temp[ 3 * index1 + 1 ] += color.g();
                    temp[ 3 * index1 + 2 ] += color.b();
                    counter[ index1 ] += 1;

                    temp[ 3 * index2 + 0 ] += color.r();
                    temp[ 3 * index2 + 1 ] += color.g();
                    temp[ 3 * index2 + 2 ] += color.b();
                    counter[ index2 ] += 1;
                }

                vismodule::ValueArray<vismodule::UInt8> colors( nvertices * 3 );
                for ( size_t i = 0; i < nvertices; i++ )
                {
                    if ( counter[i] == 0 ) continue;
                    colors[ 3 * i + 0 ] = vismodule::UInt8( temp[ 3 * i + 0 ] / counter[i] );
                    colors[ 3 * i + 0 ] = vismodule::UInt8( temp[ 3 * i + 0 ] / counter[i] );
                    colors[ 3 * i + 0 ] = vismodule::UInt8( temp[ 3 * i + 0 ] / counter[i] );
                }

                this->setColors( colors );
            }
        }
        else // vismodule::PolygonObject::VertexColor
        {
            this->setColors( polygon->colors() );
        }
    }

    if ( polygon->normals().size() > 0 )
    {
        if ( polygon->normalType() == vismodule::PolygonObject::PolygonNormal )
        {
            vismodule::ValueArray<vismodule::Real32> normals( nvertices * 3 ); normals.fill( 0x00 );
            vismodule::ValueArray<vismodule::UInt32> counter( nvertices ); counter.fill( 0x00 );

            const size_t npolygons = polygon->connections().size() / 3;
            const vismodule::UInt32* pconnections = m_connections.pointer();
            for ( size_t i = 0; i < npolygons; i++ )
            {
                const vismodule::UInt32 index0 = *(pconnections++);
                const vismodule::UInt32 index1 = *(pconnections++);
                const vismodule::UInt32 index2 = *(pconnections++);

                const vismodule::Vector3f normal = polygon->normal(i);
                normals[ 3 * index0 + 0 ] += normal.x();
                normals[ 3 * index0 + 1 ] += normal.y();
                normals[ 3 * index0 + 2 ] += normal.z();
                counter[ index0 ] += 1;

                normals[ 3 * index1 + 0 ] += normal.x();
                normals[ 3 * index1 + 1 ] += normal.y();
                normals[ 3 * index1 + 2 ] += normal.z();
                counter[ index1 ] += 1;

                normals[ 3 * index2 + 0 ] += normal.x();
                normals[ 3 * index2 + 1 ] += normal.y();
                normals[ 3 * index2 + 2 ] += normal.z();
                counter[ index2 ] += 1;
            }

            for ( size_t i = 0; i < nvertices; i++ )
            {
                if ( counter[i] == 0 ) continue;
                normals[ 3 * i + 0 ] /= static_cast<vismodule::Real32>( counter[i] );
                normals[ 3 * i + 1 ] /= static_cast<vismodule::Real32>( counter[i] );
                normals[ 3 * i + 2 ] /= static_cast<vismodule::Real32>( counter[i] );
            }

            this->setNormals( normals );
        }
        else if ( polygon->normalType() == vismodule::PolygonObject::VertexNormal )
        {
            this->setNormals( polygon->normals() );
        }
    }

    if ( polygon->connections().size() > 0 )
    {
        this->setConnections( polygon->connections() );
    }

    return( this );
}

} // end of namespace vismodule
