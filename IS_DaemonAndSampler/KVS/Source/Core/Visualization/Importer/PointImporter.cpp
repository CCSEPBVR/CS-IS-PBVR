/****************************************************************************/
/**
 *  @file PointImporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointImporter.cpp 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "PointImporter.h"
#include <kvs/DebugNew>
#include <kvs/KVSMLObjectPoint>
#include <kvs/Math>
#include <kvs/Vector3>
#include <string>


namespace kvs
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new PointImporter class.
 */
/*==========================================================================*/
PointImporter::PointImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointImporter class.
 *  @param  filename [in] input filename
 */
/*===========================================================================*/
PointImporter::PointImporter( const std::string& filename )
{
    if ( kvs::KVSMLObjectPoint::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectPoint* file_format = new kvs::KVSMLObjectPoint( filename );
        if( !file_format )
        {
            BaseClass::m_is_success = false;
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Cannot import '%'.",filename.c_str());
        return;
    }
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new PointImporter class.
 *  @param file_format [in] pointer to the file format
 */
/*==========================================================================*/
PointImporter::PointImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the PointImporter class.
 */
/*===========================================================================*/
PointImporter::~PointImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the import process.
 *  @param  file_format [in] pointer to the file format class
 *  @return pointer to the imported point object
 */
/*===========================================================================*/
PointImporter::SuperClass* PointImporter::exec( const kvs::FileFormatBase* file_format )
{
    if ( !file_format )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name = file_format->className();
    if ( class_name == "kvs::KVSMLObjectPoint" )
    {
        this->import( static_cast<const kvs::KVSMLObjectPoint*>( file_format ) );
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is not supported.");
        return( NULL );
    }

    return( this );
}

/*==========================================================================*/
/**
 *  @brief  Imports KVSML format data.
 *  @param  kvsml [in] pointer to the KVSML format data
 */
/*==========================================================================*/
void PointImporter::import( const kvs::KVSMLObjectPoint* kvsml )
{
    if ( kvsml->objectTag().hasExternalCoord() )
    {
        const kvs::Vector3f min_coord( kvsml->objectTag().minExternalCoord() );
        const kvs::Vector3f max_coord( kvsml->objectTag().maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( kvsml->objectTag().hasObjectCoord() )
    {
        const kvs::Vector3f min_coord( kvsml->objectTag().minObjectCoord() );
        const kvs::Vector3f max_coord( kvsml->objectTag().maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    SuperClass::setCoords( kvsml->coords() );
    SuperClass::setColors( kvsml->colors() );
    SuperClass::setNormals( kvsml->normals() );
    SuperClass::setSizes( kvsml->sizes() );
//    SuperClass::updateMinMaxCoords();
    this->set_min_max_coord();

    BaseClass::m_is_success = true;
}

/*==========================================================================*/
/**
 *  @brief  Calculates the min/max coordinate values.
 */
/*==========================================================================*/
void PointImporter::set_min_max_coord( void )
{
    kvs::Vector3f min_coord( m_coords[0], m_coords[1], m_coords[2] );
    kvs::Vector3f max_coord( min_coord );
    const size_t  dimension = 3;
    const size_t  nvertices = m_coords.size() / dimension;
    size_t        index3    = 3;
    for ( size_t i = 1; i < nvertices; i++, index3 += 3 )
    {
        min_coord.x() = kvs::Math::Min( min_coord.x(), m_coords[index3] );
        min_coord.y() = kvs::Math::Min( min_coord.y(), m_coords[index3 + 1] );
        min_coord.z() = kvs::Math::Min( min_coord.z(), m_coords[index3 + 2] );

        max_coord.x() = kvs::Math::Max( max_coord.x(), m_coords[index3] );
        max_coord.y() = kvs::Math::Max( max_coord.y(), m_coords[index3 + 1] );
        max_coord.z() = kvs::Math::Max( max_coord.z(), m_coords[index3 + 2] );
    }

    this->setMinMaxObjectCoords( min_coord, max_coord );
    this->setMinMaxExternalCoords( min_coord, max_coord );
}

} // end of namespace kvs
