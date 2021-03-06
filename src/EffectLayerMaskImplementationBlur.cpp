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
#include "Bang/Transform.h"

#include "EffectLayer.h"
#include "GLSLRayCaster.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerMaskImplementationBlur::EffectLayerMaskImplementationBlur()
{
    m_blurTexture = Assets::Create<Texture2D>();
    m_blurTexture.Get()->Fill(Color::Black(), 1, 1);
    m_blurTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_blurTextureAux = Assets::Create<Texture2D>();
    m_blurTextureAux.Get()->Fill(Color::Black(), 1, 1);
    m_blurTextureAux.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_blurShaderProgram.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "GenerateEffectMaskTextureBlur.bushader")));

    m_triangleUvsGLSLArray.SetFormat(GL::ColorFormat::RG32F);
    m_triangleNeighborhoodsGLSLArray.SetFormat(GL::ColorFormat::RG32F);
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

void EffectLayerMaskImplementationBlur::SetBlurMode(
    EffectLayerMaskImplementationBlur::BlurMode blurMode)
{
    if (blurMode != GetBlurMode())
    {
        m_blurMode = blurMode;
        Invalidate();
    }
}

void EffectLayerMaskImplementationBlur::SetBlurStepResolution(
    float blurStepResolution)
{
    if (blurStepResolution != GetBlurStepResolution())
    {
        m_blurStepResolution = blurStepResolution;
        Invalidate();
    }
}

int EffectLayerMaskImplementationBlur::GetBlurRadius() const
{
    return m_blurRadius;
}

EffectLayerMaskImplementationBlur::BlurMode
EffectLayerMaskImplementationBlur::GetBlurMode() const
{
    return m_blurMode;
}

float EffectLayerMaskImplementationBlur::GetBlurStepResolution() const
{
    return m_blurStepResolution;
}

void EffectLayerMaskImplementationBlur::Reflect()
{
    EffectLayerMaskImplementation::Reflect();

    BANG_REFLECT_VAR_ENUM("Blur mode", SetBlurMode, GetBlurMode, BlurMode);
    BANG_REFLECT_HINT_ENUM_FIELD_VALUE("Blur mode", "World", BlurMode::WORLD);
    BANG_REFLECT_HINT_ENUM_FIELD_VALUE(
        "Blur mode", "Texture", BlurMode::TEXTURE);

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationBlur,
                                   "Blur radius",
                                   SetBlurRadius,
                                   GetBlurRadius,
                                   BANG_REFLECT_HINT_SLIDER(0.0f, 10.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationBlur,
        "Blur step resolution",
        SetBlurStepResolution,
        GetBlurStepResolution,
        BANG_REFLECT_HINT_SLIDER(1.0f, 500.0f) +
            BANG_REFLECT_HINT_SHOWN(GetBlurMode() == BlurMode::WORLD));
}

bool EffectLayerMaskImplementationBlur::GetIsPostProcessEffectLayer() const
{
    return true;
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

void EffectLayerMaskImplementationBlur::ReloadShaders()
{
    EffectLayerMaskImplementationGPU::ReloadShaders();
    m_blurShaderProgram.Get()->ReImport();
}

void EffectLayerMaskImplementationBlur::FillGLSLArrays(MeshRenderer *mr)
{
    Mesh *mesh = mr->GetMesh();
    mesh->UpdateVAOsAndTables();

    Array<Array<Vector4>> neighborhoodsArray;
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Array<Mesh::TriangleId> triangleNeighbors;
        triangleNeighbors = mesh->GetNeighborTriangleIdsFromTriangleId(triId);

        Array<Vector4> neighborsVectors;
        for (Mesh::TriangleId neighborTriId : triangleNeighbors)
        {
            neighborsVectors.PushBack(Vector4(neighborTriId, 0, 0, 0));
        }
        neighborhoodsArray.PushBack(neighborsVectors);
    }
    m_triangleNeighborhoodsGLSLArray.Fill(neighborhoodsArray);

    Array<Array<Vector4>> triangleUvsArray;
    const auto &uvsPool = mesh->GetUvsPool();
    const auto &triVertIds = mesh->GetTrianglesVertexIds();
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Array<Vector4> triUvs;
        triUvs.PushBack(Vector4(uvsPool[triVertIds[triId * 3 + 0]], 0, 0));
        triUvs.PushBack(Vector4(uvsPool[triVertIds[triId * 3 + 1]], 0, 0));
        triUvs.PushBack(Vector4(uvsPool[triVertIds[triId * 3 + 2]], 0, 0));

        triangleUvsArray.PushBack(triUvs);
    }
    m_triangleUvsGLSLArray.Fill(triangleUvsArray);
}

