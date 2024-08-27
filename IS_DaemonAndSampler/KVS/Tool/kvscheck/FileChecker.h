/****************************************************************************/
/**
 *  @file FileChecker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FileChecker.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSCHECK__FILE_CHECKER_H_INCLUDE
#define KVSCHECK__FILE_CHECKER_H_INCLUDE

#include <string>
#include <iostream>


namespace kvscheck
{

/*==========================================================================*/
/**
 *  File checker class.
 */
/*==========================================================================*/
class FileChecker
{
public:

    enum FormatType
    {
        UnknownFormat = 0, ///< unknown format (not supported format)
        KVSMLFormat,       ///< KVSML format
        AVSFieldFormat,    ///< AVS Field format
        AVSUcdFormat,      ///< AVS UCD format
        STLFormat,         ///< STL format
        BitmapFormat,      ///< Bitmap image format
        PPMFormat,         ///< PPM image format
        PGMFormat,         ///< PGM image format
        PBMFormat,         ///< PBM image format
        TIFFFormat,        ///< TIFF image format
        DICOMFormat        ///< DICOM image format
    };

private:

    std::string m_filename; ///< filename

public:

    FileChecker( const std::string& filename );

public:

    const std::string& filename( void ) const;

    const FormatType fileFormat( void ) const;

    friend std::ostream& operator << ( std::ostream& os, const FileChecker& checker );
};

} // end of namespace kvscheck

#endif // KVSCHECK__FILE_CHECKER_H_INCLUDE
