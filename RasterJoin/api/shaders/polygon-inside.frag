#version 430

uniform isamplerBuffer pointsTex;
uniform sampler2D outlineTex;
uniform int width;

uniform int aggrType;
uniform int offset;

in float col;

out vec4 fragColor;
uniform layout(binding = 0, r32i) iimageBuffer bufferAgg;

bool isBorder() {
    ivec2 pos = ivec2(gl_FragCoord.xy);
    int nsize = 1;
    for(int i = -nsize;i <= nsize; i ++) {
        for(int j = -nsize;j <= nsize; j ++) {
            float b = texelFetch(outlineTex, pos + ivec2(i,j), 0).b;
            if(b != 0) {
                return true;
            }
        }
    }
    return false;
}

void main()
{
    int pos = width * int(gl_FragCoord.y) + int(gl_FragCoord.x);
    if(!isBorder()) {
        // inside
        ivec3 pix = texelFetch(pointsTex, pos).rgb;
        int ct = pix.r;
        if(ct != 0) {
            imageAtomicAdd(bufferAgg, int(col), ct);
            if(aggrType != 0) {
                int val = pix.g;
                int decimal = pix.b;
                imageAtomicAdd(bufferAgg, int(col) + offset, val);
                imageAtomicAdd(bufferAgg, int(col) + 2 * offset, decimal);
            }
        }
        fragColor = vec4(0,0,1,1);
    }
}
