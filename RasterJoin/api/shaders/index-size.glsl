#version 430

uniform samplerBuffer polys;
uniform isamplerBuffer pindex;

uniform ivec2 res;
uniform vec2 leftBottom;
uniform vec2 cellSize;

uniform int noPolys;

layout(std430, binding = 0) buffer sizeBuf
{
    int size[];
};

layout(std430, binding = 1) buffer headBuf
{
    int head[];
};

// Declare what size is the group.
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

void main() {
    int polyId = int(gl_GlobalInvocationID.x);
    if(polyId < res.x * res.y) {
        head[polyId] = -1;
    }
    if(polyId >= noPolys) {
        return;
    }

    // compute MBR
    int st = texelFetch(pindex,polyId).x;
    int en = texelFetch(pindex,polyId+1).x;
    int nvert = en - st;

    vec2 lb, rt;
    for (int i = 0; i < nvert; i++) {
        vec2 vi = texelFetch(polys,st + i).xy;
        if(i == 0) {
            lb = vi;
            rt = vi;
        } else {
            lb = min(lb,vi);
            rt = max(rt,vi);
        }
    }
    int stx = int((lb.x - leftBottom.x) / cellSize.x);
    int sty = int((lb.y - leftBottom.y) / cellSize.y);
    int enx = int((rt.x - leftBottom.x) / cellSize.x) + 1;
    int eny = int((rt.y - leftBottom.y) / cellSize.y) + 1;

    int xs = res.x;
    int ys = res.y;
    if(enx >= xs) {
        enx = xs - 1;
    }
    if(eny >= ys) {
        eny = ys - 1;
    }
    if(stx < 0) {
        stx = 0;
    }
    if(sty < 0) {
        sty = 0;
    }

    int mem = (enx - stx) * (eny - sty);
    atomicAdd(size[0],mem);
}
