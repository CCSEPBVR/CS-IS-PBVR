/*****************************************************************************/
/**
 *  @file   FileChecker.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FileChecker.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVSVIEW__FILE_CHECKER_H_INCLUDE
#define KVSVIEW__FILE_CHECKER_H_INCLUDE

#include <kvs/File>
#include <kvs/AVSField>
#include <kvs/AVSUcd>
#include <kvs/Stl>
#include <kvs/Ply>
#include <kvs/KVSMLObjectPoint>
#include <kvs/KVSMLObjectLine>
#include <kvs/KVSMLObjectPolygon>
#include <kvs/KVSMLObjectStructuredVolume>
#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/DicomList>


namespace kvsview
{

namespace FileChecker
{

/*===========================================================================*/
/**
 *  @brief  Check function for the supported point file format in the command.
 *  @param  filename [in] input filename
 *  @return true if the input file is supported
 */
/*===========================================================================*/
inline const bool ImportablePoint( const std::string& filename )
{
    // KVSML format.
    if ( kvs::KVSMLObjectPoint::CheckFileExtension( filename ) )
    {
        if ( kvs::KVSMLObjectPoint::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Check function for the supported line file format in the command.
 *  @param  filename [in] input filename
 *  @return true if the input file is supported
 */
/*===========================================================================*/
inline const bool ImportableLine( const std::string& filename )
{
    // KVSML format.
    if ( kvs::KVSMLObjectLine::CheckFileExtension( filename ) )
    {
        if ( kvs::KVSMLObjectLine::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Check function for the supported polygon file format in the command.
 *  @param  filename [in] input filename
 *  @return true if the input file is supported
 */
/*===========================================================================*/
inline const bool ImportablePolygon( const std::string& filename )
{
    // KVSML format.
    if ( kvs::KVSMLObjectPolygon::CheckFileExtension( filename ) )
    {
        if ( kvs::KVSMLObjectPolygon::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    // STL format.
    if ( kvs::Stl::CheckFileExtension( filename ) )
    {
        if ( kvs::Stl::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    // Ply format.
    if ( kvs::Ply::CheckFileExtension( filename ) )
    {
        if ( kvs::Ply::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Check for the supported structured volume object.
 *  @param  filename [in] input filename
 *  @return true if the input file is supported as structured volume object
 */
/*===========================================================================*/
inline const bool ImportableStructuredVolume( const std::string& filename )
{
    kvs::File file( filename );
    if ( file.isFile() )
    {
        // KVSML format.
        if ( kvs::KVSMLObjectStructuredVolume::CheckFileExtension( filename ) )
        {
            if ( kvs::KVSMLObjectStructuredVolume::CheckFileFormat( filename ) )
            {
                return( true );
            }
        }

        // AVS field format.
        if ( kvs::AVSField::CheckFileExtension( filename ) )
        {
            return( true );
        }
    }
    else
    {
        // DICOM list.
        if ( kvs::DicomList::CheckDirectory( filename ) )
        {
            return( true );
        }
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Check for the supported unstructured volume object.
 *  @param  filename [in] input filename
 *  @return true if the input file is supported as unstructured volume object
 */
/*===========================================================================*/
inline const bool ImportableUnstructuredVolume( const std::string& filename )
{
    // KVSML format.
    if ( kvs::KVSMLObjectUnstructuredVolume::CheckFileExtension( filename ) )
    {
        if ( kvs::KVSMLObjectUnstructuredVolume::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    // AVS UCD format.
    if ( kvs::AVSUcd::CheckFileExtension( filename ) )
    {
        return( true );
    }

    return( false );
}

} // end of namespace FileChecker

} // end of namespace kvsview

#endif // KVSVIEW__FILE_CHECKER_H_INCLUDE
