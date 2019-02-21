#include "EffectLayerMaskImplementationAmbientOcclusion.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Transform.h"

#include "View3DScene.h"

using namespace Bang;

EffectLayerMaskImplementationAmbientOcclusion::
    EffectLayerMaskImplementationAmbientOcclusion()
{
}

EffectLayerMaskImplementationAmbientOcclusion::
    ~EffectLayerMaskImplementationAmbientOcclusion()
{
}

void EffectLayerMaskImplementationAmbientOcclusion::SetNumRays(int numRays)
{
    if (numRays != GetNumRays())
    {
        m_numRays = numRays;
    }
}

void EffectLayerMaskImplementationAmbientOcclusion::SetMaxDistance(
    float maxDistance)
{
    if (maxDistance != GetMaxDistance())
    {
        m_maxDistance = maxDistance;
    }
}

int EffectLayerMaskImplementationAmbientOcclusion::GetNumRays() const
{
    return m_numRays;
}

float EffectLayerMaskImplementationAmbientOcclusion::GetMaxDistance() const
{
    return m_maxDistance;
}

void EffectLayerMaskImplementationAmbientOcclusion::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationAmbientOcclusion,
        "Num Rays",
        SetNumRays,
        GetNumRays,
        BANG_REFLECT_HINT_SLIDER(1.0f, 100.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationAmbientOcclusion,
        "Max distance",
        SetMaxDistance,
        GetMaxDistance,
        BANG_REFLECT_HINT_SLIDER(0.005f, 1.0f));
}

EffectLayerMask::Type
EffectLayerMaskImplementationAmbientOcclusion::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::AMBIENT_OCCLUSION;
}

String EffectLayerMaskImplementationAmbientOcclusion::GetTypeName() const
{
    return "Ambient Occlusion";
}

Path EffectLayerMaskImplementationAmbientOcclusion::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureAmbientOcclusion.bushader");
}

void EffectLayerMaskImplementationAmbientOcclusion::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *mr)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, mr);

    GLSLRayCaster *rayCaster = View3DScene::GetInstance()->GetGLSLRayCaster();
    rayCaster->Bind(sp);
    sp->SetInt("NumRays", GetNumRays());
    sp->SetFloat("MaxDistance", GetMaxDistance());
}

void EffectLayerMaskImplementationAmbientOcclusion::GenerateEffectMaskTexture(
    Texture2D *maskTexture,
    MeshRenderer *meshRend)
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

        for (Mesh::TriangleId triId = 0;
             triId < effectLayer->GetTextureMesh()->GetNumTriangles();
             ++triId)
        {
            GL::Render(effectLayer->GetTextureMesh()->GetVAO(),
                       GL::Primitive::TRIANGLES,
                       3,
                       triId * 3);
        }
    }

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    m_isValid = true;
}

bool EffectLayerMaskImplementationAmbientOcclusion::
    CanGenerateEffectMaskTextureInRealTime() const
{
    return false;
}
