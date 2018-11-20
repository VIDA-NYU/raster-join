#version 430

uniform samplerBuffer polys;
uniform isamplerBuffer pindex;

uniform sampler2D pointsTex;

uniform int cons;
uniform int offset;
uniform ivec2 res;

uniform mat4 mvpMatrix;

in float col;
in float eno;
in vec2 polyPt;

out vec4 fragColor;

uniform layout(binding = 0, r32i) iimageBuffer bufferAgg;

#define INSIDE 0 // 0000
#define LEFT 1   // 0001
#define  RIGHT 2  // 0010
#define  BOTTOM 4 // 0100
#define  TOP 8   // 1000

int ComputeOutCode(float x, float y, vec2 leftBottom,  vec2 rightTop) {
    int code = INSIDE;          // initialised as being inside of clip window
    if (x < leftBottom.x)           // to the left of clip window
        code |= LEFT;
    else if (x > rightTop.x)      // to the right of clip window
        code |= RIGHT;
    if (y < leftBottom.y)           // below the clip window
        code |= BOTTOM;
    else if (y > rightTop.y)      // above the clip window
        code |= TOP;

    return code;
}

float computeArea(vec3 v1, vec3 v2, vec3 v3) {
    vec3 i = v2 - v1;
    vec3 j = v3 - v1;
    float area = abs(0.5 * length(cross(i,j)));
    return area;
}

bool computeFraction(vec2 p1, vec2 p2, vec2 leftBottom,  vec2 rightTop, out float frac) {
    float x0 = p1.x;
    float y0 = p1.y;
    float x1 = p2.x;
    float y1 = p2.y;

    float xmin = leftBottom.x;
    float ymin = leftBottom.y;

    float xmax = rightTop.x;
    float ymax = rightTop.y;

    // from wiki
    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    int outcode0 = ComputeOutCode(x0, y0, leftBottom, rightTop);
    int outcode1 = ComputeOutCode(x1, y1, leftBottom, rightTop);
    bool accept = false;
    frac = 0;

    while (true) {
        if ((outcode0 | outcode1) == 0) { // Bitwise OR is 0. Trivially accept and get out of loop
            accept = true;
            break;
        } else if ((outcode0 & outcode1) != 0) { // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
            break;
        } else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            float x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            int outcodeOut = (outcode0 != 0) ? outcode0 : outcode1;

            // Now find the intersection point;
            // use formulas:
            //   slope = (y1 - y0) / (x1 - x0)
            //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
            //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
            if ((outcodeOut & TOP) != 0) {           // point is above the clip rectangle
                x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                y = ymax;
            } else if ((outcodeOut & BOTTOM) != 0) { // point is below the clip rectangle
                x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                y = ymin;
            } else if ((outcodeOut & RIGHT) != 0) {  // point is to the right of clip rectangle
                y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                x = xmax;
            } else if ((outcodeOut & LEFT) != 0) {   // point is to the left of clip rectangle
                y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                x = xmin;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = ComputeOutCode(x0, y0, leftBottom, rightTop);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(x1, y1, leftBottom, rightTop);
            }
        }
    }
    if (accept) {
        if(x0 > x1) {
            float tmp = x0;
            x0 = x1;
            x1 = tmp;
            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }

        vec3 v1, v2, v3, v4;
        bool tri = false;
        bool compute = true;
        if(x0 == xmin) {
            if(x1 == xmax) {
                // horizontal cut
                v1 = vec3(leftBottom,0);
                v2 = vec3(rightTop.x,leftBottom.y,0);
                v3 = vec3(x1,y1,0);
                v4 = vec3(x0,y0,0);
            } else if(y1 == ymax) {
                v1 = vec3(leftBottom.x,rightTop.y,0);
                v2 = vec3(x0,y0,0);
                v3 = vec3(x1,y1,0);
                tri = true;
            } else if(y1 == ymin) {
                v1 = vec3(leftBottom,0);
                v2 = vec3(x0,y0,0);
                v3 = vec3(x1,y1,0);
                tri = true;
            } else {
                compute = false;
            }
        } else if(x1 == xmax) {
            if(y0 == ymax) {
                v1 = vec3(rightTop,0);
                v2 = vec3(x0,y0,0);
                v3 = vec3(x1,y1,0);
                tri = true;
            } else if(y0 == ymin) {
                v1 = vec3(rightTop.x,leftBottom.y,0);
                v2 = vec3(x1,y1,0);
                v3 = vec3(x0,y0,0);
                tri = true;
            } else {
                compute = false;
            }
        } else if(y0 == ymin && y1 == ymax) {
            // vertical cut
            v1 = vec3(leftBottom,0);
            v2 = vec3(leftBottom.x,rightTop.y,0);
            v3 = vec3(x1,y1,0);
            v4 = vec3(x0,y0,0);
        } else if(y1 == ymin && y0 == ymax) {
            // vertical cut
            v1 = vec3(leftBottom,0);
            v2 = vec3(leftBottom.x,rightTop.y,0);
            v3 = vec3(x0,y0,0);
            v4 = vec3(x1,y1,0);
        } else {
            compute = false;
        }

        frac = 0.5;
        if(compute) {
            float area = 0;
            area = computeArea(v1,v2,v3);
            if(!tri) {
                area += computeArea(v1,v3,v4);
            }
            frac = area / ((ymax - ymin) * (xmax - xmin));
        }
    }
    return accept;
}

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

