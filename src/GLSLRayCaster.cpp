#include "GLSLRayCaster.h"

#include "Bang/GameObject.h"
#include "Bang/MeshRenderer.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Transform.h"

using namespace Bang;

GLSLRayCaster::GLSLRayCaster()
{
    m_trianglePositionsGLSLArray.SetFormat(GL::ColorFormat::RGB32F);
    m_uniformGridGLSLArray.SetFormat(GL::ColorFormat::RG32F);
}

GLSLRayCaster::~GLSLRayCaster()
{
}

void GLSLRayCaster::SetMeshRenderer(MeshRenderer *mr)
{
    if (mr != GetMeshRenderer())
    {
        p_meshRenderer = mr;

        m_meshUniformGrid.Create(GetMeshRenderer());
        FillTrianglePositionsGLSLArray();
        FillMeshUniformGridGLSLArray();
    }
}

void GLSLRayCaster::Bind(ShaderProgram *sp)
{
    m_uniformGridGLSLArray.Bind("UniformMeshGrid", sp);
    m_trianglePositionsGLSLArray.Bind("TrianglePositions", sp);

    sp->SetVector3("GridMinPoint",
                   GetMeshUniformGrid().GetGridAABox().GetMin());
    sp->SetInt("NumGridCells", GetMeshUniformGrid().GetNumCells());
    sp->SetVector3("GridCellSize", GetMeshUniformGrid().GetCellSize());
}

void GLSLRayCaster::FillTrianglePositionsGLSLArray()
{
    Array<Array<Vector4>> allTrianglePositionsArray;

    Mesh *mesh = GetMeshRenderer()->GetMesh();
    const Matrix4 &localToWorldMatrix = GetMeshRenderer()
                                            ->GetGameObject()
                                            ->GetTransform()
                                            ->GetLocalToWorldMatrix();
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

void GLSLRayCaster::FillMeshUniformGridGLSLArray()
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

const MeshUniformGrid &GLSLRayCaster::GetMeshUniformGrid() const
{
    return m_meshUniformGrid;
}

MeshRenderer *GLSLRayCaster::GetMeshRenderer() const
{
    return p_meshRenderer;
}
