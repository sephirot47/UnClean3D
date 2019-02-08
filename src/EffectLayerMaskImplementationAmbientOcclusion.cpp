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

    m_uniformGridGLSLArray.SetFormat(GL::ColorFormat::RG32F);
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

    if (!m_generatedTextureArrays)
    {
        CreateTrianglePositionsTexture(mr);
        FillMeshUniformGridGLSLArray();
        m_generatedTextureArrays = true;
    }

    Mesh *mesh = mr->GetMesh();

    const MeshUniformGrid &meshUniformGrid = GetMeshUniformGrid();

    sp->SetVector3("GridMinPoint", meshUniformGrid.GetGridAABox().GetMin());
    sp->SetInt("NumTriangles", mesh->GetNumTriangles());
    sp->SetTexture2D("TrianglePositions", m_trianglePositionsTexture.Get());
    sp->SetTexture2D("GridTexture", m_uniformGridGLSLArray.GetArrayTexture());
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

void EffectLayerMaskImplementationAmbientOcclusion::
    FillMeshUniformGridGLSLArray()
{
    const MeshUniformGrid &meshUniformGrid = GetMeshUniformGrid();

    Array<Array<Vector4>> gridArray;
    const int NC = meshUniformGrid.GetNumCells();

    for (int z = 0; z < NC; ++z)
    {
        for (int y = 0; y < NC; ++y)
        {
            for (int x = 0; x < NC; ++x)
            {
                const MeshUniformGrid::Cell &cell =
                    meshUniformGrid.GetCell(x, y, z);

                Array<Vector4> triIdsInThisCell;
                for (uint i = 0; i < cell.triangleIds.Size(); ++i)
                {
                    Vector4 element = Vector4(cell.triangleIds[i], 0, 0, 0);
                    triIdsInThisCell.PushBack(element);
                }
                gridArray.PushBack(triIdsInThisCell);
            }
        }
    }
    m_uniformGridGLSLArray.Fill(gridArray);
}

const MeshUniformGrid &
EffectLayerMaskImplementationAmbientOcclusion::GetMeshUniformGrid() const
{
    return MainScene::GetInstance()->GetView3DScene()->GetMeshUniformGrid();
}
