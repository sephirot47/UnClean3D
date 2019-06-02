#pragma once

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"

using namespace Bang;

class EffectLayerMaskImplementationCracksNoise
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationCracksNoise)

    EffectLayerMaskImplementationCracksNoise();
    virtual ~EffectLayerMaskImplementationCracksNoise() override;

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
    float m_seed = 1.0f;
    float m_size = 10.0f;
    float m_fade = 5.0f;
    float m_intensity = 1.0f;
    float m_width = 1.0f;
};
