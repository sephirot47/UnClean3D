#include "EffectLayerWaveBumps.h"

#include "Bang/Paths.h"
#include "Bang/Random.h"

using namespace Bang;

EffectLayerWaveBumps::EffectLayerWaveBumps()
{
}

EffectLayerWaveBumps::~EffectLayerWaveBumps()
{
}

void EffectLayerWaveBumps::Reflect()
{
    ReflectVar<float>("Global Height",
                      [this](float height) {
                          m_globalHeight = height;
                          Invalidate();
                      },
                      [this]() { return m_globalHeight; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));
    ReflectVar<float>("Height X",
                      [this](float height) {
                          m_heightX = height;
                          Invalidate();
                      },
                      [this]() { return m_heightX; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));
    ReflectVar<float>("Frequency X",
                      [this](float frequency) {
                          m_frequencyX = frequency;
                          Invalidate();
                      },
                      [this]() { return m_frequencyX; },
                      BANG_REFLECT_HINT_SLIDER(0.01f, FreqLimit));
    ReflectVar<float>("Height Y",
                      [this](float height) {
                          m_heightY = height;
                          Invalidate();
                      },
                      [this]() { return m_heightY; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));
    ReflectVar<float>("Frequency Y",
                      [this](float frequency) {
                          m_frequencyY = frequency;
                          Invalidate();
                      },
                      [this]() { return m_frequencyY; },
                      BANG_REFLECT_HINT_SLIDER(0.01f, FreqLimit));
    ReflectVar<float>("Height Z",
                      [this](float height) {
                          m_heightZ = height;
                          Invalidate();
                      },
                      [this]() { return m_heightZ; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));
    ReflectVar<float>("Frequency Z",
                      [this](float frequency) {
                          m_frequencyZ = frequency;
                          Invalidate();
                      },
                      [this]() { return m_frequencyZ; },
                      BANG_REFLECT_HINT_SLIDER(0.01f, FreqLimit));
}

EffectLayer::Type EffectLayerWaveBumps::GetEffectLayerType() const
{
    return EffectLayer::Type::WAVE_BUMPS;
}

String EffectLayerWaveBumps::GetTypeName() const
{
    return "Wave bumps";
}

Path EffectLayerWaveBumps::GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectTextureWaveBumps.bushader");
}

void EffectLayerWaveBumps::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    EffectLayerImplementationGPU::SetGenerateEffectUniforms(sp);

    sp->SetFloat("WaveBumpsGlobalHeight", m_globalHeight);
    sp->SetFloat("WaveBumpsFrequencyX", m_frequencyX);
    sp->SetFloat("WaveBumpsFrequencyY", m_frequencyY);
    sp->SetFloat("WaveBumpsFrequencyZ", m_frequencyZ);
    sp->SetFloat("WaveBumpsHeightX", m_heightX);
    sp->SetFloat("WaveBumpsHeightY", m_heightY);
    sp->SetFloat("WaveBumpsHeightZ", m_heightZ);
}
