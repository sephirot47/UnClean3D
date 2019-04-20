#include "EffectLayerMaskImplementationEdges.h"

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

EffectLayerMaskImplementationEdges::
    EffectLayerMaskImplementationEdges()
{
}

EffectLayerMaskImplementationEdges::
    ~EffectLayerMaskImplementationEdges()
{
}

void EffectLayerMaskImplementationEdges::SetNumRays(int numRays)
{
    if (numRays != GetNumRays())
    {
        m_numRays = numRays;
    }
}

void EffectLayerMaskImplementationEdges::SetEdgeThreshold(float edgeThreshold)
{
    m_edgeThreshold = edgeThreshold;
}

void EffectLayerMaskImplementationEdges::SetEdgeAmplitude(
    float edgeAmplitude)
{
    if (edgeAmplitude != GetEdgeAmplitude())
    {
        m_edgeAmplitude = edgeAmplitude;
    }
}

int EffectLayerMaskImplementationEdges::GetNumRays() const
{
    return m_numRays;
}

float EffectLayerMaskImplementationEdges::GetEdgeThreshold() const
{
    return m_edgeThreshold;
}

float EffectLayerMaskImplementationEdges::GetEdgeAmplitude() const
{
    return m_edgeAmplitude;
}

void EffectLayerMaskImplementationEdges::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationEdges,
        "Num Rays",
        SetNumRays,
        GetNumRays,
        BANG_REFLECT_HINT_SLIDER(1.0f, 100.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationEdges,
        "Edge threshold",
        SetEdgeThreshold,
        GetEdgeThreshold,
        BANG_REFLECT_HINT_SLIDER(0.01f, 10.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationEdges,
        "Edge amplitude",
        SetEdgeAmplitude,
        GetEdgeAmplitude,
        BANG_REFLECT_HINT_SLIDER(0.01f, 20.0f));
}

EffectLayerMask::Type
EffectLayerMaskImplementationEdges::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::EDGES;
}

String EffectLayerMaskImplementationEdges::GetTypeName() const
{
    return "Edges";
}

Path EffectLayerMaskImplementationEdges::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureEdges.bushader");
}

void EffectLayerMaskImplementationEdges::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *mr)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, mr);

    GLSLRayCaster *rayCaster = View3DScene::GetInstance()->GetGLSLRayCaster();
    rayCaster->Bind(sp);
    sp->SetInt("NumRays", GetNumRays());
    sp->SetFloat("EdgeThreshold", GetEdgeThreshold());
    sp->SetFloat("EdgeAmplitude", GetEdgeAmplitude());
}

void EffectLayerMaskImplementationEdges::GenerateEffectMaskTexture(
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

bool EffectLayerMaskImplementationEdges::
    CanGenerateEffectMaskTextureInRealTime() const
{
    return false;
}
