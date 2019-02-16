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

void EffectLayerMaskImplementationAmbientOcclusion::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();
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
