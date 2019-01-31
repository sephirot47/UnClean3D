#ifndef EFFECTLAYERAMBIENTOCCLUSIONGPU_H
#define EFFECTLAYERAMBIENTOCCLUSIONGPU_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementationGPU.h"

using namespace Bang;

class EffectLayerAmbientOcclusionGPU : public EffectLayerImplementationGPU
{
public:
    SERIALIZABLE(EffectLayerAmbientOcclusionGPU)

    EffectLayerAmbientOcclusionGPU();
    virtual ~EffectLayerAmbientOcclusionGPU() override;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayer
    virtual EffectLayer::Type GetEffectLayerType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectTextureInRealTime() const override;

private:
    uint PositionsTextureSize = 1024;
    AH<Texture2D> m_trianglePositionsTexture;
};

#endif  // EFFECTLAYERAMBIENTOCCLUSIONGPU_H
