#ifndef EFFECTLAYERMASK_H
#define EFFECTLAYERMASK_H

#include "Bang/Bang.h"
#include "Bang/Texture2D.h"
#include "BangEditor/BangEditor.h"

using namespace Bang;

class EffectLayer;
class EffectLayerMask : public Serializable
{
public:
    SERIALIZABLE(EffectLayerMask);

    EffectLayerMask();
    virtual ~EffectLayerMask() override;

    void SetEffectLayer(EffectLayer *effectLayer);
    void Clear();
    void Fill();

    Texture2D *GetMaskTexture() const;
    EffectLayer *GetEffectLayer() const;

    // IReflectable
    virtual void Reflect() override;

private:
    EffectLayer *p_effectLayer = nullptr;
    AH<Texture2D> m_maskTexture;
};

#endif  // EFFECTLAYERMASK_H
