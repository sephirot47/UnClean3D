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

String EffectLayerDirt::GetUniformName() const
{
    return "DirtTexture";
}

void EffectLayerDirt::SetGenerateEffectUniforms(ShaderProgram *sp)
{
    ControlPanel *cp = GetEffectLayer()->GetControlPanel();
    sp->SetFloat("DirtOctaves", cp->GetDirtOctaves());
    sp->SetFloat("DirtFrequency", cp->GetDirtFrequency());
    sp->SetFloat("DirtFrequencyMultiply", cp->GetDirtFrequencyMultiply());
    sp->SetFloat("DirtAmplitude", cp->GetDirtAmplitude());
    sp->SetFloat("DirtAmplitudeMultiply", cp->GetDirtAmplitudeMultiply());
    sp->SetFloat("DirtSeed", cp->GetDirtSeed());
}
