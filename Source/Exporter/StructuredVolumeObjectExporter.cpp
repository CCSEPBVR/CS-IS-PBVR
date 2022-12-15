/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "StructuredVolumeObjectExporter.h"

#include "kvs/StructuredVolumeObject"

kvs::FileFormatBase* cvt::StructuredVolumeObjectExporter::exec( const kvs::ObjectBase* object )
{
    if ( auto volume = dynamic_cast<const kvs::StructuredVolumeObject*>( object ) )
    {
        switch ( volume->gridType() )
        {
        case kvs::StructuredVolumeObject::Uniform: {
            this->setGridType( "uniform" );
            break;
        }
        case kvs::StructuredVolumeObject::Rectilinear:
            this->setGridType( "rectilinear" );
            this->setCoords( volume->coords() );
            break;
        case kvs::StructuredVolumeObject::Curvilinear:
            this->setGridType( "curvilinear" );
            this->setCoords( volume->coords() );
            break;
        default: {
            BaseClass::setSuccess( false );
            throw std::runtime_error( "Unknown grid type." );
            break;
        }
        }
        this->setVeclen( volume->veclen() );
        this->setResolution( volume->resolution() );
        this->setValues( volume->values() );

        if ( volume->hasMinMaxValues() )
        {
            this->setMinValue( volume->minValue() );
            this->setMaxValue( volume->maxValue() );
        }
        if ( volume->hasMinMaxExternalCoords() )
        {
            this->setMinMaxExternalCoords( volume->minExternalCoord(), volume->maxExternalCoord() );
        }
        if ( volume->hasMinMaxObjectCoords() )
        {
            this->setMinMaxObjectCoords( volume->minObjectCoord(), volume->maxObjectCoord() );
        }

        return this;
    }
    else
    {
        return nullptr;
    }
}