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
    MeshRaycaster();
    virtual ~MeshRaycaster();

    void SetMesh(Mesh *mesh);

    void Raycast(const Ray &worldSpaceRay,
                 const Matrix4 &localToWorldMatrix,
                 bool *intersected,
                 Vector3 *collisionPoint);
    bool RaycastTest(const Ray &worldSpaceRay,
                     const Matrix4 &localToWorldMatrix);

    Mesh *GetMesh() const;

private:
    AH<Mesh> p_mesh;
    Octree<Mesh::TriangleId> *m_octree = nullptr;
};

#endif  // MESHRAYCASTER_H
