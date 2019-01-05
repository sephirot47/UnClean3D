#include "Dirter.h"

#include "Bang/Array.h"
#include "Bang/Assets.h"
#include "Bang/GameObject.h"
#include "Bang/Image.h"
#include "Bang/Material.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Random.h"
#include "Bang/SimplexNoise.h"
#include "Bang/Texture2D.h"
#include "Bang/TextureFactory.h"
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
            Image originalAlbedoImg = originalAlbedoTex->ToImage();
            Image finalAlbedoImg = originalAlbedoImg;
            const Vector2i imgSizei(originalAlbedoImg.GetSize());
            const Vector2 imgSize(imgSizei);

            for (uint triId = 0; triId < mesh->GetNumTriangles(); ++triId)
            {
                Triangle tri3D = mesh->GetTriangle(triId);
                Color triColor = Random::GetColorOpaque();

                Mesh::VertexId vId0 =
                    mesh->GetTrianglesVertexIds()[triId * 3 + 0];
                Mesh::VertexId vId1 =
                    mesh->GetTrianglesVertexIds()[triId * 3 + 1];
                Mesh::VertexId vId2 =
                    mesh->GetTrianglesVertexIds()[triId * 3 + 2];

                const Vector2 &p0Uv = mesh->GetUvsPool()[vId0];
                const Vector2 &p1Uv = mesh->GetUvsPool()[vId1];
                const Vector2 &p2Uv = mesh->GetUvsPool()[vId2];
                Vector2i p0Texel = Vector2i(p0Uv * imgSize);
                Vector2i p1Texel = Vector2i(p1Uv * imgSize);
                Vector2i p2Texel = Vector2i(p2Uv * imgSize);
                p0Texel.y = (imgSize.y - p0Texel.y - 1);
                p1Texel.y = (imgSize.y - p1Texel.y - 1);
                p2Texel.y = (imgSize.y - p2Texel.y - 1);

                AARecti triTexelsRect;
                triTexelsRect.AddPoint(p0Texel);
                triTexelsRect.AddPoint(p1Texel);
                triTexelsRect.AddPoint(p2Texel);

                const Triangle2D triTexels2D = Triangle2D(
                    Vector2(p0Texel), Vector2(p1Texel), Vector2(p2Texel));
                Vector2i rMin = triTexelsRect.GetMin();
                Vector2i rMax = triTexelsRect.GetMax();
                rMin = Vector2i::Clamp(rMin, Vector2i::Zero(), imgSizei - 1);
                rMax = Vector2i::Clamp(rMax, Vector2i::Zero(), imgSizei - 1);
                for (int txy = rMin.y; txy < rMax.y; ++txy)
                {
                    for (int txx = rMin.x; txx < rMax.x; ++txx)
                    {
                        Vector2 texelPoint(txx, txy);
                        if (triTexels2D.Contains(texelPoint))
                        {
                            finalAlbedoImg.SetPixel(txx, txy, triColor);
                        }
                    }
                }
            }

            AH<Texture2D> finalAlbedoTex = Assets::Create<Texture2D>();
            finalAlbedoTex.Get()->Import(finalAlbedoImg);

            mr->GetMaterial()->SetAlbedoTexture(finalAlbedoTex.Get());
        }
    }
}
