#include "EffectLayer.h"

#include "Bang/Array.h"
#include "Bang/Assets.h"
#include "Bang/Camera.h"
#include "Bang/Dialog.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
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
#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayer::EffectLayer(MeshRenderer *mr)
{
    p_meshRenderer = mr;

    m_framebuffer = new Framebuffer();

    // Create effect texture
    m_effectTexture = Assets::Create<Texture2D>();

    // Create mask texture
    m_maskPingPongTexture0 = Assets::Create<Texture2D>();
    m_maskPingPongTexture1 = Assets::Create<Texture2D>();
    m_maskPingPongTexture0.Get()->Fill(Color::Zero(), 1, 1);
    m_maskPingPongTexture1.Get()->Fill(Color::Zero(), 1, 1);
    p_lastDrawnMaskTexture = m_maskPingPongTexture0.Get();

    // ShaderPrograms...
    m_paintMaskBrushSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "PaintMaskBrush.bushader")));

    // Create texture mesh
    m_textureMesh = Assets::Create<Mesh>();
    if (Mesh *originalMesh = mr->GetMesh())
    {
        m_positionsVBO = new VBO();

        // Gather some data
        Array<Vector3> texTriMeshPositions;
        Array<Vector3> originalVertexPositions;
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
                originalVertexPositions.PushBack(oriVertPos);
            }
        }

        // Set original mesh uvs as texture mesh positions
        GetTextureMesh()->SetPositionsPool(texTriMeshPositions);
        GetTextureMesh()->SetTrianglesVertexIds({});

        // Add actual original mesh 3D positions as another VBO
        m_positionsVBO->CreateAndFill(
            originalVertexPositions.Data(),
            originalVertexPositions.Size() * sizeof(float) * 3);
        GetTextureMesh()->GetVAO()->SetVBO(
            m_positionsVBO, 1, 3, GL::VertexAttribDataType::FLOAT);

        GetTextureMesh()->UpdateVAOs();
    }
}

EffectLayer::~EffectLayer()
{
    delete m_framebuffer;
}

void EffectLayer::GenerateEffectTexture()
{
    if (!GetImplementation())
    {
        return;
    }

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::VIEWPORT);
    GL::Push(GL::Pushable::SHADER_PROGRAM);

    // Bind framebuffer and render to texture
    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(GetEffectTexture(),
                                        GL::Attachment::COLOR0);
    m_framebuffer->SetAllDrawBuffers();

    Vector2i texSize = GetControlPanel()->GetTextureSize();
    GetEffectTexture()->Resize(texSize);
    GL::SetViewport(0, 0, texSize.x, texSize.y);

    ShaderProgram *sp = GetGenerateEffectTextureShaderProgram();
    sp->Bind();
    GetImplementation()->SetGenerateEffectUniforms(sp);

    GL::Render(GetTextureMesh()->GetVAO(),
               GL::Primitive::TRIANGLES,
               GetTextureMesh()->GetNumVerticesIds());

    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
}

void EffectLayer::ReloadShaders()
{
    m_generateEffectTextureSP.Get()->ReImport();
    m_paintMaskBrushSP.Get()->ReImport();
}

void EffectLayer::UpdateParameters(const EffectLayerParameters &parameters)
{
    m_params = parameters;
    GenerateEffectTexture();
}

void EffectLayer::SetEffectLayerImplementation(EffectLayerImplementation *impl)
{
    if (GetImplementation())
    {
        delete p_implementation;
    }

    p_implementation = impl;

    if (GetImplementation())
    {
        GetImplementation()->p_effectLayer = this;

        Path spPath =
            GetImplementation()->GetGenerateEffectTextureShaderProgramPath();
        m_generateEffectTextureSP.Set(ShaderProgramFactory::Get(spPath));

        GenerateEffectTexture();
    }
}

void EffectLayer::PaintMaskBrush()
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::VIEWPORT);

    m_maskPingPongTexture0.Get()->ResizeConservingData(
        GetEffectTexture()->GetWidth(), GetEffectTexture()->GetHeight());
    m_maskPingPongTexture1.Get()->ResizeConservingData(
        GetEffectTexture()->GetWidth(), GetEffectTexture()->GetHeight());
    GL::SetViewport(
        0, 0, GetMaskTexture()->GetWidth(), GetMaskTexture()->GetHeight());

    Texture2D *drawTexture =
        p_lastDrawnMaskTexture == m_maskPingPongTexture0.Get()
            ? m_maskPingPongTexture1.Get()
            : m_maskPingPongTexture0.Get();
    Texture2D *readTexture = drawTexture == m_maskPingPongTexture0.Get()
                                 ? m_maskPingPongTexture1.Get()
                                 : m_maskPingPongTexture0.Get();

    ShaderProgram *sp = m_paintMaskBrushSP.Get();
    sp->Bind();
    {
        View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
        sp->SetTexture2D("PreviousMaskTexture", readTexture);
        sp->SetMatrix4("SceneModelMatrix",
                       view3DScene->GetModelGameObject()
                           ->GetTransform()
                           ->GetLocalToWorldMatrix());
        sp->SetMatrix4("SceneProjectionViewMatrix",
                       view3DScene->GetCamera()->GetProjectionMatrix() *
                           view3DScene->GetCamera()->GetViewMatrix());
        sp->SetVector2("ViewportSize",
                       Vector2(view3DScene->GetCamera()->GetRenderSize()));
        GetControlPanel()->SetMaskUniforms(m_paintMaskBrushSP.Get());
    }

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(drawTexture, GL::Attachment::COLOR0);
    m_framebuffer->SetAllDrawBuffers();

    GL::Render(GetTextureMesh()->GetVAO(),
               GL::Primitive::TRIANGLES,
               GetTextureMesh()->GetNumVerticesIds());

    p_lastDrawnMaskTexture = drawTexture;

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

void EffectLayer::ClearMask()
{
    GEngine *ge = GEngine::GetInstance();
    ge->FillTexture(m_maskPingPongTexture0.Get(), Color::Zero());
    ge->FillTexture(m_maskPingPongTexture1.Get(), Color::Zero());
}

Mesh *EffectLayer::GetTextureMesh() const
{
    return m_textureMesh.Get();
}

const EffectLayerParameters &EffectLayer::GetParameters() const
{
    return m_params;
}

Texture2D *EffectLayer::GetEffectTexture() const
{
    return m_effectTexture.Get();
}

Texture2D *EffectLayer::GetMaskTexture() const
{
    return m_maskPingPongTexture0.Get();
}

ControlPanel *EffectLayer::GetControlPanel() const
{
    return MainScene::GetInstance()->GetControlPanel();
}

EffectLayerImplementation *EffectLayer::GetImplementation() const
{
    return p_implementation;
}

ShaderProgram *EffectLayer::GetGenerateEffectTextureShaderProgram() const
{
    return m_generateEffectTextureSP.Get();
}

EffectLayerImplementation::EffectLayerImplementation()
{
}

EffectLayerImplementation::~EffectLayerImplementation()
{
}

void EffectLayerImplementation::SetGenerateEffectUniforms(ShaderProgram *)
{
}

EffectLayer *EffectLayerImplementation::GetEffectLayer() const
{
    return p_effectLayer;
}
