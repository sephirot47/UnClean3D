#include "PullPush.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Texture2D.h"

#include "View3DScene.h"

using namespace Bang;

PullPush::PullPush()
{
    m_framebuffer = new Framebuffer();

    m_informationMaskTexture = Assets::Create<Texture2D>();
    m_informationMaskTexture.Get()->Fill(Color::Zero(), 1, 1);
    m_informationMaskTexture.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_texture0 = Assets::Create<Texture2D>();
    m_texture0.Get()->Fill(Color::Black(), 1, 1);
    m_texture0.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_texture1 = Assets::Create<Texture2D>();
    m_texture1.Get()->Fill(Color::Black(), 1, 1);
    m_texture1.Get()->SetFormat(GL::ColorFormat::RGBA8);

    m_pullPushSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "PullPush.bushader")));
}

PullPush::~PullPush()
{
}

void PullPush::ReloadShaders()
{
    m_pullPushSP.Get()->ReImport();
}

void PullPush::PullPushTexture(Texture2D *textureToPullPush)
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::BLEND);
    GL::Disable(GL::Enablable::CULL_FACE);

    Vector2i texSize = textureToPullPush->GetSize();
    m_texture0.Get()->Resize(texSize);
    m_texture1.Get()->Resize(texSize);
    m_informationMaskTexture.Get()->Resize(texSize);
    Texture2D *drawTexture = m_texture0.Get();
    Texture2D *readTexture = m_texture1.Get();

    m_framebuffer->Bind();

    m_pullPushSP.Get()->Bind();
    m_pullPushSP.Get()->SetTexture2D("InformationMask",
                                     m_informationMaskTexture.Get());
    m_pullPushSP.Get()->SetVector2("TextureToPullPushSize",
                                   Vector2(textureToPullPush->GetSize()));
    m_pullPushSP.Get()->SetVector2("TexelSize", (1.0f / Vector2(texSize)));

    int pullPushIterations = SCAST<int>(Math::Log(texSize.x) / Math::Log(2.0f));

    GEngine::GetInstance()->CopyTexture(textureToPullPush, readTexture);
    GEngine::GetInstance()->FillTexture(drawTexture, Color::Zero());

    enum ShaderPhase
    {
        MARKING_INFO_ZONES = 0,
        PULLING = 1,
        PUSHING = 2
    };

    // Mark zones with information
    {
        Mesh *textureMesh = View3DScene::GetInstance()->GetTextureMesh();
        GL::SetViewport(0, 0, texSize.x, texSize.y);

        m_framebuffer->SetAttachmentTexture(m_informationMaskTexture.Get(),
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        GL::ClearColorBuffer(Color::Zero());

        m_pullPushSP.Get()->SetInt("ShaderPhase", MARKING_INFO_ZONES);
        GL::Render(textureMesh->GetVAO(),
                   GL::Primitive::TRIANGLES,
                   textureMesh->GetNumVerticesIds());
    }

    // Pull
    Vector2 currentMinPoint = Vector2::Zero();
    Vector2 currentTexSize = Vector2(texSize);
    m_pullPushSP.Get()->SetInt("ShaderPhase", PULLING);
    for (int i = 0; i < pullPushIterations; ++i)
    {
        currentTexSize /= 2.0f;
        GL::SetViewport(
            currentMinPoint.x, 0, currentTexSize.x, currentTexSize.y);

        m_framebuffer->SetAttachmentTexture(drawTexture,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        Vector2 prevIterTexSize = (currentTexSize * 2.0f);
        Vector2 prevIterTexMin = (currentMinPoint - prevIterTexSize.x);
        m_pullPushSP.Get()->SetInt("Iteration", i);
        m_pullPushSP.Get()->SetVector2("PreviousIterationTextureSize",
                                       prevIterTexSize);
        m_pullPushSP.Get()->SetVector2("PreviousIterationTextureMin",
                                       prevIterTexMin);
        m_pullPushSP.Get()->SetVector2("IterationTextureSize", currentTexSize);
        m_pullPushSP.Get()->SetTexture2D("TextureToPullPush", readTexture);

        GEngine::GetInstance()->RenderViewportPlane();

        currentMinPoint.x += currentTexSize.x;

        GEngine::GetInstance()->CopyTexture(drawTexture, readTexture);
        std::swap(drawTexture, readTexture);
    }

    /*
    // Push
    currTexSize = texSize;
    m_pullPushSP.Get()->SetInt("ShaderPhase", PUSHING);
    for (int i = 0; i < pullPushIterations; ++i)
    {
        currTexSize /= 2.0f;
        GL::SetViewport(0, 0, currTexSize.x, currTexSize.y);

        m_framebuffer->SetAttachmentTexture(drawTexture,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        m_pullPushSP.Get()->SetInt("Iteration", i);
        m_pullPushSP.Get()->SetVector2("IterationTextureSize",
                                       Vector2(currTexSize));
        m_pullPushSP.Get()->SetTexture2D("TextureToPullPush", readTexture);

        GEngine::GetInstance()->RenderViewportPlane();

        std::swap(drawTexture, readTexture);
    }
    */

    GEngine::GetInstance()->CopyTexture(readTexture, textureToPullPush);

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}
