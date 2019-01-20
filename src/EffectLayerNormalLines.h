#ifndef EFFECTLAYERNORMALLINES_H
#define EFFECTLAYERNORMALLINES_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementation.h"

using namespace Bang;

class EffectLayerNormalLines : public EffectLayerImplementation
{
public:
    EffectLayerNormalLines();
    virtual ~EffectLayerNormalLines() override;

protected:
    // EffectLayer
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;
};

#endif  // EFFECTLAYERNORMALLINES_H
