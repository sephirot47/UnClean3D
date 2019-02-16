#ifndef EFFECTLAYERMASKIMPLEMENTATION_H
#define EFFECTLAYERMASKIMPLEMENTATION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMask.h"

namespace Bang
{
class Texture2D;
class MeshRenderer;
};

using namespace Bang;

class EffectLayerMaskImplementation : public Serializable,
                                      public EventEmitter<IEventsDestroy>
{
public:
    SERIALIZABLE_ABSTRACT(EffectLayerMaskImplementation)

    EffectLayerMaskImplementation();
    virtual ~EffectLayerMaskImplementation() override;

    virtual void Init();
    virtual void ReloadShaders();
    virtual bool GetIsPostProcessEffectLayer() const;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const = 0;
    void SetEffectLayerMask(EffectLayerMask *effectLayerMask);
    virtual void GenerateEffectMaskTexture(Texture2D *effectTexture,
                                           MeshRenderer *meshRend) = 0;
    virtual void GenerateEffectMaskTextureOnCompositeBefore(
        Texture2D *mergedMaskTextureUntilNow,
        MeshRenderer *meshRend);
    virtual void GenerateEffectMaskTextureOnCompositeAfter(
        Texture2D *mergedMaskTextureUntilNow,
        MeshRenderer *meshRend);
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const = 0;
    virtual Texture2D *GetMaskTextureToSee() const;
    virtual bool CompositeThisMask() const;

    virtual String GetTypeName() const = 0;
    EffectLayerMask *GetEffectLayerMask() const;

    virtual void Update();
    bool IsValid() const;

    // Serializable
    virtual void Reflect() override;
    void Invalidate();

protected:
    bool m_isValid = false;

private:
    EffectLayerMask *p_effectLayerMask = nullptr;

    friend class EffectLayer;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATION_H
