/*****************************************************************************/
/**
 *  @file   SphereGlyph.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SphereGlyph.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__SPHERE_GLYPH_H_INCLUDE
#define VIS_MODULE__SPHERE_GLYPH_H_INCLUDE

#include "GlyphBase.h"
#include <vismodule/PointObject>
#include <vismodule/MapperBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Sphere glyph class.
 */
/*===========================================================================*/
class SphereGlyph : public vismodule::GlyphBase
{
    // Class name.
    visModuleClassName( vismodule::SphereGlyph );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::GlyphBase );

protected:

    GLUquadricObj* m_sphere; ///< glyph primitive
    size_t m_nslices; ///< number of subdivisions around the z-axis (lines of longitude)
    size_t m_nstacks; ///< number of subdivisions along the z-axis (lines of latitude)

public:

    SphereGlyph( void );

    SphereGlyph(
        const vismodule::PointObject* point );

    SphereGlyph(
        const vismodule::VolumeObjectBase& volume );

    SphereGlyph(
        const vismodule::VolumeObjectBase& volume,
        const vismodule::TransferFunction& transfer_function );

    virtual ~SphereGlyph( void );

public:

    void setNSlices( const size_t nslices );

    void setNStacks( const size_t nstacks );

/*
    const ObjectType objectType( void ) const
    {
        return( Geometry );
    }
*/
public:

    BaseClass::SuperClass* exec( const vismodule::ObjectBase& object );

    void draw( void );

protected:

    BaseClass::SuperClass* exec_point_object( const vismodule::PointObject* point );

    BaseClass::SuperClass* exec_volume_object( const vismodule::VolumeObjectBase& volume );

    void draw_element( const vismodule::RGBColor& color, const vismodule::UInt8 opacity );

protected:

    void initialize( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__SPHERE_GLYPH_H_INCLUDE
