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
#include <kvs/ObjectBase>
#include <kvs/VolumeObjectBase>
#include <kvs/UnstructuredVolumeObject>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a KVSMLObjectUnstructuredVolume data from given object.
 *  @param  object [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
UnstructuredVolumeExporter<kvs::KVSMLObjectUnstructuredVolume>::UnstructuredVolumeExporter(
    const kvs::UnstructuredVolumeObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports to a KVSMLObjectUnstructuredVolume data.
 *  @param  object [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
kvs::KVSMLObjectUnstructuredVolume* UnstructuredVolumeExporter<kvs::KVSMLObjectUnstructuredVolume>::exec(
    const kvs::ObjectBase* object )
{
    if ( !object )
    {
        m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    // Cast to the structured volume object.
    const kvs::UnstructuredVolumeObject* volume = kvs::UnstructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        m_is_success = false;
        kvsMessageError("Input object is not structured volume object.");
        return( NULL );
    }

    // Check the cell type of the given unstructured volume object.
    switch ( volume->cellType() )
    {
    case kvs::UnstructuredVolumeObject::UnknownCellType:
    {
        kvsMessageError("Unknown cell type.");
        break;
    }
    case kvs::UnstructuredVolumeObject::Tetrahedra:
    {
        this->setCellType("tetrahedra");
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticTetrahedra:
    {
        this->setCellType("quadratic tetrahedra");
        break;
    }
    case kvs::UnstructuredVolumeObject::Hexahedra:
    {
        this->setCellType("hexahedra");
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticHexahedra:
    {
        this->setCellType("quadratic hexahedra");
        break;
    }
    case kvs::UnstructuredVolumeObject::Pyramid:
    {
        this->setCellType("pyramid");
        break;
    }
    case kvs::UnstructuredVolumeObject::Point:
    {
        this->setCellType("point");
        break;
    }
    default:
    {
        m_is_success = false;
        kvsMessageError("Not supported cell type.");
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

} // end of namespace kvs
