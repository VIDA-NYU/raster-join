#version 410
uniform mat4 mvpMatrix;
uniform ivec2 res;

layout(location=0) in  vec2 vertex;

out vec2 polyPt;

void main()
{
    gl_Position = mvpMatrix * vec4(vertex,0,1);

    polyPt = gl_Position.xy / gl_Position.w;
    polyPt = (polyPt + vec2(1,1)) / 2.0;
    polyPt.x *= res.x;
    polyPt.y *= res.y;
}
