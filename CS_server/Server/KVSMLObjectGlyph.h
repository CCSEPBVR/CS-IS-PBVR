/****************************************************************************/
/**
 *  @file KVSMLObjectGlyph.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectGlyph.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__KVSML_OBJECT_GLYPH_H_INCLUDE
#define KVS__KVSML_OBJECT_GLYPH_H_INCLUDE

#include <fstream>
#include <iostream>
#include <string>

#include <kvs/FileFormatBase>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <sstream>
//#include "KVSMLTag.h"
//#include "ObjectTag.h"
#include <kvs/XMLDocument>
#include <kvs/XMLDeclaration>
#include <kvs/XMLElement>
#include <kvs/XMLComment>
#include <kvs/ValueArray>
#include <kvs/File>
#include <kvs/Type>
#include <kvs/File>
#include <kvs/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>



namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  KVSML point object format.
 */
/*===========================================================================*/
class KVSMLObjectGlyph : public kvs::FileFormatBase
{
    kvsClassName( kvs::KVSMLObjectGlyph );

public:

    enum WritingDataType
    {
        Ascii = 0,     ///< ascii data type
        ExternalAscii, ///< external ascii data type
        ExternalBinary ///< external binary data type
    };

protected:

    kvs::ValueArray<kvs::Real32> m_coords;       ///< coordinate array
    kvs::ValueArray<kvs::UInt8>  m_colors;       ///< color(r,g,b) array
    kvs::ValueArray<kvs::Real32> m_directions;
    kvs::ValueArray<kvs::Real32> m_normals;      ///< normal array
    kvs::ValueArray<kvs::Real32> m_sizes;        ///< size array

public:

    KVSMLObjectGlyph( );

    KVSMLObjectGlyph( const std::string& filename );

    KVSMLObjectGlyph( const kvs::ValueArray<kvs::Real32> coords, const kvs::ValueArray<kvs::UInt8>  colors,
                      const kvs::ValueArray<kvs::Real32> directions, const kvs::ValueArray<kvs::Real32> sizes   );
    
    
    virtual ~KVSMLObjectGlyph( void );

public:

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::Real32>& normals( void ) const;
    
    const kvs::ValueArray<kvs::Real32>& directions( void ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;

public:

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setNormals( const kvs::ValueArray<kvs::Real32>& normals );

    void setDirections( const kvs::ValueArray<kvs::Real32>& deirections );
    
    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

public:

    void clear();
    
    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

    std::vector<std::string> split(const std::string& input, char delimiter)
    {
        std::istringstream stream(input);

        std::string field;
        std::vector<std::string> result;
        while (std::getline(stream, field, delimiter)) {
            result.push_back(field);
        }
        return result;
    }


public:

    KVSMLObjectGlyph& operator = ( const KVSMLObjectGlyph& other );
    
    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectGlyph& rhs );
};

} // end of namespace kvs

#endif // KVS__KVSML_OBJECT_POINT_H_INCLUDE
