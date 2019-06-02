#include "EffectLayerMaskImplementationCracksNoise.h"

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

EffectLayerMaskImplementationCracksNoise::EffectLayerMaskImplementationCracksNoise()
{
    m_seed = Random::GetRange(0, 1000);
}

EffectLayerMaskImplementationCracksNoise::~EffectLayerMaskImplementationCracksNoise()
{
}

void EffectLayerMaskImplementationCracksNoise::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    ReflectVar<float>("Intensity",
                      [this](float intensity) {
                          m_intensity = intensity;
                          Invalidate();
                      },
                      [this]() { return m_intensity; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 2.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Size",
                      [this](float size) {
                          m_size = size;
                          Invalidate();
                      },
                      [this]() { return m_size; },
                      BANG_REFLECT_HINT_SLIDER(0.01f, 200.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Width",
                      [this](float width) {
                          m_width = width;
                          Invalidate();
                      },
                      [this]() { return m_width; },
                      BANG_REFLECT_HINT_SLIDER(0.01f, 30.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Fade",
                      [this](float fade) {
                          m_fade = fade;
                          Invalidate();
                      },
                      [this]() { return m_fade; },
                      BANG_REFLECT_HINT_SLIDER(1.2f, 30.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Seed",
                      [this](float seed) {
                          m_seed = seed;
                          Invalidate();
                      },
                      [this]() { return m_seed; },
                      BANG_REFLECT_HINT_MIN_VALUE(0.0f));
}

Path EffectLayerMaskImplementationCracksNoise::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureCracksNoise.bushader");
}

EffectLayerMask::Type
EffectLayerMaskImplementationCracksNoise::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::CRACKS_NOISE;
}

String EffectLayerMaskImplementationCracksNoise::GetTypeName() const
{
    return "CracksNoise";
}

void EffectLayerMaskImplementationCracksNoise::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);

    sp->SetFloat("Seed", m_seed);
    sp->SetFloat("Intensity", m_intensity);
    sp->SetFloat("Fade", m_fade);
    sp->SetFloat("Width", m_width);
    sp->SetFloat("Size", m_size);
}
