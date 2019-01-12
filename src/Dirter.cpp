#include "Dirter.h"

#include "Bang/Array.h"
#include "Bang/Assets.h"
#include "Bang/Dialog.h"
#include "Bang/Framebuffer.h"
#include "Bang/GameObject.h"
#include "Bang/Image.h"
#include "Bang/Material.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
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

using namespace Bang;

Dirter::Dirter(MeshRenderer *mr)
{
    p_meshRenderer = mr;

    m_framebuffer = new Framebuffer();

    m_createDirtShaderProgram.Set(ShaderProgramFactory::Get(
        Paths::GetProjectAssetsDir().Append("CreateDirtShader.bushader")));

    // Create textures
    m_dirtTexture = Assets::Create<Texture2D>();
    Texture2D *albedoTex = mr->GetMaterial()->GetAlbedoTexture();
    if (albedoTex)
    {
        const uint albedoTexW = albedoTex->GetWidth();
        const uint albedoTexH = albedoTex->GetHeight();
        GetDirtTexture()->Fill(Color::Zero(), albedoTexW, albedoTexH);
    }

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

Dirter::~Dirter()
{
    delete m_framebuffer;
}

void Dirter::CreateDirtTexture()
{
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Push(GL::Pushable::VIEWPORT);
    GL::Push(GL::Pushable::SHADER_PROGRAM);

    // Bind framebuffer and render to texture
    m_framebuffer->Bind();
    m_framebuffer->SetAttachmentTexture(GetDirtTexture(),
                                        GL::Attachment::COLOR0);
    m_framebuffer->SetAllDrawBuffers();
    // GL::ClearColorBuffer(Color::Zero());

    GL::SetViewport(
        0, 0, GetDirtTexture()->GetWidth(), GetDirtTexture()->GetHeight());

    ShaderProgram *sp = m_createDirtShaderProgram.Get();
    sp->Bind();
    sp->SetFloat("DirtOctaves", GetControlPanel()->GetDirtOctaves());
    sp->SetFloat("DirtFrequency", GetControlPanel()->GetDirtFrequency());
    sp->SetFloat("DirtFrequencyMultiply",
                 GetControlPanel()->GetDirtFrequencyMultiply());
    sp->SetFloat("DirtAmplitude", GetControlPanel()->GetDirtAmplitude());
    sp->SetFloat("DirtAmplitudeMultiply",
                 GetControlPanel()->GetDirtAmplitudeMultiply());
    sp->SetFloat("DirtSeed", GetControlPanel()->GetDirtSeed());

    GL::Render(GetTextureMesh()->GetVAO(),
               GL::Primitive::TRIANGLES,
               GetTextureMesh()->GetNumVerticesIds());

    sp->UnBind();

    m_framebuffer->UnBind();

    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Pop(GL::Pushable::VIEWPORT);
    GL::Pop(GL::Pushable::SHADER_PROGRAM);

    // GetDirtTexture()->ToImage().Export(Path("test.png"));
}

void Dirter::ReloadShaders()
{
    m_createDirtShaderProgram.Get()->ReImport();
}

Texture2D *Dirter::GetDirtTexture() const
{
    return m_dirtTexture.Get();
}

Mesh *Dirter::GetTextureMesh() const
{
    return m_textureMesh.Get();
}

ControlPanel *Dirter::GetControlPanel() const
{
    return MainScene::GetInstance()->GetControlPanel();
}
