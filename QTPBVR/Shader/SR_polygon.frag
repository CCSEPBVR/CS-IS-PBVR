/*****************************************************************************/
/**
 *  @file   polygon.frag
 *  @author Jun Nishimura, Naohisa Sakamoto
 */
/*****************************************************************************/
#version 120
#include "shading.h"
#include "qualifire.h"
#include "texture.h"


// Input parameters from vertex shader
FragIn vec3 position; // vertex position in camera coordinate
FragIn vec3 normal; // normal vector in camera coodinate
FragIn vec2 index; // index for accessing to the random texture

//Shading Types.
uniform bool  is_lambert_shading;
uniform bool  is_phong_shading;
uniform bool  is_blinn_phong_shading;

// Uniform parameters.
uniform sampler2D random_texture; // random texture to generate random number
uniform float random_texture_size_inv; // reciprocal value of the random texture size
uniform vec2 random_offset; // offset values for accessing to the random texture
uniform ShadingParameter shading; // shading parameters
uniform float edge_factor; // edge enhacement factor


/*===========================================================================*/
/**
 *  @brief  Returns random index.
 *  @param  p [in] pixel coordinate value
 *  @return random index as 2d vector
 */
/*===========================================================================*/
vec2 RandomIndex( in vec2 p )
{
    float x = float( int( index.x ) * 73 );
    float y = float( int( index.y ) * 31 );
    return ( vec2( x, y ) + random_offset + p ) * random_texture_size_inv;
}

/*===========================================================================*/
/**
 *  @brief  Main function of fragment shader.
 */
/*===========================================================================*/
void main()
{
    vec3 color = gl_Color.rgb;
    float alpha = gl_Color.a;
    if ( alpha == 0.0 ) { discard; return; }

    // Edge enhancement
    if ( edge_factor > 0.0 )
    {
        vec3 n = normalize( normal );
        vec3 v = normalize( -position );
        alpha = min( 1.0, alpha / pow( abs( dot( n, v ) ), edge_factor ) );
    }

    // Stochastic color assignment.
    float R = LookupTexture2D( random_texture, RandomIndex( gl_FragCoord.xy ) ).a;
    if ( R > alpha ) { discard; return; }

    // Light position in camera coordinate.
    vec3 light_position = gl_LightSource[0].position.xyz;

    // Light vector (L) and Normal vector (N) in camera coordinate.
    vec3 L = normalize( light_position - position );
    vec3 N = normalize( normal );

    // Shading.
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
