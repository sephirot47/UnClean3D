#ifndef EFFECTLAYERWAVEBUMPS_H
#define EFFECTLAYERWAVEBUMPS_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationGPU.h"

using namespace Bang;

class EffectLayerWaveBumps : public EffectLayerImplementationGPU
{
    SERIALIZABLE(EffectLayerWaveBumps)

public:
    EffectLayerWaveBumps();
    virtual ~EffectLayerWaveBumps() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayer
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;

private:
    const float FreqLimit = 50;
    float m_globalHeight = 1.0f;
    float m_frequencyX = 1.0f;
    float m_heightX = 0.5f;
    float m_frequencyY = 1.0f;
    float m_heightY = 0.5f;
    float m_frequencyZ = 1.0f;
    float m_heightZ = 0.5f;
};

#endif  // EFFECTLAYERWAVEBUMPS_H
