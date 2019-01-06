#include "Dirter.h"

#include "Bang/Array.h"
#include "Bang/Assets.h"
#include "Bang/Dialog.h"
#include "Bang/GameObject.h"
#include "Bang/Image.h"
#include "Bang/Material.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Random.h"
#include "Bang/SimplexNoise.h"
#include "Bang/Texture2D.h"
#include "Bang/TextureFactory.h"
#include "Bang/Thread.h"
#include "Bang/Transform.h"
#include "Bang/Triangle.h"
#include "Bang/Triangle2D.h"

using namespace Bang;

void Dirter::AddDirt(GameObject *modelGameObject)
{
    if (!modelGameObject)
    {
        return;
    }

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

        if (Texture2D *originalAlbedoTex = mat->GetAlbedoTexture())
        {
            AddDirtToTexture(mr, originalAlbedoTex);
        }
    }
}

void Dirter::AddDirtToTexture(MeshRenderer *mr, Texture2D *texture)
{
    Image originalImg = texture->ToImage();
    Image finalImg = originalImg;
    const Vector2i imgSizei(originalImg.GetSize());
    const Vector2 imgSize(imgSizei);
    AABox boxWorld = mr->GetGameObject()->GetAABBoxWorld();
    const float maxSize = boxWorld.GetSize().GetMax();

    float freq = 10.0f * (1.0f / maxSize);
    Vector3 randOffset = Random::GetInsideUnitSphere() * 1000.0f;
    Array<Array<bool>> alreadyVisitedTexels(imgSize.y,
                                            Array<bool>(imgSize.x, false));
    SimplexNoise sxNoise = SimplexNoise(freq, 1.0f, 2.0f, 0.5f);

    Mesh *mesh = mr->GetMesh();
    for (uint triId = 0; triId < mr->GetMesh()->GetNumTriangles(); ++triId)
    {
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

        Triangle2D texelsTri(p0Texel, p1Texel, p2Texel);
        Vector2 txRMin = triTexelsRect.GetMin();
        Vector2 txRMax = triTexelsRect.GetMax();
        txRMin = Vector2::Clamp(txRMin, Vector2(0), imgSize - 1.0f);
        txRMax = Vector2::Clamp(txRMax, Vector2(0), imgSize - 1.0f);
        for (int txy = txRMin.y; txy <= txRMax.y; ++txy)
        {
            for (int txx = txRMin.x; txx <= txRMax.x; ++txx)
            {
                if (alreadyVisitedTexels[txy][txx])
                {
                    continue;
                }

                Vector2 texelPoint(txx, txy);
                if (!texelsTri.Contains(texelPoint))
                {
                    continue;
                }
                Vector3 texelBaryCoords =
                    texelsTri.GetBarycentricCoordinates(texelPoint);

                Triangle tri3D = mesh->GetTriangle(triId);
                tri3D = mr->GetGameObject()
                            ->GetTransform()
                            ->GetLocalToWorldMatrix() *
                        tri3D;
                Vector3 point3D = tri3D.GetPoint(texelBaryCoords);
                point3D += randOffset;

                float sxv = sxNoise.Fractal(8, point3D.x, point3D.y, point3D.z);
                sxv = sxv * 0.5f + 0.5f;

                Color dirt = Color(sxv, sxv, sxv, 1.0f);
                Color originalColor = originalImg.GetPixel(txx, txy);
                Color dirtedColor = originalColor * dirt;

                finalImg.SetPixel(txx, txy, dirtedColor);
                alreadyVisitedTexels[txy][txx] = true;
            }
        }
    }

    texture->Import(finalImg);
}
