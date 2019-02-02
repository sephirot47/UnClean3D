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
class EffectLayerMask;
class EffectLayerMaskImplementation;

class EffectLayer : public Serializable
{
public:
    SERIALIZABLE(EffectLayer);

    enum class BlendMode
    {
        ADD = 0,
        SUBTRACT,
        MULTIPLY
    };

    EffectLayer(MeshRenderer *mr = nullptr);
    virtual ~EffectLayer() override;

    virtual void GenerateEffectTexture();
    virtual void MergeMasks();
    void ReloadShaders();

    void SetImplementation(EffectLayerMaskImplementation *impl);
    void SetVisible(bool visible);
    void SetName(const String &name);
    void SetBlendMode(EffectLayer::BlendMode blendMode);

    EffectLayerMask *AddNewMask();
    void RemoveMask(EffectLayerMask *mask);
    void Invalidate();

    bool GetVisible() const;
    Mesh *GetMesh() const;
    EffectLayer::BlendMode GetBlendMode() const;
    Texture2D *GetEffectColorTexture() const;
    Texture2D *GetEffectMiscTexture() const;
    const Array<EffectLayerMask *> &GetMasks() const;
    ControlPanel *GetControlPanel() const;
    Mesh *GetTextureMesh() const;
    MeshRenderer *GetMeshRenderer() const;
    Texture2D *GetMergedMaskTexture() const;
    const String &GetName() const;
    bool IsValid() const;

    // IReflectable
    virtual void Reflect() override;

private:
    bool m_isValid = false;
    Array<EffectLayerMask *> m_masks;
    MeshRenderer *p_meshRenderer = nullptr;

    Color m_color = Color::Red();
    float m_height = 0.0f;
    float m_roughness = 0.0f;
    float m_metalness = 0.0f;
    BlendMode m_blendMode = BlendMode::ADD;

    VBO *m_positionsVBO = nullptr;
    VBO *m_normalsVBO = nullptr;
    Framebuffer *m_framebuffer = nullptr;
    bool m_visible = true;
    String m_name = "Layer";

    AH<Mesh> m_textureMesh;
    AH<ShaderProgram> m_generateEffectTextureSP;
    AH<ShaderProgram> m_growTextureBordersSP;

    AH<Texture2D> m_effectColorTexture;
    AH<Texture2D> m_effectMiscTexture;
    AH<Texture2D> m_mergedMaskTexture;
    AH<Texture2D> m_growAuxiliarTexture;

    void GrowTextureBorders(Texture2D *texture);
};

#endif  // EFFECTLAYER_H