vec2 getWorldCoords(vec2 fragCoords) {
    vec2 fpt = vec2(fragCoords.x / res.x,fragCoords.y / res.y);
    fpt = fpt * 2 - vec2(1,1);
    vec4 pt = inverse(mvpMatrix) * vec4(fpt,0,1);
    pt /= pt.w;
    return pt.xy;
}

void main()
{
    if(abs(polyPt.x - gl_FragCoord.x) >= 0.5 || abs(polyPt.y - gl_FragCoord.y) >= 0.5) {
        return;
    }
    int val = int (ceil(texelFetch(pointsTex, ivec2(gl_FragCoord.xy), 0).r));
    if(val != 0) {
        int polyId = int(col);

        int st = texelFetch(pindex, polyId).r;
        int en = texelFetch(pindex, polyId + 1).r;
        int nvert = en - st;

        bool c = false;
        float frac = 0;
        vec2 lb = getWorldCoords(floor(gl_FragCoord.xy));
        vec2 rt = getWorldCoords(ceil(gl_FragCoord.xy));

        bool intersects = false;
        int i = int(eno);
        int j = int(eno + 1) % nvert;
        vec2 vi = (texelFetch(polys, st + i).rg);
        vec2 vj = (texelFetch(polys, st + j).rg);
        intersects = computeFraction(vi, vj, lb, rt, frac);

        if(intersects) {
            vec2 pt = getWorldCoords(gl_FragCoord.xy);
            bool inside = isInsidePoly(polyId, pt.x, pt.y);
            if(frac > 0.5) {
                frac = 1 - frac;
            }
            int toAdd = int(ceil(frac * val));
            if(inside) {
                imageAtomicAdd(bufferAgg, int(col) + offset, toAdd);
                imageAtomicAdd(bufferAgg, int(col) + offset * 3, int(val));
            } else {
                imageAtomicAdd(bufferAgg, int(col), toAdd);
                imageAtomicAdd(bufferAgg, int(col) + offset * 2, int(val));
            }
        }
    }
}

//void oldmain()
//{
//    int val = int (ceil(texelFetch(pointsTex, ivec2(gl_FragCoord.xy), 0).r));
//    if(val != 0) {
//        vec2 fpt = vec2(gl_FragCoord.x / res.x,gl_FragCoord.y / res.y);
//        fpt = fpt * 2 - vec2(1,1);
//        vec4 pt = inverse(mvpMatrix) * vec4(fpt,0,1);
//        pt /= pt.w;

//        bool inside = isInsidePoly(int(col), pt.x, pt.y);
//        float frac = getApproxFraction(false, polyPt);

//        int toAdd = int(ceil(frac * val));
//        float frac1 = (frac == 0)?0:1;
//        if(inside) {
//            imageAtomicAdd(bufferAgg, int(col) + offset, toAdd);
//            imageAtomicAdd(bufferAgg, int(col) + offset * 3, int(val * frac1));
//        } else {
//            imageAtomicAdd(bufferAgg, int(col), toAdd);
//            imageAtomicAdd(bufferAgg, int(col) + offset * 2, int(val * frac1));
//        }
//    }
//}
