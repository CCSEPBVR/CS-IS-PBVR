/*****************************************************************************/
/**
 *  @file   PointExporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointExporter.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "PointExporter.h"
#include <kvs/Message>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointExporter class for KVMLObjectPoint format.
 *  @param  object [in] pointer to the input point object
 */
/*===========================================================================*/
PointExporter<kvs::KVSMLObjectPoint>::PointExporter( const kvs::PointObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Executes the export process.
 *  @param  object [in] pointer to the input object
 *  @return pointer to the KVSMLObjectPoint format
 */
/*===========================================================================*/
kvs::KVSMLObjectPoint* PointExporter<kvs::KVSMLObjectPoint>::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    const kvs::PointObject* point = kvs::PointObject::DownCast( object );
    if ( !point )
    {
        m_is_success = false;
        kvsMessageError("Input object is not point object.");
        return( NULL );
    }

    this->setCoords( point->coords() );
    this->setColors( point->colors() );
    this->setNormals( point->normals() );
    this->setSizes( point->sizes() );

    return( this );
}

} // end of namespace kvs
