#include "Clipboard.h"

#include "Bang/Assets.h"
#include "Bang/GEngine.h"
#include "Bang/Texture2D.h"

#include "MainScene.h"

using namespace Bang;

Clipboard::Clipboard()
{
    m_maskTextureCopy = Assets::Create<Texture2D>();
    m_maskTextureCopy.Get()->Resize(2, 2);
}

Clipboard::~Clipboard()
{
}

void Clipboard::CopyMaskTexture(Texture2D *maskTexture)
{
    m_maskTextureCopy.Get()->Resize(maskTexture->GetSize());
    GEngine::GetInstance()->CopyTexture(maskTexture, m_maskTextureCopy.Get());
}

bool Clipboard::HasCopiedMaskTexture() const
{
    return m_maskTextureCopy.Get()->GetWidth() != 2;
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
