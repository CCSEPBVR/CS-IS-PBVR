/*****************************************************************************/
/**
 *  @file   zooming.vert
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright 2007 Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: zooming.vert 992 2011-10-15 00:24:45Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
uniform float densityFactor;
uniform int circle_threshold;
uniform vec2 screen_scale;
#if defined( ENABLE_RANDOM_TEXTURE )
uniform sampler2D random_texture;
uniform float random_texture_size_inv;
attribute vec2 identifier;
#endif

const float CIRCLE_SCALE = 0.564189583547756; // 1.0 / sqrt(PI)

varying vec3  position;
varying vec3  normal;
varying vec2  center;
varying float radius;


#if defined( ENABLE_RANDOM_TEXTURE )
/*===========================================================================*/
/**
 *  @brief  Return the footprint size of the particle in pixel.
 *  @param  p [in] particle position
 *  @return footprint size [pixel]
 */
/*===========================================================================*/
float zooming( in vec4 p )
{
    // Depth value.
    float D = p.z;
    if ( D < 1.0 ) D = 1.0; // to avoid front-clip

    // Calculate the footprint size of the particle.
    float s = densityFactor / D; // footprint size of the particle in pixel
    float sf = floor( s );       // round-down value of s
    float sc = ceil( s );        // round-up value of s

    // Calculate a probability 'pc' that the footprint size is 'sc'.
    float fraction = fract( s );
    float pc = fraction * ( 2.0 * sf + fraction ) / ( 2.0 * sf + 1.0 );

    // Random number from the random number texture.
    vec2 random_texture_index = identifier * random_texture_size_inv;
    float R = texture2D( random_texture, random_texture_index ).x;

    if ( circle_threshold <= 0 || s <= float( circle_threshold ) )
    {
        // Draw the particle as square.
        s = ( ( R < pc ) ? sc : sf );
        radius = 0.0;
    }
    else
    {
        // Draw the particle as circle.
        // Convert position to screen coordinates.
        center = screen_scale + ( ( p.xy / p.w ) * screen_scale );
        radius = ( ( R < pc ) ? sc : sf ) * CIRCLE_SCALE;
        s = ceil( s * CIRCLE_SCALE * 2.0 ) + 1.0;
    }

    return( s );
}

#else
/*===========================================================================*/
/**
 *  @brief  Return the footprint size of the particle in pixel.
 *  @param  p [in] particle position
 *  @return footprint size [pixel]
 */
/*===========================================================================*/
float zooming( in vec4 p )
{
    // Depth value.
    float D = p.z;
    if ( D < 1.0 ) D = 1.0; // to avoid front-clip

    // Calculate the footprint size of the particle.
    float s = densityFactor / D; // footprint size of the particle in pixel
    float sf = floor( s );       // round-down value of s
    float sc = ceil( s );        // round-up value of s

    // Calculate a probability 'pc' that the footprint size is 'sc'.
    float fraction = fract( s );
    float pc = fraction * ( 2.0 * sf + fraction ) / ( 2.0 * sf + 1.0 );

    // Generate a random floating point using the vertex position.
    float myF = p.x + p.z * p.y;
    int Ri = int( myF * float( 0x0000ffff ) ); // pick 4 bits using mask
    Ri = Ri & 0x000000f0;
    Ri >>= 4;
    float Rf = float( Ri ) / 16.0;
    float R = Rf + sf;

    if ( circle_threshold <= 0 || s <= float( circle_threshold ) )
    {
        // Draw the particle as square.
        s = ( ( R < pc ) ? sc : sf );
        radius = 0.0;
    }
    else
    {
        // Draw the particle as circle.
        // Convert position to screen coordinates.
        center = screen_scale + ( ( p.xy / p.w ) * screen_scale );
        radius = ( ( R < pc ) ? sc : sf ) * CIRCLE_SCALE;
        s = ceil( s * CIRCLE_SCALE * 2.0 ) + 1.0;
    }

    return( s );
}
#endif

/*===========================================================================*/
/**
 *  @brief  Calculates a size of the particle in pixel.
 */
/*===========================================================================*/
void main( void )
{
    gl_FrontColor = gl_Color;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; // same as 'ftransform()'

    normal = gl_Normal.xyz;
    position = vec3( gl_ModelViewMatrix * gl_Vertex );

#if defined( ENABLE_ZOOMING )
    gl_PointSize = zooming( gl_Position );
#else
    radius = 0.0;
    center = vec2(0.0);
    gl_PointSize = 1.0;
#endif
}
