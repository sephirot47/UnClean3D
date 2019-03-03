#ifndef EFFECTLAYERMASKIMPLEMENTATIONNORMAL_H
#define EFFECTLAYERMASKIMPLEMENTATIONNORMAL_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"

using namespace Bang;

class EffectLayerMaskImplementationNormal
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationNormal)

    EffectLayerMaskImplementationNormal();
    virtual ~EffectLayerMaskImplementationNormal() override;

    // Serializable
    virtual void Reflect() override;

    void SetIntensity(float intensity);
    void SetFadeout(float fadeout);
    void SetNormal(const Vector3 &normal);

    float GetFadeout() const;
    float GetIntensity() const;
    const Vector3 &GetNormal() const;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;

private:
    Vector3 m_normal = Vector3::Up();
    float m_intensity = 1.0f;
    float m_fadeout = 0.4f;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONNORMAL_H
