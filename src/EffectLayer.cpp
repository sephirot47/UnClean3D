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

    // Create mask textures
    m_mergedMaskTexture = Assets::Create<Texture2D>();
    m_mergedMaskTexture.Get()->Fill(Color::Zero(), 1, 1);

    // ShaderPrograms
    m_generateEffectTextureSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "GenerateEffectTexture.bushader")));
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
            if (!mask->IsValid() &&
                mask->CanGenerateEffectMaskTextureInRealTime())
            {
                mask->GenerateMask();
            }
        }
    }
    MergeMasks(GetMeshRenderer());

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

    m_isValid = true;
}

void EffectLayer::MergeMasks(MeshRenderer *mr)
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

        EffectLayerMaskImplementation *impl = mask->GetImplementation();
        if (impl)
        {
            impl->GenerateEffectMaskTextureOnCompositeBefore(
                GetMergedMaskTexture(), mr);
        }

        if (mask->GetVisible())
        {
            if (impl->CompositeThisMask())
            {
                switch (mask->GetBlendMode())
                {
                    case EffectLayerMask::BlendMode::IGNORE: continue; break;

                    case EffectLayerMask::BlendMode::ADD:
                        GL::BlendFunc(GL::BlendFactor::ONE,
                                      GL::BlendFactor::ONE);
                        GL::BlendEquation(GL::BlendEquationE::FUNC_ADD);
                        break;

                    case EffectLayerMask::BlendMode::MULTIPLY:
                        GL::BlendFunc(GL::BlendFactor::ZERO,
                                      GL::BlendFactor::SRC_COLOR);
                        GL::BlendEquation(GL::BlendEquationE::FUNC_ADD);
                        break;

                    case EffectLayerMask::BlendMode::SUBTRACT:
                        GL::BlendFunc(GL::BlendFactor::ONE,
                                      GL::BlendFactor::ONE);
                        GL::BlendEquation(
                            GL::BlendEquationE::FUNC_REVERSE_SUBTRACT);
                        break;
                }

                mask->GetMaskTexture()->ResizeConservingData(size.x, size.y);
                GEngine::GetInstance()->RenderTexture(mask->GetMaskTexture());
            }
        }

        if (impl)
        {
            impl->GenerateEffectMaskTextureOnCompositeAfter(
                GetMergedMaskTexture(), mr);
        }
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
    m_masks.PushFront(newMask);
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
    m_masks.Remove(mask);
    Invalidate();
    delete mask;
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
    return View3DScene::GetInstance()->GetTextureMesh();
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

void EffectLayer::ImportMeta(const MetaNode &meta)
{
    Serializable::ImportMeta(meta);

    const auto &masksMetas = meta.GetChildren("EffectLayerMasksMeta");
    for (auto it = masksMetas.RBegin(); it != masksMetas.REnd(); ++it)
    {
        const MetaNode &maskMeta = *it;
        EffectLayerMask *mask = AddNewMask();
        mask->ImportMeta(maskMeta);
    }
}

void EffectLayer::ExportMeta(MetaNode *meta) const
{
    Serializable::ExportMeta(meta);
    for (EffectLayerMask *mask : GetMasks())
    {
        MetaNode maskMeta;
        mask->ExportMeta(&maskMeta);
        meta->AddChild(maskMeta, "EffectLayerMasksMeta");
    }
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

    ReflectVar<String>("Name",
                       [this](const String &name) { SetName(name); },
                       [this]() { return GetName(); },
                       BANG_REFLECT_HINT_SHOWN(false));

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayer,
                                   "Visible",
                                   SetVisible,
                                   GetVisible,
                                   BANG_REFLECT_HINT_SHOWN(false));
}
