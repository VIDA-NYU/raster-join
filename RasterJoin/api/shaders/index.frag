#version 430

uniform samplerBuffer polys;
uniform isamplerBuffer pindex;
uniform isamplerBuffer headIndex;
uniform isamplerBuffer linkedList;

uniform ivec2 res;
uniform vec2 leftBottom;
uniform vec2 cellSize;

in vec2 pt;

out vec4 fragColor;
uniform layout(binding = 0, r32ui) uimageBuffer bufferAgg;

const float EPSILON = 1e-5f;

bool isInsidePoly(int polyId, float testx, float testy) {
    int st = texelFetch(pindex, polyId).r;
    int en = texelFetch(pindex, polyId + 1).r;
    int nvert = en - st;

    int i, j;
    bool c = false;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        vec2 vi = (texelFetch(polys, st + i).rg);
        vec2 vj = (texelFetch(polys, st + j).rg);

        if (((vi.y > testy) != (vj.y > testy))
                && (testx
                    < (vj.x - vi.x)
                    * (testy - vi.y)
                    / (vj.y - vi.y)
                    + vi.x))
            c = !c;
    }
    return c;
}

// test main
void main() {
    fragColor = vec4(1,0,0,1);
    vec2 fcellid = (pt - leftBottom);
    int xp = int(fcellid.x / cellSize.x);
    int yp = int(fcellid.y / cellSize.y);
    if(!(xp < 0 || xp >= res.x || yp < 0 || yp >= res.y)) {
        int cellIn = xp + res.x * yp;
        int pin = int(texelFetch(headIndex, cellIn).r);

        while(pin != -1) {
            ivec2 node = texelFetch(linkedList, pin).rg;
            int polyId = node.x;
            if(isInsidePoly(polyId,pt.x,pt.y)) {
                imageAtomicAdd(bufferAgg, polyId, 1);
                fragColor = vec4(0,1,0,1);
            }
            pin = node.y;
        }
    }
}
