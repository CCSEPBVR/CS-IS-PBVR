/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "UnstructuredVolumeObjectExporter.h"

kvs::FileFormatBase* cvt::UnstructuredVolumeObjectExporter::exec( const kvs::ObjectBase* object )
{
    BaseClass::setSuccess( true );

    if ( !object )
    {
        BaseClass::setSuccess( false );
        throw std::runtime_error( "Input object is NULL." );
        return nullptr;
    }

    // Cast to the structured volume object.
    const kvs::UnstructuredVolumeObject* volume = kvs::UnstructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        BaseClass::setSuccess( false );
        throw std::runtime_error( "Input object is not structured volume object." );
        return nullptr;
    }

    if ( volume->label() != "" )
    {
        this->setLabel( volume->label() );
    }
    if ( volume->unit() != "" )
    {
        this->setUnit( volume->unit() );
    }

    // Check the cell type of the given unstructured volume object.
    switch ( volume->cellType() )
    {
    case kvs::UnstructuredVolumeObject::UnknownCellType: {
        kvsMessageError( "Unknown cell type." );
        break;
    }
    case kvs::UnstructuredVolumeObject::Tetrahedra: {
        this->setCellType( "tetrahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticTetrahedra: {
        this->setCellType( "quadratic tetrahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Hexahedra: {
        this->setCellType( "hexahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticHexahedra: {
        this->setCellType( "quadratic hexahedra" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Pyramid: {
        this->setCellType( "pyramid" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Prism: {
        this->setCellType( "prism" );
        break;
    }
    case kvs::UnstructuredVolumeObject::Point: {
        this->setCellType( "point" );
        break;
    }
    default: {
        BaseClass::setSuccess( false );
        kvsMessageError( "Not supported cell type." );
        break;
    }
    }

    this->setVeclen( volume->veclen() );
    this->setNNodes( volume->numberOfNodes() );
    this->setNCells( volume->numberOfCells() );
    this->setValues( volume->values() );
    this->setCoords( volume->coords() );
    this->setConnections( volume->connections() );
    this->setMinMaxObjectCoords( object->minObjectCoord(), object->maxObjectCoord() );
    this->setMinMaxExternalCoords( object->minExternalCoord(), object->maxExternalCoord() );

    if ( volume->hasMinMaxValues() )
    {
        this->setMinValue( volume->minValue() );
        this->setMaxValue( volume->maxValue() );
    }

    return this;
}