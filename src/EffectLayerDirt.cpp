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

EffectLayer::BlendMode EffectLayerDirt::GetBlendMode() const
{
    return EffectLayer::BlendMode::INV_MULT;
}

const ControlPanel::Parameters &EffectLayerDirt::GetParameters() const
{
    return GetEffectLayer()->GetParameters();
}

void EffectLayerDirt::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    sp->SetFloat("DirtOctaves", 4.0f);
    sp->SetFloat("DirtFrequency", 5.3f - GetParameters().m_dirtFrequency);
    sp->SetFloat("DirtFrequencyMultiply",
                 GetParameters().m_dirtFrequencyMultiply);
    sp->SetFloat("DirtAmplitude", GetParameters().m_dirtAmplitude);
    sp->SetFloat("DirtAmplitudeMultiply",
                 GetParameters().m_dirtAmplitudeMultiply);
    sp->SetFloat("DirtSeed", GetParameters().m_dirtSeed);
}
