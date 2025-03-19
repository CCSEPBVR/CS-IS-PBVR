/*****************************************************************************/
/**
 *  @file   DiamondGlyph.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DiamondGlyph.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__DIAMOND_GLYPH_H_INCLUDE
#define VIS_MODULE__DIAMOND_GLYPH_H_INCLUDE

#include "GlyphBase.h"
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Diamond glyph class.
 */
/*===========================================================================*/
class DiamondGlyph : public vismodule::GlyphBase
{
    // Class name.
    visModuleClassName( vismodule::DiamondGlyph );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::GlyphBase );

public:

    DiamondGlyph( void );

    DiamondGlyph(
        const vismodule::VolumeObjectBase& volume );

    DiamondGlyph(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~DiamondGlyph( void );

/*
public:

    const ObjectType objectType( void ) const
    {
        return( Geometry );
    }
*/
public:

    BaseClass::SuperClass* exec( const vismodule::ObjectBase& object );

    void draw( void );

protected:

    void draw_element( const vismodule::RGBColor& color, const vismodule::UInt8 opacity );

protected:

    void initialize( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__DIAMOND_GLYPH_H_INCLUDE
