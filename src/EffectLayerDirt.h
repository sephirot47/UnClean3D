#ifndef EFFECTLAYERDIRT_H
#define EFFECTLAYERDIRT_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayer.h"

namespace Bang
{
class MeshRenderer;
}
using namespace Bang;

class EffectLayerDirt : public EffectLayerImplementation
{
public:
    EffectLayerDirt();
    virtual ~EffectLayerDirt() override;

    // EffectLayer
    virtual EffectLayer::BlendMode GetBlendMode() const override;

    const ControlPanel::Parameters &GetParameters() const;

protected:
    // EffectLayer
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;
};

#endif  // EFFECTLAYERDIRT_H
