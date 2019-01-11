#ifndef DIRTER_H
#define DIRTER_H

#include "Bang/Bang.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

namespace Bang
{
class VBO;
class Framebuffer;
class MeshRenderer;
}

class ControlPanel;

class Dirter
{
public:
    Dirter(MeshRenderer *mr);
    virtual ~Dirter();

    void CreateDirtTexture();
    void ReloadShaders();

    Texture2D *GetDirtTexture() const;

private:
    MeshRenderer *p_meshRenderer = nullptr;

    VBO *m_positionsVBO = nullptr;
    Framebuffer *m_framebuffer = nullptr;
    AH<Mesh> m_textureMesh;
    AH<ShaderProgram> m_createDirtShaderProgram;

    AH<Texture2D> m_dirtTexture;

    Mesh *GetTextureMesh() const;
    ControlPanel *GetControlPanel() const;
};

#endif  // DIRTER_H
