R"(
#version 330 core

uniform mat4 ProjectionMatrix;
uniform float maxZEDDepth;
uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
in vec2 v_texCoord;
in vec3 v_vertex;

/** always 4x4 identity matrix */
const mat4 ModelMatrix = mat4(1.0); 

/**
always:
  eye position (0.0, 0.0, 0.0)
  look at (0.0, 0.0, 1.0)
  up (0.0, -1.0, 0.0)
the view matrix can be calculated with gluLookAt algorithm.
*/
const mat4 ViewMatrix = mat4(
  vec4(1.0, 0.0, 0.0, 0.0), // first column
  vec4(0.0, 1.0, 0.0, 0.0), // second column
  vec4(0.0, 0.0, -1.0, 0.0), // third column
  vec4(0.0, 0.0, 0.0, 1.0));// fourth column


void main( void )
{
    gl_FragColor = vec4(texture(colorTexture, v_texCoord).bgr, 1);

    float raw_depth = texture(depthTexture, v_texCoord).x;
    
    // Adjust scaling factor for sample data.
    raw_depth *= maxZEDDepth;
    
    // Calculate coordinate in clip space.
    vec4 clip_space_coord = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(v_vertex.x, v_vertex.y, raw_depth, 1.0);
    
    // Calculate z-coordinate (=depth) in Normalized Device Coordinate. The value should be in the range of [-1, 1].
    float ndc_space_depth = clip_space_coord.z / clip_space_coord.w;

    //gl_FragDepth = ndc_space_depth;
    
    // Calculate OpenGL depth. Convert NDC depth from [-1, 1] to [gl_DepthRange.near, gl_DepthRange.far].
    //gl_FragDepth = (gl_DepthRange.far - gl_DepthRange.near) * 0.5 * ndc_space_depth + (gl_DepthRange.far + gl_DepthRange.near) * 0.5;
    
    // Convert NDC depth [-1, 1] to [0, 1]
    gl_FragDepth = ndc_space_depth*0.5 + 0.5;
}

)"