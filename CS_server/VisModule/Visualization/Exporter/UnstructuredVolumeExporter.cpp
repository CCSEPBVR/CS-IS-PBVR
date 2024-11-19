/*****************************************************************************/
/**
 *  @file   UnstructuredVolumeExporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeExporter.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "UnstructuredVolumeExporter.h"
#include <vismodule/ObjectBase>
#include <vismodule/VolumeObjectBase>
#include <vismodule/UnstructuredVolumeObject>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a KVSMLObjectUnstructuredVolume data from given object.
 *  @param  object [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
UnstructuredVolumeExporter<vismodule::KVSMLObjectUnstructuredVolume>::UnstructuredVolumeExporter(
    const vismodule::UnstructuredVolumeObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports to a KVSMLObjectUnstructuredVolume data.
 *  @param  object [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
vismodule::KVSMLObjectUnstructuredVolume* UnstructuredVolumeExporter<vismodule::KVSMLObjectUnstructuredVolume>::exec(
    const vismodule::ObjectBase* object )
{
    if ( !object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    // Cast to the structured volume object.
    const vismodule::UnstructuredVolumeObject* volume = vismodule::UnstructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not structured volume object.");
        return( NULL );
    }

    // Check the cell type of the given unstructured volume object.
    switch ( volume->cellType() )
    {
    case vismodule::UnstructuredVolumeObject::UnknownCellType:
    {
        visModuleMessageError("Unknown cell type.");
        break;
    }
    case vismodule::UnstructuredVolumeObject::Tetrahedra:
    {
        this->setCellType("tetrahedra");
        break;
    }
    case vismodule::UnstructuredVolumeObject::QuadraticTetrahedra:
    {
        this->setCellType("quadratic tetrahedra");
        break;
    }
    case vismodule::UnstructuredVolumeObject::Hexahedra:
    {
        this->setCellType("hexahedra");
        break;
    }
    case vismodule::UnstructuredVolumeObject::QuadraticHexahedra:
    {
        this->setCellType("quadratic hexahedra");
        break;
    }
    case vismodule::UnstructuredVolumeObject::Pyramid:
    {
        this->setCellType("pyramid");
        break;
    }
    case vismodule::UnstructuredVolumeObject::Point:
    {
        this->setCellType("point");
        break;
    }
    default:
    {
        m_is_success = false;
        visModuleMessageError("Not supported cell type.");
        break;
    }
    }

    this->setVeclen( volume->veclen() );
    this->setNNodes( volume->nnodes() );
    this->setNCells( volume->ncells() );
    this->setValues( volume->values() );
    this->setCoords( volume->coords() );
    this->setConnections( volume->connections() );

    return( this );
}

} // end of namespace vismodule
