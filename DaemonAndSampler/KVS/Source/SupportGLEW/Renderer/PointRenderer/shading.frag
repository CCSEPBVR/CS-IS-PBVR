/*****************************************************************************/
/**
 *  @file   shading.frag
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright 2007 Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: shading.frag 485 2010-03-09 05:44:44Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "../Shader/shading.h"

varying vec3    position;
varying vec3    normal;
uniform Shading shading;


void main( void )
{
    vec3 L = normalize( gl_LightSource[0].position.xyz - position );
    vec3 N = normalize( normal );

#if   defined( ENABLE_LAMBERT_SHADING )
    vec3 shaded_color = ShadingLambert( shading, gl_Color.xyz, L, N );

#elif defined( ENABLE_PHONG_SHADING )
    vec3 V = normalize( -position );
    vec3 shaded_color = ShadingPhong( shading, gl_Color.xyz, L, N, V );

#elif defined( ENABLE_BLINN_PHONG_SHADING )
    vec3 V = normalize( -position );
    vec3 shaded_color = ShadingBlinnPhong( shading, gl_Color.xyz, L, N, V );

#else // DISABLE SHADING
    vec3 shaded_color = ShadingNone( shading, gl_Color.xyz );
#endif

    gl_FragColor.xyz = shaded_color;
    gl_FragColor.w = 1.0;
}
