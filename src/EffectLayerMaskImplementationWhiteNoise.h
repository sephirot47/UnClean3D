#ifndef EFFECTLAYERMASKIMPLEMENTATIONWHITENOISE_H
#define EFFECTLAYERMASKIMPLEMENTATIONWHITENOISE_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"

using namespace Bang;

class EffectLayerMaskImplementationWhiteNoise
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationWhiteNoise)

    EffectLayerMaskImplementationWhiteNoise();
    virtual ~EffectLayerMaskImplementationWhiteNoise() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;

private:
    float m_seed = 0.0f;
    float m_offset = 0.0f;
    float m_intensity = 1.0f;
    float m_frequency = 1.0f;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONWHITENOISE_H
