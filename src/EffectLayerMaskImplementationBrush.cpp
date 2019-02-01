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
}

void EffectLayerMaskImplementationBrush::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();
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
    m_framebuffer->SetAttachmentTexture(maskTexture, GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    GL::Render(
        GetEffectLayerMask()->GetEffectLayer()->GetTextureMesh()->GetVAO(),
        GL::Primitive::TRIANGLES,
        GetEffectLayerMask()
            ->GetEffectLayer()
            ->GetTextureMesh()
            ->GetNumVerticesIds());

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    Invalidate();
}
