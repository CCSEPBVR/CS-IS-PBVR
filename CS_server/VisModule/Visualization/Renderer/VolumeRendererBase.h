/****************************************************************************/
/**
 *  @file VolumeRendererBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VolumeRendererBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__VOLUME_RENDERER_BASE_H_INCLUDE
#define VIS_MODULE__VOLUME_RENDERER_BASE_H_INCLUDE

#include <vismodule/DebugNew>
#include <vismodule/RendererBase>
#include <vismodule/TransferFunction>
#include <vismodule/FrameBuffer>
#include <vismodule/ValueArray>
#include <vismodule/Shader>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Volume renderer base class.
 */
/*==========================================================================*/
class VolumeRendererBase : public vismodule::RendererBase
{
    visModuleClassName( vismodule::VolumeRendererBase );

protected:

    size_t                       m_width;          ///< width of rendering image
    size_t                       m_height;         ///< height of rendering image
    vismodule::FrameBuffer             m_depth_buffer;   ///< depth buffer
    vismodule::ValueArray<vismodule::Real32> m_depth_data;     ///< depth data as float type
    vismodule::FrameBuffer             m_color_buffer;   ///< color (RGBA) buffer
    vismodule::ValueArray<vismodule::UInt8>  m_color_data;     ///< color (RGBA) data as uchar type
    bool                         m_enable_shading; ///< shading flag
    vismodule::TransferFunction        m_tfunc;          ///< transfer function
    vismodule::Shader::shader_type*    m_shader;         ///< shading method

public:

    VolumeRendererBase( void );

    virtual ~VolumeRendererBase( void );

public:

    virtual void exec(
        vismodule::ObjectBase* object,
        vismodule::Camera*     camera = NULL,
        vismodule::Light*      light  = NULL ) = 0;

public:

    template <typename ShadingType>
    void setShader( const ShadingType shader );

    void setTransferFunction( const vismodule::TransferFunction& tfunc );

    void enableShading( void );

    void disableShading( void );

public:

    const bool isEnabledShading( void ) const;

    const vismodule::TransferFunction& transferFunction( void ) const;

    vismodule::TransferFunction& transferFunction( void );

public:

    void initialize( void );

    void clear( void );

protected:

    void draw_image( void );

    void draw_depth_buffer( const int* viewport );

    void draw_color_buffer( const int* viewport );
};

template <typename ShadingType>
inline void VolumeRendererBase::setShader( const ShadingType shader )
{
    if ( m_shader )
    {
        delete m_shader;
        m_shader = NULL;
    }

    m_shader = new ShadingType( shader );
    if ( !m_shader )
    {
        visModuleMessageError("Cannot create a specified shader.");
    }
};


} // end of namespace vismodule

#endif // VIS_MODULE__VOLUME_RENDERER_BASE_H_INCLUDE
