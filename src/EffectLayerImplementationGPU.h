#ifndef EFFECTLAYERIMPLEMENTATIONGPU_H
#define EFFECTLAYERIMPLEMENTATIONGPU_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerImplementation.h"

using namespace Bang;

class EffectLayerImplementationGPU : public EffectLayerImplementation
{
public:
    SERIALIZABLE_ABSTRACT(EffectLayerImplementationGPU)

    EffectLayerImplementationGPU();
    virtual ~EffectLayerImplementationGPU() override;

    // EffectLayerImplementation
    virtual void Init() override;
    virtual void ReloadShaders() override;
    void GenerateEffectTexture(Texture2D *effectTexture,
                               MeshRenderer *meshRend) override;
    virtual bool CanGenerateEffectTextureInRealTime() const override;

protected:
    // EffectLayerImplementation
    virtual Path GetGenerateEffectTextureShaderProgramPath() const = 0;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend);
    ShaderProgram *GetGenerateEffectTextureShaderProgram() const;

private:
    Framebuffer *m_framebuffer = nullptr;
    AH<ShaderProgram> m_generateEffectTextureSP;
};

#endif  // EFFECTLAYERIMPLEMENTATIONGPU_H
