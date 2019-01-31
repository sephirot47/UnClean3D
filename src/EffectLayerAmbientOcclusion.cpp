#include "EffectLayerAmbientOcclusion.h"

#include "Bang/Dialog.h"
#include "Bang/GameObject.h"
#include "Bang/Geometry.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Random.h"
#include "Bang/Transform.h"
#include "Bang/Triangle2D.h"

#include "View3DScene.h"

using namespace Bang;

EffectLayerAmbientOcclusion::EffectLayerAmbientOcclusion()
{
}

EffectLayerAmbientOcclusion::~EffectLayerAmbientOcclusion()
{
}

using TriId = Mesh::TriangleId;
struct GridCell
{
    Array<TriId> triIds;
    AABox cellBox;
};
using UniformGrid1 = Array<GridCell>;
using UniformGrid2 = Array<UniformGrid1>;
using UniformGrid = Array<UniformGrid2>;

void ProcessTriangleTexels(
    Mesh::TriangleId triId,
    const Vector2 &imgSize,
    Mesh *mesh,
    const UniformGrid &triGrid,
    const Array<Array<Vector3i>> &triangleLocationsInGrid,
    const Matrix4 &localToWorldMatrix,
    Array<Array<bool>> &paintedTexels,
    Image *effectImage)
{
    if (triangleLocationsInGrid[triId].Size() == 0)
    {
        Debug_Log("OH FUCK");
    }
    // Debug_Peek(triangleLocationsInGrid[triId].Size());

    const auto &triVIds = mesh->GetTrianglesVertexIds();
    auto vId0 = triVIds[triId * 3 + 0];
    auto vId1 = triVIds[triId * 3 + 1];
    auto vId2 = triVIds[triId * 3 + 2];

    Vector2 p0Uv = mesh->GetUvsPool()[vId0];
    Vector2 p1Uv = mesh->GetUvsPool()[vId1];
    Vector2 p2Uv = mesh->GetUvsPool()[vId2];
    p0Uv = Vector2::Clamp(p0Uv, Vector2(0), Vector2(1));
    p1Uv = Vector2::Clamp(p1Uv, Vector2(0), Vector2(1));
    p2Uv = Vector2::Clamp(p2Uv, Vector2(0), Vector2(1));

    Vector2 p0Texel = (p0Uv * imgSize);
    Vector2 p1Texel = (p1Uv * imgSize);
    Vector2 p2Texel = (p2Uv * imgSize);
    p0Texel.y = (imgSize.y - p0Texel.y - 1);
    p1Texel.y = (imgSize.y - p1Texel.y - 1);
    p2Texel.y = (imgSize.y - p2Texel.y - 1);

    AARect triTexelsRect;
    triTexelsRect.AddPoint(p0Texel);
    triTexelsRect.AddPoint(p1Texel);
    triTexelsRect.AddPoint(p2Texel);

    Triangle tri3D = mesh->GetTriangle(triId);
    tri3D = localToWorldMatrix * tri3D;

    Triangle2D texelsTri(p0Texel, p1Texel, p2Texel);

    Vector2 txRMin = triTexelsRect.GetMin();
    Vector2 txRMax = triTexelsRect.GetMax();
    txRMin = Vector2::Clamp(txRMin, Vector2(0), imgSize - 1.0f);
    txRMax = Vector2::Clamp(txRMax, Vector2(0), imgSize - 1.0f);

    for (int txy = txRMin.y; txy <= txRMax.y; ++txy)
    {
        for (int txx = txRMin.x; txx <= txRMax.x; ++txx)
        {
            Vector2 texelPoint(txx, txy);

            if (paintedTexels[txy][txx] || !texelsTri.Contains(texelPoint))
            {
                continue;
            }

            Vector3 texelBaryCoords =
                texelsTri.GetBarycentricCoordinates(texelPoint);
            Vector3 point3D = tri3D.GetPoint(texelBaryCoords);
            Vector3 triNormal = tri3D.GetNormal();

            Color pixelColor;
            const auto &locationsInGrid = triangleLocationsInGrid[triId];
            for (const Vector3i &xyzInGrid : locationsInGrid)
            {
                const uint x = xyzInGrid.x;
                const uint y = xyzInGrid.y;
                const uint z = xyzInGrid.z;
                const AABox &cellBox = triGrid[x][y][z].cellBox;
                if (!cellBox.Contains(point3D))
                {
                    continue;
                }

                const Vector3 origin = point3D + triNormal * 0.001f;
                const auto &closeTriangleIds = triGrid[x][y][z].triIds;

                uint intersectedRays = 0;
                uint totalRays = 10;
                for (uint i = 0; i < totalRays; ++i)
                {
                    Vector3 rayDir = Random::GetInsideUnitSphere();
                    if (Vector3::Dot(rayDir, triNormal) < 0.0f)
                    {
                        rayDir *= -1.0f;  // Normal hemisphere;
                    }

                    const float Radius = 0.05f;
                    Ray ray(origin, rayDir * Radius);
                    for (TriId closeTriId : closeTriangleIds)
                    {
                        Triangle closeTri = mesh->GetTriangle(closeTriId);
                        closeTri = localToWorldMatrix * closeTri;

                        float dist;
                        bool rayIntersected;
                        Geometry::IntersectRayTriangle(
                            ray, closeTri, &rayIntersected, &dist);
                        if (rayIntersected && dist < Radius)
                        {
                            ++intersectedRays;
                            break;
                        }
                    }
                }

                float ambientOcclusion = (float(intersectedRays) / totalRays);
                ambientOcclusion = (1.0f - ambientOcclusion);
                const float &ao = ambientOcclusion;
                pixelColor = Color(ao, ao, ao, 1.0f);
                break;
            }

            effectImage->SetPixel(txx, txy, pixelColor);
            paintedTexels[txy][txx] = true;
        }
    }
}

