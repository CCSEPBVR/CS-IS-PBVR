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
#ifndef KVS__SPHERE_GLYPH_H_INCLUDE
#define KVS__SPHERE_GLYPH_H_INCLUDE

#include "GlyphBase.h"
#include <kvs/PointObject>
#include <kvs/MapperBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Sphere glyph class.
 */
/*===========================================================================*/
class SphereGlyph : public kvs::GlyphBase
{
    // Class name.
    kvsClassName( kvs::SphereGlyph );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::GlyphBase );

protected:

    GLUquadricObj* m_sphere; ///< glyph primitive
    size_t m_nslices; ///< number of subdivisions around the z-axis (lines of longitude)
    size_t m_nstacks; ///< number of subdivisions along the z-axis (lines of latitude)

public:

    SphereGlyph( void );

    SphereGlyph(
        const kvs::PointObject* point );

    SphereGlyph(
        const kvs::VolumeObjectBase* volume );

    SphereGlyph(
        const kvs::VolumeObjectBase* volume,
        const kvs::TransferFunction& transfer_function );

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

    BaseClass::SuperClass* exec( const kvs::ObjectBase* object );

    void draw( void );

protected:

    BaseClass::SuperClass* exec_point_object( const kvs::PointObject* point );

    BaseClass::SuperClass* exec_volume_object( const kvs::VolumeObjectBase* volume );

    void draw_element( const kvs::RGBColor& color, const kvs::UInt8 opacity );

protected:

    void initialize( void );
};

} // end of namespace kvs

#endif // KVS__SPHERE_GLYPH_H_INCLUDE
