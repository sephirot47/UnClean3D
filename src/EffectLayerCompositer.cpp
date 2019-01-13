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

    m_auxiliarTexture = Assets::Create<Texture2D>();
    m_compositeTexture = Assets::Create<Texture2D>();

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

Texture2D *EffectLayerCompositer::CompositeLayers(
    Texture2D *albedoReadTexture,
    const Array<EffectLayer *> &effectLayers)
{
    ControlPanel *controlPanel = MainScene::GetInstance()->GetControlPanel();

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::VIEWPORT);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);

    // Set viewport
    GL::SetViewport(
        0, 0, albedoReadTexture->GetWidth(), albedoReadTexture->GetHeight());

    GL::Disable(GL::Enablable::BLEND);

    m_auxiliarTexture.Get()->Resize(albedoReadTexture->GetSize());
    m_compositeTexture.Get()->Resize(albedoReadTexture->GetSize());

    // Bind framebuffer and render to texture
    m_framebuffer->Bind();
    Texture2D *drawTexture = m_compositeTexture.Get();
    Texture2D *readTexture = albedoReadTexture;

    m_framebuffer->SetAttachmentTexture(drawTexture, GL::Attachment::COLOR0);
    m_framebuffer->SetAllDrawBuffers();

    // Bind ShaderProgram and set uniforms
    ShaderProgram *sp = m_compositeLayersSP.Get();
    sp->Bind();

    for (uint i = 0; i < effectLayers.Size(); ++i)
    {
        if (!controlPanel->IsVisibleUIEffectLayer(i))
        {
            continue;
        }

        m_framebuffer->SetAttachmentTexture(drawTexture,
                                            GL::Attachment::COLOR0);

        // Set uniforms
        {
            EffectLayer *effectLayer = effectLayers[i];
            sp->SetInt("EffectLayerBlendMode",
                       effectLayer->GetImplementation()->GetBlendMode());
            sp->SetTexture2D("OriginalColorTexture", albedoReadTexture);
            sp->SetTexture2D("PreviousColorTexture", readTexture);
            sp->SetTexture2D("EffectLayerTexture",
                             effectLayer->GetEffectTexture());
        }

        GEngine::GetInstance()->RenderViewportPlane();

        std::swap(drawTexture, readTexture);
        if (i == 0)
        {
            drawTexture = m_auxiliarTexture.Get();
        }
    }

    sp->UnBind();
    m_framebuffer->UnBind();

    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);

    return readTexture;
}
