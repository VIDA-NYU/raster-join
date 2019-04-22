#version 410
uniform mat4 mvpMatrix;

layout(location=0) in  vec2 vertex;
layout(location=1) in  float id;
layout(location=2) in  float eid;

uniform ivec2 res;

out float col;
out float eno;
out vec2 polyPt;

void main()
{
    gl_Position = mvpMatrix * vec4(vertex,0,1);
    col = id;
    eno = eid;

    polyPt = gl_Position.xy / gl_Position.w;
    polyPt = (polyPt + vec2(1,1)) / 2.0;
    polyPt.x *= res.x;
    polyPt.y *= res.y;
}
