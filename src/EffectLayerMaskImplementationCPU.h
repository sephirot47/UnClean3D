#ifndef EFFECTLAYERMASKIMPLEMENTATIONCPU_H
#define EFFECTLAYERMASKIMPLEMENTATIONCPU_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementation.h"

using namespace Bang;

class EffectLayerMaskImplementationCPU : public EffectLayerMaskImplementation
{
public:
    SERIALIZABLE_ABSTRACT(EffectLayerMaskImplementationCPU)

    EffectLayerMaskImplementationCPU();
    virtual ~EffectLayerMaskImplementationCPU() override;

    // EffectLayerImplementation
    virtual void Init() override;
    void GenerateEffectMaskTexture(Texture2D *effectTexture,
                                   MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const override;

    // IReflectable
    virtual void Reflect() override;

protected:
    virtual void GenerateEffectMaskImage(Image *effectImage,
                                         MeshRenderer *meshRend) = 0;

private:
    Image m_image;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONCPU_H
