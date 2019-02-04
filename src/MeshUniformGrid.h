#ifndef MESHUNIFORMGRID_H
#define MESHUNIFORMGRID_H

#include "Bang/Bang.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class MeshUniformGrid
{
public:
    struct Cell
    {
        Array<Mesh::TriangleId> triangleIds;
    };

    MeshUniformGrid();
    virtual ~MeshUniformGrid();

    void Create(MeshRenderer *meshRenderer);
    uint GetNumCells() const;
    uint GetCellCoord(uint x, uint y, uint z) const;
    const Cell &GetCell(uint x, uint y, uint z) const;
    const Vector3 &GetCellSize() const;
    const AABox &GetGridAABox() const;

private:
    AABox m_gridAABox;
    Array<Cell> m_grid;
    uint m_gridNumCells = 0;
    Vector3 m_gridCellSize = Vector3::Zero();

    Cell &GetCell_(uint x, uint y, uint z);
};

#endif  // MESHUNIFORMGRID_H
