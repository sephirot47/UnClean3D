#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class Texture2D;
}

using namespace Bang;

class Clipboard
{
public:
    Clipboard();
    virtual ~Clipboard();

    void CopyMaskTexture(Texture2D *maskTexture);
    bool HasCopiedMaskTexture() const;
    void PasteMaskTexture(Texture2D *maskTextureDestiny) const;

    static Clipboard *GetInstance();

private:
    AH<Texture2D> m_maskTextureCopy;
};

#endif  // CLIPBOARD_H
