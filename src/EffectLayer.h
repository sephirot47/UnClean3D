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
class EffectLayerImplementation;

class EffectLayer
{
public:
    enum BlendMode
    {
        ADD = 0,
        INV_ADD = 1,
        MULT = 2,
        INV_MULT = 3
    };

    EffectLayer(MeshRenderer *mr);
    virtual ~EffectLayer();

    virtual void GenerateEffectTexture();
    void ReloadShaders();

    void SetEffectLayerImplementation(EffectLayerImplementation *impl);

    Texture2D *GetEffectTexture() const;
    ControlPanel *GetControlPanel() const;
    Mesh *GetTextureMesh() const;
    EffectLayerImplementation *GetImplementation() const;
    ShaderProgram *GetGenerateEffectTextureShaderProgram() const;

private:
    EffectLayerImplementation *p_implementation = nullptr;
    MeshRenderer *p_meshRenderer = nullptr;

    VBO *m_positionsVBO = nullptr;
    Framebuffer *m_framebuffer = nullptr;
    AH<Mesh> m_textureMesh;
    AH<ShaderProgram> m_generateEffectTextureSP;

    AH<Texture2D> m_effectTexture;
};

// PIMPL
class EffectLayerImplementation
{
public:
    EffectLayerImplementation();
    virtual ~EffectLayerImplementation();

    virtual EffectLayer::BlendMode GetBlendMode() const = 0;
    Texture2D *GetEffectTexture() const;
    EffectLayer *GetEffectLayer() const;

protected:
    virtual Path GetGenerateEffectTextureShaderProgramPath() const = 0;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp);

private:
    EffectLayer *p_effectLayer = nullptr;

    friend class EffectLayer;
};

#endif  // EFFECTLAYER_H
