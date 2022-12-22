/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#include "StructuredVolumeObjectExporter.h"

#include "kvs/Message"
#include "kvs/StructuredVolumeObject"

kvs::FileFormatBase* cvt::StructuredVolumeObjectExporter::exec( const kvs::ObjectBase* object )
{
    BaseClass::setSuccess( false );

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
            kvsMessageError( "Not supported cell type." );
            return nullptr;
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

        BaseClass::setSuccess( true );

        return this;
    }
    else
    {
        kvsMessageError( "Input object is not structured volume object." );
        return nullptr;
    }
}