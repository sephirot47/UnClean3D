#include "EffectLayer.h"

#include "Bang/Array.h"
#include "Bang/Assets.h"
#include "Bang/Camera.h"
#include "Bang/Dialog.h"
#include "Bang/Framebuffer.h"
#include "Bang/GBuffer.h"
#include "Bang/GEngine.h"
#include "Bang/GEngine.h"
#include "Bang/GLUniforms.h"
#include "Bang/GameObject.h"
#include "Bang/Image.h"
#include "Bang/Material.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/Random.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/SimplexNoise.h"
#include "Bang/Texture2D.h"
#include "Bang/TextureFactory.h"
#include "Bang/Thread.h"
#include "Bang/Transform.h"
#include "Bang/Triangle.h"
#include "Bang/Triangle2D.h"
#include "Bang/VAO.h"
#include "Bang/VBO.h"

#include "ControlPanel.h"
#include "EffectLayerMask.h"
#include "EffectLayerMaskImplementation.h"
#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayer::EffectLayer(MeshRenderer *mr)
{
    p_meshRenderer = mr;

    m_color = Random::GetColorOpaque();
    m_height = Random::GetRange(-1, 1);
    m_roughness = Random::GetRange(0, 1);
    m_metalness = Random::GetRange(0, 1);

    m_framebuffer = new Framebuffer();

    // Create some textures
    m_effectColorTexture = Assets::Create<Texture2D>();
    m_effectColorTexture.Get()->Fill(Color::Zero(), 1, 1);
    m_effectColorTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_effectMiscTexture = Assets::Create<Texture2D>();
    m_effectMiscTexture.Get()->Fill(Color::Zero(), 1, 1);
    m_effectMiscTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_growAuxiliarTexture = Assets::Create<Texture2D>();
    m_growAuxiliarTexture.Get()->Fill(Color::Zero(), 1, 1);
    m_growAuxiliarTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    // Create mask textures
    m_mergedMaskTexture = Assets::Create<Texture2D>();
    m_mergedMaskTexture.Get()->Fill(Color::Zero(), 1, 1);

    // ShaderPrograms
    m_generateEffectTextureSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "GenerateEffectTexture.bushader")));
    m_growTextureBordersSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "GrowBorders.bushader")));

    // Create texture mesh
    m_textureMesh = Assets::Create<Mesh>();
    if (Mesh *originalMesh = mr->GetMesh())
    {
        // Gather some data
        Array<Vector3> texTriMeshPositions;
        Array<Vector3> originalVertexPositions;
        Array<Vector3> originalVertexNormals;
        for (Mesh::VertexId triId = 0; triId < originalMesh->GetNumTriangles();
             ++triId)
        {
            for (uint i = 0; i < 3; ++i)
            {
                Mesh::VertexId vId =
                    originalMesh->GetTrianglesVertexIds()[triId * 3 + i];
                if (vId >= originalMesh->GetUvsPool().Size())
                {
                    break;
                }

                const Vector2 &oriVertUv = originalMesh->GetUvsPool()[vId];
                Vector3 texTriMeshPos = oriVertUv.xy0() * 2.0f - 1.0f;
                texTriMeshPos.y *= -1;
                texTriMeshPositions.PushBack(texTriMeshPos);

                const Vector3 &oriVertPos =
                    originalMesh->GetPositionsPool()[vId];
                const Vector3 &oriVertNormal =
                    originalMesh->GetNormalsPool()[vId];
                originalVertexPositions.PushBack(oriVertPos);
                originalVertexNormals.PushBack(oriVertNormal);
            }
        }

        // Set original mesh uvs as texture mesh positions
        GetTextureMesh()->SetPositionsPool(texTriMeshPositions);
        GetTextureMesh()->SetTrianglesVertexIds({});

        // Add actual original mesh attributes as other VBOs
        m_positionsVBO = new VBO();
        m_positionsVBO->CreateAndFill(
            originalVertexPositions.Data(),
            originalVertexPositions.Size() * sizeof(float) * 3);
        GetTextureMesh()->GetVAO()->SetVBO(
            m_positionsVBO, 1, 3, GL::VertexAttribDataType::FLOAT);

        m_normalsVBO = new VBO();
        m_normalsVBO->CreateAndFill(
            originalVertexNormals.Data(),
            originalVertexNormals.Size() * sizeof(float) * 3);
        GetTextureMesh()->GetVAO()->SetVBO(
            m_normalsVBO, 2, 3, GL::VertexAttribDataType::FLOAT);

        GetTextureMesh()->UpdateVAOs();
    }
}

EffectLayer::~EffectLayer()
{
    delete m_framebuffer;

    for (EffectLayerMask *effectLayerMask : m_masks)
    {
        delete effectLayerMask;
    }
}

