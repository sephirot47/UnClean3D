#include "EffectLayerMaskImplementationGPU.h"

#include "Bang/Framebuffer.h"
#include "Bang/GameObject.h"
#include "Bang/MeshRenderer.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Transform.h"

#include "EffectLayer.h"
#include "EffectLayerMask.h"

using namespace Bang;

EffectLayerMaskImplementationGPU::EffectLayerMaskImplementationGPU()
{
    m_framebuffer = new Framebuffer();
}

EffectLayerMaskImplementationGPU::~EffectLayerMaskImplementationGPU()
{
}

void EffectLayerMaskImplementationGPU::Init()
{
    EffectLayerMaskImplementation::Init();

    Path spPath = GetGenerateEffectTextureShaderProgramPath();
    if (spPath.IsFile())
    {
        m_generateEffectTextureSP.Set(ShaderProgramFactory::Get(spPath));
    }
}

void EffectLayerMaskImplementationGPU::ReloadShaders()
{
    if (m_generateEffectTextureSP)
    {
        m_generateEffectTextureSP.Get()->ReImport();
    }
    Invalidate();
}

void EffectLayerMaskImplementationGPU::GenerateEffectMaskTexture(
    Texture2D *maskTexture,
    MeshRenderer *meshRend)
{
    if (GetIsPostProcessEffectLayer())
    {
        return;
    }

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::BLEND);
    GL::Disable(GL::Enablable::CULL_FACE);

    // Bind framebuffer and render to texture
    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(maskTexture, GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    Vector2i texSize = maskTexture->GetSize();
    GL::SetViewport(0, 0, texSize.x, texSize.y);

    if (ShaderProgram *sp = GetGenerateEffectTextureShaderProgram())
    {
        sp->Bind();
        SetGenerateEffectUniforms(sp, meshRend);

        GL::ClearColorBuffer(Color::Zero());
        EffectLayer *effectLayer = GetEffectLayerMask()->GetEffectLayer();
        GL::Render(effectLayer->GetTextureMesh()->GetVAO(),
                   GL::Primitive::TRIANGLES,
                   effectLayer->GetTextureMesh()->GetNumVerticesIds());
    }

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    m_isValid = true;
}

bool EffectLayerMaskImplementationGPU::CanGenerateEffectMaskTextureInRealTime()
    const
{
    return true;
}

void EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    sp->SetMatrix4(
        "SceneModelMatrix",
        meshRend->GetGameObject()->GetTransform()->GetLocalToWorldMatrix());
}

ShaderProgram *
EffectLayerMaskImplementationGPU::GetGenerateEffectTextureShaderProgram() const
{
    return m_generateEffectTextureSP.Get();
}
