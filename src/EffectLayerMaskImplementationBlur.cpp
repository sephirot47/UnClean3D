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

    m_triangleUvsGLSLArray.SetFormat(GL::ColorFormat::RG32F);
    m_trianglePositionsGLSLArray.SetFormat(GL::ColorFormat::RGB32F);
    m_oneRingNeighborhoodsGLSLArray.SetFormat(GL::ColorFormat::RG32F);
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

float EffectLayerMaskImplementationBlur::GetBlurStepResolution() const
{
    return m_blurStepResolution;
}

void EffectLayerMaskImplementationBlur::Reflect()
{
    EffectLayerMaskImplementation::Reflect();

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationBlur,
                                   "Blur radius",
                                   SetBlurRadius,
                                   GetBlurRadius,
                                   BANG_REFLECT_HINT_SLIDER(0.0f, 5.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationBlur,
                                   "Blur step resolution",
                                   SetBlurStepResolution,
                                   GetBlurStepResolution,
                                   BANG_REFLECT_HINT_SLIDER(1.0f, 500.0f));
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

    Array<Array<Vector4>> oneRingNeighborhoodsArray;
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Array<Mesh::TriangleId> oneRingNeighbors;
        oneRingNeighbors = mesh->GetNeighborTriangleIdsFromTriangleId(triId);

        Array<Vector4> oneRingNeighborsVectors;
        for (Mesh::TriangleId neighborTriId : oneRingNeighbors)
        {
            oneRingNeighborsVectors.PushBack(Vector4(neighborTriId, 0, 0, 0));
        }

        oneRingNeighborhoodsArray.PushBack(oneRingNeighborsVectors);
    }
    m_oneRingNeighborhoodsGLSLArray.Fill(oneRingNeighborhoodsArray);

    const Matrix4 &localToWorldMatrix =
        mr->GetGameObject()->GetTransform()->GetLocalToWorldMatrix();
    Array<Array<Vector4>> trianglePositionsArray;
    for (Mesh::TriangleId triId = 0; triId < mesh->GetNumTriangles(); ++triId)
    {
        Triangle tri = mesh->GetTriangle(triId);
        tri = localToWorldMatrix * tri;

        Array<Vector4> triPositions;
        triPositions.PushBack(Vector4(tri.GetPoint(0), 0));
        triPositions.PushBack(Vector4(tri.GetPoint(1), 0));
        triPositions.PushBack(Vector4(tri.GetPoint(2), 0));

        trianglePositionsArray.PushBack(triPositions);
    }
    m_trianglePositionsGLSLArray.Fill(trianglePositionsArray);

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
    return GetEffectLayerMask()->GetVisible() ? m_blurTexture0.Get()
                                              : m_blurTexture1.Get();
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

    if (!m_generatedTextureArrays)
    {
        FillGLSLArrays(mr);
        m_generatedTextureArrays = true;
    }

    bool needsToBlur =
        (GetEffectLayerMask()->GetVisible() && GetBlurRadius() > 0);
    if (needsToBlur)
    {
        GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
        GL::Push(GL::Pushable::SHADER_PROGRAM);
        GL::Push(GL::Pushable::BLEND_STATES);
        GL::Push(GL::Pushable::CULL_FACE);
        GL::Push(GL::Pushable::VIEWPORT);

        GL::Disable(GL::Enablable::BLEND);
        GL::Disable(GL::Enablable::CULL_FACE);

        m_blurTexture0.Get()->Resize(mergedMaskTextureUntilNow->GetSize());

        m_framebuffer->Bind();
        m_framebuffer->SetAttachmentTexture(m_blurTexture0.Get(),
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        Vector2i texSize = mergedMaskTextureUntilNow->GetSize();
        GL::SetViewport(0, 0, texSize.x, texSize.y);

        ShaderProgram *sp = m_blurShaderProgram.Get();
        sp->Bind();
        SetGenerateEffectUniforms(sp, mr);
        sp->SetFloat("BlurRadius", GetBlurRadius());
        sp->SetFloat("BlurStep", (1.0f / GetBlurStepResolution()));
        sp->SetTexture2D("TextureToBlur", mergedMaskTextureUntilNow);
        m_triangleUvsGLSLArray.Bind("TriangleUvs", sp);
        m_trianglePositionsGLSLArray.Bind("TrianglePositions", sp);
        m_oneRingNeighborhoodsGLSLArray.Bind("OneRingNeighborhoods", sp);

        GL::ClearColorBuffer(Color::Zero());
        EffectLayer *effectLayer = GetEffectLayerMask()->GetEffectLayer();
        GL::Render(effectLayer->GetTextureMesh()->GetVAO(),
                   GL::Primitive::TRIANGLES,
                   effectLayer->GetTextureMesh()->GetNumVerticesIds());
        ge->CopyTexture(m_blurTexture0.Get(), mergedMaskTextureUntilNow);

        /*
        ge->BlurTexture(mergedMaskTextureUntilNow,
                        m_blurTexture0.Get(),
                        m_blurTexture1.Get(),
                        GetBlurRadius(),
                        BlurType::GAUSSIAN);
        ge->CopyTexture(m_blurTexture1.Get(), mergedMaskTextureUntilNow);
        */

        GL::Pop(GL::Pushable::VIEWPORT);
        GL::Pop(GL::Pushable::CULL_FACE);
        GL::Pop(GL::Pushable::BLEND_STATES);
        GL::Pop(GL::Pushable::SHADER_PROGRAM);
        GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    }
    else
    {
        m_blurTexture1.Get()->Resize(mergedMaskTextureUntilNow->GetSize());
        ge->CopyTexture(mergedMaskTextureUntilNow, m_blurTexture1.Get());
    }
}
