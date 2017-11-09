#version 430

uniform sampler2D pointsTex;
uniform int offset;
uniform int aggrType;

in float col;

out vec4 fragColor;
uniform layout(binding = 0, r32i) iimageBuffer bufferAgg;

void main()
{
    vec4 pix = texelFetch(pointsTex, ivec2(gl_FragCoord.xy), 0);
    int ct = int (ceil(pix.r));
    if(ct != 0) {
        imageAtomicAdd(bufferAgg, int(col), ct);
        if(aggrType != 0) {
            int val = int(pix.g * 100);
            imageAtomicAdd(bufferAgg, int(col) + offset, val);
        }
    }
    fragColor = vec4(0,0,1,1);
}
