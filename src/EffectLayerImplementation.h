#ifndef EFFECTLAYERIMPLEMENTATION_H
#define EFFECTLAYERIMPLEMENTATION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayer.h"

namespace Bang
{
class Texture2D;
};

using namespace Bang;

class EffectLayerImplementation : public Serializable,
                                  public EventEmitter<IEventsDestroy>
{
public:
    SERIALIZABLE_ABSTRACT(EffectLayerImplementation)

    EffectLayerImplementation();
    virtual ~EffectLayerImplementation() override;

    virtual void Init();
    virtual void ReloadShaders();
    virtual bool CanGenerateEffectTextureInRealTime() const = 0;

    virtual void GenerateEffectTexture(Texture2D *effectTexture,
                                       MeshRenderer *meshRend) = 0;
    virtual EffectLayer::Type GetEffectLayerType() const = 0;
    virtual String GetTypeName() const = 0;
    EffectLayer *GetEffectLayer() const;

    // Serializable
    virtual void Reflect() override;
    void Invalidate();

private:
    EffectLayer *p_effectLayer = nullptr;

    friend class EffectLayer;
};

#endif  // EFFECTLAYERIMPLEMENTATION_H
