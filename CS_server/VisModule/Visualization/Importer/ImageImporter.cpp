/****************************************************************************/
/**
 *  @file ImageImporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageImporter.cpp 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ImageImporter.h"
#include <kvs/DebugNew>
#include <kvs/Message>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <string>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ImageImporter class.
 */
/*===========================================================================*/
ImageImporter::ImageImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ImageImporter class.
 *  @param  filename [in] input filename
 */
/*===========================================================================*/
ImageImporter::ImageImporter( const std::string& filename )
{
    if ( kvs::KVSMLObjectImage::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectImage* file_format = new kvs::KVSMLObjectImage( filename );
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
    else if ( kvs::Bmp::CheckFileExtension( filename ) )
    {
        kvs::Bmp* file_format = new kvs::Bmp( filename );
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
    else if ( kvs::Tiff::CheckFileExtension( filename ) )
    {
        kvs::Tiff* file_format = new kvs::Tiff( filename );
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
    else if ( kvs::Ppm::CheckFileExtension( filename ) )
    {
        kvs::Ppm* file_format = new kvs::Ppm( filename );
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
    else if ( kvs::Pgm::CheckFileExtension( filename ) )
    {
        kvs::Pgm* file_format = new kvs::Pgm( filename );
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
    else if ( kvs::Pbm::CheckFileExtension( filename ) )
    {
        kvs::Pbm* file_format = new kvs::Pbm( filename );
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
    else if ( kvs::Dicom::CheckFileExtension( filename ) )
    {
        kvs::Dicom* file_format = new kvs::Dicom( filename );
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

/*===========================================================================*/
/**
 *  @brief  Constructs a new ImageImporter class.
 *  @param  file_format [in] pointer to the data
 */
/*===========================================================================*/
ImageImporter::ImageImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ImageImporter class.
 */
/*===========================================================================*/
ImageImporter::~ImageImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Imports the image data.
 *  @param  file_format [in] pointer to the image data
 *  @return pointer to the imported image object
 */
/*===========================================================================*/
ImageImporter::SuperClass* ImageImporter::exec( const kvs::FileFormatBase* file_format )
{
    if ( !file_format )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name = file_format->className();
    if ( class_name == "kvs::KVSMLObjectImage" )
    {
        this->import( static_cast<const kvs::KVSMLObjectImage*>( file_format ) );
    }
    else if ( class_name == "kvs::Bmp" )
    {
        this->import( static_cast<const kvs::Bmp*>( file_format ) );
    }
    else if ( class_name == "kvs::Tiff" )
    {
        this->import( static_cast<const kvs::Tiff*>( file_format ) );
    }
    else if ( class_name == "kvs::Ppm" )
    {
        this->import( static_cast<const kvs::Ppm*>( file_format ) );
    }
    else if ( class_name == "kvs::Pgm" )
    {
        this->import( static_cast<const kvs::Pgm*>( file_format ) );
    }
    else if ( class_name == "kvs::Pbm" )
    {
        this->import( static_cast<const kvs::Pbm*>( file_format ) );
    }
    else if ( class_name == "kvs::Dicom" )
    {
        this->import( static_cast<const kvs::Dicom*>( file_format ) );
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is not supported.");
        return( NULL );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Imports the KVSML image format data.
 *  @param  kvsml [in] pointer to the KVSML image format data
 */
/*===========================================================================*/
void ImageImporter::import( const kvs::KVSMLObjectImage* kvsml )
{
    kvs::ImageObject::PixelType pixel_type = kvs::ImageObject::Gray8;
    if ( kvsml->pixelType() == "gray" )
    {
        pixel_type = kvs::ImageObject::Gray8;
    }
    else if ( kvsml->pixelType() == "color" )
    {
        pixel_type = kvs::ImageObject::Color24;
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Unknown pixel type.");
        return;
    }

    SuperClass::m_width  = kvsml->width();
    SuperClass::m_height = kvsml->height();
    SuperClass::m_data   = kvsml->data(); // shallow copy
    SuperClass::m_type   = pixel_type;
}

/*==========================================================================*/
/**
 *  @brief  Imports the BMP image format data.
 *  @param  bmp [in] pointer to BMP image format data
 */
/*==========================================================================*/
void ImageImporter::import( const kvs::Bmp* bmp )
{
    SuperClass::m_width  = bmp->width();
    SuperClass::m_height = bmp->height();
    SuperClass::m_data   = bmp->data(); // shallow copy
    SuperClass::m_type   = static_cast<SuperClass::PixelType>( bmp->bitsPerPixel() );
}

/*==========================================================================*/
/**
 *  @brief  Imports the TIFF image format data.
 *  @param  tiff [in] pointer to TIFF image format data
 */
/*==========================================================================*/
void ImageImporter::import( const kvs::Tiff* tiff )
{
    kvs::ImageObject::PixelType pixel_type = kvs::ImageObject::Gray8;
    if ( tiff->colorMode() == kvs::Tiff::Gray8 )
    {
        pixel_type = kvs::ImageObject::Gray8;
    }
    else if ( tiff->colorMode() == kvs::Tiff::Gray16 )
    {
        pixel_type = kvs::ImageObject::Gray16;
    }
    else if ( tiff->colorMode() == kvs::Tiff::Color24 )
    {
        pixel_type = kvs::ImageObject::Color24;
    }
    else //  tiff->colorMode() == kvs::Tiff::UnknownColorMode
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Unknown TIFF color mode.");
        return;
    }

    const kvs::UInt8* raw_data = static_cast<const kvs::UInt8*>( tiff->rawData().pointer() );
    const size_t      raw_size = tiff->rawData().byteSize();
    kvs::ValueArray<kvs::UInt8> data( raw_data, raw_size ); // deep copy

    SuperClass::m_width  = tiff->width();
    SuperClass::m_height = tiff->height();
    SuperClass::m_data   = data; // shallow copy
    SuperClass::m_type   = pixel_type;
}

/*==========================================================================*/
/**
 *  @brief  Imports the PPM image format data.
 *  @param  ppm [in] pointer to PPM image format data
 */
/*==========================================================================*/
void ImageImporter::import( const kvs::Ppm* ppm )
{
    SuperClass::m_width  = ppm->width();
    SuperClass::m_height = ppm->height();
    SuperClass::m_data   = ppm->data();
    SuperClass::m_type   = kvs::ImageObject::Color24;
}

/*==========================================================================*/
/**
 *  @brief  Imports the PGM image format data.
 *  @param  pgm [in] pointer to PGM image format data
 */
/*==========================================================================*/
void ImageImporter::import( const kvs::Pgm* pgm )
{
    SuperClass::m_width  = pgm->width();
    SuperClass::m_height = pgm->height();
    SuperClass::m_data   = pgm->data();
    SuperClass::m_type   = kvs::ImageObject::Gray8;
}

/*==========================================================================*/
/**
 *  @brief  Imports the PBM image format data.
 *  @param  pbm [in] pointer to PBM image format data
 */
/*==========================================================================*/
void ImageImporter::import( const kvs::Pbm* pbm )
{
    const size_t npixels = pbm->width() * pbm->height();
    kvs::ValueArray<kvs::UInt8> data( npixels );

    for ( size_t i = 0; i < npixels; i++ )
    {
        data[i] = pbm->data().test(i) ? 0 : 255;
    }

    SuperClass::m_width  = pbm->width();
    SuperClass::m_height = pbm->height();
    SuperClass::m_data   = data;
    SuperClass::m_type   = kvs::ImageObject::Gray8;
}

/*==========================================================================*/
/**
 *  @brief  Imports the DICOM image format data.
 *  @param  dicom [in] pointer to DICOM image format data
 */
/*==========================================================================*/
void ImageImporter::import( const kvs::Dicom* dicom )
{
    SuperClass::m_width  = dicom->column();
    SuperClass::m_height = dicom->row();
    SuperClass::m_data   = dicom->pixelData();
    SuperClass::m_type   = kvs::ImageObject::Gray8;
}

} // end of namespace kvs
