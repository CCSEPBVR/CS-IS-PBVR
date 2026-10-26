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
#ifndef VIS_MODULE__IMAGE_OBJECT_H_INCLUDE
#define VIS_MODULE__IMAGE_OBJECT_H_INCLUDE

#include <vismodule/ObjectBase>
#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Image object class.
 */
/*===========================================================================*/
class ImageObject : public vismodule::ObjectBase
{
    // Class name.
    visModuleClassName( vismodule::ImageObject );

    // Module information.
    visModuleCategory( Object );
    visModuleBaseClass( vismodule::ObjectBase );

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
    std::size_t                      m_width;  ///< image width
    std::size_t                      m_height; ///< image height
    vismodule::ValueArray<vismodule::UInt8> m_data;   ///< pixel data

public:

    ImageObject( void );

    ImageObject(
        const std::size_t                       width,
        const std::size_t                       height,
        const vismodule::ValueArray<vismodule::UInt8>& data,
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

    const std::size_t width( void ) const;

    const std::size_t height( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& data( void ) const;

    const std::size_t bitsPerPixel( void ) const;

    const std::size_t bytesPerPixel( void ) const;

    const std::size_t nchannels( void ) const;

protected:

    const std::size_t get_npixels( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__IMAGE_OBJECT_H_INCLUDE
