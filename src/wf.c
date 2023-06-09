#include <stdio.h>
#include <stdlib.h>
#include <math.h>  
#include <string.h>

const double PI = 4*atan(1.f);

double scanDoubleDef(double def)
{
    (void)!scanf("%lf", &def);
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

typedef struct POINT
{
    double x;
    double y;
} Point;

int rotate1(int x, int max)
{
    while (x < 0)
        x += max;
    while (x >= max)
        x -= max;
    return x;
}

Object renderCustomRing(const Point* points, int pointCount, int countPerRound)
{
    int verticlesCount = countPerRound * pointCount;
    Object result = {NULL, NULL, verticlesCount, 2 * verticlesCount};
    result.verticles = calloc(result.vertexCount, sizeof(Vertex));
    result.faces = calloc(result.faceCount, sizeof(Face));

    int vi = 0;
    int fi = 0;

    for (int i=0; i<countPerRound;i++)
    {   
        double angle = 2.f * PI * i / (double)countPerRound;
        double c = cos(angle);
        double s = sin(angle);
        
        for (int n=0;n<pointCount;n++)
            result.verticles[vi++] = (Vertex){c*points[n].x, s*points[n].x, points[n].y};

        for (int n=0;n<pointCount;n++)
        {
            int a = n+pointCount*i;
            int b = rotate1(n+1, pointCount)+pointCount*i;
            int c = rotate1(n+pointCount*(i+1), verticlesCount);
            int d = rotate1(rotate1(n+1, pointCount)+pointCount*(i+1), verticlesCount);            
            result.faces[fi++] = (Face){a, b, c};
            result.faces[fi++] = (Face){b, d, c};
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

void mergeObjects(Object* preallocatedDest, Object* source, int* vertexOffset, int* faceOffset, double offsetX, double offsetY, double offsetZ, double scaleX, double scaleY, double scaleZ)
{
    for (int i=0;i<source->faceCount;i++)
        preallocatedDest->faces[i+(*faceOffset)] = (Face){source->faces[i].a + (*vertexOffset), source->faces[i].b + (*vertexOffset), source->faces[i].c + (*vertexOffset)};
    (*faceOffset) += source->faceCount;
    for (int i=0;i<source->vertexCount;i++)
        preallocatedDest->verticles[i+(*vertexOffset)] = (Vertex){(source->verticles[i].x + offsetX) * scaleX, (source->verticles[i].y + offsetY) * scaleY, (source->verticles[i].z + offsetZ) * scaleZ};
    (*vertexOffset) += source->vertexCount;
}

void printObject(FILE* destination, Object obj)
{
    for (int i=0;i<obj.vertexCount;i++)
        fprintf(destination, "v %.17le %.17le %.17le\n", obj.verticles[i].x, obj.verticles[i].y, obj.verticles[i].z);
    for (int i=0;i<obj.faceCount;i++)
        fprintf(destination, "f %d %d %d\n", obj.faces[i].a+1, obj.faces[i].b+1, obj.faces[i].c+1);
}

void printObjectStl(FILE* destination, Object obj)
{
    fprintf(destination, "solid bearing\n\n");
    for (int i=0;i<obj.faceCount;i++)
    {
        double nx, ny, nz;
        Vertex A = {obj.verticles[obj.faces[i].b].x - obj.verticles[obj.faces[i].a].x, obj.verticles[obj.faces[i].b].y - obj.verticles[obj.faces[i].a].y, obj.verticles[obj.faces[i].b].z - obj.verticles[obj.faces[i].a].z};
        Vertex B = {obj.verticles[obj.faces[i].c].x - obj.verticles[obj.faces[i].a].x, obj.verticles[obj.faces[i].c].y - obj.verticles[obj.faces[i].a].y, obj.verticles[obj.faces[i].c].z - obj.verticles[obj.faces[i].a].z};
        nx = A.y * B.z - A.z * B.y;
        ny = A.z * B.x - A.x * B.z;
        nz = A.x * B.y - A.y * B.x;
        double r = sqrt(nx*nx + ny*ny + nz*nz);        
        if (r == 0) r = 1;
        fprintf(destination, "facet normal %.17le %.17le %.17le\n", nx/r, ny/r, nz/r);
        fprintf(destination, "\touter loop\n");
        fprintf(destination, "\t\tvertex %.17le %.17le %.17le\n", obj.verticles[obj.faces[i].a].x, obj.verticles[obj.faces[i].a].y, obj.verticles[obj.faces[i].a].z);
        fprintf(destination, "\t\tvertex %.17le %.17le %.17le\n", obj.verticles[obj.faces[i].b].x, obj.verticles[obj.faces[i].b].y, obj.verticles[obj.faces[i].b].z);
        fprintf(destination, "\t\tvertex %.17le %.17le %.17le\n", obj.verticles[obj.faces[i].c].x, obj.verticles[obj.faces[i].c].y, obj.verticles[obj.faces[i].c].z);
        fprintf(destination, "\tendloop\n");
        fprintf(destination, "endfacet\n\n");        
    }
    fprintf(destination, "endsolid bearing\n");
}

int hasArg(int argc, const char** argv, const char* arg)
{
    int arglen = strlen(arg);
    for (int i=1;i<argc;i++)
        if (!strncmp(argv[i], arg, arglen))
            return 1;
    return 0;
}

int main(int argc, const char** argv)
{
    double R, r, d, thickness, coverThickness, epsilon, zepsilon, rollCover, rollLiner, rollRailWidth, rollRailHeight, scale;
    int count, rollCount;
    
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
    
    fprintf(stdout, "# Count [3-65536]: ");
    fflush(stdout);
    count = abs(scanDoubleDef(256));
    count = (count<3)?3:((count>65536)?65536:count);
    fprintf(stdout, "%d\n", count);
    
    fprintf(stdout, "# Roll (vertex per round) count [3-65536]: ");
    fflush(stdout);
    rollCount = abs(scanDoubleDef(64));
    rollCount = (rollCount<3)?3:((rollCount>65536)?65536:rollCount);
    fprintf(stdout, "%d\n", rollCount);
    
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
    
    double rollCoverSize = rollCover * (rollRay + 2 * epsilon);
    double innerRingOuterRay = rayToRollCenter - rollRay - epsilon;
    Point ringIntersection1[8], ringIntersection2[8];
    ringIntersection1[0] = (Point){r, 0};
    ringIntersection1[1] = (Point){r, d/2};
    ringIntersection1[2] = (Point){innerRingOuterRay + rollCoverSize, d/2};
    ringIntersection1[3] = (Point){innerRingOuterRay + rollCoverSize, d/2 - coverThickness};
    ringIntersection1[4] = (Point){innerRingOuterRay, d/2 - coverThickness};
    ringIntersection1[5] = (Point){innerRingOuterRay, rollRailWidth/2};
    ringIntersection1[6] = (Point){innerRingOuterRay + rollRailHeight, 0};
    ringIntersection1[7] = (Point){innerRingOuterRay, 0};
    for (int i=0;i<8;i++)
        ringIntersection2[i] = (Point){r - ringIntersection1[i].x + R, ringIntersection1[i].y};

    Object ring1 = renderCustomRing(ringIntersection1, 8, count);
    Object ring2 = renderCustomRing(ringIntersection2, 8, count);

    double rollWallThickness = rollRay * rollLiner;
    double rollRayZero = rollRay - rollWallThickness;
    double rollRayZeroOnRails = fmax(0.f, rollRayZero - rollRailHeight);
    double delta45 = (rollRailHeight>0) ? (rollRayZero - rollRayZeroOnRails) / (rollRailHeight) * (rollRailWidth / 2 + zepsilon) / 2 : 0;
    Point rollIntersection[7];
    rollIntersection[0] = (Point){rollRay - rollRailHeight, 0};
    rollIntersection[1] = (Point){rollRay, rollRailWidth/2 + zepsilon};
    rollIntersection[2] = (Point){rollRay, d/2 - zepsilon - coverThickness};
    rollIntersection[3] = (Point){rollRayZero, d/2 - zepsilon - coverThickness};
    rollIntersection[4] = (Point){rollRayZero, rollRailWidth/2 + rollWallThickness + delta45};
    rollIntersection[5] = (Point){rollRayZeroOnRails, rollRailWidth/2 + rollWallThickness - delta45};
    rollIntersection[6] = (Point){rollRayZeroOnRails, 0};

    Object roll = renderCustomRing(rollIntersection, 7, rollCount);

    Object bearing;
    bearing.vertexCount = 2 * (roll.vertexCount * nroll + ring1.vertexCount + ring2.vertexCount);
    bearing.faceCount = 2 * (roll.faceCount * nroll + ring1.faceCount + ring2.faceCount);
    bearing.verticles = calloc(bearing.vertexCount, sizeof(Vertex));
    bearing.faces = calloc(bearing.faceCount, sizeof(Face));
    
    int vertexOffset = 0;
    int faceOffset = 0;

    for (int zscale=-1;zscale < 2;zscale+=2)
    {
        mergeObjects(&bearing, &ring1, &vertexOffset, &faceOffset, 0, 0, 0, scale, scale, scale * zscale);
        mergeObjects(&bearing, &ring2, &vertexOffset, &faceOffset, 0, 0, 0, scale, scale, scale * zscale);

        for (int i=0;i<nroll;i++)
            mergeObjects(&bearing, &roll, &vertexOffset, &faceOffset, rayToRollCenter * cos(i*2.f*PI/nroll), rayToRollCenter * sin(i*2.f*PI/nroll), 0, scale, scale, scale * zscale);
    }

    if (argc <= 1 || hasArg(argc, argv, "obj"))
        printObject(stdout, bearing);
    if (argc <= 1 || hasArg(argc, argv, "stl"))
        printObjectStl(stderr, bearing);
        
    freeObject(&ring1);
    freeObject(&ring2);
    freeObject(&roll);
    freeObject(&bearing);
    
    return 0;
}
