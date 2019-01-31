#ifndef EFFECTLAYERIMPLEMENTATIONCPU_H
#define EFFECTLAYERIMPLEMENTATIONCPU_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementation.h"

using namespace Bang;

class EffectLayerImplementationCPU : public EffectLayerImplementation
{
public:
    SERIALIZABLE_ABSTRACT(EffectLayerImplementationCPU)

    EffectLayerImplementationCPU();
    virtual ~EffectLayerImplementationCPU() override;

    // EffectLayerImplementation
    virtual void Init() override;
    void GenerateEffectTexture(Texture2D *effectTexture,
                               MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectTextureInRealTime() const override;

    // IReflectable
    virtual void Reflect() override;

protected:
    virtual void GenerateEffectImage(Image *effectImage,
                                     MeshRenderer *meshRend) = 0;

private:
    Image m_image;
};

#endif  // EFFECTLAYERIMPLEMENTATIONCPU_H
