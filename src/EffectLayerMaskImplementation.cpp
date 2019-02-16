#include "EffectLayerMaskImplementation.h"

#include "Bang/GameObject.h"
#include "Bang/Transform.h"

#include "MainScene.h"

using namespace Bang;

EffectLayerMaskImplementation::EffectLayerMaskImplementation()
{
}

EffectLayerMaskImplementation::~EffectLayerMaskImplementation()
{
    EventEmitter<IEventsDestroy>::PropagateToListeners(
        &IEventsDestroy::OnDestroyed, this);
}

void EffectLayerMaskImplementation::Init()
{
    m_isValid = !CanGenerateEffectMaskTextureInRealTime();
}

void EffectLayerMaskImplementation::ReloadShaders()
{
}

bool EffectLayerMaskImplementation::GetIsPostProcessEffectLayer() const
{
    return false;
}

void EffectLayerMaskImplementation::SetEffectLayerMask(
    EffectLayerMask *effectLayerMask)
{
    p_effectLayerMask = effectLayerMask;
}

void EffectLayerMaskImplementation::GenerateEffectMaskTextureOnCompositeBefore(
    Texture2D *mergedMaskTextureUntilNow,
    MeshRenderer *meshRend)
{
    BANG_UNUSED_2(mergedMaskTextureUntilNow, meshRend);
}

void EffectLayerMaskImplementation::GenerateEffectMaskTextureOnCompositeAfter(
    Texture2D *mergedMaskTextureUntilNow,
    MeshRenderer *meshRend)
{
    BANG_UNUSED_2(mergedMaskTextureUntilNow, meshRend);
}

Texture2D *EffectLayerMaskImplementation::GetMaskTextureToSee() const
{
    return GetEffectLayerMask()->GetMaskTexture();
}

bool EffectLayerMaskImplementation::CompositeThisMask() const
{
    return true;
}

EffectLayerMask *EffectLayerMaskImplementation::GetEffectLayerMask() const
{
    return p_effectLayerMask;
}

void EffectLayerMaskImplementation::Update()
{
}

bool EffectLayerMaskImplementation::IsValid() const
{
    return m_isValid;
}

void EffectLayerMaskImplementation::Reflect()
{
    Serializable::Reflect();

    if (!CanGenerateEffectMaskTextureInRealTime())
    {
        BANG_REFLECT_BUTTON(
            EffectLayerMaskImplementation, "Generate", [this]() {
                GetEffectLayerMask()->Invalidate(true);
                GetEffectLayerMask()->GenerateMask();
            });
    }
}

void EffectLayerMaskImplementation::Invalidate()
{
    m_isValid = false;
    GetEffectLayerMask()->Invalidate();
}
