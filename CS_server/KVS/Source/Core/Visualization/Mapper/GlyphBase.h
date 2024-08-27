/*****************************************************************************/
/**
 *  @file   GlyphBase.h
 *  @brief  
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlyphBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLYPH_BASE_H_INCLUDE
#define KVS__GLYPH_BASE_H_INCLUDE

#include <vector>
#include <string>
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/GlyphObject>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Glyph base class.
 */
/*===========================================================================*/
class GlyphBase : public kvs::MapperBase, public kvs::GlyphObject
{
    // Class name.
    kvsClassName( kvs::GlyphBase );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::MapperBase );
    kvsModuleSuperClass( kvs::GlyphObject );

public:

    enum SizeMode
    {
        SizeByDefault = 0,
        SizeByMagnitude
    };

    enum DirectionMode
    {
        DirectionByDefault = 0,
        DirectionByNormal
    };

    enum ColorMode
    {
        ColorByDefault = 0,
        ColorByMagnitude
    };

    enum OpacityMode
    {
        OpacityByDefault = 0,
        OpacityByMagnitude
    };

protected:

    SizeMode m_size_mode; ///< size mode
    DirectionMode m_direction_mode; ///< direction mode
    ColorMode m_color_mode; ///< color mode
    OpacityMode m_opacity_mode; ///< opacity mode

public:

    GlyphBase( void );

    virtual ~GlyphBase( void );

public:

    virtual kvs::ObjectBase* exec( const kvs::ObjectBase* object ) = 0;

    virtual void draw( void ) = 0;

public:

    void setSizeMode( const SizeMode mode );

    void setDirectionMode( const DirectionMode mode );

    void setColorMode( const ColorMode mode );

    void setOpacityMode( const OpacityMode mode );

protected:

    void transform( const kvs::Vector3f& position, const kvs::Real32 size );

    void transform( const kvs::Vector3f& position, const kvs::Vector3f& direction, const kvs::Real32 size );

protected:

    void calculate_coords( const kvs::VolumeObjectBase* volume );

    void calculate_coords( const kvs::StructuredVolumeObject* volume );

    void calculate_uniform_coords( const kvs::StructuredVolumeObject* volume );

    void calculate_rectilinear_coords( const kvs::StructuredVolumeObject* volume );

    template <typename T>
    void calculate_sizes( const kvs::VolumeObjectBase* volume );

    template <typename T>
    void calculate_directions( const kvs::VolumeObjectBase* volume );

    template <typename T>
    void calculate_colors( const kvs::VolumeObjectBase* volume );

    template <typename T>
    void calculate_opacities( const kvs::VolumeObjectBase* volume );
};

} // end of namespace kvs

#endif // KVS__GLYPH_BASE_H_INCLUDE
