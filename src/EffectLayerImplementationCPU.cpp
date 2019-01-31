#include "EffectLayerImplementationCPU.h"

#include "Bang/Assets.h"

#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerImplementationCPU::EffectLayerImplementationCPU()
{
    m_image.Create(1, 1);
}

EffectLayerImplementationCPU::~EffectLayerImplementationCPU()
{
}

void EffectLayerImplementationCPU::Init()
{
    EffectLayerImplementation::Init();
}

void EffectLayerImplementationCPU::GenerateEffectTexture(
    Texture2D *effectTexture,
    MeshRenderer *meshRend)
{
    m_image.Resize(effectTexture->GetWidth(), effectTexture->GetHeight());
    GenerateEffectImage(&m_image, meshRend);
    effectTexture->Import(m_image);
}

bool EffectLayerImplementationCPU::CanGenerateEffectTextureInRealTime() const
{
    return false;
}

void EffectLayerImplementationCPU::Reflect()
{
    EffectLayerImplementation::Reflect();
}
