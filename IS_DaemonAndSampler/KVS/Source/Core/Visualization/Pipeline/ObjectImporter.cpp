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
#include <kvs/DebugNew>
#include <kvs/File>
// KVS file format classes.
#include <kvs/AVSField>
#include <kvs/AVSUcd>
#include <kvs/Bmp>
#include <kvs/Ppm>
#include <kvs/Pgm>
#include <kvs/Pbm>
#include <kvs/Stl>
#include <kvs/Ply>
#include <kvs/Tiff>
#include <kvs/Dicom>
#include <kvs/KVSMLObjectImage>
#include <kvs/KVSMLObjectPoint>
#include <kvs/KVSMLObjectLine>
#include <kvs/KVSMLObjectPolygon>
#include <kvs/KVSMLObjectStructuredVolume>
#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/XMLDocument>
#include <kvs/DicomList>
// KVS importer classes.
#include <kvs/PointImporter>
#include <kvs/LineImporter>
#include <kvs/PolygonImporter>
#include <kvs/StructuredVolumeImporter>
#include <kvs/UnstructuredVolumeImporter>
#include <kvs/ImageImporter>


namespace kvs
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
    //       since it is shared by kvs::PipelineModule.

    if ( m_file_format ) delete m_file_format;
}

kvs::ObjectBase* ObjectImporter::import( void )
{
    if ( !this->estimate_file_format() )
    {
        kvsMessageError( "Cannot create a file format class for '%s'.", m_filename.c_str() );
        return( NULL );
    }

    if ( !this->estimate_importer() )
    {
        kvsMessageError( "Cannot create a importer class for '%s'.", m_filename.c_str() );
        return( NULL );
    }

    if ( !m_file_format->read( m_filename ) )
    {
        kvsMessageError( "Cannot read a '%s'.", m_filename.c_str() );
        return( NULL );
    }

    kvs::ObjectBase* object = m_importer->exec( m_file_format );
    if ( !object )
    {
        kvsMessageError( "Cannot import a object." );

        // NOTE: Delete m_importer only when the memory allocation
        //       of m_importer is failed.
        delete m_importer;

        return( NULL );
    }

    return( object );
}

bool ObjectImporter::estimate_file_format( void )
{
    kvs::File file( m_filename );
    if ( kvs::AVSField::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::StructuredVolume;
        m_file_format = new kvs::AVSField;
    }

    else if ( kvs::AVSUcd::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::UnstructuredVolume;
        m_file_format = new kvs::AVSUcd;
    }

    else if ( kvs::Bmp::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new kvs::Bmp;
    }

    else if ( kvs::Ppm::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new kvs::Ppm;
    }

    else if ( kvs::Pgm::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new kvs::Pgm;
    }

    else if ( kvs::Pbm::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new kvs::Pbm;
    }

    else if ( kvs::Stl::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Polygon;
        m_file_format = new kvs::Stl;
    }

    else if ( kvs::Ply::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Polygon;
        m_file_format = new kvs::Ply;
    }

    else if ( kvs::Tiff::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new kvs::Tiff;
    }

    else if ( kvs::Dicom::CheckFileExtension( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::Image;
        m_file_format = new kvs::Dicom;
    }

    else if ( file.extension() == "kvsml" ||
              file.extension() == "KVSML" ||
              file.extension() == "xml"   ||
              file.extension() == "XML" )
    {
        if ( kvs::KVSMLObjectImage::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Image;
            m_file_format = new kvs::KVSMLObjectImage;
        }

        else if ( kvs::KVSMLObjectPoint::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Point;
            m_file_format = new kvs::KVSMLObjectPoint;
        }

        else if ( kvs::KVSMLObjectLine::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Line;
            m_file_format = new kvs::KVSMLObjectLine;
        }

        else if ( kvs::KVSMLObjectPolygon::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::Polygon;
            m_file_format = new kvs::KVSMLObjectPolygon;
        }

        else if ( kvs::KVSMLObjectStructuredVolume::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::StructuredVolume;
            m_file_format = new kvs::KVSMLObjectStructuredVolume;
        }

        else if ( kvs::KVSMLObjectUnstructuredVolume::CheckFileFormat( file.filePath() ) )
        {
            m_importer_type = ObjectImporter::UnstructuredVolume;
            m_file_format = new kvs::KVSMLObjectUnstructuredVolume;
        }
    }

    else if ( kvs::DicomList::CheckDirectory( file.filePath() ) )
    {
        m_importer_type = ObjectImporter::StructuredVolume;
        m_file_format = new kvs::DicomList;
    }

    return( m_file_format != NULL );
}

bool ObjectImporter::estimate_importer( void )
{
    switch( m_importer_type )
    {
    case ObjectImporter::Point:
    {
        m_importer = new kvs::PointImporter;
        break;
    }
    case ObjectImporter::Line:
    {
        m_importer = new kvs::LineImporter;
        break;
    }
    case ObjectImporter::Polygon:
    {
        m_importer = new kvs::PolygonImporter;
        break;
    }
    case ObjectImporter::StructuredVolume:
    {
        m_importer = new kvs::StructuredVolumeImporter;
        break;
    }
    case ObjectImporter::UnstructuredVolume:
    {
        m_importer = new kvs::UnstructuredVolumeImporter;
        break;
    }
    case ObjectImporter::Image:
    {
        m_importer = new kvs::ImageImporter;
        break;
    }
    default: break;
    }

    return( m_importer != NULL );
}

} // end of namespace kvs
