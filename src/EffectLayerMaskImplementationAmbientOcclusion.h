#ifndef EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H
#define EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"
#include "GLSLArrayOfArrays.h"
#include "MeshUniformGrid.h"

using namespace Bang;

class EffectLayerMaskImplementationAmbientOcclusion
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationAmbientOcclusion)

    EffectLayerMaskImplementationAmbientOcclusion();
    virtual ~EffectLayerMaskImplementationAmbientOcclusion() override;

    void SetNumRays(int numRays);
    void SetMaxDistance(float maxDistance);

    int GetNumRays() const;
    float GetMaxDistance() const;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    void GenerateEffectMaskTexture(Texture2D *maskTexture,
                                   MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const override;

private:
    int m_numRays = 15;
    float m_maxDistance = 0.05f;
    bool m_generatedTextureArrays = false;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H
