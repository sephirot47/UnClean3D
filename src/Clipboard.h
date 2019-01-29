#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "Bang/AssetHandle.h"
#include "Bang/Bang.h"
#include "Bang/MetaNode.h"
#include "BangEditor/BangEditor.h"

namespace Bang
{
class Texture2D;
}

using namespace Bang;

class EffectLayer;

class Clipboard
{
public:
    Clipboard();
    virtual ~Clipboard();

    void CopyEffectLayer(EffectLayer *effectLayer);
    bool HasCopiedEffectLayer() const;
    void PasteEffectLayer(EffectLayer *effectLayerDestiny);

    void CopyMaskTexture(Texture2D *maskTexture);
    bool HasCopiedMaskTexture() const;
    void PasteMaskTexture(Texture2D *maskTextureDestiny) const;

    static Clipboard *GetInstance();

private:
    bool m_hasCopiedMaskTexture = false;
    AH<Texture2D> m_maskTextureCopy;

    bool m_hasCopiedEffectLayer = false;
    MetaNode m_effectLayerMeta;
    MetaNode m_effectLayerImplementationMeta;
};

#endif  // CLIPBOARD_H
