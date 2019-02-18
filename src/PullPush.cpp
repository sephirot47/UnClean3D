#include "PullPush.h"

#include "Bang/Assets.h"
#include "Bang/Framebuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ShaderProgramFactory.h"
#include "Bang/Texture2D.h"
#include "Bang/TextureFactory.h"

#include "View3DScene.h"

using namespace Bang;

PullPush::PullPush()
{
    m_framebuffer = new Framebuffer();

    m_pullPushPrepareTextureSP.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("Shaders").Append(
            "PullPushPrepareTexture.bushader")));

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
    m_pullPushPrepareTextureSP.Get()->ReImport();
}

void PullPush::PullPushTexture(Texture2D *originalTextureToPullPush)
{
    if (!originalTextureToPullPush)
    {
        return;
    }

    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::SHADER_PROGRAM);
    GL::Push(GL::Pushable::BLEND_STATES);
    GL::Push(GL::Pushable::CULL_FACE);
    GL::Push(GL::Pushable::VIEWPORT);

    GL::Disable(GL::Enablable::BLEND);
    GL::Disable(GL::Enablable::CULL_FACE);

    Vector2i originalTexSize = originalTextureToPullPush->GetSize();

    m_framebuffer->Bind();

    enum ShaderPhase
    {
        PULLING = 0,
        PUSHING = 1
    };

    // Prepare texture
    m_pullPushPrepareTextureSP.Get()->Bind();
    m_pullPushPrepareTextureSP.Get()->SetTexture2D("TextureToPullPush",
                                                   originalTextureToPullPush);
    {
        Mesh *textureMesh = View3DScene::GetInstance()->GetTextureMesh();
        GL::SetViewport(0, 0, originalTexSize.x, originalTexSize.y);

        Texture2D *drawTexture = GetPullTexture(originalTexSize.x);
        m_framebuffer->SetAttachmentTexture(drawTexture,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        GL::ClearColorBuffer(Color::Zero());
        GL::Render(textureMesh->GetVAO(),
                   GL::Primitive::TRIANGLES,
                   textureMesh->GetNumVerticesIds());
    }

    // Pull
    Vector2i currentTexSize = Vector2i(originalTexSize);

    m_pullPushSP.Get()->Bind();
    m_pullPushSP.Get()->SetVector2(
        "TextureToPullPushSize", Vector2(originalTextureToPullPush->GetSize()));
    m_pullPushSP.Get()->SetVector2("OriginalTextureToPullPushTexelSize",
                                   (1.0f / Vector2(originalTexSize)));
    m_pullPushSP.Get()->SetInt("ShaderPhase", PULLING);

    m_pullPushSP.Get()->SetTexture2D("CurrentPulledTexture",
                                     TextureFactory::GetWhiteTexture());
    m_pullPushSP.Get()->SetTexture2D("PreviousPulledTexture",
                                     TextureFactory::GetWhiteTexture());
    m_pullPushSP.Get()->SetTexture2D("PreviousPushedTexture",
                                     TextureFactory::GetWhiteTexture());

    for (int i = 0; i < 99999; ++i)
    {
        currentTexSize /= 2;
        if (currentTexSize.x <= 0)
        {
            break;
        }

        Texture2D *drawTexture = GetPullTexture(currentTexSize.x);
        GL::SetViewport(0, 0, currentTexSize.x, currentTexSize.y);

        m_framebuffer->SetAttachmentTexture(drawTexture,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        Vector2i prevIterTexSize = (currentTexSize * 2);
        m_pullPushSP.Get()->SetTexture2D("PreviousPulledTexture",
                                         GetPullTexture(prevIterTexSize.x));

        GEngine::GetInstance()->RenderViewportPlane();
    }

    // Push
    currentTexSize = Vector2i(1);
    m_pullPushSP.Get()->SetInt("ShaderPhase", PUSHING);
    for (int i = 0; i < 99999; ++i)
    {
        if (currentTexSize.x > originalTexSize.x)
        {
            break;
        }

        currentTexSize *= 2;
        GL::SetViewport(0, 0, currentTexSize.x, currentTexSize.y);

        Texture2D *drawTexture = GetPushTexture(currentTexSize.x);
        m_framebuffer->SetAttachmentTexture(drawTexture,
                                            GL::Attachment::COLOR0);
        m_framebuffer->SetDrawBuffers({GL::Attachment::COLOR0});

        Vector2i prevIterTexSize = (currentTexSize / 2);
        m_pullPushSP.Get()->SetTexture2D("CurrentPulledTexture",
                                         GetPullTexture(currentTexSize.x));
        m_pullPushSP.Get()->SetTexture2D("PreviousPushedTexture",
                                         GetPushTexture(prevIterTexSize.x));

        GEngine::GetInstance()->RenderViewportPlane();
    }

    GEngine::GetInstance()->CopyTexture(GetPushTexture(originalTexSize.x),
                                        originalTextureToPullPush);

    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::CULL_FACE);
    GL::Pop(GL::Pushable::BLEND_STATES);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
}

Texture2D *PullPush::GetPullPushTexture(int size, bool pull)
{
    auto &textureMap = (pull ? m_pullTextures : m_pushTextures);
    if (!textureMap.ContainsKey(size))
    {
        AH<Texture2D> texture = Assets::Create<Texture2D>();
        texture.Get()->Fill(Color::Zero(), size, size);
        texture.Get()->SetFormat(GL::ColorFormat::RGBA8);
        texture.Get()->SetFilterMode(GL::FilterMode::BILINEAR);

        textureMap.Add(size, texture);
    }
    return textureMap.Get(size).Get();
}
Texture2D *PullPush::GetPullTexture(int size)
{
    return GetPullPushTexture(size, true);
}
Texture2D *PullPush::GetPushTexture(int size)
{
    return GetPullPushTexture(size, false);
}
