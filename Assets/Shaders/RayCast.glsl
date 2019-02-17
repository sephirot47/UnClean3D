#ifndef RAYCAST_GLSL
#define RAYCAST_GLSL

#include "Math.glsl"
#include "ArrayOfArrays.glsl"

uniform mat4 SceneModelMatrix;

uniform int NumGridCells;
uniform vec3 GridCellSize;
uniform vec3 GridMinPoint;

ARRAY_OF_ARRAYS(UniformMeshGrid)
ARRAY_OF_ARRAYS(TrianglePositions)

bool RayCast(in vec3 rayOrigin,
             in vec3 rayDirection,
             in float maxDistance,
             out float hitDistance,
             out int hitTriId,
             out vec3 hitBaryCoords)
{
    if (rayDirection == vec3(0) || maxDistance <= 0)
    {
        return false;
    }

    vec3 rayDirectionSign = sign(rayDirection);
    vec3[3] rayDirsXYZ;
    rayDirsXYZ[0] = vec3(1,0,0);
    rayDirsXYZ[1] = vec3(0,1,0);
    rayDirsXYZ[2] = vec3(0,0,1);

    bool insideGrid = true;
    vec3 currentRayOrigin = rayOrigin;
    float currentMaxDistance = maxDistance;
    while (insideGrid && currentMaxDistance > 0)
    {
        vec3 gridCoordXYZ = floor( (currentRayOrigin.xyz - GridMinPoint.xyz) / GridCellSize.xyz );
        insideGrid = (all(greaterThanEqual(gridCoordXYZ, vec3(0))) &&
                      all(lessThan(gridCoordXYZ, vec3(NumGridCells))));
        if (!insideGrid)
        {
            return false;
        }

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

            IntersectRayTriangle(rayOrigin, rayDirection, triPoints,
                                 hitDistance, hitBaryCoords);
            if (hitDistance < maxDistance)
            {
                hitTriId = triId;
                return true;
            }
        }

        // Advance in ray to the next grid cell
        vec3 cellMin = GridMinPoint + (gridCoordXYZ * GridCellSize);
        vec3 cellCenter = cellMin + GridCellSize * 0.5f;
        {
            const float INF = 1e12;
            float minIntersectionDist = INF;
            for (int i = 0; i < 3; ++i)
            {
                if (rayDirectionSign[i] != 0)
                {
                    vec3 rayDirXYZ = rayDirsXYZ[i];
                    vec3 planePoint = cellCenter + rayDirXYZ * GridCellSize[i] * rayDirectionSign[i] * 0.5f;
                    float intDistPlane = IntersectRayPlaneDist(currentRayOrigin, rayDirection,
                                                                planePoint, rayDirXYZ);
                    if (intDistPlane >= 0)
                    {
                        minIntersectionDist = min(minIntersectionDist, intDistPlane);
                    }
                }
            }

            // Advance in grid
            currentMaxDistance -= minIntersectionDist;
            currentRayOrigin += rayDirection * (minIntersectionDist + 0.001f);
        }

    }

    return false;
}

#endif
