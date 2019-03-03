#include "EffectLayerMaskImplementationNormal.h"

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

EffectLayerMaskImplementationNormal::EffectLayerMaskImplementationNormal()
{
}

EffectLayerMaskImplementationNormal::~EffectLayerMaskImplementationNormal()
{
}

void EffectLayerMaskImplementationNormal::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationNormal,
                                   "Normal",
                                   SetNormal,
                                   GetNormal,
                                   BANG_REFLECT_HINT_MINMAX_VALUE(-1.0f, 1.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationNormal,
                                   "Intensity",
                                   SetIntensity,
                                   GetIntensity,
                                   BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));

    BANG_REFLECT_VAR_MEMBER_HINTED(EffectLayerMaskImplementationNormal,
                                   "Fadeout",
                                   SetFadeout,
                                   GetFadeout,
                                   BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));
}

void EffectLayerMaskImplementationNormal::SetIntensity(float intensity)
{
    if (intensity != GetIntensity())
    {
        m_intensity = intensity;
        Invalidate();
    }
}

void EffectLayerMaskImplementationNormal::SetFadeout(float fadeout)
{
    if (fadeout != GetFadeout())
    {
        m_fadeout = fadeout;
        Invalidate();
    }
}

void EffectLayerMaskImplementationNormal::SetNormal(const Vector3 &normal)
{
    if (normal != GetNormal())
    {
        m_normal = normal;
        Invalidate();
    }
}

float EffectLayerMaskImplementationNormal::GetFadeout() const
{
    return m_fadeout;
}

float EffectLayerMaskImplementationNormal::GetIntensity() const
{
    return m_intensity;
}

const Vector3 &EffectLayerMaskImplementationNormal::GetNormal() const
{
    return m_normal;
}

Path EffectLayerMaskImplementationNormal::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureNormal.bushader");
}

EffectLayerMask::Type
EffectLayerMaskImplementationNormal::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::NORMAL;
}

String EffectLayerMaskImplementationNormal::GetTypeName() const
{
    return "Normal";
}

void EffectLayerMaskImplementationNormal::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);

    sp->SetVector3("Normal", GetNormal());
    sp->SetFloat("Intensity", GetIntensity());
    sp->SetFloat("Fadeout", GetFadeout());
}
