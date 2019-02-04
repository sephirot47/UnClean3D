#include "MeshUniformGrid.h"

#include "Bang/AABox.h"
#include "Bang/GameObject.h"
#include "Bang/Geometry.h"
#include "Bang/Transform.h"

using namespace Bang;

MeshUniformGrid::MeshUniformGrid()
{
}

MeshUniformGrid::~MeshUniformGrid()
{
}

void MeshUniformGrid::Create(MeshRenderer *mr)
{
    using TriId = Mesh::TriangleId;

    m_grid.Clear();

    Mesh *mesh = mr->GetMesh();
    if (!mesh || mesh->GetUvsPool().IsEmpty())
    {
        return;
    }

    const Matrix4 &localToWorldMatrix =
        mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
    m_gridAABox = mesh->GetAABBox();
    m_gridAABox = localToWorldMatrix * GetGridAABox();
    const Vector3 padding = GetGridAABox().GetSize() * 0.01f;
    m_gridAABox.SetMin(GetGridAABox().GetMin() - padding);
    m_gridAABox.SetMax(GetGridAABox().GetMax() + padding);

    m_gridNumCells = 20;
    m_gridCellSize = (GetGridAABox().GetSize() / SCAST<float>(m_gridNumCells));
    m_grid.Resize(m_gridNumCells * m_gridNumCells * m_gridNumCells);

    const uint meshNumTris = mesh->GetNumTriangles();
    for (uint x = 0; x < m_gridNumCells; ++x)
    {
        for (uint y = 0; y < m_gridNumCells; ++y)
        {
            for (uint z = 0; z < m_gridNumCells; ++z)
            {
                const Vector3 xyz(x, y, z);
                const auto &GCS = m_gridCellSize;

                AABox cellBox;
                const Vector3 cellPadding = GCS * 0.2f;
                cellBox.SetMin(GetGridAABox().GetMin() + xyz * GCS -
                               cellPadding);
                cellBox.SetMax(cellBox.GetMin() + GCS + cellPadding);

                Cell &cell = GetCell_(x, y, z);
                for (TriId triId = 0; triId < meshNumTris; ++triId)
                {
                    Triangle triangle = mesh->GetTriangle(triId);
                    triangle = localToWorldMatrix * triangle;
                    if (Geometry::IntersectAABoxTriangle(cellBox, triangle))
                    {
                        cell.triangleIds.PushBack(triId);
                    }
                }
            }
        }
    }
}

uint MeshUniformGrid::GetNumCells() const
{
    return m_gridNumCells;
}

uint MeshUniformGrid::GetCellCoord(uint x, uint y, uint z) const
{
    return z * m_gridNumCells * m_gridNumCells + y * m_gridNumCells + x;
}

const MeshUniformGrid::Cell &MeshUniformGrid::GetCell(uint x,
                                                      uint y,
                                                      uint z) const
{
    return m_grid[GetCellCoord(x, y, z)];
}

const Vector3 &MeshUniformGrid::GetCellSize() const
{
    return m_gridCellSize;
}

const AABox &MeshUniformGrid::GetGridAABox() const
{
    return m_gridAABox;
}

MeshUniformGrid::Cell &MeshUniformGrid::GetCell_(uint x, uint y, uint z)
{
    return m_grid[GetCellCoord(x, y, z)];
}
