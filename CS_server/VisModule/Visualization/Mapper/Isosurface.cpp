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
#include <vismodule/DebugNew>
#include <vismodule/MarchingCubes>
#include <vismodule/MarchingTetrahedra>
#include <vismodule/MarchingHexahedra>
#include <vismodule/MarchingPyramid>


namespace vismodule
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new Isosurface class.
 */
/*==========================================================================*/
Isosurface::Isosurface( void ):
    vismodule::MapperBase(),
    vismodule::PolygonObject(),
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
    const vismodule::VolumeObjectBase& volume,
    const double                 isolevel,
    const NormalType             normal_type ):
    vismodule::MapperBase(),
    vismodule::PolygonObject(),
    m_isolevel( isolevel ),
    m_duplication( true )
{
    SuperClass::m_normal_type = normal_type;

    // In the case of VertexNormal-type, the duplicated vertices are forcibly deleted.
    if ( normal_type == vismodule::PolygonObject::VertexNormal )
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
    const vismodule::VolumeObjectBase& volume,
    const double                 isolevel,
    const NormalType             normal_type,
    const bool                   duplication,
    const vismodule::TransferFunction& transfer_function ):
    vismodule::MapperBase( transfer_function ),
    vismodule::PolygonObject(),
    m_isolevel( isolevel ),
    m_duplication( duplication )
{
    SuperClass::m_normal_type = normal_type;

    // In the case of VertexNormal-type, the duplicated vertices are forcibly deleted.
    if ( normal_type == vismodule::PolygonObject::VertexNormal )
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
Isosurface::SuperClass* Isosurface::exec( const vismodule::ObjectBase& object )
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

    this->mapping( *volume );

    return( this );
}

/*==========================================================================*/
/**
 *  @brief  Extracts the surfaces.
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
void Isosurface::mapping( const vismodule::VolumeObjectBase& volume )
{
    // Check whether the volume can be processed or not.
    if ( volume.veclen() != 1 )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not scalar field dat.");
        return;
    }

    if ( volume.volumeType() == vismodule::VolumeObjectBase::Structured )
    {
        const vismodule::StructuredVolumeObject* structured_volume =
            vismodule::StructuredVolumeObject::DownCast( volume );

        vismodule::PolygonObject* polygon = new vismodule::MarchingCubes(
            *structured_volume,
            m_isolevel,
            SuperClass::normalType(),
            m_duplication,
            BaseClass::transferFunction() );
        if ( !polygon )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot create isosurfaces.");
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
    else // volume.volumeType() == vismodule::VolumeObjectBase::Unstructured
    {
        const vismodule::UnstructuredVolumeObject* unstructured_volume =
            vismodule::UnstructuredVolumeObject::DownCast( volume );

        switch ( unstructured_volume->cellType() )
        {
        case vismodule::VolumeObjectBase::Tetrahedra:
        {
            vismodule::PolygonObject* polygon = new vismodule::MarchingTetrahedra(
                *unstructured_volume,
                m_isolevel,
                SuperClass::normalType(),
                m_duplication,
                BaseClass::transferFunction() );
            if ( !polygon )
            {
                BaseClass::m_is_success = false;
                visModuleMessageError("Cannot create isosurfaces.");
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
        case vismodule::VolumeObjectBase::Hexahedra:
        {
            vismodule::PolygonObject* polygon = new vismodule::MarchingHexahedra(
                *unstructured_volume,
                m_isolevel,
                SuperClass::normalType(),
                m_duplication,
                BaseClass::transferFunction() );
            if ( !polygon )
            {
                visModuleMessageError("Cannot create isosurfaces.");
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
        case vismodule::VolumeObjectBase::Pyramid:
        {
            vismodule::PolygonObject* polygon = new vismodule::MarchingPyramid(
                *unstructured_volume,
                m_isolevel,
                SuperClass::normalType(),
                m_duplication,
                BaseClass::transferFunction() );
            if ( !polygon )
            {
                BaseClass::m_is_success = false;
                visModuleMessageError("Cannot create isosurfaces.");
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

} // end of namespace vismodule
