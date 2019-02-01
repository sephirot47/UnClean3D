#ifndef EFFECTLAYERMASKIMPLEMENTATIONBRUSH_H
#define EFFECTLAYERMASKIMPLEMENTATIONBRUSH_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"

using namespace Bang;

class EffectLayerMaskImplementationBrush
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationBrush)

    EffectLayerMaskImplementationBrush();
    virtual ~EffectLayerMaskImplementationBrush() override;

    virtual void ReloadShaders() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayer
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    void Update() override;

    void PaintMaskBrush();

    ControlPanel *GetControlPanel() const;

private:
    AH<ShaderProgram> m_paintMaskBrushSP;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONBRUSH_H
