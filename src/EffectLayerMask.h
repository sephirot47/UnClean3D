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

    enum class Type
    {
        FRACTAL,
        AMBIENT_OCCLUSION,
        BRUSH
    };

    EffectLayerMask();
    virtual ~EffectLayerMask() override;

    virtual void Update();
    void ReloadShaders();
    void SetType(EffectLayerMask::Type type);
    void SetEffectLayer(EffectLayer *effectLayer);
    void SetName(const String &name);
    void GenerateMask();
    void Clear();
    void Fill();
    void Invalidate();
    void PaintMaskBrush();

    const String &GetName() const;
    EffectLayerMask::Type GetType() const;
    Texture2D *GetMaskTexture() const;
    EffectLayer *GetEffectLayer() const;
    EffectLayerMaskImplementation *GetImplementation() const;
    ControlPanel *GetControlPanel() const;
    bool IsValid() const;

    // IReflectable
    virtual void Reflect() override;

private:
    String m_name = "LayerMask";
    EffectLayerMaskImplementation *m_implementation = nullptr;
    EffectLayerMask::Type m_type = Undef<EffectLayerMask::Type>();
    EffectLayer *p_effectLayer = nullptr;
    bool m_isValid = false;

    Framebuffer *m_framebuffer = nullptr;
    AH<Texture2D> m_maskTexture;
};

#endif  // EFFECTLAYERMASK_H
