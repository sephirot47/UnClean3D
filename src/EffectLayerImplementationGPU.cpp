#include "EffectLayerImplementationGPU.h"

#include "Bang/Framebuffer.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Transform.h"

#include "View3DScene.h"

using namespace Bang;

EffectLayerImplementationGPU::EffectLayerImplementationGPU()
{
    m_framebuffer = new Framebuffer();
}

EffectLayerImplementationGPU::~EffectLayerImplementationGPU()
{
}

void EffectLayerImplementationGPU::Init()
{
    EffectLayerImplementation::Init();

    Path spPath = GetGenerateEffectTextureShaderProgramPath();
    m_generateEffectTextureSP.Set(ShaderProgramFactory::Get(spPath));
}

void EffectLayerImplementationGPU::ReloadShaders()
{
    m_generateEffectTextureSP.Get()->ReImport();
}

void EffectLayerImplementationGPU::GenerateEffectTexture(
    Texture2D *effectTexture)
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::BLEND);
    GL::Disable(GL::Enablable::CULL_FACE);

    // Bind framebuffer and render to texture
    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(effectTexture, GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    Vector2i texSize = effectTexture->GetSize();
    GL::SetViewport(0, 0, texSize.x, texSize.y);

    if (ShaderProgram *sp = GetGenerateEffectTextureShaderProgram())
    {
        sp->Bind();
        SetGenerateEffectUniforms(sp);

        GL::ClearColorBuffer(Color::Zero());
        GL::Render(GetEffectLayer()->GetTextureMesh()->GetVAO(),
                   GL::Primitive::TRIANGLES,
                   GetEffectLayer()->GetTextureMesh()->GetNumVerticesIds());
    }

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

bool EffectLayerImplementationGPU::CanGenerateEffectTextureInRealTime() const
{
    return true;
}

void EffectLayerImplementationGPU::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
    sp->SetMatrix4("SceneModelMatrix",
                   view3DScene->GetModelGameObject()
                       ->GetTransform()
                       ->GetLocalToWorldMatrix());
}

ShaderProgram *
EffectLayerImplementationGPU::GetGenerateEffectTextureShaderProgram() const
{
    return m_generateEffectTextureSP.Get();
}
