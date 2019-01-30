#include "MeshRaycaster.h"

#include "Bang/Geometry.h"
#include "Bang/Mesh.h"
#include "Bang/Octree.h"

using namespace Bang;

MeshRaycaster::MeshRaycaster()
{
}

MeshRaycaster::~MeshRaycaster()
{
}

void MeshRaycaster::SetMesh(Mesh *mesh)
{
    p_mesh.Set(mesh);

    // AABox meshAABox = mesh->GetAABBox();
    // meshAABox.SetMin(meshAABox.GetMin() - Vector3(0.1f));
    // meshAABox.SetMax(meshAABox.GetMax() + Vector3(0.1f));
    //
    // Array<Mesh::TriangleId> meshTriangleIds;
    // for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles();
    // ++triId)
    // {
    //     meshTriangleIds.PushBack(triId);
    // }
    //
    // m_octree = new Octree<Mesh::TriangleId>();
    // m_octree->SetClassifyFunction(
    //     [this](const AABox &box, const Mesh::TriangleId &triId) {
    //         Triangle triangle = GetMesh()->GetTriangle(triId);
    //         return Geometry::IntersectAABoxTriangle(box, triangle);
    //     });
    // m_octree->SetAABox(meshAABox);
    // m_octree->Fill(meshTriangleIds, 8);
}

void MeshRaycaster::Raycast(const Ray &worldSpaceRay,
                            const Matrix4 &localToWorldMatrix,
                            bool *intersected,
                            Vector3 *collisionPoint)
{
    // for (Mesh::TriangleId triId = 0; triId < GetMesh()->GetNumTriangles();
    //      ++triId)
    // {
    //     Triangle tri = GetMesh()->GetTriangle(triId);
    //
    //     Geometry::IntersectRayTriangle()
    // }
}

bool MeshRaycaster::RaycastTest(const Ray &worldSpaceRay,
                                const Matrix4 &localToWorldMatrix)
{
    for (Mesh::TriangleId triId = 0; triId < GetMesh()->GetNumTriangles();
         ++triId)
    {
        Triangle tri = GetMesh()->GetTriangle(triId);
        tri = localToWorldMatrix * tri;

        float _;
        bool intersected;

        Geometry::IntersectRayTriangle(worldSpaceRay, tri, &intersected, &_);
        if (intersected)
        {
            return true;
        }
    }
    return false;
}

Mesh *MeshRaycaster::GetMesh() const
{
    return p_mesh.Get();
}
