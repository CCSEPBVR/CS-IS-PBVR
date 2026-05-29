/****************************************************************************/
/**
 *  @file KVSMLObjectImage.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectImage.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__KVSML_OBJECT_IMAGE_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_IMAGE_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include "KVSMLTag.h"
#include "ObjectTag.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KVSML image object format.
 */
/*===========================================================================*/
class KVSMLObjectImage : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectImage );

public:

    enum WritingDataType
    {
        Ascii = 0,     ///< ascii data type
        ExternalAscii, ///< external ascii data type
        ExternalBinary ///< external binary data type
    };

protected:

    vismodule::kvsml::KVSMLTag         m_kvsml_tag;    ///< KVSML tag information
    vismodule::kvsml::ObjectTag        m_object_tag;   ///< Object tag information
    std::size_t                       m_width;        ///< image width
    std::size_t                       m_height;       ///< image height
    std::string                  m_pixel_type;   ///< pixel type
    WritingDataType              m_writing_type; ///< writing data type
    vismodule::ValueArray<vismodule::UInt8>  m_data;         ///< color(r,g,b) array

public:

    KVSMLObjectImage( void );

    KVSMLObjectImage( const std::string& filename );

    virtual ~KVSMLObjectImage( void );

public:

    const vismodule::kvsml::KVSMLTag& KVSMLTag( void ) const;

    const vismodule::kvsml::ObjectTag& objectTag( void ) const;

public:

    const std::size_t width( void ) const;

    const std::size_t height( void ) const;

    const std::string& pixelType( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& data( void ) const;

public:

    void setWidth( const std::size_t width );

    void setHeight( const std::size_t height );

    void setPixelType( const std::string& pixel_type );

    void setWritingDataType( const WritingDataType writing_type );

    void setData( const vismodule::ValueArray<vismodule::UInt8>& data );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectImage& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML_OBJECT_IMAGE_H_INCLUDE
