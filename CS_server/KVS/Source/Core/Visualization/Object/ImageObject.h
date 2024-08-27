/*****************************************************************************/
/**
 *  @file   ImageObject.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__IMAGE_OBJECT_H_INCLUDE
#define KVS__IMAGE_OBJECT_H_INCLUDE

#include <kvs/ObjectBase>
#include <kvs/ClassName>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Image object class.
 */
/*===========================================================================*/
class ImageObject : public kvs::ObjectBase
{
    // Class name.
    kvsClassName( kvs::ImageObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::ObjectBase );

public:

    enum PixelType
    {
        Gray8   =  8, ///< 8 bit gray pixel
        Gray16  = 16, ///< 16 bit gray pixel
        Color24 = 24, ///< 24 bit RGB color pixel (8x8x8 bits)
        Color32 = 32  ///< 32 bit RGBA color pixel (8x8x8x8 bits)
    };

protected:

    ImageObject::PixelType      m_type;   ///< pixel type
    size_t                      m_width;  ///< image width
    size_t                      m_height; ///< image height
    kvs::ValueArray<kvs::UInt8> m_data;   ///< pixel data

public:

    ImageObject( void );

    ImageObject(
        const size_t                       width,
        const size_t                       height,
        const kvs::ValueArray<kvs::UInt8>& data,
        const ImageObject::PixelType       type = ImageObject::Color24 );

    ImageObject( const ImageObject& other );

    virtual ~ImageObject( void );

public:

    ImageObject& operator = ( const ImageObject& image );

    friend std::ostream& operator << ( std::ostream& os, const ImageObject& object );

public:

    void shallowCopy( const ImageObject& object );

    void deepCopy( const ImageObject& object );

public:

    const ObjectType objectType( void ) const;

    const ImageObject::PixelType type( void ) const;

    const size_t width( void ) const;

    const size_t height( void ) const;

    const kvs::ValueArray<kvs::UInt8>& data( void ) const;

    const size_t bitsPerPixel( void ) const;

    const size_t bytesPerPixel( void ) const;

    const size_t nchannels( void ) const;

protected:

    const size_t get_npixels( void ) const;
};

} // end of namespace kvs

#endif // KVS__IMAGE_OBJECT_H_INCLUDE
