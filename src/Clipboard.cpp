#include "Clipboard.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/Texture2D.h"

#include "EffectLayer.h"
#include "EffectLayerMaskImplementation.h"
#include "MainScene.h"

using namespace Bang;

Clipboard::Clipboard()
{
    m_copiedMaskTexture = Assets::Create<Texture2D>();
}

Clipboard::~Clipboard()
{
}

void Clipboard::CopyEffectLayer(EffectLayer *effectLayer)
{
    m_effectLayerMeta = effectLayer->GetMeta();
    m_hasCopiedEffectLayer = true;
}

bool Clipboard::HasCopiedEffectLayer() const
{
    return m_hasCopiedEffectLayer;
}

void Clipboard::PasteEffectLayer(EffectLayer *effectLayerDestiny)
{
    effectLayerDestiny->ImportMeta(m_effectLayerMeta);
}

void Clipboard::CopyEffectLayerMask(EffectLayerMask *mask)
{
    m_effectLayerMaskMeta = mask->GetMeta();
    m_effectLayerMaskImplementationMeta =
        mask->GetImplementation() ? mask->GetImplementation()->GetMeta()
                                  : MetaNode();

    m_copiedMaskTexture.Get()->Resize(mask->GetMaskTexture()->GetSize());
    GEngine::GetInstance()->CopyTexture(mask->GetMaskTexture(),
                                        m_copiedMaskTexture.Get());
    m_hasCopiedEffectLayerMask = true;
}

bool Clipboard::HasCopiedEffectLayerMask() const
{
    return m_hasCopiedEffectLayerMask;
}

void Clipboard::PasteEffectLayerMask(EffectLayerMask *maskDestiny) const
{
    maskDestiny->GetMaskTexture()->Resize(m_copiedMaskTexture.Get()->GetSize());
    GEngine::GetInstance()->CopyTexture(m_copiedMaskTexture.Get(),
                                        maskDestiny->GetMaskTexture());
}

Clipboard *Clipboard::GetInstance()
{
    return MainScene::GetInstance()->GetClipboard();
}
