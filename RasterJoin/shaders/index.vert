#version 410
uniform mat4 mvpMatrix;
uniform int aggrType;
uniform int attrCt;
uniform int queryCt;

uniform ivec2 res;
uniform vec2 leftBottom;
uniform vec2 cellSize;

layout (std140) uniform queryBuffer {
    vec4 query[10];
};

layout (std140) uniform attrBuffer {
    vec4 type[6];
};

layout(location=0) in  vec2 vertex;
layout(location=1) in  float attributes[5];

out vec2 pt;

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

bool validFilter(int aid, int filterType, float filterVal) {
    bool ret = false;
    switch(filterType) {
    case EQ:
        ret = aEqual(int(type[aid].x),filterVal,attributes[aid-1]);
        break;

    case LT:
        ret = aLessThan(int(type[aid].x),filterVal,attributes[aid-1]);
        break;

    case LTE:
        ret = aLessThan(int(type[aid].x),filterVal,attributes[aid-1]) || (filterVal == attributes[aid-1]);
        break;

    case GT:
        ret = aGreaterThan(int(type[aid].x),filterVal,attributes[aid-1]);
        break;

    case GTE:
        ret = aGreaterThan(int(type[aid].x),filterVal,attributes[aid-1]) || (filterVal == attributes[aid-1]);
        break;

    default:
        break;
    }
    return ret;
}

void main()
{
    bool include = true;
    for(int i = 0;i < queryCt;i ++) {
        int aid = floatBitsToInt(query[i].x);
        int fil = floatBitsToInt(query[i].y);
        if(!validFilter(aid, fil, query[i].z)) {
            include = false;
            break;
        }
    }

    if(include) {
        gl_Position = mvpMatrix * vec4(vertex,0,1);
        if(gl_Position.x >= -1 && gl_Position.x <= 1 && gl_Position.y >= -1 && gl_Position.y <= 1) {
            int vpos = gl_VertexID % (res.x * res.y);
            float x = float(vpos % res.x);
            float y = float(vpos / res.x);

            gl_Position = vec4(x / res.x,y/res.y,0,1);
        }
        pt = vertex;
    } else {
        gl_Position = vec4(-5,-5,0,1);
        pt = vec2(0,0);
    }
}