bool EffectLayerMaskImplementationBlur::CanGenerateEffectMaskTextureInRealTime()
    const
{
    return true;
}

Texture2D *EffectLayerMaskImplementationBlur::GetMaskTextureToSee() const
{
    return m_blurTexture.Get();
}

bool EffectLayerMaskImplementationBlur::CompositeThisMask() const
{
    return false;
}

void EffectLayerMaskImplementationBlur::
    GenerateEffectMaskTextureOnCompositeAfter(
        Texture2D *mergedMaskTextureUntilNow,
        MeshRenderer *mr)
{
    GEngine *ge = GEngine::GetInstance();
    const bool needsToBlur =
        (GetEffectLayerMask()->GetVisible() && GetBlurRadius() > 0);
    Vector2i texSize = mergedMaskTextureUntilNow->GetSize();
    m_blurTexture.Get()->Resize(texSize.x, texSize.y);
    GL::SetViewport(0, 0, texSize.x, texSize.y);
    if (needsToBlur)
    {
        if (GetBlurMode() == BlurMode::WORLD)
        {
            if (!m_generatedTextureArrays)
            {
                FillGLSLArrays(mr);
                m_generatedTextureArrays = true;
            }

            GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
            GL::Push(GL::Pushable::SHADER_PROGRAM);
            GL::Push(GL::Pushable::BLEND_STATES);
            GL::Push(GL::Pushable::CULL_FACE);
            GL::Push(GL::Pushable::VIEWPORT);

            GL::Disable(GL::Enablable::BLEND);
            GL::Disable(GL::Enablable::CULL_FACE);

            m_framebuffer->Bind();
            m_framebuffer->SetAttachmentTexture(m_blurTexture.Get(),
                                                GL::Attachment::COLOR0);
            m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

            ShaderProgram *sp = m_blurShaderProgram.Get();
            sp->Bind();
            SetGenerateEffectUniforms(sp, mr);
            GLSLRayCaster *rayCaster =
                View3DScene::GetInstance()->GetGLSLRayCaster();
            rayCaster->Bind(sp);

            sp->SetFloat("BlurRadius", GetBlurRadius());
            sp->SetFloat("BlurStep", (1.0f / GetBlurStepResolution()));
            sp->SetTexture2D("TextureToBlur", mergedMaskTextureUntilNow);
            m_triangleUvsGLSLArray.Bind("TriangleUvs", sp);
            m_triangleNeighborhoodsGLSLArray.Bind("TriangleNeighborhoods", sp);

            GL::ClearColorBuffer(Color::Zero());
            EffectLayer *effectLayer = GetEffectLayerMask()->GetEffectLayer();
            GL::Render(effectLayer->GetTextureMesh()->GetVAO(),
                       GL::Primitive::TRIANGLES,
                       effectLayer->GetTextureMesh()->GetNumVerticesIds());
            ge->CopyTexture(m_blurTexture.Get(), mergedMaskTextureUntilNow);

            GL::Pop(GL::Pushable::VIEWPORT);
            GL::Pop(GL::Pushable::CULL_FACE);
            GL::Pop(GL::Pushable::BLEND_STATES);
            GL::Pop(GL::Pushable::SHADER_PROGRAM);
            GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
        }
        else
        {
            m_blurTextureAux.Get()->Resize(texSize);
            ge->BlurTexture(mergedMaskTextureUntilNow,
                            m_blurTextureAux.Get(),
                            m_blurTexture.Get(),
                            GetBlurRadius(),
                            BlurType::GAUSSIAN);
            ge->CopyTexture(m_blurTexture.Get(), mergedMaskTextureUntilNow);
        }
    }
    else
    {
        m_blurTexture.Get()->Resize(texSize);
        ge->CopyTexture(mergedMaskTextureUntilNow, m_blurTexture.Get());
    }
}
