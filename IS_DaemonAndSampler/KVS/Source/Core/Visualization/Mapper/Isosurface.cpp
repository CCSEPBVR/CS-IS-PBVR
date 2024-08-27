/****************************************************************************/
/**
 *  @file Isosurface.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Isosurface.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Isosurface.h"
#include <kvs/DebugNew>
#include <kvs/MarchingCubes>
#include <kvs/MarchingTetrahedra>
#include <kvs/MarchingHexahedra>
#include <kvs/MarchingPyramid>


namespace kvs
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new Isosurface class.
 */
/*==========================================================================*/
Isosurface::Isosurface( void ):
    kvs::MapperBase(),
    kvs::PolygonObject(),
    m_isolevel( 0 ),
    m_duplication( true )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Isosurface class.
 *  @param  volume [in] pointer to the input volume object
 *  @param  isolevel [in] isolevel
 *  @param  normal_type [in] normal vector type
 */
/*===========================================================================*/
Isosurface::Isosurface(
    const kvs::VolumeObjectBase* volume,
    const double                 isolevel,
    const NormalType             normal_type ):
    kvs::MapperBase(),
    kvs::PolygonObject(),
    m_isolevel( isolevel ),
    m_duplication( true )
{
    SuperClass::m_normal_type = normal_type;

    // In the case of VertexNormal-type, the duplicated vertices are forcibly deleted.
    if ( normal_type == kvs::PolygonObject::VertexNormal )
    {
        m_duplication = false;
    }

    // Extract the surfaces.
    this->exec( volume );
}

/*==========================================================================*/
/**
 *  @brief  Constructs and creates a polygon object.
 *  @param  volume [in] pointer to the volume object
 *  @param  isolevel [in] level of the isosurfaces
 *  @param  normal_type [in] type of the normal vector
 *  @param  duplication [in] duplication flag
 *  @param  transfer_function [in] transfer function
 */
/*==========================================================================*/
Isosurface::Isosurface(
    const kvs::VolumeObjectBase* volume,
    const double                 isolevel,
    const NormalType             normal_type,
    const bool                   duplication,
    const kvs::TransferFunction& transfer_function ):
    kvs::MapperBase( transfer_function ),
    kvs::PolygonObject(),
    m_isolevel( isolevel ),
    m_duplication( duplication )
{
    SuperClass::m_normal_type = normal_type;

    // In the case of VertexNormal-type, the duplicated vertices are forcibly deleted.
    if ( normal_type == kvs::PolygonObject::VertexNormal )
    {
        m_duplication = false;
    }

    // Extract the surfaces.
    this->exec( volume );
}

/*==========================================================================*/
/**
 *  @brief  Destroys the Isosurface class.
 */
/*==========================================================================*/
Isosurface::~Isosurface( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a isolevel.
 *  @param  isolevel [in] isolevel
 */
/*===========================================================================*/
void Isosurface::setIsolevel( const double isolevel )
{
    m_isolevel = isolevel;
}

/*===========================================================================*/
/**
 *  @brief  Executes the mapper process.
 *  @param  object [in] pointer to the input volume object
 *  @return pointer to the polygon object
 */
/*===========================================================================*/
Isosurface::SuperClass* Isosurface::exec( const kvs::ObjectBase* object )
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

    this->mapping( volume );

    return( this );
}

