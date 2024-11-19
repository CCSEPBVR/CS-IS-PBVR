/*****************************************************************************/
/**
 *  @file   ArrowGlyph.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ArrowGlyph.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__ARROW_GLYPH_H_INCLUDE
#define VIS_MODULE__ARROW_GLYPH_H_INCLUDE

#include "GlyphBase.h"
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Arrow glyph class.
 */
/*===========================================================================*/
class ArrowGlyph : public vismodule::GlyphBase
{
    // Class name.
    visModuleClassName( vismodule::ArrowGlyph );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::GlyphBase );

public:

    enum ArrowType
    {
        LineArrow = 0,
        TubeArrow
    };

protected:

    GLUquadricObj* m_cylinder; ///< glyph primitive
    ArrowType m_type; ///< arrow type

public:

    ArrowGlyph( void );

    ArrowGlyph(
        const vismodule::VolumeObjectBase* volume );

    ArrowGlyph(
        const vismodule::VolumeObjectBase* volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~ArrowGlyph( void );

/*
public:

    const ObjectType objectType( void ) const
    {
        return( Geometry );
    }
*/
public:

    BaseClass::SuperClass* exec( const vismodule::ObjectBase* object );

    void draw( void );

public:

    const ArrowType type( void ) const;

    void setType( const ArrowType type );

protected:

    void draw_lines( void );

    void draw_tubes( void );

    void draw_line_element( const vismodule::RGBColor& color, const vismodule::UInt8 opacity );

    void draw_tube_element( const vismodule::RGBColor& color, const vismodule::UInt8 opacity );

protected:

    void initialize( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__ARROW_GLYPH_H_INCLUDE
