#ifndef EFFECTLAYERNORMALLINES_H
#define EFFECTLAYERNORMALLINES_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementation.h"

using namespace Bang;

class EffectLayerNormalLines : public EffectLayerImplementation
{
public:
    SERIALIZABLE(EffectLayerNormalLines)

    EffectLayerNormalLines();
    virtual ~EffectLayerNormalLines() override;

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
};

#endif  // EFFECTLAYERNORMALLINES_H
