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
#include <vismodule/DebugNew>
#include <vismodule/KVSMLObjectPolygon>
#include <vismodule/Math>
#include <vismodule/Vector3>



namespace
{

/*==========================================================================*/
/**
 *  @breif  Returns the polygon type from the given string.
 *  @param  polygon_type [in] polygon type string
 *  @return polygon type
 */
/*==========================================================================*/
const vismodule::PolygonObject::PolygonType StringToPolygonType( const std::string& polygon_type )
{
    if (      polygon_type == "triangle"   ) { return( vismodule::PolygonObject::Triangle ); }
    else if ( polygon_type == "quadrangle" ) { return( vismodule::PolygonObject::Quadrangle ); }
    else
    {
        visModuleMessageError( "Unknown polygon type '%s'.", polygon_type.c_str() );
        return( vismodule::PolygonObject::UnknownPolygonType );
    }
}

/*==========================================================================*/
/**
 *  @brief  Returns the polygon color type from the given string.
 *  @param  color_type [in] polygon color type string
 *  @return polygon color type
 */
/*==========================================================================*/
const vismodule::PolygonObject::ColorType StringToColorType( const std::string& color_type )
{
    if (      color_type == "vertex"  ) { return( vismodule::PolygonObject::VertexColor ); }
    else if ( color_type == "polygon" ) { return( vismodule::PolygonObject::PolygonColor ); }
    else
    {
        visModuleMessageError( "Unknown polygon color type '%s'.", color_type.c_str() );
        return( vismodule::PolygonObject::UnknownColorType );
    }
}

/*==========================================================================*/
/**
 *  @brief  Returns the polygon normal type from the given string.
 *  @param  normal_type [in] polygon normal type string
 *  @return polygon normal type
 */
/*==========================================================================*/
const vismodule::PolygonObject::NormalType StringToNormalType( const std::string& normal_type )
{
    if (      normal_type == "vertex"  ) { return( vismodule::PolygonObject::VertexNormal ); }
    else if ( normal_type == "polygon" ) { return( vismodule::PolygonObject::PolygonNormal ); }
    else
    {
        visModuleMessageError( "Unknown polygon normal type '%s'.", normal_type.c_str() );
        return( vismodule::PolygonObject::UnknownNormalType );
    }
}

} // end of namespace


namespace vismodule
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
    if ( vismodule::KVSMLObjectPolygon::CheckFileExtension( filename ) )
    {
        vismodule::KVSMLObjectPolygon* file_format = new vismodule::KVSMLObjectPolygon( filename );
        if( !file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else if ( vismodule::Stl::CheckFileExtension( filename ) )
    {
        vismodule::Stl* file_format = new vismodule::Stl( filename );
        if( !file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else if ( vismodule::Ply::CheckFileExtension( filename ) )
    {
        vismodule::Ply* file_format = new vismodule::Ply( filename );
        if( !file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Cannot import '%s'.",filename.c_str());
        return;
    }
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new PolygonImporter class.
 *  @param  file_format [in] pointer to the file format
 */
/*==========================================================================*/
PolygonImporter::PolygonImporter( const vismodule::FileFormatBase& file_format )
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
PolygonImporter::SuperClass* PolygonImporter::exec( const vismodule::FileFormatBase& file_format )
{
    if ( !&file_format )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name = file_format.className();
    if ( class_name == "vismodule::KVSMLObjectPolygon" )
    {
        this->import( static_cast<const vismodule::KVSMLObjectPolygon*>( &file_format ) );
    }
    else if ( class_name == "vismodule::Stl" )
    {
        this->import( static_cast<const vismodule::Stl*>( &file_format ) );
    }
    else if ( class_name == "vismodule::Ply" )
    {
        this->import( static_cast<const vismodule::Ply*>( &file_format ) );
    }
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input file format is not supported.");
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
void PolygonImporter::import( const vismodule::KVSMLObjectPolygon* kvsml )
{
    if ( kvsml->objectTag().hasExternalCoord() )
    {
        const vismodule::Vector3f min_coord( kvsml->objectTag().minExternalCoord() );
        const vismodule::Vector3f max_coord( kvsml->objectTag().maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( kvsml->objectTag().hasObjectCoord() )
    {
        const vismodule::Vector3f min_coord( kvsml->objectTag().minObjectCoord() );
        const vismodule::Vector3f max_coord( kvsml->objectTag().maxObjectCoord() );
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
void PolygonImporter::import( const vismodule::Stl* stl )
{
    m_polygon_type = vismodule::PolygonObject::Triangle;
    m_color_type = vismodule::PolygonObject::PolygonColor;
    m_normal_type = vismodule::PolygonObject::PolygonNormal;

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

void PolygonImporter::import( const vismodule::Ply* ply )
{
    SuperClass::setPolygonType( vismodule::PolygonObject::Triangle );
    SuperClass::setNormalType( vismodule::PolygonObject::VertexNormal );

    SuperClass::setCoords( ply->coords() );
    SuperClass::setNormals( ply->normals() );
    SuperClass::setOpacity( 255 );

    if ( ply->hasColors() )
    {
        SuperClass::setColorType( vismodule::PolygonObject::VertexColor );
        SuperClass::setColors( ply->colors() );
    }
    else
    {
        SuperClass::setColorType( vismodule::PolygonObject::PolygonColor );
        SuperClass::setColor( vismodule::RGBColor( 255, 255, 255 ) );
    }

    if ( ply->hasConnections() )
    {
        SuperClass::setConnections( ply->connections() );
    }

    const vismodule::Vector3f min_coord( ply->minCoord().x(), ply->minCoord().y(), ply->minCoord().z() );
    const vismodule::Vector3f max_coord( ply->maxCoord().x(), ply->maxCoord().y(), ply->maxCoord().z() );
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
    vismodule::Vector3f min_coord( m_coords[0], m_coords[1], m_coords[2] );
    vismodule::Vector3f max_coord( min_coord );
    const size_t  dimension = 3;
    const size_t  nvertices = m_coords.size() / dimension;
    size_t        index3    = 3;
    for ( size_t i = 1; i < nvertices; i++, index3 += 3 )
    {
        min_coord.x() = vismodule::Math::Min( min_coord.x(), m_coords[index3] );
        min_coord.y() = vismodule::Math::Min( min_coord.y(), m_coords[index3 + 1] );
        min_coord.z() = vismodule::Math::Min( min_coord.z(), m_coords[index3 + 2] );

        max_coord.x() = vismodule::Math::Max( max_coord.x(), m_coords[index3] );
        max_coord.y() = vismodule::Math::Max( max_coord.y(), m_coords[index3 + 1] );
        max_coord.z() = vismodule::Math::Max( max_coord.z(), m_coords[index3 + 2] );
    }

    this->setMinMaxObjectCoords( min_coord, max_coord );
    this->setMinMaxExternalCoords( min_coord, max_coord );
}

} // end of namespace vismodule
