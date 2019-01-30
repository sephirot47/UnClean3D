#ifndef EFFECTLAYERFRACTALBUMPS_H
#define EFFECTLAYERFRACTALBUMPS_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationGPU.h"

using namespace Bang;

class EffectLayerFractalBumps : public EffectLayerImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerFractalBumps)

    EffectLayerFractalBumps();
    virtual ~EffectLayerFractalBumps() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayer
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;

private:
    float m_height = 0.5f;
    float m_frequency = 20.0f;
};

#endif  // EFFECTLAYERFRACTALBUMPS_H
