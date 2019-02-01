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
#include "EffectLayerMaskImplementationFractal.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerMask::EffectLayerMask()
{
    m_maskTexture = Assets::Create<Texture2D>();
    m_maskTexture.Get()->Fill(Color::Zero(), 1, 1);

    m_framebuffer = new Framebuffer();

    m_paintMaskBrushSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "PaintMaskBrush.bushader")));

    SetType(EffectLayerMask::Type::FRACTAL);
}

EffectLayerMask::~EffectLayerMask()
{
    delete m_framebuffer;
}

void EffectLayerMask::Update()
{
    // Generate needed textures
    if (Input::GetMouseButton(MouseButton::LEFT) &&
        GetControlPanel()->GetSelectedEffectLayerMask() == this)
    {
        PaintMaskBrush();
    }
}

void EffectLayerMask::ReloadShaders()
{
    m_paintMaskBrushSP.Get()->ReImport();
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
        }
    }
}

void EffectLayerMask::GenerateEffectMaskTexture() const
{
    if (EffectLayerMaskImplementation *impl = GetImplementation())
    {
        MeshRenderer *mr = GetEffectLayer()->GetMeshRenderer();
        impl->GenerateEffectMaskTexture(GetMaskTexture(), mr);
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

void EffectLayerMask::Invalidate()
{
    m_isValid = false;
    GetEffectLayer()->Invalidate();
}

void EffectLayerMask::PaintMaskBrush()
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::CULL_FACE);

    GL::SetViewport(
        0, 0, GetMaskTexture()->GetWidth(), GetMaskTexture()->GetHeight());

    ShaderProgram *sp = m_paintMaskBrushSP.Get();
    sp->Bind();
    {
        View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
        Transform *meshTR = GetEffectLayer()
                                ->GetMeshRenderer()
                                ->GetGameObject()
                                ->GetTransform();
        sp->SetMatrix4("SceneModelMatrix", meshTR->GetLocalToWorldMatrix());
        sp->SetMatrix4(
            "SceneNormalMatrix",
            GLUniforms::CalculateNormalMatrix(meshTR->GetLocalToWorldMatrix()));
        Camera *cam = view3DScene->GetCamera();
        sp->SetMatrix4("SceneProjectionViewMatrix",
                       cam->GetProjectionMatrix() * cam->GetViewMatrix());
        sp->SetVector2("ViewportSize", Vector2(cam->GetRenderSize()));
        sp->SetTexture2D("SceneDepth",
                         cam->GetGBuffer()->GetSceneDepthStencilTexture());
        sp->SetVector3("CameraWorldPos",
                       cam->GetGameObject()->GetTransform()->GetPosition());
        sp->SetTexture2D(
            "SceneNormalTexture",
            cam->GetGBuffer()->GetAttachmentTex2D(GBuffer::AttNormal));
        GetControlPanel()->SetControlPanelUniforms(m_paintMaskBrushSP.Get());
    }

    GL::Enable(GL::Enablable::BLEND);
    if (GetControlPanel()->GetMaskBrushErasing())
    {
        GL::BlendEquation(GL::BlendEquationE::FUNC_REVERSE_SUBTRACT);
        GL::BlendFunc(GL::BlendFactor::ONE, GL::BlendFactor::ONE);
    }
    else
    {
        GL::BlendEquation(GL::BlendEquationE::FUNC_ADD);
        GL::BlendFunc(GL::BlendFactor::ONE, GL::BlendFactor::ONE);
    }

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(GetMaskTexture(),
                                        GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    GL::Render(GetEffectLayer()->GetTextureMesh()->GetVAO(),
               GL::Primitive::TRIANGLES,
               GetEffectLayer()->GetTextureMesh()->GetNumVerticesIds());

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    Invalidate();
}

const String &EffectLayerMask::GetName() const
{
    return m_name;
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
