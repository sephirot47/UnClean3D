#include "EffectLayerMask.h"

#include "Bang/Assets.h"

using namespace Bang;

EffectLayerMask::EffectLayerMask()
{
    m_maskTexture = Assets::Create<Texture2D>();
    m_maskTexture.Get()->CreateEmpty(1, 1);
}

EffectLayerMask::~EffectLayerMask()
{
}
