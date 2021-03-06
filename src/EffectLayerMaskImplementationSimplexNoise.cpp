#include "EffectLayerMaskImplementationSimplexNoise.h"

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

EffectLayerMaskImplementationSimplexNoise::EffectLayerMaskImplementationSimplexNoise()
{
    m_seed = Random::GetRange(0, 1000);
    m_amplitude = Random::GetRange(0.5f, 2.0f);
}

EffectLayerMaskImplementationSimplexNoise::~EffectLayerMaskImplementationSimplexNoise()
{
}

void EffectLayerMaskImplementationSimplexNoise::Reflect()
{
    EffectLayerMaskImplementationGPU::Reflect();

    ReflectVar<float>("Intensity",
                      [this](float intensity) {
                          m_amplitude = intensity;
                          Invalidate();
                      },
                      [this]() { return m_amplitude; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Offset",
                      [this](float offset) {
                          m_offset = offset;
                          Invalidate();
                      },
                      [this]() { return m_offset; },
                      BANG_REFLECT_HINT_SLIDER(-1.0f, 1.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Stains size",
                      [this](float stainsSize) {
                          m_stainsSize = stainsSize;
                          Invalidate();
                      },
                      [this]() { return m_stainsSize; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 25.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Grain",
                      [this](float grain) {
                          m_grain = grain;
                          Invalidate();
                      },
                      [this]() { return m_grain; },
                      BANG_REFLECT_HINT_SLIDER(1.0f, 10.0f) +
                          BANG_REFLECT_HINT_STEP_VALUE(0.1f));
    ReflectVar<float>("Seed",
                      [this](float seed) {
                          m_seed = seed;
                          Invalidate();
                      },
                      [this]() { return m_seed; },
                      BANG_REFLECT_HINT_MIN_VALUE(0.0f));
}

Path EffectLayerMaskImplementationSimplexNoise::
    GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectMaskTextureSimplexNoise.bushader");
}

EffectLayerMask::Type
EffectLayerMaskImplementationSimplexNoise::GetEffectLayerMaskType() const
{
    return EffectLayerMask::Type::SIMPLEX_NOISE;
}

String EffectLayerMaskImplementationSimplexNoise::GetTypeName() const
{
    return "SimplexNoise";
}

void EffectLayerMaskImplementationSimplexNoise::SetGenerateEffectUniforms(
    ShaderProgram *sp,
    MeshRenderer *meshRend)
{
    EffectLayerMaskImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);

    sp->SetFloat("FractalOctaves", 8.0f);
    sp->SetFloat("FractalOffset", m_offset);
    sp->SetFloat("FractalFrequency", (1.0f / m_stainsSize));
    sp->SetFloat("FractalFrequencyMultiply", m_grain);
    sp->SetFloat("FractalAmplitude", m_amplitude);
    sp->SetFloat("FractalAmplitudeMultiply", (1.0f / m_grain));
    sp->SetFloat("FractalSeed", m_seed);
}
