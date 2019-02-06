#include "EffectLayerMaskImplementationBlur.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
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

    m_blurShaderProgram.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "GenerateEffectMaskTextureBlur.bushader")));
}

EffectLayerMaskImplementationBlur::~EffectLayerMaskImplementationBlur()
{
}

void EffectLayerMaskImplementationBlur::SetBlurRadius(int blurRadius)
{
    if (blurRadius != GetBlurRadius())
    {
        m_blurRadius = blurRadius;
        Invalidate();
    }
}

int EffectLayerMaskImplementationBlur::GetBlurRadius() const
{
    return m_blurRadius;
}

void EffectLayerMaskImplementationBlur::Reflect()
{
    EffectLayerMaskImplementation::Reflect();

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationBlur,
                                   "Blur radius",
                                   SetBlurRadius,
                                   GetBlurRadius,
                                   BANG_REFLECT_HINT_SLIDER(0.0f, 10.0f));
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

    if (GetEffectLayerMask()->GetVisible() && GetBlurRadius() > 0)
    {
        m_blurTexture0.Get()->Resize(mergedMaskTextureUntilNow->GetSize());

        m_framebuffer->Bind();
        m_framebuffer->SetAttachmentTexture(m_blurTexture0.Get(),
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        ShaderProgram *sp = m_blurShaderProgram.Get();
        sp->Bind();
        sp->SetTexture2D("TextureToBlur", mergedMaskTextureUntilNow);

        // ge->RenderTexture(m_);
        ge->RenderViewportPlane();
        ge->CopyTexture(m_blurTexture0.Get(), mergedMaskTextureUntilNow);

        /*
        ge->BlurTexture(mergedMaskTextureUntilNow,
                        m_blurTexture0.Get(),
                        m_blurTexture1.Get(),
                        GetBlurRadius(),
                        BlurType::GAUSSIAN);
        ge->CopyTexture(m_blurTexture1.Get(), mergedMaskTextureUntilNow);
        */
    }
    else
    {
        ge->CopyTexture(mergedMaskTextureUntilNow, m_blurTexture1.Get());
    }
}
