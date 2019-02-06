#ifndef EFFECTLAYERMASKIMPLEMENTATIONFRACTAL_H
#define EFFECTLAYERMASKIMPLEMENTATIONFRACTAL_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"

using namespace Bang;

class EffectLayerMaskImplementationFractal
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationFractal)

    EffectLayerMaskImplementationFractal();
    virtual ~EffectLayerMaskImplementationFractal() override;

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
    float m_offset = 0.0f;
    float m_seed = 0.0f;
    float m_stainsSize = 4.0f;
    float m_grain = 2.5f;
    float m_amplitude = 1.0f;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONFRACTAL_H
