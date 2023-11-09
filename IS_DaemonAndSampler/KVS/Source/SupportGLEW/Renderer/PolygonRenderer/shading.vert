/*****************************************************************************/
/**
 *  @file   shading.vert
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright 2007 Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: shading.vert 476 2010-02-14 15:45:46Z naohisa.sakamoto $
 */
/*****************************************************************************/
varying vec3 position;
varying vec3 normal;

void main( void )
{
    gl_FrontColor = gl_Color;
    gl_Position = ftransform();

    position = vec3( gl_ModelViewMatrix * gl_Vertex );
    normal = gl_NormalMatrix * gl_Normal.xyz;
}
