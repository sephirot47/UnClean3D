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
    virtual bool CanGenerateEffectMaskTextureInRealTime() const = 0;
    void SetEffectLayerMask(EffectLayerMask *effectLayerMask);
    virtual void GenerateEffectMaskTexture(Texture2D *effectTexture,
                                           MeshRenderer *meshRend) = 0;
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const = 0;

    virtual String GetTypeName() const = 0;
    EffectLayerMask *GetEffectLayerMask() const;

    // Serializable
    virtual void Reflect() override;
    void Invalidate();

private:
    EffectLayerMask *p_effectLayerMask = nullptr;

    friend class EffectLayer;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATION_H
