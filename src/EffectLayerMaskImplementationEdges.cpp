#include "EffectLayerMaskImplementationEdges.h"

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

EffectLayerMaskImplementationEdges::EffectLayerMaskImplementationEdges()
{
    m_threshold = 10.0f;
    m_curvaturesTexture = Assets::Create<Texture2D>();
}

EffectLayerMaskImplementationEdges::~EffectLayerMaskImplementationEdges()
{
    // delete m_curvaturesVBO;
}

void EffectLayerMaskImplementationEdges::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    ReflectVar<float>("Threshold",
                      [this](float threshold) {
                          m_threshold = threshold;
                          Invalidate();
                      },
                      [this]() { return m_threshold; },
                      BANG_REFLECT_HINT_MIN_VALUE(0.0f));
}

Path EffectLayerMaskImplementationEdges::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureEdges.bushader");
}

EffectLayerMask::Type
EffectLayerMaskImplementationEdges::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::EDGES;
}

String EffectLayerMaskImplementationEdges::GetTypeName() const
{
    return "Edges";
}

void EffectLayerMaskImplementationEdges::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    if (!m_curvaturesCreated)
    {
        m_curvaturesCreated = true;

        meshRend->GetMesh()->UpdateVAOsAndTables();

        const int texSize = 1024;
        m_curvaturesTexture.Get()->CreateEmpty(Vector2i(texSize));
        m_curvaturesTexture.Get()->SetFilterMode(GL::FilterMode::NEAREST);
        m_curvaturesTexture.Get()->SetFormat(GL::ColorFormat::R16F);

        Array<float> curvatures(texSize * texSize, 0.0f);
        Mesh *mesh = meshRend->GetMesh();
        for (auto triId = 0; triId < mesh->GetNumTriangles(); ++triId)
        {
            for (auto i = 0; i < 3; ++i)
            {
                auto cornerId = triId * 3 + i;
                auto vertexId = mesh->GetVertexIdFromCornerId(cornerId);
                float curvature = mesh->GetVertexMeanCurvature(vertexId);
                curvatures[cornerId] = curvature;
                Debug_Peek(triId);
                Debug_Peek(cornerId);
                Debug_Peek(vertexId);
                Debug_Peek(curvature);
                Debug_Log("----");
            }
        }
        Debug_Log(mesh->GetNumTriangles());
        Debug_Log(mesh->GetNumVertices());
        Debug_Log(mesh->GetNumVerticesIds());
        m_curvaturesTexture.Get()->Fill(RCAST<Byte*>(curvatures.Data()), texSize, texSize,
                                        GL::ColorComp::R, GL::DataType::FLOAT);
        m_curvaturesTexture.Get()->ToImage().Export(Path("curvaturesTexture.png"));
    }

    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);
    sp->SetFloat("Threshold", m_threshold);
    sp->SetTexture2D("CurvaturesTexture", m_curvaturesTexture.Get());
}
