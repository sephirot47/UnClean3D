#ifndef EFFECTLAYERMASKIMPLEMENTATIONBLUR_H
#define EFFECTLAYERMASKIMPLEMENTATIONBLUR_H

#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

#include "EffectLayerMaskImplementationGPU.h"
#include "GLSLArrayOfArrays.h"

using namespace Bang;

class EffectLayerMaskImplementationBlur
    : public EffectLayerMaskImplementationGPU
{
    SERIALIZABLE(EffectLayerMaskImplementationBlur)

public:
    enum BlurMode
    {
        WORLD,
        TEXTURE
    };

    EffectLayerMaskImplementationBlur();
    virtual ~EffectLayerMaskImplementationBlur() override;

    void SetBlurRadius(int blurRadius);
    void SetBlurMode(BlurMode blurMode);
    void SetBlurStepResolution(float blurStepResolution);

    int GetBlurRadius() const;
    BlurMode GetBlurMode() const;
    float GetBlurStepResolution() const;

    // Serializable
    virtual void Reflect() override;

protected:
    // EffectLayerMaskImplementation
    virtual bool GetIsPostProcessEffectLayer() const override;
    virtual EffectLayerMask::Type GetEffectLayerMaskType() const override;
    virtual String GetTypeName() const override;
    virtual Path GetGenerateEffectTextureShaderProgramPath() const override;
    virtual void SetGenerateEffectUniforms(ShaderProgram *sp,
                                           MeshRenderer *meshRend) override;
    virtual void ReloadShaders() override;
    virtual bool CanGenerateEffectMaskTextureInRealTime() const override;
    virtual Texture2D *GetMaskTextureToSee() const override;
    virtual bool CompositeThisMask() const override;
    virtual void GenerateEffectMaskTextureOnCompositeAfter(
        Texture2D *mergedMaskTextureUntilNow,
        MeshRenderer *meshRend) override;

private:
    int m_blurRadius = 3;
    BlurMode m_blurMode = BlurMode::TEXTURE;
    float m_blurStepResolution = 30.0f;

    bool m_generatedTextureArrays = false;
    GLSLArrayOfArrays m_triangleUvsGLSLArray;
    GLSLArrayOfArrays m_triangleNeighborhoodsGLSLArray;

    void FillGLSLArrays(MeshRenderer *mr);

    AH<ShaderProgram> m_blurShaderProgram;
    AH<Texture2D> m_blurTexture;
    AH<Texture2D> m_blurTextureAux;
};

#endif  // EFFECTLAYERMASKIMPLEMENTATIONBLUR_H
