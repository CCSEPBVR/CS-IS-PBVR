/*****************************************************************************/
/**
 *  @file   SphereGlyph.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SphereGlyph.h 1797 2014-08-04 01:36:37Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#ifndef KVS__SPHERE_GLYPH_H_INCLUDE
#define KVS__SPHERE_GLYPH_H_INCLUDE

#include "GlyphBase.h"
#include <kvs/PointObject>
#include <kvs/Module>
#include <kvs/OpenGL>
#include <kvs/Deprecated>
#include <kvs/PolygonObject>
#include <kvs/GL>


namespace kvs
{

class ObjectBase;
class Camera;
class Light;
class PointObject;
class VolumeObjectBase;
class TransferFunction;
class RGBColor;

/*===========================================================================*/
/**
 *  @brief  Sphere glyph class.
 */
/*===========================================================================*/
class SphereGlyph : public kvs::GlyphBase, public kvs::PolygonObject
{
    kvsModule( kvs::SphereGlyph, Renderer );
    kvsModuleBaseClass( kvs::GlyphBase );
    kvsModuleSuperClass( kvs::PolygonObject );

private:

    size_t m_nslices; ///< number of subdivisions around the z-axis (lines of longitude)
    size_t m_nstacks; ///< number of subdivisions along the z-axis (lines of latitude)
    const kvs::PointObject* m_point; ///< pointer to the point object (reference)
    const kvs::VolumeObjectBase* m_volume; ///< pointer to the volume object (reference)
    kvs::ValueArray<kvs::Real32> m_position;
    kvs::Real32   m_size = 1.0f;
    kvs::RGBColor m_color = kvs::RGBColor( 255, 255, 255 );
    kvs::UInt8 m_opacity = 255;
    int m_glyph_num = 1;

public:

    SphereGlyph();
    SphereGlyph( const kvs::PointObject* point );
    SphereGlyph( const kvs::VolumeObjectBase* volume );
    SphereGlyph( const kvs::VolumeObjectBase* volume, const kvs::TransferFunction& transfer_function );
    SphereGlyph( const int nglyphs, const kvs::ValueArray<kvs::Real32>& position );

    void setNumberOfSlices( const size_t nslices ) { m_nslices = nslices; }
    void setNumberOfStacks( const size_t nstacks ) { m_nstacks = nstacks; }
    void setPosition(const kvs::ValueArray<kvs::Real32>& position ){m_position = position;}
    void setSphereSize(const kvs::Real32 size ){ m_size = size; }
    void setRGBColor(const kvs::RGBColor color ){ m_color = color ;}
    void setOpacity(const kvs::UInt8 opacity ){ m_opacity = opacity ;}
    void setGlyphNum(const int glyph_num ) { m_glyph_num = glyph_num;}
    kvs::UInt8 opacity();

    void setpolygon();
    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

private:

    void attach_point( const kvs::PointObject* point );
    void attach_volume( const kvs::VolumeObjectBase* volume );
    void draw();
    void draw_element( const kvs::RGBColor& color, const kvs::UInt8 opacity );
    void initialize();

public:
    KVS_DEPRECATED( void setNSlices( const size_t nslices ) ) { this->setNumberOfSlices( nslices ); }
    KVS_DEPRECATED( void setNStacks( const size_t nstacks ) ) { this->setNumberOfStacks( nstacks ); }
};

} // end of namespace kvs

#endif // KVS__SPHERE_GLYPH_H_INCLUDE
