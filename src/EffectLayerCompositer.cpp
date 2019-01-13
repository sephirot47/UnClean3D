#include "EffectLayerCompositer.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Texture2D.h"

#include "ControlPanel.h"
#include "EffectLayer.h"
#include "MainScene.h"

using namespace Bang;

EffectLayerCompositer::EffectLayerCompositer()
{
    m_framebuffer = new Framebuffer();

    m_albedoPingPongTexture0 = Assets::Create<Texture2D>();
    m_albedoPingPongTexture1 = Assets::Create<Texture2D>();
    m_normalPingPongTexture0 = Assets::Create<Texture2D>();
    m_normalPingPongTexture1 = Assets::Create<Texture2D>();
    m_roughnessPingPongTexture0 = Assets::Create<Texture2D>();
    m_roughnessPingPongTexture1 = Assets::Create<Texture2D>();
    m_metalnessPingPongTexture0 = Assets::Create<Texture2D>();
    m_metalnessPingPongTexture1 = Assets::Create<Texture2D>();

    m_compositeLayersSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "CompositeLayers.bushader")));
}

EffectLayerCompositer::~EffectLayerCompositer()
{
    delete m_framebuffer;
}

void EffectLayerCompositer::ReloadShaders()
{
    m_compositeLayersSP.Get()->ReImport();
}

void EffectLayerCompositer::CompositeLayers(
    const Array<EffectLayer *> &effectLayers,
    Texture2D *albedoOriginalTex,
    Texture2D *normalOriginalTex,
    Texture2D *roughnessOriginalTex,
    Texture2D *metalnessOriginalTex,
    Texture2D **outAlbedoTexture,
    Texture2D **outNormalTexture,
    Texture2D **outRoughnessTexture,
    Texture2D **outMetalnessTexture)
{
    ControlPanel *controlPanel = MainScene::GetInstance()->GetControlPanel();

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::VIEWPORT);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);

    // Set viewport
    GL::SetViewport(
        0, 0, albedoOriginalTex->GetWidth(), albedoOriginalTex->GetHeight());

    GL::Disable(GL::Enablable::BLEND);

    m_albedoPingPongTexture0.Get()->Resize(albedoOriginalTex->GetSize());
    m_albedoPingPongTexture1.Get()->Resize(albedoOriginalTex->GetSize());
    m_normalPingPongTexture0.Get()->Resize(normalOriginalTex->GetSize());
    m_normalPingPongTexture1.Get()->Resize(normalOriginalTex->GetSize());
    m_roughnessPingPongTexture0.Get()->Resize(roughnessOriginalTex->GetSize());
    m_roughnessPingPongTexture1.Get()->Resize(roughnessOriginalTex->GetSize());
    m_metalnessPingPongTexture0.Get()->Resize(metalnessOriginalTex->GetSize());
    m_metalnessPingPongTexture1.Get()->Resize(metalnessOriginalTex->GetSize());

    // Bind framebuffer and render to texture
    m_framebuffer->Bind();
    Texture2D *albedoDrawTex = m_albedoPingPongTexture0.Get();
    Texture2D *albedoReadTex = albedoOriginalTex;
    Texture2D *normalDrawTex = m_normalPingPongTexture0.Get();
    Texture2D *normalReadTex = normalOriginalTex;
    Texture2D *roughnessDrawTex = m_roughnessPingPongTexture0.Get();
    Texture2D *roughnessReadTex = roughnessOriginalTex;
    Texture2D *metalnessDrawTex = m_metalnessPingPongTexture0.Get();
    Texture2D *metalnessReadTex = metalnessOriginalTex;

    // Bind ShaderProgram and set uniforms
    ShaderProgram *sp = m_compositeLayersSP.Get();
    sp->Bind();

    for (uint i = 0; i < effectLayers.Size(); ++i)
    {
        if (!controlPanel->IsVisibleUIEffectLayer(i))
        {
            continue;
        }

        m_framebuffer->SetAttachmentTexture(albedoDrawTex,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetAttachmentTexture(normalDrawTex,
                                            GL::Attachment::COLOR1);
        m_framebuffer->SetAttachmentTexture(roughnessDrawTex,
                                            GL::Attachment::COLOR2);
        m_framebuffer->SetAttachmentTexture(metalnessDrawTex,
                                            GL::Attachment::COLOR3);
        m_framebuffer->SetAllDrawBuffers();

        // Set uniforms
        {
            EffectLayer *effectLayer = effectLayers[i];
            sp->SetInt("EffectLayerBlendMode",
                       effectLayer->GetImplementation()->GetBlendMode());
            sp->SetTexture2D("EffectLayerTexture",
                             effectLayer->GetEffectTexture());

            sp->SetTexture2D("OriginalAlbedoTexture", albedoOriginalTex);
            sp->SetTexture2D("PreviousAlbedoTexture", albedoReadTex);
            sp->SetTexture2D("OriginalNormalTexture", normalOriginalTex);
            sp->SetTexture2D("PreviousNormalTexture", normalReadTex);
            sp->SetTexture2D("OriginalRoughnessTexture", roughnessOriginalTex);
            sp->SetTexture2D("PreviousRoughnessTexture", roughnessReadTex);
            sp->SetTexture2D("OriginalMetalnessTexture", metalnessOriginalTex);
            sp->SetTexture2D("PreviousMetalnessTexture", metalnessReadTex);
        }

        GEngine::GetInstance()->RenderViewportPlane();

        std::swap(albedoDrawTex, albedoReadTex);
        std::swap(normalDrawTex, normalReadTex);
        std::swap(roughnessDrawTex, roughnessReadTex);
        std::swap(metalnessDrawTex, metalnessReadTex);

        if (i == 0)
        {
            albedoDrawTex = m_albedoPingPongTexture1.Get();
            normalDrawTex = m_normalPingPongTexture1.Get();
            roughnessDrawTex = m_roughnessPingPongTexture1.Get();
            metalnessDrawTex = m_metalnessPingPongTexture1.Get();
        }
    }

    sp->UnBind();
    m_framebuffer->UnBind();

    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);

    *outAlbedoTexture = albedoReadTex;
    *outNormalTexture = normalOriginalTex;
    *outRoughnessTexture = roughnessOriginalTex;
    *outMetalnessTexture = metalnessOriginalTex;
}
