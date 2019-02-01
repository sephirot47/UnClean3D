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
#include "EffectLayerAmbientOcclusion.h"
#include "EffectLayerAmbientOcclusionGPU.h"
#include "EffectLayerDirt.h"
#include "EffectLayerFractalBumps.h"
#include "EffectLayerImplementation.h"
#include "EffectLayerMask.h"
#include "EffectLayerNormalLines.h"
#include "EffectLayerWaveBumps.h"
#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayer::EffectLayer(MeshRenderer *mr)
{
    p_meshRenderer = mr;

    m_framebuffer = new Framebuffer();

    // Create some textures
    m_effectTexture = Assets::Create<Texture2D>();
    m_effectTexture.Get()->Fill(Color::Zero(), 1, 1);
    m_effectTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_growAuxiliarTexture = Assets::Create<Texture2D>();
    m_growAuxiliarTexture.Get()->Fill(Color::Zero(), 1, 1);
    m_effectTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    // Create mask textures
    m_mergedMaskTexture = Assets::Create<Texture2D>();
    m_mergedMaskTexture.Get()->Fill(Color::Zero(), 1, 1);

    // ShaderPrograms...
    m_paintMaskBrushSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "PaintMaskBrush.bushader")));
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

    SetType(EffectLayer::Type::DIRT);
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
    if (EffectLayerImplementation *impl = GetImplementation())
    {
        Vector2i texSize = GetControlPanel()->GetTextureSize();
        GetEffectTexture()->ResizeConservingData(texSize.x, texSize.y);

        impl->GenerateEffectTexture(GetEffectTexture(), p_meshRenderer);
        GrowTextureBorders(GetEffectTexture());
    }
}

void EffectLayer::MergeMasks()
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Enable(GL::Enablable::BLEND);
    GL::BlendFunc(GL::BlendFactor::ONE, GL::BlendFactor::ONE);

    Vector2i size = GetEffectTexture()->GetSize();
    GL::SetViewport(0, 0, size.x, size.y);

    GetMergedMaskTexture()->ResizeConservingData(size.x, size.y);

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(GetMergedMaskTexture(),
                                        GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    GL::ClearColorBuffer(Color::Zero());

    for (EffectLayerMask *mask : GetMasks())
    {
        mask->GetMaskTexture()->ResizeConservingData(size.x, size.y);
        GEngine::GetInstance()->RenderTexture(mask->GetMaskTexture());
    }

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

void EffectLayer::ReloadShaders()
{
    if (EffectLayerImplementation *impl = GetImplementation())
    {
        impl->ReloadShaders();
    }
    m_paintMaskBrushSP.Get()->ReImport();
    m_growTextureBordersSP.Get()->ReImport();

    if (GetImplementation() &&
        GetImplementation()->CanGenerateEffectTextureInRealTime())
    {
        GenerateEffectTexture();
    }
}

void EffectLayer::SetImplementation(EffectLayerImplementation *impl)
{
    if (GetImplementation())
    {
        delete p_implementation;
    }

    p_implementation = impl;

    if (GetImplementation())
    {
        GetImplementation()->p_effectLayer = this;
        MainScene::GetInstance()->GetView3DScene()->InvalidateTextures();
    }
}

void EffectLayer::SetType(EffectLayer::Type type)
{
    EffectLayerImplementation *effLayerImpl = GetImplementation();
    if (!effLayerImpl || GetType() != type)
    {
        EffectLayerImplementation *newImpl = nullptr;
        switch (type)
        {
            case EffectLayer::Type::DIRT:
                newImpl = new EffectLayerDirt();
                break;

            case EffectLayer::Type::NORMAL_LINES:
                newImpl = new EffectLayerNormalLines();
                break;

            case EffectLayer::Type::FRACTAL_BUMPS:
                newImpl = new EffectLayerFractalBumps();
                break;

            case EffectLayer::Type::WAVE_BUMPS:
                newImpl = new EffectLayerWaveBumps();
                break;

            case EffectLayer::Type::AMBIENT_OCCLUSION:
                newImpl = new EffectLayerAmbientOcclusion();
                break;

            case EffectLayer::Type::AMBIENT_OCCLUSION_GPU:
                newImpl = new EffectLayerAmbientOcclusionGPU();
                break;

            default: ASSERT(false);
        }
        m_type = type;
        newImpl->Init();
        SetImplementation(newImpl);
    }
}

void EffectLayer::SetVisible(bool visible)
{
    m_visible = visible;
}

EffectLayerMask *EffectLayer::AddNewMask()
{
    EffectLayerMask *newMask = new EffectLayerMask();
    newMask->SetEffectLayer(this);
    m_masks.PushBack(newMask);
    return newMask;
}

void EffectLayer::PaintMaskBrush(Texture2D *maskTexture)
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::CULL_FACE);

    GL::SetViewport(0, 0, maskTexture->GetWidth(), maskTexture->GetHeight());

    ShaderProgram *sp = m_paintMaskBrushSP.Get();
    sp->Bind();
    {
        View3DScene *view3DScene = MainScene::GetInstance()->GetView3DScene();
        Transform *meshTR = p_meshRenderer->GetGameObject()->GetTransform();
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

    GL::Render(GetTextureMesh()->GetVAO(),
               GL::Primitive::TRIANGLES,
               GetTextureMesh()->GetNumVerticesIds());

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

bool EffectLayer::GetVisible() const
{
    return m_visible;
}

Mesh *EffectLayer::GetMesh() const
{
    return p_meshRenderer ? p_meshRenderer->GetMesh() : nullptr;
}

EffectLayer::Type EffectLayer::GetType() const
{
    return m_type;
}

Mesh *EffectLayer::GetTextureMesh() const
{
    return m_textureMesh.Get();
}

Texture2D *EffectLayer::GetMergedMaskTexture() const
{
    return m_mergedMaskTexture.Get();
}

Texture2D *EffectLayer::GetEffectTexture() const
{
    return m_effectTexture.Get();
}

const Array<EffectLayerMask *> &EffectLayer::GetMasks() const
{
    return m_masks;
}

ControlPanel *EffectLayer::GetControlPanel() const
{
    return MainScene::GetInstance()->GetControlPanel();
}

EffectLayerImplementation *EffectLayer::GetImplementation() const
{
    return p_implementation;
}

void EffectLayer::Reflect()
{
    Serializable::Reflect();

    BANG_REFLECT_VAR_MEMBER(EffectLayer, "Visible", SetVisible, GetVisible);
    BANG_REFLECT_VAR_ENUM("Type", SetType, GetType, EffectLayer::Type);
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
    {
        sp->SetTexture2D("TextureToGrow", m_growAuxiliarTexture.Get());
    }
    GL::SetViewport(0, 0, texture->GetWidth(), texture->GetHeight());

    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(texture, GL::Attachment::COLOR0);
    m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

    GL::ClearColorBuffer(Color::Zero());
    GEngine::GetInstance()->RenderViewportPlane();

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}
