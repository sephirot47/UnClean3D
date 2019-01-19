#include "EffectLayerDirt.h"

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

EffectLayerDirt::EffectLayerDirt()
{
}

EffectLayerDirt::~EffectLayerDirt()
{
}

Path EffectLayerDirt::GetGenerateEffectTextureShaderProgramPath() const
{
    return Paths::GetProjectAssetsDir().Append("Shaders").Append(
        "GenerateEffectTextureDirt.bushader");
}

EffectLayer::Type EffectLayerDirt::GetEffectLayerType() const
{
    return EffectLayer::Type::DIRT;
}

String EffectLayerDirt::GetTypeName() const
{
    return "Dirt";
}

const EffectLayerParameters &EffectLayerDirt::GetParameters() const
{
    return GetEffectLayer()->GetParameters();
}

void EffectLayerDirt::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    EffectLayerImplementation::SetGenerateEffectUniforms(sp);

    sp->SetFloat("DirtOctaves", 4.0f);
    sp->SetFloat("DirtFrequency", 10.3f - GetParameters().dirtFrequency);
    sp->SetFloat("DirtFrequencyMultiply",
                 GetParameters().dirtFrequencyMultiply);
    sp->SetFloat("DirtAmplitude", GetParameters().dirtAmplitude);
    sp->SetFloat("DirtAmplitudeMultiply",
                 GetParameters().dirtAmplitudeMultiply);
    sp->SetFloat("DirtSeed", GetParameters().dirtSeed);
    sp->SetColor("DirtColor0", GetParameters().dirtColor0);
    sp->SetColor("DirtColor1", GetParameters().dirtColor1);
}
