#version 410
uniform mat4 mvpMatrix;

layout(location=0) in  vec2 vertex;
layout(location=1) in  float id;

out float col;

void main()
{
    gl_Position = mvpMatrix * vec4(vertex,0,1);
    col = id;
}
