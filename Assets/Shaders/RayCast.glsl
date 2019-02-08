#include "ArrayOfArrays.glsl"

uniform mat4 SceneModelMatrix;
uniform int NumTriangles;

uniform int NumGridCells;
uniform vec3 GridCellSize;
uniform vec3 GridMinPoint;

ARRAY_OF_ARRAYS(UniformMeshGrid)
ARRAY_OF_ARRAYS(TrianglePositions)

float IntersectRayPlaneDist(vec3 rayOrig, vec3 rayDir, vec3 planePoint, vec3 planeNormal)
{
    float dotProd = dot(planeNormal, rayDir);
    return dot(planePoint - rayOrig, planeNormal) / dotProd;
}

void IntersectRayTriangle(in vec3 rayOrig,
                          in vec3 rayDir,
                          in vec3 triPoints[3],
                          out float hitDistance,
                          out vec3 hitBaryCoords)
{
    const float INF = 1e12;
    const float Epsilon = 1e-8;

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

bool RayCast(in vec3 rayOrigin,
             in vec3 rayDirection,
             in float maxDistance,
             out float hitDistance,
             out int hitTriId,
             out vec3 hitBaryCoords)
{
    vec3 rayDirectionSign = sign(rayDirection);

    vec3 gridWorldPos = (rayOrigin.xyz - GridMinPoint.xyz);
    vec3 gridCoordXYZ = floor(gridWorldPos.xyz / GridCellSize.xyz);

    bool insideGrid = true;
    float currentMaxDistance = maxDistance;
    while (insideGrid && currentMaxDistance > 0)
    {
        int NC = NumGridCells;
        int cellIndex = int(dot(vec3(NC * NC, NC, 1), gridCoordXYZ.zyx));
        int numTriIdsInCell = GetUniformMeshGridArraySize(cellIndex);
        for (int i = 0; i < numTriIdsInCell; ++i)
        {
            int triId = int(GetUniformMeshGridElement(cellIndex, i).x);
            vec3 triPoints[3];
            for (int vi = 0; vi < 3; ++vi)
            {
                triPoints[vi] = GetTrianglePositionsElement(triId, vi).xyz;
            }

            IntersectRayTriangle(rayOrigin, rayDirection, triPoints, hitDistance, hitBaryCoords);
            if (hitDistance < currentMaxDistance)
            {
                hitDistance += (maxDistance - currentMaxDistance);
                hitTriId = triId;
                return true;
            }
        }

        // Find which is the next grid cell we need to go to
        vec3 gridStepXYZ;
        vec3 cellMin = GridMinPoint + (gridCoordXYZ * GridCellSize);
        vec3 cellCenter = cellMin + GridCellSize * 0.5f;
        {
            vec3 planeXPoint = cellCenter + vec3(1,0,0) * 0.5f * GridCellSize.x * rayDirectionSign.x;
            float intDistPlaneX = IntersectRayPlaneDist(rayOrigin, rayDirection, planeXPoint, vec3(1,0,0));

            vec3 planeYPoint = cellCenter + vec3(0,1,0) * 0.5f * GridCellSize.y * rayDirectionSign.y;
            float intDistPlaneY = IntersectRayPlaneDist(rayOrigin, rayDirection, planeYPoint, vec3(0,1,0));

            vec3 planeZPoint = cellCenter + vec3(0,0,1) * 0.5f * GridCellSize.z * rayDirectionSign.z;
            float intDistPlaneZ = IntersectRayPlaneDist(rayOrigin, rayDirection, planeZPoint, vec3(0,0,1));

            float minIntersectionDist;
            if (intDistPlaneX <= intDistPlaneY && intDistPlaneX <= intDistPlaneZ)
            {
                minIntersectionDist = intDistPlaneX;
                gridStepXYZ = vec3(1,0,0) * rayDirectionSign.x;
            }
            else if (intDistPlaneY <= intDistPlaneX && intDistPlaneY <= intDistPlaneZ)
            {
                minIntersectionDist = intDistPlaneY;
                gridStepXYZ = vec3(0,1,0) * rayDirectionSign.y;
            }
            else
            {
                minIntersectionDist = intDistPlaneZ;
                gridStepXYZ = vec3(0,0,1) * rayDirectionSign.z;
            }

            currentMaxDistance -= minIntersectionDist;
            rayOrigin += rayDirection * minIntersectionDist;
        }

        // Advance in grid and update whether we are inside
        gridCoordXYZ += gridStepXYZ;
        insideGrid = (all(greaterThanEqual(gridCoordXYZ, vec3(0))) &&
                      all(lessThan(gridCoordXYZ, vec3(NumGridCells))));
    }

    return false;
}
