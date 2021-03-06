#include "EffectLayerCompositer.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Texture2D.h"

#include "ControlPanel.h"
#include "EffectLayer.h"
#include "EffectLayerMask.h"
#include "View3DScene.h"
#include "EffectLayerMaskImplementation.h"
#include "MainScene.h"
#include "PullPush.h"

using namespace Bang;

EffectLayerCompositer::EffectLayerCompositer()
{
    m_framebuffer = new Framebuffer();
    m_pullPush = new PullPush();

    m_albedoPingPongTexture0 = Assets::Create<Texture2D>();
    m_albedoPingPongTexture1 = Assets::Create<Texture2D>();
    m_heightPingPongTexture0 = Assets::Create<Texture2D>();
    m_heightPingPongTexture1 = Assets::Create<Texture2D>();
    p_finalNormalTexture = Assets::Create<Texture2D>();
    m_roughnessPingPongTexture0 = Assets::Create<Texture2D>();
    m_roughnessPingPongTexture1 = Assets::Create<Texture2D>();
    m_metalnessPingPongTexture0 = Assets::Create<Texture2D>();
    m_metalnessPingPongTexture1 = Assets::Create<Texture2D>();

    m_albedoPingPongTexture0.Get()->SetFormat(GL::ColorFormat::RGBA16F);
    m_albedoPingPongTexture1.Get()->SetFormat(GL::ColorFormat::RGBA16F);
    m_heightPingPongTexture0.Get()->SetFormat(GL::ColorFormat::RGBA32F);
    m_heightPingPongTexture1.Get()->SetFormat(GL::ColorFormat::RGBA32F);
    p_finalNormalTexture.Get()->SetFormat(GL::ColorFormat::RGBA32F);
    m_roughnessPingPongTexture0.Get()->SetFormat(GL::ColorFormat::RGBA16F);
    m_roughnessPingPongTexture1.Get()->SetFormat(GL::ColorFormat::RGBA16F);
    m_metalnessPingPongTexture0.Get()->SetFormat(GL::ColorFormat::RGBA16F);
    m_metalnessPingPongTexture1.Get()->SetFormat(GL::ColorFormat::RGBA16F);

    m_compositeLayersSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "CompositeLayers.bushader")));

    m_heightfieldToNormalTextureSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "HeightfieldToNormalTexture.bushader")));
}

EffectLayerCompositer::~EffectLayerCompositer()
{
    delete m_framebuffer;
    delete m_pullPush;
}

void EffectLayerCompositer::ReloadShaders()
{
    m_pullPush->ReloadShaders();
    m_compositeLayersSP.Get()->ReImport();
    m_heightfieldToNormalTextureSP.Get()->ReImport();
}

Texture2D *EffectLayerCompositer::GetFinalAlbedoTexture() const
{
    return p_finalAlbedoTexture.Get();
}

Texture2D *EffectLayerCompositer::GetFinalNormalTexture() const
{
    return p_finalNormalTexture.Get();
}

Texture2D *EffectLayerCompositer::GetFinalHeightTexture() const
{
    return p_finalHeightTexture.Get();
}

Texture2D *EffectLayerCompositer::GetFinalRoughnessTexture() const
{
    return p_finalRoughnessTexture.Get();
}

Texture2D *EffectLayerCompositer::GetFinalMetalnessTexture() const
{
    return p_finalMetalnessTexture.Get();
}

