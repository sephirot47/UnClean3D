#ifndef EFFECTLAYERMASKIMPLEMENTATIONGPU_H
#define EFFECTLAYERMASKIMPLEMENTATIONGPU_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementation.h"

namespace Bang
{
class Framebuffer;
};

using namespace Bang;

class EffectLayerMaskImplementationGPU : public EffectLayerMaskImplementation
{
public:
    SERIALIZABLE_ABSTRACT(EffectLayerMaskImplementationGPU)

    EffectLayerMaskImplementationGPU();
    virtual ~EffectLayerMaskImplementationGPU() override;

    // EffectLayerImplementation
    virtual void Init() override;
    virtual void ReloadShaders() override;
    virtual void GenerateEffectMaskTexture(Texture2D *maskTexture,
                                           MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const override;

protected:
    Framebuffer *m_framebuffer = nullptr;

    // EffectLayerImplementation
    virtual Path GetGenerateEffectTextureShaderProgramPath() const = 0;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend);
    ShaderProgram *GetGenerateEffectTextureShaderProgram() const;

private:
    AH<ShaderProgram> m_generateEffectTextureSP;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONGPU_H
