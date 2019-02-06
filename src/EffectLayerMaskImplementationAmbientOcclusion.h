#ifndef EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H
#define EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"
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
    const uint UniformGridTextureSize = 2048;
    bool m_generatedTextures = false;

    AH<Texture2D> m_trianglePositionsTexture;
    AH<Texture2D> m_uniformGridTexture;

    Texture2D *CreateTrianglePositionsTexture(MeshRenderer *mr);
    Texture2D *CreateMeshUniformGridTexture(MeshRenderer *mr);
    const MeshUniformGrid &GetMeshUniformGrid() const;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONAMBIENTOCCLUSION_H
