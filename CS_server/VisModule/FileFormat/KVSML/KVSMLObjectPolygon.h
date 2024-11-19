/****************************************************************************/
/**
 *  @file KVSMLObjectPolygon.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectPolygon.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__KVSML_OBJECT_POLYGON_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_POLYGON_H_INCLUDE

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
 *  @brief  KVSML polygon object format.
 */
/*===========================================================================*/
class KVSMLObjectPolygon : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectPolygon );

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
    WritingDataType              m_writing_type; ///< writing data type
    std::string                  m_polygon_type; ///< polygon type
    std::string                  m_color_type;   ///< polygon color type
    std::string                  m_normal_type;  ///< polygon normal type
    vismodule::ValueArray<vismodule::Real32> m_coords;       ///< coordinate array
    vismodule::ValueArray<vismodule::UInt32> m_connections;  ///< connection array
    vismodule::ValueArray<vismodule::UInt8>  m_colors;       ///< color (r,g,b) array
    vismodule::ValueArray<vismodule::UInt8>  m_opacities;    ///< opacity array
    vismodule::ValueArray<vismodule::Real32> m_normals;      ///< normal array

public:

    KVSMLObjectPolygon( void );

    KVSMLObjectPolygon( const std::string& filename );

    virtual ~KVSMLObjectPolygon( void );

public:

    const vismodule::kvsml::KVSMLTag& KVSMLTag( void ) const;

    const vismodule::kvsml::ObjectTag& objectTag( void ) const;

public:

    const std::string& polygonType( void ) const;

    const std::string& colorType( void ) const;

    const std::string& normalType( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connections( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& colors( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& opacities( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& normals( void ) const;

public:

    void setWritingDataType( const WritingDataType writing_type );

    void setPolygonType( const std::string& polygon_type );

    void setColorType( const std::string& color_type );

    void setNormalType( const std::string& normal_type );

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setConnections( const vismodule::ValueArray<vismodule::UInt32>& connections );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setOpacities( const vismodule::ValueArray<vismodule::UInt8>& opacities );

    void setNormals( const vismodule::ValueArray<vismodule::Real32>& normals );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectPolygon& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML_OBJECT_POLYGON_H_INCLUDE
