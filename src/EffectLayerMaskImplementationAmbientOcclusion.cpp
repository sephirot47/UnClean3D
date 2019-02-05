#include "EffectLayerMaskImplementationAmbientOcclusion.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Transform.h"

#include "View3DScene.h"

using namespace Bang;

EffectLayerMaskImplementationAmbientOcclusion::
    EffectLayerMaskImplementationAmbientOcclusion()
{
    m_trianglePositionsTexture = Assets::Create<Texture2D>();
    m_trianglePositionsTexture.Get()->CreateEmpty(PositionsTextureSize,
                                                  PositionsTextureSize);
    m_trianglePositionsTexture.Get()->SetFormat(GL::ColorFormat::RGB32F);
    m_trianglePositionsTexture.Get()->SetFilterMode(GL::FilterMode::NEAREST);
    m_trianglePositionsTexture.Get()->Fill(
        Color::White(), PositionsTextureSize, PositionsTextureSize);

    m_uniformGridTexture = Assets::Create<Texture2D>();
    m_uniformGridTexture.Get()->CreateEmpty(UniformGridTextureSize,
                                            UniformGridTextureSize);
    m_uniformGridTexture.Get()->SetFormat(GL::ColorFormat::RG32F);
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

    if (!m_generatedTextures)
    {
        CreateTrianglePositionsTexture(mr);
        CreateMeshUniformGridTexture(mr);
        m_generatedTextures = true;
    }

    Mesh *mesh = mr->GetMesh();

    const MeshUniformGrid &meshUniformGrid = GetMeshUniformGrid();

    sp->SetVector3("GridMinPoint", meshUniformGrid.GetGridAABox().GetMin());
    sp->SetInt("NumTriangles", mesh->GetNumTriangles());
    sp->SetTexture2D("TrianglePositions", m_trianglePositionsTexture.Get());
    sp->SetTexture2D("GridTexture", m_uniformGridTexture.Get());
    sp->SetInt("NumGridCells", meshUniformGrid.GetNumCells());
    sp->SetVector3("GridCellSize", meshUniformGrid.GetCellSize());
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
    Array<Vector3> positionsTextureData(PositionsTextureSize *
                                        PositionsTextureSize);
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Triangle tri = mesh->GetTriangle(triId);
        tri = localToWorldMatrix * tri;

        for (uint i = 0; i < 3; ++i)
        {
            uint coord = (triId * 3 + i);
            positionsTextureData[coord] = Vector3(tri.GetPoint(i));
        }
    }

    m_trianglePositionsTexture.Get()->Fill(
        RCAST<Byte *>(positionsTextureData.Data()),
        PositionsTextureSize,
        PositionsTextureSize,
        GL::ColorComp::RGB,
        GL::DataType::FLOAT);
    return m_trianglePositionsTexture.Get();
}

Texture2D *
EffectLayerMaskImplementationAmbientOcclusion::CreateMeshUniformGridTexture(
    MeshRenderer *mr)
{
    const MeshUniformGrid &meshUniformGrid = GetMeshUniformGrid();

    const int TotalTexSize = UniformGridTextureSize * UniformGridTextureSize;
    Array<Vector2> gridTextureData(TotalTexSize);

    const int NC = meshUniformGrid.GetNumCells();

    int triangleListBegin = NC * NC * NC;
    int currentTexCoord = 0;
    int triangleListSize = 0;
    for (int z = 0; z < NC; ++z)
    {
        for (int y = 0; y < NC; ++y)
        {
            for (int x = 0; x < NC; ++x)
            {
                const MeshUniformGrid::Cell &cell =
                    meshUniformGrid.GetCell(x, y, z);
                const uint numCellTriangles = cell.triangleIds.Size();

                gridTextureData[currentTexCoord] = Vector2(
                    triangleListBegin + triangleListSize,
                    triangleListBegin + triangleListSize + numCellTriangles);
                triangleListSize += numCellTriangles;

                ++currentTexCoord;
            }
        }
    }

    for (int z = 0; z < NC; ++z)
    {
        for (int y = 0; y < NC; ++y)
        {
            for (int x = 0; x < NC; ++x)
            {
                if (currentTexCoord < TotalTexSize)
                {
                    const MeshUniformGrid::Cell &cell =
                        meshUniformGrid.GetCell(x, y, z);

                    Vector2 pixelRG;
                    for (uint i = 0; i < cell.triangleIds.Size(); ++i)
                    {
                        pixelRG.x = cell.triangleIds[i];
                        gridTextureData[currentTexCoord] = pixelRG;
                        ++currentTexCoord;
                    }
                }
                else
                {
                    Debug_Error("Not enough texture size to represent the "
                                "uniform grid");
                    ASSERT(false);
                    exit(1);
                }
            }
        }
    }

    m_uniformGridTexture.Get()->Fill(RCAST<Byte *>(gridTextureData.Data()),
                                     UniformGridTextureSize,
                                     UniformGridTextureSize,
                                     GL::ColorComp::RG,
                                     GL::DataType::FLOAT);
    return m_uniformGridTexture.Get();
}

const MeshUniformGrid &
EffectLayerMaskImplementationAmbientOcclusion::GetMeshUniformGrid() const
{
    return MainScene::GetInstance()->GetView3DScene()->GetMeshUniformGrid();
}
