/****************************************************************************/
/**
 *  @file Shader.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Shader.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Shader.h"
#include <vismodule/Math>
#include <vismodule/IgnoreUnusedVariable>
#include <vismodule/RGBColor>


#define visModuleShaderAmbientTerm( ka ) \
    ka

#define visModuleShaderDiffuseTerm( kd, N, L ) \
    kd * vismodule::Math::Max( N.dot( L ), 0.0f )

#define visModuleShaderSpecularTerm( ks, S, R, V ) \
    Ks * std::pow( vismodule::Math::Max( R.dot( V ), 0.0f ), S )


namespace
{

inline const vismodule::RGBColor Shade(
    const vismodule::RGBColor& color,
    const float Ia,
    const float Id,
    const float Is )
{
    const float I1 = Ia + Id;
    const float I2 = Is * 255.0f;
    const vismodule::UInt8 r = static_cast<vismodule::UInt8>( vismodule::Math::Min( color.r() * I1 + I2, 255.0f ) + 0.5f );
    const vismodule::UInt8 g = static_cast<vismodule::UInt8>( vismodule::Math::Min( color.g() * I1 + I2, 255.0f ) + 0.5f );
    const vismodule::UInt8 b = static_cast<vismodule::UInt8>( vismodule::Math::Min( color.b() * I1 + I2, 255.0f ) + 0.5f );
    return( vismodule::RGBColor( r, g, b ) );
}

} // end of namespace

namespace vismodule
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Shader::Base::Base( void )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Shader::Base::~Base( void )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Shader::Lambert::Lambert( void )
{
    Ka = 0.4f;
    Kd = 0.6f;
}

/*==========================================================================*/
/**
 *  Copy constructor.
 *  @param shader [in] shader
 */
