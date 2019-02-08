#include "EffectLayerMaskImplementationBrush.h"

#include "Bang/Camera.h"
#include "Bang/Framebuffer.h"
#include "Bang/GBuffer.h"
#include "Bang/GL.h"
#include "Bang/GLUniforms.h"
#include "Bang/GameObject.h"
#include "Bang/Input.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Texture2D.h"
#include "Bang/Transform.h"

#include "ControlPanel.h"
#include "EffectLayer.h"
#include "EffectLayerMask.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerMaskImplementationBrush::EffectLayerMaskImplementationBrush()
{
    m_paintMaskBrushSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "PaintMaskBrush.bushader")));
}

EffectLayerMaskImplementationBrush::~EffectLayerMaskImplementationBrush()
{
}

void EffectLayerMaskImplementationBrush::ReloadShaders()
{
    EffectLayerMaskImplementationGPU::ReloadShaders();
    m_paintMaskBrushSP.Get()->ReImport();
    Invalidate();
}

float EffectLayerMaskImplementationBrush::GetBrushSize() const
{
    return m_size;
}

void EffectLayerMaskImplementationBrush::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    ReflectVar<float>("Strength",
                      [this](float strength) {
                          m_strength = strength;
                          Invalidate();
                      },
                      [this]() { return m_strength; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));

    ReflectVar<float>("Hardness",
                      [this](float hardness) {
                          m_hardness = hardness;
                          Invalidate();
                      },
                      [this]() { return m_hardness; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));

    ReflectVar<float>("Size",
                      [this](float size) {
                          m_size = size;
                          Invalidate();
                      },
                      [this]() { return m_size; },
                      BANG_REFLECT_HINT_SLIDER(1.0f, 500.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(1.0f));

    ReflectVar<bool>("Depth aware",
                     [this](bool depthAware) {
                         m_depthAware = depthAware;
                         Invalidate();
                     },
                     [this]() { return m_depthAware; });

    ReflectVar<bool>("Erase",
                     [this](bool erase) {
                         m_erasing = erase;
                         Invalidate();
                     },
                     [this]() { return m_erasing; },
                     BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f) +
                         BANG_REFLECT_HINT_STEP_VALUE(0.1f));

    BANG_REFLECT_BUTTON(EffectLayerMaskImplementationBrush,
                        "Clear Mask",
                        [this]() { GetEffectLayerMask()->ClearMask(); });
    BANG_REFLECT_BUTTON(EffectLayerMaskImplementationBrush,
                        "Fill Mask",
                        [this]() { GetEffectLayerMask()->FillMask(); });
}

EffectLayerMask::Type
EffectLayerMaskImplementationBrush::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::BRUSH;
}

String EffectLayerMaskImplementationBrush::GetTypeName() const
{
    return "Brush";
}

Path EffectLayerMaskImplementationBrush::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Path::Empty();
}

void EffectLayerMaskImplementationBrush::SetGenerateEffectUniforms(
    ShaderProgram *,
    MeshRenderer *)
{
}

void EffectLayerMaskImplementationBrush::Update()
{
    EffectLayerMaskImplementationGPU::Update();

    // Generate needed textures
    if (Input::GetMouseButton(MouseButton::LEFT) &&
        GetControlPanel()->GetSelectedEffectLayerMask() == GetEffectLayerMask())
    {
        PaintMaskBrush();
    }

    if (Input::GetKeyDown(Key::LCTRL))
    {
        m_erasing = true;
    }

    if (Input::GetKeyUp(Key::LCTRL))
    {
        m_erasing = false;
    }

    if (Input::GetKey(Key::LSHIFT))
    {
        float maskBrushSizeIncrement = Input::GetMouseWheel().y;
        maskBrushSizeIncrement *= (m_size / 10);
        m_size += maskBrushSizeIncrement;
    }
}

ControlPanel *EffectLayerMaskImplementationBrush::GetControlPanel() const
{
    return GetEffectLayerMask()->GetControlPanel();
}

void EffectLayerMaskImplementationBrush::PaintMaskBrush()
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::CULL_FACE);

    Texture2D *maskTexture = GetEffectLayerMask()->GetMaskTexture();
    GL::SetViewport(0, 0, maskTexture->GetWidth(), maskTexture->GetHeight());

    ShaderProgram *sp = m_paintMaskBrushSP.Get();
    sp->Bind();
    {
        View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
        Transform *meshTR = GetEffectLayerMask()
                                ->GetEffectLayer()
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

        sp->SetBool("MaskBrushDepthAware", m_depthAware);
        sp->SetBool("MaskBrushErasing", m_erasing);
        sp->SetVector2("MaskBrushCenter", Vector2(Input::GetMousePosition()));
        sp->SetFloat("MaskBrushHardness", m_hardness);
        sp->SetFloat("MaskBrushSize", m_size);
        sp->SetFloat("MaskBrushStrength", m_strength);

        GetControlPanel()->SetControlPanelUniforms(m_paintMaskBrushSP.Get());
    }

    GL::Enable(GL::Enablable::BLEND);
    if (m_erasing)
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
    m_framebuffer->SetAttachmentTexture(maskTexture, GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    EffectLayer *effectLayer = GetEffectLayerMask()->GetEffectLayer();
    GL::Render(effectLayer->GetTextureMesh()->GetVAO(),
               GL::Primitive::TRIANGLES,
               effectLayer->GetTextureMesh()->GetNumVerticesIds());

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    Invalidate();
}
