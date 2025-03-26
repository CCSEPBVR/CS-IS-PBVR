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
#ifndef VIS_MODULE__GLYPH_BASE_H_INCLUDE
#define VIS_MODULE__GLYPH_BASE_H_INCLUDE

#include <vector>
#include <string>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/GlyphObject>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Glyph base class.
 */
/*===========================================================================*/
class GlyphBase : public vismodule::MapperBase, public vismodule::GlyphObject
{
    // Class name.
    visModuleClassName( vismodule::GlyphBase );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::MapperBase );
    visModuleSuperClass( vismodule::GlyphObject );

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

    virtual vismodule::ObjectBase* exec( const vismodule::ObjectBase& object ) = 0;

    virtual void draw( void ) = 0;

public:

    void setSizeMode( const SizeMode mode );

    void setDirectionMode( const DirectionMode mode );

    void setColorMode( const ColorMode mode );

    void setOpacityMode( const OpacityMode mode );

protected:

    void transform( const vismodule::Vector3f& position, const vismodule::Real32 size );

    void transform( const vismodule::Vector3f& position, const vismodule::Vector3f& direction, const vismodule::Real32 size );

protected:

    void calculate_coords( const vismodule::VolumeObjectBase& volume );

    void calculate_coords( const vismodule::StructuredVolumeObject& volume );

    void calculate_uniform_coords( const vismodule::StructuredVolumeObject& volume );

    void calculate_rectilinear_coords( const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void calculate_sizes( const vismodule::VolumeObjectBase& volume );

    template <typename T>
    void calculate_directions( const vismodule::VolumeObjectBase& volume );

    template <typename T>
    void calculate_colors( const vismodule::VolumeObjectBase& volume );

    template <typename T>
    void calculate_opacities( const vismodule::VolumeObjectBase& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__GLYPH_BASE_H_INCLUDE
