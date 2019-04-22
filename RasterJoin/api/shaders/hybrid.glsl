#version 430

uniform samplerBuffer polys;
uniform isamplerBuffer pindex;
uniform isamplerBuffer headIndex;
uniform isamplerBuffer linkedList;

uniform ivec2 res;
uniform vec2 leftBottom;
uniform vec2 cellSize;

uniform int noPoints;

uniform int aggrType;
uniform int aggrId;
uniform int attrCt;
uniform int queryCt;

uniform int offset;

uniform sampler2D outlineTex;

uniform ivec2 fboRes;
uniform vec2 rightTop;

layout (std140) uniform queryBuffer {
    vec4 query[10];
};

layout (std140) uniform attrBuffer {
    vec4 type[6];
};

layout(std430, binding = 2) buffer vertexBuffer
{
    vec2 points[];
};

layout(std430, binding = 3) buffer offsetBuffer
{
    uint attrOffsets[];
};

layout(std430, binding = 4) buffer texBuf
{
    int bufferAgg[];
};

layout(std430, binding = 5) buffer pointsTex
{
    ivec4 fbo[];
};

// Declare what size is the group.
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

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

const int EQ = 0, LT = 1, LTE = 2, GT = 3, GTE = 4;
const int Location = 0, Uint = 1, Int = 2, Float = 3;

bool aLessThan(int typ, float fVal, float aVal) {
    bool ret = true;
    switch(typ) {
    case Uint:
        ret = (floatBitsToUint(aVal) < floatBitsToUint(fVal));
        break;

    case Int:
        ret = (floatBitsToInt(aVal) < floatBitsToInt(fVal));
        break;

    case Float:
        ret =  (aVal < fVal);
        break;

    default:
        break;
    }
    return ret;
}

bool aGreaterThan(int typ, float fVal, float aVal) {
    bool ret = true;
    switch(typ) {
    case Uint:
        ret = (floatBitsToUint(aVal) > floatBitsToUint(fVal));
        break;

    case Int:
        ret = (floatBitsToInt(aVal) > floatBitsToInt(fVal));
        break;

    case Float:
        ret = (aVal > fVal);
        break;

    default:
        break;
    }
    return ret;
}

bool aEqual(int typ, float fVal, float aVal) {
    bool ret = true;
    switch(typ) {
    case Uint:
        ret = (floatBitsToUint(aVal) == floatBitsToUint(fVal));
        break;

    case Int:
        ret = (floatBitsToInt(aVal) == floatBitsToInt(fVal));
        break;

    case Float:
        ret = (aVal == fVal);
        break;

    default:
        break;
    }
    return ret;
}

bool validFilter(int type, int filterType, float filterVal, float attVal) {
    bool ret = false;
    switch(filterType) {
    case EQ:
        ret = aEqual(type,filterVal,attVal);
        break;

    case LT:
        ret = aLessThan(type,filterVal,attVal);
        break;

    case LTE:
        ret = aLessThan(type,filterVal,attVal) || (filterVal == attVal);
        break;

    case GT:
        ret = aGreaterThan(type,filterVal,attVal);
        break;

    case GTE:
        ret = aGreaterThan(type,filterVal,attVal) || (filterVal == attVal);
        break;

    default:
        break;
    }
    return ret;
}

bool isBorder(ivec2 pos) {
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

ivec2 rasterize(vec2 pt) {
    vec2 origin = (rightTop + leftBottom) / 2;
    vec2 diff = (rightTop - leftBottom) / 2;
    pt = pt - origin;
    pt.x /= diff.x;
    pt.y /= diff.y;
    if(pt.x < -1 || pt.x >= 1 || pt.y < -1 || pt.y >= 1) {
        return ivec2(-1,-1);
    }
    pt = (pt + 1) / 2;
    int x = int(pt.x * fboRes.x);
    int y = int(pt.y * fboRes.y);
    return ivec2(x,y);
}

float floatVal(int aid, float attVal) {
    float ret = 0;
    int typ = int(type[aid].x);
    switch(typ) {
    case Uint:
        ret = float(floatBitsToUint(attVal));
        break;

    case Int:
        ret = float(floatBitsToInt(attVal));
        break;

    case Float:
        ret = attVal;
        break;

    default:
        break;
    }
    return ret;
}

void main() {
    uint lin = uint(gl_GlobalInvocationID.x);
    if(lin >= noPoints) {
        return;
    }

    vec2 pt = points[lin];

    ivec2 pix = rasterize(pt);

    if(pix.x == -1) {
        return;
    }

    // filter based on constraints
    for(int i = 0;i < queryCt;i ++) {
        int aid = floatBitsToInt(query[i].x);
        int fil = floatBitsToInt(query[i].y);
        uint off = attrOffsets[aid];
        uint offin = (off + lin);
        bool useX = (offin % 2 == 0);
        offin /= 2;
        vec2 attValVec = points[offin];
        float attVal = attValVec.y;
        if(useX) attVal = attValVec.x;
        if(!validFilter(int(type[aid].x), fil, query[i].z, attVal)) {
            return;
        }
    }

    int fboLoc = pix.y * fboRes.x + pix.x;
    atomicAdd(fbo[fboLoc].r,1);


    float pixg = 0;
    float decimal = 0;
    int val = 0;
    if(aggrType != 0) {
        uint off = attrOffsets[aggrId];
        uint offin = (off + lin);
        bool useX = (offin %2 == 0);
        offin /= 2;
        vec2 attValVec = points[offin];
        float attVal = attValVec.y;
        if(useX) attVal = attValVec.x;

        pixg = floatVal(aggrId, attVal);
        val = int(pixg);
        val /= 1000;
        decimal = pixg - 1000.f * val;
        decimal *= 10;

        atomicAdd(fbo[fboLoc].g,val);
        atomicAdd(fbo[fboLoc].b,int(decimal));
    }
    if(isBorder(pix)) {
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
                    atomicAdd(bufferAgg[polyId],1);
                    if(aggrType != 0) {
                        atomicAdd(bufferAgg[polyId + offset],val);
                        atomicAdd(bufferAgg[polyId + 2 * offset],int(decimal));
                    }
                }
                pin = node.y;
            }
        }
    }
}
