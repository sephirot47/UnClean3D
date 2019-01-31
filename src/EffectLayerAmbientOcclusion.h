#ifndef EFFECTLAYERAMBIENTOCCLUSION_H
#define EFFECTLAYERAMBIENTOCCLUSION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationCPU.h"

using namespace Bang;

class EffectLayerAmbientOcclusion : public EffectLayerImplementationCPU
{
public:
    SERIALIZABLE(EffectLayerAmbientOcclusion)

    EffectLayerAmbientOcclusion();
    virtual ~EffectLayerAmbientOcclusion() override;

    // EffectLayerImplementationCPU
    virtual void GenerateEffectImage(Image *effectImage,
                                     MeshRenderer *meshRend) override;

    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
};

#endif  // EFFECTLAYERAMBIENTOCCLUSION_H
