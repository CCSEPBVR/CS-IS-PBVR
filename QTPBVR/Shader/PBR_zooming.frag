/*****************************************************************************/
/**
 *  @file   zooming.frag
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#include "shading.h"

varying vec3  position;
varying vec3  normal;
varying vec2  center;
varying float radius;

//Shading Types.
uniform bool  is_lambert_shading;
uniform bool  is_phong_shading;
uniform bool  is_blinn_phong_shading;

uniform ShadingParameter shading;

/*===========================================================================*/
/**
 *  @brief  Calculates a shaded color of the particle with Lambert shading.
 */
/*===========================================================================*/
void main( void )
{
#if defined( ENABLE_PARTICLE_ZOOMING )
    // Discard pixels outside circle.
    if ( radius > 0.0 )
    {
        if ( distance( gl_FragCoord.xy, center ) > radius ) discard;
    }
#endif

    // Light position.
    vec3 light_position = gl_LightSource[0].position.xyz;

    // Light vector (L) and Normal vector (N)
    vec3 L = normalize( light_position - position );
    vec3 N = normalize( gl_NormalMatrix * normal );

    vec3 shaded_color;
    if ( is_lambert_shading )
    {
        shaded_color = ShadingLambert( shading, gl_Color.xyz, L, N );
    }
    else if ( is_phong_shading )
    {
        vec3 V = normalize ( -position );
        shaded_color = ShadingPhong( shading, gl_Color.xyz, L, N, V );
    }
    else if ( is_blinn_phong_shading )
    {
        vec3 V = normalize ( -position );
        shaded_color = ShadingBlinnPhong( shading, gl_Color.xyz, L, N, V );
    }
    else
    {
        shaded_color = ShadingNone( shading, gl_Color.xyz );
    }

    gl_FragColor = vec4( shaded_color, 1.0 );
}
