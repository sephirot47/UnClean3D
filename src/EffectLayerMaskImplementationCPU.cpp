#include "EffectLayerMaskImplementationCPU.h"

#include "Bang/Assets.h"

#include "MainScene.h"
#include "View3DScene.h"

using namespace Bang;

EffectLayerMaskImplementationCPU::EffectLayerMaskImplementationCPU()
{
    m_image.Create(1, 1);
}

EffectLayerMaskImplementationCPU::~EffectLayerMaskImplementationCPU()
{
}

void EffectLayerMaskImplementationCPU::Init()
{
    EffectLayerMaskImplementationCPU::Init();
}

void EffectLayerMaskImplementationCPU::GenerateEffectMaskTexture(
    Texture2D *effectTexture,
    MeshRenderer *meshRend)
{
    m_image.Resize(effectTexture->GetWidth(), effectTexture->GetHeight());
    GenerateEffectMaskImage(&m_image, meshRend);
    effectTexture->Import(m_image);
}

bool EffectLayerMaskImplementationCPU::CanGenerateEffectMaskTextureInRealTime()
    const
{
    return false;
}

void EffectLayerMaskImplementationCPU::Reflect()
{
    EffectLayerMaskImplementationCPU::Reflect();
}
