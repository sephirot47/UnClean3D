#include "EffectLayerMask.h"

#include "Bang/Assets.h"
#include "Bang/Camera.h"
#include "Bang/GBuffer.h"
#include "Bang/GEngine.h"
#include "Bang/GLUniforms.h"
#include "Bang/Input.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Transform.h"

#include "ControlPanel.h"
#include "EffectLayer.h"
#include "EffectLayerMaskImplementationAmbientOcclusion.h"
#include "EffectLayerMaskImplementationBrush.h"
#include "EffectLayerMaskImplementationFractal.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerMask::EffectLayerMask()
{
    m_maskTexture = Assets::Create<Texture2D>();
    m_maskTexture.Get()->Fill(Color::Zero(), 1, 1);

    m_framebuffer = new Framebuffer();

    SetType(EffectLayerMask::Type::FRACTAL);
}

EffectLayerMask::~EffectLayerMask()
{
    delete m_framebuffer;
}

void EffectLayerMask::Update()
{
    if (GetImplementation())
    {
        GetImplementation()->Update();
    }
}

void EffectLayerMask::ReloadShaders()
{
    if (GetImplementation())
    {
        GetImplementation()->ReloadShaders();
    }
    Invalidate();
}

void EffectLayerMask::SetType(EffectLayerMask::Type type)
{
    if (type != GetType())
    {
        if (GetImplementation())
        {
            delete m_implementation;
            m_implementation = nullptr;
        }

        m_type = type;

        switch (type)
        {
            case EffectLayerMask::Type::FRACTAL:
                m_implementation = new EffectLayerMaskImplementationFractal();
                break;

            case EffectLayerMask::Type::AMBIENT_OCCLUSION:
                m_implementation =
                    new EffectLayerMaskImplementationAmbientOcclusion();
                break;

            case EffectLayerMask::Type::BRUSH:
                m_implementation = new EffectLayerMaskImplementationBrush();
                break;

            default: ASSERT(false); break;
        }

        m_implementation->SetEffectLayerMask(this);
        m_implementation->Init();
        Clear();
        Invalidate();
    }
}

void EffectLayerMask::SetBlendMode(EffectLayerMask::BlendMode blendMode)
{
    if (blendMode != GetBlendMode())
    {
        m_blendMode = blendMode;
        Invalidate(true);
    }
}

void EffectLayerMask::SetEffectLayer(EffectLayer *effectLayer)
{
    p_effectLayer = effectLayer;
}

void EffectLayerMask::SetName(const String &name)
{
    m_name = name;
}

void EffectLayerMask::GenerateMask()
{
    if (EffectLayerMaskImplementation *impl = GetImplementation())
    {
        MeshRenderer *mr = GetEffectLayer()->GetMeshRenderer();
        GetMaskTexture()->ResizeConservingData(
            GetEffectLayer()->GetEffectColorTexture()->GetWidth(),
            GetEffectLayer()->GetEffectColorTexture()->GetHeight());
        impl->GenerateEffectMaskTexture(GetMaskTexture(), mr);
    }
    m_isValid = true;
}

void EffectLayerMask::Clear()
{
    GEngine *ge = GEngine::GetInstance();
    ge->FillTexture(GetMaskTexture(), Color::Zero());
    Invalidate();
}

void EffectLayerMask::Fill()
{
    GEngine *ge = GEngine::GetInstance();
    ge->FillTexture(GetMaskTexture(), Color::One());
    Invalidate();
}

void EffectLayerMask::SetVisible(bool visible)
{
    if (visible != GetVisible())
    {
        m_visible = visible;
        Invalidate();
    }
}

void EffectLayerMask::Invalidate(bool recursiveDown)
{
    m_isValid = false;
    if (GetEffectLayer())
    {
        GetEffectLayer()->Invalidate();
    }

    if (recursiveDown)
    {
        if (GetImplementation())
        {
            GetImplementation()->Invalidate();
        }
    }
}

const String &EffectLayerMask::GetName() const
{
    return m_name;
}

EffectLayerMask::BlendMode EffectLayerMask::GetBlendMode() const
{
    return m_blendMode;
}

EffectLayerMask::Type EffectLayerMask::GetType() const
{
    return m_type;
}

Texture2D *EffectLayerMask::GetMaskTexture() const
{
    return m_maskTexture.Get();
}

EffectLayer *EffectLayerMask::GetEffectLayer() const
{
    return p_effectLayer;
}

EffectLayerMaskImplementation *EffectLayerMask::GetImplementation() const
{
    return m_implementation;
}

bool EffectLayerMask::IsValid() const
{
    return m_isValid;
}

void EffectLayerMask::Reflect()
{
    Serializable::Reflect();

    BANG_REFLECT_VAR_ENUM("Type", SetType, GetType, EffectLayerMask::Type);
}

ControlPanel *EffectLayerMask::GetControlPanel() const
{
    return GetEffectLayer()->GetControlPanel();
}

bool EffectLayerMask::GetVisible() const
{
    return m_visible;
}
