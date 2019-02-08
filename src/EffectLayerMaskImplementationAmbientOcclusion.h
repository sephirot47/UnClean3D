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

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const override;

private:
    const uint PositionsTextureSize = 1024;
    bool m_generatedTextureArrays = false;

    GLSLArrayOfArrays m_trianglePositionsGLSLArray;
    GLSLArrayOfArrays m_uniformGridGLSLArray;

    void FillTrianglePositionsGLSLArray(MeshRenderer *mr);
    void FillMeshUniformGridGLSLArray();
    const MeshUniformGrid &GetMeshUniformGrid() const;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H
