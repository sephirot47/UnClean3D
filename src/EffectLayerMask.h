#ifndef EFFECTLAYERMASK_H
#define EFFECTLAYERMASK_H

#include "Bang/Bang.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class Framebuffer;
};

using namespace Bang;

class EffectLayer;
class ControlPanel;
class EffectLayerMaskImplementation;
class EffectLayerMask : public Serializable
{
public:
    SERIALIZABLE(EffectLayerMask);

    enum class BlendMode
    {
        IGNORE = 0,
        ADD,
        SUBTRACT,
        MULTIPLY
    };

    enum class Type
    {
        SIMPLEX_NOISE,
        NORMAL,
        AMBIENT_OCCLUSION,
        WHITE_NOISE,
        POLKA_DOTS_NOISE,
        CELLS_NOISE,
        CRACKS_NOISE,
        SCRATCHES_NOISE,
        EDGES,
        BRUSH,
        BLUR,
        PEELING
    };

    EffectLayerMask();
    virtual ~EffectLayerMask() override;

    virtual void Update();
    void ReloadShaders();
    void SetType(EffectLayerMask::Type type);
    void SetBlendMode(EffectLayerMask::BlendMode blendMode);
    void SetEffectLayer(EffectLayer *effectLayer);
    void SetName(const String &name);
    void GenerateMask();
    void ClearMask();
    void FillMask();
    void SetVisible(bool visible);
    void Invalidate(bool recursiveDown = false);

    const String &GetName() const;
    EffectLayerMask::BlendMode GetBlendMode() const;
    EffectLayerMask::Type GetType() const;
    Texture2D *GetMaskTexture() const;
    EffectLayer *GetEffectLayer() const;
    EffectLayerMaskImplementation *GetImplementation() const;
    bool CanGenerateEffectMaskTextureInRealTime() const;
    ControlPanel *GetControlPanel() const;
    bool GetVisible() const;
    bool IsValid() const;

    // Serializable
    void ImportMeta(const MetaNode &meta) override;
    void ExportMeta(MetaNode *meta) const override;
    void ImportMetaForSave(const MetaNode &meta);
    void ExportMetaForSave(MetaNode *meta);

    // IReflectable
    virtual void Reflect() override;

private:
    String m_name = "LayerMask";
    bool m_visible = true;
    EffectLayerMask::BlendMode m_blendMode = EffectLayerMask::BlendMode::ADD;
    EffectLayerMaskImplementation *m_implementation = nullptr;
    EffectLayerMask::Type m_type = Undef<EffectLayerMask::Type>();
    EffectLayer *p_effectLayer = nullptr;

    Framebuffer *m_framebuffer = nullptr;
    AH<Texture2D> m_maskTexture;
};

#endif  // EFFECTLAYERMASK_H