void EffectLayer::GenerateEffectTexture()
{
    Vector2i texSize = GetControlPanel()->GetTextureSize();
    GetEffectColorTexture()->ResizeConservingData(texSize.x, texSize.y);
    GetEffectMiscTexture()->ResizeConservingData(texSize.x, texSize.y);

    // Masks
    {
        for (EffectLayerMask *mask : GetMasks())
        {
            if (!mask->IsValid())
            {
                mask->GenerateMask();
            }
        }
    }
    MergeMasks();

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::BLEND);

    GL::SetViewport(0,
                    0,
                    GetEffectColorTexture()->GetWidth(),
                    GetEffectColorTexture()->GetHeight());

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(GetEffectColorTexture(),
                                        GL::Attachment::COLOR0);
    m_framebuffer->SetAttachmentTexture(GetEffectMiscTexture(),
                                        GL::Attachment::COLOR1);
    m_framebuffer->SetDrawBuffers(
        {GL::Attachment::COLOR0, GL::Attachment::COLOR1});

    ShaderProgram *sp = m_generateEffectTextureSP.Get();
    sp->Bind();
    sp->SetTexture2D("MaskTexture", GetMergedMaskTexture());
    sp->SetColor("Color", GetColor());
    sp->SetFloat("Height", GetHeight());
    sp->SetFloat("Roughness", GetRoughness());
    sp->SetFloat("Metalness", GetMetalness());

    GEngine::GetInstance()->RenderViewportPlane();

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    GrowTextureBorders(GetEffectColorTexture());

    m_isValid = true;
}

