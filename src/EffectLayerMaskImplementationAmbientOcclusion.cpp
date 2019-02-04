#include "EffectLayerMaskImplementationAmbientOcclusion.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Transform.h"

using namespace Bang;

EffectLayerMaskImplementationAmbientOcclusion::
    EffectLayerMaskImplementationAmbientOcclusion()
{
    m_trianglePositionsTexture = Assets::Create<Texture2D>();
    m_trianglePositionsTexture.Get()->CreateEmpty(PositionsTextureSize,
                                                  PositionsTextureSize);
    m_trianglePositionsTexture.Get()->SetFormat(GL::ColorFormat::RGBA32F);
    m_trianglePositionsTexture.Get()->SetFilterMode(GL::FilterMode::NEAREST);
    m_trianglePositionsTexture.Get()->Fill(
        Color::White(), PositionsTextureSize, PositionsTextureSize);

    m_uniformGridTexture = Assets::Create<Texture2D>();
    m_uniformGridTexture.Get()->CreateEmpty(UniformGridTextureSize,
                                            UniformGridTextureSize);
    m_uniformGridTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);
    m_uniformGridTexture.Get()->SetFilterMode(GL::FilterMode::NEAREST);
    m_uniformGridTexture.Get()->Fill(
        Color::White(), UniformGridTextureSize, UniformGridTextureSize);
}

EffectLayerMaskImplementationAmbientOcclusion::
    ~EffectLayerMaskImplementationAmbientOcclusion()
{
}

void EffectLayerMaskImplementationAmbientOcclusion::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();
}

EffectLayerMask::Type
EffectLayerMaskImplementationAmbientOcclusion::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::AMBIENT_OCCLUSION;
}

String EffectLayerMaskImplementationAmbientOcclusion::GetTypeName() const
{
    return "Ambient Occlusion";
}

Path EffectLayerMaskImplementationAmbientOcclusion::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureAmbientOcclusion.bushader");
}

void EffectLayerMaskImplementationAmbientOcclusion::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *mr)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, mr);

    CreateTrianglePositionsTexture(mr);
    CreateMeshUniformGridTexture(mr);

    Mesh *mesh = mr->GetMesh();

    sp->SetInt("NumTrisPerCell", NumTrisPerCell);
    sp->SetVector3("GridMinPoint", m_meshUniformGrid.GetGridAABox().GetMin());
    sp->SetInt("NumTriangles", mesh->GetNumTriangles());
    sp->SetTexture2D("TrianglePositions", m_trianglePositionsTexture.Get());
    sp->SetTexture2D("GridTexture", m_uniformGridTexture.Get());
    sp->SetInt("NumGridCells", m_meshUniformGrid.GetNumCells());
    sp->SetVector3("GridCellSize", m_meshUniformGrid.GetCellSize());
}

bool EffectLayerMaskImplementationAmbientOcclusion::
    CanGenerateEffectMaskTextureInRealTime() const
{
    return false;
}

Texture2D *
EffectLayerMaskImplementationAmbientOcclusion::CreateTrianglePositionsTexture(
    MeshRenderer *mr)
{
    Mesh *mesh = mr->GetMesh();
    const Matrix4 &localToWorldMatrix =
        mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
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

    return m_trianglePositionsTexture.Get();
}

#include "Bang/Random.h"
Texture2D *
EffectLayerMaskImplementationAmbientOcclusion::CreateMeshUniformGridTexture(
    MeshRenderer *mr)
{
    m_meshUniformGrid.Create(mr);

    Array<Color> gridTextureData(UniformGridTextureSize *
                                 UniformGridTextureSize);
    for (int x = 0; x < m_meshUniformGrid.GetNumCells(); ++x)
    {
        for (int y = 0; y < m_meshUniformGrid.GetNumCells(); ++y)
        {
            for (int z = 0; z < m_meshUniformGrid.GetNumCells(); ++z)
            {
                uint baseCoord =
                    m_meshUniformGrid.GetCellCoord(x, y, z) * NumTrisPerCell;
                const MeshUniformGrid::Cell &cell =
                    m_meshUniformGrid.GetCell(x, y, z);
                Color pixelColor;
                for (int i = 0; i < NumTrisPerCell; ++i)
                {
                    if (i < cell.triangleIds.Size())
                    {
                        constexpr int C = 255;
                        constexpr float CF = SCAST<float>(C);
                        int triId = cell.triangleIds[i];
                        pixelColor.r = ((triId / (C * C)) % C) / CF;
                        pixelColor.g = ((triId / (C)) % C) / CF;
                        pixelColor.b = ((triId / (1)) % C) / CF;
                        pixelColor.a = 1.0f;
                    }
                    else
                    {
                        pixelColor.a = 0.0f;
                    }
                    gridTextureData[baseCoord + i] = pixelColor;
                }
            }
        }
    }

    m_uniformGridTexture.Get()->Fill(RCAST<Byte *>(gridTextureData.Data()),
                                     UniformGridTextureSize,
                                     UniformGridTextureSize,
                                     GL::ColorComp::RGBA,
                                     GL::DataType::FLOAT);

    return m_uniformGridTexture.Get();
}
