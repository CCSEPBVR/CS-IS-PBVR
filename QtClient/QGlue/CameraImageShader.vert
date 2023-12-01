R"(
#version 330 core

layout(location=0) in vec3 in_vertex;
layout(location=1) in vec2 in_texCoord;
uniform int isLeft;
out vec2 v_texCoord;
out vec3 v_vertex;

void main()
{
    if (isLeft == 1) {
        v_texCoord = in_texCoord;
        gl_Position = vec4(in_vertex.x, in_vertex.y, in_vertex.z, 1);
    } else {
        v_texCoord = vec2(1.0 - in_texCoord.x, in_texCoord.y);
        gl_Position = vec4(-in_vertex.x, in_vertex.y, in_vertex.z, 1);
    }
    v_vertex = in_vertex;
}
)"