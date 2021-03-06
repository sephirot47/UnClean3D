#ifndef MATH_GLSL
#define MATH_GLSL

const float PI = 3.1415926f;
const float PI2 = PI * 2.0f;

float rand(vec3 co)
{
    return fract(sin(dot(co.xyz, vec3(12.9898, 78.233, 32.123))) * 43758.5453);
}

vec2 rand2(vec2 co)
{
    return vec2(rand(co.xyx), rand(co.yxy));
}

vec3 rand3(vec3 co)
{
    return vec3(rand(co.xyz), rand(co.zxy), rand(co.yzx));
}

float side(vec2 p1, vec2 p2, vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

float Map(float value, float srcMin, float srcMax, float destMin, float destMax)
{
    return ((value - srcMin) / (srcMax - srcMin)) * (destMax - destMin) + destMin;
}

float GetProjectedDistance(vec3 point, vec3 planePoint, vec3 planeNormal)
{
    return dot(point - planePoint, planeNormal);
}
vec3 GetProjectedPoint(vec3 point, vec3 planePoint, vec3 planeNormal)
{
    float dist = dot(point - planePoint, planeNormal);
    return point - (planeNormal * dist);
}

float GetTriangleArea(vec2 p0, vec2 p1, vec2 p2)
{
    return abs(p0.x * (p1.y - p2.y) + p1.x * (p2.y - p0.y) + p2.x * (p0.y - p1.y)) * 0.5f;
}
float GetTriangleArea(vec3 p0, vec3 p1, vec3 p2)
{
    return length(cross((p1 - p0), (p2 - p0))) * 0.5f;
}

vec3 GetBarycentricCoordinates(vec3 point, vec3 triP0, vec3 triP1, vec3 triP2)
{
    vec3 v0 = triP1 - triP0;
    vec3 v1 = triP2 - triP0;
    vec3 triNormal = normalize(cross(v0, v1));
    vec3 projPoint = GetProjectedPoint(point, triP0, triNormal);

    vec3 v2 = projPoint - triP0;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;

    vec3 baryCoords;
    baryCoords.y = (d11 * d20 - d01 * d21) / denom;
    baryCoords.z = (d00 * d21 - d01 * d20) / denom;
    baryCoords.x = 1.0f - baryCoords.y - baryCoords.z;
    return baryCoords;
}

vec3 GetBarycentricCoordinates(vec3 point, vec3[3] trianglePoints)
{
    return GetBarycentricCoordinates(point, trianglePoints[0],
                                     trianglePoints[1], trianglePoints[2]);
}

bool PointInTriangle(vec3 baryCoords)
{
    const float Epsilon = 1e-2f;
    return (baryCoords[0] >= -Epsilon && baryCoords[0] <= 1.0f + Epsilon &&
            baryCoords[1] >= -Epsilon && baryCoords[1] <= 1.0f + Epsilon &&
            baryCoords[2] >= -Epsilon && baryCoords[2] <= 1.0f + Epsilon);
}

bool PointInTriangle(vec3 point, vec3 triP0, vec3 triP1, vec3 triP2)
{
    vec3 baryCoords = GetBarycentricCoordinates(point, triP0, triP1, triP2);
    return PointInTriangle(baryCoords);
}

bool IsDegenerateTriangle(vec3[3] trianglePoints)
{
    vec3 tp0 = trianglePoints[0];
    vec3 tp1 = trianglePoints[1];
    vec3 tp2 = trianglePoints[2];

    float area = GetTriangleArea(tp0, tp1, tp2);
    if (area <= 1e-5)
    {
        return true;
    }

    float minLength = 1e-5;
    if (distance(tp0, tp1) < minLength ||
        distance(tp0, tp2) < minLength ||
        distance(tp2, tp1) < minLength)
    {
        return true;
    }

    return false;
}

float DistToLineWithDir(vec2 linePt, vec2 lineDir, vec2 testPt)
{
    vec2 perpDir = vec2(lineDir.y, -lineDir.x);
    vec2 dirToPt1 = linePt - testPt;
    return abs(dot(normalize(perpDir), dirToPt1));
}
float DistToLine(vec2 pt1, vec2 pt2, vec2 testPt)
{
    vec2 lineDir = pt2 - pt1;
    return DistToLineWithDir(pt1, lineDir, testPt);
}

bool PointInTriangle(vec3 point, vec3[3] trianglePoints)
{
    vec3 baryCoords = GetBarycentricCoordinates(point, trianglePoints[0],
                                                trianglePoints[1],
                                                trianglePoints[2]);
    return PointInTriangle(baryCoords);
}

float IntersectRayPlaneDist(vec3 rayOrig, vec3 rayDir, vec3 planePoint, vec3 planeNormal)
{
    float dotProd = dot(planeNormal, rayDir);
    if (abs(dotProd) > 1e-6)
    {
        return dot(planePoint - rayOrig, planeNormal) / dotProd;
    }
    return 1e9f;
}

void IntersectRayTriangle(in vec3 rayOrig,
                          in vec3 rayDir,
                          in vec3 triPoints[3],
                          out float hitDistance,
                          out vec3 hitBaryCoords)
{
    const float INF = 1e12f;
    const float Epsilon = 1e-8f;

    vec3 v10 = (triPoints[1] - triPoints[0]);
    vec3 v20 = (triPoints[2] - triPoints[0]);

    vec3 h = cross(rayDir, v20);
    float a = dot(v10, h);

    if (a > -Epsilon && a < Epsilon) { hitDistance = INF; return; }

    float f = 1.0 / a;
    vec3 s = (rayOrig - triPoints[0]);
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) { hitDistance = INF; return; }

    vec3 q = cross(s, v10);
    float v = f * dot(rayDir, q);
    if (v < 0.0 || u + v > 1.0) { hitDistance = INF; return; }

    hitBaryCoords = vec3(u, v, 1.0 - u - v);

    hitDistance = (f * dot(v20, q));
    if (hitDistance < Epsilon) { hitDistance = INF; return; }
}

#endif
