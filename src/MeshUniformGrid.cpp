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

    m_gridNumCells = 50;
    m_gridCellSize = (GetGridAABox().GetSize() / SCAST<float>(m_gridNumCells));
    m_grid.Resize(m_gridNumCells * m_gridNumCells * m_gridNumCells);

    const uint meshNumTris = mesh->GetNumTriangles();

    Array<Mesh::TriangleId> allTriIds;
    for (Mesh::TriangleId triId = 0; triId < meshNumTris; ++triId)
    {
        allTriIds.PushBack(triId);
    }
    Fill(Vector3i::Zero(), Vector3i(m_gridNumCells), mr, allTriIds);
}

uint MeshUniformGrid::GetNumCells() const
{
    return m_gridNumCells;
}

uint MeshUniformGrid::GetCellCoord(uint x, uint y, uint z) const
{
    return z * m_gridNumCells * m_gridNumCells + y * m_gridNumCells + x;
}

AABox MeshUniformGrid::GetCellBox(uint x, uint y, uint z) const
{
    return GetCellBox(x, y, z, Vector3i::One());
}

AABox MeshUniformGrid::GetCellBox(uint x,
                                  uint y,
                                  uint z,
                                  const Vector3i &rangeSize) const
{
    const Vector3 xyz(x, y, z);
    const Vector3 normalSize = m_gridCellSize;
    const Vector3 padding = normalSize * 0.05f;

    AABox cellBox;
    cellBox.SetMin(GetGridAABox().GetMin() + xyz * normalSize - padding);
    cellBox.SetMax(cellBox.GetMin() + Vector3(rangeSize) * normalSize +
                   padding * 2.0f);

    return cellBox;
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

void MeshUniformGrid::Fill(const Vector3i &rangeBegin,
                           const Vector3i &rangeSize,
                           MeshRenderer *mr,
                           const Array<Mesh::TriangleId> &triangleIds)
{
    Mesh *mesh = mr->GetMesh();
    const Matrix4 &localToWorldMatrix =
        mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();

    if (rangeSize <= Vector3i::One())
    {
        Cell &cell = GetCell_(rangeBegin.x, rangeBegin.y, rangeBegin.z);
        AABox cellBox = GetCellBox(rangeBegin.x, rangeBegin.y, rangeBegin.z);
        for (Mesh::TriangleId triId : triangleIds)
        {
            Triangle triangle = mesh->GetTriangle(triId);
            triangle = localToWorldMatrix * triangle;
            if (Geometry::IntersectAABoxTriangle(cellBox, triangle))
            {
                cell.triangleIds.PushBack(triId);
            }
        }
    }
    else
    {
        for (int x = 0; x <= 1; ++x)
        {
            for (int y = 0; y <= 1; ++y)
            {
                for (int z = 0; z <= 1; ++z)
                {
                    const Vector3i xyz(x, y, z);
                    const Vector3i subCellRangeSize =
                        (rangeSize + Vector3i::One()) / 2;
                    const Vector3i subCellRangeBegin =
                        rangeBegin + xyz * (subCellRangeSize);

                    AABox subCellBox = GetCellBox(subCellRangeBegin.x,
                                                  subCellRangeBegin.y,
                                                  subCellRangeBegin.z,
                                                  subCellRangeSize);
                    Cell subCell = CreateCell(subCellBox, mr, triangleIds);

                    if (subCell.triangleIds.Size() >= 1)
                    {
                        Fill(subCellRangeBegin,
                             subCellRangeSize,
                             mr,
                             subCell.triangleIds);
                    }
                }
            }
        }
    }
}

MeshUniformGrid::Cell MeshUniformGrid::CreateCell(
    const AABox &box,
    MeshRenderer *mr,
    const Array<Mesh::TriangleId> &triangleIds)
{
    Mesh *mesh = mr->GetMesh();
    const Matrix4 &localToWorldMatrix =
        mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();

    Cell cell;
    for (Mesh::TriangleId triId : triangleIds)
    {
        Triangle triangle = mesh->GetTriangle(triId);
        triangle = localToWorldMatrix * triangle;
        if (Geometry::IntersectAABoxTriangle(box, triangle))
        {
            cell.triangleIds.PushBack(triId);
        }
    }
    return cell;
}

MeshUniformGrid::Cell &MeshUniformGrid::GetCell_(uint x, uint y, uint z)
{
    return m_grid[GetCellCoord(x, y, z)];
}
