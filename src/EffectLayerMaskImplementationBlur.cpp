#include "EffectLayerMaskImplementationBlur.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Texture2D.h"

using namespace Bang;

EffectLayerMaskImplementationBlur::EffectLayerMaskImplementationBlur()
{
    m_blurTexture0 = Assets::Create<Texture2D>();
    m_blurTexture0.Get()->CreateEmpty(1, 1);
    m_blurTexture0.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_blurTexture1 = Assets::Create<Texture2D>();
    m_blurTexture1.Get()->CreateEmpty(1, 1);
    m_blurTexture1.Get()->SetFormat(GL::ColorFormat::RGBA8);
}

EffectLayerMaskImplementationBlur::~EffectLayerMaskImplementationBlur()
{
}

void EffectLayerMaskImplementationBlur::Reflect()
{
    EffectLayerMaskImplementation::Reflect();
}

EffectLayerMask::Type
EffectLayerMaskImplementationBlur::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::BLUR;
}

String EffectLayerMaskImplementationBlur::GetTypeName() const
{
    return "Blur";
}

Path EffectLayerMaskImplementationBlur::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Path::Empty();
}

void EffectLayerMaskImplementationBlur::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *mr)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, mr);
}

bool EffectLayerMaskImplementationBlur::CanGenerateEffectMaskTextureInRealTime()
    const
{
    return true;
}

Texture2D *EffectLayerMaskImplementationBlur::GetMaskTextureToSee() const
{
    return m_blurTexture1.Get();
}

bool EffectLayerMaskImplementationBlur::CompositeThisMask() const
{
    return false;
}

void EffectLayerMaskImplementationBlur::
    GenerateEffectMaskTextureOnCompositeAfter(
        Texture2D *mergedMaskTextureUntilNow,
        MeshRenderer *)
{
    GEngine *ge = GEngine::GetInstance();

    if (GetEffectLayerMask()->GetVisible())
    {
        ge->BlurTexture(mergedMaskTextureUntilNow,
                        m_blurTexture0.Get(),
                        m_blurTexture1.Get(),
                        3.0f,
                        BlurType::KAWASE);
        ge->CopyTexture(m_blurTexture1.Get(), mergedMaskTextureUntilNow);
    }
    else
    {
        ge->CopyTexture(mergedMaskTextureUntilNow, m_blurTexture1.Get());
    }
}
