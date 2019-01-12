#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H

#include "Bang/Bang.h"
#include "Bang/Mesh.h"
#include "Bang/ShaderProgram.h"
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

class EffectLayer
{
public:
    EffectLayer();
    virtual ~EffectLayer();

    void Init(MeshRenderer *mr);
    virtual void GenerateEffectTexture();
    void ReloadShaders();

    virtual String GetUniformName() const = 0;
    Texture2D *GetEffectTexture() const;

protected:
    virtual Path GetGenerateEffectTextureShaderProgramPath() const = 0;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp);

    Mesh *GetTextureMesh() const;
    ControlPanel *GetControlPanel() const;
    ShaderProgram *GetGenerateEffectTextureShaderProgram() const;

private:
    MeshRenderer *p_meshRenderer = nullptr;

    VBO *m_positionsVBO = nullptr;
    Framebuffer *m_framebuffer = nullptr;
    AH<Mesh> m_textureMesh;
    AH<ShaderProgram> m_generateEffectTextureSP;

    AH<Texture2D> m_effectTexture;
};

#endif  // EFFECTLAYER_H
