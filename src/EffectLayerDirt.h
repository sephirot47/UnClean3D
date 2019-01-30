#ifndef EFFECTLAYERDIRT_H
#define EFFECTLAYERDIRT_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationGPU.h"

using namespace Bang;

class EffectLayerDirt : public EffectLayerImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerDirt)

    EffectLayerDirt();
    virtual ~EffectLayerDirt() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayer
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;

private:
    float m_seed = 0.0f;
    float m_stainsSize = 4.0f;
    float m_grain = 2.5f;
    float m_amplitude = 1.0f;
    Color m_outerColor = Color::Red();
    Color m_innerColor = Color::Black();
};

#endif  // EFFECTLAYERDIRT_H