/*==========================================================================*/
/**
 *  @brief  Extracts the surfaces.
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
void Isosurface::mapping( const kvs::VolumeObjectBase* volume )
{
    // Check whether the volume can be processed or not.
    if ( volume->veclen() != 1 )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is not scalar field dat.");
        return;
    }

    if ( volume->volumeType() == kvs::VolumeObjectBase::Structured )
    {
        const kvs::StructuredVolumeObject* structured_volume =
            kvs::StructuredVolumeObject::DownCast( volume );

        kvs::PolygonObject* polygon = new kvs::MarchingCubes(
            structured_volume,
            m_isolevel,
            SuperClass::normalType(),
            m_duplication,
            BaseClass::transferFunction() );
        if ( !polygon )
        {
            BaseClass::m_is_success = false;
            kvsMessageError("Cannot create isosurfaces.");
            return;
        }

        // Shallow copy.
        SuperClass::m_coords       = polygon->coords();
        SuperClass::m_colors       = polygon->colors();
        SuperClass::m_normals      = polygon->normals();
        SuperClass::m_connections  = polygon->connections();
        SuperClass::m_opacities    = polygon->opacities();
        SuperClass::m_polygon_type = polygon->polygonType();
        SuperClass::m_color_type   = polygon->colorType();
        SuperClass::m_normal_type  = polygon->normalType();

        SuperClass::setMinMaxObjectCoords(
            polygon->minObjectCoord(),
            polygon->maxObjectCoord() );
        SuperClass::setMinMaxExternalCoords(
            polygon->minExternalCoord(),
            polygon->maxExternalCoord() );

        delete polygon;
    }
    else // volume->volumeType() == kvs::VolumeObjectBase::Unstructured
    {
        const kvs::UnstructuredVolumeObject* unstructured_volume =
            kvs::UnstructuredVolumeObject::DownCast( volume );

        switch ( unstructured_volume->cellType() )
        {
        case kvs::VolumeObjectBase::Tetrahedra:
        {
            kvs::PolygonObject* polygon = new kvs::MarchingTetrahedra(
                unstructured_volume,
                m_isolevel,
                SuperClass::normalType(),
                m_duplication,
                BaseClass::transferFunction() );
            if ( !polygon )
            {
                BaseClass::m_is_success = false;
                kvsMessageError("Cannot create isosurfaces.");
                return;
            }

            // Shallow copy.
            SuperClass::m_coords       = polygon->coords();
            SuperClass::m_colors       = polygon->colors();
            SuperClass::m_normals      = polygon->normals();
            SuperClass::m_connections  = polygon->connections();
            SuperClass::m_opacities    = polygon->opacities();
            SuperClass::m_polygon_type = polygon->polygonType();
            SuperClass::m_color_type   = polygon->colorType();
            SuperClass::m_normal_type  = polygon->normalType();

            SuperClass::setMinMaxObjectCoords(
                polygon->minObjectCoord(),
                polygon->maxObjectCoord() );
            SuperClass::setMinMaxExternalCoords(
                polygon->minExternalCoord(),
                polygon->maxExternalCoord() );

            delete polygon;
            break;
        }
        case kvs::VolumeObjectBase::Hexahedra:
        {
            kvs::PolygonObject* polygon = new kvs::MarchingHexahedra(
                unstructured_volume,
                m_isolevel,
                SuperClass::normalType(),
                m_duplication,
                BaseClass::transferFunction() );
            if ( !polygon )
            {
                kvsMessageError("Cannot create isosurfaces.");
                return;
            }

            // Shallow copy.
            SuperClass::m_coords       = polygon->coords();
            SuperClass::m_colors       = polygon->colors();
            SuperClass::m_normals      = polygon->normals();
            SuperClass::m_connections  = polygon->connections();
            SuperClass::m_opacities    = polygon->opacities();
            SuperClass::m_polygon_type = polygon->polygonType();
            SuperClass::m_color_type   = polygon->colorType();
            SuperClass::m_normal_type  = polygon->normalType();

            SuperClass::setMinMaxObjectCoords(
                polygon->minObjectCoord(),
                polygon->maxObjectCoord() );
            SuperClass::setMinMaxExternalCoords(
                polygon->minExternalCoord(),
                polygon->maxExternalCoord() );

            delete polygon;

            break;
        }
        case kvs::VolumeObjectBase::Pyramid:
        {
            kvs::PolygonObject* polygon = new kvs::MarchingPyramid(
                unstructured_volume,
                m_isolevel,
                SuperClass::normalType(),
                m_duplication,
                BaseClass::transferFunction() );
            if ( !polygon )
            {
                BaseClass::m_is_success = false;
                kvsMessageError("Cannot create isosurfaces.");
                return;
            }

            // Shallow copy.
            SuperClass::m_coords       = polygon->coords();
            SuperClass::m_colors       = polygon->colors();
            SuperClass::m_normals      = polygon->normals();
            SuperClass::m_connections  = polygon->connections();
            SuperClass::m_opacities    = polygon->opacities();
            SuperClass::m_polygon_type = polygon->polygonType();
            SuperClass::m_color_type   = polygon->colorType();
            SuperClass::m_normal_type  = polygon->normalType();

            SuperClass::setMinMaxObjectCoords(
                polygon->minObjectCoord(),
                polygon->maxObjectCoord() );
            SuperClass::setMinMaxExternalCoords(
                polygon->minExternalCoord(),
                polygon->maxExternalCoord() );

            delete polygon;
            break;
        }
        default: break;
        }
    }
}

} // end of namespace kvs
