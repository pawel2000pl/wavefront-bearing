#include <stdio.h>
#include <stdlib.h>
#include <math.h>  

const double PI = 4*atan(1.f);

double scanDoubleDef(double def)
{
    int _ = scanf("%lf", &def);
    return def;
}

typedef struct VERTEX 
{
    double x;
    double y;
    double z;
} Vertex;

typedef struct FACE
{
    int a;
    int b;
    int c;
} Face;

struct OBJECT
{
    Vertex* verticles;
    Face* faces;
    int vertexCount;
    int faceCount;
};
typedef struct OBJECT Object;

int rotate1(int x, int max)
{
    while (x < 0)
        x += max;
    while (x >= max)
        x -= max;
    return x;
}

typedef struct HEIGHTS 
{
    double innerHi, innerLo, outerHi, outerLo;
} Heights;

Heights makeHeights(double height)
{
    return (Heights){height/2, -height/2, height/2, -height/2};
}

Heights makeHeightsFull(double innerHi, double innerLo, double outerHi, double outerLo)
{
    return (Heights){innerHi, innerLo, outerHi, outerLo};
}

Object renderRing(double r, double R, Heights heights, int counts)
{
    if (R == 0 || (heights.innerHi == heights.innerLo && heights.outerHi == heights.outerLo) || counts == 0)
        return (Object){NULL, NULL, 0, 0};
    if (r < 0) r = 0;
    
    int c4 = 4 * counts;
    Object result = {NULL, NULL, c4, 8 * counts};
    result.verticles = calloc(result.vertexCount, sizeof(Vertex));
    result.faces = calloc(result.faceCount, sizeof(Face));

    int j = 0;
    int k = 0;
    
    for (int i=0; i<counts;i++)
    {   
        double angle = 2.f * PI * i / (double)counts;
        double c = cos(angle);
        double s = sin(angle);
        
        result.verticles[j++] = (Vertex){c*r, s*r, heights.innerHi};
        result.verticles[j++] = (Vertex){c*r, s*r, heights.innerLo};
        result.verticles[j++] = (Vertex){c*R, s*R, heights.outerLo};
        result.verticles[j++] = (Vertex){c*R, s*R, heights.outerHi};
        
        int jn[8];
        for (int n=0;n<8;n++)
            jn[n] = rotate1(j-n-1, c4);
    
        int faceMap[4][4] = {{0, 4, 5, 1}, {1, 5, 6, 2}, {2, 6, 7, 3}, {3, 7, 4, 0}};
        for (int n=0;n<4;n++)
        {
            result.faces[k++] = (Face){jn[faceMap[n][0]], jn[faceMap[n][1]], jn[faceMap[n][3]]};
            result.faces[k++] = (Face){jn[faceMap[n][1]], jn[faceMap[n][2]], jn[faceMap[n][3]]};
        }
    }
    
    return result;
}

void freeObject(Object* obj)
{
    free(obj->verticles);
    free(obj->faces);
    obj->verticles = NULL;
    obj->faces = NULL; 
    obj->vertexCount = 0;
    obj->faceCount = 0;
}

void printObject(Object obj)
{
    for (int i=0;i<obj.vertexCount;i++)
        fprintf(stdout, "v %.17le %.17le %.17le\n", obj.verticles[i].x, obj.verticles[i].y, obj.verticles[i].z);
    for (int i=0;i<obj.faceCount;i++)
        fprintf(stdout, "f %d %d %d\n", obj.faces[i].a+1, obj.faces[i].b+1, obj.faces[i].c+1);
}

void mergeObjects(Object* preallocatedDest, Object* source, int* vertexOffset, int* faceOffset, double offsetX, double offsetY, double offsetZ)
{
    for (int i=0;i<source->faceCount;i++)
        preallocatedDest->faces[i+(*faceOffset)] = (Face){source->faces[i].a + (*vertexOffset), source->faces[i].b + (*vertexOffset), source->faces[i].c + (*vertexOffset)};
    (*faceOffset) += source->faceCount;
    for (int i=0;i<source->vertexCount;i++)
        preallocatedDest->verticles[i+(*vertexOffset)] = (Vertex){source->verticles[i].x + offsetX, source->verticles[i].y + offsetY, source->verticles[i].z + offsetZ};
    (*vertexOffset) += source->vertexCount;
}

