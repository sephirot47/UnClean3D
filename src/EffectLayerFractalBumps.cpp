#include "EffectLayerFractalBumps.h"

#include "Bang/Paths.h"

using namespace Bang;

EffectLayerFractalBumps::EffectLayerFractalBumps()
{
}

EffectLayerFractalBumps::~EffectLayerFractalBumps()
{
}

void EffectLayerFractalBumps::Reflect()
{
    ReflectVar<float>("Height",
                      [this](float height) {
                          m_height = height;
                          Invalidate();
                      },
                      [this]() { return m_height; },
                      BANG_REFLECT_HINT_SLIDER(0.0f, 1.0f));

    ReflectVar<float>("Frequency",
                      [this](float frequency) {
                          m_frequency = frequency;
                          Invalidate();
                      },
                      [this]() { return m_frequency; },
                      BANG_REFLECT_HINT_SLIDER(0.01f, 30.0f));
}

EffectLayer::Type EffectLayerFractalBumps::GetEffectLayerType() const
{
    return EffectLayer::Type::FRACTAL_BUMPS;
}

String EffectLayerFractalBumps::GetTypeName() const
{
    return "Fractal bumps";
}

Path EffectLayerFractalBumps::GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectTextureFractalBumps.bushader");
}

void EffectLayerFractalBumps::SetGenerateEffectUniforms(ShaderProgram *sp,
                                                        MeshRenderer *meshRend)
{
    EffectLayerImplementationGPU::SetGenerateEffectUniforms(sp, meshRend);

    sp->SetFloat("FractalBumpsHeight", m_height);
    sp->SetFloat("FractalBumpsFrequency", m_frequency);
}