void EffectLayerCompositer::CompositeLayers(
    const Array<EffectLayer *> &effectLayers,
    Texture2D *albedoOriginalTex,
    Texture2D *normalOriginalTex,
    Texture2D *roughnessOriginalTex,
    Texture2D *metalnessOriginalTex)
{
    ControlPanel *controlPanel = MainScene::GetInstance()->GetControlPanel();

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::VIEWPORT);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);

    const Vector2i texSize = controlPanel->GetTextureSize();
    GL::SetViewport(0, 0, texSize.x, texSize.y);

    m_framebuffer->Bind();

    m_albedoPingPongTexture0.Get()->Resize(texSize);
    m_albedoPingPongTexture1.Get()->Resize(texSize);
    m_heightPingPongTexture0.Get()->Resize(texSize);
    m_heightPingPongTexture1.Get()->Resize(texSize);
    p_finalNormalTexture.Get()->Resize(texSize);
    m_roughnessPingPongTexture0.Get()->Resize(texSize);
    m_roughnessPingPongTexture1.Get()->Resize(texSize);
    m_metalnessPingPongTexture0.Get()->Resize(texSize);
    m_metalnessPingPongTexture1.Get()->Resize(texSize);

    Texture2D *albedoDrawTex = m_albedoPingPongTexture0.Get();
    Texture2D *albedoReadTex = m_albedoPingPongTexture1.Get();
    Texture2D *heightDrawTex = m_heightPingPongTexture0.Get();
    Texture2D *heightReadTex = m_heightPingPongTexture1.Get();
    Texture2D *roughnessDrawTex = m_roughnessPingPongTexture0.Get();
    Texture2D *roughnessReadTex = m_roughnessPingPongTexture1.Get();
    Texture2D *metalnessDrawTex = m_metalnessPingPongTexture0.Get();
    Texture2D *metalnessReadTex = m_metalnessPingPongTexture1.Get();

    // Initialize read textures
    {
        // Clear height
        GL::Disable(GL::Enablable::BLEND);
        m_framebuffer->SetAttachmentTexture(heightReadTex,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});
        GL::ClearColorBuffer(Color(0.5f));

        // Copy albedo
        GEngine::GetInstance()->CopyTexture(albedoOriginalTex, albedoReadTex);

        // Add base roughness and metalness on top of original textures
        GL::Enable(GL::Enablable::BLEND);
        {
            GL::BlendFunc(GL::BlendFactor::ONE, GL::BlendFactor::ONE);

            // Roughness
            {
                m_framebuffer->SetAttachmentTexture(roughnessReadTex,
                                                    GL::Attachment::COLOR0);
                m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});
                GL::ClearColorBuffer(Color::One() *
                                     controlPanel->GetBaseRoughness());
            }

            // Metalness
            {
                m_framebuffer->SetAttachmentTexture(metalnessReadTex,
                                                    GL::Attachment::COLOR0);
                m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});
                GL::ClearColorBuffer(Color::One() *
                                     controlPanel->GetBaseMetalness());
            }
        }
        GL::Disable(GL::Enablable::BLEND);
    }

    // Bind ShaderProgram and set uniforms
    ShaderProgram *sp = m_compositeLayersSP.Get();
    sp->Bind();
    for (int i = effectLayers.Size() - 1; i >= 0; --i)
    {
        EffectLayer *effectLayer = effectLayers[i];
        if (!effectLayer->GetVisible() || effectLayer->GetMasks().Size() == 0)
        {
            continue;
        }

        m_framebuffer->SetAttachmentTexture(albedoDrawTex,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetAttachmentTexture(heightDrawTex,
                                            GL::Attachment::COLOR2);
        m_framebuffer->SetAttachmentTexture(roughnessDrawTex,
                                            GL::Attachment::COLOR3);
        m_framebuffer->SetAttachmentTexture(metalnessDrawTex,
                                            GL::Attachment::COLOR4);
        m_framebuffer->SetAllDrawBuffers();

        // Set uniforms
        {
            sp->SetTexture2D("EffectLayerColorTexture",
                             effectLayer->GetEffectColorTexture());
            sp->SetTexture2D("EffectLayerMiscTexture",
                             effectLayer->GetEffectMiscTexture());
            sp->SetTexture2D("EffectLayerMaskTexture",
                             effectLayer->GetMergedMaskTexture());

            sp->SetTexture2D("PreviousAlbedoTexture", albedoReadTex);
            sp->SetTexture2D("PreviousHeightTexture", heightReadTex);
            sp->SetTexture2D("PreviousRoughnessTexture", roughnessReadTex);
            sp->SetTexture2D("PreviousMetalnessTexture", metalnessReadTex);

            sp->SetInt("ColorBlendMode",
                       SCAST<int>(effectLayer->GetColorBlendMode()));
            sp->SetInt("HeightBlendMode",
                       SCAST<int>(effectLayer->GetHeightBlendMode()));
            sp->SetInt("RoughnessBlendMode",
                       SCAST<int>(effectLayer->GetRoughnessBlendMode()));
            sp->SetInt("MetalnessBlendMode",
                       SCAST<int>(effectLayer->GetMetalnessBlendMode()));
        }

        GEngine::GetInstance()->RenderViewportPlane();

        std::swap(albedoDrawTex, albedoReadTex);
        std::swap(heightDrawTex, heightReadTex);
        std::swap(roughnessDrawTex, roughnessReadTex);
        std::swap(metalnessDrawTex, metalnessReadTex);
    }

    p_finalAlbedoTexture.Set(albedoReadTex);
    p_finalHeightTexture.Set(heightReadTex);
    p_finalRoughnessTexture.Set(roughnessReadTex);
    p_finalMetalnessTexture.Set(metalnessReadTex);

    m_pullPush->PullPushTexture(GetFinalAlbedoTexture());
    m_pullPush->PullPushTexture(GetFinalHeightTexture());
    m_pullPush->PullPushTexture(GetFinalMetalnessTexture());
    m_pullPush->PullPushTexture(GetFinalRoughnessTexture());

    sp = m_heightfieldToNormalTextureSP.Get();
    {
        sp->Bind();
        GL::SetViewport(0, 0, p_finalNormalTexture.Get()->GetWidth(),
                        p_finalNormalTexture.Get()->GetHeight());
        m_framebuffer->SetAttachmentTexture(p_finalNormalTexture.Get(),
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});
        sp->SetTexture2D("HeightfieldTexture", GetFinalHeightTexture());
        sp->SetFloat("Smoothness",
                     MainScene::GetInstance()->GetControlPanel()->
                     GetNormalMapSmoothness());
        GEngine::GetInstance()->RenderViewportPlane();
    }

    m_pullPush->PullPushTexture(GetFinalNormalTexture());

    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);

}
