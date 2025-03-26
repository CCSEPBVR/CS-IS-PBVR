/*****************************************************************************/
/**
 *  @file   GlyphObject.h
 *  @brief  
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlyphObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__GLYPH_OBJECT_H_INCLUDE
#define VIS_MODULE__GLYPH_OBJECT_H_INCLUDE

#include <vector>
#include <string>
#include <vismodule/ValueArray>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>
#include <vismodule/ObjectBase>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/TransferFunction>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Glyph base class.
 */
/*===========================================================================*/
class GlyphObject : public vismodule::ObjectBase
{
    // Class name.
    visModuleClassName( vismodule::GlyphObject );

    // Module information.
    visModuleCategory( Object );
    visModuleSuperClass( vismodule::ObjectBase );

protected:

    vismodule::ValueArray<vismodule::Real32> m_coords; ///< coordinate value array
    vismodule::ValueArray<vismodule::Real32> m_sizes; ///< size value array
    vismodule::ValueArray<vismodule::Real32> m_directions; ///< direction vector array
    vismodule::ValueArray<vismodule::UInt8> m_colors; ///< color value array
    vismodule::ValueArray<vismodule::UInt8> m_opacities; ///< opacity value array
    vismodule::Vector3f m_scale; ///< scaling vector

public:

    GlyphObject( void );

    virtual ~GlyphObject( void );

public:

    virtual void draw( void ) = 0;

public:

    const ObjectType objectType( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& directions( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& colors( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& opacities( void ) const;

    const vismodule::Vector3f& scale( void ) const;

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void setDirections( const vismodule::ValueArray<vismodule::Real32>& directions );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setOpacities( const vismodule::ValueArray<vismodule::UInt8>& opacities );

    void setScale( const vismodule::Real32 scale );

    void setScale( const vismodule::Vector3f& scale );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GLYPH_OBJECT_H_INCLUDE
