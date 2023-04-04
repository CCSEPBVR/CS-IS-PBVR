/****************************************************************************/
/**
 *  @file KVSMLObjectLine.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectLine.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__KVSML__KVSML_OBJECT_LINE_H_INCLUDE
#define KVS__KVSML__KVSML_OBJECT_LINE_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include "KVSMLTag.h"
#include "ObjectTag.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  KVSML line object format.
 */
/*===========================================================================*/
class KVSMLObjectLine : public kvs::FileFormatBase
{
    kvsClassName( kvs::KVSMLObjectLine );

public:

    enum WritingDataType
    {
        Ascii = 0,     ///< ascii data type
        ExternalAscii, ///< external ascii data type
        ExternalBinary ///< external binary data type
    };

protected:

    kvs::kvsml::KVSMLTag         m_kvsml_tag;    ///< KVSML tag information
    kvs::kvsml::ObjectTag        m_object_tag;   ///< Object tag information
    WritingDataType              m_writing_type; ///< writing data type
    std::string                  m_line_type;    ///< line type
    std::string                  m_color_type;   ///< line color type
    kvs::ValueArray<kvs::Real32> m_coords;       ///< coordinate array
    kvs::ValueArray<kvs::UInt32> m_connections;  ///< connection array
    kvs::ValueArray<kvs::UInt8>  m_colors;       ///< color (r,g,b) array
    kvs::ValueArray<kvs::Real32> m_sizes;        ///< size array

public:

    KVSMLObjectLine( void );

    KVSMLObjectLine( const std::string& filename );

    virtual ~KVSMLObjectLine( void );

public:

    const kvs::kvsml::KVSMLTag& KVSMLTag( void ) const;

    const kvs::kvsml::ObjectTag& objectTag( void ) const;

public:

    const std::string& lineType( void ) const;

    const std::string& colorType( void ) const;

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;

public:

    void setWritingDataType( const WritingDataType writing_type );

    void setLineType( const std::string& line_type );

    void setColorType( const std::string& color_type );

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectLine& rhs );
};

} // end of namespace kvs

#endif // KVS__KVSML_OBJECT_LINE_H_INCLUDE
