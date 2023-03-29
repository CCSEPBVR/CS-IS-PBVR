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
#ifndef KVS__ARROW_GLYPH_H_INCLUDE
#define KVS__ARROW_GLYPH_H_INCLUDE

#include "GlyphBase.h"
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Arrow glyph class.
 */
/*===========================================================================*/
class ArrowGlyph : public kvs::GlyphBase
{
    // Class name.
    kvsClassName( kvs::ArrowGlyph );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::GlyphBase );

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
        const kvs::VolumeObjectBase* volume );

    ArrowGlyph(
        const kvs::VolumeObjectBase* volume,
        const kvs::TransferFunction& transfer_function );

    virtual ~ArrowGlyph( void );

/*
public:

    const ObjectType objectType( void ) const
    {
        return( Geometry );
    }
*/
public:

    BaseClass::SuperClass* exec( const kvs::ObjectBase* object );

    void draw( void );

public:

    const ArrowType type( void ) const;

    void setType( const ArrowType type );

protected:

    void draw_lines( void );

    void draw_tubes( void );

    void draw_line_element( const kvs::RGBColor& color, const kvs::UInt8 opacity );

    void draw_tube_element( const kvs::RGBColor& color, const kvs::UInt8 opacity );

protected:

    void initialize( void );
};

} // end of namespace kvs

#endif // KVS__ARROW_GLYPH_H_INCLUDE