void EffectLayer::MergeMasks()
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Enable(GL::Enablable::BLEND);

    Vector2i size = GetEffectColorTexture()->GetSize();
    GL::SetViewport(0, 0, size.x, size.y);

    GetMergedMaskTexture()->ResizeConservingData(size.x, size.y);

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(GetMergedMaskTexture(),
                                        GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    GL::ClearColorBuffer(Color::Zero());
    for (int i = GetMasks().Size() - 1; i >= 0; --i)
    {
        EffectLayerMask *mask = GetMasks()[i];
        if (!mask->GetVisible())
        {
            continue;
        }

        switch (mask->GetBlendMode())
        {
            case EffectLayerMask::BlendMode::IGNORE: continue; break;

            case EffectLayerMask::BlendMode::ADD:
                GL::BlendFunc(GL::BlendFactor::ONE, GL::BlendFactor::ONE);
                GL::BlendEquation(GL::BlendEquationE::FUNC_ADD);
                break;

            case EffectLayerMask::BlendMode::MULTIPLY:
                GL::BlendFunc(GL::BlendFactor::ZERO,
                              GL::BlendFactor::SRC_COLOR);
                GL::BlendEquation(GL::BlendEquationE::FUNC_ADD);
                break;

            case EffectLayerMask::BlendMode::SUBTRACT:
                GL::BlendFunc(GL::BlendFactor::ONE, GL::BlendFactor::ONE);
                GL::BlendEquation(GL::BlendEquationE::FUNC_REVERSE_SUBTRACT);
                break;
        }

        mask->GetMaskTexture()->ResizeConservingData(size.x, size.y);
        GEngine::GetInstance()->RenderTexture(mask->GetMaskTexture());
    }

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

void EffectLayer::ReloadShaders()
{
    for (EffectLayerMask *effectLayerMask : GetMasks())
    {
        effectLayerMask->ReloadShaders();
    }
    m_generateEffectTextureSP.Get()->ReImport();
    m_growTextureBordersSP.Get()->ReImport();
    Invalidate();
}

void EffectLayer::SetVisible(bool visible)
{
    m_visible = visible;
}

void EffectLayer::SetName(const String &name)
{
    m_name = name;
}

void EffectLayer::SetColorBlendMode(EffectLayer::BlendMode blendMode)
{
    if (blendMode != GetColorBlendMode())
    {
        m_colorBlendMode = blendMode;
        Invalidate();
    }
}

void EffectLayer::SetHeightBlendMode(EffectLayer::BlendMode blendMode)
{
    if (blendMode != GetHeightBlendMode())
    {
        m_heightBlendMode = blendMode;
        Invalidate();
    }
}

void EffectLayer::SetRoughnessBlendMode(EffectLayer::BlendMode blendMode)
{
    if (blendMode != GetRoughnessBlendMode())
    {
        m_roughnessBlendMode = blendMode;
        Invalidate();
    }
}

void EffectLayer::SetMetalnessBlendMode(EffectLayer::BlendMode blendMode)
{
    if (blendMode != GetMetalnessBlendMode())
    {
        m_metalnessBlendMode = blendMode;
        Invalidate();
    }
}

void EffectLayer::SetColor(const Color &color)
{
    if (color != GetColor())
    {
        m_color = color;
        Invalidate();
    }
}

void EffectLayer::SetHeight(float height)
{
    if (height != GetHeight())
    {
        m_height = height;
        Invalidate();
    }
}

void EffectLayer::SetRoughness(float roughness)
{
    if (roughness != GetRoughness())
    {
        m_roughness = roughness;
        Invalidate();
    }
}

void EffectLayer::SetMetalness(float metalness)
{
    if (metalness != GetMetalness())
    {
        m_metalness = metalness;
        Invalidate();
    }
}

EffectLayerMask *EffectLayer::AddNewMask()
{
    EffectLayerMask *newMask = new EffectLayerMask();
    newMask->SetEffectLayer(this);
    m_masks.PushBack(newMask);
    Invalidate();
    return newMask;
}

void EffectLayer::MoveMask(EffectLayerMask *effectLayerMask, uint newIndex)
{
    m_masks.Remove(effectLayerMask);
    m_masks.Insert(effectLayerMask, newIndex);
    Invalidate();
}

void EffectLayer::RemoveMask(EffectLayerMask *mask)
{
    delete mask;
    m_masks.Remove(mask);
    Invalidate();
}

void EffectLayer::Invalidate(bool recursiveDown)
{
    m_isValid = false;
    if (recursiveDown)
    {
        for (EffectLayerMask *mask : GetMasks())
        {
            mask->Invalidate(recursiveDown);
        }
    }
}

bool EffectLayer::GetVisible() const
{
    return m_visible;
}

Mesh *EffectLayer::GetMesh() const
{
    return p_meshRenderer ? p_meshRenderer->GetMesh() : nullptr;
}

const Color &EffectLayer::GetColor() const
{
    return m_color;
}

float EffectLayer::GetHeight() const
{
    return m_height;
}

float EffectLayer::GetRoughness() const
{
    return m_roughness;
}

float EffectLayer::GetMetalness() const
{
    return m_metalness;
}

EffectLayer::BlendMode EffectLayer::GetColorBlendMode() const
{
    return m_colorBlendMode;
}

EffectLayer::BlendMode EffectLayer::GetHeightBlendMode() const
{
    return m_heightBlendMode;
}

EffectLayer::BlendMode EffectLayer::GetRoughnessBlendMode() const
{
    return m_roughnessBlendMode;
}

EffectLayer::BlendMode EffectLayer::GetMetalnessBlendMode() const
{
    return m_metalnessBlendMode;
}

Mesh *EffectLayer::GetTextureMesh() const
{
    return m_textureMesh.Get();
}

MeshRenderer *EffectLayer::GetMeshRenderer() const
{
    return p_meshRenderer;
}

Texture2D *EffectLayer::GetMergedMaskTexture() const
{
    return m_mergedMaskTexture.Get();
}

const String &EffectLayer::GetName() const
{
    return m_name;
}

bool EffectLayer::IsValid() const
{
    return m_isValid;
}

Texture2D *EffectLayer::GetEffectColorTexture() const
{
    return m_effectColorTexture.Get();
}

Texture2D *EffectLayer::GetEffectMiscTexture() const
{
    return m_effectMiscTexture.Get();
}

const Array<EffectLayerMask *> &EffectLayer::GetMasks() const
{
    return m_masks;
}

ControlPanel *EffectLayer::GetControlPanel() const
{
    return MainScene::GetInstance()->GetControlPanel();
}

void EffectLayer::Reflect()
{
    Serializable::Reflect();

    ReflectVar<Color>("Color",
                      [this](const Color &color) { SetColor(color); },
                      [this]() { return GetColor(); },
                      BANG_REFLECT_HINT_SHOWN(false));

    ReflectVar<float>("Height",
                      [this](float height) { SetHeight(height); },
                      [this]() { return GetHeight(); },
                      BANG_REFLECT_HINT_SHOWN(false));

    ReflectVar<float>("Roughness",
                      [this](float roughness) { SetRoughness(roughness); },
                      [this]() { return GetRoughness(); },
                      BANG_REFLECT_HINT_SHOWN(false));

    ReflectVar<float>("Metalness",
                      [this](float metalness) { SetMetalness(metalness); },
                      [this]() { return GetMetalness(); },
                      BANG_REFLECT_HINT_SHOWN(false));

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayer,
                                   "Visible",
                                   SetVisible,
                                   GetVisible,
                                   BANG_REFLECT_HINT_SHOWN(false));
}

void EffectLayer::GrowTextureBorders(Texture2D *texture)
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::BLEND);
    m_growAuxiliarTexture.Get()->ResizeConservingData(texture->GetWidth(),
                                                      texture->GetHeight());
    GEngine::GetInstance()->CopyTexture(texture, m_growAuxiliarTexture.Get());

    ShaderProgram *sp = m_growTextureBordersSP.Get();
    sp->Bind();
    sp->SetTexture2D("TextureToGrow", m_growAuxiliarTexture.Get());
    GL::SetViewport(0, 0, texture->GetWidth(), texture->GetHeight());

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(texture, GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    // GL::ClearColorBuffer(Color::Zero());
    GEngine::GetInstance()->RenderViewportPlane();

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}
