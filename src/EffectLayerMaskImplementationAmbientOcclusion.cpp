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
    m_trianglePositionsGLSLArray.SetFormat(GL::ColorFormat::RGB32F);
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
        FillTrianglePositionsGLSLArray(mr);
        FillMeshUniformGridGLSLArray();
        m_generatedTextureArrays = true;
    }

    Mesh *mesh = mr->GetMesh();

    const MeshUniformGrid &meshUniformGrid = GetMeshUniformGrid();

    sp->SetVector3("GridMinPoint", meshUniformGrid.GetGridAABox().GetMin());
    sp->SetInt("NumTriangles", mesh->GetNumTriangles());
    m_uniformGridGLSLArray.Bind("UniformMeshGrid", sp);
    m_trianglePositionsGLSLArray.Bind("TrianglePositions", sp);
    sp->SetInt("NumGridCells", meshUniformGrid.GetNumCells());
    sp->SetVector3("GridCellSize", meshUniformGrid.GetCellSize());
}

bool EffectLayerMaskImplementationAmbientOcclusion::
    CanGenerateEffectMaskTextureInRealTime() const
{
    return false;
}

void EffectLayerMaskImplementationAmbientOcclusion::
    FillTrianglePositionsGLSLArray(MeshRenderer *mr)
{
    Array<Array<Vector4>> allTrianglePositionsArray;

    Mesh *mesh = mr->GetMesh();
    const Matrix4 &localToWorldMatrix =
        mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Triangle tri = mesh->GetTriangle(triId);
        tri = localToWorldMatrix * tri;

        Array<Vector4> thisTrianglePositions;
        for (uint i = 0; i < 3; ++i)
        {
            thisTrianglePositions.PushBack(Vector4(tri.GetPoint(i), 0));
        }
        allTrianglePositionsArray.PushBack(thisTrianglePositions);
    }
    m_trianglePositionsGLSLArray.Fill(allTrianglePositionsArray);
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
