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
#ifndef VISMODULEVIEW__FILE_CHECKER_H_INCLUDE
#define VISMODULEVIEW__FILE_CHECKER_H_INCLUDE

#include <vismodule/File>
#include <vismodule/AVSField>
#include <vismodule/AVSUcd>
#include <vismodule/Stl>
#include <vismodule/Ply>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/KVSMLObjectLine>
#include <vismodule/KVSMLObjectPolygon>
#include <vismodule/KVSMLObjectStructuredVolume>
#include <vismodule/KVSMLObjectUnstructuredVolume>
//#include <vismodule/DicomList>
//#include "DicomList.h"


namespace vismoduleview
{

namespace FileChecker
{

///*===========================================================================*/
///**
// *  @brief  Check function for the supported point file format in the command.
// *  @param  filename [in] input filename
// *  @return true if the input file is supported
// */
///*===========================================================================*/
//inline const bool ImportablePoint( const std::string& filename )
//{
//    // KVSML format.
//    if ( vismodule::KVSMLObjectPoint::CheckFileExtension( filename ) )
//    {
//        if ( vismodule::KVSMLObjectPoint::CheckFileFormat( filename ) )
//        {
//            return( true );
//        }
//    }
//
//    return( false );
//}
//
///*===========================================================================*/
///**
// *  @brief  Check function for the supported line file format in the command.
// *  @param  filename [in] input filename
// *  @return true if the input file is supported
// */
///*===========================================================================*/
//inline const bool ImportableLine( const std::string& filename )
//{
//    // KVSML format.
//    if ( vismodule::KVSMLObjectLine::CheckFileExtension( filename ) )
//    {
//        if ( vismodule::KVSMLObjectLine::CheckFileFormat( filename ) )
//        {
//            return( true );
//        }
//    }
//
//    return( false );
//}
//
///*===========================================================================*/
///**
// *  @brief  Check function for the supported polygon file format in the command.
// *  @param  filename [in] input filename
// *  @return true if the input file is supported
// */
///*===========================================================================*/
//inline const bool ImportablePolygon( const std::string& filename )
//{
//    // KVSML format.
//    if ( vismodule::KVSMLObjectPolygon::CheckFileExtension( filename ) )
//    {
//        if ( vismodule::KVSMLObjectPolygon::CheckFileFormat( filename ) )
//        {
//            return( true );
//        }
//    }
//
//    // STL format.
//    if ( vismodule::Stl::CheckFileExtension( filename ) )
//    {
//        if ( vismodule::Stl::CheckFileFormat( filename ) )
//        {
//            return( true );
//        }
//    }
//
//    // Ply format.
//    if ( vismodule::Ply::CheckFileExtension( filename ) )
//    {
//        if ( vismodule::Ply::CheckFileFormat( filename ) )
//        {
//            return( true );
//        }
//    }
//
//    return( false );
//}
//
/*===========================================================================*/
/**
 *  @brief  Check for the supported structured volume object.
 *  @param  filename [in] input filename
 *  @return true if the input file is supported as structured volume object
 */
/*===========================================================================*/
inline const bool ImportableStructuredVolume( const std::string& filename )
{
    vismodule::File file( filename );
    if ( file.isFile() )
    {
        // KVSML format.
        if ( vismodule::KVSMLObjectStructuredVolume::CheckFileExtension( filename ) )
        {
            if ( vismodule::KVSMLObjectStructuredVolume::CheckFileFormat( filename ) )
            {
                return( true );
            }
        }

        // AVS field format.
        if ( vismodule::AVSField::CheckFileExtension( filename ) )
        {
            return( true );
        }
    }
    else
    {
//        // DICOM list.
//        if ( vismodule::DicomList::CheckDirectory( filename ) )
//        {
//            return( true );
//        }
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
    if ( vismodule::KVSMLObjectUnstructuredVolume::CheckFileExtension( filename ) )
    {
        if ( vismodule::KVSMLObjectUnstructuredVolume::CheckFileFormat( filename ) )
        {
            return( true );
        }
    }

    // AVS UCD format.
    if ( vismodule::AVSUcd::CheckFileExtension( filename ) )
    {
        return( true );
    }

    return( false );
}

} // end of namespace FileChecker

} // end of namespace vismoduleview

#endif // VISMODULEVIEW__FILE_CHECKER_H_INCLUDE
