#ifndef MESHRAYCASTER_H
#define MESHRAYCASTER_H

#include "Bang/AssetHandle.h"
#include "Bang/Mesh.h"
#include "Bang/Octree.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class MeshRaycaster
{
public:
    MeshRaycaster(Mesh *mesh);
    virtual ~MeshRaycaster();

    Mesh *GetMesh() const;

private:
    AH<Mesh> p_mesh;
    Octree<Mesh::TriangleId> *m_octree = nullptr;
};

#endif  // MESHRAYCASTER_H
