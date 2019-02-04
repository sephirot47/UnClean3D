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

void EffectLayerMaskImplementation::SetEffectLayerMask(
    EffectLayerMask *effectLayerMask)
{
    p_effectLayerMask = effectLayerMask;
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
                GetEffectLayerMask()->Invalidate();
                GetEffectLayerMask()->GenerateMask();
            });
    }
}

void EffectLayerMaskImplementation::Invalidate()
{
    m_isValid = false;
    GetEffectLayerMask()->Invalidate();
}