/*==========================================================================*/
Shader::Lambert::Lambert( const Shader::Lambert& shader )
{
    camera_position = shader.camera_position;
    light_position = shader.light_position;
    Ka = shader.Ka;
    Kd = shader.Kd;
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param ka [in] ambient
 *  @param kd [in] diffuse
 */
/*==========================================================================*/
Shader::Lambert::Lambert( const float ka, const float kd )
{
    Ka = ka;
    Kd = kd;
}

/*==========================================================================*/
/**
 *  Set the camera and light.
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
//void Shader::Lambert::set( const Camera* camera, const Light* light )
//{
//    light_position = camera->projectWorldToObject( light->position() );
//}

const Shader::Type Shader::Lambert::type( void ) const
{
    return( Shader::LambertShading );
}

/*===========================================================================*/
/**
 *  @brief  Returns shaded color.
 *  @param  color [in] source color
 *  @param  vertex [in] vertex position
 *  @param  normal [in] normal vector
 *  @return shaded color
 */
/*===========================================================================*/
const vismodule::RGBColor Shader::Lambert::shadedColor(
    const vismodule::RGBColor& color,
    const vismodule::Vector3f& vertex,
    const vismodule::Vector3f& normal ) const
{
    // Light vector L and normal vector N.
    const vismodule::Vector3f L = ( light_position - vertex ).normalize();
    const vismodule::Vector3f N = normal.normalize();

    // Intensity values.
    const float Ia = visModuleShaderAmbientTerm( Ka );
    const float Id = visModuleShaderDiffuseTerm( Kd, N, L );

    return( color * ( Ia + Id ) );
}

/*==========================================================================*/
/**
 *  Get the attenuation value.
 *  @return attenuation value
 */
/*==========================================================================*/
inline const float Shader::Lambert::attenuation( const vismodule::Vector3f& vertex, const vismodule::Vector3f& gradient ) const
{
    // Light vector L and normal vector N.
    const vismodule::Vector3f L = ( light_position - vertex ).normalize();
    const vismodule::Vector3f N = gradient.normalize();

    const float dd = vismodule::Math::Max( N.dot( L ), 0.0f );

    /* I = Ia + Id
     *
     * Ia = Ka (constant term)
     * Id = Ip *  Kd * cos(A) = Ip * Kd * ( L dot N )
     *
     * Ip : the intensity emitted from the light source.
     */
    const float Ia = Ka;
    const float Id = Kd * dd;

    return( Ia + Id );
}

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Shader::Phong::Phong( void )
{
    Ka = 0.3f;
    Kd = 0.5f;
    Ks = 0.8f;
    S = 100.0f;
}

/*==========================================================================*/
/**
 *  Copy constructor.
 *  @param shader [in] shader
 */
/*==========================================================================*/
Shader::Phong::Phong( const Shader::Phong& shader )
{
    camera_position = shader.camera_position;
    light_position = shader.light_position;
    Ka = shader.Ka;
    Kd = shader.Kd;
    Ks = shader.Ks;
    S = shader.S;
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param ka [in] ambient
 *  @param kd [in] diffuse
 *  @param ks [in] specular
 *  @param s [in] shininess
 */
/*==========================================================================*/
Shader::Phong::Phong( const float ka, const float kd, const float ks, const float s )
{
    Ka = ka;
    Kd = kd;
    Ks = ks;
    S  = s;
}

/*==========================================================================*/
/**
 *  Set the camera and light.
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
//void Shader::Phong::set( const vismodule::Camera* camera, const vismodule::Light* light )
//{
//    camera_position = camera->projectWorldToObject( camera->position() );
//    light_position = camera->projectWorldToObject( light->position() );
//}

/*===========================================================================*/
/**
 *  @brief  Returns the shader type.
 *  @return shader type
 */
/*===========================================================================*/
const Shader::Type Shader::Phong::type( void ) const
{
    return( Shader::PhongShading );
}

/*===========================================================================*/
/**
 *  @brief  Returns shaded color.
 *  @param  color [in] source color
 *  @param  vertex [in] vertex position
 *  @param  normal [in] normal vector
 *  @return shaded color
 */
/*===========================================================================*/
const vismodule::RGBColor Shader::Phong::shadedColor(
    const vismodule::RGBColor& color,
    const vismodule::Vector3f& vertex,
    const vismodule::Vector3f& normal ) const
{
    // Light vector L, normal vector N and reflection vector R.
    const vismodule::Vector3f V = ( camera_position - vertex ).normalize();
    const vismodule::Vector3f L = ( light_position - vertex ).normalize();
    const vismodule::Vector3f N = normal.normalize();
    const vismodule::Vector3f R = 2.0f * N.dot( L ) * N - L;

    // Intensity values.
    const float Ia = visModuleShaderAmbientTerm( Ka );
    const float Id = visModuleShaderDiffuseTerm( Kd, N, L );
    const float Is = visModuleShaderSpecularTerm( ks, S, R, V );

    return( ::Shade( color, Ia, Id, Is ) );
}

/*==========================================================================*/
/**
 *  Get the attenuation value.
 *  @return attenuation value
 */
/*==========================================================================*/
inline const float Shader::Phong::attenuation( const vismodule::Vector3f& vertex, const vismodule::Vector3f& gradient ) const
{
    // Light vector L, normal vector N and reflection vector R.
    const vismodule::Vector3f L = ( light_position - vertex ).normalize();
    const vismodule::Vector3f N = gradient.normalize();
    const vismodule::Vector3f R = 2.0f * N.dot( L ) * N - L;

    const float dd = Math::Max( N.dot( L ), 0.0f );
    const float ds = Math::Max( N.dot( R ), 0.0f );

    /* I = Ia + Id + Is
     *
     * Is = Ip * Ks * cos^s(B) = Ip * Ks * ( R dot N )^s
     */
    const float Ia = Ka;
    const float Id = Kd * dd;
    const float Is = Ks * std::pow( ds, S );

    return( Ia + Id + Is );
}

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
Shader::BlinnPhong::BlinnPhong( void )
{
    Ka = 0.3f;
    Kd = 0.5f;
    Ks = 0.8f;
    S = 100.0f;
}

/*==========================================================================*/
/**
 *  Copy constructor.
 *  @param shader [in] shader
 */
/*==========================================================================*/
Shader::BlinnPhong::BlinnPhong( const Shader::BlinnPhong& shader )
{
    camera_position = shader.camera_position;
    light_position = shader.light_position;
    Ka = shader.Ka;
    Kd = shader.Kd;
    Ks = shader.Ks;
    S = shader.S;
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param ka [in] ambient
 *  @param kd [in] diffuse
 *  @param ks [in] specular
 *  @param s [in] shininess
 */
/*==========================================================================*/
Shader::BlinnPhong::BlinnPhong( const float ka, const float kd, const float ks, const float s )
{
    Ka = ka;
    Kd = kd;
    Ks = ks;
    S  = s;
}

/*==========================================================================*/
/**
 *  Set the camera and light
 *  @param camera [in] pointer to the camera
 *  @param light [in] pointer to the light
 */
/*==========================================================================*/
//void Shader::BlinnPhong::set( const vismodule::Camera* camera, const vismodule::Light* light )
//{
//    camera_position = camera->projectWorldToObject( camera->position() );
//    light_position = camera->projectWorldToObject( light->position() );
//}

/*===========================================================================*/
/**
 *  @brief  Returns the shader type.
 *  @return shader type
 */
/*===========================================================================*/
const Shader::Type Shader::BlinnPhong::type( void ) const
{
    return( Shader::BlinnPhongShading );
}

/*===========================================================================*/
/**
 *  @brief  Returns shaded color.
 *  @param  color [in] source color
 *  @param  vertex [in] vertex position
 *  @param  normal [in] normal vector
 *  @return shaded color
 */
/*===========================================================================*/
const vismodule::RGBColor Shader::BlinnPhong::shadedColor(
    const vismodule::RGBColor& color,
    const vismodule::Vector3f& vertex,
    const vismodule::Vector3f& normal ) const
{
    // Camera vector V, light vector L, halfway vector H and normal vector N.
    const vismodule::Vector3f V = ( camera_position - vertex ).normalize();
    const vismodule::Vector3f L = ( light_position - vertex ).normalize();
    const vismodule::Vector3f H = ( V + L ).normalize();
    const vismodule::Vector3f N = normal.normalize();

    // Intensity values.
    const float Ia = visModuleShaderAmbientTerm( Ka );
    const float Id = visModuleShaderDiffuseTerm( Kd, N, L );
    const float Is = visModuleShaderSpecularTerm( ks, S, H, N );

    return( ::Shade( color, Ia, Id, Is ) );
}

/*==========================================================================*/
/**
 *  Get the attenuation value.
 *  @return attenuation value
 */
/*==========================================================================*/
inline const float Shader::BlinnPhong::attenuation( const vismodule::Vector3f& vertex, const vismodule::Vector3f& gradient ) const
{
    // Camera vector C, light vector L, halfway vector H and normal vector N.
    const vismodule::Vector3f C = ( camera_position - vertex ).normalize();
    const vismodule::Vector3f L = ( light_position - vertex ).normalize();
    const vismodule::Vector3f H = ( C + L ).normalize();
    const vismodule::Vector3f N = gradient.normalize();

    const float dd = vismodule::Math::Max( N.dot( L ), 0.0f );
    const float ds = vismodule::Math::Max( N.dot( H ), 0.0f );

    /* I = Ia + Id + Is
     *
     * Is = Ip * Ks * cos^s(B) = Ip * Ks * ( H dot N )^s
     */
    const float Ia = Ka;
    const float Id = Kd * dd;
    const float Is = Ks * ::pow( ds, S );

    return( Ia + Id + Is );
}

} // end of namespace vismodule
