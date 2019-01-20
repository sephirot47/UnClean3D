#ifndef EFFECTLAYERDIRT_H
#define EFFECTLAYERDIRT_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementation.h"

using namespace Bang;

class EffectLayerDirt : public EffectLayerImplementation
{
public:
    SERIALIZABLE(EffectLayerDirt)

    EffectLayerDirt();
    virtual ~EffectLayerDirt() override;

protected:
    // EffectLayer
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;
};

#endif  // EFFECTLAYERDIRT_H
