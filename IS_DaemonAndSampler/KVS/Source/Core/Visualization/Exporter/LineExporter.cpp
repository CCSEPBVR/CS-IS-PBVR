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
#include <kvs/Message>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new LineExporter class for KVMLObjectLine format.
 *  @param  object [in] pointer to the input line object
 */
/*===========================================================================*/
LineExporter<kvs::KVSMLObjectLine>::LineExporter( const kvs::LineObject* object )
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
kvs::KVSMLObjectLine* LineExporter<kvs::KVSMLObjectLine>::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    const kvs::LineObject* line = kvs::LineObject::DownCast( object );
    if ( !line )
    {
        m_is_success = false;
        kvsMessageError("Input object is not line object.");
        return( NULL );
    }

    switch ( line->lineType() )
    {
    case kvs::LineObject::Strip: this->setLineType( "strip" ); break;
    case kvs::LineObject::Uniline: this->setLineType( "uniline" ); break;
    case kvs::LineObject::Polyline: this->setLineType( "polyline" ); break;
    case kvs::LineObject::Segment:this->setLineType( "segment" ); break;
    default: break;
    }

    switch ( line->colorType() )
    {
    case kvs::LineObject::VertexColor: this->setColorType( "vertex" ); break;
    case kvs::LineObject::LineColor: this->setColorType( "line" ); break;
    default: break;
    }

    this->setCoords( line->coords() );
    this->setColors( line->colors() );
    this->setConnections( line->connections() );
    this->setSizes( line->sizes() );

    return( this );
}

} // end of namespace kvs
