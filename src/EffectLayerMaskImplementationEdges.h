#ifndef EFFECTLAYERMASKIMPLEMENTATIONEDGES_H
#define EFFECTLAYERMASKIMPLEMENTATIONEDGES_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"
#include "GLSLArrayOfArrays.h"
#include "MeshUniformGrid.h"

using namespace Bang;

class EffectLayerMaskImplementationEdges
    : public EffectLayerMaskImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerMaskImplementationEdges)

    EffectLayerMaskImplementationEdges();
    virtual ~EffectLayerMaskImplementationEdges() override;

    void SetNumRays(int numRays);
    void SetEdgeThreshold(float edgeThreshold);
    void SetEdgeAmplitude(float edgeAmplitude);

    int GetNumRays() const;
    float GetEdgeThreshold() const;
    float GetEdgeAmplitude() const;

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
    int m_numRays = 5;
    float m_edgeThreshold = 0.1f;
    float m_edgeAmplitude = 0.5f;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONEDGES_H
