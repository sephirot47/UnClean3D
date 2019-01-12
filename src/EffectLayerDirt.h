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

class EffectLayerDirt : public EffectLayer
{
public:
    EffectLayerDirt();
    virtual ~EffectLayerDirt() override;

    // EffectLayer
    virtual String GetUniformName() const override;

protected:
    // EffectLayer
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp) override;

private:
};

#endif  // EFFECTLAYERDIRT_H
