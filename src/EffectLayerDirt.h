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
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;

    const EffectLayerParameters &GetParameters() const;

protected:
    // EffectLayer
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;
};

#endif  // EFFECTLAYERDIRT_H
