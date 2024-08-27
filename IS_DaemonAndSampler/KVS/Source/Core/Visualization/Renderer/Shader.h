/****************************************************************************/
/**
 *  @file Shader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Shader.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__SHADER_H_INCLUDE
#define KVS__SHADER_H_INCLUDE

#include <kvs/Vector3>
#include <kvs/Camera>
#include <kvs/Light>


namespace kvs
{

/*==========================================================================*/
/**
 *  Shader class.
 */
/*==========================================================================*/
class Shader
{
public:

    enum Type
    {
        UnknownShading = 0, ///< unknown shading type
        LambertShading,     ///< Lambertian shading
        PhongShading,       ///< Phong shading
        BlinnPhongShading   ///< Blinn-Phong shading
    };

    struct Base
    {
        kvs::Vector3f camera_position; ///< camera position in the object coordinate
        kvs::Vector3f light_position; ///< light position in the object coordinate
        float Ka; ///< ambient coefficient
        float Kd; ///< diffuse coefficient
        float Ks; ///< specular coefficient
        float S;  ///< shininess

        Base( void );

        virtual ~Base( void );

        virtual void set( const kvs::Camera* camera, const kvs::Light* light ) = 0;

        virtual const Shader::Type type( void ) const = 0;

        virtual const kvs::RGBColor shadedColor(
            const kvs::RGBColor& color,
            const kvs::Vector3f& vertex,
            const kvs::Vector3f& normal ) const = 0;

        virtual const float attenuation( const kvs::Vector3f& vertex, const kvs::Vector3f& gradient ) const = 0;
    };

public:

    typedef Base shader_type;

public:

    struct Lambert : public Base
    {
        Lambert( void );

        Lambert( const Lambert& shader );

        Lambert( const float ka, const float kd );

        void set( const kvs::Camera* camera, const kvs::Light* light );

        const Shader::Type type( void ) const;

        const kvs::RGBColor shadedColor(
            const kvs::RGBColor& color,
            const kvs::Vector3f& vertex,
            const kvs::Vector3f& normal ) const;

        const float attenuation( const kvs::Vector3f& vertex, const kvs::Vector3f& gradient ) const;
    };

    struct Phong : public Base
    {
        Phong( void );

        Phong( const Phong& shader );

        Phong( const float ka, const float kd, const float ks, const float s );

        void set( const kvs::Camera* camera, const kvs::Light* light );

        const Shader::Type type( void ) const;

        const kvs::RGBColor shadedColor(
            const kvs::RGBColor& color,
            const kvs::Vector3f& vertex,
            const kvs::Vector3f& normal ) const;

        const float attenuation( const kvs::Vector3f& vertex, const kvs::Vector3f& gradient ) const;
    };

    struct BlinnPhong : public Base
    {
        BlinnPhong( void );

        BlinnPhong( const BlinnPhong& shader );

        BlinnPhong( const float ka, const float kd, const float ks, const float s );

        void set( const kvs::Camera* camera, const kvs::Light* light );

        const Shader::Type type( void ) const;

        const kvs::RGBColor shadedColor(
            const kvs::RGBColor& color,
            const kvs::Vector3f& vertex,
            const kvs::Vector3f& normal ) const;

        const float attenuation( const kvs::Vector3f& vertex, const kvs::Vector3f& gradient ) const;
    };
};

} // end of namespace kvs

#endif // KVS__SHADER_H_INCLUDE
