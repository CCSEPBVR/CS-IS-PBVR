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
#ifndef VIS_MODULE__KVSML_OBJECT_GLYPH_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_GLYPH_H_INCLUDE

#include <fstream>
#include <iostream>
#include <string>

#include <vismodule/FileFormatBase>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <sstream>
//#include "KVSMLTag.h"
//#include "ObjectTag.h"
#include <vismodule/XMLDocument>
#include <vismodule/XMLDeclaration>
#include <vismodule/XMLElement>
#include <vismodule/XMLComment>
#include <vismodule/ValueArray>
#include <vismodule/File>
#include <vismodule/Type>
#include <vismodule/File>
#include <vismodule/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>



namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KVSML point object format.
 */
/*===========================================================================*/
class KVSMLObjectGlyph : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectGlyph );

public:

    enum WritingDataType
    {
        Ascii = 0,     ///< ascii data type
        ExternalAscii, ///< external ascii data type
        ExternalBinary ///< external binary data type
    };

protected:

    vismodule::ValueArray<vismodule::Real32> m_coords;       ///< coordinate array
    vismodule::ValueArray<vismodule::UInt8>  m_colors;       ///< color(r,g,b) array
    vismodule::ValueArray<vismodule::Real32> m_directions;
    vismodule::ValueArray<vismodule::Real32> m_normals;      ///< normal array
    vismodule::ValueArray<vismodule::Real32> m_sizes;        ///< size array

   float m_color_min;
   float m_color_max;
   float m_size_min;
   float m_size_max;

public:

    KVSMLObjectGlyph( );

    KVSMLObjectGlyph( const std::string& filename );

    KVSMLObjectGlyph( const vismodule::ValueArray<vismodule::Real32> coords, const vismodule::ValueArray<vismodule::UInt8>  colors,
                      const vismodule::ValueArray<vismodule::Real32> directions, const vismodule::ValueArray<vismodule::Real32> sizes   );
    
    
    virtual ~KVSMLObjectGlyph( void );

public:

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& colors( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& normals( void ) const;
    
    const vismodule::ValueArray<vismodule::Real32>& directions( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes( void ) const;

    const float colorMin() const;
    const float colorMax() const;
    const float sizeMin() const;
    const float sizeMax() const;
public:

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setNormals( const vismodule::ValueArray<vismodule::Real32>& normals );

    void setDirections( const vismodule::ValueArray<vismodule::Real32>& deirections );
    
    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void setColorMin(const float color_min);
    void setColorMax(const float color_max);
    void setSizeMin(const float size_min);
    void setSizeMax(const float size_max);
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

} // end of namespace vismodule

#endif // KVS__KVSML_OBJECT_POINT_H_INCLUDE
