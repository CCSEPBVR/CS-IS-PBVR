/****************************************************************************/
/**
 *  @file ImageRenderer.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImageRenderer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__IMAGE_RENDERER_H_INCLUDE
#define KVS__IMAGE_RENDERER_H_INCLUDE

#include "RendererBase.h"
#include <kvs/ClassName>
#include <kvs/Texture2D>
#include <kvs/ImageObject>
#include <kvs/Module>


namespace kvs
{

class ObjectBase;
class Camera;
class Light;

/*==========================================================================*/
/**
 *  Image renderer class.
 */
/*==========================================================================*/
class ImageRenderer : public kvs::RendererBase
{
    // Class name.
    kvsClassName( kvs::ImageRenderer );

    // Module information.
    kvsModuleCategory( Renderer );
    kvsModuleBaseClass( kvs::RendererBase );

public:

    enum Type
    {
        Stretching = 0,
        Centering  = 1
    };

protected:

    double         m_initial_aspect_ratio; ///< initial aspect ratio
    double         m_left;                 ///< screen left position
    double         m_right;                ///< screen right position
    double         m_bottom;               ///< screen bottom position
    double         m_top;                  ///< screen top position
    Type           m_type;                 ///< rendering type
    kvs::Texture2D m_texture;              ///< texture image

public:

    ImageRenderer( const Type& type = ImageRenderer::Centering );

    virtual ~ImageRenderer( void );

public:

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );

protected:

    void create_texture( const kvs::ImageObject* image );

    void centering( const double width, const double height );
};

} // end of namespace kvs

#endif // KVS__IMAGE_RENDERER_H_INCLUDE
