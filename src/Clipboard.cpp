#include "Clipboard.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/Texture2D.h"

#include "EffectLayer.h"
#include "EffectLayerImplementation.h"
#include "MainScene.h"

using namespace Bang;

Clipboard::Clipboard()
{
    m_maskTextureCopy = Assets::Create<Texture2D>();
}

Clipboard::~Clipboard()
{
}

void Clipboard::CopyEffectLayer(EffectLayer *effectLayer)
{
    m_effectLayerMeta = effectLayer->GetMeta();
    m_effectLayerImplementationMeta =
        effectLayer->GetImplementation()
            ? effectLayer->GetImplementation()->GetMeta()
            : MetaNode();
    m_hasCopiedEffectLayer = true;
}

bool Clipboard::HasCopiedEffectLayer() const
{
    return m_hasCopiedEffectLayer;
}

void Clipboard::PasteEffectLayer(EffectLayer *effectLayerDestiny)
{
    effectLayerDestiny->ImportMeta(m_effectLayerMeta);
    if (EffectLayerImplementation *impl =
            effectLayerDestiny->GetImplementation())
    {
        impl->ImportMeta(m_effectLayerImplementationMeta);
    }
}

void Clipboard::CopyMaskTexture(Texture2D *maskTexture)
{
    m_maskTextureCopy.Get()->Resize(maskTexture->GetSize());
    GEngine::GetInstance()->CopyTexture(maskTexture, m_maskTextureCopy.Get());
    m_hasCopiedMaskTexture = true;
}

bool Clipboard::HasCopiedMaskTexture() const
{
    return m_hasCopiedMaskTexture;
}

void Clipboard::PasteMaskTexture(Texture2D *maskTextureDestiny) const
{
    maskTextureDestiny->Resize(m_maskTextureCopy.Get()->GetSize());
    GEngine::GetInstance()->CopyTexture(m_maskTextureCopy.Get(),
                                        maskTextureDestiny);
}

Clipboard *Clipboard::GetInstance()
{
    return MainScene::GetInstance()->GetClipboard();
}
