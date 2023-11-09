/****************************************************************************/
/**
 *  @file PolygonImporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PolygonImporter.cpp 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "PolygonImporter.h"
#include <kvs/DebugNew>
#include <kvs/KVSMLObjectPolygon>
#include <kvs/Math>
#include <kvs/Vector3>



namespace
{

/*==========================================================================*/
/**
 *  @breif  Returns the polygon type from the given string.
 *  @param  polygon_type [in] polygon type string
 *  @return polygon type
 */
/*==========================================================================*/
const kvs::PolygonObject::PolygonType StringToPolygonType( const std::string& polygon_type )
{
    if (      polygon_type == "triangle"   ) { return( kvs::PolygonObject::Triangle ); }
    else if ( polygon_type == "quadrangle" ) { return( kvs::PolygonObject::Quadrangle ); }
    else
    {
        kvsMessageError( "Unknown polygon type '%s'.", polygon_type.c_str() );
        return( kvs::PolygonObject::UnknownPolygonType );
    }
}

/*==========================================================================*/
/**
 *  @brief  Returns the polygon color type from the given string.
 *  @param  color_type [in] polygon color type string
 *  @return polygon color type
 */
/*==========================================================================*/
const kvs::PolygonObject::ColorType StringToColorType( const std::string& color_type )
{
    if (      color_type == "vertex"  ) { return( kvs::PolygonObject::VertexColor ); }
    else if ( color_type == "polygon" ) { return( kvs::PolygonObject::PolygonColor ); }
    else
    {
        kvsMessageError( "Unknown polygon color type '%s'.", color_type.c_str() );
        return( kvs::PolygonObject::UnknownColorType );
    }
}

/*==========================================================================*/
/**
 *  @brief  Returns the polygon normal type from the given string.
 *  @param  normal_type [in] polygon normal type string
 *  @return polygon normal type
 */
/*==========================================================================*/
const kvs::PolygonObject::NormalType StringToNormalType( const std::string& normal_type )
{
    if (      normal_type == "vertex"  ) { return( kvs::PolygonObject::VertexNormal ); }
    else if ( normal_type == "polygon" ) { return( kvs::PolygonObject::PolygonNormal ); }
    else
    {
        kvsMessageError( "Unknown polygon normal type '%s'.", normal_type.c_str() );
        return( kvs::PolygonObject::UnknownNormalType );
    }
}

} // end of namespace


namespace kvs
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new PolygonImporter class.
 */
/*==========================================================================*/
PolygonImporter::PolygonImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PolygonImporter class.
 *  @param  filename [in] input filename
 */
/*===========================================================================*/
PolygonImporter::PolygonImporter( const std::string& filename )
{
    if ( kvs::KVSMLObjectPolygon::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectPolygon* file_format = new kvs::KVSMLObjectPolygon( filename );
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
    else if ( kvs::Stl::CheckFileExtension( filename ) )
    {
        kvs::Stl* file_format = new kvs::Stl( filename );
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
    else if ( kvs::Ply::CheckFileExtension( filename ) )
    {
        kvs::Ply* file_format = new kvs::Ply( filename );
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
        kvsMessageError("Cannot import '%s'.",filename.c_str());
        return;
    }
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new PolygonImporter class.
 *  @param  file_format [in] pointer to the file format
 */
/*==========================================================================*/
PolygonImporter::PolygonImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the PolygonImporter class.
 */
/*===========================================================================*/
PolygonImporter::~PolygonImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the import process.
 *  @param  file_format [in] pointer to the file format class
 *  @return pointer to the imported polygon object
 */
/*===========================================================================*/
PolygonImporter::SuperClass* PolygonImporter::exec( const kvs::FileFormatBase* file_format )
{
    if ( !file_format )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name = file_format->className();
    if ( class_name == "kvs::KVSMLObjectPolygon" )
    {
        this->import( static_cast<const kvs::KVSMLObjectPolygon*>( file_format ) );
    }
    else if ( class_name == "kvs::Stl" )
    {
        this->import( static_cast<const kvs::Stl*>( file_format ) );
    }
    else if ( class_name == "kvs::Ply" )
    {
        this->import( static_cast<const kvs::Ply*>( file_format ) );
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
 *  @breif  Imports the KVSML format data.
 *  @param  document [in] pointer to the KVSML document
 */
/*==========================================================================*/
void PolygonImporter::import( const kvs::KVSMLObjectPolygon* kvsml )
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

    SuperClass::setPolygonType( ::StringToPolygonType( kvsml->polygonType() ) );
    SuperClass::setColorType( ::StringToColorType( kvsml->colorType() ) );
    SuperClass::setNormalType( ::StringToNormalType( kvsml->normalType() ) );
    SuperClass::setCoords( kvsml->coords() );
    SuperClass::setColors( kvsml->colors() );
    SuperClass::setNormals( kvsml->normals() );
    SuperClass::setConnections( kvsml->connections() );
    SuperClass::setOpacities( kvsml->opacities() );
//    SuperClass::updateMinMaxCoords();
    this->set_min_max_coord();
}

/*==========================================================================*/
/**
 *  @brief  Imports the STL format data.
 *  @param  stl [in] pointer to the STL format file
 */
/*==========================================================================*/
void PolygonImporter::import( const kvs::Stl* stl )
{
    m_polygon_type = kvs::PolygonObject::Triangle;
    m_color_type = kvs::PolygonObject::PolygonColor;
    m_normal_type = kvs::PolygonObject::PolygonNormal;

    m_coords = stl->coords();
    m_normals = stl->normals();

    m_colors.allocate( 3 );
    m_colors[0] = 255;
    m_colors[1] = 255;
    m_colors[2] = 255;

    m_opacities.allocate( 1 );
    m_opacities[0] = 255;

    this->set_min_max_coord();
}

void PolygonImporter::import( const kvs::Ply* ply )
{
    SuperClass::setPolygonType( kvs::PolygonObject::Triangle );
    SuperClass::setNormalType( kvs::PolygonObject::VertexNormal );

    SuperClass::setCoords( ply->coords() );
    SuperClass::setNormals( ply->normals() );
    SuperClass::setOpacity( 255 );

    if ( ply->hasColors() )
    {
        SuperClass::setColorType( kvs::PolygonObject::VertexColor );
        SuperClass::setColors( ply->colors() );
    }
    else
    {
        SuperClass::setColorType( kvs::PolygonObject::PolygonColor );
        SuperClass::setColor( kvs::RGBColor( 255, 255, 255 ) );
    }

    if ( ply->hasConnections() )
    {
        SuperClass::setConnections( ply->connections() );
    }

    const kvs::Vector3f min_coord( ply->minCoord().x(), ply->minCoord().y(), ply->minCoord().z() );
    const kvs::Vector3f max_coord( ply->maxCoord().x(), ply->maxCoord().y(), ply->maxCoord().z() );
    SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
}

/*==========================================================================*/
/**
 *  @brief  Calculates the min/max coordinate values.
 */
/*==========================================================================*/
void PolygonImporter::set_min_max_coord( void )
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
