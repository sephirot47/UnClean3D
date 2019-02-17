#include "EffectLayerMaskImplementationAmbientOcclusion.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/Mesh.h"
#include "Bang/MeshRenderer.h"
#include "Bang/Paths.h"
#include "Bang/ShaderProgram.h"
#include "Bang/Transform.h"

#include "View3DScene.h"

using namespace Bang;

EffectLayerMaskImplementationAmbientOcclusion::
    EffectLayerMaskImplementationAmbientOcclusion()
{
}

EffectLayerMaskImplementationAmbientOcclusion::
    ~EffectLayerMaskImplementationAmbientOcclusion()
{
}

void EffectLayerMaskImplementationAmbientOcclusion::SetNumRays(int numRays)
{
    if (numRays != GetNumRays())
    {
        m_numRays = numRays;
    }
}

void EffectLayerMaskImplementationAmbientOcclusion::SetMaxDistance(
    float maxDistance)
{
    if (maxDistance != GetMaxDistance())
    {
        m_maxDistance = maxDistance;
    }
}

int EffectLayerMaskImplementationAmbientOcclusion::GetNumRays() const
{
    return m_numRays;
}

float EffectLayerMaskImplementationAmbientOcclusion::GetMaxDistance() const
{
    return m_maxDistance;
}

void EffectLayerMaskImplementationAmbientOcclusion::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationAmbientOcclusion,
        "Num Rays",
        SetNumRays,
        GetNumRays,
        BANG_REFLECT_HINT_SLIDER(0.0f, 50.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(
        EffectLayerMaskImplementationAmbientOcclusion,
        "Max distance",
        SetMaxDistance,
        GetMaxDistance,
        BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));
}

EffectLayerMask::Type
EffectLayerMaskImplementationAmbientOcclusion::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::AMBIENT_OCCLUSION;
}

String EffectLayerMaskImplementationAmbientOcclusion::GetTypeName() const
{
    return "Ambient Occlusion";
}

Path EffectLayerMaskImplementationAmbientOcclusion::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureAmbientOcclusion.bushader");
}

void EffectLayerMaskImplementationAmbientOcclusion::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *mr)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, mr);

    GLSLRayCaster *rayCaster = View3DScene::GetInstance()->GetGLSLRayCaster();
    rayCaster->Bind(sp);
}

bool EffectLayerMaskImplementationAmbientOcclusion::
    CanGenerateEffectMaskTextureInRealTime() const
{
    return false;
}
