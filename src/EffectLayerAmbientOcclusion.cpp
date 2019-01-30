#include "EffectLayerAmbientOcclusion.h"

#include "Bang/Dialog.h"
#include "Bang/GameObject.h"
#include "Bang/Material.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Random.h"
#include "Bang/Transform.h"
#include "Bang/Triangle2D.h"

#include "MeshRaycaster.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerAmbientOcclusion::EffectLayerAmbientOcclusion()
{
    m_meshRaycaster = new MeshRaycaster();
}

EffectLayerAmbientOcclusion::~EffectLayerAmbientOcclusion()
{
    delete m_meshRaycaster;
}

void EffectLayerAmbientOcclusion::GenerateEffectImage(Image *effectImage)
{
    GameObject *modelGameObject =
        MainScene::GetInstance()->GetView3DScene()->GetModelGameObject();
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

        m_meshRaycaster->SetMesh(mesh);

        const Matrix4 localToWorldMatrix =
            mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();

        const Vector2 imgSize = Vector2(effectImage->GetSize());
        Array<Array<bool>> paintedTexels(imgSize.y,
                                         Array<bool>(imgSize.x, false));
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
                    if (paintedTexels[txy][txx])
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
                    Vector3 point3D = tri3D.GetPoint(texelBaryCoords);
                    Vector3 triNormal = tri3D.GetNormal();

                    uint intersectedRays = 0;
                    uint totalRays = 10;
                    for (uint i = 0; i < totalRays; ++i)
                    {
                        Vector3 origin = point3D + triNormal * 0.1f;
                        Ray ray(origin, Random::GetInsideUnitSphere() * 10.0f);
                        bool rayIntersected = m_meshRaycaster->RaycastTest(
                            ray, localToWorldMatrix);
                        if (rayIntersected)
                        {
                            ++intersectedRays;
                        }
                    }

                    float ambientOcclusion =
                        (float(intersectedRays) / totalRays);
                    const float &ao = ambientOcclusion;

                    effectImage->SetPixel(txx, txy, Color(ao, ao, ao, 1.0f));
                    paintedTexels[txy][txx] = true;
                }
            }
        }
    }
}

EffectLayer::Type EffectLayerAmbientOcclusion::GetEffectLayerType() const
{
    return EffectLayer::Type::AMBIENT_OCCLUSION;
}

String EffectLayerAmbientOcclusion::GetTypeName() const
{
    return "Ambient Occlusion";
}
