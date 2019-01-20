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
        DIRT = 0,
        NORMAL_LINES
    };

    EffectLayer(MeshRenderer *mr);
    virtual ~EffectLayer();

    virtual void GenerateEffectTexture();
    void ReloadShaders();

    virtual void UpdateParameters(const EffectLayerParameters &parameters);
    void SetImplementation(EffectLayerImplementation *impl);
    void SetType(EffectLayer::Type type);

    void PaintMaskBrush();

    void ClearMask();
    Texture2D *GetEffectTexture() const;
    Texture2D *GetMaskTexture() const;
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
    VBO *m_normalsVBO = nullptr;
    Framebuffer *m_framebuffer = nullptr;
    AH<Mesh> m_textureMesh;
    AH<ShaderProgram> m_generateEffectTextureSP;
    AH<ShaderProgram> m_paintMaskBrushSP;
    AH<ShaderProgram> m_growTextureBordersSP;

    AH<Texture2D> m_effectTexture;
    AH<Texture2D> m_maskPingPongTexture0;
    AH<Texture2D> m_maskPingPongTexture1;
    AH<Texture2D> m_growAuxiliarTexture;
    Texture2D *p_lastDrawnMaskTexture = nullptr;

    void GrowTextureBorders(Texture2D *texture);
};

// PIMPL
class EffectLayerImplementation
{
public:
    EffectLayerImplementation();
    virtual ~EffectLayerImplementation();

    const EffectLayerParameters &GetParameters() const;
    virtual EffectLayer::Type GetEffectLayerType() const = 0;
    EffectLayer *GetEffectLayer() const;
    virtual String GetTypeName() const = 0;

protected:
    virtual Path GetGenerateEffectTextureShaderProgramPath() const = 0;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp);

private:
    EffectLayer *p_effectLayer = nullptr;

    friend class EffectLayer;
};

#endif  // EFFECTLAYER_H
