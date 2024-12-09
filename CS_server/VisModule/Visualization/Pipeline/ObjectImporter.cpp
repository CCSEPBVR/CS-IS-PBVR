/****************************************************************************/
/**
 *  @file ObjectImporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectImporter.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ObjectImporter.h"
#include <string>
#include <vismodule/DebugNew>
#include <vismodule/File>
// VIS_MODULE file format classes.
#include <vismodule/AVSField>
#include <vismodule/AVSUcd>
#include <vismodule/Bmp>
#include <vismodule/Ppm>
#include <vismodule/Pgm>
#include <vismodule/Pbm>
#include <vismodule/Stl>
#include <vismodule/Ply>
#include <vismodule/Tiff>
#include <vismodule/Dicom>
#include <vismodule/KVSMLObjectImage>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/KVSMLObjectLine>
#include <vismodule/KVSMLObjectPolygon>
#include <vismodule/KVSMLObjectStructuredVolume>
#include <vismodule/KVSMLObjectUnstructuredVolume>
#include <vismodule/XMLDocument>
#include <vismodule/DicomList>
// VIS_MODULE importer classes.
#include <vismodule/PointImporter>
#include <vismodule/LineImporter>
#include <vismodule/PolygonImporter>
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/ImageImporter>


namespace vismodule
{

ObjectImporter::ObjectImporter( const std::string& filename ):
    m_filename( filename ),
    m_importer_type( ObjectImporter::Unknown ),
    m_file_format( NULL ),
    m_importer( NULL )
{
}

ObjectImporter::~ObjectImporter( void )
{
    // NOTE: Never delete the m_importer in this class,
    //       since it is shared by vismodule::PipelineModule.

    if ( m_file_format ) delete m_file_format;
}

vismodule::ObjectBase* ObjectImporter::import( void )
{
    if ( !this->estimate_file_format() )
    {
        visModuleMessageError( "Cannot create a file format class for '%s'.", m_filename.c_str() );
        return( NULL );
    }

    if ( !this->estimate_importer() )
    {
        visModuleMessageError( "Cannot create a importer class for '%s'.", m_filename.c_str() );
        return( NULL );
    }

    if ( !m_file_format->read( m_filename ) )
    {
        visModuleMessageError( "Cannot read a '%s'.", m_filename.c_str() );
        return( NULL );
    }

    vismodule::ObjectBase* object = m_importer->exec( *m_file_format );
    if ( !object )
    {
        visModuleMessageError( "Cannot import a object." );

        // NOTE: Delete m_importer only when the memory allocation
        //       of m_importer is failed.
        delete m_importer;

        return( NULL );
    }

    return( object );
}

bool ObjectImporter::estimate_file_format( void )
{
    vismodule::File file( m_filename );
    if ( vismodule::AVSField::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::StructuredVolume;
        m_file_format = new vismodule::AVSField;
    }

    else if ( vismodule::AVSUcd::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::UnstructuredVolume;
        m_file_format = new vismodule::AVSUcd;
    }

    else if ( vismodule::Bmp::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new vismodule::Bmp;
    }

    else if ( vismodule::Ppm::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new vismodule::Ppm;
    }

    else if ( vismodule::Pgm::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new vismodule::Pgm;
    }

    else if ( vismodule::Pbm::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new vismodule::Pbm;
    }

    else if ( vismodule::Stl::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Polygon;
        m_file_format = new vismodule::Stl;
    }

    else if ( vismodule::Ply::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Polygon;
        m_file_format = new vismodule::Ply;
    }

    else if ( vismodule::Tiff::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new vismodule::Tiff;
    }

    else if ( vismodule::Dicom::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new vismodule::Dicom;
    }

    else if ( file.extension() == "kvsml" ||
              file.extension() == "KVSML" ||
              file.extension() == "xml"   ||
              file.extension() == "XML" )
    {
        if ( vismodule::KVSMLObjectImage::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Image;
            m_file_format = new vismodule::KVSMLObjectImage;
        }

        else if ( vismodule::KVSMLObjectPoint::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Point;
            m_file_format = new vismodule::KVSMLObjectPoint;
        }

        else if ( vismodule::KVSMLObjectLine::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Line;
            m_file_format = new vismodule::KVSMLObjectLine;
        }

        else if ( vismodule::KVSMLObjectPolygon::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Polygon;
            m_file_format = new vismodule::KVSMLObjectPolygon;
        }

        else if ( vismodule::KVSMLObjectStructuredVolume::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::StructuredVolume;
            m_file_format = new vismodule::KVSMLObjectStructuredVolume;
        }

        else if ( vismodule::KVSMLObjectUnstructuredVolume::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::UnstructuredVolume;
            m_file_format = new vismodule::KVSMLObjectUnstructuredVolume;
        }
    }

    else if ( vismodule::DicomList::CheckDirectory( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::StructuredVolume;
        m_file_format = new vismodule::DicomList;
    }

    return( m_file_format != NULL );
}

bool ObjectImporter::estimate_importer( void )
{
    switch( m_importer_type )
    {
    case ObjectImporter::Point:
    {
        m_importer = new vismodule::PointImporter;
        break;
    }
    case ObjectImporter::Line:
    {
        m_importer = new vismodule::LineImporter;
        break;
    }
    case ObjectImporter::Polygon:
    {
        m_importer = new vismodule::PolygonImporter;
        break;
    }
    case ObjectImporter::StructuredVolume:
    {
        m_importer = new vismodule::StructuredVolumeImporter;
        break;
    }
    case ObjectImporter::UnstructuredVolume:
    {
        m_importer = new vismodule::UnstructuredVolumeImporter;
        break;
    }
    case ObjectImporter::Image:
    {
        m_importer = new vismodule::ImageImporter;
        break;
    }
    default: break;
    }

    return( m_importer != NULL );
}

} // end of namespace vismodule
