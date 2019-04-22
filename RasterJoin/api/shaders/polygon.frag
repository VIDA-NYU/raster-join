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
            int val = int(pix.g);
            val /= 1000;
            float decimal = pix.g - 1000.f * val;
            decimal *= 10;
            imageAtomicAdd(bufferAgg, int(col) + offset, val);
            imageAtomicAdd(bufferAgg, int(col) + 2 * offset, int(decimal));
        }
    }
    fragColor = vec4(0,0,1,1);
}