void EffectLayerAmbientOcclusion::GenerateEffectImage(Image *effectImage)
{
    GameObject *modelGameObject =
        MainScene::GetInstance()->GetView3DScene()->GetModelGameObject();
    if (!modelGameObject)
    {
        return;
    }

    Time time = Time::GetNow();

    Array<MeshRenderer *> mrs =
        modelGameObject->GetComponentsInDescendantsAndThis<MeshRenderer>();
    for (MeshRenderer *mr : mrs)
    {
        Mesh *mesh = mr->GetMesh();
        Material *mat = mr->GetMaterial();
        if (!mat || !mesh || mesh->GetUvsPool().IsEmpty())
        {
            continue;
        }

        const Matrix4 &localToWorldMatrix =
            mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
        AABox meshAABox = mesh->GetAABBox();
        meshAABox = localToWorldMatrix * meshAABox;
        meshAABox.SetMin(meshAABox.GetMin() - meshAABox.GetSize() * 0.01f);
        meshAABox.SetMax(meshAABox.GetMax() + meshAABox.GetSize() * 0.01f);

        Vector3 gridCellSize = Vector3(meshAABox.GetSize() / 20.0f);
        Vector3i numBlocks = Vector3i(meshAABox.GetSize() / gridCellSize);

        UniformGrid triGrid(
            numBlocks.x, UniformGrid2(numBlocks.y, UniformGrid1(numBlocks.z)));
        Array<Array<Vector3i>> triangleLocationsInGrid(mesh->GetNumTriangles());

        const uint meshNumTris = mesh->GetNumTriangles();
        for (uint x = 0; x < numBlocks.x; ++x)
        {
            for (uint y = 0; y < numBlocks.y; ++y)
            {
                for (uint z = 0; z < numBlocks.z; ++z)
                {
                    const auto &GCS = gridCellSize;
                    AABox cellBox;
                    cellBox.SetMin(meshAABox.GetMin() + Vector3(x, y, z) * GCS -
                                   GCS * 0.2f);
                    cellBox.SetMax(cellBox.GetMin() + GCS * 1.2f);
                    triGrid[x][y][z].cellBox = cellBox;

                    Array<TriId> &triIdsInThisCell = triGrid[x][y][z].triIds;
                    for (TriId triId = 0; triId < meshNumTris; ++triId)
                    {
                        Triangle triangle = mesh->GetTriangle(triId);
                        triangle = localToWorldMatrix * triangle;
                        if (Geometry::IntersectAABoxTriangle(cellBox, triangle))
                        {
                            triangleLocationsInGrid[triId].PushBack(
                                Vector3i(x, y, z));
                            triIdsInThisCell.PushBack(triId);
                        }
                    }
                }
            }
        }

        const Vector2 imgSize = Vector2(effectImage->GetSize());
        Array<Array<bool>> paintedTexels(imgSize.y,
                                         Array<bool>(imgSize.x, false));
        for (uint triId = 0; triId < meshNumTris; ++triId)
        {
            ProcessTriangleTexels(triId,
                                  imgSize,
                                  mesh,
                                  triGrid,
                                  triangleLocationsInGrid,
                                  localToWorldMatrix,
                                  paintedTexels,
                                  effectImage);
        }
    }
    Debug_Peek((Time::GetNow() - time).GetSeconds());
}

EffectLayer::Type EffectLayerAmbientOcclusion::GetEffectLayerType() const
{
    return EffectLayer::Type::AMBIENT_OCCLUSION;
}

String EffectLayerAmbientOcclusion::GetTypeName() const
{
    return "Ambient Occlusion";
}
