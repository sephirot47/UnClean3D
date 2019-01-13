#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H

#include "Bang/Bang.h"
#include "Bang/Mesh.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"
#include "EffectLayerParameters.h"

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
    enum Type
    {
        DIRT = 0
    };

    EffectLayer(MeshRenderer *mr);
    virtual ~EffectLayer();

    virtual void GenerateEffectTexture();
    void ReloadShaders();

    virtual void UpdateParameters(const EffectLayerParameters &parameters);
    void SetEffectLayerImplementation(EffectLayerImplementation *impl);

    Texture2D *GetEffectTexture() const;
    ControlPanel *GetControlPanel() const;
    Mesh *GetTextureMesh() const;
    const EffectLayerParameters &GetParameters() const;
    EffectLayerImplementation *GetImplementation() const;
    ShaderProgram *GetGenerateEffectTextureShaderProgram() const;

private:
    EffectLayerParameters m_params;

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

    virtual EffectLayer::Type GetEffectLayerType() const = 0;
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
