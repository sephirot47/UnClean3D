#ifndef GLSLRAYCASTER_H
#define GLSLRAYCASTER_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "GLSLArrayOfArrays.h"
#include "MeshUniformGrid.h"

namespace Bang
{
class MeshRenderer;
};

using namespace Bang;

class GLSLRayCaster
{
public:
    GLSLRayCaster();
    virtual ~GLSLRayCaster();

    void SetMeshRenderer(MeshRenderer *mr);
    void Bind(ShaderProgram *sp);

    MeshRenderer *GetMeshRenderer() const;

private:
    MeshRenderer *p_meshRenderer = nullptr;
    MeshUniformGrid m_meshUniformGrid;
    GLSLArrayOfArrays m_uniformGridGLSLArray;
    GLSLArrayOfArrays m_trianglePositionsGLSLArray;

    void FillTrianglePositionsGLSLArray();
    void FillMeshUniformGridGLSLArray();
    const MeshUniformGrid &GetMeshUniformGrid() const;
};

#endif  // GLSLRAYCASTER_H
