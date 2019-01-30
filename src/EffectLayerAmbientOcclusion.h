#ifndef EFFECTLAYERAMBIENTOCCLUSION_H
#define EFFECTLAYERAMBIENTOCCLUSION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationCPU.h"

using namespace Bang;

class MeshRaycaster;

class EffectLayerAmbientOcclusion : public EffectLayerImplementationCPU
{
public:
    SERIALIZABLE(EffectLayerAmbientOcclusion)

    EffectLayerAmbientOcclusion();
    virtual ~EffectLayerAmbientOcclusion() override;

    // EffectLayerImplementationCPU
    virtual void GenerateEffectImage(Image *effectImage) override;

    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;

private:
    MeshRaycaster *m_meshRaycaster = nullptr;
};

#endif  // EFFECTLAYERAMBIENTOCCLUSION_H
