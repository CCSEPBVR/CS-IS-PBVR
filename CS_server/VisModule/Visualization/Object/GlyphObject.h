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
#ifndef KVS__GLYPH_OBJECT_H_INCLUDE
#define KVS__GLYPH_OBJECT_H_INCLUDE

#include <vector>
#include <string>
#include <kvs/ValueArray>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/ObjectBase>
#include <kvs/VolumeObjectBase>
#include <kvs/StructuredVolumeObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/TransferFunction>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Glyph base class.
 */
/*===========================================================================*/
class GlyphObject : public kvs::ObjectBase
{
    // Class name.
    kvsClassName( kvs::GlyphObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleSuperClass( kvs::ObjectBase );

protected:

    kvs::ValueArray<kvs::Real32> m_coords; ///< coordinate value array
    kvs::ValueArray<kvs::Real32> m_sizes; ///< size value array
    kvs::ValueArray<kvs::Real32> m_directions; ///< direction vector array
    kvs::ValueArray<kvs::UInt8> m_colors; ///< color value array
    kvs::ValueArray<kvs::UInt8> m_opacities; ///< opacity value array
    kvs::Vector3f m_scale; ///< scaling vector

public:

    GlyphObject( void );

    virtual ~GlyphObject( void );

public:

    virtual void draw( void ) = 0;

public:

    const ObjectType objectType( void ) const;

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;

    const kvs::ValueArray<kvs::Real32>& directions( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::UInt8>& opacities( void ) const;

    const kvs::Vector3f& scale( void ) const;

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

    void setDirections( const kvs::ValueArray<kvs::Real32>& directions );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setOpacities( const kvs::ValueArray<kvs::UInt8>& opacities );

    void setScale( const kvs::Real32 scale );

    void setScale( const kvs::Vector3f& scale );
};

} // end of namespace kvs

#endif // KVS__GLYPH_OBJECT_H_INCLUDE
