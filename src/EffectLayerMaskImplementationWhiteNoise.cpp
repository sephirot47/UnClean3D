#include "EffectLayerMaskImplementationWhiteNoise.h"

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

EffectLayerMaskImplementationWhiteNoise::EffectLayerMaskImplementationWhiteNoise()
{
    m_seed = Random::GetRange(0, 100);
}

EffectLayerMaskImplementationWhiteNoise::~EffectLayerMaskImplementationWhiteNoise()
{
}

void EffectLayerMaskImplementationWhiteNoise::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    ReflectVar<float>("Seed",
                      [this](float seed) {
                          m_seed = seed;
                          Invalidate();
                      },
                      [this]() { return m_seed; },
                      BANG_REFLECT_HINT_MIN_VALUE(0.0f));

    ReflectVar<float>("Offset",
                      [this](float offset) {
                          m_offset = offset;
                          Invalidate();
                      },
                      [this]() { return m_offset; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));

    ReflectVar<float>("Intensity",
                      [this](float intensity) {
                          m_intensity = intensity;
                          Invalidate();
                      },
                      [this]() { return m_intensity; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));

    // ReflectVar<float>("Frequency",
    //                   [this](float frequency) {
    //                       m_frequency = frequency;
    //                       Invalidate();
    //                   },
    //                   [this]() { return m_frequency; },
    //                   BANG_REFLECT_HINT_SLIDER(0.01f, 10.0f) +
    //                       BANG_REFLECT_HINT_STEP_VALUE(0.1f));

}

Path EffectLayerMaskImplementationWhiteNoise::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureWhiteNoise.bushader");
}

EffectLayerMask::Type
EffectLayerMaskImplementationWhiteNoise::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::WHITE_NOISE;
}

String EffectLayerMaskImplementationWhiteNoise::GetTypeName() const
{
    return "WhiteNoise";
}

void EffectLayerMaskImplementationWhiteNoise::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);
    sp->SetFloat("NoiseIntensity", m_intensity);
    sp->SetFloat("NoiseFrequency", m_frequency);
    sp->SetFloat("NoiseOffset", m_offset);
    sp->SetFloat("NoiseSeed", m_seed);
}
