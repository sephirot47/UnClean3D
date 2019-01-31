#include "EffectLayerAmbientOcclusionGPU.h"

#include "Bang/Assets.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/Transform.h"

using namespace Bang;

EffectLayerAmbientOcclusionGPU::EffectLayerAmbientOcclusionGPU()
{
    m_trianglePositionsTexture = Assets::Create<Texture2D>();
    m_trianglePositionsTexture.Get()->CreateEmpty(PositionsTextureSize,
                                                  PositionsTextureSize);
    m_trianglePositionsTexture.Get()->SetFormat(GL::ColorFormat::RGBA32F);
    m_trianglePositionsTexture.Get()->SetFilterMode(GL::FilterMode::NEAREST);
}

EffectLayerAmbientOcclusionGPU::~EffectLayerAmbientOcclusionGPU()
{
}

void EffectLayerAmbientOcclusionGPU::Reflect()
{
    EffectLayerImplementationGPU::Reflect();
}

EffectLayer::Type EffectLayerAmbientOcclusionGPU::GetEffectLayerType() const
{
    return EffectLayer::Type::AMBIENT_OCCLUSION_GPU;
}

String EffectLayerAmbientOcclusionGPU::GetTypeName() const
{
    return "Ambient Occlusion GPU";
}

Path EffectLayerAmbientOcclusionGPU::GetGenerateEffectTextureShaderProgramPath()
    const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectTextureAmbientOcclusion.bushader");
}

void EffectLayerAmbientOcclusionGPU::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    EffectLayerImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);

    const Matrix4 &localToWorldMatrix =
        meshRend->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
    Mesh *mesh = meshRend->GetMesh();
    Array<Vector4> positionsTextureData(PositionsTextureSize *
                                        PositionsTextureSize);
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Triangle tri = mesh->GetTriangle(triId);
        tri = localToWorldMatrix * tri;

        for (uint i = 0; i < 3; ++i)
        {
            uint coord = (triId * 3 + i);
            positionsTextureData[coord] = Vector4(tri.GetPoint(i), 1);
        }
    }

    m_trianglePositionsTexture.Get()->Fill(
        RCAST<Byte *>(positionsTextureData.Data()),
        PositionsTextureSize,
        PositionsTextureSize,
        GL::ColorComp::RGBA,
        GL::DataType::FLOAT);

    sp->SetInt("NumTriangles", mesh->GetNumTriangles());
    sp->SetTexture2D("TrianglePositions", m_trianglePositionsTexture.Get());
}

bool EffectLayerAmbientOcclusionGPU::CanGenerateEffectTextureInRealTime() const
{
    return false;
}
