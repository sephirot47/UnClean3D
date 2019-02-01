#include "EffectLayerMask.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"

using namespace Bang;

EffectLayerMask::EffectLayerMask()
{
    m_maskTexture = Assets::Create<Texture2D>();
    m_maskTexture.Get()->Fill(Color::Zero(), 1, 1);
}

EffectLayerMask::~EffectLayerMask()
{
}

void EffectLayerMask::SetEffectLayer(EffectLayer *effectLayer)
{
    p_effectLayer = effectLayer;
}

void EffectLayerMask::Clear()
{
    GEngine *ge = GEngine::GetInstance();
    ge->FillTexture(GetMaskTexture(), Color::Zero());
}

void EffectLayerMask::Fill()
{
    GEngine *ge = GEngine::GetInstance();
    ge->FillTexture(GetMaskTexture(), Color::One());
}

Texture2D *EffectLayerMask::GetMaskTexture() const
{
    return m_maskTexture.Get();
}

EffectLayer *EffectLayerMask::GetEffectLayer() const
{
    return p_effectLayer;
}

void EffectLayerMask::Reflect()
{
    Serializable::Reflect();
}
