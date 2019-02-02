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
        IGNORE,
        ADD,
        SUBTRACT,
        MULTIPLY
    };

    EffectLayer(MeshRenderer *mr = nullptr);
    virtual ~EffectLayer() override;

    virtual void GenerateEffectTexture();
    void MergeMasks();
    void ReloadShaders();

    void SetImplementation(EffectLayerMaskImplementation *impl);
    void SetVisible(bool visible);
    void SetName(const String &name);
    void SetColorBlendMode(EffectLayer::BlendMode blendMode);
    void SetHeightBlendMode(EffectLayer::BlendMode blendMode);
    void SetRoughnessBlendMode(EffectLayer::BlendMode blendMode);
    void SetMetalnessBlendMode(EffectLayer::BlendMode blendMode);
    void SetColor(const Color &color);
    void SetHeight(float height);
    void SetRoughness(float roughness);
    void SetMetalness(float metalness);

    EffectLayerMask *AddNewMask();
    void MoveMask(EffectLayerMask *effectLayerMask, uint newIndex);
    void RemoveMask(EffectLayerMask *mask);
    void Invalidate(bool recursiveDown = false);

    bool GetVisible() const;
    Mesh *GetMesh() const;
    const Color &GetColor() const;
    float GetHeight() const;
    float GetRoughness() const;
    float GetMetalness() const;
    EffectLayer::BlendMode GetColorBlendMode() const;
    EffectLayer::BlendMode GetHeightBlendMode() const;
    EffectLayer::BlendMode GetRoughnessBlendMode() const;
    EffectLayer::BlendMode GetMetalnessBlendMode() const;
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

    Color m_color = Color::Blue();
    float m_height = -1.0f;
    float m_roughness = 0.5f;
    float m_metalness = 0.0f;
    BlendMode m_colorBlendMode = BlendMode::MULTIPLY;
    BlendMode m_heightBlendMode = BlendMode::ADD;
    BlendMode m_roughnessBlendMode = BlendMode::MULTIPLY;
    BlendMode m_metalnessBlendMode = BlendMode::IGNORE;

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
