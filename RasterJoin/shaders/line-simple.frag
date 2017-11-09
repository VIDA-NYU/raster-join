#version 410

in vec2 polyPt;

out vec4 fragColor;

void main()
{
    if(abs(polyPt.x - gl_FragCoord.x) >= 0.5 || abs(polyPt.y - gl_FragCoord.y) >= 0.5) {
        fragColor = vec4(0,0,0,0);
    } else {
        fragColor = vec4(0,0,1,1);
    }
}