int main()
{
    double R, r, d, thickness, coverThickness, epsilon, zepsilon, rollCover, rollLiner, rollRailWidth, rollRailHeight, scale;
    int counts, rollCounts;
    
    fprintf(stdout, "# Bearing wavefront generator\n# (c) Paweł Bielecki 2023\n\n");
    
    fprintf(stdout, "# External bearing ray: ");
    fflush(stdout);
    R = fabs(scanDoubleDef(0.015));
    fprintf(stdout, "%lf\n", R);
    
    fprintf(stdout, "# Internal bearing ray: ");
    fflush(stdout);
    r = fabs(scanDoubleDef(0.005));
    fprintf(stdout, "%lf\n", r);
    
    fprintf(stdout, "# Depth: ");
    fflush(stdout);
    d = fabs(scanDoubleDef(0.012));
    fprintf(stdout, "%lf\n", d);
    
    fprintf(stdout, "# Minimal wall thickness: ");
    fflush(stdout);
    thickness = fabs(scanDoubleDef(1e-3));
    fprintf(stdout, "%lf\n", thickness);
    
    fprintf(stdout, "# Roll cover [0-1]: ");
    fflush(stdout);
    rollCover = scanDoubleDef(0.5);
    rollCover = (rollCover<0)?0:((rollCover>1)?1:rollCover);   
    fprintf(stdout, "%lf\n", rollCover);    
    
    fprintf(stdout, "# Roll cover thickness: ");
    fflush(stdout);
    coverThickness = fabs(scanDoubleDef(5e-4));
    fprintf(stdout, "%lf\n", coverThickness);   
    
    fprintf(stdout, "# Roll liner [0-1]: ");
    fflush(stdout);
    rollLiner = scanDoubleDef(0.5);
    rollLiner = (rollLiner<0)?0:((rollLiner>1)?1:rollLiner);
    fprintf(stdout, "%lf\n", rollLiner);   
    
    fprintf(stdout, "# Roll rail width: ");
    fflush(stdout);
    rollRailWidth = fabs(scanDoubleDef(0.001));
    fprintf(stdout, "%lf\n", rollRailWidth);    

    fprintf(stdout, "# Roll rail height: ");
    fflush(stdout);
    rollRailHeight = fabs(scanDoubleDef(0.001));
    fprintf(stdout, "%lf\n", rollRailHeight);   
    
    fprintf(stdout, "# Epsilon in x-axis and y-axis: ");
    fflush(stdout);
    epsilon = fabs(scanDoubleDef(1e-4)); 
    fprintf(stdout, "%lf\n", epsilon);
    
    fprintf(stdout, "# Epsilon in z-axis: ");
    fflush(stdout);
    zepsilon = fabs(scanDoubleDef(3e-4));
    fprintf(stdout, "%lf\n", zepsilon);
    
    fprintf(stdout, "# Counts [3-65536]: ");
    fflush(stdout);
    counts = abs(scanDoubleDef(256));
    counts = (counts<3)?3:((counts>65536)?65536:counts);
    fprintf(stdout, "%d\n", counts);
    
    fprintf(stdout, "# Roll (vertex per round) counts [3-65536]: ");
    fflush(stdout);
    rollCounts = abs(scanDoubleDef(64));
    rollCounts = (rollCounts<3)?3:((rollCounts>65536)?65536:rollCounts);
    fprintf(stdout, "%d\n", rollCounts);
    
    fprintf(stdout, "# Scale: ");
    fflush(stdout);
    scale = fabs(scanDoubleDef(100));
    fprintf(stdout, "%lf\n", scale);
    
    double rayToRollCenter = (r + R) / 2;
    int nroll = 2;
    double rollRay = R/2;
    while (rollRay>(R-r)/2-thickness)
        rollRay=rayToRollCenter*sin(PI/++nroll);
    rollRay -= epsilon/2;
    fprintf(stdout, "\n# Calculated roll count: %d\n# Calculated roll ray: %le\n\n", nroll, rollRay);
    
    double innerRingOuterRay = rayToRollCenter - rollRay - epsilon;
    Object innerRing = renderRing(r, innerRingOuterRay, makeHeights(d), counts);
    Object innerCover = renderRing(r, innerRingOuterRay + rollRay*rollCover/2, makeHeights(coverThickness), counts);
    
    double outerRingInnerRay = rayToRollCenter + rollRay + epsilon;
    Object outerRing = renderRing(outerRingInnerRay, R, makeHeights(d), counts);
    Object outerCover = renderRing(outerRingInnerRay - rollRay*rollCover/2, R, makeHeights(coverThickness), counts);
    
    double rollHeight = d - ((coverThickness>0)?2*(coverThickness+zepsilon):0);
    double railEpsilon = (rollRailHeight>0)?(sqrt(epsilon*epsilon + zepsilon*zepsilon)):0;
    
    Object rollPartUp = renderRing((1.f-rollLiner) * rollRay, rollRay, makeHeightsFull(rollHeight/2, rollRailWidth/2+zepsilon, rollHeight/2, rollRailWidth/2+zepsilon), rollCounts);    
    Object rollPartDown = renderRing((1.f-rollLiner) * rollRay, rollRay, makeHeightsFull(-rollRailWidth/2-zepsilon, -rollHeight/2, -rollRailWidth/2-zepsilon, -rollHeight/2), rollCounts);
    
    Object rollInnerPart = renderRing(fmin((1.f-rollLiner) * rollRay, rollRay - thickness - rollRailHeight - epsilon), rollRay - ((rollRailHeight>0)?(rollRailHeight + railEpsilon):0), makeHeights(rollRailWidth + 2*zepsilon), rollCounts);    
    
    Object rollRailUp = renderRing(rollRay - rollRailHeight - railEpsilon, rollRay - epsilon, makeHeightsFull(rollRailWidth/2+railEpsilon, 0, rollRailWidth/2+railEpsilon, rollRailWidth/2+railEpsilon), rollCounts);
    Object rollRailDown = renderRing(rollRay - rollRailHeight - railEpsilon, rollRay - epsilon, makeHeightsFull(-rollRailWidth/2-railEpsilon, 0, -rollRailWidth/2-railEpsilon, -rollRailWidth/2-railEpsilon), rollCounts);
    
    Object roll;
    roll.vertexCount = rollPartUp.vertexCount + rollPartDown.vertexCount + rollRailUp.vertexCount + rollRailDown.vertexCount + rollInnerPart.vertexCount;
    roll.faceCount = rollPartUp.faceCount + rollPartDown.faceCount + rollRailUp.faceCount + rollRailDown.faceCount + rollInnerPart.faceCount;
    roll.verticles = calloc(roll.vertexCount, sizeof(Vertex));
    roll.faces = calloc(roll.faceCount, sizeof(Face));
    
    int vertexOffset = 0;
    int faceOffset = 0;
    
    mergeObjects(&roll, &rollPartUp, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&roll, &rollPartDown, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&roll, &rollRailUp, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&roll, &rollRailDown, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&roll, &rollInnerPart, &vertexOffset, &faceOffset, 0, 0, 0);
    
    Object innerRail = renderRing(innerRingOuterRay, innerRingOuterRay + rollRailHeight, makeHeightsFull(rollRailWidth/2, -rollRailWidth/2, 0, 0), counts);
    Object outerRail = renderRing(outerRingInnerRay - rollRailHeight, outerRingInnerRay, makeHeightsFull(0, 0, rollRailWidth/2, -rollRailWidth/2), counts);
    
    Object bearing;
    bearing.vertexCount = innerRing.vertexCount + 2 * innerCover.vertexCount + outerRing.vertexCount + 2 * outerCover.vertexCount + nroll * roll.vertexCount + innerRail.vertexCount + outerRail.vertexCount;
    bearing.faceCount = innerRing.faceCount + 2 * innerCover.faceCount + outerRing.faceCount + 2 * outerCover.faceCount + nroll * roll.faceCount + innerRail.faceCount + outerRail.faceCount;    
    bearing.verticles = calloc(bearing.vertexCount, sizeof(Vertex));
    bearing.faces = calloc(bearing.faceCount, sizeof(Face));
    
    vertexOffset = 0;
    faceOffset = 0;
    
    mergeObjects(&bearing, &innerRing, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&bearing, &innerCover, &vertexOffset, &faceOffset, 0, 0, d/2-coverThickness/2);
    mergeObjects(&bearing, &innerCover, &vertexOffset, &faceOffset, 0, 0, -d/2+coverThickness/2);
    
    mergeObjects(&bearing, &outerRing, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&bearing, &outerCover, &vertexOffset, &faceOffset, 0, 0, d/2-coverThickness/2);
    mergeObjects(&bearing, &outerCover, &vertexOffset, &faceOffset, 0, 0, -d/2+coverThickness/2);
    
    mergeObjects(&bearing, &innerRail, &vertexOffset, &faceOffset, 0, 0, 0);
    mergeObjects(&bearing, &outerRail, &vertexOffset, &faceOffset, 0, 0, 0);
    
    for (int i=0;i<nroll;i++)
        mergeObjects(&bearing, &roll, &vertexOffset, &faceOffset, rayToRollCenter * cos(i*2.f*PI/nroll), rayToRollCenter * sin(i*2.f*PI/nroll), 0);
    
    for (int i=0;i<bearing.vertexCount;i++)
        bearing.verticles[i] = (Vertex){bearing.verticles[i].x * scale, bearing.verticles[i].y * scale, bearing.verticles[i].z * scale};
    
    printObject(bearing);

    freeObject(&rollPartUp);
    freeObject(&rollPartDown);
    freeObject(&rollRailUp);
    freeObject(&rollRailDown);
    freeObject(&rollInnerPart);    
    freeObject(&innerRing);
    freeObject(&innerCover);
    freeObject(&outerRing);
    freeObject(&outerCover);
    freeObject(&roll);
    freeObject(&innerRail);
    freeObject(&outerRail);
    freeObject(&bearing);
    
    return 0;
}
