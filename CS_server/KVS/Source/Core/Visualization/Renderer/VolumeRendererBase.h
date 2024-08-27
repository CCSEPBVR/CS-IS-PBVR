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
#ifndef KVS__VOLUME_RENDERER_BASE_H_INCLUDE
#define KVS__VOLUME_RENDERER_BASE_H_INCLUDE

#include <kvs/DebugNew>
#include <kvs/RendererBase>
#include <kvs/TransferFunction>
#include <kvs/FrameBuffer>
#include <kvs/ValueArray>
#include <kvs/Shader>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Volume renderer base class.
 */
/*==========================================================================*/
class VolumeRendererBase : public kvs::RendererBase
{
    kvsClassName( kvs::VolumeRendererBase );

protected:

    size_t                       m_width;          ///< width of rendering image
    size_t                       m_height;         ///< height of rendering image
    kvs::FrameBuffer             m_depth_buffer;   ///< depth buffer
    kvs::ValueArray<kvs::Real32> m_depth_data;     ///< depth data as float type
    kvs::FrameBuffer             m_color_buffer;   ///< color (RGBA) buffer
    kvs::ValueArray<kvs::UInt8>  m_color_data;     ///< color (RGBA) data as uchar type
    bool                         m_enable_shading; ///< shading flag
    kvs::TransferFunction        m_tfunc;          ///< transfer function
    kvs::Shader::shader_type*    m_shader;         ///< shading method

public:

    VolumeRendererBase( void );

    virtual ~VolumeRendererBase( void );

public:

    virtual void exec(
        kvs::ObjectBase* object,
        kvs::Camera*     camera = NULL,
        kvs::Light*      light  = NULL ) = 0;

public:

    template <typename ShadingType>
    void setShader( const ShadingType shader );

    void setTransferFunction( const kvs::TransferFunction& tfunc );

    void enableShading( void );

    void disableShading( void );

public:

    const bool isEnabledShading( void ) const;

    const kvs::TransferFunction& transferFunction( void ) const;

    kvs::TransferFunction& transferFunction( void );

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
        kvsMessageError("Cannot create a specified shader.");
    }
};


} // end of namespace kvs

#endif // KVS__VOLUME_RENDERER_BASE_H_INCLUDE
