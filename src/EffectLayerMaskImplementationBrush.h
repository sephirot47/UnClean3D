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

    float GetBrushSize() const;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    void Update() override;

    void PaintMaskBrush();

    ControlPanel *GetControlPanel() const;

private:
    float m_strength = 0.5f;
    float m_size = 50.0f;
    bool m_erasing = false;
    bool m_depthAware = true;

    AH<ShaderProgram> m_paintMaskBrushSP;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONBRUSH_H
