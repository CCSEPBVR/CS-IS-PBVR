/*****************************************************************************/
/**
 *  @file   LineExporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineExporter.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LineExporter.h"
#include <vismodule/Message>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new LineExporter class for KVMLObjectLine format.
 *  @param  object [in] pointer to the input line object
 */
/*===========================================================================*/
LineExporter<vismodule::KVSMLObjectLine>::LineExporter( const vismodule::LineObject& object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Executes the export process.
 *  @param  object [in] pointer to the input object
 *  @return pointer to the KVSMLObjectLine format
 */
/*===========================================================================*/
vismodule::KVSMLObjectLine* LineExporter<vismodule::KVSMLObjectLine>::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::LineObject* line = vismodule::LineObject::DownCast( object );
    if ( !&line )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not line object.");
        return( NULL );
    }

    switch ( line->lineType() )
    {
    case vismodule::LineObject::Strip: this->setLineType( "strip" ); break;
    case vismodule::LineObject::Uniline: this->setLineType( "uniline" ); break;
    case vismodule::LineObject::Polyline: this->setLineType( "polyline" ); break;
    case vismodule::LineObject::Segment:this->setLineType( "segment" ); break;
    default: break;
    }

    switch ( line->colorType() )
    {
    case vismodule::LineObject::VertexColor: this->setColorType( "vertex" ); break;
    case vismodule::LineObject::LineColor: this->setColorType( "line" ); break;
    default: break;
    }

    this->setCoords( line->coords() );
    this->setColors( line->colors() );
    this->setConnections( line->connections() );
    this->setSizes( line->sizes() );

    return( this );
}

} // end of namespace vismodule
