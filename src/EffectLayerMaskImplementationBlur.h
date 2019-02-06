#ifndef EFFECTLAYERMASKIMPLEMENTATIONBLUR_H
#define EFFECTLAYERMASKIMPLEMENTATIONBLUR_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"

using namespace Bang;

class EffectLayerMaskImplementationBlur
    : public EffectLayerMaskImplementationGPU
{
    SERIALIZABLE(EffectLayerMaskImplementationBlur)

public:
    EffectLayerMaskImplementationBlur();
    virtual ~EffectLayerMaskImplementationBlur() override;

    void SetBlurRadius(int blurRadius);

    int GetBlurRadius() const;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const override;
    virtual Texture2D *GetMaskTextureToSee() const override;
    virtual bool CompositeThisMask() const override;
    virtual void GenerateEffectMaskTextureOnCompositeAfter(
        Texture2D *mergedMaskTextureUntilNow,
        MeshRenderer *meshRend) override;

private:
    int m_blurRadius = 3;

    AH<Texture2D> m_blurTexture0;
    AH<Texture2D> m_blurTexture1;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONBLUR_H
