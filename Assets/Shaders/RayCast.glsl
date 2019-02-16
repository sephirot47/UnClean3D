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
            if (rayDirectionSign.x != 0 &&
                intDistPlaneX <= intDistPlaneY &&
                intDistPlaneX <= intDistPlaneZ)
            {
                minIntersectionDist = intDistPlaneX;
                gridStepXYZ = vec3(1,0,0) * rayDirectionSign.x;
            }
            else if (rayDirectionSign.y != 0 &&
                     intDistPlaneY <= intDistPlaneX &&
                     intDistPlaneY <= intDistPlaneZ)
            {
                minIntersectionDist = intDistPlaneY;
                gridStepXYZ = vec3(0,1,0) * rayDirectionSign.y;
            }
            else if (rayDirectionSign.z != 0 &&
                     intDistPlaneZ <= intDistPlaneX &&
                     intDistPlaneZ <= intDistPlaneY)
            {
                minIntersectionDist = intDistPlaneZ;
                gridStepXYZ = vec3(0,0,1) * rayDirectionSign.z;
            }
            else
            {
                return false;
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

#endif
