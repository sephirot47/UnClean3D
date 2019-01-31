#ifndef EFFECTLAYERNORMALLINES_H
#define EFFECTLAYERNORMALLINES_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationGPU.h"

using namespace Bang;

class EffectLayerNormalLines : public EffectLayerImplementationGPU
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
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;

private:
    float m_height = 0.5f;
    float m_width = 0.5f;
    Axis3D m_axis = Axis3D::X;
};

#endif  // EFFECTLAYERNORMALLINES_H
