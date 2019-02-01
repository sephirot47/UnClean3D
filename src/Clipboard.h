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
class EffectLayerMask;

class Clipboard
{
public:
    Clipboard();
    virtual ~Clipboard();

    void CopyEffectLayer(EffectLayer *effectLayer);
    bool HasCopiedEffectLayer() const;
    void PasteEffectLayer(EffectLayer *effectLayerDestiny);

    void CopyEffectLayerMask(EffectLayerMask *mask);
    bool HasCopiedEffectLayerMask() const;
    void PasteEffectLayerMask(EffectLayerMask *maskDestiny) const;

    static Clipboard *GetInstance();

private:
    AH<Texture2D> m_copiedMaskTexture;
    bool m_hasCopiedEffectLayerMask = false;
    bool m_hasCopiedEffectLayer = false;

    MetaNode m_effectLayerMeta;
    MetaNode m_effectLayerMaskMeta;
    MetaNode m_effectLayerMaskImplementationMeta;
};

#endif  // CLIPBOARD_H
