/****************************************************************************/
/**
 *  @file ImageImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__IMAGE_IMPORTER_H_INCLUDE
#define VIS_MODULE__IMAGE_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
#include <string>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/ImageObject>
#include <vismodule/KVSMLObjectImage>
#include <vismodule/Bmp>
#include <vismodule/Tiff>
#include <vismodule/Ppm>
#include <vismodule/Pgm>
#include <vismodule/Pbm>
#include <vismodule/Dicom>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Image object importer class.
 */
/*==========================================================================*/
class ImageImporter : public vismodule::ImporterBase, public vismodule::ImageObject
{
    // Class name.
    visModuleClassName( vismodule::ImageImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::ImageObject );

public:

    ImageImporter( void );

    ImageImporter( const std::string& filename );

    ImageImporter( const vismodule::FileFormatBase& file_format );

    virtual ~ImageImporter( void );

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectImage* kvsml );

    void import( const vismodule::Bmp* bmp );

    void import( const vismodule::Tiff* tiff );

    void import( const vismodule::Ppm* ppm );

    void import( const vismodule::Pgm* pgm );

    void import( const vismodule::Pbm* pbm );

    void import( const vismodule::Dicom* dicom );
};

} // end of namespace vismodule

#endif // VIS_MODULE__IMAGE_IMPORTER_H_INCLUDE
