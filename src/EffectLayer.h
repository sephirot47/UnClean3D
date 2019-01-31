#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H

#include "Bang/Bang.h"
#include "Bang/Mesh.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"
#include "BangEditor/SerializableInspectorWidget.h"

using namespace Bang;

namespace Bang
{
class VBO;
class Framebuffer;
class MeshRenderer;
}

class ControlPanel;
class EffectLayerImplementation;

class EffectLayer : public Serializable
{
public:
    SERIALIZABLE(EffectLayer);

    enum class Type
    {
        DIRT = 0,
        NORMAL_LINES,
        FRACTAL_BUMPS,
        WAVE_BUMPS,
        AMBIENT_OCCLUSION,
        AMBIENT_OCCLUSION_GPU
    };

    enum class MaskType
    {
        FRACTAL,
        NORMAL_BASED,
        AMBIENT_OCCLUSION
    };

    EffectLayer(MeshRenderer *mr = nullptr);
    virtual ~EffectLayer() override;

    virtual void GenerateEffectTexture();
    void ReloadShaders();

    void SetImplementation(EffectLayerImplementation *impl);
    void SetType(EffectLayer::Type type);
    void SetVisible(bool visible);

    void PaintMaskBrush();

    void FillMask();
    void ClearMask();
    bool GetVisible() const;
    Mesh *GetMesh() const;
    EffectLayer::Type GetType() const;
    Texture2D *GetEffectTexture() const;
    Texture2D *GetMaskTexture() const;
    ControlPanel *GetControlPanel() const;
    Mesh *GetTextureMesh() const;
    EffectLayerImplementation *GetImplementation() const;

    // IReflectable
    virtual void Reflect() override;

private:
    EffectLayer::Type m_type = Undef<EffectLayer::Type>();
    EffectLayerImplementation *p_implementation = nullptr;  // PIMPL
    MeshRenderer *p_meshRenderer = nullptr;

    VBO *m_positionsVBO = nullptr;
    VBO *m_normalsVBO = nullptr;
    Framebuffer *m_framebuffer = nullptr;
    bool m_visible = true;
    AH<Mesh> m_textureMesh;
    AH<ShaderProgram> m_paintMaskBrushSP;
    AH<ShaderProgram> m_growTextureBordersSP;

    AH<Texture2D> m_effectTexture;
    AH<Texture2D> m_maskPingPongTexture0;
    AH<Texture2D> m_maskPingPongTexture1;
    AH<Texture2D> m_growAuxiliarTexture;
    Texture2D *p_lastDrawnMaskTexture = nullptr;

    void GrowTextureBorders(Texture2D *texture);
};

#endif  // EFFECTLAYER_H
