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
#ifndef VIS_MODULE__IMAGE_RENDERER_H_INCLUDE
#define VIS_MODULE__IMAGE_RENDERER_H_INCLUDE

#include "RendererBase.h"
#include <vismodule/ClassName>
#include <vismodule/Texture2D>
#include <vismodule/ImageObject>
#include <vismodule/Module>


namespace vismodule
{

class ObjectBase;
class Camera;
class Light;

/*==========================================================================*/
/**
 *  Image renderer class.
 */
/*==========================================================================*/
class ImageRenderer : public vismodule::RendererBase
{
    // Class name.
    visModuleClassName( vismodule::ImageRenderer );

    // Module information.
    visModuleCategory( Renderer );
    visModuleBaseClass( vismodule::RendererBase );

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
    vismodule::Texture2D m_texture;              ///< texture image

public:

    ImageRenderer( const Type& type = ImageRenderer::Centering );

    virtual ~ImageRenderer( void );

public:

    void exec( vismodule::ObjectBase* object, vismodule::Camera* camera, vismodule::Light* light );

protected:

    void create_texture( const vismodule::ImageObject* image );

    void centering( const double width, const double height );
};

} // end of namespace vismodule

#endif // VIS_MODULE__IMAGE_RENDERER_H_INCLUDE
