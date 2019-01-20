#include "EffectLayerNormalLines.h"

#include "Bang/Paths.h"

using namespace Bang;

EffectLayerNormalLines::EffectLayerNormalLines()
{
}

EffectLayerNormalLines::~EffectLayerNormalLines()
{
}

EffectLayer::Type EffectLayerNormalLines::GetEffectLayerType() const
{
    return EffectLayer::Type::NORMAL_LINES;
}

String EffectLayerNormalLines::GetTypeName() const
{
    return "Normal Lines";
}

Path EffectLayerNormalLines::GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectTextureNormalLines.bushader");
}

void EffectLayerNormalLines::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    EffectLayerImplementation::SetGenerateEffectUniforms(sp);

    sp->SetFloat("NormalLinesHeight", GetParameters().normalLinesHeight);
    sp->SetFloat("NormalLinesFrequency",
                 (30.0f - GetParameters().normalLinesWidth));
}
